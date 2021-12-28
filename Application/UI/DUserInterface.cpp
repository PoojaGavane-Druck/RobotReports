/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DUserInterface.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     30 March 2020
*
* @brief    The user interface class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DUserInterface.h"

MISRAC_DISABLE
#include <stdlib.h>
MISRAC_ENABLE


#include "DInstrument.h"
#include "DPV624.h"
#include "Types.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define UI_HANDLER_TASK_STK_SIZE   512u    //this is not bytes (CPU_STK is 4 bytes, so multiply by 4 for stack size in bytes)


/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DUserInterface class constructor
 * @param   void
 * @retval  void
 */
DUserInterface::DUserInterface(OS_ERR *osErr)
: DTask()
{
    myName = "ui";
   
    //safe to 'new' a stack here as it is never 'free'd.
    CPU_STK_SIZE stackBytes = UI_HANDLER_TASK_STK_SIZE * (CPU_STK_SIZE)sizeof(CPU_STK_SIZE);
    myTaskStack = (CPU_STK *)new char[stackBytes];

    activate(myName, (CPU_STK_SIZE)UI_HANDLER_TASK_STK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)10u, osErr);
    
    statusLedBlinkRateCounter = 0u;
    bluettothLedBlinkRateCounter = 0u;
    batteryLedUpdateRateCounter = 0u;
    
    
}

/**
 * @brief   DUserInterface class destructor
 * @param   void
 * @retval  void
 */
DUserInterface::~DUserInterface()
{
}

void DUserInterface::initialise(void)
{
   

    

    
}

