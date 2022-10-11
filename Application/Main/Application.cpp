/**
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     Application.cpp
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     21 January 2020
*
* @brief    The main application source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <cpu.h>
#include <lib_mem.h>
#include <os.h>
#include <stm32l4xx_hal.h>
#include "main.h"
#include "bsp_os.h"
#include "app_cfg.h"
#include "os_app_hooks.h"
MISRAC_ENABLE

#include "MainApp.h"
#include "Application.h"
#include "Utilities.h"

#include "DPV624.h"
#include "uart.h"

/* Error handler instance parameter starts from 3801 to 3900 */

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
extern IWDG_HandleTypeDef hiwdg;
static OS_TCB startupTaskTCB;
static CPU_STK startupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE];

/* Prototypes -------------------------------------------------------------------------------------------------------*/
static void startupTask(void *p_arg);


/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   The application start function that kicks off all other tasks
 * @param   void
 * @return  void
 */
void createAppTask(OS_ERR *os_error)
{
    /* Create the startup task */
    RTOSTaskCreate(&startupTaskTCB,
                   "startup Task",
                   startupTask,
                   0u,
                   APP_CFG_STARTUP_TASK_PRIO,
                   &startupTaskStk[0u],
                   APP_CFG_STARTUP_TASK_STK_SIZE / 10u,
                   APP_CFG_STARTUP_TASK_STK_SIZE,
                   (OS_MSG_QTY)1,
                   0u,
                   0u,
                   (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                   os_error);

    if(*os_error != static_cast<OS_ERR>(OS_ERR_NONE))
    {
        // Resort to global error handler
        Error_Handler();
    }
}

/**
* @brief    Startup task. Must initialize the ticker after multitasking has started.
* @param    p_arg is the argument passed to 'StartupTask()' by 'OSTaskCreate()'.
* @return   void
*/
static void startupTask(void *p_arg)
{

    // Create the instrument class
    PV624 = new DPV624();

    PV624->createApplicationObjects();

#ifdef FIT_SECOND_MICRO_COMM_FAILURE
    PV624->holdStepperMicroInReset();
#endif

    // Task body, endless loop
    while(DEF_TRUE)
    {
        bool healthy = PV624->IsAllTasksAreAlive();
        // Release builds only

        HAL_IWDG_Refresh(&hiwdg);

        if(healthy)
        {
            //HAL_IWDG_Refresh(&hiwdg);
        }

        else
        {
            // woof!
            // PV624->handleError(E_ERROR_CODE_IWDG, os_error, 0u);
        }

        sleep(1000u);
    }

}


