/**
  ******************************************************************************
  * @file    bsp_i2c.c
  * @brief   双 I2C 总线封装。全部阻塞式，只能在主循环上下文调用。
  ******************************************************************************
  * 四个必须知道的坑（都在这份代码里有对应处理）：
  *   1. HAL 超时后 hi2c->State 可能停在 BUSY，之后每次调用都秒回 HAL_BUSY，
  *      看起来像"总线彻底死了"。-> i2c_check() 里 DeInit + Init 掰回 READY。
  *   2. 从机把 SDA 拉死（上电瞬间、热插拔）时，重新初始化也救不回来。
  *      -> Bsp_I2c_BusRecover()：SCL 打 9 拍 + 手动 STOP。
  *   3. 两条总线各自恢复，互不影响（这就是分总线的意义）。
  *   4. 这些函数内部有 HAL 延时等待，绝对不能在中断里调用。
  ******************************************************************************
  */

#include "bsp_i2c.h"
#include "bsp_uart.h"    /* Bsp_Printf */
#include "i2c.h"        /* CubeMX 生成：extern hi2c1/hi2c2; MX_I2C1_Init/MX_I2C2_Init */
#include <stdio.h>

typedef struct {
    I2C_HandleTypeDef *h;
    GPIO_TypeDef      *port;
    uint16_t           scl;
    uint16_t           sda;
    uint32_t           err;
} BspI2cCtx;

/* 总线表：改引脚只改这里，不要散落到各个函数里 */
static BspI2cCtx s_ctx[BSP_I2C_NUM] = {
    { &hi2c1, GPIOB, GPIO_PIN_8,  GPIO_PIN_9,  0U },   /* BSP_I2C1 */
    { &hi2c2, GPIOB, GPIO_PIN_10, GPIO_PIN_11, 0U },   /* BSP_I2C2 */
};

/* -------------------------------------------------------------------------- */
/* 内部                                                                       */
/* -------------------------------------------------------------------------- */
static void i2c_reinit(BspI2cBus bus)
{
    HAL_I2C_DeInit(s_ctx[bus].h);

    if (bus == BSP_I2C1) {
        MX_I2C1_Init();
    } else {
        MX_I2C2_Init();
    }
}

/* 统一检查返回值，顺便救活卡死的状态机 */
static bool i2c_check(BspI2cBus bus, HAL_StatusTypeDef st)
{
    if (st == HAL_OK) {
        return true;
    }

    s_ctx[bus].err++;

    /* 坑 1：状态机卡在 BUSY。不清掉的话，后面每次调用都会立刻返回 HAL_BUSY，
     * 现象是"从某一次开始永远失败"，非常像硬件坏了。 */
    if (s_ctx[bus].h->State != HAL_I2C_STATE_READY) {
        i2c_reinit(bus);
    }

    return false;
}

/* 粗略延时：只用于恢复脉冲，对精度没要求，SCL < 100kHz 即可。
 * 想要更准就换成 bsp_time 里的 DWT 版本。 */
static void i2c_delay_us(uint32_t us)
{
    volatile uint32_t n = us * 140U;   /* 168MHz 下约 1us ≈ 140 次空转 */
    while (n-- > 0U) {
        __NOP();
    }
}

/* -------------------------------------------------------------------------- */
/* 初始化 / 状态查询                                                          */
/* -------------------------------------------------------------------------- */
void Bsp_I2c_Init(BspI2cBus bus)
{
    /* MX_I2Cx_Init() 已由 CubeMX 在 main() 里调用过，这里不再重复初始化，
     * 只清计数，方便区分"开机就有错"还是"跑着才出错"。 */
    s_ctx[bus].err = 0U;

    /* 注意：不在这里探测设备。设备没接时探测失败会让计数虚高，
     * 排查时反而分不清是"没接"还是"接坏了"。探测交给 Bsp_I2c_Scan()。 */
}

bool Bsp_I2c_IsReady(BspI2cBus bus, uint8_t addr7)
{
    return i2c_check(bus, HAL_I2C_IsDeviceReady(s_ctx[bus].h,
                                               (uint16_t)(addr7 << 1),
                                               2U, BSP_I2C_TIMEOUT_MS));
}

/* -------------------------------------------------------------------------- */
/* 带寄存器地址的读写                                                         */
/* -------------------------------------------------------------------------- */
bool Bsp_I2c_MemWrite(BspI2cBus bus, uint8_t addr7, uint8_t reg,
                      const uint8_t *buf, uint16_t len)
{
    if ((buf == NULL) || (len == 0U)) {
        return false;
    }

    return i2c_check(bus, HAL_I2C_Mem_Write(s_ctx[bus].h, (uint16_t)(addr7 << 1),
                                            (uint16_t)reg, I2C_MEMADD_SIZE_8BIT,
                                            (uint8_t *)buf, len, BSP_I2C_TIMEOUT_MS));
}

