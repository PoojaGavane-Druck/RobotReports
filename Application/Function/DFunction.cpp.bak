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
* @file     DFunction.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DFunction class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <stdint.h>
#include <os.h>
MISRAC_ENABLE

#include "DPV624.h"

#include "DFunction.h"
#include "memory.h"
#include "DProcessFilter.h"
#include "DProcessTare.h"
#include "DProcessMax.h"
#include "DProcessMin.h"
//#include "DProcessAlarmSensorHi.h"
//#include "DProcessAlarmSensorLo.h"
//#include "DProcessAlarmUserHi.h"
//#include "DProcessAlarmUserLo.h"
#include "DUserInterface.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DFunction class constructor
 * @param   void
 * @retval  void
 */
DFunction::DFunction(uint32_t index)
: DTask()
{
    OS_ERR os_error;
    myFunction = E_FUNCTION_NONE;
    myDirection = E_FUNCTION_DIR_MEASURE;
    myChannelIndex = index;

    mySlot = NULL;

    //create mutex for resource locking
    char *name = "Func";
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &os_error);

    if (os_error != (OS_ERR)OS_ERR_NONE)
    {
        //TODO: myStatus.osError = 1u;
    }

    //specify the flags that this function must respond to (add more as necessary in derived class)
    myWaitFlags =   EV_FLAG_TASK_SHUTDOWN |
                    EV_FLAG_TASK_NEW_READING |
                    EV_FLAG_TASK_NEW_SETPOINT |
                    EV_FLAG_TASK_SENSOR_CAL_REJECTED |
                    EV_FLAG_TASK_SENSOR_FAULT |
                    EV_FLAG_TASK_SENSOR_CONNECT |
                    EV_FLAG_TASK_SENSOR_DISCONNECT |
                    EV_FLAG_TASK_SENSOR_PAUSE;

    //add processes
    //processes[E_PROCESS_FILTER] = new DProcessFilter(myChannelIndex);
    //processes[E_PROCESS_TARE] = new DProcessTare(myChannelIndex);
    //processes[E_PROCESS_MAXIMUM] = new DProcessMax(myChannelIndex);
    //processes[E_PROCESS_MINIMUM] = new DProcessMin(myChannelIndex);
    //processes[E_PROCESS_SENSOR_ALARM_HI] = new DProcessAlarmSensorHi(myChannelIndex);
    //processes[E_PROCESS_SENSOR_ALARM_LO] = new DProcessAlarmSensorLo(myChannelIndex);
    //processes[E_PROCESS_USER_ALARM_HI] = new DProcessAlarmUserHi(myChannelIndex);
    //processes[E_PROCESS_USER_ALARM_LO] = new DProcessAlarmUserLo(myChannelIndex);

    //real time values
    myReading = 0.0f;                    //processed measurement value
    myPosFullscale = 0.0f;               //Positive fullscale of function sensor
    myNegFullscale = 0.0f;               //Negative fullscale of function sensor
    myResolution = 0.0f;                 //Resolution (accuracy of measurements)
}

/**
 * @brief   Create function slots
 * @param   void
 * @retval  void
 */
void DFunction::createSlots(void)
{
    //each instance overrides this function
}

/**
 * @brief   Get function settings
 * @param   void
 * @retval  void
 */
void DFunction::getSettingsData(void)
{
}

/**
 * @brief   Validate function settings as retrieved from persistent storage
 * @note    Invalid/uninitialised data is set to defaults
 * @param   void
 * @retval  void
 */
void DFunction::validateSettings(void)
{
}

/**
 * @brief   Apply function settings as retrieved from persistent storage
 * @param   void
 * @retval  void
 */
void DFunction::applySettings(void)
{
}

/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */
void DFunction::start(void)
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
        myTaskStack = NULL;
        //report error
    }
}

void DFunction::runFunction(void)
{
    //this is a while loop that pends on event flags
    bool runFlag = true;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;

    //start my main slot - this is set up by each derived class and cannot be NULL
    mySlot->start();

    //enter while loop
    while (runFlag == true)
    {
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)5000u,
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
        //check for events
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
            //no events
        }
        else if (os_error != (OS_ERR)OS_ERR_NONE)
        {
            //report error
        }
        else
        {
            //check for shutdown first
            if ((actualEvents & EV_FLAG_TASK_SHUTDOWN) == EV_FLAG_TASK_SHUTDOWN)
            {
                //shut down the main slot before exiting
                mySlot->shutdown();
                runFlag = false;
            }
            else
            {
                handleEvents(actualEvents);
            }
        }
    }
}

/**
 * @brief   Run processing
 * @param   void
 * @return  void
 */
void DFunction::runProcessing(void)
{
    float32_t value = 0.0f;

    //get compensated measurement from this function's sensor (index = 0)
    if (getSensorValue(0u, &value) == true)
    {
#ifdef PROCESS_ENABLED
        for(int32_t i = 0; i < (int32_t)E_PROCESS_NUMBER; i++)
        {
            value = processes[i]->run(value);
        }
#endif
        //value is now the processed value
        setReading(value);
    }
    else
    {
        //TODO: shouldn't ever get here, but what if we do?
        //setFunctionStatus(E_FUNC_STATUS_NO_READING);
    }
}

