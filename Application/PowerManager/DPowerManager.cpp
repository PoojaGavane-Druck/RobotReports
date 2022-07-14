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
* @file     DSlot.cpp
* @version  1.00.00
* @author   Nageswara Pydisetty / Makarand Deshmukh
* @date     28 April 2020
*
* @brief    The DSlot class source file
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

/* Prototypes -------------------------------------------------------------------------------------------------------*/
const float batteryCriticalLevelThreshold = 5.0f;
const float batteryWarningLevelThreshold = 10.0f;
const float motorVoltageThreshold = 21.0f;
//const float valveVoltageThreshold = 5.9f;
const float refSensorVoltageThreshold = 4.75f;
CPU_STK powerManagerTaskStack[APP_CFG_POWER_MANAGER_TASK_STACK_SIZE];
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

    // Create mutex for resource locking
    char *name = "PowerManager";

    myTaskId = ePowerManagerTask;

    /* Create objects required by task */
    ltc4100 = new LTC4100(smbus);
    battery = new smartBattery(smbus);
    voltageMonitor = new DVoltageMonitor();
    chargingStatus = 0u;

    /* Read the full capacity of the battery */
    battery->getValue(eFullChargeCapacity, &fullCapacity);

    // check and raise error if battery full capacity is read as 0 as battery full capacity cannot be 0
    if(fullCapacity == 0u)
    {
        // Raise battery error
    }

    //handleChargerAlert();
    /* Dont update status here as UI task will still not be running */
    //PV624->userInterface->updateBatteryStatus(5000u, 0u);

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
    eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;


    if(E_BATTERY_ERROR_NONE == batteryErr)
    {
        if(E_BATTERY_ERROR_NONE != batteryErr)
        {
            //ToDO: Set Error Flag
        }
        else
        {
            //PV624->errorHandler->handleError(errorCode);
        }
    }
    else
    {
        //PV624->errorHandler-> handleError(errorCode);
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

    uint32_t terminateCharging = 0u;
    uint32_t fullyChargedStatus = 0u;

    eBatteryErr_t batError = eBatteryError;
    float remainingPercentage = 0.0f;
    float voltageValue = 0.0f;
    bool retStatus = false;

    while(runFlag == true)
    {
#ifdef TASK_HEALTH_MONITORING_IMPLEMENTED
        PV624->keepAlive(myTaskId);
#endif
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
                               29u);
        }

        //check for events
        if(ok)
        {
            // Task runs at timeout only
            if(os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT))
            {
                timeElapsed++;
                battery->getTerminateChargeAlarm(&terminateCharging);
                battery->getFullyChargedStatus(&fullyChargedStatus);

                if((1u == fullyChargedStatus) ||
                        (1u == terminateCharging))
                {
                    if(eBatteryCharging == chargingStatus)
                    {
                        /* We require to write the LTC4100 current and voltage reigsters if the battery is charging */
                        PV624->handleError(E_ERROR_BATTERY_WARNING_LEVEL,
                                           eClearError,
                                           0.0f,
                                           11u);

                        PV624->handleError(E_ERROR_BATTERY_CRITICAL_LEVEL,
                                           eClearError,
                                           0.0f,
                                           eDataTypeFloat,
                                           12u);

                        if((1u == fullyChargedStatus) ||
                                (1u == terminateCharging))
                        {
                            chargingStatus = eBatteryDischarging;
                            stopCharging();
                        }
                    }
                }

                else
                {
                    if(chargingStatus == eBatteryDischarging)
                    {
                        handleChargerAlert();
                    }

                    else
                    {
                        keepCharging();
                    }
                }

                if((uint32_t)(BATTERY_POLLING_INTERVAL) <= timeElapsed)
                {
                    timeElapsed = 0u;
                    batError = battery->getAllParameters();

                    if(eBatteryError == batError)
                    {
                        battery->resetBatteryParameters();
                        PV624->handleError(E_ERROR_BATTERY_COMM,
                                           eSetError,
                                           (uint32_t)batError,
                                           13u);

                    }

                    else
                    {
                        PV624->handleError(E_ERROR_BATTERY_COMM,
                                           eClearError,
                                           (uint32_t)batError,
                                           14u);

                        battery->getValue(ePercentage, &remainingPercentage);

                        if(remainingPercentage <= batteryCriticalLevelThreshold)
                        {

                            PV624->handleError(E_ERROR_BATTERY_CRITICAL_LEVEL,
                                               eSetError,
                                               remainingPercentage,
                                               15u);

                            PV624->handleError(E_ERROR_BATTERY_WARNING_LEVEL,
                                               eClearError,
                                               remainingPercentage,
                                               16u);
                        }

                        else if(remainingPercentage <= batteryWarningLevelThreshold)
                        {

                            PV624->handleError(E_ERROR_BATTERY_WARNING_LEVEL,
                                               eSetError,
                                               remainingPercentage,
                                               17u);

                            PV624->handleError(E_ERROR_BATTERY_CRITICAL_LEVEL,
                                               eClearError,
                                               remainingPercentage,
                                               18u);
                        }

                        else
                        {
                            PV624->handleError(E_ERROR_BATTERY_WARNING_LEVEL,
                                               eClearError,
                                               remainingPercentage,
                                               19u);

                            PV624->handleError(E_ERROR_BATTERY_CRITICAL_LEVEL,
                                               eClearError,
                                               remainingPercentage,
                                               20u);

                        }
                    }
                }

                retStatus = getValue(EVAL_INDEX_BATTERY_5VOLT_VALUE, &voltageValue);

                if(retStatus == retStatus)
                {

                    if(voltageValue < refSensorVoltageThreshold)
                    {

                        PV624->handleError(E_ERROR_LOW_REFERENCE_SENSOR_VOLTAGE,
                                           eSetError,
                                           voltageValue,
                                           21u);
                    }

                    else
                    {

                        PV624->handleError(E_ERROR_LOW_REFERENCE_SENSOR_VOLTAGE,
                                           eClearError,
                                           voltageValue,
                                           22u);
                    }
                }

                retStatus = getValue(EVAL_INDEX_BATTERY_24VOLT_VALUE, &voltageValue);

                if(retStatus == retStatus)
                {
                    if(voltageValue < motorVoltageThreshold)
                    {

                        PV624->handleError(E_ERROR_MOTOR_VOLTAGE,
                                           eSetError,
                                           voltageValue,
                                           23u);
                    }

                    else
                    {

                        PV624->handleError(E_ERROR_MOTOR_VOLTAGE,
                                           eClearError,
                                           voltageValue,
                                           24u);
                    }
                }
            }

            else
            {
                if((actualEvents & EV_FLAG_TASK_BATT_CHARGER_ALERT) == EV_FLAG_TASK_BATT_CHARGER_ALERT)
                {
                    handleChargerAlert();
                }

                else
                {
                    /* For misra */
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
 * @brief   Handles a charger alert condition
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
        // battery is available, read the full capacity

        if(fullCapacity == 0u)
        {
            battery->getMainParameters();
            battery->getValue(eFullChargeCapacity, &fullCapacity);
        }

        if((uint32_t)(AC_PRESENT) == acStatus)
        {
            // Set if a charger is connected to the PV624
            PV624->errorHandler->handleError(E_ERROR_CHARGER_CONNECTED,
                                             eSetError,
                                             0u,
                                             63u,
                                             false);
            /* Both AC and battery are present
            So, read the battery percentage */
            battery->getRemainingCapacity(&capacity);

            if(capacity < fullCapacity)
            {
                /* Current capacity is less than full so start charging */
                chargingStatus = eBatteryCharging;
                startCharging();
                //PV624->userInterface->updateBatteryStatus(5000u, 1u);
            }

            else
            {
                /* Current capacity is equal to or more than full so start charging */
                chargingStatus = eBatteryDischarging;
                stopCharging();
                //PV624->userInterface->updateBatteryStatus(5000u, 0u);
            }
        }

        else
        {
            // Clear if a charger is not connected to the PV624
            PV624->errorHandler->handleError(E_ERROR_CHARGER_CONNECTED,
                                             eClearError,
                                             0u,
                                             64u,
                                             false);
            /* Current capacity is equal to or more than full so start charging */
            chargingStatus = eBatteryDischarging;
            stopCharging();
            //PV624->userInterface->updateBatteryStatus(5000u, 0u);
        }
    }
}

