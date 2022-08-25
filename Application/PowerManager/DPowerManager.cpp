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
* @file     DPowerManager.cpp
* @version  1.00.00
* @author   Nageswara Pydisetty / Makarand Deshmukh
* @date     28 April 2020
*
* @brief    The DPowerManager class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <rtos.h>
#include "app_cfg.h"
MISRAC_ENABLE
#include "DPowerManager.h"
#include "DErrorhandler.h"
#include "memory.h"
#include "smbus.h"
#include "DLock.h"
#include "DPV624.h"
#include "main.h"
#include "Utilities.h"
/* Error handler instance parameter starts from 2401 to 2500 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define BATTERY_POLLING_INTERVAL 20
#define KELVIN_TO_CEL    273.15f
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
OS_ERR pSmbusErr;
OS_FLAG_GRP smbusErrFlagGroup;
OS_FLAGS smbusErrFlag;
OS_FLAGS smbusErrPendFlags;

const float32_t battVoltageCritThreshold = 10.1f;
const float32_t battVoltageWarnThreshold = 10.3f;

const float32_t batteryCriticalLevelThreshold = 5.0f;
const float32_t batteryWarningLevelThreshold = 10.0f;
const float32_t motorVoltageThreshold = 21.0f;
const float32_t refSensorVoltageThreshold = 4.75f;

const uint8_t battManufacturerName[] = "RRC";
const uint8_t battDeviceName[] = "RRC2040-2";
const uint8_t battChemistryType[] = "LION";

#ifdef FIT_BATTERY_CHARGER_COMM
const uint32_t battChargerIndentity = 0x555u;
#else
const uint32_t battChargerIndentity = 0x202u;
#endif
CPU_STK powerManagerTaskStack[APP_CFG_POWER_MANAGER_TASK_STACK_SIZE];

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   Power manager class constructor
 * @note
 * @param   owner: the task that created this slot
 * @retval  void
 */
DPowerManager::DPowerManager(SMBUS_HandleTypeDef *smbus, OS_ERR *osErr)
    : DTask()
{
    OS_ERR osError;

    uint8_t dataArray[10] = { 0 };
    uint32_t isEqual = 0u;

    // Create mutex for resource locking
    char *name = "PowerManager";

    myTaskId = ePowerManagerTask;

    /* Create objects required by task */
    ltc4100 = new LTC4100(smbus);           // Battery charger object
    battery = new smartBattery(smbus);      // Battery object
    voltageMonitor = new DVoltageMonitor(); // Voltage monitor object
    chargingStatus = 0u;
    ltcIdentity = 0u;

    initialise();

    ltc4100->getLtcVersionInfo(&ltcIdentity);

    if(battChargerIndentity == ltcIdentity)
    {
        // Read the manufacturer name of the battery
        battery->getValue(eManufacturerName, dataArray);
        memcpy(manufacturerName, &dataArray[1], (sizeof(manufacturerName) - 1u));
        isEqual = compareArrays(battManufacturerName, (const uint8_t *)(manufacturerName), sizeof(manufacturerName));

        if(0u != isEqual)
        {
            battery->getValue(eDeviceName, dataArray);
            memcpy(batteryName, &dataArray[1], (sizeof(batteryName) - 1u));
            isEqual = compareArrays(battDeviceName, (const uint8_t *)(batteryName), sizeof(batteryName));

            if(0u != isEqual)
            {
                battery->getValue(eDeviceChemistry, dataArray);
                memcpy(batteryChemistry, &dataArray[1], (sizeof(batteryChemistry) - 1u));
                isEqual = compareArrays(battChemistryType, (const uint8_t *)(batteryChemistry), sizeof(batteryChemistry));

                if(0u != isEqual)
                {
                    // Read the full capacity of the battery
                    battery->getValue(eFullChargeCapacity, &fullCapacity);

                    // check and raise error if battery full capacity is read as 0 as battery full capacity cannot be 0
                    if(fullCapacity == 0u)
                    {
                        // Generate battery comm error
                        PV624->handleError(E_ERROR_BATTERY_COMM, eSetError, 0u, 2423u, true);
                    }
                }

                else
                {
                    PV624->handleError(E_ERROR_BATTERY_COMM, eSetError, 0u, 2424u, true);
                }
            }

            else
            {
                PV624->handleError(E_ERROR_BATTERY_COMM, eSetError, 0u, 2425u, true);
            }
        }

        else
        {
            PV624->handleError(E_ERROR_BATTERY_COMM, eSetError, 0u, 2426u, true);
        }
    }

    else
    {
        PV624->handleError(E_ERROR_BATTERY_CHARGER_COMM, eSetError, 0u, 2428u, true);
    }

    /* Init class veriables */
    timeElapsed = 0u;

// Specify the flags that this function must respond to
    myWaitFlags = EV_FLAG_TASK_SHUTDOWN |
                  EV_FLAG_TASK_UPDATE_BATTERY_STATUS |
                  EV_FLAG_TASK_BATT_CHARGER_ALERT;

    RTOSMutexCreate(&myMutex, (CPU_CHAR *)name, &osError);

    myTaskStack = (CPU_STK *)&powerManagerTaskStack[0];

#ifdef ENABLE_STACK_MONITORING
    stackArray.uiStack.addr = (void *)myTaskStack;
    stackArray.uiStack.size = (uint32_t)(APP_CFG_POWER_MANAGER_TASK_STACK_SIZE * 4u);
    fillStack((char *)myTaskStack, 0x5A, (size_t)(APP_CFG_POWER_MANAGER_TASK_STACK_SIZE * 4u));
#endif

    // Memory block from the partition obtained, so can go ahead and run
    activate(name, (CPU_STK_SIZE)APP_CFG_POWER_MANAGER_TASK_STACK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)10u, &osError);

}

