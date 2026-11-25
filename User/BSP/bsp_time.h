#ifndef BSP_TIME_H
#define BSP_TIME_H

#include "stdint.h"
#include "main.h"

void Bsp_TIM_INIT(void);   /* 初始化 TIM4 */

uint32_t Bsp_GetMs(void);  /* 得到时间(ms) */


#endif