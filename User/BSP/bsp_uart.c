#include "bsp_uart.h"
#include "usart.h"        /* huart1 */
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

#define BSP_LOG_MAX  128U

void Bsp_Send(const uint8_t *buf, uint16_t len)
{
  if ((buf == NULL) || (len == 0U))
  {
    return;
  }
  (void)HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 30U);
}

void Bsp_Printf(const char *fmt, ...)
{
  char    buf[BSP_LOG_MAX];
  va_list ap;
  int     n;

  va_start(ap, fmt);
  n = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  if (n <= 0)
  {
    return;
  }
  if ((uint32_t)n >= sizeof(buf))
  {
    n = (int)(sizeof(buf) - 1U);      /* 超长自动截断，不会越界 */
  }

  Bsp_Send((const uint8_t *)buf, (uint16_t)n);
}