/**
  * @brief  class destructor.
  * @param  none
  * @retval None
  */
DPowerManager::~DPowerManager()
{

}

/**
 * @brief   Initialisation for the power manager parameters
 * @note
 * @param   owner: the task that created this slot
 * @retval  void
 */
void DPowerManager::initialise(void)
{
    resetDataArray(manufacturerName, sizeof(manufacturerName));
    resetDataArray(batteryName, sizeof(batteryName));
    resetDataArray(batteryChemistry, sizeof(batteryChemistry));
}

/**
 * @brief   Reset data array at address
 * @note
 * @param   owner: the task that created this slot
 * @retval  void
 */
void DPowerManager::resetDataArray(uint8_t *source, uint32_t length)
{
    uint32_t counter = 0u;

    for(counter = 0u; counter < length; counter++)
    {
        source[counter] = 0u;
    }
}

/**
 * @brief   This function monitors the battery parameters
 * @param   void
 * @retval  void
 */
void DPowerManager::monitorBatteryParams(void)
{

}
/**
 * @brief   Run DPowerManager task funtion
 * @param   void
 * @retval  void
 */
void DPowerManager::runFunction(void)
{
    //this is a while loop that pends on event flags
    bool runFlag = true;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;

    while(runFlag == true)
    {
        PV624->keepAlive(myTaskId);

        actualEvents = RTOSFlagPend(&myEventFlags,
                                    myWaitFlags, (OS_TICK)500u, //runs, nominally, at 2Hz by default
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
#ifdef ENABLE_STACK_MONITORING
        lastTaskRunning = myTaskId;
#endif
        bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT));

        if(!ok)
        {
            MISRAC_DISABLE
#ifdef ASSERT_ENABLED
            assert(false);
#endif
            MISRAC_ENABLE
            PV624->handleError(E_ERROR_OS,
                               eSetError,
                               (uint32_t)os_error,
                               2401u);
        }

        //check for events
        if(ok)
        {
            // Task runs at timeout only
            if(os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT))
            {
                /* Task has timed out with no event. Check the battery charger and battery for presence and check
                status of external power source availability and current status provided by the battery charger */
                getPowerInfo();
            }

            else
            {
                /* Some event has occured on the task, serve the event */
                if((actualEvents & EV_FLAG_TASK_BATT_CHARGER_ALERT) == EV_FLAG_TASK_BATT_CHARGER_ALERT)
                {
                    handleChargerAlert();
                }
            }
        }
    }
}

