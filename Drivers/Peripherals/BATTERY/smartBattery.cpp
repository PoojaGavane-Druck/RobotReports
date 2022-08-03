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

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include <stdbool.h>
#include <stm32l4xx_hal.h>
#include <os.h>
MISRAC_ENABLE
#include "Types.h"
#include "smbus.h"
#include "smartBattery.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   Smart Battery Class Constructor
 * @param   SMBUS Handle - handle to SMBUS I2C peripheral required by battery
 * @retval  None
 */
smartBattery::smartBattery(SMBUS_HandleTypeDef *smbus)
{
    batterySmbus = new SMBUS(smbus);

    batteryStatus = 0u;
    batteryAddress = (uint32_t)(BATTERY_ADDRESS);
    battTimeout = (uint32_t)(BATTERY_COMM_TIMEOUT_MS);
    
    resetBatteryParameters();
    getAllParameters();
}

/**
 * @brief   Smart Battery Class Destructor
 * @param   None
 * @retval  None
 */
smartBattery::~smartBattery()
{

}

/**
 * @brief   Resets all battery variables
 * @param   None
 * @retval  None
 */
void smartBattery::resetBatteryParameters(void)
{
    manufacturerAccess = 0u;
    remainingCapacityAlarm = 0u;
    remaniningTimeAlarm = 0u;
    batteryMode = 0u;
    atRate = 0u;
    atRateTimeToFull = 0u;
    atRateTimeToEmpty = 0u;
    atRateOk = 0u;
    temperature = 0u;
    voltage = 0.0f;
    current = 0;
    averageCurrent = 0u;
    maxError = 0u;
    relativeStateOfCharge = 0u;
    absoluteStateOfCharge = 0u;
    remainingCapacity = 0u;
    fullChargeCapacity = 0u;
    runTimeToEmpty = 0u;
    averageTimeToEmpty = 0u;
    averageTimeToFull = 0u;
    batteryStatus = 0u;
    cycleCount = 0u;
    designCapacity = 0u;
    designVoltage = 0u;
    specificationInfo = 0u;
    manufactureDate = 0u;
    serialNumber = 0u;
    manufacturerData = 0u; 
    overChargedAlarmStatus = 0u;
    terminateChargeAlarmStatus = 0u;
    overTempAlarmStatus = 0u;
    terminateDischargeAlarmStatus = 0u;
    remainingCapacityAlarmStatus = 0u;
    remainingTimeAlarmStatus = 0u;
    initializedStatus = 0u;
    dischargingStatus = 0u;
    fullyChargedStatus = 0u;
    fullyDischargedStatus = 0u;
    errors = 0u;    
    percentageLife = 0.0f;
}

