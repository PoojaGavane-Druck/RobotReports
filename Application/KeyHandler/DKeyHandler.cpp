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
#include "app_cfg.h"
MISRAC_ENABLE

#include "DPV624.h"
#include "main.h"
#include "uart.h"

/* Error handler instance parameter starts from 3601 to 3700 */

/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define KEY_HANDLER_TASK_STK_SIZE   512u    //not bytes (CPU_STK is 4 bytes, so multiply by 4 for stack size in bytes)
#define KEY_NEXT_KEY_WAIT_TIME_MS   100u

#define USE_OS 1
/* Variables --------------------------------------------------------------------------------------------------------*/
const uint32_t debounceTimeInMilliSec = 50u;
const uint32_t longPressTimeInMilliSec = 7000u;
const uint32_t batteryStatusTimeInMilliSec = 400u;
const uint32_t powerOnOffKeyPressTimeInMilliSecMin = 1000u;
const uint32_t powerOnOffKeyPressTimeInMilliSecMax = 5000u;
const uint32_t usbSwitchMsMin = 1000u;
const uint32_t usbSwitchMsMax = 3000u;
const uint32_t fwUpgradeMsMin = 4000u;
const uint32_t fwUpgradeMsMax = 6000u;
const uint32_t bleAdvertTimeMin = 1000u;
const uint32_t bleAdvertTimeMax = 3000u;

CPU_STK keyHandlerTaskStack[APP_CFG_KEY_HANDLER_TASK_STK_SIZE];

const uint32_t keyHandlerTaskTimeoutInMilliSec = 100u;

extern uint32_t extiIntFlag;
extern OS_SEM spiDataReady;
extern OS_FLAGS optInt1;
extern OS_FLAGS optInt2;
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

    myTaskId = eKeypadTask;
    myTaskStack = &keyHandlerTaskStack[0];

#ifdef ENABLE_STACK_MONITORING
    stackArray.uiStack.addr = (void *)myTaskStack;
    stackArray.uiStack.size = (uint32_t)(APP_CFG_KEY_HANDLER_TASK_STK_SIZE * 4u);
    fillStack((char *)myTaskStack, 0xCC, (size_t)(APP_CFG_KEY_HANDLER_TASK_STK_SIZE * 4u));
#endif

    triggered = false;
    timeoutCount = 0u;
    timeoutPowerKey = 0u;
    timeoutBtKey = 0u;
    pressType.bytes =  0u;

    powerPressed = 0u;
    powerReleased = 0u;
    btPressed = 0u;
    btReleased = 0u;

    powerTimer = 0u;
    btTimer = 0u;

    bothPressed = 0u;
    powerSequenceStarted = 0u;

    RTOSSemCreate(&gpioIntSem, "GpioSem", (OS_SEM_CTR)0, osErr); /* Create GPIO interrupt semaphore */

    activate(myName, (CPU_STK_SIZE)APP_CFG_KEY_HANDLER_TASK_STK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)0u, osErr);
}