/**
 * @brief   Clean up after termination of task
 * @param   void
 * @return  void
 */
void DPowerManager::cleanUp(void)
{
    OS_ERR err;

    //if a stack was allocated then free that memory to the partition
    if(myTaskStack != NULL)
    {
        //Return the stack memory block back to the partition
        RTOSMemPut((OS_MEM *)&memPartition, (void *)myTaskStack, (OS_ERR *)&err);

        if(err == (OS_ERR)OS_ERR_NONE)
        {
            //memory block from the partition obtained
        }

        myTaskStack = NULL;
    }
}

/**
 * @brief   Acquires the PV624 power information from the battery and battery charger. First checks if the battery and
            charger is available, if none is then charging is stopped. Then controls the charging of the battery based
            on the status of the bits from the battery charger IC status
 * @param   void
 * @return  void
 */
void DPowerManager::getPowerInfo(void)
{
    uint8_t dataArray[10] = { 0 };

    uint32_t isEqual = 0u;
    uint32_t acStatus = 0u;
    uint32_t terminateCharging = 0u;
    uint32_t fullyChargedStatus = 0u;

    ltc4100->getLtcVersionInfo(&ltcIdentity);

    if(battChargerIndentity == ltcIdentity)
    {
        battery->getValue(eDeviceName, dataArray);
        memcpy(batteryName, &dataArray[1], (sizeof(batteryName) - 1u));
        isEqual = compareArrays(battDeviceName, (const uint8_t *)(batteryName), sizeof(batteryName));

        if(0u != isEqual)
        {
            timeElapsed++;
            battery->getTerminateChargeAlarm(&terminateCharging);
            battery->getFullyChargedStatus(&fullyChargedStatus);
            ltc4100->getIsAcPresent(&acStatus);

            if((1u == fullyChargedStatus) ||
                    (1u == terminateCharging))
            {
                if(eBatteryCharging == chargingStatus)
                {
                    /* We require to write the LTC4100 current and voltage reigsters if the battery is
                    charging */
                    clearAllBatteryErrors(2432u);

                    if((1u == fullyChargedStatus) ||
                            (1u == terminateCharging))
                    {
                        chargingStatus = eBatteryDischarging;
                        stopCharging();
                    }
                }
            }

            else if(((uint32_t)(AC_PRESENT) == acStatus) && (chargingStatus == eBatteryDischarging))
            {
                /* Charger is connected, evaluate whether charging is required or not and start / stop the
                same */
                handleChargerAlert();
            }

            else
            {
                if(chargingStatus == eBatteryDischarging)
                {
                    // handle charger alert does not need to be done if the battery is discharging
                    // Explore that the charger connected is handled in the SMBUS alert interrupt
                    keepDischarging();
                }

                else
                {
                    keepCharging();
                }
            }

            checkVoltages();

            if((uint32_t)(BATTERY_POLLING_INTERVAL) <= timeElapsed)
            {
                /* Polling interval for the battery has elapsed, check all the parameters from the battery
                and generate / clear errors if any. These parameters will also be used by RB DUCI commands
                to provide PV624 battery information to the GENII */
                checkRemainingBattery();

                timeElapsed = 0u;
            }
        }

        else
        {
            /* Battery Manufacturer name and device name do not match with what is expected. This signals
            that an incorrect battery installed or there is no battery but power has been applied from the
            wall adapter. In this case, generate battery error */
            PV624->handleError(E_ERROR_BATTERY_COMM, eSetError, 0u, 2427u, true);
        }
    }

    else
    {
        PV624->handleError(E_ERROR_BATTERY_CHARGER_COMM, eSetError, 0u, 2429u, true);
    }
}


/**
 * @brief   Checks how much battery percentage is available and also checks battery voltage level, if any one of the
            two is lower than required threshold, a battery error either critical or warning is generated by this
            function
 * @param   void
 * @return  void
 */
