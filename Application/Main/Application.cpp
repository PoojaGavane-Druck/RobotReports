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
* @author   Harvinder Bhuhi
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
//#include "UiTask.h"
//#include "DuciTask.h"
//#include "DInstrument.h"
//#include "DKeyHandler.h"
//#include "DUserInterface.h"
//#include "DComms.h"
#include "DPV624.h"
#include "uart.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

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
    OSTaskCreate(&startupTaskTCB,
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
    //create instrument 
  /*
      USART_ConfigParams configParams;
    configParams.baudRate = BAUDRATE_115200;
    configParams.dataLength = DATA_LENGTH_8BITS;
    configParams.direction = DIRECTION_TX_RX;
    configParams.flowControlMode = FLOW_CONTROL_NONE;
    configParams.numOfStopBits = STOPBITS_1;
    configParams.overSamplingType = OVER_SAMPLE_BY_16;
    //configParams.parityType = PARITY_ODD;
    
    // Changed because ODD parity is not owrking in UART3 - MAKARAND - TODO
    configParams.parityType = PARITY_NONE;
    configParams.portNumber = UART_PORT2;
    
    uartInit(configParams);
    */
  
    PV624 = new DPV624();
    
    // Create OWI for comms with DPI620G

                            
    
    while(DEF_TRUE)                          /* Task body, always written as an infinite loop. */
    {
      
        sleep(900u);        
    }
}


