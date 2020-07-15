/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DProcess.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The measurement input process class source file
*
**********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DProcess.h"

//MISRAC_DISABLE
//#include <string.h>
//MISRAC_ENABLE

#include "DDPI610E.h"

/* Constants and Defines --------------------------------------------------------------------------------------------*/

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProcess class constructor
 * @param   void
 * @retval  void
 */
DProcess::DProcess(uint32_t channelIndex)
{
    OS_ERR os_error;

    //store channel index
	myChannelIndex = channelIndex;

    //by default a process is disabled unless user enabled
    myEnabledState = false;

    //create mutex for resource locking
    char *name = "Process";

    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &os_error);

    if (os_error != (OS_ERR)OS_ERR_NONE)
    {
        //Error handler?
    }
}

/**
 * @brief   Enable process
 * @param   void
 * @retval  void
 */
void DProcess::enable(void)
{
    DLock is_on(&myMutex);

    if (myEnabledState != true)
    {
        myEnabledState = true;
        notify(E_UI_MSG_PROCESS_ENABLED);
    }
}

/**
 * @brief   Disable process
 * @param   void
 * @retval  void
 */
void DProcess::disable(void)
{
    DLock is_on(&myMutex);

    if (myEnabledState != false)
    {
        myEnabledState = false;
        notify(E_UI_MSG_PROCESS_DISABLED);
    }
}

/**
 * @brief   Run process
 * @param   input value
 * @retval  processed value
 */
float32_t DProcess::run(float32_t input)
{
    return input;
}

/**
 * @brief   Reset process
 * @param   void
 * @retval  void
 */
void DProcess::reset(void)
{
}

/**
 * @brief   Notify events to user interface
 * @param   event id
 * @retval  void
 */
void DProcess::notify(eUiMessage_t event)
{
    DPI610E->userInterface->notify(event, myChannelIndex, myProcessIndex);
}

/**
 * @brief   Get last process input value
 * @param   void
 * @retval  input value
 */
float32_t DProcess::getInput(void)
{
    DLock is_on(&myMutex);
    return myInput;
}

/**
 * @brief   Set last process input value
 * @param   input value
 * @retval  void
 */
void DProcess::setInput(float32_t value)
{
    DLock is_on(&myMutex);
    myInput = value;
}

/**
 * @brief   get last processed output value
 * @param   void
 * @retval  processed value
 */
float32_t DProcess::getOutput(void)
{
    DLock is_on(&myMutex);
    return myOutput;
}

/**
 * @brief   Set last processed output value
 * @param   processed value
 * @retval  void
 */
void DProcess::setOutput(float32_t value)
{
    DLock is_on(&myMutex);
    myInput = value;
}

/**
 * @brief   Set process parameter (instance specific meaning)
 * @param   value is the paramter value
 * @param   index is an identifier for the parameter
 * @retval  void
 */
void DProcess::setParameter(float32_t value, uint32_t index)
{
}

/**
 * @brief   Get process parameter (instance specific meaning)
 * @param   index is an identifier for the parameter
 * @retval  parameter value
 */
float32_t DProcess::getParameter(uint32_t index)
{
    return 0.0f;
}

