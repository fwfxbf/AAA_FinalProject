#ifndef __USBH_XBOX_H
#define __USBH_XBOX_H

#include "stdint.h"
#include "usbh_core.h"

/* Xbox 主接口信息 */
#define XBOX_INTERFACE_CLASS       0xFFU
#define XBOX_INTERFACE_SUBCLASS    0x5DU
#define XBOX_INTERFACE_PROTOCOL    0x01U

/* Xbox 主接口端点 */
#define XBOX_IN_EP                 0x81U
#define XBOX_OUT_EP                0x02U

/* 实际观察到的报告长度 */
#define XBOX_REPORT_LENGTH         20U

/* USB 最大接收长度 */
#define XBOX_BUFFER_SIZE           32U

typedef struct
{
  uint8_t InPipe;
  uint8_t OutPipe;

  uint8_t InEp;
  uint8_t OutEp;

  uint16_t InEpSize;
  uint16_t OutEpSize;

  uint8_t Interface;

  uint8_t RxBuffer[XBOX_BUFFER_SIZE];

  uint8_t RxState;

  uint8_t Connected;

} USBH_XBOX_HandleTypeDef;


/* 手柄按键状态 */
typedef struct
{
  uint8_t A;
  uint8_t B;
  uint8_t X;
  uint8_t Y;

  uint8_t LB;
  uint8_t RB;

  uint8_t Back;
  uint8_t Start;

  uint8_t Guide;

  uint8_t Up;
  uint8_t Down;
  uint8_t Left;
  uint8_t Right;

  uint8_t LT;
  uint8_t RT;

  int16_t LX;
  int16_t LY;

  int16_t RX;
  int16_t RY;

} Xbox_Controller_TypeDef;


extern Xbox_Controller_TypeDef Xbox_Controller;

/* Xbox 驱动回调函数 */
USBH_StatusTypeDef USBH_XBOX_InterfaceInit(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_XBOX_InterfaceDeInit(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef USBH_XBOX_Requests(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_XBOX_BgndProcess(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_XBOX_SOFProcess(USBH_HandleTypeDef *phost);


extern USBH_ClassTypeDef USBH_XBOX_CLASS;

#endif