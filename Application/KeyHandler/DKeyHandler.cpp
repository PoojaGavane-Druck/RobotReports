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
const uint32_t debounceTimeInMilliSec = (uint32_t)50;
const uint32_t longPressTimeInMilliSec = (uint32_t)4500;
const uint32_t batteryStatusTimeInMilliSec = (uint32_t)350;
const uint32_t powerOnOffKeyPressTimeInMilliSec = (uint32_t)500;
const uint32_t keyHandlerTaskTimeoutInMilliSec = (uint32_t)100;

extern uint32_t extiIntFlag;
extern OS_SEM spiDataReady;
OS_SEM gpioIntSem;

/**
* @brief    Constructor
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
    powerState = 1u;
    pressType.bytes =  0u;
    OSSemCreate(&gpioIntSem, "GpioSem", (OS_SEM_CTR)0, osErr); /* Create GPIO interrupt semaphore */

    bool ok = (*osErr == static_cast<OS_ERR>(OS_ERR_NONE)) || (*osErr == static_cast<OS_ERR>(OS_ERR_TIMEOUT));

    if(!ok)
    {
#ifdef ASSERT_IMPLEMENTED      
MISRAC_DISABLE
        assert(false);
MISRAC_ENABLE
#endif
        PV624->handleError(E_ERROR_OS, 
                           eSetError,
                           (uint32_t)osErr,
                           (uint16_t)30);
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
* @brief    Key Task run - the top level functon that handles key presses.
* @param    void
* @return   void
*/
void DKeyHandler::runFunction(void)
{
    OS_ERR os_error = OS_ERR_NONE;

    //task main loop
    while(DEF_TRUE)
    {
        //pend until timeout, blocking, on the task message - posted by GPIO ISR on key press or a remote key press (eg, over DUCI)
        OSSemPend(&gpioIntSem, keyHandlerTaskTimeoutInMilliSec, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_error);

        bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT));

        if(!ok)
        {
#ifdef ASSERT_IMPLEMENTED
MISRAC_DISABLE
            assert(false);
MISRAC_ENABLE
#endif
           PV624->handleError(E_ERROR_OS, 
                               eSetError,
                               (uint32_t)os_error,
                               (uint16_t)4);
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
* @param keyCode represents what are all the buttons are pressed
* @param keyPressType tells long press or short press
* @return void
*/
void DKeyHandler::sendKey(gpioButtons_t keyCode, pressType_t keyPressType)
{
    
    if (keyCode.bit.powerOnOff)
    {
        if(keyPressType.bit.updateBattery)
        {
            PV624->powerManager->updateBatteryStatus();
        }
        else if(keyPressType.bit.powerOnOff)
        {
            /* Handling Power Off button */
            
        }
        else
        {
            /* Do Nothing */
        }
           
    }
    else if(keyCode.bit.blueTooth)
    {       
        /*Handling Blue tooth */
    }
    else
    {
        /* Do Nothing */
    }
}

/**
* @brief Sends validated key press
* @param void
* @return void
*/
void DKeyHandler::sendKey(void)
{
    
    static uint32_t ledNum = 0;
    if (keys.bit.powerOnOff)
    {
        if(pressType.bit.updateBattery)
        {
            PV624->powerManager->updateBatteryStatus();
        }
        else if(pressType.bit.powerOnOff)
        {
            /* Handling Power Off button */
            if(1u == powerState)
            {
                PV624->shutdown();
                powerState = 0u;
            }
            else
            {
                PV624->startup();
                powerState = 1u;
            }              
        }
        else
        {
            /* Do Nothing */
        }
           
    }
    else if(keys.bit.blueTooth)
    {        
        /*Handling Blue tooth */
        /* ToDO: Added by nag for testing blue tooth button. Need to remove */        
        if(0 == ledNum)
        {
           HAL_GPIO_WritePin(BT_INDICATION_PE5_GPIO_Port, BT_INDICATION_PE5_Pin, GPIO_PIN_SET);  
           ledNum =1;
        }
        else
        {
           HAL_GPIO_WritePin(BT_INDICATION_PE5_GPIO_Port, BT_INDICATION_PE5_Pin, GPIO_PIN_RESET);     
           ledNum =0;
        }       
    }
    else
    {
      /* Do Nothing */
    }
    keys.bytes = 0u;
    pressType.bytes = 0u;
    //send key to user interface
    //PV624->userInterface->handleKey(keyPressed, pressType);
    OS_ERR os_error = OS_ERR_NONE;
    OSTimeDlyHMSM(0u, 0u, 0u, KEY_NEXT_KEY_WAIT_TIME_MS, OS_OPT_TIME_HMSM_STRICT, &os_error);
}

/**
* @brief Read port pins to get the current state of buttons 
* @param timeout : if any button pressed this flag is true otherwise it is false.
* @return void
*/
void DKeyHandler::processKey(bool timedOut)
{
    const uint32_t timeLimitForLongPress = longPressTimeInMilliSec / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timeLimitForBatteryStatus = batteryStatusTimeInMilliSec / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timeLimitForPowerOnOff = powerOnOffKeyPressTimeInMilliSec / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timePowerOnOffMin = 1000u / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timePowerOnOffMax = 3000u / keyHandlerTaskTimeoutInMilliSec;
    
    if (!triggered)
    {
        if (!timedOut)
        {
            // key down (falling edge)
            // debounce
            OS_ERR os_error = OS_ERR_NONE;
            OSTimeDlyHMSM(0u, 0u, 0u, debounceTimeInMilliSec, OS_OPT_TIME_HMSM_STRICT, &os_error);
            keys = getKey();
            if (keys.bit.powerOnOff != 0u)
            {
                timeoutCount = 0u;
                pressType.bytes = 0u;
                triggered = true;
            }
            else if (keys.bit.blueTooth != 0u)
            {            
                timeoutCount = 0u;                
                triggered = true;
            }
            else
            {
                /* Do Nothing */
            }
        }
    }
    else
    {
        if (timeoutCount < timeLimitForLongPress)
        {
            if (!timedOut)
            {
                // key up (rising edge) before time limit
                if((timeoutCount < timeLimitForBatteryStatus) && 
                   (keys.bit.powerOnOff))                   
                {
                    timeoutCount = 0u;
                    triggered = false;
                    pressType.bit.updateBattery = true;
                    sendKey();
                }
                if((timeoutCount > timeLimitForPowerOnOff) && 
                   (keys.bit.powerOnOff))                   
                {
                    PV624->leds->statusLedControl(eStatusOff);
                    timeoutCount = 0u;
                    triggered = false;
                    pressType.bit.powerOnOff = true;
                    sendKey();
                }
                if((timeoutCount < timeLimitForBatteryStatus) && 
                   (keys.bit.blueTooth))                   
                {                  
                    triggered = false;
                    sendKey();                  
                }                               
            }
            else
            {
                // key still pressed (no edge) before time limit
                timeoutCount++;
                // trigger LEDs as per key pressed
                if((timeoutCount >= timePowerOnOffMin) && (timeoutCount <= timePowerOnOffMax))
                {
                    PV624->leds->statusLedControl(eStatusCyan);
                }
            }
        }
        else
        {            
            // exceeded time limit
            PV624->leds->statusLedControl(eStatusOff);
            timeoutCount = 0u;
            triggered = false;
            if(keys.bit.powerOnOff)
            {
                pressType.bit.powerOnOff = true;  
            }
            else
            {
                pressType.bytes = 0u;
            }
            sendKey();
        }
    }
}

/**
* @brief Read port pins to get the current state of buttons 
* @param void
* @return gpioButtons_t buttons current status
*/
gpioButtons_t DKeyHandler::getKey(void)
{
    gpioButtons_t keyCodeMsg;
    keyCodeMsg.bytes = 0u;

    keyCodeMsg.bit.powerOnOff    = !HAL_GPIO_ReadPin(POWER_KEY_PF8_GPIO_Port,
                                                POWER_KEY_PF8_Pin);

    keyCodeMsg.bit.blueTooth     = !HAL_GPIO_ReadPin(BT_KEY_PF9_GPIO_Port,
                                                BT_KEY_PF9_Pin);
  
    return keyCodeMsg;
}

/**
* @brief callback function for gpio external interrupt
* @param GPIO_Pin gpio pin number in the port
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

    OS_ERR osErr = OS_ERR_NONE;

    if(GPIO_PIN_0 & GPIO_Pin)
    {
      OSSemPost(&spiDataReady, OS_OPT_POST_ALL, &osErr);    
    }
    
    if ((GPIO_PIN_8 & GPIO_Pin) ||(GPIO_PIN_9 & GPIO_Pin) )
    {
        OSSemPost(&gpioIntSem, OS_OPT_POST_ALL, &osErr);
    }

}

/**
* @brief Gets current state of keys for test purposes
* @param void
* @return gpioButtons_t buttons current status
*/
uint32_t DKeyHandler::getKeys(void)
{
    return static_cast<uint32_t>(getKey().bytes);
}

/**
* @brief Simulates key presses for test purposes
* @param keyCodes : respective key code  to simulate the keypress
* @param duration : keypress time
*/

void DKeyHandler::setKeys(uint32_t keyCodes, uint32_t duration)
{
    gpioButtons_t key;
    key.bytes = static_cast<uint32_t>(1 << (keyCodes - 1));
    keys.bytes = key.bytes;
    if((duration < batteryStatusTimeInMilliSec) && 
                   (key.bit.powerOnOff))
    {
        pressType.bit.updateBattery = true;
    }
    if((duration > powerOnOffKeyPressTimeInMilliSec) && 
                   (keys.bit.powerOnOff))
    {
      pressType.bit.powerOnOff = true;
    }
    

}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm128")
