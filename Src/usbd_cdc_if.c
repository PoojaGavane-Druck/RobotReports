/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#define __STDC_WANT_LIB_EXT1__ 1
/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include "rtos.h"
#include <assert.h>
#include <stdbool.h>
#include "usb_device.h"
#include <string.h>
#include <stdlib.h>
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define RX_SEMA usbSemRcv
extern OS_SEM usbSemRcv;

USBD_CDC_LineCodingTypeDef linecoding =
  {
    38400, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
  };
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
#define APP_CIRCULAR_BUFFER_SIZE  1792
#define APP_RX_BUFFER_SIZE  64
// These are not used
#define APP_TX_DATA_SIZE  1
#define APP_RX_DATA_SIZE  1
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
uint8_t *CircularRxBufferFS;
uint8_t *pUserTxBufferFS;
uint8_t *pUserRxBufferFS;
uint16_t outRxCount;
volatile uint16_t inRxCount;
uint16_t outIndex;
uint16_t inIndex;
uint8_t  *saveBuf;
uint32_t saveLen;

static uint8_t appBuffer[APP_CIRCULAR_BUFFER_SIZE] = { 0 };
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS,
  CDC_TransmitCplt_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
    /* Set Application Buffers */
    inIndex = 0;
    outIndex = 0;
    inRxCount = 0;
    outRxCount = 0;
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t *) pUserTxBufferFS, 0);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, (uint8_t *) pUserRxBufferFS);

    return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
    return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
    switch(cmd)
    {
        case CDC_SEND_ENCAPSULATED_COMMAND:

            break;

        case CDC_GET_ENCAPSULATED_RESPONSE:

            break;

        case CDC_SET_COMM_FEATURE:

            break;

        case CDC_GET_COMM_FEATURE:

            break;

        case CDC_CLEAR_COMM_FEATURE:

            break;

        /*******************************************************************************/
        /* Line Coding Structure                                                       */
        /*-----------------------------------------------------------------------------*/
        /* Offset | Field       | Size | Value  | Description                          */
        /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
        /* 4      | bCharFormat |   1  | Number | Stop bits                            */
        /*                                        0 - 1 Stop bit                       */
        /*                                        1 - 1.5 Stop bits                    */
        /*                                        2 - 2 Stop bits                      */
        /* 5      | bParityType |  1   | Number | Parity                               */
        /*                                        0 - None                             */
        /*                                        1 - Odd                              */
        /*                                        2 - Even                             */
        /*                                        3 - Mark                             */
        /*                                        4 - Space                            */
        /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
        /*******************************************************************************/
        case CDC_SET_LINE_CODING:
            // SET line encoding for GET
            linecoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) | (pbuf[2] << 16) | (pbuf[3] << 24));
            linecoding.format     = pbuf[4];
            linecoding.paritytype = pbuf[5];
            linecoding.datatype   = pbuf[6];
            break;

        case CDC_GET_LINE_CODING:
            // GET line encoding from SET
            pbuf[0] = (uint8_t)(linecoding.bitrate);
            pbuf[1] = (uint8_t)(linecoding.bitrate >> 8);
            pbuf[2] = (uint8_t)(linecoding.bitrate >> 16);
            pbuf[3] = (uint8_t)(linecoding.bitrate >> 24);
            pbuf[4] = linecoding.format;
            pbuf[5] = linecoding.paritytype;
            pbuf[6] = linecoding.datatype;
            break;

        case CDC_SET_CONTROL_LINE_STATE:

            break;

        case CDC_SEND_BREAK:

            break;

        default:
            break;
    }

    return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
    int8_t result = USBD_OK;

    uint32_t length = *Len;
    uint32_t space = APP_CIRCULAR_BUFFER_SIZE  - (inRxCount - outRxCount );
       
    if((NULL != Buf) && (NULL != Len))
    {
      /* Is there enough space in circular buffer? */
      if (space  < length )
      {
          /* No. Force application to take recovery action. */
          CDC_Init_FS();
          memset_s(CircularRxBufferFS, APP_CIRCULAR_BUFFER_SIZE, '\0', APP_CIRCULAR_BUFFER_SIZE);
          length = 13u;
          memcpy_s(Buf, *Len, "messageTooBig", length + 1u);
      }

      saveBuf = NULL;
      saveLen = 0;

      uint16_t inidx = inIndex;
      /* Optimized for 1 character stream. */
      if (length == 1)
      {
          *(CircularRxBufferFS + inidx) = *Buf;
          inidx++;
      }
      /* else Will copy reach end of circular buffer? */
      else if ((inidx + length) > APP_CIRCULAR_BUFFER_SIZE)
      {
          /* Yes. Move to circular buffer in two pieces. */
          uint32_t length1 = APP_CIRCULAR_BUFFER_SIZE - inidx;
          memcpy_s(CircularRxBufferFS + inidx, length1, Buf, length1);
          inidx = length - length1;
          if (inidx)
          {
              memcpy_s(CircularRxBufferFS, APP_CIRCULAR_BUFFER_SIZE, Buf + length1, inidx);
          }
      }
      else
      {
          memcpy_s(CircularRxBufferFS + inidx,  (APP_CIRCULAR_BUFFER_SIZE - inidx), Buf, length);
          inidx += length;
      }
      if (inidx >= APP_CIRCULAR_BUFFER_SIZE)
      {
          inidx = 0;
      }
      inIndex = inidx;
      inRxCount += length;
      if (saveBuf == NULL)
      {
          /* Rx buffer is free - so continue. */
          USBD_CDC_SetRxBuffer(&hUsbDeviceFS, (uint8_t *) pUserRxBufferFS);
          result = USBD_CDC_ReceivePacket(&hUsbDeviceFS);
      }

      /* Set USB receive semaphore when terminating character is read */
      if( (CircularRxBufferFS[inIndex - 1u] == '\n') || (5u == *Len))
      {
          OS_ERR os_error = OS_ERR_NONE;
          RTOSSemPost(&RX_SEMA, OS_OPT_POST_1, &os_error);
      }
    }
      return (result);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
        USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
        if (hcdc->TxState != 0)
        {
            return USBD_BUSY;
        }
        USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
        result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/**
  * @brief  CDC_TransmitCplt_FS
  *         Data transmited callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
    UNUSED(Buf);
    UNUSED(Len);
    UNUSED(epnum);
  /* USER CODE END 13 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @brief  VCP_getch
  *         Return next character in circular buffer memory.
  *         @note
  *         Will wait (block) until character present.
  *
  *
  * @param  none
  * @retval character from buffer.
*/
uint8_t VCP_getch(void)
{
    uint8_t ch = 0;
    uint32_t oRx = outRxCount;
    uint32_t idx = outIndex;
    /* thread safe character count check but a sad  spin loop*/
    while  ( inRxCount == oRx);

    ch = CircularRxBufferFS[idx++];
    outRxCount = oRx + 1;
    if (idx >= APP_CIRCULAR_BUFFER_SIZE)
    {
        idx = 0;
    }
    outIndex = idx;
    /* Call callback so the Rx will restart as needed */
    if (saveBuf != NULL)
    {
        CDC_Receive_FS(saveBuf, &saveLen);
    }
    return ch;
}

