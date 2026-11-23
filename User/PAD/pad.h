#ifndef PAD_H
#define PAD_H

#include "stdint.h"
#include "stdbool.h"

#define PAD_DPAD_UP         (1UL << 0)
#define PAD_DPAD_DOWN       (1UL << 1)
#define PAD_DPAD_LEFT       (1UL << 2)
#define PAD_DPAD_RIGHT      (1UL << 3)
#define PAD_START           (1UL << 4)
#define PAD_BACK            (1UL << 5)
#define PAD_L3              (1UL << 6)   // 左摇杆按下
#define PAD_R3              (1UL << 7)   // 右摇杆按下
#define PAD_LB              (1UL << 8)   // 左肩键
#define PAD_RB              (1UL << 9)   // 右肩键
#define PAD_A               (1UL << 12)
#define PAD_B               (1UL << 13)
#define PAD_X               (1UL << 14)
#define PAD_Y               (1UL << 15)  /* 左扳机 LT 和右扳机 RT 不在此定义 */

typedef struct {
  int16_t lx, ly, rx, ry;            /* 左右摇杆 -32768 ~ 32768 */
  int16_t lt, rt;                    /* 左右扳机 0 ~ 255 */
  uint32_t buttons;
  uint32_t last_ms;
} Padstate;                          /* 这个结构体存储手柄按键状态 */

const Padstate *Pad_Get(void);       /* 此函数返回一个指向Padstate结构体的指针 */
bool            Pad_IsAlive(void);   /* 300ms 无数据 = 掉线 */

void Xbox_UpdateFromReport(const uint8_t *raw, uint32_t len);

#endif