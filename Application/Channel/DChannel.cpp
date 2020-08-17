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
* @file     DChannel.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DChannel class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <stdint.h>
MISRAC_ENABLE

#include "DChannel.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DChannel class constructor
 * @param   void
 * @retval  void
 */
DChannel::DChannel()
{
    //clear the function arrays
    for (int32_t i = 0; i < (int32_t)E_FUNCTION_MAX; i++)
    {
        myMeasureFunctions[i] = NULL;        
    }

    //nothing running yet
    myCurrentFunction = NULL;
}

/**
 * @brief   Get instrument function
 * @param   func is pointer to function (for return value)
 * @param   value is the output setpoint (for return value)
 * @return  true if valid function, false means no function selected
 */
bool DChannel::getFunction(eFunction_t *func, eFunctionDir_t *dir)
{
    bool flag = false;

    if (myCurrentFunction != NULL)
    {
        *func = myCurrentFunction->myFunction;
        *dir  = myCurrentFunction->myDirection;
        flag = true;
    }
    else
    {
        //no function set
        *func = (eFunction_t)E_FUNCTION_NONE;
        *dir  = (eFunctionDir_t)E_FUNCTION_DIR_MEASURE;
    }

    return flag;
}

/**
 * @brief   Set Instrument function
 * @param   func is the function itself
 * @param   dir is the measure/source specification
 * @retval  true if activated successfully, else false
 */
bool DChannel::setFunction(eFunction_t func, eFunctionDir_t dir)
{
    bool successFlag = true;
    DFunction* requestedFunction = NULL;

    switch (dir)
    {
        case E_FUNCTION_DIR_MEASURE:
            requestedFunction = myMeasureFunctions[func];
            break;



        default:
            successFlag = false;
            break;
    }

    //check what needs doing, if anything
    if (successFlag == true)
    {
        //function at the specified index is NULL, so check if this is a real request to select 'no function'
        if ((requestedFunction == NULL) && (func != (eFunction_t)E_FUNCTION_NONE))
        {
            successFlag = false; //error because asked for a function not valid for this channel
        }

        //if still ok to proceed
        if (successFlag == true)
        {
            if (myCurrentFunction != requestedFunction)
            {
                //if there is a function running already then shut it down first
                if (myCurrentFunction != NULL)
                {
                    //gracefully shutdown the currently running task
                    myCurrentFunction->shutdown();
                }

                //make requested function the new current function
                myCurrentFunction = requestedFunction;

                //if the new function is not 'none' then start it
                if (myCurrentFunction != NULL)
                {
                    //start the new function
                    myCurrentFunction->start();
                }
            }
        }
    }

    return successFlag;
}

/**
 * @brief   Query whether function is running and ready for further commands
 * @return  true if ready, else false
 */