/**
  * @brief  VCP_read
  *         Return receive buffer from head of circular buffer memory (flattens it) and clear USB receive semaphore.
  * @param  none
  * @retval buffer.
*/
uint8_t* VCP_read(void)
{
    if (MX_USB_DEVICE_GetUsbMode() == (int)E_USBMODE_CDC)
    {
        memset_s(appBuffer, APP_CIRCULAR_BUFFER_SIZE, '\0', APP_CIRCULAR_BUFFER_SIZE);
        memcpy_s(appBuffer, APP_CIRCULAR_BUFFER_SIZE - outIndex, CircularRxBufferFS + outIndex, APP_CIRCULAR_BUFFER_SIZE - outIndex);
        memcpy_s(appBuffer + outIndex, outIndex, CircularRxBufferFS, outIndex);
        
        /* Clear USB receive semaphore */
        OS_ERR os_error = OS_ERR_NONE;
        RTOSSemSet(&RX_SEMA, (OS_SEM_CTR)0, &os_error);

        /* Call callback so the Rx will restart as needed */
        if (saveBuf != NULL)
        {
            CDC_Receive_FS(saveBuf, &saveLen);
        }

        return appBuffer;
    }
    else
    {
        return NULL;
    }
}

/**
  * @brief  VCP_clear
  *         Clear the receive buffer and the receive semaphore
  * @param  none
  * @retval buffer.
*/
void VCP_clear(void)
{
    if (MX_USB_DEVICE_GetUsbMode() == (int)E_USBMODE_CDC)
    {
        memset_s(CircularRxBufferFS, APP_CIRCULAR_BUFFER_SIZE, '\0', APP_CIRCULAR_BUFFER_SIZE);
        outIndex = outRxCount = inIndex = inRxCount = 0;

        /* Clear USB receive semaphore */
        OS_ERR os_error = OS_ERR_NONE;
        RTOSSemSet(&RX_SEMA, (OS_SEM_CTR)0, &os_error);

    }
}