/**
 * @brief   This function returns the value of over charged alarm of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - pointer that holds value of the alarm variable
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getOverChargedAlarm(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = overChargedAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value of terminate charge alarm of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - pointer that holds value of the alarm variable
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getTerminateChargeAlarm(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = terminateChargeAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value of over temperature alarm of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - pointer that holds value of the alarm variable
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getOverTempAlarm(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = overTempAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value of terminate discharge alarm of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - pointer that holds value of the alarm variable
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getTerminateDischargeAlarm(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = terminateDischargeAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value of remaining capacity alarm of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - pointer that holds value of the alarm variable
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getRemainingCapacityAlarmStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = remainingCapacityAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value of remaining time alarm of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - pointer that holds value of the alarm variable
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getRemainingTimeAlarmStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = remainingTimeAlarmStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value of battery initialized status of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - pointer that holds value of the alarm variable
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getInitializedStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = initializedStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value of discharging status of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - 1, charging, 0 discharging
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getDischargingStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = dischargingStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value of fully charged status of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - 1 - fully charged, 0 - not fully charged
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getFullyChargedStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = fullyChargedStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value of fully discharged status of the battery if set after reading get battery 
            parameters
 * @param   uint32_t status - 1 - fully discharged, 0 - discharging
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getFullyDischargedStatus(uint32_t *status)
{
    eBatteryErr_t error = eBatteryError;
    *status = fullyDischargedStatus;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function returns the value current errors in the battery
 * @param   uint32_t *error - pointer to error value
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::getErrorCode(uint32_t *errorCode)
{
    eBatteryErr_t error = eBatteryError;
    *errorCode = errors;
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   This function is called to set class variables of the battery after reading all the battery parameters via
            SMBUS
            These values are then used by the upper layers for logic execution
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
 eBatteryErr_t smartBattery::setBatteryStatus(uint32_t status)
 {
    eBatteryErr_t error = eBatteryError;

    /* Over charged alarm */
    if((status & (uint32_t)(STATUS_ALARM_OVER_CHARGED_MASK)) == 
                              (uint32_t)(STATUS_ALARM_OVER_CHARGED_MASK))
    {
        overChargedAlarmStatus = 1u;
    }
    else
    {
        overChargedAlarmStatus = 0u;
    }
    /* Over terminate charge alarm */
    if((status & (uint32_t)(STATUS_ALARM_TERMINATE_CHARGE_MASK)) == 
                              (uint32_t)(STATUS_ALARM_TERMINATE_CHARGE_MASK))
    {
        terminateChargeAlarmStatus = 1u;
    }
    else
    {
        terminateChargeAlarmStatus = 0u;
    }
    /* Temperature alarm */
    if((status & (uint32_t)(STATUS_ALARM_OVER_TEMP_MASK)) == 
                              (uint32_t)(STATUS_ALARM_OVER_TEMP_MASK))
    {
        overTempAlarmStatus = 1u;
    }
    else
    {
        overTempAlarmStatus = 0u;
    }
    /* Terminate discharge alarm */
    if((status & (uint32_t)(STATUS_ALARM_TERMINATE_DISCHARGE_MASK)) == 
                              (uint32_t)(STATUS_ALARM_TERMINATE_DISCHARGE_MASK))
    {
        terminateDischargeAlarmStatus = 1u;
    }
    else
    {
        terminateDischargeAlarmStatus = 0u;
    }
    /* Remaining capacity alarm */
    if((status & (uint32_t)(STATUS_ALARM_REMAINING_CAPCITY_MASK)) == 
                              (uint32_t)(STATUS_ALARM_REMAINING_CAPCITY_MASK))
    {
        remainingCapacityAlarmStatus = 1u;
    }
    else
    {
        remainingCapacityAlarmStatus = 0u;
    }          
    /* Remaining time alarm */
    if((status & (uint32_t)(STATUS_ALARM_REMAINING_TIME_MASK)) == 
                              (uint32_t)(STATUS_ALARM_REMAINING_TIME_MASK))
    {
        remainingTimeAlarmStatus = 1u;
    }
    else
    {
        remainingTimeAlarmStatus = 0u;
    }   
    /* Status initialized */
    if((status & (uint32_t)(STATUS_INITIALIZED_MASK)) == 
                              (uint32_t)(STATUS_INITIALIZED_MASK))
    {
        initializedStatus = 1u;
    }
    else
    {
        initializedStatus = 0u;
    }   
    /* Status discharging */
    if((status & (uint32_t)(STATUS_DISCAHRGING_MASK)) == 
                              (uint32_t)(STATUS_DISCAHRGING_MASK))
    {
        dischargingStatus = 1u;
    }
    else
    {
        dischargingStatus = 0u;
    }     
    /* Status fully charged */
    if((status & (uint32_t)(STATUS_FULLY_CHARGED_MASK)) == 
                              (uint32_t)(STATUS_FULLY_CHARGED_MASK))
    {
        fullyChargedStatus = 1u;
    }
    else
    {
        fullyChargedStatus = 0u;
    } 
    /* Status fully discharged */
    if((status & (uint32_t)(STATUS_FULLY_DISCHARGED_MASK)) == 
                              (uint32_t)(STATUS_FULLY_DISCHARGED_MASK))
    {
        fullyDischargedStatus = 1u;
    }
    else
    {
        fullyDischargedStatus = 0u;
    }         
    error = eBatterySuccess;
    setBatteryErrors(status);
    return error;
}

