/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v2.0_Cube
  * @brief          : This file implements the USB Device
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

/* Includes ------------------------------------------------------------------*/

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

/* USER CODE BEGIN Includes */
#include <os.h>
#include <assert.h>
#include "usbd_msc.h"
#include "usbd_storage_if.h"
#include "rtos.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define RX_SEMA usbSemRcv

OS_SEM usbSemRcv;

eUsbMode_t eUsbMode = E_USBMODE_CDC;
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_DescriptorsTypeDef FS_Desc;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */
/**
  * Stop USB device and DeInit the library
  * @retval None
  */
void MX_USB_DEVICE_DeInit(void)
{
    USBD_DeInit(&hUsbDeviceFS);
}

void MX_USB_DEVICE_SetUsbMode(eUsbMode_t mode)
{
    if (eUsbMode != mode)
    {
        eUsbMode = mode;
        MX_USB_DEVICE_DeInit();
        MX_USB_DEVICE_Init();

#if 0  /* a) Configure USB_PEN_PC9 as an output and set HIGH (to turn on LDO IC19 / VDD2) */
        HAL_GPIO_WritePin(USB_PEN_PC9_GPIO_Port, USB_PEN_PC9_Pin, GPIO_PIN_SET);
#endif
        /* b) Wait for 130ms (to allow LDO to turn on) */
        HAL_Delay(130u);
    }
}

eUsbMode_t MX_USB_DEVICE_GetUsbMode()
{
    return eUsbMode;
}
/* USER CODE END 1 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
    /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */
    if (MX_USB_DEVICE_GetUsbMode() == (int)E_USBMODE_CDC)
    {
        CDC_alloc_FS(DEVICE_FS);
    }

    OS_ERR os_error = OS_ERR_NONE;
    memset((void*)&RX_SEMA, 0, sizeof(OS_SEM));
    RTOSSemCreate(&RX_SEMA,"UsbRCV",  (OS_SEM_CTR)0,  &os_error);


    /* USER CODE END USB_DEVICE_Init_PreTreatment */

    /* Init Device Library, add supported class and start the library. */
    if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
    {
        Error_Handler();
    }

    if (MX_USB_DEVICE_GetUsbMode() == (int)E_USBMODE_CDC)
    {
        if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
        {
            Error_Handler();
        }
        if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
        {
            Error_Handler();
        }
    }
    else
    {
        if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MSC) != USBD_OK)
        {
            Error_Handler();
        }
        if (USBD_MSC_RegisterStorage(&hUsbDeviceFS, &USBD_Storage_Interface_fops_FS) != USBD_OK)
        {
            Error_Handler();
        }
    }


    if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */

    /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
