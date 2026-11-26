#ifndef __CHASSIS_H
#define __CHASSIS_H
#include <stdint.h>
/* ================================坐标系===========================================
 车体俯视（x 轴指向车头，y 轴指向车左，右手系 z 向上）：
 *
 *              车头 (vx > 0)
 *                  ▲
 *                  │
 *      FL ◣        │        ◢ FR       
 *                  │
 *   车左(vy>0) ◀───┼───▶ 车右(vy<0)
 *                  │
 *      RL ◤        │        ◥ RR
 *                  ▼
 *              车尾 (vx < 0)
 *
 * Chassis_SetVelocity(vx, vy, w) 的量纲与正方向：
 *   vx : 正 = 向车头方向平移      单位 mm/s    
 *   vy : 正 = 向车左侧平移        单位 mm/s    
 *   w  : 正 = 逆时针旋转(俯视)    单位 度/秒   
 *        
 * 四个轮子编号（与 MotorId_t 的 WHEEL_FL/FR/RL/RR 一一对应）：
 *   FL = 左前    FR = 右前
 *   RL = 左后    RR = 右后
 *
 * 符号速查（记忆法：平移看"往哪边走"，自转看"左右反向"）：
 *   (vx>0, vy=0, w=0)  → 四轮同速正转            前进
 *   (vx<0, vy=0, w=0)  → 四轮同速反转            后退
 *   (vx=0, vy>0, w=0)  → FL/RR 反转、FR/RL 正转  左移
 *   (vx=0, vy=0, w>0)  → FL/RL 反转、FR/RR 正转  左转（逆时针）
 *  ==========================================================================*/
typedef enum {
    CHASSIS_STOP = 0,      /* 停止 */
    CHASSIS_FWD,           /* 前进 */
    CHASSIS_BACK,          /* 后退 */
    CHASSIS_LEFT,          /* 左移 */
    CHASSIS_RIGHT,         /* 右移 */
    CHASSIS_TURN_LEFT,     /* 左转 */
    CHASSIS_TURN_RIGHT,    /* 右转 */
    CHASSIS_MOTION_NUM
} Chassis_Motion_t;/*整车运动方向（给 Chassis_Move 用的动作枚举）。*/
void  Chassis_Init(void);/*初始化底盘：启动电机、清空内部状态。调用时放 main.c 的 USER CODE 2（在 MX_TIM1_Init() 之后）。
它内部会调用 DrvMotor_Init()，所以不要再单独调一次。*/

void  Chassis_SetVelocity(int16_t vx, int16_t vy, int16_t w);/*
设置底盘的运动速度（mm/s，度/s），由 Chassis_Task() 里每 10ms 调用 DrvMotor_SetPwm() 输出到电机。
vx,vy的范围是60到100（因为还没测试此处的速度不是实际的速度，而是通过v/vmax算出pwm的占空比，vmax是100，v不能小于60，因为有电压损失，小于60就不转了
同理，w的范围是-25—25，当w为-15—15时，电机不转）*/
void  Chassis_Stop(void); /*立即停车（制动，不是滑行）：目标值/当前值全部清零并刹车*/
void  Chassis_GetWheelSpeed(int16_t out[4]); /*没装编码器所以没有使用*/
void  Chassis_Task(void);
/**
 * @brief  周期执行器：失联自检 → 斜坡限速 → 下发到电机。全程序唯一的"碰硬件"入口。
 * @par    何时调用
 *         **每 10ms 调一次，无条件调用**（不论有没有新指令）。
 *         main.c 的 while 里直接调也行，但那样斜坡会随循环速度变化；
 *         正式做法是用 TIM 中断或 systick 做 10ms 节拍。
 * @par    示例
 *         // 10ms 定时中断里：
 *         Chassis_Task();
 * @warning · 不能在里面加 HAL_Delay；
 *          · 必须每轮都调，否则车不会动、失联保护也不会生效；
 *          · 它的执行周期决定了斜坡速度：每次最多变 60‰，
 *            10ms 一次 → 0→600‰ 约 100ms。
 */
void Chassis_Move(Chassis_Motion_t dir, int16_t speed);
/**
 * @brief  用一个动作枚举 + 一个速度，控制整车走一个方向。推荐优先使用此函数，替代 Chassis_SetVelocity。
 *         复杂运动（边走边转）仍要用 Chassis_SetVelocity。（虽然我觉得用不到）
 * @param  dir    CHASSIS_FWD / BACK / LEFT / RIGHT / TURN_LEFT / TURN_RIGHT / STOP
 * @param  speed  取绝对值，方向只由 dir 决定。
 *                **单位随动作变化**：
 *                  平移类（FWD/BACK/LEFT/RIGHT）→ mm/s，建议 60~100
 *                  旋转类（TURN_LEFT/TURN_RIGHT）→ °/s，建议 15~25
 * @par    何时调用
 *         测试、演示、简单遥控。
 * @par    示例
 *         Chassis_Move(CHASSIS_FWD, 300);        // 前进
 *         Chassis_Move(CHASSIS_TURN_LEFT, 90);   // 原地 90°/s 左转
 *         Chassis_Move(CHASSIS_STOP, 0);         // 停车（内部走 Chassis_Stop）
 */

#endif