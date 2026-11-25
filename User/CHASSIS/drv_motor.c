#include "drv_motor.h"
#include "main.h"
#include "tim.h"
#define MOTOR_PWM_PERIOD   (htim1.Init.Period + 1U)
typedef struct {
    GPIO_TypeDef *port1;   /* IN1 所在端口 */
    uint16_t      pin1;    /* IN1 引脚 */
    GPIO_TypeDef *port2;
    uint16_t      pin2;
    uint32_t      ch;      /* TIM1 通道 */
    int8_t        sign;    /* 方向校准系数 ±1 */
} MotorHw_t;
static const MotorHw_t s_hw[WHEEL_NUM] = {
    
     { FL_IN1_GPIO_Port, FL_IN1_Pin, FL_IN2_GPIO_Port, FL_IN2_Pin, TIM_CHANNEL_1, DRV_MOTOR_SIGN_FL },
     { FR_IN1_GPIO_Port, FR_IN1_Pin, FR_IN2_GPIO_Port, FR_IN2_Pin, TIM_CHANNEL_2, DRV_MOTOR_SIGN_FR },
     { RL_IN1_GPIO_Port, RL_IN1_Pin, RL_IN2_GPIO_Port, RL_IN2_Pin, TIM_CHANNEL_3, DRV_MOTOR_SIGN_RL },
     { RR_IN1_GPIO_Port, RR_IN1_Pin, RR_IN2_GPIO_Port, RR_IN2_Pin, TIM_CHANNEL_4, DRV_MOTOR_SIGN_RR },
};
static int16_t s_pwm[WHEEL_NUM];
static void Motor_WriteDir(const MotorHw_t *hw, int8_t dir)
{
    if (dir > 0) {
        HAL_GPIO_WritePin(hw->port1, hw->pin1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(hw->port2, hw->pin2, GPIO_PIN_RESET);
    } else if (dir < 0) {
        HAL_GPIO_WritePin(hw->port1, hw->pin1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(hw->port2, hw->pin2, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(hw->port1, hw->pin1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(hw->port2, hw->pin2, GPIO_PIN_RESET);
    }
}
static void Motor_WriteBrake(const MotorHw_t *hw)
{
    HAL_GPIO_WritePin(hw->port1, hw->pin1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hw->port2, hw->pin2, GPIO_PIN_SET);
}
static void Motor_WriteCcr(const MotorHw_t *hw, uint32_t ccr)
{
    __HAL_TIM_SET_COMPARE(&htim1, hw->ch, ccr);
}
void DrvMotor_Init(void)
{
    uint32_t i;

    
    for (i = 0; i < WHEEL_NUM; i++) {
        Motor_WriteDir(&s_hw[i], 0);        /* 滑行 */
        Motor_WriteCcr(&s_hw[i], 0U);       /* 占空比 0 */
        s_pwm[i] = 0;
    }
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
}
void DrvMotor_SetPwm(MotorId_t id, int16_t pwm)
{
    int16_t  mag;
    int8_t   dir;
    uint32_t ccr;
    if ((uint32_t)id >= (uint32_t)WHEEL_NUM) return;

    
    if (pwm >  DRV_MOTOR_PWM_MAX) pwm =  DRV_MOTOR_PWM_MAX;
    if (pwm < -DRV_MOTOR_PWM_MAX) pwm = -DRV_MOTOR_PWM_MAX;

    
    if (pwm >= 0) { mag = pwm;           dir =  1; }
    else          { mag = (int16_t)-pwm; dir = -1; }

    
    if (mag < DRV_MOTOR_PWM_MIN) { mag = 0; dir = 0; }

    
    if (mag > DRV_MOTOR_PWM_LIMIT) mag = DRV_MOTOR_PWM_LIMIT;

    
    dir = (int8_t)(dir * s_hw[id].sign);
    ccr = (uint32_t)mag * MOTOR_PWM_PERIOD / DRV_MOTOR_PWM_MAX; 
    Motor_WriteDir(&s_hw[id], dir);
    Motor_WriteCcr(&s_hw[id], ccr);
    s_pwm[id] = (int16_t)(mag * dir);   /* 记录实际输出（含校准后的方向） */
}

void DrvMotor_Brake(MotorId_t id)  
{
     if (id < WHEEL_NUM)
      { 
        Motor_WriteCcr(&s_hw[id].ch, 0U);
        Motor_WriteBrake(&s_hw[id]); 
        s_pwm[id] = 0;
     } 
}
void DrvMotor_Coast(MotorId_t id)
  { 
    if (id < WHEEL_NUM) 
    { 
        Motor_WriteCcr(&s_hw[id].ch, 0U); 
        Motor_WriteDir(&s_hw[id],0); 
        s_pwm[id] = 0; }
 }
 void DrvMotor_BrakeAll(void)
{
    uint32_t i;
    for (i = 0; i < WHEEL_NUM; i++) DrvMotor_Brake((MotorId_t)i);
}

void DrvMotor_CoastAll(void)
{
    uint32_t i;
    for (i = 0; i < WHEEL_NUM; i++) DrvMotor_Coast((MotorId_t)i);
}
int16_t DrvMotor_GetPwm(MotorId_t id)
{
    if ((uint32_t)id >= (uint32_t)WHEEL_NUM) return 0;
    return s_pwm[id];
}