void DPowerManager::checkRemainingBattery(void)
{
    eBatteryErr_t batError = eBatteryError;
    float32_t percentage = 0.0f;
    float32_t voltage = 0.0f;

    batError = battery->readAllParameters();

    if(eBatterySuccess == batError)
    {
        battery->getValue(ePercentage, &percentage);
        battery->getValue(eVoltage, &voltage);

        if(chargingStatus == eBatteryDischarging)
        {
            if((percentage <= batteryCriticalLevelThreshold) || (voltage <= battVoltageCritThreshold))
            {
                setBatteryCriticalError(percentage, 2430u);
            }

            else
            {
                if((percentage <= batteryWarningLevelThreshold) || (voltage <= battVoltageWarnThreshold))
                {
                    setBatteryWarningError(percentage, 2431u);
                }
            }
        }

        else
        {
            clearAllBatteryErrors(2433u);
        }
    }
}

/**
 * @brief   Handles a charger alert condition - this can occur when a battery is connected or a charger is connected
            or either of them being disconnected. Upon this condition, the charger status register is read and then
            decision is made to:
            1. Charger and battery connected and battery level is low - start charging
            2. Charger and battery connected and battery level is charged - stop charging
            3. Charger is removed - stop charging
            4. Battery is removed - stop charging
 * @param   void
 * @return  void
 */
void DPowerManager::handleChargerAlert(void)
{
    uint32_t status = 0u;
    uint32_t batteryStatus = 0u;
    uint32_t acStatus = 0u;
    uint32_t capacity = 0u;

    /* A charger alert condition has occured
    1. Read the charger status
    2. Interpret the changes in status
    3. If AC is present
        a. Read the battery charge
        b. Start charging if battery is not fully charged
    4. If AC is absent
        a. Inhibit charging by turning off charger and CHGEN pin
    5. If battery is absent, generate battery error
    */
    ltc4100->getChargerStatus(&status);
    ltc4100->getIsBatteryPresent(&batteryStatus);
    ltc4100->getIsAcPresent(&acStatus);

    if((uint32_t)(BATTERY_PRESENT) == batteryStatus)
    {
        // Check if previously battery was not available

        // battery is available, read the full capacity
        if(fullCapacity == 0u)
        {
            battery->getMainParameters();
            battery->getValue(eFullChargeCapacity, &fullCapacity);
        }

        if((uint32_t)(AC_PRESENT) == acStatus)
        {
            // Set if a charger is connected to the PV624
            setPvIsCharging(2416u);

            /* Both AC and battery are present
            So, read the battery percentage */
            battery->readRemainingCapacity(&capacity);

            if(capacity < fullCapacity)
            {
                /* Current capacity is less than full so start charging */
                chargingStatus = eBatteryCharging;
                startCharging();
                clearAllBatteryErrors(2434u);
            }

            else
            {
                /* Current capacity is equal to or more than full so stop charging */
                chargingStatus = eBatteryDischarging;
                stopCharging();
            }
        }

        else
        {
            /* Current capacity is equal to or more than full so stop charging */
            chargingStatus = eBatteryDischarging;
            stopCharging();
        }
    }

    else
    {
        // Battery is not found, generate error
        PV624->handleError(E_ERROR_BATTERY_COMM, eSetError, 0u, 2422u, true);
    }
}

/**
 * @brief   Starts battery charging
 * @param   void
 * @return  bool status - always returns true
 */
bool DPowerManager::startCharging(void)
{
    bool status = true;

    ltc4100->startCharging();

    PV624->userInterface->updateBatteryStatus(5000u, 0u);

    setPvIsCharging(2418u);

    return status;
}

/**
 * @brief   Stops battery charging
 * @param   void
 * @return  bool status - always returns true
 */
bool DPowerManager::stopCharging(void)
{
    bool status = true;

    ltc4100->stopCharging();

    setPvIsDischarging(2419u);

    return status;
}

/**
 * @brief   Keeps charging the battery
 * @param   void
 * @return  bool status - always returns true
 */
bool DPowerManager::keepCharging(void)
{
    eLtcError_t ltcError = eLtcSuccess;

    ltc4100->keepCharging();

    return ltcError;
}

