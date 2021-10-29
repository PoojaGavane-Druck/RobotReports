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
* @file     DSlot.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlot class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <os.h>
#include <math.h>
MISRAC_ENABLE

#include "DSlot.h"
#include "memory.h"
#include "uart.h"
#include "DPV624.h"
#include "Utilities.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlot class constructor
 * @note    An owner is needed if sensor events are to be sent back. This is expected to be a DFunction instance.
 * @param   owner: the task that created this slot
 * @retval  void
 */
DSlot::DSlot(DTask *owner)
: DTask()
{
    OS_ERR os_error;

    myOwner = owner;

    //create mutex for resource locking
    char *name = "Slot";
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &os_error);

    if (os_error != (OS_ERR)OS_ERR_NONE)
    {
        //Error handler?
    }

    //specify the flags that this function must respond to (add more as necessary in derived class)
    myWaitFlags = EV_FLAG_TASK_SHUTDOWN | EV_FLAG_TASK_SENSOR_CONTINUE ;
    
    myAcqMode = (eAquisationMode_t)E_CONTINIOUS_ACQ_MODE;
}

/**
 * @brief   Get sensor
 * @param   void
 * @retval  pointer to this slot's sensor
 */
DSensor *DSlot::getSensor(void)
{
    return mySensor;
}

/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */
void DSlot::start(void)
{
    OS_ERR err;

    //get stack area from the memory partition memory block for function tasks
    myTaskStack = (CPU_STK*)OSMemGet((OS_MEM*)&memPartition, (OS_ERR*)&err);   
    
    if (err == (OS_ERR)OS_ERR_NONE)
    {
        
        //memory block from the partition obtained, so can go ahead and run
        activate(myName, (CPU_STK_SIZE)MEM_PARTITION_BLK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)10u, &err);
        
    }
    else
    {
        //report error
    }
}

/**
 * @brief   Run DSlot task funtion
 * @param   void
 * @retval  void
 */
