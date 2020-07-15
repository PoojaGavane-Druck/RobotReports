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
* @file     DProcessAlarmUserLo.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The sensor low alarm process class source file
*
**********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DProcessAlarmUserLo.h"

/* Constants and Defines --------------------------------------------------------------------------------------------*/

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProcessAlarmUserLo class constructor
 * @param   void
 * @retval  void
 */
DProcessAlarmUserLo::DProcessAlarmUserLo(uint32_t channelIndex)
: DProcessAlarm(channelIndex)
{
    myProcessIndex = E_PROCESS_USER_ALARM_LO;

    //threshold is set to a very big number and expected to be user set as somewhere within the sensor fullscale/range
    myThreshold = BIG_NEGATIVE_NUMBER;
}

/**
 * @brief   Run process
 * @param   input value
 * @retval  input value (unchanged)
 */
float32_t DProcessAlarmUserLo::run(float32_t input)
{
    //only need to process if enabled
    if (myEnabledState == true)
    {
        //if at or below threshold value and not currently in alarm then set alarm state
        //else if above threshold value and currently in alarm then clear alarm state
        if ((input <= myThreshold) && (myAlarmState == false))
        {
            myAlarmState = true;
            notify(E_UI_MSG_ALARM_SET);
        }
        else if ((input > myThreshold) && (myAlarmState == true))
        {
            myAlarmState = false;
            notify(E_UI_MSG_ALARM_CLEAR);
        }
        else
        {
            //required by MISRA C rules
        }
    }

	return input;
}
