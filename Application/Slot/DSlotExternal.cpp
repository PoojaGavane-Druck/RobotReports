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
* @file     DSlotExternal.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The DSlotExternal class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <rtos.h>
#include <memory.h>
MISRAC_ENABLE

#include "DPV624.h"
#include "DSlotExternal.h"
#include "DSensorOwiAmc.h"
#include "Utilities.h"
#include "DSensorExternal.h"
#include "uart.h"
#include "leds.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlotExternal class constructor
 * @param   owner: the task that created this slot
 * @retval  void
 */
DSlotExternal::DSlotExternal(DTask *owner)
    : DSlot(owner)
{
    myWaitFlags |= EV_FLAG_TASK_SLOT_POWER_DOWN | EV_FLAG_TASK_SLOT_POWER_UP | EV_FLAG_TASK_SENSOR_CONTINUE |
                   EV_FLAG_TASK_SENSOR_RETRY | EV_FLAG_TASK_SLOT_SENSOR_CONTINUE |
                   EV_FLAG_TASK_SENSOR_TAKE_NEW_READING;
}

/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */
void DSlotExternal::start(void)
{
    OS_ERR err;

    //get stack area from the memory partition memory block for function tasks
    myTaskStack = (CPU_STK *)RTOSMemGet((OS_MEM *)&memPartition, (OS_ERR *)&err);

    if(err == (OS_ERR)OS_ERR_NONE)
    {

        //memory block from the partition obtained, so can go ahead and run
        activate(myName, (CPU_STK_SIZE)MEM_PARTITION_BLK_SIZE, (OS_PRIO)3u, (OS_MSG_QTY)10u, &err);

    }

    else
    {
        //report error
    }
}

/**
 * @brief   Run DSlotExternal task function
 * @param   void
 * @retval  void
 */
