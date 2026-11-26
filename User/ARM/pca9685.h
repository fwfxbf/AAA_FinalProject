#ifndef __PCA9685_H
#define __PCA9685_H

#include "main.h"

#define PCA9685_ADDR      0x40

#define PCA9685_ADDR8     ((uint16_t)(PCA9685_ADDR << 1))

#define PCA9685_MODE1     0x00
#define PCA9685_MODE2     0x01
#define PCA9685_PRESCALE  0xFE
#define PCA9685_LED0_ON_L 0x06

#define PCA9685_MODE1_RESTART  0x80  
#define PCA9685_MODE1_EXTCLK   0x40  
#define PCA9685_MODE1_AI       0x20  
#define PCA9685_MODE1_SLEEP    0x10  
#define PCA9685_MODE1_ALLCALL  0x01  

#define SERVO_FREQ        50

#define SERVO_MIN_PWM     102
#define SERVO_MAX_PWM     512
#define SERVO_MID_PWM     307

HAL_StatusTypeDef PCA9685_Init(void);
HAL_StatusTypeDef PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off);
HAL_StatusTypeDef PCA9685_SetServoAngle(uint8_t channel, uint8_t angle);

#endif
