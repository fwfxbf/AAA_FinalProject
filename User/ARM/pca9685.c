#include "pca9685.h"
#include "i2c.h"

static HAL_StatusTypeDef PCA9685_WriteByte(uint8_t reg, uint8_t val)
{
    return HAL_I2C_Mem_Write(&hi2c1, PCA9685_ADDR8, reg, I2C_MEMADD_SIZE_8BIT,
                             &val, 1, 100);
}

static HAL_StatusTypeDef PCA9685_ReadByte(uint8_t reg, uint8_t *val)
{
    return HAL_I2C_Mem_Read(&hi2c1, PCA9685_ADDR8, reg, I2C_MEMADD_SIZE_8BIT,
                            val, 1, 100);
}

HAL_StatusTypeDef PCA9685_Init(void)
{
    HAL_StatusTypeDef st;
    uint8_t prescale;
    uint8_t mode1;

    prescale = (uint8_t)(25000000.0f / (4096.0f * SERVO_FREQ) - 1.0f + 0.5f);

    st = PCA9685_WriteByte(PCA9685_MODE1, PCA9685_MODE1_SLEEP | PCA9685_MODE1_ALLCALL);
    if (st != HAL_OK) return st;
    HAL_Delay(2);

    st = PCA9685_WriteByte(PCA9685_PRESCALE, prescale);
    if (st != HAL_OK) return st;

    st = PCA9685_WriteByte(PCA9685_MODE1,
                           PCA9685_MODE1_SLEEP | PCA9685_MODE1_AI | PCA9685_MODE1_ALLCALL);
    if (st != HAL_OK) return st;
    HAL_Delay(2);

    st = PCA9685_WriteByte(PCA9685_MODE1, PCA9685_MODE1_AI | PCA9685_MODE1_ALLCALL);
    if (st != HAL_OK) return st;
    HAL_Delay(5);

    mode1 = PCA9685_MODE1_RESTART | PCA9685_MODE1_AI | PCA9685_MODE1_ALLCALL;
    st = PCA9685_WriteByte(PCA9685_MODE1, mode1);
    if (st != HAL_OK) return st;

    st = PCA9685_WriteByte(PCA9685_MODE2, 0x04);
    if (st != HAL_OK) return st;

    for (uint8_t ch = 0; ch < 16; ch++) {
        st = PCA9685_SetPWM(ch, 0, 0);
        if (st != HAL_OK) return st;
    }
    return HAL_OK;
}

HAL_StatusTypeDef PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off)
{
    uint8_t buf[4];

    if (channel > 15) return HAL_ERROR;
    if (on  > 0x0FFF) on  = 0x0FFF;
    if (off > 0x0FFF) off = 0x0FFF;

    uint8_t reg = PCA9685_LED0_ON_L + channel * 4;
    buf[0] = (uint8_t)(on & 0xFF);
    buf[1] = (uint8_t)(on >> 8);
    buf[2] = (uint8_t)(off & 0xFF);
    buf[3] = (uint8_t)(off >> 8);

    return HAL_I2C_Mem_Write(&hi2c1, PCA9685_ADDR8, reg, I2C_MEMADD_SIZE_8BIT,
                             buf, 4, 100);
}

HAL_StatusTypeDef PCA9685_SetServoAngle(uint8_t channel, uint8_t angle)
{
    if (angle > 180) angle = 180;
    uint16_t pwm = SERVO_MIN_PWM +
                   (uint16_t)(((uint32_t)(SERVO_MAX_PWM - SERVO_MIN_PWM) * angle) / 180);
    return PCA9685_SetPWM(channel, 0, pwm);
}