void DSlot::runFunction(void)
{
    //this is a while loop that pends on event flags
    bool runFlag = true;
    uint32_t failCount = (uint32_t)0; //used for retrying in the event of failure
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
    eSensorError_t sensorError;
    eSensorStatus_t sensorCommStatus = E_SENSOR_STATUS_RUNNING;
    sensorError = mySensor->initialise();

    //sensor is immediately ready
    myState = E_SENSOR_STATUS_READY;

    //notify parent that we have connected, awaiting next action - this is to allow
    //the higher level to decide what other initialisation/registration may be required
    myOwner->postEvent(EV_FLAG_TASK_SENSOR_CONNECT);

    while (runFlag == true)
    {
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)500u, //runs, nominally, at 2Hz by default
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
        //check for events
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {

            //only have something to do if in running state
            if (myState == E_SENSOR_STATUS_RUNNING)
            {
                //take measurement and post event
                sensorError = mySensor->measure();

                //if no sensor error than proceed as normal (errors will be mopped up below)
                if (sensorError == E_SENSOR_ERROR_NONE)
                {
                  failCount = 0u;
                   // myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
                  myOwner->postEvent(EV_FLAG_TASK_NEW_BARO_VALUE);
                }
                else
                {
                  failCount++;
                }
                if ((failCount > 3u) && (sensorCommStatus != E_SENSOR_STATUS_DISCONNECTED))
                {
                   
                    sensorCommStatus = E_SENSOR_STATUS_DISCONNECTED;
                    //notify parent that we have hit a problem and are awaiting next action from higher level functions
                    myOwner->postEvent(EV_FLAG_TASK_BARO_SENSOR_DISCONNECT);
                    
                }
                if ((failCount == 0u) && (sensorCommStatus == E_SENSOR_STATUS_DISCONNECTED))
                {
                  sensorCommStatus = E_SENSOR_STATUS_RUNNING;
                   
                  myOwner->postEvent(EV_FLAG_TASK_BARO_SENSOR_CONNECT);
                }
            }

        }
        else if (os_error != (OS_ERR)OS_ERR_NONE)
        {
            //os error
            sensorError = E_SENSOR_ERROR_OS;
        }
        else
        {
            if ((actualEvents & EV_FLAG_TASK_SHUTDOWN) == EV_FLAG_TASK_SHUTDOWN)
            {
                runFlag = false;
            }
            else
            {
                //check events that can occur at any time first
                switch (myState)
                {
                    case E_SENSOR_STATUS_READY:
                        //waiting to be told to start running
                        if ((actualEvents & EV_FLAG_TASK_SLOT_SENSOR_CONTINUE) == EV_FLAG_TASK_SLOT_SENSOR_CONTINUE)
                        {
                            myState = E_SENSOR_STATUS_RUNNING;
                        }
                        break;
                    case E_SENSOR_STATUS_RUNNING:
                      if ((actualEvents & EV_FLAG_TASK_SENSOR_TAKE_NEW_READING) == EV_FLAG_TASK_SENSOR_TAKE_NEW_READING)
                      {
                              //take measurement and post event
                            sensorError = mySensor->measure();

                            //if no sensor error than proceed as normal (errors will be mopped up below)
                            if (sensorError == E_SENSOR_ERROR_NONE)
                            {
                                failCount = 0u;
                                myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
                            }
                            else
                            {
                              failCount++;
                            }
                            if ((failCount > 3u) && (sensorCommStatus != E_SENSOR_STATUS_DISCONNECTED))
                            {
                               
                                sensorCommStatus = E_SENSOR_STATUS_DISCONNECTED;
                                //notify parent that we have hit a problem and are awaiting next action from higher level functions
                                myOwner->postEvent(EV_FLAG_TASK_BARO_SENSOR_DISCONNECT);
                                
                            }
                            if ((failCount == 0u) && (sensorCommStatus == E_SENSOR_STATUS_DISCONNECTED))
                            {
                              sensorCommStatus = E_SENSOR_STATUS_RUNNING;
                               
                              myOwner->postEvent(EV_FLAG_TASK_BARO_SENSOR_CONNECT);
                            }
                            
                        
                      }
					  if (handleCalibrationEvents(actualEvents) != E_SENSOR_ERROR_NONE)
                      {
                        sensorError = E_SENSOR_ERROR_CAL_COMMAND;
                      }
                        break;

                    default:
                        break;
                }
            }
        }

        //handle reported sensor status changes and set/clear any errors from sensor functions
        #ifdef CONTROLLER_BOARD
		updateSensorStatus(sensorError);
        #endif
    }

    sensorError = mySensor->close();
   
    if (sensorError != E_SENSOR_ERROR_NONE)
    {
        myOwner->postEvent(EV_FLAG_TASK_SENSOR_FAULT);
    }
}

/**
 * @brief   handle sensor error
 * @param   sensor error
 * @return  void
 */
void DSlot::updateSensorStatus(eSensorError_t sensorError)
{
    //sensor status
    sSensorStatus_t statusChanges;

    //Unspecified error the sensor itself may not be able to signal
    if (sensorError != E_SENSOR_ERROR_NONE)
    {
        myOwner->postEvent(EV_FLAG_TASK_SENSOR_FAULT);
    }

    //check if status has changed since last read (re-using local variable to hold changes in status)
    statusChanges = mySensor->getStatusChanges();

    //if one or more elements have changed then need to signal the event
    if (statusChanges.value != 0u)
    {
        if (statusChanges.fault == 1u) //sensor not working properly, eg comms timeout or command error
        {
            myOwner->postEvent(EV_FLAG_TASK_SENSOR_FAULT);
        }

        if (statusChanges.rangeChanged == 1u) //range has auto-changed
        {
            myOwner->postEvent(EV_FLAG_TASK_SENSOR_NEW_RANGE);
        }

        if (statusChanges.inLimit == 1u) //measured vaue is with in tolerance limit of the setpoint
        {
            myOwner->postEvent(EV_FLAG_TASK_SENSOR_IN_LIMIT);
        }

        if (statusChanges.zeroError == 1u) //zero attempt was unsuccessful
        {
            myOwner->postEvent(EV_FLAG_TASK_SENSOR_ZERO_ERROR);
        }

        if (statusChanges.calRejected == 1u) //calibration rejected
        {
            myOwner->postEvent(EV_FLAG_TASK_SENSOR_CAL_REJECTED);
        }

        if (statusChanges.calDefault == 1u) //status change in default cal being used
        {
            myOwner->postEvent(EV_FLAG_TASK_SENSOR_CAL_DEFAULT);
        }

        if (statusChanges.calOverdue == 1u) //calibration overdue status changes
        {
            myOwner->postEvent(EV_FLAG_TASK_SENSOR_CAL_DUE);
        }

        if (statusChanges.calDateCheck == 1u) //status change in calibration date invalid
        {
            myOwner->postEvent(EV_FLAG_TASK_SENSOR_CAL_DATE);
        }
    }
}

