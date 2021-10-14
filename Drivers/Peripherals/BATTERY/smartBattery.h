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
* @file     smartBattery.h
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     25 June 2021
*
* @brief    Smart Battery Library Header File
*/

#ifndef _SMART_BATTERY_
#define _SMART_BATTERY_

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

#include "smbus.h"

/* Defines ------------------------------------------------------------------*/
/* Battery address */
#define BATTERY_ADDRESS 0x16

/* Battery Modes */
#define MODE_INTERNAL_CHARGE_CTLR_MASK 0x0001
#define MODE_PRIMARY_BATTERY_SUPPOR_MASK 0x0002
#define MODE_CONDITION_FLAG_MASK 0x0080
#define MODE_CHARGE_CTLR_ENABLED_MASK 0x0100
#define MODE_CHARGE_CTLR_DISABLE 0x0000
#define MODE_CHARGE_CTLR_ENABLE 0x0100
#define MODE_ALARM_MODE_MASK 0x2000
#define MODE_ALARM_ENABLE 0x0000
#define MODE_ALARM_DISABLE 0x2000
#define MODE_CHARGER_MODE_MASK 0x4000
#define MODE_CHARGER_BRDCSTS_ENABLE 0x0000
#define MODE_CHARGER_BRDCSTS_DISABLE 0x4000
#define MODE_CAPACITY_MODE_MASK 0x8000
#define MODE_CAPACITY_MAH 0x0000
#define MODE_CAPACITY_WH 0x8000

/* Battery Status */
#define STATUS_ERROR_MASK 0x0007
#define STATUS_ERROR_OK 0x0000
#define STATUS_ERROR_BUSY 0x0001
#define STATUS_ERROR_RESERVED_COMMAND 0x0002
#define STATUS_ERROR_UNSUPP_COMMAND 0x0003
#define STATUS_ERROR_ACCESS_DENIED 0x0004
#define STATUS_ERROR_OVER_UNDER_FLOW 0x0005
#define STATUS_ERROR_BAD_SIZE 0x0006
#define STATUS_ERROR_UNKNOWN_ERROR 0x0007
#define STATUS_FULLY_DISCHARGED_MASK 0x0010
#define STATUS_FULLY_CHARGED_MASK 0x0020
#define STATUS_DISCAHRGING_MASK 0x0040
#define STATUS_INITIALIZED_MASK 0x0080
#define STATUS_ALARM_REMAINING_TIME_MASK 0x0100
#define STATUS_ALARM_REMAINING_CAPCITY_MASK 0x0200
#define STATUS_ALARM_TERMINATE_DISCHARGE_MASK 0x0800
#define STATUS_ALARM_OVER_TEMP_MASK 0x1000
#define STATUS_ALARM_TERMINATE_CHARGE_MASK 0x4000
#define STATUS_ALARM_OVER_CHARGED_MASK 0x8000

#define BATTERY_COMM_TIMEOUT_MS 50 /* MS */
/* Types --------------------------------------------------------------------*/

typedef enum
{
    eManufacturerAccess = 0x00,
    eRemainingCapacityAlarm = 0x01,
    eRemainingTimeAlarm = 0x02,
    eBatteryMode = 0x03,
    eAtRate = 0x04,
    eAtRateTimeToFull = 0x05,
    eAtRateTimeToEmpty = 0x06,
    eAtRateOK = 0x07,
    eTemperature = 0x08,
    eVoltage = 0x09,
    eCurrent = 0x0A,
    eAverageCurrent = 0x0B,
    eMaxError = 0x0C,
    eRelativeStateOfCharge = 0x0D,
    eAbsoluteStateOfCharge = 0x0E,
    eRemainingCapacity = 0x0F,
    eFullChargeCapacity = 0x10,
    eRunTimeToEmpty = 0x11,
    eAverageTimeToEmpty = 0x12,
    eAverageTimeToFull = 0x13,
    eBatteryStatus = 0x16,
    eCycleCount = 0x17,
    eDesignCapacity = 0x18,
    eDesignVoltage = 0x19,
    eSpecificationInfo = 0x1A,
    eManufactureDate = 0x1B,
    eSerialNumber = 0x1C,
    eManufacturerName = 0x20,
    eDeviceName = 0x21,
    eDeviceChemistry = 0x22,
    eManufacturerData = 0x23,
    ePercentage = 0x24
}eBatteryCommands_t;

typedef enum: uint32_t
{
    eBatterySuccess = 0,
    eBatteryError
}eBatteryErr_t;

/* Variables ----------------------------------------------------------------*/
class smartBattery
{
public:
    SMBUS *batterySmbus;
    smartBattery(SMBUS_HandleTypeDef *smbus);
    ~smartBattery();

    eBatteryErr_t getAllParameters();
    eBatteryErr_t getMainParameters();
    eBatteryErr_t getValue(eBatteryCommands_t command, uint32_t *value);
    eBatteryErr_t getValue(eBatteryCommands_t command, int32_t *value);
    eBatteryErr_t getValue(eBatteryCommands_t command, uint8_t *value);
    eBatteryErr_t getValue(eBatteryCommands_t command, float *value);

    eBatteryErr_t setManufacturerAccess(uint32_t data);
    eBatteryErr_t setRemainingCapacityAlarm(uint32_t data);
    eBatteryErr_t setRemainingTimeAlarm(uint32_t data);
    eBatteryErr_t setBatteryMode(uint32_t data);
    eBatteryErr_t setAtRate(uint32_t data);