/**
 * @brief   Set any errors in the battery in class variable
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t error - always returns eBatterySuccess
 */
eBatteryErr_t smartBattery::setBatteryErrors(uint32_t status)
{
    eBatteryErr_t error = eBatteryError;

    error = eBatterySuccess;
    return error;
}

/**
 * @brief   Set manufacturer access resgister
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t, error - write failed, success - write passed
 */
eBatteryErr_t smartBattery::setManufacturerAccess(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eManufacturerAccess, data);
    
    return error;
}

/**
 * @brief   Sets the remaining capacity alarm in the battery
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t, error - write failed, success - write passed
 */
eBatteryErr_t smartBattery::setRemainingCapacityAlarm(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eRemainingCapacityAlarm, data);
    
    return error;
}

/**
 * @brief   Sets the remaining time alarm value in the battery
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t, error - write failed, success - write passed
 */
eBatteryErr_t smartBattery::setRemainingTimeAlarm(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eRemainingTimeAlarm, data);
    
    return error;
}

/**
 * @brief   Sets the battery mode
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t, error - write failed, success - write passed
 */
eBatteryErr_t smartBattery::setBatteryMode(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eBatteryMode, data);
    
    return error;
}

/**
 * @brief   Sets the charging rate for the battery
 * @param   uint32_t data to be written
 * @retval  eBatteryErr_t, error - write failed, success - write passed
 */
eBatteryErr_t smartBattery::setAtRate(uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    error = setCommand(eAtRate, data);
    
    return error;
}

/**
 * @brief   Reads the manufacturer accessed registers from the battery
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getManufacturerAccess(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eManufacturerAccess, data);
    
    return error;
}

/**
 * @brief   Reads the value of the remaining capacity alarm mAh from the battery
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getRemainingCapacityAlarm(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRemainingCapacityAlarm, data);
    
    return error;
}

/**
 * @brief   Reads the value of the remaining time alarm minutes from the battery
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getRemainingTimeAlarm(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRemainingTimeAlarm, data);
    
    return error;
}

/**
 * @brief   Reads the battery mode
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getBatteryMode(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eBatteryMode, data);
    
    return error;
}

/**
 * @brief   Reads battery at rate
 * @param   smBus reference
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getAtRate(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAtRate, data);
    
    return error;
}

/**
 * @brief   Reads the time to full value at set charging rate from the battery
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getAtRateTimeToFull(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAtRateTimeToFull, data);
    
    return error;
}

/**
 * @brief   Reads the time to empty at the current discharging rate
 * @param   smBus reference
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getAtRateTimeToEmpty(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAtRateTimeToEmpty, data);
    
    return error;
}

/**
 * @brief   Reads if battery can deliver additional energy at current rate for next 10s
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getAtRateOK(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAtRateOK, data);
    
    return error;
}

/**
 * @brief   Reads the battery temperature
 * @param   smBus reference
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getTemperature(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eTemperature, data);
    
    return error;
}

/**
 * @brief   Reads battery voltage
 * @param   pointer to data to be read in mulitples of 1000
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getVoltage(float32_t *data)
{
    eBatteryErr_t error = eBatteryError;
    uint32_t readData = 0u;

    error = getCommand(eVoltage, &readData);
    
    /* Convert mV to V */
    *data = (float32_t)(readData) / (1000.0f);
    return error;
}

