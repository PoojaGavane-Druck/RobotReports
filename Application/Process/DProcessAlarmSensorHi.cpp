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
* @file     DProcessAlarmSensorHi.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The sensor high alarm process class source file
*
**********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DProcessAlarmSensorHi.h"

/* Constants and Defines --------------------------------------------------------------------------------------------*/

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProcessAlarmSensorHi class constructor
 * @param   void
 * @retval  void
 */
DProcessAlarmSensorHi::DProcessAlarmSensorHi(uint32_t channelIndex)
    : DProcessAlarm(channelIndex)
{
    myProcessIndex = E_PROCESS_SENSOR_ALARM_HI;

    //by default sensor high alarm is enabled
    myEnabledState = true;

    //the threshold is set to a very big number and expected to be set once the sensor fullscale/range is known
    myThreshold = BIG_POSITIVE_NUMBER;
}

/**
 * @brief   Run process
 * @param   input value
 * @retval  input value (unchanged)
 */
float32_t DProcessAlarmSensorHi::run(float32_t input)
{
    //only need to process if enabled
    if(myEnabledState == true)
    {
        //if at or above threshold value and not currently in alarm then set alarm state
        //else if below threshold value and currently in alarm then clear alarm state
        if((input >= myThreshold) && (myAlarmState == false))
        {
            myAlarmState = true;
            notify(E_UI_MSG_ALARM_SET);
        }

        else if((input < myThreshold) && (myAlarmState == true))
        {
            myAlarmState = false;
            notify(E_UI_MSG_ALARM_CLEAR);
        }

        else
        {
            //required to satisfy MISRA C rules
        }
    }

    return input;
}
