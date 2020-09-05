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

MISRAC_ENABLE

#include "DSlot.h"
#include "memory.h"
#include "uart.h"

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
    myWaitFlags = EV_FLAG_TASK_SHUTDOWN | EV_FLAG_TASK_SENSOR_CONTINUE;
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
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
    eSensorError_t sensorError;

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
                    myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
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
                        if ((actualEvents & EV_FLAG_TASK_SENSOR_CONTINUE) == EV_FLAG_TASK_SENSOR_CONTINUE)
                        {
                            myState = E_SENSOR_STATUS_RUNNING;
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
    postEvent(EV_FLAG_TASK_SENSOR_CONTINUE);
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
    postEvent(EV_FLAG_TASK_SENSOR_SYNC);
}

/**
 *  @brief  Retry slot task
 *  @note   Assert go-ahead signal for sensor to re-start/retry after, for example after a fault
 *  @param  void
 *  @return void
 */
void DSlot::retry(void)
{
    postEvent(EV_FLAG_TASK_SENSOR_RETRY);
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
 * @brief   Get integer value
 * @param   index is function/sensor specific value identifier
 * @param   pointer to variable for return value of integer attribute
 * @return  true if successful, else false
 */
bool DSlot::getValue(eValueIndex_t index, uint32_t *value)
{
    return mySensor->getValue(index, value);
}

/**
 * @brief   Set integer value
 * @param   index is function/sensor specific value identifier
 * @param   value to set for integer attribute
 * @return  true if successful, else false
 */
bool DSlot::setValue(eValueIndex_t index, uint32_t value)
{
    return mySensor->setValue(index, value);
}

/**
 * @brief   Get integer value
 * @param   index is function/sensor specific value identifier
 * @param   pointer to variable for return value of integer attribute
 * @return  true if successful, else false
 */
bool DSlot::getValue(eValueIndex_t index, sDate_t *date)
{
    return mySensor->getValue(index, date);
}