void DUserInterface::cleanUp(void)
{
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as we are using snprintf which violates the rule.
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm128")

/**
* @brief	UI task run - the top level functon that handles key presses.
* @param    void
* @return   void
*/
void DUserInterface::runFunction(void)
{
    OS_ERR os_error = OS_ERR_NONE;

    OS_MSG_SIZE msg_size;
    CPU_TS ts;

    //task main loop
    while(DEF_TRUE)
    {
        //wait until timeout, blocking, for a message on the task queue
        uint32_t rxMsgValue = static_cast<uint32_t>(reinterpret_cast<intptr_t>(OSTaskQPend((OS_TICK)UI_TASK_TIMEOUT_MS, OS_OPT_PEND_BLOCKING, &msg_size, &ts, &os_error)));

#ifdef STACK_MONITOR
        lastTaskRunning = myLastTaskId;
#endif

#ifdef WATCH_MONITOR
        keepAlive();
#endif  
        switch(os_error)
        {
            case OS_ERR_NONE:
                if(msg_size == (OS_MSG_SIZE)0u)    //message size = 0 means 'rxMsg' is the message itself (always the case)
                {
                    processMessage(rxMsgValue);
                }

                break;

            case OS_ERR_TIMEOUT:
                 handleTimeout();
                break;

            default:
                break;
        }
    }

}

/**
* @brief    processMessage - processes messages received.
* @param    uint32_t rxMsgValue
* @return   void
*/
void DUserInterface::processMessage(uint32_t rxMsgValue)
{
   sLedMessage_t message;
   message.value = rxMsgValue;

   if(eStatusLed == message.led)
   {
     statusLed.colour = (eLedColour_t)message.colour;
     statusLed.operation = (eLedOperation_t)message.operation;
     statusLed.stateAfterOperationCompleted = (eLedState_t) message.ledStateAfterTimeout;
     statusLed.displayTime =  message.displayTime;
     statusLed.blinkingRate = message.blinkingRate;
     statusLedBlinkRateCounter = 0;
     switch(statusLed.operation)
     {
       case E_LED_OPERATION_NONE:
       case E_LED_OPERATION_SWITCH_OFF:
         myLeds.ledOff((eLeds_t)eStatusLed);
       break;
      
       
       case E_LED_OPERATION_SWITCH_ON:
         myLeds.ledOn((eLeds_t)eStatusLed, statusLed.colour);
       break;
       
       case E_LED_OPERATION_TOGGLE:
         myLeds.ledBlink((eLeds_t)eStatusLed, statusLed.colour);
       break;
       
     default:
       break;
     }
   }
   
   if(eBluetoothLed == message.led)
   {
     blueToothLed.colour = (eLedColour_t)message.colour;
     blueToothLed.operation = (eLedOperation_t)message.operation;
     blueToothLed.stateAfterOperationCompleted = (eLedState_t)message.ledStateAfterTimeout;
     blueToothLed.displayTime =  message.displayTime;
     blueToothLed.blinkingRate = message.blinkingRate;
     bluettothLedBlinkRateCounter = 0;
     switch(blueToothLed.operation)
     {
       case E_LED_OPERATION_NONE:
       case E_LED_OPERATION_SWITCH_OFF:
         myLeds.ledOff((eLeds_t)eBluetoothLed);
       break;
      
       
       case E_LED_OPERATION_SWITCH_ON:
         myLeds.ledOn((eLeds_t)eBluetoothLed, blueToothLed.colour);
       break;
       
       case E_LED_OPERATION_TOGGLE:
         myLeds.ledBlink((eLeds_t)eBluetoothLed, blueToothLed.colour);
       break;
       
     default:
       break;
     }
   }
   
   if(eBatteryLed == message.led)
   {
     
      batteryLed.stateAfterOperationCompleted = (eLedState_t)message.ledStateAfterTimeout;
      batteryLed.displayTime =  message.displayTime;
      batteryLed.blinkingRate = message.blinkingRate;
      batteryLedUpdateRateCounter = 0;
      float  percentCap = 0.0f;
      uint32_t chargingStatus = 0u;
      PV624->getBatLevelAndChargingStatus((float*)&percentCap, (uint32_t*)&chargingStatus);
      myLeds.updateBatteryLeds(percentCap, chargingStatus);
   }
}

OS_ERR DUserInterface::postEvent(uint32_t event)
{
    OS_ERR os_error = OS_ERR_NONE;

   //Post message to User Interface Task
    OSTaskQPost(&myTaskTCB, (void *)event, (OS_MSG_SIZE)0, (OS_OPT) OS_OPT_POST_FIFO, &os_error);

    if (os_error != OS_ERR_NONE)
    {
        //error
    }

    return os_error;
}

/**
* @brief    switch On the LED
* @param    keyPressed - code for the key pressed
* @param    pressType - short (0) or long press (1)
* @return   os_error - error status from OS related operations
*/

void DUserInterface::handleTimeout(void)
{
  
  if(blueToothLed.displayTime > 0u)
  {  
    bluettothLedBlinkRateCounter++;
    if(bluettothLedBlinkRateCounter >= blueToothLed.blinkingRate)
    {
      myLeds.ledBlink((eLeds_t)eBluetoothLed, (eLedColour_t)blueToothLed.colour);
      bluettothLedBlinkRateCounter = 0u;
    }
    blueToothLed.displayTime--;
    if(0u == blueToothLed.displayTime)
    {
      if(E_LED_STATE_SWITCH_OFF == blueToothLed.stateAfterOperationCompleted)
      {
        myLeds.ledOff(eBluetoothLed);
      }
    }
  }
  
  if(statusLed.displayTime > 0u)
  {
    statusLedBlinkRateCounter++;
    if(statusLedBlinkRateCounter >= statusLed.blinkingRate)
    {
      myLeds.ledBlink((eLeds_t)eStatusLed,(eLedColour_t)statusLed.colour);
      statusLedBlinkRateCounter = 0;
    }
    statusLed.displayTime--;
    if(0u == statusLed.displayTime)
    {
      if(E_LED_STATE_SWITCH_OFF == statusLed.stateAfterOperationCompleted)
      {
        myLeds.ledOff(eStatusLed);
      }
    }
  }
  
  if(batteryLed.displayTime > 0u)
  {
    batteryLedUpdateRateCounter++;
    if(batteryLedUpdateRateCounter >= batteryLed.blinkingRate)
    {
      float  percentCap = 0.0f;
      uint32_t chargingStatus = 0u;
      PV624->getBatLevelAndChargingStatus((float*)&percentCap, (uint32_t*)&chargingStatus);
      myLeds.updateBatteryLeds(percentCap, chargingStatus);
    }
    batteryLed.displayTime--;
    if(batteryLed.displayTime == 0u)
    {
      myLeds.ledOff(eBatteryLed);
    }
    
  }
}

/**
* @brief    To control the status LED as per operation
* @param    status - Okay/InProgress/Error
* @param    display time - time to stay in that operationn
* @param   stateAfterTimeout -after disply time completed led state
*/
void DUserInterface::statusLedControl(eStatusLed_t status,
                               eLedOperation_t operation,
                               uint16_t displayTime, 
                               eLedState_t stateAfterTimeout,
                               uint32_t blinkingRate = UI_DEFAULT_BLINKING_RATE)
{
  sLedMessage_t ledMessage;
   
   ledMessage.led = eStatusLed;
   ledMessage.displayTime = displayTime;
   ledMessage.ledStateAfterTimeout = stateAfterTimeout;
   ledMessage.operation = operation;
   ledMessage.blinkingRate = blinkingRate;
  
  switch(status)
  {
    
    case eStatusOkay:
      ledMessage.colour = eLedColourGreen;
    break;
    
    case eStatusProcessing:
      ledMessage.colour = eLedColourYellow;
    break;
    
    case eStatusError:
      ledMessage.colour = eLedColourRed;
    break;
    
  default:
    break;
  }
   postEvent(ledMessage.value);
}

/**
* @brief    To control the bluetooth LED as per operation
* @param    status - Okay/InProgress/Error
* @param    display time - time to stay in that operationn
* @param   stateAfterTimeout -after disply time completed led state
*/
void DUserInterface::bluetoothLedControl(eBlueToothLed_t status,
                               eLedOperation_t operation,
                               uint16_t displayTime, 
                               eLedState_t stateAfterTimeout,
                               uint32_t blinkingRate = UI_DEFAULT_BLINKING_RATE)
{
  sLedMessage_t ledMessage;
   
   ledMessage.led = eStatusLed;
   ledMessage.displayTime = displayTime;
   ledMessage.ledStateAfterTimeout = stateAfterTimeout;
   ledMessage.operation = operation;
   ledMessage.blinkingRate = blinkingRate;
  
  switch(status)
  {
    
    case eStatusOkay:
      ledMessage.colour = eLedColourGreen;
    break;
    
    case eStatusProcessing:
      ledMessage.colour = eLedColourYellow;
    break;
    
    case eStatusError:
      ledMessage.colour = eLedColourRed;
    break;
    
  default:
    break;
  }
   postEvent(ledMessage.value);
}

void DUserInterface::updateBatteryStatus(uint16_t displayTime, 
                                         uint32_t updateRate)
{
    sLedMessage_t ledMessage;
   
   ledMessage.led = eBatteryLed;
   ledMessage.displayTime = displayTime;
   ledMessage.blinkingRate = updateRate;
   postEvent(ledMessage.value);
   
   
}
/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm128")