/**
* @brief    Destructor
* @param    void
* @return   void
*/
DKeyHandler::~DKeyHandler()
{
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as we are using snprintf which violates the rule.
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm128")

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
        PV624->keepAlive(myTaskId);

        //pend until timeout, blocking, on the task message - posted by GPIO ISR on key press or a remote key press (eg, over DUCI)
        RTOSSemPend(&gpioIntSem, keyHandlerTaskTimeoutInMilliSec, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_error);

#ifdef ENABLE_STACK_MONITORING
        lastTaskRunning = myTaskId;
#endif
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
                               3601u);
        }

        else
        {
            processKey(os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT));
        }


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

    if(keyCode.bit.powerOnOff)
    {
        if(keyPressType.bit.updateBattery)
        {
            PV624->updateBatteryStatus();
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
    // New logic for single and multiple key presses
    if(1u == pressType.bit.fwUpgrade)
    {
        // Start the FW upgrade process
        PV624->performUpgrade();
    }

    if(1u == pressType.bit.usbSwitch)
    {
        // Switch USB mode between MSC and VCP
        PV624->switchUsbPortConfiguration();
    }

    if(1u == pressType.bit.powerOnOff)
    {
        // Power on or power off the device
        PV624->managePower();
    }

    if(1u == pressType.bit.updateBattery)
    {
        // Update battery level on the battery indication leds
        PV624->userInterface->updateBatteryStatus(5000u, 0u);
    }

    if(1u == pressType.bit.blueTooth)
    {
        // Start or stop bluetooth connectivity
        eBL652State_t bl652State = PV624->getBlState();

        if(((eBL652State_t)BL_STATE_RUN_ADV_IN_PROGRESS != bl652State) &&
                ((eBL652State_t)BL_STATE_RUN_CONNECTION_ESTABLISHED != bl652State))
        {
            PV624->manageBlueToothConnection(BL_STATE_START_ADVERTISING);
        }
    }

    keys.bytes = 0u;
    pressType.bytes = 0u;
    OS_ERR os_error = OS_ERR_NONE;
    RTOSTimeDlyHMSM(0u, 0u, 0u, KEY_NEXT_KEY_WAIT_TIME_MS, OS_OPT_TIME_HMSM_STRICT, &os_error);
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
    const uint32_t timeForPowerOnOffMin = powerOnOffKeyPressTimeInMilliSecMin / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timeForPowerOnOffMax = powerOnOffKeyPressTimeInMilliSecMax / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timeForUsbSwitchMin = usbSwitchMsMin / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timeForUsbSwitchMax = usbSwitchMsMax / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timeForFwUpgradeMin = fwUpgradeMsMin / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timeForFwUpgradeMax = fwUpgradeMsMax / keyHandlerTaskTimeoutInMilliSec;

    const uint32_t timeForBleAdventMin = bleAdvertTimeMin / keyHandlerTaskTimeoutInMilliSec;
    const uint32_t timeForBleAdventMax = bleAdvertTimeMax / keyHandlerTaskTimeoutInMilliSec;

    if(!triggered)
    {
        if(!timedOut)
        {
            // key down (falling edge)

            // debounce
            OS_ERR os_error = OS_ERR_NONE;
            RTOSTimeDlyHMSM(0u, 0u, 0u, debounceTimeInMilliSec, OS_OPT_TIME_HMSM_STRICT, &os_error);
            keys = getKey();

            if((1u == keys.bit.powerOnOff) && (1u == keys.bit.blueTooth))
            {
                // both keys are pressed
                timeoutCount = 0u;
                bothPressed = 1u;
                pressType.bytes = 0u;
                triggered = true;
            }

            else if(1u == keys.bit.powerOnOff)
            {
                timeoutCount = 0u;
                bothPressed = 0u;
                pressType.bytes = 0u;
                triggered = true;
            }

            else if(1u == keys.bit.blueTooth)
            {
                timeoutCount = 0u;
                bothPressed = 0u;
                pressType.bytes = 0u;
                triggered = true;
            }

            else
            {
                /* Do Nothing */
            }

            PV624->userInterface->saveStatusLedState();
            PV624->userInterface->saveBluetoothLedState();
        }
    }

    else
    {
        if(timeoutCount < timeLimitForLongPress)
        {
            if(!timedOut)
            {
                // key up (rising edge) before time limit
                /* Check both keys first */
                if((timeoutPowerKey >= timeForUsbSwitchMin) &&
                        (timeoutPowerKey <= timeForUsbSwitchMax) &&
                        (timeoutBtKey >= timeForUsbSwitchMin) &&
                        (timeoutBtKey <= timeForUsbSwitchMax) &&
                        (1u == keys.bit.powerOnOff) &&
                        (1u == keys.bit.blueTooth))
                {
                    /* both keys were pressed for time limit of usb port switch */
                    timeoutPowerKey = 0u;
                    timeoutBtKey = 0u;
                    pressType.bit.usbSwitch = true;
                    sendKey();
                    PV624->userInterface->restoreStatusLedState();
                    PV624->userInterface->restoreBluetoothLedState();
                    triggered = false;
                }

                if((timeoutBtKey >= timeForFwUpgradeMin) &&
                        (timeoutBtKey <= timeForFwUpgradeMax) &&
                        (1u == keys.bit.blueTooth))
                {
                    // Begin firmware upgrade process here
                    timeoutPowerKey = 0u;
                    timeoutBtKey = 0u;
                    pressType.bit.fwUpgrade = true;
                    sendKey();
                    PV624->userInterface->restoreStatusLedState();
                    PV624->userInterface->restoreBluetoothLedState();
                    triggered = false;
                }

                else if((timeoutPowerKey <= timeLimitForBatteryStatus) &&
                        (1u == keys.bit.powerOnOff) &&
                        (0u == keys.bit.blueTooth))
                {
                    timeoutPowerKey = 0u;
                    timeoutBtKey = 0u;
                    pressType.bit.updateBattery = true;
                    sendKey();
                    PV624->userInterface->restoreStatusLedState();
                    PV624->userInterface->restoreBluetoothLedState();
                    triggered = false;
                }

                else if((timeoutPowerKey >= timeForPowerOnOffMin) &&
                        (timeoutPowerKey <= timeForPowerOnOffMax) &&
                        (1u == keys.bit.powerOnOff) &&
                        (0u == keys.bit.blueTooth))
                {
                    timeoutPowerKey = 0u;
                    timeoutBtKey = 0u;
                    powerSequenceStarted = 0u;
                    triggered = false;
                }

                else if((timeoutBtKey >= timeForBleAdventMin) &&
                        (timeoutBtKey <= timeForBleAdventMax) &&
                        (0u == keys.bit.powerOnOff) &&
                        (1u == keys.bit.blueTooth))
                {
                    timeoutBtKey = 0u;
                    pressType.bit.blueTooth = true;
                    sendKey();
                    PV624->userInterface->restoreStatusLedState();
                    PV624->userInterface->restoreBluetoothLedState();
                    triggered = false;
                }

                else
                {
                    // Only reset if entered on timeout, meaning not a valid key press
                    if(1u == bothPressed)
                    {
                        keys = getKey();

                        // Wait for both to be released
                        if((0u == keys.bit.powerOnOff) &&
                                (0u == keys.bit.blueTooth))
                        {
                            triggered = false;
                            PV624->userInterface->restoreStatusLedState();
                            PV624->userInterface->restoreBluetoothLedState();
                            bothPressed = 0u;
                        }
                    }

                    else
                    {
                        triggered = false;
                        PV624->userInterface->restoreStatusLedState();
                        PV624->userInterface->restoreBluetoothLedState();
                    }
                }

                if(1u == bothPressed)
                {
                    keys = getKey();

                    // Wait for both to be released
                    if((0u == keys.bit.powerOnOff) &&
                            (0u == keys.bit.blueTooth))
                    {
                        triggered = false;
                        bothPressed = 0u;
                        PV624->userInterface->restoreStatusLedState();
                        PV624->userInterface->restoreBluetoothLedState();
                    }
                }

                else
                {
                    bothPressed = 0u;
                }
            }

            else
            {
                // key still pressed (no edge) before time limit
                timeoutCount++;

                if(1u == keys.bit.powerOnOff)
                {
                    timeoutPowerKey++;
                }

                if(1u == keys.bit.blueTooth)
                {
                    timeoutBtKey++;
                }

                // Read keys again to check if second key has been pressed
                keys = getKey();

                // Indicate using LEDS what key is pressed
                if((timeoutCount >= timeForUsbSwitchMin) &&
                        (timeoutCount <= timeForUsbSwitchMax) &&
                        (1u == keys.bit.powerOnOff) &&
                        (1u == keys.bit.blueTooth))
                {
                    PV624->userInterface->statusLedControl(eStatusProcessing,
                                                           E_LED_OPERATION_SWITCH_ON,
                                                           65535,
                                                           E_LED_STATE_SWITCH_OFF,
                                                           0u);

                    PV624->userInterface->bluetoothLedControl(eBlueToothPurple,
                            E_LED_OPERATION_SWITCH_ON,
                            65535,
                            E_LED_STATE_SWITCH_OFF,
                            0u);
                }

                else if((timeoutCount >= timeForFwUpgradeMin) &&
                        (timeoutCount <= timeForFwUpgradeMax) &&
                        (1u == keys.bit.powerOnOff) &&
                        (1u == keys.bit.blueTooth))
                {
                    PV624->userInterface->bluetoothLedControl(eBlueToothPurple,
                            E_LED_OPERATION_SWITCH_ON,
                            65535,
                            E_LED_STATE_SWITCH_OFF,
                            0u);
                }

                else if((timeoutCount >= timeForBleAdventMin) &&
                        (timeoutCount <= timeForBleAdventMax) &&
                        (0u == keys.bit.powerOnOff) &&
                        (1u == keys.bit.blueTooth))
                {
                    PV624->userInterface->bluetoothLedControl(eBlueToothPairing,
                            E_LED_OPERATION_SWITCH_ON,
                            65535,
                            E_LED_STATE_SWITCH_OFF,
                            0u);
                }

                else if((timeoutCount >= timeForPowerOnOffMin) &&
                        (timeoutCount <= timeForPowerOnOffMax) &&
                        (1u == keys.bit.powerOnOff))
                {
                    if(0u == powerSequenceStarted)
                    {
                        pressType.bit.powerOnOff = true;
                        sendKey();
                        powerSequenceStarted = 1u;
                    }

                }

                else
                {

                }
            }
        }

        else
        {
            // exceeded time limit
            timeoutCount = 0u;
            timeoutPowerKey = 0u;
            timeoutBtKey = 0u;
            triggered = false;
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
        RTOSSemPost(&spiDataReady, OS_OPT_POST_ALL, &osErr);
    }

    if((GPIO_PIN_8 & GPIO_Pin) || (GPIO_PIN_9 & GPIO_Pin))
    {
        RTOSSemPost(&gpioIntSem, OS_OPT_POST_ALL, &osErr);
    }

    if(GPIO_PIN_2 & GPIO_Pin)
    {
        /* Check if it is optical sensor interrupt 1 */
        PV624->instrument->opticalEvent(1u);
    }

    if(GPIO_PIN_13 & GPIO_Pin)
    {
        /* Check if it is optical sensor interrupt 2 */
        PV624->instrument->opticalEvent(2u);
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
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma("diag_default=Pm128")