/**
 * @brief   Clean up after termination of task
 * @param   void
 * @return  void
 */
void DSlot::cleanUp(void)
{
    OS_ERR err;

    //if a stack was allocated then free that memory to the partition
    if (myTaskStack != NULL)
    {
        //Return the stack memory block back to the partition
        OSMemPut((OS_MEM*)&memPartition, (void*)myTaskStack, (OS_ERR*)&err);

        if (err == (OS_ERR)OS_ERR_NONE)
        {
            //memory block from the partition obtained
        }

        myTaskStack = NULL;
    }
}

/**
 * @brief   Set output for this slot
 * @note    This is used by source slots to update setpoint
 * @param   index is slot specific
 * @param   value is the output setpoint
 * @return  true if successful, false if fails or no valid operation
 */
bool DSlot::setOutput(uint32_t index, float32_t value)
{
    return false;
}

/**
 *  @brief  Pause slot task
 *  @note   Used to pause normal operation and wait for next action; continue or synchronised re-start
 *  @param  void
 *  @return void
 */
void DSlot::pause(void)
{
    postEvent(EV_FLAG_TASK_SENSOR_PAUSE);
}

/**
 *  @brief  Continue/resume slot task
 *  @note   Assert go-ahead signal for sensor to resume after a pause
 *  @param  void
 *  @return void
 */
void DSlot::resume(void)
{
    postEvent(EV_FLAG_TASK_SLOT_SENSOR_CONTINUE);
}

/**
 *  @brief  Synchronised restart of slot task
 *  @note   Assert signal for a synchronised measurement (used after a pause) for example when multiple slots
 *          are required to get measurements "as simultaneously as possible"
 *  @param  void
 *  @return void
 */
void DSlot::synchronise(void)
{
    postEvent(EV_FLAG_TASK_SLOT_SYNCHRONISE);
}

/**
 *  @brief  Retry slot task
 *  @note   Assert go-ahead signal for sensor to re-start/retry after, for example after a fault
 *  @param  void
 *  @return void
 */
void DSlot::retry(void)
{
    postEvent(EV_FLAG_TASK_SLOT_SENSOR_RETRY);
}


/**
 * @brief   Get floating point value
 * @param   index is function/sensor specific value identifier
 * @param   pointer to variable for return value
 * @return  true if successful, else false
 */
bool DSlot::getValue(eValueIndex_t index, float32_t *value)
{
    return mySensor->getValue(index, value);
}

/**
 * @brief   Set floating point value
 * @param   index is function/sensor specific value identifier
 * @param   value to set
 * @return  true if successful, else false
 */
bool DSlot::setValue(eValueIndex_t index, float32_t value)
{
    return mySensor->setValue(index, value);
}

/**
 * @brief   get string 
 * @param   index is function/sensor specific value identifier
 * @param   pointer to variable for return value
 * @return  true if successful, else false
 */
bool DSlot::getValue(eValueIndex_t index, char *value)
{       
    bool flag = true;

    DLock is_on(&myMutex);
    
    switch(index)
    {
    case E_VAL_INDEX_SENSOR_BRAND_UNITS:
        mySensor->getBrandUnits(value);
        break;
        
    default:
          break;
    }
    
    return flag;
}
  