/**
 * @brief   Get Sensor Reading Value
 * @param   index is function/sensor specific
 * @param   pointer to variable for return value of compensated and processed measurement value in selects user units
 * @return  true if successful, else false
 */
bool DFunction::getSensorValue(uint32_t index, float32_t *value)
{
    bool success = false;

    if (mySlot != NULL)
    {
        DSensor * sensor = mySlot->getSensor();

        if (sensor != NULL)
        {
            //index not used - but can be if future need to have multiple outputs on a sensor
            *value = sensor->getMeasurement();
            success = true;
        }
    }

    return success;
}

/**
 * @brief   Handle function events
 * @param   event flags
 * @return  void
 */
void DFunction::handleEvents(OS_FLAGS actualEvents)
{
    DUserInterface* ui = PV624->userInterface;

    if ((actualEvents & EV_FLAG_TASK_NEW_READING) == EV_FLAG_TASK_NEW_READING)
    {
        //process and update value and inform UI
        runProcessing();

        ui->updateReading(myChannelIndex);
    }

    //only if setpoints can change in an automated way (eg, ramp, step, etc)
    if ((actualEvents & EV_FLAG_TASK_NEW_SETPOINT) == EV_FLAG_TASK_NEW_SETPOINT)
    {
        ui->notify(E_UI_MSG_NEW_SETPOINT, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_FAULT) == EV_FLAG_TASK_SENSOR_FAULT)
    {
        ui->notify(E_UI_MSG_SENSOR_FAULT, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_DISCONNECT) == EV_FLAG_TASK_SENSOR_DISCONNECT)
    {
        ui->sensorDisconnected(myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_PAUSE) == EV_FLAG_TASK_SENSOR_PAUSE)
    {
        ui->sensorPaused(myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CONNECT) == EV_FLAG_TASK_SENSOR_CONNECT)
    {
        //update sensor information
        updateSensorInformation();

        //inform UI tha we are ready to run
        ui->sensorConnected(myChannelIndex);                     //TODO: Discuss with Simon: which of these two ways to do this? WAY 1
        //ui->notify(E_UI_MSG_SENSOR_CONNECTED, myChannelIndex); //TODO: Discuss with Simon: which of these two ways to do this? WAY 2
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_REJECTED) == EV_FLAG_TASK_SENSOR_CAL_REJECTED)
    {
        ui->notify(E_UI_MSG_CAL_REJECTED, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DEFAULT) == EV_FLAG_TASK_SENSOR_CAL_DEFAULT)
    {
        ui->notify(E_UI_MSG_CAL_DEFAULT, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DUE) == EV_FLAG_TASK_SENSOR_CAL_DUE)
    {
        ui->notify(E_UI_MSG_CAL_DUE, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DATE) == EV_FLAG_TASK_SENSOR_CAL_DATE)
    {
        ui->notify(E_UI_MSG_CAL_DATE_BAD, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_ZERO_ERROR) == EV_FLAG_TASK_SENSOR_ZERO_ERROR)
    {
        ui->notify(E_UI_MSG_ZERO_ERROR, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_IN_LIMIT) == EV_FLAG_TASK_SENSOR_IN_LIMIT)
    {
        ui->notify(E_UI_MSG_SETPOINT_REACHED, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_NEW_RANGE) == EV_FLAG_TASK_SENSOR_NEW_RANGE)
    {
        ui->notify(E_UI_MSG_AUTO_RANGE, myChannelIndex);
    }
}

/**
 * @brief   Update sensor information locally
 * @param   void
 * @return  void
 */
void DFunction::updateSensorInformation(void)
{
    if (mySlot != NULL)
    {
        DSensor * sensor = mySlot->getSensor();

        if (sensor != NULL)
        {
            DLock is_on(&myMutex);

            myPosFullscale = sensor->getFullScaleMax();
            myNegFullscale = sensor->getFullScaleMin();
            myAbsPosFullscale = sensor->getAbsFullScaleMax();
            myAbsNegFullscale = sensor->getAbsFullScaleMin();
            myResolution = sensor->getResolution();
        }
    }
}

/**
 * @brief   Clean up after termination of task
 * @param   void
 * @return  void
 */
void DFunction::cleanUp(void)
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

    //signal shutdown to UI
    PV624->userInterface->functionShutdown(myChannelIndex);
}

/**
 * @brief   read function output
 * @param   index is function specific
 * @param   pointer for return value of output setpoint
 * @return  true if successful, else false
 */
bool DFunction::getOutput(uint32_t index, float32_t *value)
{
    bool success = false;

    if (mySlot != NULL)
    {
        DSensor * sensor = mySlot->getSensor();

        if (sensor != NULL)
        {
            //index not used - but can be if future need to have multiple outputs on a sensor
            *value = sensor->getOutput();
            success = true;
        }
    }

    return success;
}

