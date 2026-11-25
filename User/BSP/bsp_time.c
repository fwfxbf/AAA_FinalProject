#include "bsp_time.h"
#include "tim.h"

volatile uint32_t system_ms = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    system_ms++;
  }
}

void Bsp_TIM_INIT(void)
{
  HAL_TIM_Base_Start_IT(&htim4);
}

uint32_t Bsp_GetMs(void)
{
  return system_ms;
}