void DSlotExternal::runFunction(void)
{
    //this is a while loop that pends on event flags
    bool runFlag = true;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
    uint32_t failCount = 0u; //used for retrying in the event of failure
    uint32_t channelSel = 0u;
    uint32_t value = 0u;
    uint32_t sampleRate = 0u;
    eSensorError_t sensorError = mySensor->initialise();
    uSensorIdentity_t sensorId;
    uint32_t sampleTimeout = 10u;

    myState = E_SENSOR_STATUS_DISCOVERING;

    while(runFlag == true)
    {
#ifdef TASK_HEALTH_MONITORING_IMPLEMENTED
        PV624->keepAlive(myTaskId);
#endif
        /* Sensor data acquisition is stopping after a certain amount of time
        The following code is changed to test it quickly */
        actualEvents = RTOSFlagPend(&myEventFlags,
                                    myWaitFlags, (OS_TICK)(sampleTimeout), //runs, nominally, at 10ms by default
                                    OS_OPT_PEND_BLOCKING |
                                    OS_OPT_PEND_FLAG_SET_ANY |
                                    OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
#ifdef ENABLE_STACK_MONITORING
        lastTaskRunning = myTaskId;
#endif

        //check actions to execute routinely (ie, timed)
        if(os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
            switch(myState)
            {
            case E_SENSOR_STATUS_DISCOVERING:
                //any sensor error will be mopped up below
                // Add delay to allow sensor to be powered up and running
                // Set PM 620 not connected error

                PV624->errorHandler->handleError(E_ERROR_REFERENCE_SENSOR_COM,
                                                 eSetError,
                                                 0u,
                                                 60u,
                                                 false);

                // Set an error here so that sensor will not be looked for by the GENII

                sensorError = mySensorChecksumDisable();

                if(E_SENSOR_ERROR_NONE == sensorError)
                {
                    sensorError = mySensorDiscover();
                }

                if(E_SENSOR_ERROR_NONE == sensorError)
                {

                    mySensor->getValue(E_VAL_INDEX_PM620_APP_IDENTITY, &sensorId.value);

                    if(PM620_TERPS_APP_DK_NUMBER == sensorId.dk)
                    {
                        myState = E_SENSOR_STATUS_IDENTIFYING;
                    }

                    else
                    {
                        myState = E_SENSOR_STATUS_READ_ZERO;
                    }
                }

                break;

            case E_SENSOR_STATUS_READ_ZERO:
#ifdef SET_ZERO
                sensorError = mySensorSetZero();
#endif
                sensorError = mySensorReadZero();

                if(sensorError == E_SENSOR_ERROR_NONE)
                {
                    myState = E_SENSOR_STATUS_IDENTIFYING;
                }

                break;

            case E_SENSOR_STATUS_IDENTIFYING:
                sensorError = mySensorIdentify();

                //if no sensor error than proceed as normal (errors will be mopped up below)
                if(sensorError == E_SENSOR_ERROR_NONE)
                {
                    /* have one reading available from the sensor */
                    setValue(E_VAL_INDEX_SAMPLE_RATE, (uint32_t)E_ADC_SAMPLE_RATE_27_5_HZ);
                    channelSel = E_CHANNEL_0 | E_CHANNEL_1;
                    sensorError = mySensor->measure(channelSel);
                    //notify parent that we have connected, awaiting next action - this is to allow
                    //the higher level to decide what other initialisation/registration may be required
                    myOwner->postEvent(EV_FLAG_TASK_SENSOR_CONNECT);
                    // Clear the  PM 620 not connected error
                    PV624->errorHandler->handleError(E_ERROR_REFERENCE_SENSOR_COM,
                                                     eClearError,
                                                     0u,
                                                     61u,
                                                     false);
                    PV624->instrument->initController();
                    resume();
                }

                break;

            case E_SENSOR_STATUS_RUNNING:
                if((EV_FLAG_TASK_SLOT_FIRMWARE_UPGRADE == (actualEvents & EV_FLAG_TASK_SLOT_FIRMWARE_UPGRADE)) &&
                        (PM620_TERPS_APP_DK_NUMBER == sensorId.dk))
                {

                    myState = E_SENSOR_STATUS_UPGRADING;
                }

                else
                {
                    if((eAquisationMode_t)E_CONTINIOUS_ACQ_MODE == myAcqMode)
                    {
                        mySensor->getValue(E_VAL_INDEX_SENSOR_TYPE, &value);
                        getValue(E_VAL_INDEX_SAMPLE_RATE, &sampleRate);
                        getValue(E_VAL_INDEX_SAMPLE_TIMEOUT, &sampleTimeout);
                        // Always read both channels
                        channelSel = E_CHANNEL_0 | E_CHANNEL_1;
                        sensorError = mySensor->measure(channelSel);

                        //if no sensor error than proceed as normal (errors will be mopped up below)
                        if(sensorError == E_SENSOR_ERROR_NONE)
                        {
                            myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
                        }

                    }
                }

                break;

            case E_SENSOR_STATUS_UPGRADING:
                if((eSensorMode_t)E_SENSOR_MODE_FW_UPGRADE != mySensor->getMode())
                {
                    mySensor->setMode((eSensorMode_t)E_SENSOR_MODE_FW_UPGRADE);
                    sensorError = mySensor->upgradeFirmware();

                    //if no sensor error than proceed as normal (errors will be mopped up below)
                    if(sensorError == E_SENSOR_ERROR_NONE)
                    {
                        myState = E_SENSOR_STATUS_DISCOVERING;
                    }
                }

                break;

            default:

                break;
            }

            //check if there is an issue
            if(sensorError != E_SENSOR_ERROR_NONE)
            {
                failCount++;

                if((failCount > 3u) && (myState != E_SENSOR_STATUS_DISCONNECTED))
                {
                    //TODO: what error stattus to set????

                    myState = E_SENSOR_STATUS_DISCONNECTED;
                    sensorError = (eSensorError_t)(E_SENSOR_ERROR_NONE);
                    //notify parent that we have hit a problem and are awaiting next action from higher level functions
                    // Set the  PM 620 not connected error
                    PV624->errorHandler->handleError(E_ERROR_REFERENCE_SENSOR_COM,
                                                     eSetError,
                                                     0u,
                                                     62u,
                                                     false);
                    myOwner->postEvent(EV_FLAG_TASK_SENSOR_DISCONNECT);

                }
            }

            else
            {
                failCount = 0u;
            }
        }

        else if(os_error != (OS_ERR)OS_ERR_NONE)
        {
            //os error
            sensorError = E_SENSOR_ERROR_OS;
        }

        else if((actualEvents & EV_FLAG_TASK_SLOT_POWER_DOWN) == EV_FLAG_TASK_SLOT_POWER_DOWN)
        {
            myState = E_SENSOR_STATUS_SHUTDOWN;
        }

        else if((actualEvents & EV_FLAG_TASK_SLOT_POWER_UP) == EV_FLAG_TASK_SLOT_POWER_UP)
        {
            myState = E_SENSOR_STATUS_DISCOVERING;
        }

        else if((actualEvents & EV_FLAG_TASK_SENSOR_CONNECT) == EV_FLAG_TASK_SENSOR_CONNECT)
        {
            resume();
        }

        else
        {
            //check events that can occur at any time first
            switch(myState)
            {
            case E_SENSOR_STATUS_READY:

                //waiting to be told to start running
                if((actualEvents & EV_FLAG_TASK_SLOT_SENSOR_CONTINUE) == EV_FLAG_TASK_SLOT_SENSOR_CONTINUE)
                {
                    myState = E_SENSOR_STATUS_RUNNING;
                }

                break;

            case E_SENSOR_STATUS_DISCONNECTED:

                //disconnected or not responding
                //waiting to be told to re-start (go again from the beginning)
                if((actualEvents & EV_FLAG_TASK_SENSOR_RETRY) == EV_FLAG_TASK_SENSOR_RETRY)
                {
                    myState = E_SENSOR_STATUS_DISCOVERING;
                    //TODO: allow some delay before restarting, or rely on higher level to do that?
                    sleep(250u);
                }

                break;

            case E_SENSOR_STATUS_RUNNING:
                if((actualEvents & EV_FLAG_TASK_SENSOR_TAKE_NEW_READING) == EV_FLAG_TASK_SENSOR_TAKE_NEW_READING)
                {
                    mySensor->getValue(E_VAL_INDEX_SENSOR_TYPE, &value);
                    getValue(E_VAL_INDEX_SAMPLE_RATE, &sampleRate);
                    getValue(E_VAL_INDEX_SAMPLE_TIMEOUT, &sampleTimeout);

                    /* Always check both bridge and diode */
                    channelSel = E_CHANNEL_0 | E_CHANNEL_1;

                    sensorError = mySensor->measure(channelSel);

                    //if no sensor error than proceed as normal (errors will be mopped up below)
                    if(sensorError == E_SENSOR_ERROR_NONE)
                    {
                        myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
                    }
                }

                break;

            default:
                break;
            }
        }

        //set/clear any errors in executing sensor functions
        updateSensorStatus(sensorError);
    }

    sensorError = mySensor->close();
}

/**
 * @brief   Discover sensor on external comms
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorDiscover(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;
    OS_ERR os_error;
    eSensorError_t sensorError = sensor->readAppIdentity();

    if(sensorError == E_SENSOR_ERROR_NONE)
    {
        RTOSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_error);
        sensorError = sensor->readBootLoaderIdentity();

        if(sensorError == E_SENSOR_ERROR_NONE)
        {
            sensorError = sensor->readSerialNumber();

            if(sensorError == E_SENSOR_ERROR_NONE)
            {
                myState = E_SENSOR_STATUS_IDENTIFYING;
            }

            else
            {
                myState = E_SENSOR_STATUS_DISCONNECTED;
            }
        }
    }

    return sensorError;
}

/**
 * @brief   Get sensor details
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorIdentify(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;

    eSensorError_t sensorError = E_SENSOR_ERROR_TIMEOUT;

    sensorError = sensor->getCoefficientsData();

    if(E_SENSOR_ERROR_NONE == sensorError)
    {
        //sensorError = sensor->getCalibrationData();
        sensorError = E_SENSOR_ERROR_NONE;
        myState = E_SENSOR_STATUS_READY;
    }

    return sensorError;
}

/**
 * @brief   Read the zero value from the sensor
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorReadZero(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;

    eSensorError_t sensorError = E_SENSOR_ERROR_TIMEOUT;

    sensorError = sensor->getZeroData();

    if(E_SENSOR_ERROR_NONE == sensorError)
    {
        //sensorError = sensor->getCalibrationData();
        sensorError = E_SENSOR_ERROR_NONE;
        myState = E_SENSOR_STATUS_READY;
    }

    return sensorError;
}


/**
 * @brief   Read the zero value from the sensor
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorSetZero(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;

    eSensorError_t sensorError = E_SENSOR_ERROR_TIMEOUT;

    sensorError = sensor->setZeroData();

    if(E_SENSOR_ERROR_NONE == sensorError)
    {
        //sensorError = sensor->getCalibrationData();
        sensorError = E_SENSOR_ERROR_NONE;
        myState = E_SENSOR_STATUS_READY;
    }

    return sensorError;
}


/**
 * @brief   Instruct the sensor to enable check sum in OWI protocol while communication
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorChecksumEnable(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;

    eSensorError_t sensorError = E_SENSOR_ERROR_TIMEOUT;

    sensorError = sensor->setCheckSum(E_CHECKSUM_ENABLED);

    return sensorError;
}

/**
 * @brief   Instruct the sensor to enable check sum in OWI protocol while communication
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorChecksumDisable(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;

    eSensorError_t sensorError = E_SENSOR_ERROR_TIMEOUT;

    sensorError = sensor->setCheckSum(E_CHECKSUM_DISABLED);

    return sensorError;
}