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
    myInstrumentMode.value = 0u;

    //safe to 'new' a stack here as it is never 'free'd.
    CPU_STK_SIZE stackBytes = UI_HANDLER_TASK_STK_SIZE * (CPU_STK_SIZE)sizeof(CPU_STK_SIZE);
    myTaskStack = (CPU_STK *)new char[stackBytes];

    activate(myName, (CPU_STK_SIZE)UI_HANDLER_TASK_STK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)10u, osErr);
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
    OS_ERR os_error;

    OS_MSG_SIZE msg_size;
    CPU_TS ts;
    uint32_t rxMsg;

    

    //task main loop
    while (DEF_TRUE)
    {
        //wait forever, blocking, for a message on the task queue
        rxMsg = (uint32_t)OSTaskQPend((OS_TICK)0, OS_OPT_PEND_BLOCKING, &msg_size, &ts, &os_error);

        //handle error first, in case something goes wrong in the OS
        if (os_error != OS_ERR_NONE)
        {
            //display error
        }
        else if (msg_size == (OS_MSG_SIZE)0u)	//message size = 0 means 'rxMsg' is the message itself (always the case)
        {
            sTaskMessage_t message;
            message.value = rxMsg;

 
        }
        else
        {
            //do nothing
        }
    }
}

OS_ERR DUserInterface::postEvent(uint32_t event, uint32_t param8, uint32_t param16)
{
    OS_ERR os_error = OS_ERR_NONE;

    sTaskMessage_t message;
    message.value = 0u;
    message.event = event;

    message.param8 = param8;
    message.param16 = param16;

    //Post message to User Interface Task
    OSTaskQPost(&myTaskTCB, (void *)message.value, (OS_MSG_SIZE)0, (OS_OPT) OS_OPT_POST_FIFO, &os_error);

    if (os_error != OS_ERR_NONE)
    {
        //error
    }

    return os_error;
}

/**
* @brief	handleKey - process key presses (from key handler or simulated via remote command).
* @param    keyPressed - code for the key pressed
* @param    pressType - short (0) or long press (1)
* @return   os_error - error status from OS related operations
*/
OS_ERR DUserInterface::handleKey(uint32_t keyPressed, uint32_t pressType)
{
    return postEvent(E_UI_MSG_KEYPRESS, pressType, keyPressed);
}

//DONE This way just to prove the mechanism that key presses are sent to the task and actioned from there - so everything goes through the UI?
void DUserInterface::processKey(uint32_t keyPressed, uint32_t pressType)
{
//    float32_t setpoint[] = { 10.0f, 16.0f, 20.0f, 24.0f };
//    static int index = 0;

    if (pressType == 0u)
    {
        switch (keyPressed)
        {
            case 1u:
                
                break;
            case 2u:
                
                break;
           
            default:
                //displayText("--");
                break;
        }
    }
    else
    {
        //long press
        switch (keyPressed)
        {
            case 1u:
                
                break;
            case 2u:
               
                break;
            case 4u:
                
                break;
            case 8u:
                
                break;
            case 16u:
               
                break;
            case 32u:
                
                break;
            default:
                
                break;
        }
    }
}





/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm128")

/**
* @brief	handleMessage - process messaages from the rest of the system
* @param    uiMessage - enumerated type message identifier value
* @return   os_error - error status from OS related operations
*/
OS_ERR handleMessage(eUiMessage_t uiMessage)
{
    return OS_ERR_NONE;
}




/**
* @brief	Get instrument mode
* @param    void
* @return   mode
*/
sInstrumentMode_t DUserInterface::getMode(void)
{
    return myInstrumentMode;
}

/**
* @brief	Set instrument status bits as specified
* @param    bit mask
* @return   void
*/
void DUserInterface::setMode(sInstrumentMode_t mask)
{
    myInstrumentMode.value |= mask.value;
}

/**
* @brief	Clear instrument status bits as specified
* @param    bit mask
* @return   void
*/
void DUserInterface::clearMode(sInstrumentMode_t mask)
{
    myInstrumentMode.value &= ~mask.value;
}

void DUserInterface::functionShutdown(uint32_t channel)
{
    postEvent(E_UI_MSG_FUNCTION_SHUTDOWN, 0u, channel);
}

void DUserInterface::updateReading(uint32_t channel)
{
    postEvent(E_UI_MSG_NEW_READING, 0u, channel);
}

void DUserInterface::notify(eUiMessage_t event, uint32_t channel, uint32_t index)
{
    postEvent(event, index, channel);
}

void DUserInterface::sensorConnected(uint32_t channel)
{
    postEvent(E_UI_MSG_SENSOR_CONNECTED, 0u, channel);
}

void DUserInterface::sensorDisconnected(uint32_t channel)
{
    postEvent(E_UI_MSG_SENSOR_DISCONNECTED, 0u, channel);
}

void DUserInterface::sensorPaused(uint32_t channel)
{
    postEvent(E_UI_MSG_SENSOR_PAUSED, 0u, channel);
}