/**
  * @brief  VCP_kbhit
  *         Check if character in circular buffer memory.
  *         @note
  *
  *
  * @param  none
  * @retval Count of characters waiting in buffer.
*/
uint32_t VCP_kbhit(void)
{
    if (MX_USB_DEVICE_GetUsbMode() == (int)E_USBMODE_CDC)
    {
        return  (inRxCount - outRxCount);
    }
    else
    {
        return 0;
    }
}

/**
  * @brief  CDC_alloc_FS
  *         Allocate memory for buffers.
  *         @note
  *         Cant do during USB init they are call backs and alloc is not reentrant
  *
  *
  * @param  id: == DEVICE_FS for full speed buffer allocation else high speed
  * @retval none
*/
void CDC_alloc_FS(uint8_t id)
{
    if(NULL != pUserRxBufferFS)
    {
      free(pUserRxBufferFS);
      pUserRxBufferFS =  NULL;
    }
    if (pUserRxBufferFS == NULL)
    {
        pUserRxBufferFS = (uint8_t*)malloc( (id == DEVICE_FS) ? CDC_DATA_FS_MAX_PACKET_SIZE : CDC_DATA_HS_MAX_PACKET_SIZE);
        assert_param(pUserRxBufferFS);
    }
    
    if(NULL != pUserTxBufferFS)
    {
      free(pUserTxBufferFS);
      pUserTxBufferFS =  NULL;
    }
    if (pUserTxBufferFS == NULL)
    {
        pUserTxBufferFS = (uint8_t*)malloc( APP_TX_DATA_SIZE);
        assert_param(pUserTxBufferFS);
    }

    if(NULL != CircularRxBufferFS)
    {
      free(CircularRxBufferFS);
      CircularRxBufferFS =  NULL;
    }
    if (CircularRxBufferFS == NULL)
    {
        CircularRxBufferFS = (uint8_t*)malloc(APP_CIRCULAR_BUFFER_SIZE);
        assert_param(CircularRxBufferFS);
    }
}
/**
  * @brief  CDC_free_FS
  *         DeAllocate memory for buffers.
  *         @note
  *         Cant do during USB init they are call backs and alloc is not reentrant
  *
  *
  * @param  none
  * @retval none
*/
void CDC_free_FS(void)
{
    if(NULL != pUserRxBufferFS)
    {
      free(pUserRxBufferFS);
      pUserRxBufferFS =  NULL;
    }
    
    if(NULL != pUserTxBufferFS)
    {
      free(pUserTxBufferFS);
      pUserTxBufferFS =  NULL;
    }
    
    if(NULL != CircularRxBufferFS)
    {
      free(CircularRxBufferFS);
      CircularRxBufferFS =  NULL;
    }

}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
