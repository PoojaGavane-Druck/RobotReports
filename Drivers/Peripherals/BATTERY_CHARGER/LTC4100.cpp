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
* @file     LTC4100.cpp
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

#include "LTC4100.h"
#include "smbus.h"
#include "main.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   LTC4100 Class Constructor
 * @param   smBus reference
 * @retval  void
 */
LTC4100::LTC4100(SMBUS_HandleTypeDef *smbus)
{   
    uint32_t ltcVersion = 0u;
    ltcSmbus = new SMBUS(smbus);
    
    maxChargingVoltage = (eChargingVoltage_t)(0);
    maxChargingCurrent = (eChargingCurrent_t)(0);
    

    chargeInhibited = 0u;
    currentOverRange = 0u;
    voltageOverRange = 0u;
    tempOverRange = 0u;
    tempCold = 0u;
    tempHot = 0u;
    tempUnderRange = 0u;
    alarmInhibited = 0u;    
    powerFailure = 0u;
    batteryPresent = 0u;
    acPresent = 0u;    
    chargerStatus = 0u;
    ltcCommTimeout = (uint32_t)(LTC_COMM_TIMEOUT_MS);

    chargerPort = GPIOB;
    chargerPin = GPIO_PIN_0;

    /* Set the max voltage and current so that it is not exceeded when
    a user writes the values */
    setMaxChargeCurrent(eChargeCurrent1023mA);
    setMaxChargeVoltage(eChargingVoltage13100mV);

    /* Enable SMBUS alert signal for error signalling */
    //enableChargerAlert();

    getLtcVersionInfo(&ltcVersion);
    /* Read initial charger status */
    getChargerStatus(&chargerStatus);
    setChargerPin(eChargerPinReset);
    setChargerMode((uint32_t)(CHARGE_INHIBIT));    
}

/**
 * @brief   destructor
 * @param   NA
 * @retval  none
 */
LTC4100::~LTC4100()
{
  if(NULL != ltcSmbus)
  {
    delete ltcSmbus;
  }
}

/**
 * @brief   Enables the SMBUS alert signal
 * @param   NA
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::enableChargerAlert(void)
{
    eLtcError_t error = eLtcSuccess;
    
    ltcSmbus->smbusEnableAlert();
    
    return error;  
}

/**
 * @brief   Starts battery charging by writing 0 to charge inhibit bit
 * @param   NA
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::startCharging(void)
{
    eLtcError_t error = eLtcSuccess;
    
    /* To start charging we have to write the charging current
    and charging voltage registers and set the inhibit charge bit
    in charger mode register to 0 */
    setChargingCurrent((uint32_t)(eChargeCurrent1023mA));
    setChargingVoltage((uint32_t)(eChargingVoltage13100mV));
    setChargerMode((uint32_t)(CHARGE_START));

    /* CHGEN pin has to be enabled */
    setChargerPin(eChargerPinSet);

    return error;  
}

/**
 * @brief   Keeps battery charging by writing 0 to charge inhibit
 * @param   NA
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::keepCharging(void)
{
    eLtcError_t error = eLtcSuccess;
    
    /* To keep charging we have to write the charging current
    and charging voltage registers and set the inhibit charge bit
    in charger mode register to 0 */
    setChargingCurrent((uint32_t)(eChargeCurrent1023mA));
    setChargingVoltage((uint32_t)(eChargingVoltage13100mV));

    return error;  
}

/**
 * @brief   Stops battery charging by writing 1 to charge inhibit
 * @param   NA
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::stopCharging(void)
{
    eLtcError_t error = eLtcSuccess;
    
    /* CHGEN pin has to be disabled */
    setChargerPin(eChargerPinReset);

    /* To stop charging, we have to write the charge inhibit bit in
    the charger mode register to 1 */
    setChargerMode((uint32_t)(CHARGE_INHIBIT));

    return error;  
}