/**
 * @brief   Get integer value
 * @param   index is function/sensor specific value identifier
 * @param   pointer to variable for return value of integer attribute
 * @return  true if successful, else false
 */
bool DSlot::getValue(eValueIndex_t index, uint32_t *value)
{
    bool flag = true;

    DLock is_on(&myMutex);

    switch (index)
    {
        case E_VAL_INDEX_SYNCH_TIME:
            *value = mySyncTime;
            break;

        case E_VAL_INDEX_SAMPLE_TIME:
            *value = mySampleTime;
            break;

        case E_VAL_INDEX_CAL_TYPE:
            *value = myCalType;
            break;

        case E_VAL_INDEX_CAL_RANGE:
            *value = myCalRange;
            break;

        case E_VAL_INDEX_CAL_POINT:
            *value = myCalPointIndex;
            break;

        case E_VAL_INDEX_CAL_SAMPLE_COUNT:
            *value = myCalSamplesRemaining;
            break;

        default:
            if (mySensor != NULL)
            {
                flag = mySensor->getValue(index, value);
            }
            break;
    }
    return flag;

}

/**
 * @brief   Set integer value
 * @param   index is function/sensor specific value identifier
 * @param   value to set for integer attribute
 * @return  true if successful, else false
 */
bool DSlot::setValue(eValueIndex_t index, uint32_t value)
{
        bool flag = true;

    DLock is_on(&myMutex);

    switch (index)
    {
        case E_VAL_INDEX_SYNCH_TIME:
            mySyncTime = value;
            break;

        case E_VAL_INDEX_SAMPLE_TIME:
            mySampleTime = value;
            break;

        case E_VAL_INDEX_CAL_TYPE:
            myCalType = value;
            break;

        case E_VAL_INDEX_CAL_RANGE:
            myCalRange = value;
            break;

        case E_VAL_INDEX_CAL_POINT:
            myCalPointIndex = value;
            break;

        case E_VAL_INDEX_CAL_SAMPLE_COUNT:
            myCalSamplesRemaining = value;
            break;

        default:
            if (mySensor != NULL)
            {
                flag = mySensor->setValue(index, value);
            }
            break;

    }

    return flag;

}

/**
 * @brief   Get date value
 * @param   index is function/sensor specific value identifier
 * @param   pointer to variable for return value of date attribute
 * @return  true if successful, else false
 */
bool DSlot::getValue(eValueIndex_t index, sDate_t *date)
{
    return mySensor->getValue(index, date);
}

/**
 * @brief   Set calibration type
 * @param   calType - function specific calibration type (0 = user calibration)
 * @param   range - sensor range
 * @retval  true = success, false = failed
 */
bool DSlot::setCalibrationType(int32_t calType, uint32_t range)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        uint32_t numCalPoints;
        getRequiredNumCalPoints(&numCalPoints);

        //must have non-zero calibration points for it to be calibratable
        if (numCalPoints > 0u)
        {
            mySensor->setMode(E_SENSOR_MODE_CALIBRATION);

            sSensorStatus_t sensorStatus = mySensor->getStatus();

            if (sensorStatus.canSetCalType == 1u)
            {
                //update cal point index
                flag = setValue(E_VAL_INDEX_CAL_TYPE, (uint32_t)calType);

                if (flag == true)
                {
                    //update cal point value
                    flag = setValue(E_VAL_INDEX_CAL_RANGE, range);

                    if (flag == true)
                    {
                        //post event flag to slot to set the calibration type
                        postEvent(EV_FLAG_TASK_SLOT_CAL_SET_TYPE);
                    }
                }
            }
        }
    }

    return flag;
}


/**
 * @brief   Set sample interval
 * @param   interval is the period value in ms (parameter value 0 is interpreted as default)
 * @retval  flag: true if successfully set, else false
 */
bool DSlot::setSampleInterval(uint32_t interval)
{
    bool flag = true;

    DLock is_on(&myMutex);

    if (interval == 0u)
    {
        mySampleInterval = myDefaultSampleInterval;
    }
    else
    {
        if (interval < myMinSampleInterval) // && (interval > myMaxSampleInterval)
        {
            flag = false;
        }
        else
        {
            mySampleInterval = (OS_TICK)interval;
        }
    }

    return flag;
}

