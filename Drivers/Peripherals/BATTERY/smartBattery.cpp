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
* @file     smartBattery.cpp
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     25 May 2021
*
* @brief    LTC4100 Battery Charger Library Source File
*/
//*********************************************************************************************************************

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include <stdbool.h>
#include <stm32l4xx_hal.h>
#include <os.h>
MISRAC_ENABLE

#include "smbus.h"
#include "smartBattery.h"

/* Typedefs -----------------------------------------------------------------*/

/* Defines ------------------------------------------------------------------*/

/* Macros -------------------------------------------------------------------*/

/* Variables ----------------------------------------------------------------*/

/* Prototypes ---------------------------------------------------------------*/

/* User code ----------------------------------------------------------------*/
/*
 * @brief   Smart Battery Class Constructor
 * @param   SMBUS Handle
 * @retval  None
 */
smartBattery::smartBattery(SMBUS_HandleTypeDef *smbus)
{
    batterySmbus = new SMBUS(smbus);

    batteryStatus = (uint32_t)(0);
    batteryAddress = (uint32_t)(BATTERY_ADDRESS);
    battTimeout = (uint32_t)(BATTERY_COMM_TIMEOUT_MS);
    
    resetBatteryParameters();
    getAllParameters();
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   None
 * @retval  eBatteryErr_t
 */
smartBattery::~smartBattery()
{

}

/*
 * @brief   Resets all battery variables
 * @param   None
 * @retval  eBatteryErr_t
 */
void smartBattery::resetBatteryParameters(void)
{
    manufacturerAccess = (uint32_t)(0);
    remainingCapacityAlarm = (uint32_t)(0);
    remaniningTimeAlarm = (uint32_t)(0);
    batteryMode = (uint32_t)(0);
    atRate = (uint32_t)(0);
    atRateTimeToFull = (uint32_t)(0);
    atRateTimeToEmpty = (uint32_t)(0);
    atRateOk = (uint32_t)(0);
    temperature = (uint32_t)(0);
    voltage = (float)(0);
    current = (int32_t)(0);
    averageCurrent = (uint32_t)(0);
    maxError = (uint32_t)(0);
    relativeStateOfCharge = (uint32_t)(0);
    absoluteStateOfCharge = (uint32_t)(0);
    remainingCapacity = (uint32_t)(0);
    fullChargeCapacity = (uint32_t)(0);
    runTimeToEmpty = (uint32_t)(0);
    averageTimeToEmpty = (uint32_t)(0);
    averageTimeToFull = (uint32_t)(0);
    batteryStatus = (uint32_t)(0);
    cycleCount = (uint32_t)(0);
    designCapacity = (uint32_t)(0);
    designVoltage = (uint32_t)(0);
    specificationInfo = (uint32_t)(0);
    manufactureDate = (uint32_t)(0);
    serialNumber = (uint32_t)(0);
    manufacturerData = (uint32_t)(0); 
    overChargedAlarmStatus = (uint32_t)(0);
    terminateChargeAlarmStatus = (uint32_t)(0);
    overTempAlarmStatus = (uint32_t)(0);
    terminateDischargeAlarmStatus = (uint32_t)(0);
    remainingCapacityAlarmStatus = (uint32_t)(0);
    remainingTimeAlarmStatus = (uint32_t)(0);
    initializedStatus = (uint32_t)(0);
    dischargingStatus = (uint32_t)(0);
    fullyChargedStatus = (uint32_t)(0);
    fullyDischargedStatus = (uint32_t)(0);
    errors = (uint32_t)(0);    
    percentageLife = (float)(0);
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getOverChargedAlarm(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = overChargedAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getTerminateChargeAlarm(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = terminateChargeAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getOverTempAlarm(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = overTempAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getTerminateDischargeAlarm(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = terminateDischargeAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getRemainingCapacityAlarmStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = remainingCapacityAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getRemainingTimeAlarmStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = remainingTimeAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getInitializedStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = initializedStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getDischargingStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = dischargingStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getFullyChargedStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = fullyChargedStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getFullyDischargedStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = fullyDischargedStatus;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::getErrorCode(uint32_t *errorCode)
{
    eBatteryErr_t error = eBatteryError;
    *errorCode = errors;
    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
 eBatteryErr_t smartBattery::setBatteryStatus(uint32_t status)
 {
    eBatteryErr_t error = eBatteryError;

    /* Over charged alarm */
    if((status & (uint32_t)(STATUS_ALARM_OVER_CHARGED_MASK)) == 
                              (uint32_t)(STATUS_ALARM_OVER_CHARGED_MASK))
    {
        overChargedAlarmStatus = (uint32_t)(1);
    }
    else
    {
        overChargedAlarmStatus = (uint32_t)(0);
    }
    /* Over terminate charge alarm */
    if((status & (uint32_t)(STATUS_ALARM_TERMINATE_CHARGE_MASK)) == 
                              (uint32_t)(STATUS_ALARM_TERMINATE_CHARGE_MASK))
    {
        terminateChargeAlarmStatus = (uint32_t)(1);
    }
    else
    {
        terminateChargeAlarmStatus = (uint32_t)(0);
    }
    /* Temperature alarm */
    if((status & (uint32_t)(STATUS_ALARM_OVER_TEMP_MASK)) == 
                              (uint32_t)(STATUS_ALARM_OVER_TEMP_MASK))
    {
        overTempAlarmStatus = (uint32_t)(1);
    }
    else
    {
        overTempAlarmStatus = (uint32_t)(0);
    }
    /* Terminate discharge alarm */
    if((status & (uint32_t)(STATUS_ALARM_TERMINATE_DISCHARGE_MASK)) == 
                              (uint32_t)(STATUS_ALARM_TERMINATE_DISCHARGE_MASK))
    {
        terminateDischargeAlarmStatus = (uint32_t)(1);
    }
    else
    {
        terminateDischargeAlarmStatus = (uint32_t)(0);
    }
    /* Remaining capacity alarm */
    if((status & (uint32_t)(STATUS_ALARM_REMAINING_CAPCITY_MASK)) == 
                              (uint32_t)(STATUS_ALARM_REMAINING_CAPCITY_MASK))
    {
        remainingCapacityAlarmStatus = (uint32_t)(1);
    }
    else
    {
        remainingCapacityAlarmStatus = (uint32_t)(0);
    }          
    /* Remaining time alarm */
    if((status & (uint32_t)(STATUS_ALARM_REMAINING_TIME_MASK)) == 
                              (uint32_t)(STATUS_ALARM_REMAINING_TIME_MASK))
    {
        remainingTimeAlarmStatus = (uint32_t)(1);
    }
    else
    {
        remainingTimeAlarmStatus = (uint32_t)(0);
    }   
    /* Status initialized */
    if((status & (uint32_t)(STATUS_INITIALIZED_MASK)) == 
                              (uint32_t)(STATUS_INITIALIZED_MASK))
    {
        initializedStatus = (uint32_t)(1);
    }
    else
    {
        initializedStatus = (uint32_t)(0);
    }   
    /* Status discharging */
    if((status & (uint32_t)(STATUS_DISCAHRGING_MASK)) == 
                              (uint32_t)(STATUS_DISCAHRGING_MASK))
    {
        dischargingStatus = (uint32_t)(1);
    }
    else
    {
        dischargingStatus = (uint32_t)(0);
    }     
    /* Status fully charged */
    if((status & (uint32_t)(STATUS_FULLY_CHARGED_MASK)) == 
                              (uint32_t)(STATUS_FULLY_CHARGED_MASK))
    {
        fullyChargedStatus = (uint32_t)(1);
    }
    else
    {
        fullyChargedStatus = (uint32_t)(0);
    } 
    /* Status fully discharged */
    if((status & (uint32_t)(STATUS_FULLY_DISCHARGED_MASK)) == 
                              (uint32_t)(STATUS_FULLY_DISCHARGED_MASK))
    {
        fullyDischargedStatus = (uint32_t)(1);
    }
    else
    {
        fullyDischargedStatus = (uint32_t)(0);
    }         
    error = eBatterySuccess;
    setBatteryErrors(status);
    return error;
}

/*
 * @brief   Set any errors in the battery 
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::setBatteryErrors(uint32_t status)
{
    eBatteryErr_t error = eBatteryError;

    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  None
 */
eBatteryErr_t smartBattery::setManufacturerAccess(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eManufacturerAccess, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::setRemainingCapacityAlarm(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eRemainingCapacityAlarm, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::setRemainingTimeAlarm(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eRemainingTimeAlarm, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::setBatteryMode(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eBatteryMode, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::setAtRate(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eAtRate, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getManufacturerAccess(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eManufacturerAccess, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getRemainingCapacityAlarm(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRemainingCapacityAlarm, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getRemainingTimeAlarm(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRemainingTimeAlarm, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getBatteryMode(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eBatteryMode, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getAtRate(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAtRate, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getAtRateTimeToFull(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAtRateTimeToFull, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getAtRateTimeToEmpty(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAtRateTimeToEmpty, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getAtRateOK(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAtRateOK, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getTemperature(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eTemperature, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getVoltage(float *data)
{
    eBatteryErr_t error = eBatteryError;
    uint32_t readData = (uint32_t)(0);

    error = getCommand(eVoltage, &readData);
    
    /* Convert mV to V */
    *data = (float)(readData) / (float)(1000);
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getCurrent(int32_t *data)
{
    eBatteryErr_t error = eBatteryError;
    uint32_t readData = (uint32_t)(0);

    error = getCommand(eCurrent, &readData);

    if((uint32_t)(0x8000) < readData)
    {
        *data = (int32_t)(readData)  - (int32_t)(0xFFFF) - (int32_t)(1);
    }
    else
    {
        *data = (int32_t)(readData);
    }
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getAverageCurrent(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAverageCurrent, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getMaxError(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eMaxError, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getRelativeStateOfCharge(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRelativeStateOfCharge, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getAbsoluteStateOfCharge(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAbsoluteStateOfCharge, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getRemainingCapacity(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRemainingCapacity, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getFullChargeCapacity(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eFullChargeCapacity, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getRunTimeToEmpty(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRunTimeToEmpty, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getAverageTimeToEmpty(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAverageTimeToEmpty, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getAverageTimeToFull(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAverageTimeToFull, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getChargingCurrent(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    //error = getCommand(eChargingCurrent, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getChargingVoltage(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    //error = getCommand(eChargingVoltage, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getBatteryStatus(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;
    
    error = getCommand(eBatteryStatus, data);
    error = setBatteryStatus(*data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getCycleCount(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eCycleCount, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getDesignCapacity(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eDesignCapacity, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getDesignVoltage(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eDesignVoltage, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getSpecificationInfo(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eSpecificationInfo, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getManufactureDate(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eManufactureDate, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getSerialNumber(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eSerialNumber, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getManufacturerName(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eManufacturerName, data);
    
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getDeviceName(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eDeviceName, data);
    
    return error;    
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getDeviceChemistry(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eDeviceChemistry, data);
    
    return error;    
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::getManufacturerData(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eManufacturerData, data);
    
    return error;
}

/*
 * @brief   Reads all battery parameters
 * @param   eBatteryCommands_t commandCode, uint32_t *data
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getAllParameters(void)
{
    eBatteryErr_t error = eBatteryError;

    error = getMainParameters();

    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Reads main parameters from the battery
 * @param   eBatteryCommands_t commandCode, uint32_t *data
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getMainParameters(void)
{
    eBatteryErr_t error = eBatteryError;

    error = getBatteryStatus(&batteryStatus);
    if(eBatterySuccess == error)
    {
        error = getTemperature(&temperature);
        if(eBatterySuccess == error)
        {
            error = getVoltage(&voltage);
            if(eBatterySuccess == error)
            {
                error = getCurrent(&current);
                if(eBatterySuccess == error)
                {
                    error = getRemainingCapacity(&remainingCapacity);
                    if(eBatterySuccess == error)
                    {
                        error = getFullChargeCapacity(&fullChargeCapacity);

                        percentageLife = (float)(remainingCapacity) * (float)(100) / 
                                                            float(fullChargeCapacity);
                        if(eBatterySuccess == error)
                        {
                            error = getRunTimeToEmpty(&runTimeToEmpty);
                        }                        
                    }
                }
            }
        }
    }

    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Reads uint32_t values from battery
 * @param   eBatteryCommands_t commandCode, uint32_t *data
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getValue(eBatteryCommands_t command, uint32_t *value)
{
    eBatteryErr_t error = eBatteryError;

    switch(command)
    {
    case eManufacturerAccess:
        *value = manufacturerAccess;
        break;

    case eRemainingCapacityAlarm:
        *value = remainingCapacityAlarm;
        break;
            
    case eRemainingTimeAlarm:
        *value = remaniningTimeAlarm;
        break;
            
    case eBatteryMode:
        *value = batteryMode;
        break;
            
    case eAtRate:
        *value = atRate;
        break;
            
    case eAtRateTimeToFull:
        *value = atRateTimeToFull;
        break;
            
    case eAtRateTimeToEmpty:
        *value = atRateTimeToEmpty;
        break;
            
    case eAtRateOK:
        *value = atRateOk;
        break;

    case eTemperature:
        *value = temperature;
        break;
            
    case eAverageCurrent:
        *value = averageCurrent;
        break;
            
    case eMaxError:
        *value = maxError;
        break;
            
    case eRelativeStateOfCharge:
        *value = relativeStateOfCharge;
        break;
            
    case eAbsoluteStateOfCharge:
        *value = absoluteStateOfCharge;
        break;
            
    case eRemainingCapacity:
        *value = remainingCapacity;
        break;
            
    case eFullChargeCapacity:
        *value = fullChargeCapacity;
        break;
            
    case eRunTimeToEmpty:
        *value = runTimeToEmpty;
        break;
            
    case eAverageTimeToEmpty:
        *value = averageTimeToEmpty;
        break;
            
    case eAverageTimeToFull:
        *value = averageTimeToFull;
        break;
            
    case eBatteryStatus:
        *value = batteryStatus;
        break;
            
    case eCycleCount:
        *value = cycleCount;
        break;
            
    case eDesignCapacity:
        *value = designCapacity;
        break;
            
    case eDesignVoltage:
        *value = designVoltage;
        break;
            
    case eSpecificationInfo:
        *value = specificationInfo;
        break;
            
    case eManufactureDate:
        *value = manufactureDate;
        break;
            
    case eSerialNumber:
        *value = serialNumber;
        break;

    default:      
        break;
    }

    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Reads uint32_t values from battery
 * @param   eBatteryCommands_t commandCode, uint32_t *data
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getValue(eBatteryCommands_t command, int32_t *value)
{
    eBatteryErr_t error = eBatteryError;

    switch(command)
    {
    case eCurrent:
        *value = current;
        break;
    default:
        break;
    }   

    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Reads character values from battery
 * @param   eBatteryCommands_t commandCode, uint32_t *data
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getValue(eBatteryCommands_t command, uint8_t *value)
{
    eBatteryErr_t error = eBatteryError;

    switch(command)
    {
    case eManufacturerName:
        break;
    case eDeviceName:
        break;
    case eDeviceChemistry:
        break;
    case eManufacturerData:
        break;
    default:
        break;
    }

    error = eBatterySuccess;
    return error;
}

/*
 * @brief   Reads floating values from battery
 * @param   eBatteryCommands_t commandCode, uint32_t *data
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getValue(eBatteryCommands_t command, float *value)
{
    eBatteryErr_t error = eBatteryError;
    error = eBatterySuccess;
    switch(command)
    {            
    case eVoltage:
        *value = voltage;
        break;
    
    case ePercentage:
        *value = percentageLife;
        break;

    default:
        break;
    }
        
    return error;
}

/*
 * @brief   Smart Battery Class Destructor
 * @param   smBus reference
 * @retval  void
 */
eBatteryErr_t smartBattery::setCommand(eBatteryCommands_t commandCode, uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    /* Write a word to the LTC4100 */
    batterySmbus->smBusWriteWord((uint8_t)(batteryAddress), 
                              (uint8_t*)(&commandCode), 
                              (uint16_t*)(&data));

    error = eBatterySuccess;

    return error;
}

/*
 * @brief   Used to read battery registers
 * @param   eBatteryCommands_t commandCode, uint32_t *data
 * @retval  eBatteryErr_t
 */
eBatteryErr_t smartBattery::getCommand(eBatteryCommands_t commandCode, uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = (eBatteryErr_t)(batterySmbus->smBusReadWord((uint8_t)(batteryAddress),
                                (uint8_t*)(&commandCode),
                                (uint16_t*)(data)));

    error = eBatterySuccess;
    return error;
}