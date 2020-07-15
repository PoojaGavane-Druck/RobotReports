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

/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define KEY_HANDLER_TASK_STK_SIZE   256u    //not bytes (CPU_STK is 4 bytes, so multiply by 4 for stack size in bytes)

/* Variables --------------------------------------------------------------------------------------------------------*/

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

    activate(myName, (CPU_STK_SIZE)KEY_HANDLER_TASK_STK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)10u, osErr);
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
    OS_ERR os_error;
    gpioButtons_t keyCodeMsg[2];
    OS_TICK timeout = (OS_TICK)0u;

    keyCodeMsg[0].bytes = 0u;
    keyCodeMsg[1].bytes = 0u;

    //task main loop
    while (DEF_TRUE)
    {
#ifdef BUTTON_IRQ_ENABLED
        //pend forever, blocking, on the task message - posted by GPIO ISR on key press or a remote key press (eg, over DUCI)
        os_error = gpio_IRQWait(timeout, &keyCodeMsg[0]);

        //reset timeout to wait forever - will be modified below if needed
        timeout = (OS_TICK)0u;

        if (os_error == OS_ERR_TIMEOUT)
        {
            //timeout occured but one or more keys were pressed
            if ((keyCodeMsg[1].bytes & UI_KEY_MASK) != 0u)
            {
                keyCodeMsg[1].bit.LongPress = 1u;
                sendKey(keyCodeMsg[1]);
                keyCodeMsg[1].bytes = 0u;
            }
        }
        else if (keyCodeMsg[0].bytes != 0u)
        {
            //key still pressed
            keyCodeMsg[1] = keyCodeMsg[0];
            timeout = KEY_LONG_PRESS_TIME_MS;
        }
        else //keys released
        {
            if (keyCodeMsg[1].bytes != 0u)
            {
                //would get here on key release before the long press time elapsed
                sendKey(keyCodeMsg[1]);
                keyCodeMsg[1].bytes = 0u;
            }
        }

        gpioEnIrq();
#else
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT + OS_OPT_TIME_PERIODIC,
                      &os_error);
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
    PV624->userInterface->handleKey(keyPressed, pressType);
}


/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm128")
