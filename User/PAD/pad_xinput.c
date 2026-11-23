#include "pad.h"
#include "usbh_xbox.h"
#include "string.h"

static Padstate s_pad;

void Xbox_UpdateFromReport(const uint8_t *raw, uint32_t len)
{
  if ((raw == NULL) || (len < 14U))
  {
    return;
  }
  if ((raw[0] != 0x00U) || (raw[1] != 0x14U))
  {
    return;                      /* 不是输入报文，丢掉 */
  }

  /* 按键：小端 16 位，位定义和 pad.h 完全一致，直接赋值 */
  s_pad.buttons = (uint32_t)raw[2] | ((uint32_t)raw[3] << 8);

  s_pad.lt = raw[4];
  s_pad.rt = raw[5];

  s_pad.lx = (int16_t)((uint16_t)raw[6]  | ((uint16_t)raw[7]  << 8));
  s_pad.ly = (int16_t)((uint16_t)raw[8]  | ((uint16_t)raw[9]  << 8));
  s_pad.rx = (int16_t)((uint16_t)raw[10] | ((uint16_t)raw[11] << 8));
  s_pad.ry = (int16_t)((uint16_t)raw[12] | ((uint16_t)raw[13] << 8));

  s_pad.last_ms = HAL_GetTick();   /* 心跳：活着的证据 */
}

const Padstate *Pad_Get(void)  { return &s_pad; }
bool Pad_IsAlive(void)        { return ((HAL_GetTick() - s_pad.last_ms) < 300U); }