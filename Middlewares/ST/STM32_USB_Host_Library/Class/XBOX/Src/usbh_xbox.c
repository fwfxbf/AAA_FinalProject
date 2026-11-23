#include "usbh_xbox.h"
#include "string.h"
#include "pad.h"

Xbox_Controller_TypeDef Xbox_Controller;

#define XBOX_RX_IDLE       0U
#define XBOX_RX_WAIT       1U
#define XBOX_RX_DONE       2U
#define XBOX_RX_ERROR      3U

static uint8_t USBH_XBOX_FindInterface(USBH_HandleTypeDef *phost)
{
  uint8_t i;

  for (i = 0U; i < phost->device.CfgDesc.bNumInterfaces; i++)
  {
    if ((phost->device.CfgDesc.Itf_Desc[i].bInterfaceClass == XBOX_INTERFACE_CLASS) && 
        (phost->device.CfgDesc.Itf_Desc[i].bInterfaceSubClass == XBOX_INTERFACE_SUBCLASS) &&     
        (phost->device.CfgDesc.Itf_Desc[i].bInterfaceProtocol == XBOX_INTERFACE_PROTOCOL))
    {
      return i;
    }
  }

  return 0XFFU;
}


static USBH_StatusTypeDef USBH_XBOX_FindEndpoints(USBH_HandleTypeDef *phost, USBH_XBOX_HandleTypeDef *xbox)
{
  uint8_t i;
  uint8_t interface;

  USBH_EpDescTypeDef *ep;

  interface = xbox->Interface;

  xbox->InEp = 0U;
  xbox->OutEp = 0U;

  for (i = 0U; i < phost->device.CfgDesc.Itf_Desc[interface].bNumEndpoints; i++)
  {
    ep = &phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[i];

    if ((ep->bmAttributes & 0x03U) == 0X03U)
    {
      if ((ep->bEndpointAddress & 0x80U) != 0U)
      {
        xbox->InEp = ep->bEndpointAddress;
        xbox->InEpSize = ep->wMaxPacketSize;
      }
      else
      {
        xbox->OutEp = ep->bEndpointAddress;
        xbox->OutEpSize = ep->wMaxPacketSize;
      }
    }
  }

  if ((xbox->InEp == 0U) || (xbox->OutEp == 0U))
  {
    return USBH_FAIL;
  }

  return USBH_OK;
}


USBH_StatusTypeDef USBH_XBOX_InterfaceInit(USBH_HandleTypeDef *phost)
{
  USBH_XBOX_HandleTypeDef *xbox;

  uint8_t interface;

  interface = USBH_XBOX_FindInterface(phost);

  if (interface == 0xFFU)
  {
    return USBH_FAIL;
  }

  if (USBH_SelectInterface(phost, interface) != USBH_OK)
  {
    return USBH_FAIL;
  }

  /* 创建 Xbox 句柄 */

  xbox = (USBH_XBOX_HandleTypeDef *)USBH_malloc(sizeof(USBH_XBOX_HandleTypeDef));

  if (xbox == NULL)
  {
    return USBH_FAIL;
  }

  memset(xbox, 0, sizeof(USBH_XBOX_HandleTypeDef));

  xbox->InPipe = 0xFFU;
  xbox->OutPipe = 0xFFU;

  /* 保存 Xbox 句柄 */

  phost->pActiveClass->pData = xbox;
  xbox->Interface = interface;

  if (USBH_XBOX_FindEndpoints(phost, xbox) != USBH_OK)
  {
    USBH_free(xbox);
    phost->pActiveClass->pData = NULL;

    return USBH_FAIL;
  }


  /* 分配 IN Pipe */
  xbox->InPipe = USBH_AllocPipe(phost, xbox->InEp);

  if (xbox->InPipe == 0xFFU)
  {
    goto Error;
  }

  /* 打开 IN Pipe */
  if (USBH_OpenPipe(
          phost,
          xbox->InPipe,
          xbox->InEp,
          phost->device.address,
          phost->device.speed,
          USB_EP_TYPE_INTR,
          xbox->InEpSize
      ) != USBH_OK)
  {
    goto Error;
  }

  /* 清零 IN DATA Toggle */
  USBH_LL_SetToggle(phost, xbox->InPipe, 0U);

  /* 分配 OUT Pipe */
  xbox->OutPipe = USBH_AllocPipe(phost, xbox->OutEp);

  if (xbox->OutPipe == 0xFFU)
  {
    goto Error;
  }

  /* 打开 OUT Pipe */
  if (USBH_OpenPipe(
          phost,
          xbox->OutPipe,
          xbox->OutEp,
          phost->device.address,
          phost->device.speed,
          USB_EP_TYPE_INTR,
          xbox->OutEpSize
      ) != USBH_OK)
  {
    goto Error;
  }

  /* 清零 OUT DATA Toggle */
  USBH_LL_SetToggle(phost, xbox->OutPipe, 0U);

  return USBH_OK;


Error:

  /* 发生错误时释放已经分配的资源 */

  if (xbox->InPipe != 0xFFU)
  {
    USBH_FreePipe(phost, xbox->InPipe);
  }

  if (xbox->OutPipe != 0xFFU)
  {
    USBH_FreePipe(phost, xbox->OutPipe);
  }

  USBH_free(xbox);

  phost->pActiveClass->pData = NULL;

  return USBH_FAIL;
}