/**
 * @brief   Set output for this function
 * @note    This is used by source slots to update setpoint.
 *          Operations that change sensor values must go through the slot and not directly to sensor
 * @param   index is function specific
 * @param   value is the output setpoint
 * @return  true if successful, else false
 */
bool DFunction::setOutput(uint32_t index, float32_t value)
{
    return false;
}

/**
 * @brief   Get Value
 * @param   index is function/sensor specific
 * @param   pointer to variable for return value of compensated and processed measurement value in selects user units
 * @return  true if successful, else false
 */
bool DFunction::getValue(uint32_t index, float32_t *value)
{
    bool success = false;

    if (mySlot != NULL)
    {
        success = true;

        switch (index)
        {
            case 0u:    //index 0 = processed value
                *value = getReading();
                break;

            case 1u:    //index 1 = positive FS
                *value = getPosFullscale();
                break;

            case 2u:    //index 2 = negative FS
                *value = getNegFullscale();
                break;

            case 3u:    //index 3 = resolution
                *value = getResolution();
                break;

            default:
                success = false;
                break;
        }
    }

    return success;
}

/**
 * @brief   Signal sensor to continue/resume
 * @param   void
 * @retval  true if all's well, else false
 */
bool DFunction::sensorContinue(void)
{
    bool success = false;

    if (mySlot != NULL)
    {
        //apply the user settings for this function
        applySettings();

        //and go!
        mySlot->resume();
        success = true;
    }

    return success;
}

/**
 * @brief   Signal sensor to retry after failure reported
 * @param   void
 * @retval  true if all's well, else false
 */
bool DFunction::sensorRetry(void)
{
    bool success = false;

    if (mySlot != NULL)
    {
        mySlot->retry();
        success = true;
    }

    return success;
}

/**
 * @brief   Get processed measurement value
 * @param   void
 * @retval  value of
 */
float32_t DFunction::getReading(void)
{
    DLock is_on(&myMutex);
    return myReading;
}

/**
 * @brief   Set processed measurement value
 * @param   value of processed measurement value
 * @retval  void
 */
void DFunction::setReading (float32_t value)
{
    DLock is_on(&myMutex);
    myReading = value;
}

/**
 * @brief   Get positive fullscale of function sensor
 * @param   void
 * @retval  value of positive fullscale of function sensor
 */
float32_t DFunction::getPosFullscale(void)
{
    DLock is_on(&myMutex);
    return myPosFullscale;
}

/**
 * @brief   Set
 * @param   value of positive fullscale of function sensor
 * @retval  void
 */
void DFunction::setPosFullscale(float32_t value)
{
    DLock is_on(&myMutex);
    myPosFullscale = value;
}

/**
 * @brief   Get negative fullscale of function sensor
 * @param   void
 * @retval  value of negative fullscale of function sensor
 */
float32_t DFunction::getNegFullscale(void)
{
    DLock is_on(&myMutex);
    return myNegFullscale;
}

/**
 * @brief   Set negative fullscale of function sensor
 * @param   value of negative fullscale of function sensor
 * @retval  void
 */
void DFunction::setNegFullscale(float32_t value)
{
    DLock is_on(&myMutex);
    myNegFullscale = value;
}

/**
 * @brief   Get absolute positive fullscale of function sensor
 * @param   void
 * @retval  value of absolute positive fullscale of function sensor
 */
float32_t DFunction::getAbsPosFullscale(void)
{
    DLock is_on(&myMutex);
    return myAbsPosFullscale;
}

/**
 * @brief   Set
 * @param   value of absolute positive fullscale of function sensor
 * @retval  void
 */
void DFunction::setAbsPosFullscale(float32_t value)
{
    DLock is_on(&myMutex);
    myAbsPosFullscale = value;
}

/**
 * @brief   Get abslolute negative fullscale of function sensor
 * @param   void
 * @retval  value of absolute negative fullscale of function sensor
 */
float32_t DFunction::getAbsNegFullscale(void)
{
    DLock is_on(&myMutex);
    return myAbsNegFullscale;
}

/**
 * @brief   Set abslolute negative fullscale of function sensor
 * @param   value of absolute negative fullscale of function sensor
 * @retval  void
 */
void DFunction::setAbsNegFullscale(float32_t value)
{
    DLock is_on(&myMutex);
    myAbsNegFullscale = value;
}

/**
 * @brief   Get resolution (accuracy of measurements)
 * @param   void
 * @retval  value of resolution (accuracy of measurements)
 */
float32_t DFunction::getResolution(void)
{
    DLock is_on(&myMutex);
    return myResolution;
}

/**
 * @brief   Set resolution (accuracy of measurements)
 * @param   value of resolution (accuracy of measurements)
 * @retval  void
 */
void DFunction::setResolution(float32_t value)
{
    DLock is_on(&myMutex);
    myResolution = value;
}

//Scaling defaults:
//
//•	Volts Measure:			(0V, 10V), (0% , 100%)
//•	mV Measure:			(0mV , 2000mV) (0% 100%)
//•	Current Measure / Source    	(4mA, 20mA) (0%, 100%)
//•	Pressure   				(0, FS) (0%, 100%)
