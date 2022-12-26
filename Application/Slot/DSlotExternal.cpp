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
#include <os.h>
#include <memory.h>
#include "app_cfg.h"
MISRAC_ENABLE

#include "DPV624.h"
#include "DSlotExternal.h"
#include "DSensorOwiAmc.h"
#include "Utilities.h"
#include "DSensorExternal.h"
#include "uart.h"
#include "leds.h"
#include "app_cfg.h"
/* Error handler instance parameter starts from 4901 to 5000 */

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
    myWaitFlags |= EV_FLAG_TASK_SENSOR_CONTINUE |
                   EV_FLAG_TASK_SENSOR_RETRY |
                   EV_FLAG_TASK_SLOT_SENSOR_CONTINUE |
                   EV_FLAG_TASK_SLOT_TAKE_NEW_READING |
                   EV_FLAG_TASK_SLOT_SENSOR_FW_UPGRADE |
                   EV_FLAG_TASK_SENSOR_SET_ZERO;

    myTaskId = ePM620Task;
}

/**
 * @brief   DSlotExternal class destructor
 * @param   void
 * @retval  void
 */
DSlotExternal::~DSlotExternal(void)
{

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
    myTaskStack = (CPU_STK *)OSMemGet((OS_MEM *)&memPartition, (OS_ERR *)&err);

    if(err == (OS_ERR)OS_ERR_NONE)
    {

        //memory block from the partition obtained, so can go ahead and run
        activate(myName, (CPU_STK_SIZE)MEM_PARTITION_BLK_SIZE, (OS_PRIO)APP_CFG_EXTERNAL_SLOT_TASK_PRIO, (OS_MSG_QTY)APP_CFG_EXTERNAL_SLOT_TASK_MSG_QTY, &err);

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
    eSysMode_t sysMode = E_SYS_MODE_NONE;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
    uint32_t failCount = 0u; //used for retrying in the event of failure

    uint32_t channelSel = 0u;
    uint32_t value = 0u;
    uint32_t sampleRate = 0u;
    uSensorIdentity_t sensorId;
    sensorId.value = 0u;
    eSensorError_t sensorError = mySensor->initialise();

    myState = E_SENSOR_STATUS_DISCOVERING;

    while(runFlag == true)
    {
        PV624->keepAlive(myTaskId);

        /* Sensor data acquisition is stopping after a certain amount of time
        The following code is changed to test it quickly */
        actualEvents = OSFlagPend(&myEventFlags,
                                  myWaitFlags, (OS_TICK)200u, //runs, nominally, at 20Hz by default
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
                sysMode = PV624->getSysMode();

                if((eSysMode_t)E_SYS_MODE_RUN == sysMode)
                {
                    PV624->errorHandler->handleError(E_ERROR_REFERENCE_SENSOR_COM,
                                                     eSetError,
                                                     0u,
                                                     4901u,
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

                        if(sensorId.dk == (uint32_t)(PM_TERPS_APPLICATION))
                        {
                            myState = E_SENSOR_DISCOVER_TERPS;
                        }

                        else
                        {
                            myState = E_SENSOR_STATUS_IDENTIFYING;
                            myOwner->postEvent(EV_FLAG_SENSOR_DISCOVERED);
                        }
                    }
                }

                break;

            case E_SENSOR_DISCOVER_TERPS:
                sensorError = mySensorDiscoverTerps();

                if(E_SENSOR_ERROR_NONE == sensorError)
                {
                    mySensor->getValue(E_VAL_INDEX_PM620_APP_IDENTITY, &sensorId.value);

                    if((472u == sensorId.dk) && (sensorId.major < 2u))
                    {
                        myState = E_SENSOR_STATUS_FW_UPGRADE;
                        PV624->errorHandler->handleError(E_ERROR_REFERENCE_SENSOR_COM,
                                                         eClearError,
                                                         0u,
                                                         4902u,
                                                         false);
                    }

                    else
                    {
                        myOwner->postEvent(EV_FLAG_SENSOR_DISCOVERED);
                        myState = E_SENSOR_STATUS_IDENTIFYING;
                    }

                }

                break;

            case E_SENSOR_STATUS_IDENTIFYING:
                sensorError = mySensorIdentify();

                if(sensorError == E_SENSOR_ERROR_NONE)
                {
                    /* have one reading available from the sensor */

                    if((472u == sensorId.dk) && (sensorId.major < 2u))
                    {
                        myOwner->postEvent(EV_FLAG_TASK_SENSOR_CONNECT);
                        // Terps requires a slight delay so centering can be faster TODO
                        myState = E_SENSOR_STATUS_FW_UPGRADE;
                        PV624->errorHandler->handleError(E_ERROR_REFERENCE_SENSOR_COM,
                                                         eClearError,
                                                         0u,
                                                         4902u,
                                                         false);
                    }

                    else
                    {
                        mySensorReadZero();
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
                                                         4902u,
                                                         false);
                        PV624->instrument->initController();
                        resume();
                    }
                }

                break;

            case E_SENSOR_STATUS_RUNNING:
                if(false == PV624->engModeStatus())
                {
                    channelSel = E_CHANNEL_0 | E_CHANNEL_1;
                    sensorError = mySensor->measure(channelSel);

                    //if no sensor error than proceed as normal (errors will be mopped up below)
                    if(sensorError == E_SENSOR_ERROR_NONE)
                    {
                        myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
                    }
                }

                break;

            case E_SENSOR_STATUS_FW_UPGRADE:
                sensorError = mySensorCommsCheck();
                break;

            default:
                break;
            }

            //check if there is an issue
            if(sensorError != E_SENSOR_ERROR_NONE)
            {
                sysMode = PV624->getSysMode();

                if((eSysMode_t)E_SYS_MODE_RUN == sysMode)
                {
                    failCount++;

                    if((failCount > 3u) && (myState != E_SENSOR_STATUS_DISCONNECTED))
                    {
                        /* Found that if the sensor is non responsive, then a checksum enable disable command may not
                        work. The application by default enables checksums and then willingly disables them. To control
                        this, first disable the checksums, since if the sensor is sitting in bootloader, checksums will
                        not be sent, but the parser will be expecting them */

                        mySensor->hardControlChecksum(E_CHECKSUM_DISABLED);
                        sensorError = mySensorTryRepower();

                        if(E_SENSOR_ERROR_NONE == sensorError)
                        {
                            /* Re powering the sensor found the bootloader version of the sensor thus indicating that
                            the sensor application has some problem. Read if the bootloader is of the PM TERPS */
                            mySensor->getValue(E_VAL_INDEX_PM620_BL_IDENTITY, &sensorId.value);

                            if(sensorId.dk == (uint32_t)(PM_TERPS_BOOTLOADER))
                            {
                                /* Found a TERPS bootloader. Set the state to firmware upgrade for the TERPS. Check one more
                                time if the TERPS app is also available. If it is, it is a valid sensor and FW upgrade
                                should not be forced. If not force firmware upgrade.
                                The mySensorCommsCheck function reads the application information from the PM620 */
                                sensorError = mySensorCommsCheck();

                                if(E_SENSOR_ERROR_NONE == sensorError)
                                {
                                    mySensor->getValue(E_VAL_INDEX_PM620_APP_IDENTITY, &sensorId.value);

                                    if(472u == sensorId.dk)
                                    {

                                        /* Application found, do not force upgrade, instead wait for a firmware upgrade
                                        command to be issued by GENII */
                                        mySensor->initializeSensorInfo();
                                        myState = E_SENSOR_STATUS_DISCOVERING;
                                    }
                                }

                                else
                                {
                                    /* Retry a couple of more times to see if sensor is available after a second each as
                                    screwing in the sensor may have given corrupted data */
                                    sleep(2000u);
                                    sensorError = mySensorCommsCheck();

                                    if(E_SENSOR_ERROR_NONE == sensorError)
                                    {
                                        mySensor->getValue(E_VAL_INDEX_PM620_APP_IDENTITY, &sensorId.value);

                                        if(472u == sensorId.dk)
                                        {

                                            /* Application found, do not force upgrade, instead wait for a firmware upgrade
                                            command to be issued by GENII */
                                            mySensor->initializeSensorInfo();
                                            myState = E_SENSOR_STATUS_DISCOVERING;
                                        }
                                    }

                                    else
                                    {
                                        sleep(2000u);
                                        sensorError = mySensorCommsCheck();

                                        if(E_SENSOR_ERROR_NONE == sensorError)
                                        {
                                            mySensor->getValue(E_VAL_INDEX_PM620_APP_IDENTITY, &sensorId.value);

                                            if(472u == sensorId.dk)
                                            {

                                                /* Application found, do not force upgrade, instead wait for a firmware upgrade
                                                command to be issued by GENII */
                                                mySensor->initializeSensorInfo();
                                                myState = E_SENSOR_STATUS_DISCOVERING;
                                            }
                                        }

                                        else
                                        {
                                            /* Power cycle the sensor one more time and burst firmware upgrade command */
                                            myState = E_SENSOR_STATUS_FW_UPGRADE;
                                            powerCycleSensor();
                                            sensorError = mySensor->upgradeFirmware();

                                            if(E_SENSOR_ERROR_NONE == sensorError)
                                            {
                                                /* Forced upgrade has been succesful, start discovering again */
                                                mySensor->initializeSensorInfo();
                                                myState = E_SENSOR_STATUS_DISCOVERING;
                                            }
                                        }
                                    }
                                }
                            }

                            else
                            {
                                /* Not a TERPS bootloader hence not a TERPS that is connected, or a complete sensor
                                failure */
                                sensorError = E_SENSOR_ERROR_UNAVAILABLE;
                            }
                        }

                        if(E_SENSOR_ERROR_NONE != sensorError)
                        {
                            /* Existing problem was not solved by power cycling the sensor. Problem exists. Reset all
                            variables and wait for a new sensor */
                            myState = E_SENSOR_STATUS_DISCONNECTED;
                            PV624->setPmUpgradePercentage(0u, 0u);
                            sensorError = (eSensorError_t)(E_SENSOR_ERROR_NONE);

                            //notify parent that we have hit a problem and are awaiting next action from higher functions
                            // Set the  PM 620 not connected error
                            sysMode = PV624->getSysMode();

                            if((eSysMode_t)E_SYS_MODE_RUN == sysMode)
                            {
                                PV624->errorHandler->handleError(E_ERROR_REFERENCE_SENSOR_COM,
                                                                 eSetError,
                                                                 0u,
                                                                 4903u,
                                                                 false);
                            }

                            myOwner->postEvent(EV_FLAG_TASK_SENSOR_DISCONNECT);
                            mySensor->initializeSensorInfo();
                        }
                    }
                }

                else
                {
                    sensorError = E_SENSOR_ERROR_NONE;
                    myState = E_SENSOR_STATUS_DISCOVERING;
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

        else if((actualEvents & EV_FLAG_TASK_SHUTDOWN) == EV_FLAG_TASK_SHUTDOWN)
        {
            runFlag = false;
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
                if((actualEvents & EV_FLAG_TASK_SENSOR_SET_ZERO) == EV_FLAG_TASK_SENSOR_SET_ZERO)
                {
                    if(472u != sensorId.dk)
                    {
                        sensorError = mySensorSetZero(); // set ZeroValue

                    }
                }

                if((actualEvents & EV_FLAG_TASK_SLOT_TAKE_NEW_READING) == EV_FLAG_TASK_SLOT_TAKE_NEW_READING)
                {
                    // Don't do anything if sensor is a terps and firmware version is less than what is expected
                    if((472u == sensorId.dk) && (sensorId.major < 2u))
                    {
                        /* Read the application version of the sensor just to keep the state machine actively looking
                        for errors if any */
                        sensorError = mySensorDiscover();
                    }

                    else
                    {
                        mySensor->getValue(E_VAL_INDEX_SENSOR_TYPE, &value);
                        getValue(E_VAL_INDEX_SAMPLE_RATE, &sampleRate);

                        /* Always check both bridge and diode */
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

            case E_SENSOR_STATUS_FW_UPGRADE:
                if((actualEvents & EV_FLAG_TASK_SLOT_SENSOR_FW_UPGRADE) == EV_FLAG_TASK_SLOT_SENSOR_FW_UPGRADE)
                {
                    /* May have previously had a good firmware in which case DK number would be read as 472, but if
                    app was corrupt and no firmware was available, the DK number returned will be 473. In any case,
                    perform firmware upgrade */
                    if((PM_TERPS_APPLICATION == sensorId.dk) || (PM_TERPS_BOOTLOADER == sensorId.dk))
                    {
                        PV624->setPmUpgradePercentage(0u, 0u);
                        sensorError = mySensor->upgradeFirmware();

                        if(E_SENSOR_ERROR_NONE == sensorError)
                        {
                            // Upgrade is successful, discover sensor again to read updated values
                            myState = E_SENSOR_STATUS_DISCOVERING;
                        }

                        else
                        {
                            /* There was some error during upgrade. In any case, go to discovering state as upgrade may
                            have failed but sensor firmware is still intact. If there were errors in the upgrade, will
                            taken care of by the discovering state */
                            myState = E_SENSOR_STATUS_DISCOVERING;
                        }
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

#if 0
/**
 * @brief   Handles the detection and re programming of a bricked TERPS sensor
            Detects if only bootloader is present on the TERPS, in which case, forces upgrade of application on to it
            Detect if application is also available in which case, goes to discover state to wait for upgrade command
            from GENII to upgrade firmware if necessary

            Update: 30-11-2022
            After a successful power cycle, the bootloader runs for about 5 seconds after which control is given to
            application on the terps sensor.

            If the bootlaoder is successfully detected, wait for another 5 seconds on the TERPS to detect if the
            application is present too, if not head to force firmware upgradef
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::brickedSensorHandler(void)
{
    // This function body needs to be replaced with state machine to handle bricked sensor
}

#endif
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
        OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_error);
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
 * @brief   Discover sensor on external comms
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorDiscoverTerps(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;
    OS_ERR os_error;
    eSensorError_t sensorError = sensor->readAppIdentityTerps();

    if(sensorError == E_SENSOR_ERROR_NONE)
    {
        OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_error);
        sensorError = sensor->readBootLoaderIdentityTerps();

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
 * @brief   This function is used to check whether the sensor is communicating on OWI
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorCommsCheck(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;
    eSensorError_t sensorError = sensor->readAppIdentityTerps();

    return sensorError;
}

/**
 * @brief   This function is used to provide a power cycle to the sensor if it was not responsive earlier. After the
            power cycle, the bootloader version is read again from the sensor, if it responds, then the app version is
            also read. If it responds then we go to standard discovery cycle. If not, it is assumed that the sensor has
            a faulty app and a good bootloader. If it is a TERPS, then we force a firmware upgrade. No action is taken
            for a piezo sensor
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorTryRepower(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;

    powerCycleSensor();
    sensorError = sensor->readBootLoaderIdentity();

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

    sensorError = sensor->readCoefficientsData();

    if(E_SENSOR_ERROR_NONE == sensorError)
    {
        sensorError = sensor->readCalibrationData();
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

    sensorError = sensor->readZeroData();

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

    sensorError = sensor->setZeroData(zeroValue);

    if(E_SENSOR_ERROR_NONE == sensorError)
    {
        //sensorError = sensor->getCalibrationData();
        sensorError = E_SENSOR_ERROR_NONE;
        myState = E_SENSOR_STATUS_RUNNING;
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

/**
 * @brief   Post an event to set zero value on the sensor
 * @param   void
 * @retval  sensor error status
 */
bool DSlotExternal::setSensorZeroValue(float zeroVal)
{
    zeroValue = zeroVal;
    postEvent(EV_FLAG_TASK_SENSOR_SET_ZERO);
    return true;
}

/**
 * @brief   Read an update the zero value from the PM sensor
 * @param   void
 * @retval  sensor error status
 */
bool DSlotExternal::getSensorZeroValue(float *zeroVal)
{
    bool successFlag = false;

    if(zeroVal != NULL)
    {
        DSensorExternal *sensor = (DSensorExternal *)mySensor;

        eSensorError_t sensorError = E_SENSOR_ERROR_TIMEOUT;

        sensorError = sensor->getZeroData(zeroVal);

        if(E_SENSOR_ERROR_NONE == sensorError)
        {
            successFlag = true;
        }
    }

    return successFlag;
}

/**
 * @brief   Raises an event to upgrade sensor firmware
 * @param   void
 * @retval  void
 */
void DSlotExternal::upgradeSensorFirmware(void)
{
    postEvent(EV_FLAG_TASK_SLOT_SENSOR_FW_UPGRADE);
}

/**
 * @brief   Cycles power to the PM620 / PM620T sensors
 * @param   void
 * @retval  void
 */
void DSlotExternal::powerCycleSensor(void)
{

    /* Turn the supply off by making the enable pin low */
    HAL_GPIO_WritePin(P6V_EN_PB15_GPIO_Port, P6V_EN_PB15_Pin, GPIO_PIN_RESET);
    /* Wait for sometime */
    sleep(1000u);
    /* Turn the supply ON by making the enable pin high */
    HAL_GPIO_WritePin(P6V_EN_PB15_GPIO_Port, P6V_EN_PB15_Pin, GPIO_PIN_SET);
    /* Wait for sometime so that the power up will be complete */
    sleep(25u);
}