bool DChannel::queryFunctionReady(void)
{
    bool flag = true;

    if (myCurrentFunction != NULL)
    {
        if (myCurrentFunction->getState() != (eTaskState_t)E_TASK_STATE_RUNNING)
        {
            flag = false;
        }
    }

    return flag;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
eUnits_t DChannel::getUnits(void)
{
    return (eUnits_t)0;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::setUnits(eUnits_t units)
{
    return true;
}

/**
 * @brief   Get output for this function
 * @param   index is function specific
 * @param   pointer for return value of output setpoint
 * @return  true if successful, else false
 */
bool DChannel::getOutput(uint32_t index, float32_t *value)
{
    bool success = false;

    if (myCurrentFunction != NULL)
    {
        success = myCurrentFunction->getOutput(index, value);
    }

    return success;
}

/**
 * @brief   Set output for this slot
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @return  true if successful, else false
 */
bool DChannel::setOutput(uint32_t index, float32_t value)
{
    bool success = false;

    if (myCurrentFunction != NULL)
    {
        success = myCurrentFunction->setOutput(index, value);
    }

    return success;
}

/**
 * @brief   Get Channel Function Sensor Value
 * @param   index is function specific
 * @param   pointer to variable for return value of compensated and processed measurement value in selects user units
 * @return  true if successful, else false
 */
bool DChannel::getFunctionValue(uint32_t index, float32_t *value)
{
    bool success = false;

    if (myCurrentFunction != NULL)
    {
        success = myCurrentFunction->getValue(index, value);
    }

    return success;
}

/**
 * @brief   Signal sensor to continue
 * @param   void
 * @retval  true if all's well, else false
 */
bool DChannel::sensorContinue(void)
{
    bool success = false;

    if (myCurrentFunction != NULL)
    {
        success = myCurrentFunction->sensorContinue();
    }

    return success;
}

/**
 * @brief   Signal sensor to retry after failure reported
 * @param   void
 * @retval  true if all's well, else false
 */
bool DChannel::sensorRetry(void)
{
    bool success = false;

    if (myCurrentFunction != NULL)
    {
        success = myCurrentFunction->sensorRetry();
    }

    return success;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
eSensorType_t DChannel::getSensorType(void)
{
    return (eSensorType_t)E_SENSOR_TYPE_GENERIC;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::getProcessEnabled(eProcess_t process)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::setProcessEnabled(eProcess_t process, bool state)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
void DChannel::resetProcess(eProcess_t process)
{
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::queryOutputInTolerance(void)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::getSwitchState(void)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
void DChannel::resetSwitchTest(void)
{
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
float32_t DChannel::getSwitchOpenAt(void)
{
    return 0.0f;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
float32_t DChannel::getSwitchClosedAt(void)
{
    return 0.0f;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
float32_t DChannel::getSwitchHysteresis(void)
{
    return 0.0f;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
void DChannel::getMaxMin(float32_t *max, float32_t *min)
{
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::performZero(void)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::getCalMode(void)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::setCalMode(void)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::abortCalMode(void)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::setCalType(eCalType_t calType)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::startCalSampling(void)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::queryCalSamplingStatus(void)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::setCalPoint(uint32_t point, float32_t value)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::getCalDate(uint32_t range, sDate_t *date)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::setCalDate(uint32_t range, sDate_t date)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::getCalData(uint32_t range, sCalRange_t *calData)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
bool DChannel::getCalInterval(uint32_t range, sCalRange_t *calData)
{
    return true;
}

/**
 * @brief   //TODO: XXXXXXXX
 * @note    This is used by source slots to update setpoint
 * @param   index is function specific
 * @param   value is the output setpoint
 * @param   value is the output setpoint
 * @return  true if ok, else false
 */
uint32_t DChannel::getNumCalPoints(uint32_t range)
{
    return 0u;
}
/**
 * @brief   Get positive fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DChannel::getPosFullscale(float32_t *fs)
{
    bool success = false;

    if (myCurrentFunction != NULL)
    {
        success = myCurrentFunction->getValue((uint32_t)E_VAL_INDEX_POS_FS, fs);
    }

    return success;
}
/**
 * @brief   Get positive fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DChannel::getNegFullscale(float32_t *fs)
{
    bool success = false;

    if (myCurrentFunction != NULL)
    {
        success = myCurrentFunction->getValue((uint32_t)E_VAL_INDEX_NEG_FS, fs);
    }

    return success;
}


bool DChannel::getUserCalDate(sDate_t* caldate)
{
   bool success = false;

    if (myCurrentFunction != NULL)
    {
        myCurrentFunction->getCalDate((eSensorCalType_t)E_SENSOR_CAL_TYPE_USER, caldate);
        success = true;
    }

    return success;
}

bool DChannel::getManufactureDate( sDate_t *manfDate)
{
   bool success = false;

    if (myCurrentFunction != NULL)
    {
        myCurrentFunction->getManufactureDate(manfDate);
        success = true;
    }

    return success;
}