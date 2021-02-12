/**
* BHGE Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DKeyHandler.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The main application source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DKeyHandler.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DPV624.h"
#include "main.h"
#include "uart.h"
/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define KEY_HANDLER_TASK_STK_SIZE   512u    //not bytes (CPU_STK is 4 bytes, so multiply by 4 for stack size in bytes)
#define KEY_NEXT_KEY_WAIT_TIME_MS   100u

#define USE_OS 1
/* Variables --------------------------------------------------------------------------------------------------------*/
extern uint32_t extiIntFlag;
OS_SEM gpioIntSem;
static gpioButtons_t flag;
/**
* @brief	Constructor
* @param    osErr is pointer to OS error
* @return   void
*/
DKeyHandler::DKeyHandler(OS_ERR *osErr)
: DTask()
{
  
    myName = "key";

    //safe to 'new' a stack here as it is never 'free'd.
    CPU_STK_SIZE stackBytes = KEY_HANDLER_TASK_STK_SIZE * (CPU_STK_SIZE)sizeof(CPU_STK_SIZE);
    myTaskStack = (CPU_STK *)new char[stackBytes];
       // Register task for health monitoring
#ifdef TASK_MONITOR_IMPLEMENTED
    registerTask();
#endif
    triggered = false;
    timeoutCount = 0u;
    OSSemCreate(&gpioIntSem, "GpioSem", (OS_SEM_CTR)0, osErr); /* Create GPIO interrupt semaphore */

    bool ok = (*osErr == static_cast<OS_ERR>(OS_ERR_NONE)) || (*osErr == static_cast<OS_ERR>(OS_ERR_TIMEOUT));

    if(!ok)
    {
#ifdef ASSERT_IMPLEMENTED      
MISRAC_DISABLE
        assert(false);
MISRAC_ENABLE
#endif
        error_code_t errorCode;
        errorCode.bit.osError = SET;
        PV624->errorHandler->handleError(errorCode, *osErr);
    }
    activate(myName, (CPU_STK_SIZE)KEY_HANDLER_TASK_STK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)0u, osErr);
}

/**
* @brief	Destructor
* @param    void
* @return   void
*/
DKeyHandler::~DKeyHandler()
{
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as we are using snprintf which violates the rule.
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm128")

/**
* @brief	Key Task run - the top level functon that handles key presses.
* @param    void
* @return   void
*/
void DKeyHandler::runFunction(void)
{
    OS_ERR os_error = OS_ERR_NONE;

    //task main loop
    while(DEF_TRUE)
    {
        OSTimeDlyHMSM(0u, 0u, 0u, KEY_TASK_TIMEOUT_MS, OS_OPT_TIME_HMSM_STRICT, &os_error);
        //pend until timeout, blocking, on the task message - posted by GPIO ISR on key press or a remote key press (eg, over DUCI)
        OSSemPend(&gpioIntSem, KEY_TASK_TIMEOUT_MS, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_error);

        bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT));

        if(!ok)
        {
#ifdef ASSERT_IMPLEMENTED
MISRAC_DISABLE
            assert(false);
MISRAC_ENABLE
#endif
            error_code_t errorCode;
            errorCode.bit.osError = SET;
            PV624->errorHandler->handleError(errorCode, os_error);
        }
        else
        {
            processKey(os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT));
        }

#ifdef TASK_MONITOR_IMPLEMENTED
        keepAlive();
#endif
    }
}

/**
* @brief Sends validated key press
* @param keyCode
*/
void DKeyHandler::sendKey(gpioButtons_t keyCode)
{
    uint32_t keyPressed = (uint32_t)(keyCode.bytes & UI_KEY_MASK);
    uint32_t pressType;

    if (keyCode.bit.LongPress == 1u)
    {
        pressType = (uint32_t)E_PRESS_LONG;
           
    }
    else
    {
        pressType = (uint32_t)E_PRESS_SHORT;
       
    }

    //send key to user interface
    //PV624->userInterface->handleKey(keyPressed, pressType);
    OS_ERR os_error = OS_ERR_NONE;
    OSTimeDlyHMSM(0u, 0u, 0u, KEY_NEXT_KEY_WAIT_TIME_MS, OS_OPT_TIME_HMSM_STRICT, &os_error);
}

void DKeyHandler::processKey(bool timedOut)
{
    const uint32_t timeLimit = KEY_LONG_PRESS_TIME_MS / KEY_TASK_TIMEOUT_MS;

    if (!triggered)
    {
        if (!timedOut)
        {
             // key down (falling edge)

            // debounce
            OS_ERR os_error = OS_ERR_NONE;
            OSTimeDlyHMSM(0u, 0u, 0u, KEY_DEBOUNCE_TIME_MS, OS_OPT_TIME_HMSM_STRICT, &os_error);
            keys = getKey();
            if (keys.bytes != 0u)
            {
                timeoutCount = 0u;
                triggered = true;
            }
        }
    }
    else
    {
        if (timeoutCount < timeLimit)
        {
            if (!timedOut)
            {
                // key up (rising edge) before time limit
                timeoutCount = 0u;
                triggered = false;
                keys.bit.LongPress = false;

                sendKey(keys);
                
            }
            else
            {
                // key still pressed (no edge) before time limit
                timeoutCount++;
            }
        }
        else
        {
            // exceeded time limit
            timeoutCount = 0u;
            triggered = false;

            keys.bit.LongPress = true;
            sendKey(keys);

        }
    }
}
gpioButtons_t DKeyHandler::getKey(void)
{
    gpioButtons_t keyCodeMsg;
    keyCodeMsg.bytes = 0u;
#ifdef NUCLEO_BOARD
    keyCodeMsg.bit.powerOnOff    = HAL_GPIO_ReadPin(POWER_ON_OFF_BUTTON_GPIO_Port,
                                                POWER_ON_OFF_BUTTON_Pin);
#else
        keyCodeMsg.bit.powerOnOff    = HAL_GPIO_ReadPin(POWER_KEY_PF8_GPIO_Port,
                                                POWER_KEY_PF8_Pin);
#endif
#if 0
    keyCodeMsg.bit.blueTooth     = !HAL_GPIO_ReadPin(BT_KEY_PF9_GPIO_Port,
                                                BT_KEY_PF9_Pin);
#endif
   

    return keyCodeMsg;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

    OS_ERR osErr = OS_ERR_NONE;
#ifdef NUCLEO_BOARD
    if (GPIO_Pin == GPIO_PIN_13)
    {
        OSSemPost(&gpioIntSem, OS_OPT_POST_ALL, &osErr);
    }
#else
    if (GPIO_Pin == GPIO_PIN_8)
    {
        OSSemPost(&gpioIntSem, OS_OPT_POST_ALL, &osErr);
    }
#endif
}

/**
* @brief Gets current state of keys for test purposes
* @param void
*/
uint32_t DKeyHandler::getKeys(void)
{
    return static_cast<uint32_t>(getKey().bytes);
}

/**
* @brief Simulates key presses for test purposes
* @param void
*/

void DKeyHandler::setKeys(uint32_t keyCodes, uint32_t duration)
{
    gpioButtons_t key;
    key.bytes = static_cast<uint32_t>(1 << (keyCodes - 1));
    key.bit.LongPress = (duration == 1);

    uint32_t keyPressed = (uint32_t)(key.bytes & UI_KEY_MASK);
    uint32_t pressType = key.bit.LongPress ? (uint32_t)E_PRESS_LONG : (uint32_t)E_PRESS_SHORT;

}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm128")