    eBatteryErr_t getManufacturerAccess(uint32_t *data);
    eBatteryErr_t getRemainingCapacityAlarm(uint32_t *data);
    eBatteryErr_t getRemainingTimeAlarm(uint32_t *data);
    eBatteryErr_t getBatteryMode(uint32_t *data);
    eBatteryErr_t getAtRate(uint32_t *data);
    eBatteryErr_t getAtRateTimeToFull(uint32_t *data);
    eBatteryErr_t getAtRateTimeToEmpty(uint32_t *data);
    eBatteryErr_t getAtRateOK(uint32_t *data);
    eBatteryErr_t getTemperature(uint32_t *data);
    eBatteryErr_t getVoltage(float *data);
    eBatteryErr_t getCurrent(int32_t *data);
    eBatteryErr_t getAverageCurrent(uint32_t *data);
    eBatteryErr_t getMaxError(uint32_t *data);
    eBatteryErr_t getRelativeStateOfCharge(uint32_t *data);
    eBatteryErr_t getAbsoluteStateOfCharge(uint32_t *data);
    eBatteryErr_t getRemainingCapacity(uint32_t *data);
    eBatteryErr_t getFullChargeCapacity(uint32_t *data);
    eBatteryErr_t getRunTimeToEmpty(uint32_t *data);
    eBatteryErr_t getAverageTimeToEmpty(uint32_t *data);
    eBatteryErr_t getAverageTimeToFull(uint32_t *data);
    eBatteryErr_t getChargingCurrent(uint32_t *data);
    eBatteryErr_t getChargingVoltage(uint32_t *data);
    eBatteryErr_t getBatteryStatus(uint32_t *data);
    eBatteryErr_t getCycleCount(uint32_t *data);
    eBatteryErr_t getDesignCapacity(uint32_t *data);
    eBatteryErr_t getDesignVoltage(uint32_t *data);
    eBatteryErr_t getSpecificationInfo(uint32_t *data);
    eBatteryErr_t getManufactureDate(uint32_t *data);
    eBatteryErr_t getSerialNumber(uint32_t *data);

    eBatteryErr_t getManufacturerName(uint32_t *data);
    eBatteryErr_t getDeviceName(uint32_t *data);
    eBatteryErr_t getDeviceChemistry(uint32_t *data);
    eBatteryErr_t getManufacturerData(uint32_t *data);

    eBatteryErr_t getOverChargedAlarm(uint32_t *status);
    eBatteryErr_t getTerminateChargeAlarm(uint32_t *status);
    eBatteryErr_t getOverTempAlarm(uint32_t *status);
    eBatteryErr_t getTerminateDischargeAlarm(uint32_t *status);
    eBatteryErr_t getRemainingCapacityAlarmStatus(uint32_t *status);
    eBatteryErr_t getRemainingTimeAlarmStatus(uint32_t *status);
    eBatteryErr_t getInitializedStatus(uint32_t *status);
    eBatteryErr_t getDischargingStatus(uint32_t *status);
    eBatteryErr_t getFullyChargedStatus(uint32_t *status);
    eBatteryErr_t getFullyDischargedStatus(uint32_t *status);
    eBatteryErr_t getErrorCode(uint32_t *errorCode);

private:
    uint32_t battTimeout;
    uint32_t batteryAddress;
    uint32_t manufacturerAccess;
    uint32_t remainingCapacityAlarm;
    uint32_t remaniningTimeAlarm;
    uint32_t batteryMode;
    uint32_t atRate;
    uint32_t atRateTimeToFull;
    uint32_t atRateTimeToEmpty;
    uint32_t atRateOk;
    uint32_t temperature;
    float voltage;
    int32_t current;
    uint32_t averageCurrent;
    uint32_t maxError;
    uint32_t relativeStateOfCharge;
    uint32_t absoluteStateOfCharge;
    uint32_t remainingCapacity;
    uint32_t fullChargeCapacity;
    uint32_t runTimeToEmpty;
    uint32_t averageTimeToEmpty;
    uint32_t averageTimeToFull;
    uint32_t batteryStatus;
    uint32_t cycleCount;
    uint32_t designCapacity;
    uint32_t designVoltage;
    uint32_t specificationInfo;
    uint32_t manufactureDate;
    uint32_t serialNumber;
    uint8_t manufacturerName[3];
    uint8_t deviceName[9];
    uint8_t deviceChemistry[4];
    uint32_t manufacturerData;    
    float percentageLife;

    uint32_t overChargedAlarmStatus;
    uint32_t terminateChargeAlarmStatus;
    uint32_t overTempAlarmStatus;
    uint32_t terminateDischargeAlarmStatus;
    uint32_t remainingCapacityAlarmStatus;
    uint32_t remainingTimeAlarmStatus;
    uint32_t initializedStatus;
    uint32_t dischargingStatus;
    uint32_t fullyChargedStatus;
    uint32_t fullyDischargedStatus;
    uint32_t errors;
    
    void resetBatteryParameters(void);
    eBatteryErr_t setBatteryStatus(uint32_t status);
    eBatteryErr_t setBatteryErrors(uint32_t status);
    eBatteryErr_t setCommand(eBatteryCommands_t commandCode, uint32_t data);
    eBatteryErr_t getCommand(eBatteryCommands_t commandCode, uint32_t *data); 
};

#endif /* _SMART_BATTERY_ */
