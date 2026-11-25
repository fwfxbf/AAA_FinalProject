/**
  ******************************************************************************
  * @file    bsp_i2c.h
  * @brief   双 I2C 总线封装（STM32F407VET6 + HAL）
  *
  * 两条总线，两个不同电压域的设备，故意不合并：
  *
  *   BSP_I2C1  PB8=SCL / PB9=SDA   -> PCA9685 舵机板（3.3V 逻辑）
  *   BSP_I2C2  PB10=SCL / PB11=SDA -> YB-MUX04 循迹板（5V 供电，板上带 5V 上拉）
  *
  * 分开的三个理由：
  *   1. 电平：循迹板把总线拉到 5V，PCA9685 的 SDA/SCL 能不能吃 5V 要看手册，
  *      拿不准就别赌 —— 两条总线，各在各的电压域；
  *   2. 隔离：一条被拽死不会拖垮另一条（舵机挂了车还能循迹，反之亦然）；
  *   3. 地址：不用赌循迹板的固定地址会不会撞上 PCA9685 的 0x40。
  *
  * 所有函数都是阻塞式，只能在主循环上下文调用，禁止在中断里调。
  ******************************************************************************
  */
#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* 单次传输超时。400kHz 下 32 字节约 0.8ms，20ms 已经非常宽松；
 * 卡住时宁可快速失败返回 false，也不要让 50Hz 控制周期被拖死。 */
#define BSP_I2C_TIMEOUT_MS     20U

#define BSP_I2C_ADDR_PCA9685   0x40U   /* 7 位地址，A5~A0 全接地 */

typedef enum {
    BSP_I2C1 = 0,   /* PB8 / PB9  */
    BSP_I2C2,       /* PB10 / PB11 */
    BSP_I2C_NUM
} BspI2cBus;

void     Bsp_I2c_Init(BspI2cBus bus);

bool     Bsp_I2c_IsReady(BspI2cBus bus, uint8_t addr7);

/* 带寄存器地址的读写：PCA9685 和循迹板都是这种（先发 1 字节寄存器，再收发数据） */
bool     Bsp_I2c_MemWrite(BspI2cBus bus, uint8_t addr7, uint8_t reg,
                          const uint8_t *buf, uint16_t len);
bool     Bsp_I2c_MemRead (BspI2cBus bus, uint8_t addr7, uint8_t reg,
                                uint8_t *buf, uint16_t len);

/* 裸读写：偶尔要发没有寄存器地址的字节流时用 */
bool     Bsp_I2c_Write(BspI2cBus bus, uint8_t addr7, const uint8_t *buf, uint16_t len);
bool     Bsp_I2c_Read (BspI2cBus bus, uint8_t addr7,       uint8_t *buf, uint16_t len);

/* 调试：扫 0x08~0x77，用 Bsp_Printf 打印。会阻塞数百 ms，只在启动时调。 */
void     Bsp_I2c_Scan(BspI2cBus bus);

/* 总线恢复：SCL 手动打 9 拍 + 补 STOP + 重新初始化。调用后引脚仍是 I2C。 */
void     Bsp_I2c_BusRecover(BspI2cBus bus);

/* 累计出错次数（按总线分开计）：调试时看它涨不涨，比看波形快 */
uint32_t Bsp_I2c_ErrorCount(BspI2cBus bus);

#ifdef __cplusplus
}
#endif
#endif /* __BSP_I2C_H */