/**
 * @brief   Set sensor calibration type
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSlot::sensorSetCalibrationType(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        int32_t calType;

        //update cal point index
        flag = getValue(E_VAL_INDEX_CAL_TYPE, (uint32_t *)&calType);

        if (flag == true)
        {
            uint32_t range;

            //update cal point value
            flag = getValue(E_VAL_INDEX_CAL_RANGE, &range);

            if (flag == true)
            {
                flag = mySensor->setCalibrationType(calType);
            }
        }
    }

    return flag;
}

/**
 * @brief   Get required number of calibration points
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSlot::getRequiredNumCalPoints(uint32_t *numCalPoints)
{
    bool flag = false;
    *numCalPoints = 0u;

    if (mySensor != NULL)
    {
        flag = mySensor->getRequiredNumCalPoints(numCalPoints);
    }

    return flag;
}



/**
 * @brief   set required number of calibration points
 * @param   uint32_t number of cal points
 * @retval  true = success, false = failed
 */
bool DSlot::setRequiredNumCalPoints(uint32_t numCalPoints)
{
    bool flag = false;
    
    if (mySensor != NULL)
    {
        flag = mySensor->setRequiredNumCalPoints(numCalPoints);
    }

    return flag;
}
/**
 * @brief   Start sampling at current cal point
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSlot::startCalSampling(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        eSensorMode_t sensorMode = mySensor->getMode();

        if (sensorMode == (eSensorMode_t)E_SENSOR_MODE_CALIBRATION)
        {
            sSensorStatus_t sensorStatus = mySensor->getStatus();

            if (sensorStatus.canStartSampling == 1u)
            {
                postEvent(EV_FLAG_TASK_SLOT_CAL_START_SAMPLING);
                flag = true;
            }
        }
    }

    return flag;
}

/**
 * @brief   Start sensor sampling at current cal point
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSlot::sensorStartCalSampling(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        flag = mySensor->startCalSampling();
    }

    return flag;
}

/**
 * @brief   Get remaining number of samples at current cal point
 * @param   pointer to variable for return value of remaining number of samples
 * @retval  true = success, false = failed
 */
bool DSlot::getCalSamplesRemaining(uint32_t *samples)
{
    bool flag = false;
    *samples = 0u;

    //base class assumes there is no cummunication with external sensor, so can get the value directly

    if (mySensor != NULL)
    {
        eSensorMode_t sensorMode = mySensor->getMode();

        if (sensorMode == (eSensorMode_t)E_SENSOR_MODE_CALIBRATION)
        {
            sSensorStatus_t sensorStatus = mySensor->getStatus();

            if (sensorStatus.canQuerySampling == 1u)
            {
                flag = sensorGetCalSamplesRemaining();

                if (flag == true)
                {
                    //get cal samples remaining count, which should havbe been updated by the call above
                    flag = getValue(E_VAL_INDEX_CAL_SAMPLE_COUNT, samples);
                }
            }
        }
    }

    return flag;
}

/**
 * @brief   Get remaining number of samples at current cal point
 * @param   pointer to variable for return value of remaining number of samples
 * @retval  true = success, false = failed
 */
bool DSlot::sensorGetCalSamplesRemaining(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        uint32_t samples = 1u; //arbitrary non-zero value (could be any value except 0 to indicate sampling has not finished)

        flag = mySensor->getCalSamplesRemaining(&samples);

        if (flag == true)
        {
            //update cal samples remaining count
            flag = setValue(E_VAL_INDEX_CAL_SAMPLE_COUNT, samples);
        }
    }

    return flag;
}

/**
 * @brief   Set calibration point
 * @param   point indicates the cal point number (1 - required no of points)
 * @param   user supplied calibration value
 * @retval  true = success, false = failed
 */
