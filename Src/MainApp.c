/*!
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the
* property of Baker Hughes and its suppliers, and affiliates if any.
* The intellectual and technical concepts contained herein are
* proprietary to Baker Hughes and its suppliers and affiliates
* and may be covered by U.S. and Foreign Patents, patents in
* process, and are protected by trade secret or copyright law.
* Dissemination of this information or reproduction of this
* material is strictly forbidden unless prior written permission
* is obtained from Baker Hughes.
*
* @file     MainApp.c
*
* @version  As per Git history
*
* @author   Andy Hutchinson
* @date     25/03/2020
*
* Creates StartupTask which creates other nested Micrium tasks and starts them
*/

#include "misra.h"
MISRAC_DISABLE
#include <cpu.h>
#include <lib_mem.h>
#include <lib_ascii.h>
#include <os.h>
#include <stm32l4xx_hal.h>
#include "main.h"
#include "bsp_os.h"
#include "app_cfg.h"
#include "os_app_hooks.h"
MISRAC_ENABLE

#include "MainApp.h"
#include "Application.h"

extern LPTIM_HandleTypeDef hlptim1;
volatile uint32_t LowPowerMode = 0; // Active mode
static uint32_t PowerSaveMode = 0u; /* Desired mode */

void MainApp(void)
{
  char chValue = 0x65;
   if (ASCII_IsDig(chValue) == DEF_TRUE)
   {
   }
    OS_ERR  os_error = OS_ERR_NONE;

    BSP_OS_TickInit(&hlptim1);                                          /* Initialize kernel tick timer                         */
    BSP_OS_TickEnable();                     /* Enable the tick timer and interrupt */

    Mem_Init();                                                 /* Initialize Memory Managment Module                   */
    CPU_IntDis();                                               /* Disable all Interrupts                               */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    OSInit(&os_error);                                            /* Initialize uC/OS-III                                 */

    if(os_error != OS_ERR_NONE)
    {
        Error_Handler();
    }

    App_OS_SetAllHooks();                                       /* Set all applications hooks                           */

    createAppTask(&os_error);             /* start the DPI610E application */

    if(os_error != OS_ERR_NONE)
    {
        Error_Handler();
    }

    OSStart(&os_error);                   /* Start multitasking (i.e. give control to uC/OS-III)  */

    if(os_error != OS_ERR_NONE)
    {
        Error_Handler();
    }
   
}

/**
  * @brief  This is called by OS when Idle.
  *
  * MCU can be shutdown during RTOS idle.
  * Possible modes depend on clock and resources to wake up etc.
  * Otherwise CPU etc can be stopped. Wake up may be ISR or OS time event.
  * LPTIMER running on LSI keeps all  OS timing fully operational.
  * @retval None
  * @note - Uses global Lowpower to save mode.
  */
void EnterLowPowerMode(void)
{

    //Apparently WFE is better as it will not sleep if interrupt is pending.
    uint32_t mode = PowerSaveMode;

    switch(mode)
    {
    case 0:
        return;
        break;

    case 1:
        HAL_SuspendTick();                  /* stop systick waking up O/S */
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFE);
        //HAL_ResumeTick();                  /* resume systick waking up O/S */
        break;

    case 2:
        HAL_SuspendTick();                  /* stop systick waking up O/S */
        //HAL_UARTEx_EnableStopMode(&huart2);
        __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
        HAL_PWREx_EnterSTOP0Mode(PWR_STOPENTRY_WFE);
        break;

    case 3:
        /* not implemented - needs slow CPU clock for LPR
        HAL_SuspendTick();
        HAL_UARTEx_EnableStopMode(&huart2);
        __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
        HAL_PWREx_EnterSTOP0Mode(PWR_STOPENTRY_WFE);
        */
        break;

    case 4:
        HAL_SuspendTick();                  /* stop systick waking up O/S */
        __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
        HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFE);
        break;

    default:
        mode = 0;
        return;
    }

    LowPowerMode = mode;
}

/**
  * @brief  This is called by OS when exit Idle.
  *
  * MCU is restored
  * @retval None
  * @note - Uses global LowPowerMode to select mode. It will be set=0 if value
  * is not supported.
  */
void ExitLowPowerMode(void)
{
    // Note this could nest on ISR. But code is safe
    // Don't do anything that is not ISR safe!
    switch(LowPowerMode)
    {
    case 0:
        break;

    case 1:
        HAL_ResumeTick();                  /* resume systick waking up O/S */
        break;

    case 2:
    case 3:
    case 4:
        __HAL_RCC_HSI_ENABLE();
        HAL_ResumeTick();                  /* resume systick waking up O/S */

        /* Wait until HSI is ready */
        while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == RESET)
        {
        };

        break;

    default:
        LowPowerMode = 0;
        break;
    }

    LowPowerMode = 0;
}