/**
 * @brief   Reads the battery current, current is positive if battery charging and negative if discharging
 * @param   smBus reference
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getCurrent(int32_t *data)
{
    eBatteryErr_t error = eBatteryError;
    uint32_t readData = 0u;

    error = getCommand(eCurrent, &readData);

    if(0x8000u < readData)
    {
        *data = (int32_t)(readData)  - 0xFFFF - 1;
    }
    else
    {
        *data = (int32_t)(readData);
    }
    
    return error;
}

/**
 * @brief   Get average current supplied or sourced into the battery
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getAverageCurrent(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAverageCurrent, data);
    
    return error;
}

/**
 * @brief   Reads expected margin of error in the batterys internal measurement
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getMaxError(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eMaxError, data);
    
    return error;
}

/**
 * @brief   Reads remaining battery capacity as a function of full charge capacity in percentage
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getRelativeStateOfCharge(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRelativeStateOfCharge, data);
    
    return error;
}

/**
 * @brief   Reads remaining battery capacity as a function of design capacity in percentage
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getAbsoluteStateOfCharge(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAbsoluteStateOfCharge, data);
    
    return error;
}

/**
 * @brief   Reads the remaining capacity in terms of mAh
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getRemainingCapacity(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRemainingCapacity, data);
    
    return error;
}

/**
 * @brief   Reads full charge capacity in terms of mAh
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getFullChargeCapacity(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eFullChargeCapacity, data);
    
    return error;
}

/**
 * @brief   Returns predicted value of battery run time at the current discharge rate
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getRunTimeToEmpty(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eRunTimeToEmpty, data);
    
    return error;
}

/**
 * @brief   Reads the averaged value of run time at averaged discharge rate of 1 min
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getAverageTimeToEmpty(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAverageTimeToEmpty, data);
    
    return error;
}

/**
 * @brief   Reads the time to full at averaged charging rate of 1 min
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getAverageTimeToFull(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eAverageTimeToFull, data);
    
    return error;
}

/**
 * @brief   Reads the charging current being sourced into the battery
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getChargingCurrent(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    //error = getCommand(eChargingCurrent, data);
    
    return error;
}

/**
 * @brief   Reads the charing voltage being applied to the battery
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getChargingVoltage(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    //error = getCommand(eChargingVoltage, data);
    
    return error;
}

/**
 * @brief   Reads battery status
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getBatteryStatus(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;
    
    error = getCommand(eBatteryStatus, data);
    error = setBatteryStatus(*data);
    
    return error;
}

/**
 * @brief   Reads the number of charge discharge cycles of the battery
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getCycleCount(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eCycleCount, data);
    
    return error;
}

/**
 * @brief   Reads the battery capacity as designed by manufacturer
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getDesignCapacity(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eDesignCapacity, data);
    
    return error;
}

/**
 * @brief   Reads the battery voltage as designed by manufacturer
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getDesignVoltage(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eDesignVoltage, data);
    
    return error;
}

/**
 * @brief   Reads the battery specification 
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getSpecificationInfo(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eSpecificationInfo, data);
    
    return error;
}

/**
 * @brief   Reads the manufacturer date of the battery
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getManufactureDate(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eManufactureDate, data);
    
    return error;
}

/**
 * @brief   Reads the serial number of the battery
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getSerialNumber(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eSerialNumber, data);
    
    return error;
}

/**
 * @brief   Reads the manufactuter name of the battery manufacuterer
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getManufacturerName(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eManufacturerName, data);
    
    return error;
}

/**
 * @brief   Reads the device name of the battery
 * @param   pointer to data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getDeviceName(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eDeviceName, data);
    
    return error;    
}

/**
 * @brief   Reads the device chemistry from the battery
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getDeviceChemistry(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eDeviceChemistry, data);
    
    return error;    
}

/**
 * @brief   Reads the data specific to manufacturer
 * @param   uint32_t *data - pointer to data read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getManufacturerData(uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    error = getCommand(eManufacturerData, data);
    
    return error;
}

/**
 * @brief   Reads all battery parameters
 * @param   None
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getAllParameters(void)
{
    eBatteryErr_t error = eBatteryError;

    error = getMainParameters();

    error = eBatterySuccess;
    return error;
}

/**
 * @brief   Reads main parameters from the battery
 * @param   None
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getMainParameters(void)
{
    eBatteryErr_t error = eBatteryError;
    error = getSerialNumber(&serialNumber);
    if(eBatterySuccess == error)
    {
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

                          percentageLife = (float32_t)(remainingCapacity) * (100.0f) / 
                                                              (float32_t)(fullChargeCapacity);
                          if(eBatterySuccess == error)
                          {
                              error = getRunTimeToEmpty(&runTimeToEmpty);
                          }                        
                      }
                  }
              }
          }
      }
    }
    error = eBatterySuccess;
    return error;
}

/**
 * @brief   Reads uint32_t values from battery
 * @param   eBatteryCommands_t commandCode - battery command code
 * @param   uint32_t *data - pointer to data read from the command code
 * @retval  eBatteryErr_t, error - read failed, success - read passed
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

/**
 * @brief   Reads int32_t values from battery
 * @param   eBatteryCommands_t commandCode - battery command code
 * @param   int32_t *data - pointer to data read from the command code
 * @retval  eBatteryErr_t, error - read failed, success - read passed
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

/**
 * @brief   Reads character values from battery
 * @param   eBatteryCommands_t commandCode - battery command code
 * @param   uint8_t *data - pointer to data read from the command code
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getValue(eBatteryCommands_t command, uint8_t *value)
{
    eBatteryErr_t error = eBatteryError;

    switch(command)
    {
    case eManufacturerName:
        getCommand(command, value, LEN_MANUFACTURER_NAME);
        break;
        
    case eDeviceName:
        getCommand(command, value, LEN_DEVICE_NAME);
        break;
        
    case eDeviceChemistry:
        getCommand(command, value, LEN_DEVICE_CHEM);
        break;
        
    case eManufacturerData:
        break;
        
    default:
        break;
    }

    error = eBatterySuccess;
    return error;
}

/**
 * @brief   Reads floating values from battery
 * @param   eBatteryCommands_t commandCode - battery command code
 * @param   float32_t *value - pointer to data read from the command code
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getValue(eBatteryCommands_t command, float32_t *value)
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

/**
 * @brief   Smart Battery Class Destructor
 * @param   eBatteryCommands_t commandCode - battery command code
 * @param   uint32_t *data - pointer to data read from the command code
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::setCommand(eBatteryCommands_t commandCode, uint32_t data)
{
    eBatteryErr_t error = eBatteryError;

    batterySmbus->smBusWriteWord((uint8_t)(batteryAddress), 
                              (uint8_t*)(&commandCode), 
                              (uint16_t*)(&data));

    error = eBatterySuccess;

    return error;
}

/**
 * @brief   Used to read battery registers
 * @param   eBatteryCommands_t commandCode - battery command code
 *          uint8_t *data - pointer to data read from the command code
 *          uint32_t length - length of data to be read
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getCommand(eBatteryCommands_t commandCode, uint8_t *data, uint32_t length)
{
    eBatteryErr_t error = eBatteryError;
    
    batterySmbus->smBusReadString((uint8_t)(batteryAddress),
                                    (uint8_t*)(&commandCode),
                                    data,
                                    length);
                                   
    return error;
}

/**
 * @brief   Used to read battery registers
 * @param   eBatteryCommands_t commandCode - battery command code
 * @param   uint32_t *data - pointer to data read from the command code
 * @retval  eBatteryErr_t, error - read failed, success - read passed
 */
eBatteryErr_t smartBattery::getCommand(eBatteryCommands_t commandCode, uint32_t *data)
{
    eBatteryErr_t error = eBatteryError;

    batterySmbus->smBusReadWord((uint8_t)(batteryAddress),
                                (uint8_t*)(&commandCode),
                                (uint16_t*)(data));

    error = eBatterySuccess;
    return error;
}