#ifndef PAD_H
#define PAD_H

#include "stdint.h"

typedef struct {
  int16_t lx, ly, rx, ry, lt, rt;    /* -100 ~ +100 */
  uint32_t buttons;
  uint32_t last_ms;
} Padstate;                          /* 这个结构体存储手柄按键状态 */

const Padstate *Pad_Get(void);       /* 此函数返回一个指向Padstate结构体的指针 */
bool            Pad_IsAlive(void);   /* 300ms 无数据 = 掉线 */


#endif