/**
 * @brief   Keeps discharging
 * @param   void
 * @return  bool status - always returns true
 */
bool DPowerManager::keepDischarging(void)
{
    bool status = true;

    stopCharging();

    return status;
}

/**
 * @brief   Sets the charging bit in the PV624 status to indicate the PV624 is charging
 * @param   uint32_t errorInstace - used as a debug parameter to find where the error was generated
            Engineering use only
 * @return  void
 */
void DPowerManager::setPvIsDischarging(uint32_t errInstance)
{
    PV624->errorHandler->handleError(E_ERROR_CHARGER_CONNECTED,
                                     eClearError,
                                     0u,
                                     errInstance,
                                     false);
}

/**
 * @brief   Resets the charging bit in the PV624 status to indicate the PV624 is dis charging
 * @param   uint32_t errorInstace - used as a debug parameter to find where the error was generated
            Engineering use only
 * @return  void
 */
void DPowerManager::setPvIsCharging(uint32_t errInstance)
{
    PV624->errorHandler->handleError(E_ERROR_CHARGER_CONNECTED,
                                     eSetError,
                                     0u,
                                     errInstance,
                                     false);
}

/**
 * @brief   Clears all battery related errors of critical battery / warning level
 * @param   void
 * @return  void
 */
void DPowerManager::clearAllBatteryErrors(uint32_t errInstance)
{
    PV624->handleError(E_ERROR_BATTERY_WARNING_LEVEL,
                       eClearError,
                       0.0f,
                       errInstance);

    PV624->handleError(E_ERROR_BATTERY_CRITICAL_LEVEL,
                       eClearError,
                       0.0f,
                       errInstance);
}

/**
 * @brief   Set battery warning level error - automatically clears critical battery error as both are exclusive
 * @param   void
 * @return  void
 */
void DPowerManager::setBatteryWarningError(float32_t percentage, uint32_t errInstance)
{
    PV624->handleError(E_ERROR_BATTERY_CRITICAL_LEVEL,
                       eClearError,
                       percentage,
                       errInstance);

    PV624->handleError(E_ERROR_BATTERY_WARNING_LEVEL,
                       eSetError,
                       percentage,
                       errInstance);
}

/**
 * @brief   Set battery critical level error - automatically clears warning battery error as both are exclusive
 * @param   void
 * @return  void
 */
void DPowerManager::setBatteryCriticalError(float32_t percentage, uint32_t errInstance)
{
    PV624->handleError(E_ERROR_BATTERY_WARNING_LEVEL,
                       eClearError,
                       percentage,
                       eDataTypeFloat,
                       errInstance);

    PV624->handleError(E_ERROR_BATTERY_CRITICAL_LEVEL,
                       eSetError,
                       percentage,
                       eDataTypeFloat,
                       errInstance);
}

/**
 * @brief   Checks the voltage levels of the 24V, 5.5V and 5V supplies of the PV624. Also compares the read levels with
            voltage thresholds and generates / clears errors if there are any
 * @param   void
 * @return  void
 */
void DPowerManager::checkVoltages(void)
{
    bool retStatus = false;
    float32_t voltageValue = 0.0f;

    retStatus = getValue(EVAL_INDEX_BATTERY_5VOLT_VALUE, &voltageValue);

    if(true == retStatus)
    {
        if(voltageValue < refSensorVoltageThreshold)
        {
            PV624->handleError(E_ERROR_LOW_REFERENCE_SENSOR_VOLTAGE,
                               eSetError,
                               voltageValue,
                               2412u);
        }

        else
        {
            PV624->handleError(E_ERROR_LOW_REFERENCE_SENSOR_VOLTAGE,
                               eClearError,
                               voltageValue,
                               2413u);
        }
    }

    retStatus = getValue(EVAL_INDEX_BATTERY_24VOLT_VALUE, &voltageValue);

    if(true == retStatus)
    {
        if(voltageValue < motorVoltageThreshold)
        {
            PV624->handleError(E_ERROR_MOTOR_VOLTAGE,
                               eSetError,
                               voltageValue,
                               2414u);
        }

        else
        {
            PV624->handleError(E_ERROR_MOTOR_VOLTAGE,
                               eClearError,
                               voltageValue,
                               2415u);
        }
    }
}