/**
 * @brief   Continuously writes charging voltage and current
 * @param   NA
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::continueCharging(void)
{
    eLtcError_t error = eLtcSuccess;
    
    /* Writes charging voltage and current continuously to keep the
    battery charging on going */
    setChargingCurrent((uint32_t)(eChargeCurrent1023mA));
    setChargingVoltage((uint32_t)(eChargingVoltage13100mV));

    return error;  
}

/**
 * @brief   Charger pin control, this function writes the charging state to the GPIO pin to enable the charing hardware
 * @param   NA
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setChargerPin(eChargerPin_t pinState)
{
    eLtcError_t error = eLtcSuccess;

    HAL_GPIO_WritePin(chargerPort, chargerPin, (GPIO_PinState)(pinState));

    return error;
}

/**
 * @brief   Sets the value of max charging current
 * @param   Current value in mA
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setMaxChargeCurrent(eChargingCurrent_t current)
{
    eLtcError_t error = eLtcSuccess;
    maxChargingCurrent = current;
    return error;  
}

/**
 * @brief   Returns the value of max charging current in mA
 * @param   Pointer to the max charging current value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getMaxChargeCurrent(eChargingCurrent_t *current)
{
    eLtcError_t error = eLtcSuccess;
    *current = maxChargingCurrent;
    return error;  
}

/**
 * @brief   Sets the value of max charging voltage
 * @param   Max charging voltage value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setMaxChargeVoltage(eChargingVoltage_t voltage)
{
    eLtcError_t error = eLtcSuccess;
    maxChargingVoltage = voltage;
    return error;  
}

/**
 * @brief   Returns the value of max charging voltage
 * @param   Pointer to max charging voltage value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getMaxChargeVoltage(eChargingVoltage_t *voltage)
{
    eLtcError_t error = eLtcSuccess;
    *voltage = maxChargingVoltage;
    return error;  
}

/**
 * @brief   Reads the charger specification 
 * @param   Pointer to charger spec
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getChargerSpecInfo(uint32_t *info)
{
    eLtcError_t error = eLtcSuccess;

    return error;
}

/**
 * @brief   Reads the current charger mode
 * @param   Pointer to charger mode value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getChargerMode(uint32_t *mode)
{
    eLtcError_t error = eLtcSuccess;

    return error;
}

/**
 * @brief   Sets individual charger status bits
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setChargerStatusSignals(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    error = setIsChargeInhibited(status);
    if(eLtcSuccess == error)
    {
      error = setIsCurrentOr(status);
    }
    if(eLtcSuccess == error)
    {
      error = setIsVoltageOr(status);
    }
    if(eLtcSuccess == error)
    {
      error = setIsResOr(status);
    }
    if(eLtcSuccess == error)
    {
      error = setIsResCold(status);
    }
    if(eLtcSuccess == error)
    {
      error = setIsResHot(status);
    }
    if(eLtcSuccess == error)
    {
      error = setIsResUr(status);
    }
    if(eLtcSuccess == error)
    {
      error = setIsAlarmInhibited(status);
    }
    if(eLtcSuccess == error)
    {
      error = setIsPowerFail(status);
    }
    if(eLtcSuccess == error)
    {    
      error = setIsBatteryPresent(status);
    }
    if(eLtcSuccess == error)
    {
      error = setIsAcPresent(status);
    }
    
    return error;
}

/**
 * @brief   Reads current charge inhibit status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsChargeInhibited(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    *status = chargeInhibited;
    
    return error;
}

/**
 * @brief   Sets the charge inhibit status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsChargeInhibited(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_CHARGE_INHIBITED) == (status & (uint32_t)(CSTATUS_CHARGE_INHIBITED)))
    {
        chargeInhibited = 1u;
    }
    else
    {
        chargeInhibited = 0u;
    }    
    
    return error;
}

/**
 * @brief   Reads current overrange status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsCurrentOr(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    *status = currentOverRange;
    
    return error;
}

/**
 * @brief   Sets current overrange status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsCurrentOr(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_CURRENT_OR) == (status & (uint32_t)(CSTATUS_CURRENT_OR)))
    {
        currentOverRange = 1u;
    }
    else
    {
        currentOverRange = 0u;
    }    
    
    return error;
}

/**
 * @brief   Reads voltage overrange status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsVoltageOr(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    *status = voltageOverRange;
    
    return error;
}

/**
 * @brief   Sets voltage overrange status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsVoltageOr(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_VOLTAGE_OR) == (status & (uint32_t)(CSTATUS_VOLTAGE_OR)))
    {
        voltageOverRange = 1u;
    }
    else
    {
        voltageOverRange = 0u;
    }    
    
    return error;
}

/**
 * @brief   Reads the current charger status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsResOr(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    *status = tempOverRange;
    
    return error;
}

/**
 * @brief   Reads temperature overrange status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsResOr(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_RES_OR) == (status & (uint32_t)(CSTATUS_RES_OR)))
    {
        tempOverRange = 1u;
    }
    else
    {
        tempOverRange = 0u;
    }    
    
    return error;
}

/**
 * @brief   Sets temperature overrange status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsResCold(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    *status = tempCold;
    
    return error;
}

/**
 * @brief   Reads temperature cold status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsResCold(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_RES_COLD) == (status & (uint32_t)(CSTATUS_RES_COLD)))
    {
        tempCold = 1u;
    }
    else
    {
        tempCold = 0u;
    }    
    
    return error;
}

/**
 * @brief   Sets temperature cold status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsResHot(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    *status = tempHot;
    
    return error;
}

/**
 * @brief   Reads temperature hot status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsResHot(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_RES_HOT) == (status & (uint32_t)(CSTATUS_RES_HOT)))
    {
        tempHot = 1u;
    }
    else
    {
        tempHot = 0u;
    }    
    
    return error;
}

/**
 * @brief   Sets temperature hot status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsResUr(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    return error;
}

/**
 * @brief   Reads temperature underrange status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsResUr(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_RES_UR) == (status & (uint32_t)(CSTATUS_RES_UR)))
    {
        tempUnderRange = 1u;
    }
    else
    {
        tempUnderRange = 0u;
    }    
    
    return error;
}

/**
 * @brief   Sets temperature underrange status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsAlarmInhibited(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    *status = alarmInhibited;
    
    return error;
}

/**
 * @brief   Reads alarm status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsAlarmInhibited(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_ALARM_INHIBITED) == (status & (uint32_t)(CSTATUS_ALARM_INHIBITED)))
    {
        alarmInhibited = 1u;
    }
    else
    {
        alarmInhibited = 0u;
    }    
    
    return error;
}   
    
/**
 * @brief   Sets alarm status
 * @param   Pointer to charger status value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getChargerStatus(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    if(NULL != status)
    {
      getCommand(eChargerStatus, status);
      setChargerStatusSignals(*status);
    }
    else
    {
      error = eLtcErrorSmbus;
    }
    return error;
}

/**
 * @brief   Sets the value of charging current for battery
 * @param   Pointer to charging current value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setChargingCurrent(uint32_t current)
{
    eLtcError_t error = eLtcSuccess;
    
    error = setCommand(eChargingCurrent, current);

    return error;
}

/**
 * @brief   Reads the value of charging current
 * @param   Pointer to charging current value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getChargingCurrent(uint32_t* current)
{
    eLtcError_t error = eLtcSuccess;

    return error;
}

/**
 * @brief   Sets the charging voltage for the battery
 * @param   Value of the charging voltage
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setChargerMode(uint32_t mode)
{
    eLtcError_t error = eLtcSuccess;

    error = setCommand(eChargerMode, mode);
    
    return error;    
}

/**
 * @brief   Sets the charging voltage for the battery
 * @param   Value of the charging voltage
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setChargingVoltage(uint32_t voltage)
{
    eLtcError_t error = eLtcSuccess;
    
    /* Voltage should be less than or equal to max charging voltage */
    if(voltage <= maxChargingVoltage)
    {
        /* CHeck if voltage is one of the permitted values */
        if(((uint32_t)(eChargingVoltage8796mV) == voltage) ||
            ((uint32_t)(eChargingVoltage13100mV) == voltage) ||
            ((uint32_t)(eChargingVoltage17404mV) == voltage) ||
            ((uint32_t)(eChargingVoltage21708mV) == voltage) ||
            ((uint32_t)(eChargingVoltage27996mV) == voltage))
        {
            error = setCommand(eChargingVoltage, voltage);
        }        
    }
    else
    {
        /* Set error */
        error = eLtcErrorInvalidData;
    }

    return error;
}

