#include "app_main.h"

#include "app_mode.h"
#include "bsp_time.h"
#include "bsp_uart.h"
#include "chassis.h"
#include "track.h"
#include "arm.h"
#include "pad.h"


AppMode mode = MODE_IDLE;

void App_Init(void)
{
  Bsp_TIM_INIT();
  App_Mode_Init();
}

void App_Loop(void)
{
  App_Mode_Update();              /* 更新模式 */
  mode = App_Mode_Get();

  const Padstate *p = Pad_Get();

  switch (mode)                   /* 主逻辑 */
  {
    case MODE_IDLE:

      break;

    case MODE_TRACK:

      break;

    case MODE_REMOTE:

      break;
  }

}