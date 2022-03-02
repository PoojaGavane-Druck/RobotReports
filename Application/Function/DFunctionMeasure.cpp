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
* @file      DFunctionMeasure.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The  DFunctionMeasure class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DFunctionMeasure.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief    DFunctionMeasure class constructor
 * @param   void
 * @retval  void
 */
DFunctionMeasure:: DFunctionMeasure()
    :  DFunction()
{
    //specify the flags that measure function must respond to in addition to the generic ones
    //note zero is normally applied to pressure, but included here in case other functions ever need it too
    myWaitFlags |= EV_FLAG_TASK_SENSOR_NEW_RANGE | EV_FLAG_TASK_SENSOR_ZERO_ERROR;
}

/**
 * @brief    DFunctionMeasure class constructor
 * @param   void
 * @retval  void
 */
DFunctionMeasure:: ~DFunctionMeasure()
{
}
/**
 * @brief   Validate function settings as retrieved from persistent storage
 * @note    Invalid/uninitialised data is set to defaults
 * @param   void
 * @retval  void
 */
void DFunctionMeasure::validateSettings(void)
{
    //get setting as retrieved from persistent storage
    getSettingsData();
#if 0

    //filter setting
    if(mySettings.filter.state == E_INIT_STATE_NOT_SET)
    {
        mySettings.filter.band = 1.0f;         //arbitrary band value
        mySettings.filter.timeConstant = 3.0f; //arbitrary value - on each sample, add 1/3 of error within band
        mySettings.filter.state = E_INIT_STATE_DISABLED;
    }


    //scaling setting
    if(mySettings.scaling.state == E_INIT_STATE_NOT_SET)
    {
        mySettings.scaling.coordinates[0] = { myNegFullscale, 0.0f };     //-ve FS = 0%
        mySettings.scaling.coordinates[1] = { myPosFullscale, 100.0f };   //+ve FS = 100%
        mySettings.scaling.state = E_INIT_STATE_DISABLED;
        mySettings.scaling.caption[0] = '%';
        mySettings.scaling.caption[1] = '\0';
    }

#endif
}

/**
 * @brief   Apply function settings as retrieved from persistent storage
 * @param   void
 * @retval  void
 */
void DFunctionMeasure::applySettings(void)
{
#ifdef PROCESS_ENABLED
    //sanity check the function settings
    validateSettings();

    //filter setting
    DProcess *process = processes[E_PROCESS_FILTER];

    if(mySettings.filter.state == E_INIT_STATE_ENABLED)
    {
        process->setParameter(mySettings.filter.band, 0u);
        process->setParameter(mySettings.filter.timeConstant, 1u);
        process->enable();
    }

    else
    {
        process->disable();
    }

    //alarm setting
    process = processes[E_PROCESS_USER_ALARM_HI];

    if(mySettings.alarm.state == E_INIT_STATE_ENABLED)
    {
        //turn on high alarm
        process->setParameter(mySettings.alarm.high);
        process->enable();

        //turn on low alarm too
        process = processes[E_PROCESS_USER_ALARM_LO];
        process->setParameter(mySettings.alarm.low);
        process->enable();
    }

    else
    {
        //turn off high alarm
        process->disable();

        //turn off low alarm too
        process = processes[E_PROCESS_USER_ALARM_LO];
        process->disable();
    }

    //sensor alarm is always on but need the thresholds setting
    process = processes[E_PROCESS_SENSOR_ALARM_HI];

    //turn on sensor high alarm
    process->setParameter(myPosFullscale);
    process->enable();

    //turn on sensor low alarm too
    process = processes[E_PROCESS_SENSOR_ALARM_LO];
    process->setParameter(myNegFullscale);
    process->enable();

//
//    //scaling setting
//    if (mySettings.scaling.state == E_INIT_STATE_ENABLED)
//    {
//        mySettings.scaling.coordinates[0] = { myNegFullscale, 0.0f };     //-ve FS = 0%
//        mySettings.scaling.coordinates[1] = { myPosFullscale, 100.0f };   //+ve FS = 100%
//        mySettings.scaling.state = E_INIT_STATE_DISABLED;
//        mySettings.scaling.caption = "%";
//    }
//    else
//    {
//        process->disable();
//    }

#endif
}