/**
 * @brief   Reads different values as a floating point number
 * @param   void
 * @return  void
 */
bool DPowerManager::getValue(eValueIndex_t index, float32_t *value)   //get specified floating point function value
{
    bool successFlag = false;

    switch(index)
    {
    case EVAL_INDEX_BATTERY_TEMPERATURE:
    case EVAL_INDEX_BATTERY_VOLTAGE:
    case EVAL_INDEX_BATTERY_CURRENT:
    case EVAL_INDEX_DESIRED_CHARGING_CURRENT:
    case EVAL_INDEX_DESIRED_CHARGING_VOLTAGE:
    case EVAL_INDEX_REMAINING_BATTERY_CAPACITY: //RemainingCapacity
    case EVAL_INDEX_REMAINING_BATTERY_CAPACITY_WHEN_FULLY_CHARGED: //RemainingCapacity()
        break;

    case EVAL_INDEX_BATTERY_5VOLT_VALUE:
        successFlag = voltageMonitor->getVoltage(eVoltageLevelFiveVolts, value);
        break;

    case EVAL_INDEX_BATTERY_6VOLT_VALUE:
        successFlag = voltageMonitor->getVoltage(eVoltageLevelSixVolts, value);
        break;

    case EVAL_INDEX_BATTERY_24VOLT_VALUE:
        successFlag = voltageMonitor->getVoltage(eVoltageLevelTwentyFourVolts, value);
        break;

    default:
        successFlag = false;
        break;
    }

    return successFlag;
}

/**
 * @brief   Reads different values as a unsigned long number
 * @param   void
 * @return  void
 */
bool DPowerManager::getValue(eValueIndex_t index, uint32_t *value)    //get specified integer function value
{
    bool successFlag = false;
    DLock is_on(&myMutex);
    successFlag = true;
    eVoltageStatus_t status;
    uint32_t val = 0u;

    switch(index)
    {
    case EVAL_INDEX_REMAINING_BATTERY_PERCENTAGE: //RelativeStateOfCharge
    case EVAL_INDEX_REMAINING_BATTERY_LIFE://RunTimeToEmpty
    case EVAL_INDEX_TIME_REQUIRED_FOR_FULL_CHARGE://AverageTimeToFull
    case EVAL_INDEX_BATTERY_STATUS_INFO:
    case EVAL_INDEX_CHARGE_DISCHARGE_CYCLE_COUNT:
        break;

    case EVAL_INDEX_BATTERY_SERIAL_NUMBER:
        battery->getValue(eSerialNumber, &val);
        *value = val;
        break;

    case EVAL_INDEX_BATTERY_5VOLT_STATUS:
        successFlag = voltageMonitor->getVoltageStatus(eVoltageLevelFiveVolts, (eVoltageStatus_t *)&status);

        if((eVoltageStatus_t)eVoltageStatusOK == status)
        {
            *value = 1u;
        }

        else if((eVoltageStatus_t)eVoltageStatusNotOK == status)
        {
            *value = 0u;
        }

        else
        {
            *value = 0u;
        }

        break;

    case EVAL_INDEX_BATTERY_6VOLT_STATUS:
        successFlag = voltageMonitor->getVoltageStatus(eVoltageLevelSixVolts, (eVoltageStatus_t *)&status);

        if((eVoltageStatus_t)eVoltageStatusOK == status)
        {
            *value = 1u;
        }

        else if((eVoltageStatus_t)eVoltageStatusNotOK == status)
        {
            *value = 0u;
        }

        else
        {
            *value = 0u;
        }

        break;

    case EVAL_INDEX_BATTERY_24VOLT_STATUS:
        successFlag = voltageMonitor->getVoltageStatus(eVoltageLevelTwentyFourVolts, (eVoltageStatus_t *)&status);

        if((eVoltageStatus_t)eVoltageStatusOK == status)
        {
            *value = 1u;
        }

        else if((eVoltageStatus_t)eVoltageStatusNotOK == status)
        {
            *value = 0u;
        }

        else
        {
            *value = 0u;
        }

        break;

    case EVAL_INDEX_IR_SENSOR_ADC_COUNTS:
        successFlag = voltageMonitor->getAdcCounts(eVoltageLevelNone, value);
        break;

    case E_VAL_INDEX_CHARGING_STATUS:
        *value = chargingStatus;
        successFlag = true;
        break;

    default:
        successFlag = false;
        break;
    }

    return successFlag;
}