/**
 * @brief   Starts battery charging
 * @param   void
 * @return  void
 */
void DPowerManager::startCharging(void)
{
    eLtcError_t ltcError = eLtcSuccess;


    ltcError = ltc4100->startCharging();

    if(eLtcSuccess != ltcError)
    {

        PV624->handleError(E_ERROR_BATTERY_CHARGER_COMM,
                           eSetError,
                           (uint32_t)ltcError,
                           (uint16_t)25);
    }

    else
    {

        PV624->handleError(E_ERROR_BATTERY_CHARGER_COMM,
                           eClearError,
                           (uint32_t)ltcError,
                           (uint16_t)26);
    }
}

/**
 * @brief   Stops battery charging
 * @param   void
 * @return  void
 */
void DPowerManager::stopCharging(void)
{
    eLtcError_t ltcError = eLtcSuccess;

    ltcError = ltc4100->stopCharging();

    if(eLtcSuccess != ltcError)
    {
        PV624->handleError(E_ERROR_BATTERY_CHARGER_COMM,
                           eSetError,
                           (uint32_t)ltcError,
                           (uint16_t)27);
    }

    else
    {
        PV624->handleError(E_ERROR_BATTERY_CHARGER_COMM,
                           eClearError,
                           (uint32_t)ltcError,
                           (uint16_t)28);
    }
}

/**
 * @brief   Keeps charging the battery
 * @param   void
 * @return  void
 */
void DPowerManager::keepCharging(void)
{
    eLtcError_t ltcError = eLtcSuccess;

    ltcError = ltc4100->keepCharging();
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
void DPowerManager::getBatLevelAndChargingStatus(float32_t *pPercentCapacity,
        uint32_t *pChargingStatus)
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
 * @brief   get  batterytemperature
 * @param   *pPercentCapacity    to return percentage capacity
 * @return  *pChargingStatus     to return charging Status
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