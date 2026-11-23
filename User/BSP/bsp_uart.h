#ifndef BSP_UART_H
#define BSP_UART_H

#include "stdint.h"

void Bsp_Printf(const char *fmt, ...);             /* 推荐使用，支持格式化输出（类似c语言的printf） */
void Bsp_Send(const uint8_t *buf, uint16_t len);

#endif