USBH_StatusTypeDef USBH_XBOX_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
  USBH_XBOX_HandleTypeDef *xbox;

  /* 获取 Xbox 驱动的数据 */
  xbox = (USBH_XBOX_HandleTypeDef *)phost->pActiveClass->pData;

  /* 如果 Xbox 数据不存在，直接返回 */
  if (xbox == NULL)
  {
      return USBH_OK;
  }

  /* 释放 IN Pipe */
  if (xbox->InPipe != 0xFFU)
  {
    USBH_ClosePipe(phost, xbox->InPipe);
    USBH_FreePipe(phost, xbox->InPipe);
    xbox->InPipe = 0xFFU;
  }

  /* 释放 OUT Pipe */
  if (xbox->OutPipe != 0xFFU)
  {
    USBH_ClosePipe(phost, xbox->OutPipe);
    USBH_FreePipe(phost, xbox->OutPipe);
    xbox->OutPipe = 0xFFU;
  }

  /* 释放 Xbox 数据结构 */
  USBH_free(xbox);

  /* 清空类驱动数据指针 */
  phost->pActiveClass->pData = NULL;

  return USBH_OK;
}



USBH_StatusTypeDef USBH_XBOX_Requests(USBH_HandleTypeDef *phost)
{
  USBH_XBOX_HandleTypeDef *xbox = (USBH_XBOX_HandleTypeDef *)phost->pActiveClass->pData;

  if (xbox == NULL)
  {
    return USBH_FAIL;
  }

  // if (s_led_sent == 0U) {
  //   /* 抓包实证：PC 初始化后会发 3 字节 01 03 02 点亮 2 号灯 */
  //   static uint8_t led[3] = { 0x01U, 0x03U, 0x02U };

  //   if (xbox->OutPipe != 0xFFU)
  //   {
  //     (void)USBH_InterruptSendData(phost, led, 3U, xbox->OutPipe);
  //   }
  //   s_led_sent = 1U;
  //   return USBH_BUSY;      /* 下一轮再继续 */
  // }

  /* 通知应用层：类驱动就绪。少了这一句，Appli_state 永远是 APPLICATION_START */
  phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
  return USBH_OK;
}


USBH_StatusTypeDef USBH_XBOX_BgndProcess(USBH_HandleTypeDef *phost)
{
  USBH_XBOX_HandleTypeDef *xbox =
      (USBH_XBOX_HandleTypeDef *)phost->pActiveClass->pData;
  USBH_URBStateTypeDef urb;

  if (xbox == NULL) {
    return USBH_FAIL;
  }

  urb = USBH_LL_GetURBState(phost, xbox->InPipe);

  switch (urb) {

    case USBH_URB_DONE: {
      uint32_t n = USBH_LL_GetLastXferSize(phost, xbox->InPipe);
      Xbox_UpdateFromReport(xbox->RxBuffer, n);   /* 解析 → Padstate */
      USBH_InterruptReceiveData(phost, xbox->RxBuffer,
                                (uint8_t)xbox->InEpSize, xbox->InPipe);
      break;
    }

    case USBH_URB_STALL:
      (void)USBH_ClrFeature(phost, xbox->InEp);
      USBH_InterruptReceiveData(phost, xbox->RxBuffer,
                                (uint8_t)xbox->InEpSize, xbox->InPipe);
      break;

    case USBH_URB_NOTREADY:   /* 设备暂时没数据（NAK） */
    case USBH_URB_ERROR:
    default: {
      static uint32_t t = 0U;
      if ((phost->Timer - t) >= 1U) {
        t = phost->Timer;
        USBH_InterruptReceiveData(phost, xbox->RxBuffer,
                                  (uint8_t)xbox->InEpSize, xbox->InPipe);
      }
      break;
    }
  }

  return USBH_OK;
}


/* Xbox USB 帧处理：临时空操作 */
USBH_StatusTypeDef USBH_XBOX_SOFProcess(USBH_HandleTypeDef *phost)
{
  return USBH_OK;
}


USBH_ClassTypeDef USBH_XBOX_CLASS =
{
  .Name         = "Xbox Controller",
  .ClassCode    = 0xFFU,

  .Init         = USBH_XBOX_InterfaceInit,
  .DeInit       = USBH_XBOX_InterfaceDeInit,

  .Requests     = USBH_XBOX_Requests,
  .BgndProcess  = USBH_XBOX_BgndProcess,
  .SOFProcess   = USBH_XBOX_SOFProcess,

  .pData        = NULL
};