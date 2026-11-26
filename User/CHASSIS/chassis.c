#include "drv_motor.h"
#include "usart.h"
#include "main.h"
#include <stdio.h>

#define CHASSIS_L_MM         150.533  /* 轴距：前后轮中心距 */
#define CHASSIS_W_MM         310.735  /* 轮距：左右轮中心距 */
#define CHASSIS_VMAX_MM_S    100   /* 千分比 1000 时轮面线速度 */
#define CHASSIS_K_MM        ((CHASSIS_L_MM + CHASSIS_W_MM)/2)
#define CHASSIS_SLEW_PER_TICK    60    /* 每 10ms 最多变化 60‰，0→600‰ 约 100ms */
#define CHASSIS_CMD_TIMEOUT_MS  300    /* 与 pad.h 契约一致：300ms 没新指令就停 */


static char s_tx[64];                 /* ★ 必须是 static：DMA 是异步的，不能用栈变量 */
static volatile uint8_t s_tx_busy = 0;
static uint32_t s_last_ms = 0;


static int16_t  s_target_pwm[WHEEL_NUM];   /* 解算目标 */
static int16_t  s_now_pwm[WHEEL_NUM];      /* 当前实际（斜坡输出） */
static int16_t  s_wheel_mm_s[WHEEL_NUM];   /* 解算出来的轮面速度 */
static uint32_t s_last_cmd_ms;
static uint8_t  s_running;

typedef enum {
    CHASSIS_STOP = 0,      /* 停止（立即制动，不是滑行） */
    CHASSIS_FWD,           /* 前进：朝车头 */
    CHASSIS_BACK,          /* 后退 */
    CHASSIS_LEFT,          /* 左平移：朝车左 */
    CHASSIS_RIGHT,         /* 右平移 */
    CHASSIS_TURN_LEFT,     /* 原地左转（逆时针） */
    CHASSIS_TURN_RIGHT,    /* 原地右转（顺时针） */
    CHASSIS_MOTION_NUM
} Chassis_Motion_t;





void Chassis_Init(void)
{
    MotorId_t i;
    DrvMotor_Init();
    for (i = 0; i < WHEEL_NUM; i++) { s_target_pwm[i] = 0; s_now_pwm[i] = 0; s_wheel_mm_s[i] = 0; }
    s_last_cmd_ms = HAL_GetTick();
    s_running = 0;
}
void Chassis_SetVelocity(int16_t vx,int16_t vy,int16_t w)
{
    int32_t v_rot, v[WHEEL_NUM], maxv;
    int32_t i;

    
    if (w >  1800) w =  1800;
    if (w < -1800) w = -1800;

    
    v_rot = ((int32_t)w * CHASSIS_K_MM * 1745) / 100000;   
    v[WHEEL_FL] = (int32_t)vx - vy - v_rot;
    v[WHEEL_FR] = (int32_t)vx + vy + v_rot;
    v[WHEEL_RL] = (int32_t)vx + vy - v_rot;
    v[WHEEL_RR] = (int32_t)vx - vy + v_rot;
    maxv = 0;
    for (i = 0; i < WHEEL_NUM; i++) {
        int32_t a = (v[i] < 0) ? -v[i] : v[i];
        if (a > maxv) maxv = a;
    }
    if (maxv > CHASSIS_VMAX_MM_S) {
        for (i = 0; i < WHEEL_NUM; i++) v[i] = v[i] * CHASSIS_VMAX_MM_S / maxv;
    }

    for (i = 0; i < WHEEL_NUM; i++) {
        s_wheel_mm_s[i] = (int16_t)v[i];
        s_target_pwm[i]  = (int16_t)(v[i] * DRV_MOTOR_PWM_MAX / CHASSIS_VMAX_MM_S);
    }

    s_last_cmd_ms = HAL_GetTick();
    s_running = 1;
}
void Chassis_Stop(void)
{
    MotorId_t i;
    for (i = 0; i < WHEEL_NUM; i++) {
        s_target_pwm[i] = 0;
        s_now_pwm[i]= 0;
        DrvMotor_Brake(i);          
    s_running = 0;
}
void Chassis_Task(void)
{
    int32_t  i, d;
    int16_t  p;

    /* 失联双保险：APP 会用 Pad_IsAlive() 判断，但底盘自己也兜一层 */
    if (s_running && (HAL_GetTick() - s_last_cmd_ms) > CHASSIS_CMD_TIMEOUT_MS) {
        Chassis_Stop();
        return;
    }

    for (i = 0; i < WHEEL_NUM; i++) {
        d = (int32_t)s_target_pwm[i] - s_now_pwm[i];
        if (d >  CHASSIS_SLEW_PER_TICK) d =  CHASSIS_SLEW_PER_TICK;
        if (d < -CHASSIS_SLEW_PER_TICK) d = -CHASSIS_SLEW_PER_TICK;
        s_now_pwm[i] = (int16_t)(s_now_pwm[i] + d);
        DrvMotor_SetPwm((MotorId_t)i, s_now_pwm[i]);
        (void)p;
    }
}
void Chassis_Move(Chassis_Motion_t dir, int16_t speed)
{
    /* 单位向量表：[0]=vx [1]=vy [2]=w —— 与 Chassis_SetVelocity 的三轴一一对应 */
    static const int8_t k[CHASSIS_MOTION_NUM][3] = {
        {  0,  0,  0 },    /* STOP       */
        {  1,  0,  0 },    /* FWD        */
        { -1,  0,  0 },    /* BACK       */
        {  0,  1,  0 },    /* LEFT       */
        {  0, -1,  0 },    /* RIGHT      */
        {  0,  0,  1 },    /* TURN_LEFT  */
        {  0,  0, -1 },    /* TURN_RIGHT */
    };

    if ((uint32_t)dir >= (uint32_t)CHASSIS_MOTION_NUM) return;  /* 越界保护 */
    if (speed < 0) speed = (int16_t)-speed;                    /* 方向只看枚举，速度取绝对值 */

    if (dir == CHASSIS_STOP) { Chassis_Stop(); return; }

    Chassis_SetVelocity((int16_t)(k[dir][0] * speed),
                        (int16_t)(k[dir][1] * speed),
                        (int16_t)(k[dir][2] * speed));
}




/* 每 10ms 发一帧（100Hz），由 main 的 while(1) 调用 */
void Vofa_Task(void)
{
    int n;
    uint32_t now = HAL_GetTick();

    if (now - s_last_ms < 10U) return;      /* 10ms 节拍 */
    s_last_ms = now;

    if (s_tx_busy) return;                  /* 上一帧没发完就跳过，绝不阻塞 */

    n = sprintf(s_tx, "FL:%d,FR:%d,RL:%d,RR:%d\n",
                DrvMotor_GetPwm(WHEEL_FL), DrvMotor_GetPwm(WHEEL_FR),
                DrvMotor_GetPwm(WHEEL_RL), DrvMotor_GetPwm(WHEEL_RR));
    if (n <= 0 || n >= (int)sizeof(s_tx)) return;

    s_tx_busy = 1;
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)s_tx, (uint16_t)n);
}

/* 覆盖 HAL 的弱回调，发送完成清忙标志 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) s_tx_busy = 0;
}