bool Bsp_I2c_MemRead(BspI2cBus bus, uint8_t addr7, uint8_t reg,
                     uint8_t *buf, uint16_t len)
{
    if ((buf == NULL) || (len == 0U)) {
        return false;
    }

    return i2c_check(bus, HAL_I2C_Mem_Read(s_ctx[bus].h, (uint16_t)(addr7 << 1),
                                           (uint16_t)reg, I2C_MEMADD_SIZE_8BIT,
                                           buf, len, BSP_I2C_TIMEOUT_MS));
}

/* -------------------------------------------------------------------------- */
/* 裸读写                                                                     */
/* -------------------------------------------------------------------------- */
bool Bsp_I2c_Write(BspI2cBus bus, uint8_t addr7, const uint8_t *buf, uint16_t len)
{
    if ((buf == NULL) || (len == 0U)) {
        return false;
    }

    return i2c_check(bus, HAL_I2C_Master_Transmit(s_ctx[bus].h, (uint16_t)(addr7 << 1),
                                                  (uint8_t *)buf, len, BSP_I2C_TIMEOUT_MS));
}

bool Bsp_I2c_Read(BspI2cBus bus, uint8_t addr7, uint8_t *buf, uint16_t len)
{
    if ((buf == NULL) || (len == 0U)) {
        return false;
    }

    return i2c_check(bus, HAL_I2C_Master_Receive(s_ctx[bus].h, (uint16_t)(addr7 << 1),
                                                 buf, len, BSP_I2C_TIMEOUT_MS));
}

/* -------------------------------------------------------------------------- */
/* 调试：总线扫描                                                             */
/* -------------------------------------------------------------------------- */
void Bsp_I2c_Scan(BspI2cBus bus)
{
    uint8_t found = 0U;

    Bsp_Printf("[i2c] scan %s (%s=SCL / %s=SDA)\r\n",
               (bus == BSP_I2C1) ? "I2C1" : "I2C2",
               (bus == BSP_I2C1) ? "PB8"  : "PB10",
               (bus == BSP_I2C1) ? "PB9"  : "PB11");

    for (uint8_t a = 0x08U; a <= 0x77U; a++) {
        /* 每个地址 1 次试探、3ms 超时：112 个地址最坏 0.34s，只在启动时跑 */
        if (HAL_I2C_IsDeviceReady(s_ctx[bus].h, (uint16_t)(a << 1), 1U, 3U) == HAL_OK) {
            Bsp_Printf("[i2c]   0x%02X\r\n", a);
            found++;
        }
    }

    Bsp_Printf("[i2c] %u device(s)\r\n", (unsigned)found);

    if (found == 0U) {
        Bsp_Printf("[i2c] none! 检查引脚复用、供电、上拉、共地\r\n");
    }

    /* 扫了 112 个地址，NACK 多了偶尔会把状态机弄脏，收尾时确认一下 */
    if (s_ctx[bus].h->State != HAL_I2C_STATE_READY) {
        i2c_reinit(bus);
    }
}

/* -------------------------------------------------------------------------- */
/* 总线恢复：从机把 SDA 拉死时的唯一救法                                      */
/* -------------------------------------------------------------------------- */
void Bsp_I2c_BusRecover(BspI2cBus bus)
{
    GPIO_InitTypeDef gpio = {0};
    uint16_t pins = (uint16_t)(s_ctx[bus].scl | s_ctx[bus].sda);

    HAL_I2C_DeInit(s_ctx[bus].h);           /* 先松开外设，把引脚拿回 GPIO */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpio.Pin   = pins;
    gpio.Mode  = GPIO_MODE_OUTPUT_OD;       /* 开漏，靠上拉拉高 */
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(s_ctx[bus].port, &gpio);

    /* 空闲态：两根线都释放为高 */
    HAL_GPIO_WritePin(s_ctx[bus].port, pins, GPIO_PIN_SET);
    i2c_delay_us(5U);

    /* 打 9 个 SCL：让卡在半截的从机把最后 1 bit + ACK 吐完，释放 SDA */
    for (uint8_t i = 0U; i < 9U; i++) {
        HAL_GPIO_WritePin(s_ctx[bus].port, s_ctx[bus].scl, GPIO_PIN_RESET);
        i2c_delay_us(5U);
        HAL_GPIO_WritePin(s_ctx[bus].port, s_ctx[bus].scl, GPIO_PIN_SET);
        i2c_delay_us(5U);
    }

    /* 手动补一个 STOP：SCL 为高时，SDA 由低变高 */
    HAL_GPIO_WritePin(s_ctx[bus].port, s_ctx[bus].scl, GPIO_PIN_RESET); i2c_delay_us(5U);
    HAL_GPIO_WritePin(s_ctx[bus].port, s_ctx[bus].sda, GPIO_PIN_RESET); i2c_delay_us(5U);
    HAL_GPIO_WritePin(s_ctx[bus].port, s_ctx[bus].scl, GPIO_PIN_SET);   i2c_delay_us(5U);
    HAL_GPIO_WritePin(s_ctx[bus].port, s_ctx[bus].sda, GPIO_PIN_SET);   i2c_delay_us(5U);

    i2c_reinit(bus);                        /* 引脚切回 AF4，外设重新使能 */
}

uint32_t Bsp_I2c_ErrorCount(BspI2cBus bus)
{
    return s_ctx[bus].err;
}