bool DSlot::setCalPoint(uint32_t calPoint, float32_t value)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        eSensorMode_t sensorMode = mySensor->getMode();

        //Cal point must not exceed 10% of sensor fullscale
        float32_t posLimit = 0.0f;
        float32_t negLimit = 0.0f;

        mySensor->getValue(E_VAL_INDEX_POS_FS_ABS, &posLimit);
        mySensor->getValue(E_VAL_INDEX_POS_FS_ABS, &negLimit);

        //modify limits by 10% of pos FS
        negLimit -= 0.1f * fabs(posLimit);
        posLimit *= 1.1f;

        //check against cal point limits
        if ((value >= negLimit) || (value <= posLimit))
        {
            if (sensorMode == (eSensorMode_t)E_SENSOR_MODE_CALIBRATION)
            {
                sSensorStatus_t sensorStatus = mySensor->getStatus();

                if (sensorStatus.canSetCalPoint == 1u)
                {
                    //update cal point index
                    flag = setValue(E_VAL_INDEX_CAL_POINT, calPoint);

                    if (flag == true)
                    {
                        //update cal point value
                        flag = setValue(E_VAL_INDEX_CAL_POINT_VALUE, value);

                        if (flag == true)
                        {
                            //post event flag to slot to process the cal point
                            postEvent(EV_FLAG_TASK_SLOT_CAL_SET_POINT);
                        }
                    }
                }
            }
        }
    }

    return flag;
}

/**
 * @brief   Set calibration point
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSlot::sensorSetCalPoint(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        uint32_t calPoint;

        //get cal point index
        flag = getValue(E_VAL_INDEX_CAL_POINT, &calPoint);

        if (flag == true)
        {
            float32_t value;

            //get cal point value
            flag = getValue(E_VAL_INDEX_CAL_POINT_VALUE, &value);

            if (flag == true)
            {
                flag = mySensor->setCalPoint(calPoint, value);
            }
        }
    }

    return true;
}

/**
 * @brief   Cal accept
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSlot::acceptCalibration(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        eSensorMode_t sensorMode = mySensor->getMode();

        if (sensorMode == (eSensorMode_t)E_SENSOR_MODE_CALIBRATION)
        {
            sSensorStatus_t sensorStatus = mySensor->getStatus();

            if (sensorStatus.canAcceptCal == 1u)
            {
                postEvent(EV_FLAG_TASK_SLOT_CAL_ACCEPT);
                flag = true;
            }
        }
    }

    return flag;
}

/**
 * @brief   Cal sensor accept
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSlot::sensorAcceptCalibration(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        flag = mySensor->acceptCalibration();

    }

    return flag;
}

/**
 * @brief   Abort calibration
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSlot::abortCalibration(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        eSensorMode_t sensorMode = mySensor->getMode();

        if (sensorMode == (eSensorMode_t)E_SENSOR_MODE_CALIBRATION)
        {
            sSensorStatus_t sensorStatus = mySensor->getStatus();

            if (sensorStatus.canAbortCal == 1u)
            {
                postEvent(EV_FLAG_TASK_SLOT_CAL_ABORT);
                flag = true;
            }
        }
    }

    return flag;
}

/**
 * @brief   Abort calibration
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSlot::sensorAbortCalibration(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        flag = mySensor->abortCalibration();
    }

    return flag;
}


/**
 * @brief   Handle calibration related events
 * @param   actual event flags
 * @retval  sensor error status
 */
