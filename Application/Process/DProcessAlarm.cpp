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
* @file     DProcessAlarm.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The alarm process base class source file
*
**********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DProcessAlarm.h"

/* Constants and Defines --------------------------------------------------------------------------------------------*/

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProcessAlarm class constructor
 * @param   void
 * @retval  void
 */
DProcessAlarm::DProcessAlarm(uint32_t channelIndex)
: DProcess(channelIndex)
{
	reset();
}

/**
 * @brief   Reset process
 * @param   void
 * @retval  void
 */
void DProcessAlarm::reset()
{
    DLock is_on(&myMutex);
	myAlarmState = false;
    notify(E_UI_MSG_ALARM_CLEAR);
}

/**
 * @brief   Set process parameter (instance specific meaning)
 * @param   value is the alarm threshold level
 * @param   index is ignored (recommended to be 0)
 * @retval  void
 */
void DProcessAlarm::setParameter(float32_t value, uint32_t index)
{
    DLock is_on(&myMutex);
    myThreshold = value;
}

/**
 * @brief   Get process parameter (instance specific meaning)
 * @param   index is an identifier for the parameter
 * @retval  parameter value
 */
float32_t DProcessAlarm::getParameter(uint32_t index)
{
    DLock is_on(&myMutex);
    return myThreshold;
}




