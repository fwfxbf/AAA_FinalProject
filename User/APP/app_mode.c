#include "app_mode.h"

#include "bsp_uart.h"
#include "pad.h"
#include "chassis.h"

/* 当前模式 */
static AppMode current_mode = MODE_IDLE;

/* 初始化 */
void App_Mode_Init(void)
{
  current_mode = MODE_IDLE;
  Bsp_Printf("[APP] 初始化完毕\r\n");
  //Chassis_Stop();
}

/* 获取当前模式 */
AppMode App_Mode_Get(void)
{
  return current_mode;
}

/* 切换模式 */
static void App_Mode_Set(AppMode new_mode)
{
  if (current_mode == new_mode)
    return;

  /* 切换模式前先停止底盘 */
  //Chassis_Stop();

  current_mode = new_mode;
  
  switch (current_mode)
  {
    case MODE_IDLE:
      Bsp_Printf("[MODE] 停止\r\n");
      break;
    case MODE_TRACK:
      Bsp_Printf("[MODE] 循迹\r\n");
      break;
    case MODE_REMOTE:
      Bsp_Printf("[MODE] 遥控底盘\r\n");
      break;
    case MODE_ARM:
      Bsp_Printf("[MODE] 遥控机械臂\r\n");
  }
}

/* 模式机更新 */
void App_Mode_Update(void)
{
  const Padstate *pad = Pad_Get();

  /* 1. 手柄掉线保护 */
  if (!Pad_IsAlive())
  {
    if (current_mode == MODE_REMOTE)
    {
      //Chassis_Stop();
      App_Mode_Set(MODE_IDLE);
    }
    return;
  }

  /* 2. 判断模式切换按键 */
  if (pad->buttons & PAD_LS)
  {
    App_Mode_Set(MODE_IDLE);
  }
  else if (pad->buttons & PAD_XBOX)
  {
    App_Mode_Set(MODE_TRACK);
  }
  else if (pad->buttons & PAD_VIEW)
  {
    App_Mode_Set(MODE_REMOTE);
  }
  else if (pad->buttons & PAD_MENU)
  {
    App_Mode_Set(MODE_ARM);
  }
}