eSensorError_t DSlot::handleCalibrationEvents(OS_FLAGS actualEvents)
{
    eSensorError_t sensorError = (eSensorError_t)E_SENSOR_ERROR_NONE;

    if ((actualEvents & EV_FLAG_TASK_SLOT_CAL_SET_TYPE) == EV_FLAG_TASK_SLOT_CAL_SET_TYPE)
    {
        if (sensorSetCalibrationType() == false)
        {
            sensorError = E_SENSOR_ERROR_CAL_COMMAND;
        }
        else
        {
              PV624->handleError(E_ERROR_BAROMETER_SENSOR_MODE, 
                                 eSetError,
                                 (uint32_t)1,
                                 (uint16_t)32);
            
            //we can start sampling at the cal mode sample rate
            setSampleInterval(myCalSampleInterval);
        }
    }

    if ((actualEvents & EV_FLAG_TASK_SLOT_CAL_START_SAMPLING) == EV_FLAG_TASK_SLOT_CAL_START_SAMPLING)
    {
        if (sensorStartCalSampling() == false)
        {
            sensorError = E_SENSOR_ERROR_CAL_COMMAND;
        }
    }

    if ((actualEvents & EV_FLAG_TASK_SLOT_CAL_SET_POINT) == EV_FLAG_TASK_SLOT_CAL_SET_POINT)
    {
        if (sensorSetCalPoint() == false)
        {
            sensorError = E_SENSOR_ERROR_CAL_COMMAND;
        }
    }

    if ((actualEvents & EV_FLAG_TASK_SLOT_CAL_ACCEPT) == EV_FLAG_TASK_SLOT_CAL_ACCEPT)
    {
        if (sensorAcceptCalibration() == false)
        {
            sensorError = E_SENSOR_ERROR_CAL_COMMAND;
        }
        else
        {
           
            
            PV624->handleError(E_ERROR_BAROMETER_SENSOR_CAL_STATUS, 
                                 eSetError,
                                 (uint32_t)1,
                                 (uint16_t)33);
              
            
            PV624->handleError(E_ERROR_BAROMETER_SENSOR_MODE, 
                                 eClearError,
                                 (uint32_t)0,
                                 (uint16_t)34);
        
            //we can revert to default sampling rate on exiting cal mode
            setSampleInterval(myDefaultSampleInterval);
        }
    }

    if ((actualEvents & EV_FLAG_TASK_SLOT_CAL_ABORT) == EV_FLAG_TASK_SLOT_CAL_ABORT)
    {
        if (sensorAbortCalibration() == false)
        {
            sensorError = E_SENSOR_ERROR_CAL_COMMAND;
        }
        else
        {
            //we can revert to default sampling rate on exiting cal mode
                       
            
            PV624->handleError(E_ERROR_BAROMETER_SENSOR_CAL_STATUS, 
                                 eClearError,
                                 (uint32_t)0,
                                 (uint16_t)35);
              
            PV624->handleError(E_ERROR_BAROMETER_SENSOR_MODE, 
                                 eClearError,
                                 (uint32_t)0,
                                 (uint16_t)36);
            
            setSampleInterval(myDefaultSampleInterval);
        }
    }

    if ((actualEvents & EV_FLAG_TASK_SLOT_CAL_SET_DATE) == EV_FLAG_TASK_SLOT_CAL_SET_DATE)
    {
        if (sensorSetCalDate() == false)
        {
            sensorError = E_SENSOR_ERROR_CAL_COMMAND;
        }
    }
    
    if ((actualEvents & EV_FLAG_TASK_SLOT_CAL_SET_INTERVAL) == EV_FLAG_TASK_SLOT_CAL_SET_INTERVAL)
    {
        if (sensorSetCalInterval() == false)
        {
            sensorError = E_SENSOR_ERROR_CAL_COMMAND;
        }
    }
    
    if ((actualEvents & EV_FLAG_TASK_CAL_SAMPLES_COUNT) == EV_FLAG_TASK_CAL_SAMPLES_COUNT)
    {
        if (sensorGetCalSamplesRemaining() == false)
        {
            sensorError = E_SENSOR_ERROR_CAL_COMMAND;
        }
    }

    if ((actualEvents & EV_FLAG_TASK_SLOT_CAL_RELOAD) == EV_FLAG_TASK_SLOT_CAL_RELOAD)
    {
        if (sensorReloadCalibration() == false)
        {
            sensorError = E_SENSOR_ERROR_CAL_COMMAND;
        }
    }

    return sensorError;
}

/**
 * @brief   Post reload calibration data event flag
 * @param   void
 * @retval  flag: true = success, false = failed
 */
bool DSlot::reloadCalibration(void)
{
    postEvent(EV_FLAG_TASK_SLOT_CAL_RELOAD);
    return true;
}

/**
 * @brief   Reload sensor calibration data
 * @param   void
 * @retval  flag: true = success, false = failed
 */