/**
 * @brief   get the battery percentage and charginging status
 * @param   *pPercentCapacity    to return percentage capacity
 * @return  *pChargingStatus     to return charging Status
 */
void DPowerManager::getBatLevelAndChargingStatus(float32_t *pPercentCapacity, uint32_t *pChargingStatus)
{
    uint32_t remCapacity = 0u;
    uint32_t fullCapacity = 0u;
    float32_t percentCap = 0.0f;

    battery->getValue(eRemainingCapacity, &remCapacity);
    battery->getValue(eFullChargeCapacity, &fullCapacity);

    percentCap = (float32_t)(remCapacity) * (100.0f) / (float32_t)(fullCapacity);

    *pPercentCapacity = percentCap;
    *pChargingStatus = chargingStatus;
}

/**
  * @brief  Turn on power DC power supply
  * @param  none
  * @retval None
  */
void DPowerManager::turnOnSupply(eVoltageLevels_t supplyLevel)
{
    voltageMonitor->turnOnSupply(supplyLevel);
}

/**
  * @brief  Turn off DC power supply
  * @param  none
  * @retval None
  */
void DPowerManager::turnOffSupply(eVoltageLevels_t supplyLevel)
{
    voltageMonitor->turnOffSupply(supplyLevel);
}

/**
 * @brief   Reads the smart battery temperature
 * @param   *pPercentCapacity to return percentage capacity
 * @param   *pChargingStatus to return charging Status
 * @retval  true / false
 */
bool DPowerManager::getBatTemperature(float32_t *batteryTemperature)
{
    bool successFlag = false;
    eBatteryErr_t error = eBatteryError;
    uint32_t temperatureVal = 0u;

    if(batteryTemperature != NULL)
    {
        error = battery->getValue(eTemperature, &temperatureVal);

        if(error == eBatterySuccess)
        {
            *batteryTemperature = (float32_t)temperatureVal * 0.1f;
            *batteryTemperature = (*batteryTemperature) - KELVIN_TO_CEL;
            successFlag = true;
        }
    }

    return successFlag;
}

/**
  * @brief  SMBUS ERROR callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
void HAL_SMBUS_ErrorCallback(SMBUS_HandleTypeDef *hsmbus)
{
    if(hsmbus->Instance == I2C1)
    {
        PV624->powerManager->postEvent(EV_FLAG_TASK_BATT_CHARGER_ALERT);
    }
}


/**
 * @brief   returns battery communication status
 * @param   void
 * @retval  return true if communicationis success otherwise false
 */
bool DPowerManager::checkBatteryComm(void)
{
    uint8_t dataBuff[10] = { 0 };
    uint32_t retValue = 0u;
    bool successFlag = false;
    battery->getValue(eDeviceName, dataBuff);
    memcpy(batteryName, &dataBuff[1], (sizeof(batteryName) - 1u));
    retValue = compareArrays(battDeviceName, (const uint8_t *)(batteryName), sizeof(batteryName));
    successFlag = (retValue == 1u) ? true : false;
    return successFlag;
}

/**
 * @brief   returns battery charger communication status
 * @param   void
 * @retval  return true if communicationis success otherwise false
 */
bool DPowerManager::checkBatteryChargerComm(void)
{
    bool successFlag = false;
    ltc4100->getLtcVersionInfo(&ltcIdentity);

    if(battChargerIndentity == ltcIdentity)
    {
        successFlag = true;
    }

    return successFlag;
}