/**
 * @brief   Reads the charging voltage from LTC4100
 * @param   Pointer to charging voltage value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getChargingVoltage(uint32_t *voltage)
{
    eLtcError_t error = eLtcSuccess;

    return error;
}

/**
 * @brief   Reads the alarm warning from LTC4100
 * @param   Pointer to alarm warning value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getAlarmWarning(uint32_t *alarmWarning)
{
    eLtcError_t error = eLtcSuccess;

    return error;
}

/**
 * @brief   Reads the version information from LTC4100
 * @param   Pointer to version value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getLtcVersionInfo(uint32_t *version)
{
    eLtcError_t error = eLtcSuccess;

    getCommand(eLTCO, version);
    
    return error;
}

/**
 * @brief   Checks if power has failed and returns the value
 * @param   Pointer to power fail value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsPowerFail(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_POWER_FAIL) == (status & (uint32_t)(CSTATUS_POWER_FAIL)))
    {
        powerFailure = 1u;
    }
    else
    {
        powerFailure = 0u;
    }
    
    return error;
}

/**
 * @brief   Checks if power has failed and returns the value
 * @param   Pointer to power fail value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsPowerFail(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    *status = powerFailure;
    
    return error;
}

/**
 * @brief   Checks if battery is present and returns the value
 * @param   Pointer to battery present value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsBatteryPresent(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_BATTERY_PRESENT) == (status & (uint32_t)(CSTATUS_BATTERY_PRESENT)))
    {
        batteryPresent = 1u;
    }
    else
    {
        batteryPresent = 0u;
    }
    
    return error;
}

/**
 * @brief   Checks if battery is present and returns the value
 * @param   Pointer to battery present value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsBatteryPresent(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;
    
    *status = batteryPresent;

    return error;
}

/**
 * @brief   Checks if AC present and returns the value
 * @param   Pointer to AC present value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setIsAcPresent(uint32_t status)
{
    eLtcError_t error = eLtcSuccess;
    
    if((uint32_t)(CSTATUS_AC_PRESENT) == (status & (uint32_t)(CSTATUS_AC_PRESENT)))
    {
        acPresent = 1u;
    }
    else
    {
        acPresent = 0u;
    }

    return error;
}

/**
 * @brief   Checks if AC present and returns the value
 * @param   Pointer to AC present value
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getIsAcPresent(uint32_t *status)
{
    eLtcError_t error = eLtcSuccess;

    *status = acPresent;
    
    return error;
}

/**
 * @brief   Write a register on the SMBUS
 * @param   Value of data to be written
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::setCommand(eCommandCode_t commandCode, uint32_t data)
{
    eLtcError_t error = eLtcSuccess;

    /* Write a word to the LTC4100 */
    ltcSmbus->smBusWriteWord((uint8_t)(eLtcOwnAddress), 
                              (uint8_t*)(&commandCode), 
                              (uint16_t*)(&data));
    
    return error;
}

/**
 * @brief   Read a register from the SMBUS
 * @param   Pointer to data read
 * @retval  eLtcError_t
 */
eLtcError_t LTC4100::getCommand(eCommandCode_t commandCode, uint32_t *data)
{
    eLtcError_t error = eLtcSuccess;
    
    /* Transmit the LTC4100 address and command code */
    ltcSmbus->smBusReadWord((uint8_t)(eLtcOwnAddress), 
                              (uint8_t*)(&commandCode), 
                              (uint16_t*)(data));
   
    return error;
}