bool DSlot::sensorReloadCalibration(void)
{
    bool flag = true; //if no sensor then return true (ie, 'not required')

    if (mySensor != NULL)
    {
        flag = mySensor->loadCalibrationData();
    }

    return flag;
}
/**
 * @brief   Get cal date variables
 * @param   pointer to date structure
 * @retval  void
 */
void DSlot::getCalDateVariable(sDate_t *date)
{
    DLock is_on(&myMutex);
    date->day = myCalDate.day;
    date->month = myCalDate.month;
    date->year = myCalDate.year;
}

/**
 * @brief   Set cal date variables
 * @param   pointer to date structure
 * @retval  void
 */
void DSlot::setCalDateVariable(sDate_t *date)
{
    DLock is_on(&myMutex);
    myCalDate.day = date->day;
    myCalDate.month = date->month;
    myCalDate.year = date->year;
}
/**
 * @brief   get calibration date
 * @param   range
 * @param   pointer to variable for return value
 * @retval  void
 */
bool DSlot::getCalDate(sDate_t* date)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        flag = mySensor->getCalDate(date);
    }

    return flag;
}
/**
 * @brief   Set calibration date
 * @param   range
 * @param   pointer to date of calibration
 * @retval  void
 */
bool DSlot::setCalDate( sDate_t *date)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        //make sure the date is valid
        if (isDateValid(date->day, date->month, date->year) == true)
        {
            setCalDateVariable(date);

            //post event flag to slot to process the cal date setting
            postEvent(EV_FLAG_TASK_SLOT_CAL_SET_DATE);

            flag = true;
        }
    }

    return flag;
}

/**
 * @brief   Set calibration date
 * @param   void
 * @retval  void
 */
bool DSlot::sensorSetCalDate(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        sDate_t date;
        getCalDateVariable(&date);
        flag = mySensor->setCalDate(&date);
    }

    return flag;
}

/**
 * @brief   Set cal interval variable
 * @param   interval value to set
 * @retval  void
 */
void DSlot::setCalIntervalVariable(uint32_t interval)
{
    DLock is_on(&myMutex);
    myCalInterval = interval;
}

/**
 * @brief   Set calibration interval (in number of days)
 * @param   interval value
 * @retval  void
 */
bool DSlot::setCalInterval(uint32_t interval)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        if ((interval >= MIN_CAL_INTERVAL) && (interval <= MAX_CAL_INTERVAL))
        {
            setCalIntervalVariable(interval);

            //post event flag to slot to process the cal interval setting
            postEvent(EV_FLAG_TASK_SLOT_CAL_SET_INTERVAL);

            flag = true;
        }
    }

    return flag;
}

/**
 * @brief   Get cal interval variable
 * @param   void
 * @retval  interval value
 */
uint32_t DSlot::getCalIntervalVariable(void)
{
    DLock is_on(&myMutex);
    return myCalInterval;
}


/**
 * @brief   Set sensor calibration interval
 * @param   void
 * @retval  void
 */
bool DSlot::sensorSetCalInterval(void)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        uint32_t interval = getCalIntervalVariable();
        flag = mySensor->setCalInterval(interval);
    }

    return flag;
}
/**
 * @brief   Get calibration interval (in number of days)
 * @param   pointer to variable for return value
 * @retval  sensor error code
 */
bool DSlot::getCalInterval(uint32_t *interval)
{
    bool flag = false;

    if (mySensor != NULL)
    {
        flag = mySensor->getCalInterval(interval);
    }

    return flag;
}

/**
 * @brief   Sets aquisation mode
 * @param   newAcqMode : new Aquisation mode
 * @retval  void
 */
void DSlot::setAquisationMode(eAquisationMode_t newAcqMode)
{
  DLock is_on(&myMutex);
  myAcqMode = newAcqMode;
}

/**
 * @brief   Gets aquisation mode
 * @param   newAcqMode : new Aquisation mode
 * @retval  newAcqMode
 */
eAquisationMode_t DSlot::getAquisationMode(void)
{
  DLock is_on(&myMutex);
  return myAcqMode;
}