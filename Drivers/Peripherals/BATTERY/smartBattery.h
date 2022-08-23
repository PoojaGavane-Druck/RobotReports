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

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

#include "smbus.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
/* Battery address */
#define BATTERY_ADDRESS 0x16    // From SMBUS 1.1 specification

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

#define BATTERY_COMM_TIMEOUT_MS 50 /* MS typical SMBUS timeout is 35 miliseconds*/

#define LEN_MANUFACTURER_NAME 4u
#define LEN_DEVICE_NAME 10u
#define LEN_DEVICE_CHEM 5u
/* Types --------------------------------------------------------------------*/
typedef enum : uint32_t
{
    BATTERY_LEVEL_None = 0,
    BATTERY_LEVEL_0_TO_10,
    BATTERY_LEVEL_10_TO_20,
    BATTERY_LEVEL_20_TO_45,
    BATTERY_LEVEL_45_TO_70,
    BATTERY_LEVEL_70_TO_100
} eBatteryLevel_t;

typedef enum : uint32_t
{
    E_BATTERY_ERROR_NONE = 0u,
    E_BATTERY_ERROR_HAL,
    E_BATTERY_ERROR_OS

} eBatteryError_t;

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
} eBatteryCommands_t;

typedef enum : uint32_t
{
    eBatterySuccess = 0,
    eBatteryError
} eBatteryErr_t;

/* Variables ----------------------------------------------------------------*/
class smartBattery
{
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
    float32_t voltage;
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
    float32_t percentageLife;

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


    eBatteryErr_t setBatteryStatus(uint32_t status);
    eBatteryErr_t setBatteryErrors(uint32_t status);
    eBatteryErr_t writeParameter(eBatteryCommands_t commandCode, uint32_t data);
    eBatteryErr_t readParameter(eBatteryCommands_t commandCode, uint32_t *data);
    eBatteryErr_t readParameter(eBatteryCommands_t commandCode, uint8_t *data, uint32_t length);

public:
    SMBUS *batterySmbus;
    smartBattery(SMBUS_HandleTypeDef *smbus);
    ~smartBattery();
    void resetBatteryParameters(void);
    eBatteryErr_t readAllParameters();
    eBatteryErr_t getMainParameters();
    eBatteryErr_t getValue(eBatteryCommands_t command, uint32_t *value);
    eBatteryErr_t getValue(eBatteryCommands_t command, int32_t *value);
    eBatteryErr_t getValue(eBatteryCommands_t command, uint8_t *value);
    eBatteryErr_t getValue(eBatteryCommands_t command, float32_t *value);

    eBatteryErr_t writeManufacturerAccess(uint32_t data);
    eBatteryErr_t writeRemainingCapacityAlarm(uint32_t data);
    eBatteryErr_t writeRemainingTimeAlarm(uint32_t data);
    eBatteryErr_t writeBatteryMode(uint32_t data);
    eBatteryErr_t writeAtRate(uint32_t data);

    eBatteryErr_t readManufacturerAccess(uint32_t *data);
    eBatteryErr_t readRemainingCapacityAlarm(uint32_t *data);
    eBatteryErr_t readRemainingTimeAlarm(uint32_t *data);
    eBatteryErr_t readBatteryMode(uint32_t *data);
    eBatteryErr_t readAtRate(uint32_t *data);
    eBatteryErr_t readAtRateTimeToFull(uint32_t *data);
    eBatteryErr_t readAtRateTimeToEmpty(uint32_t *data);
    eBatteryErr_t readAtRateOK(uint32_t *data);
    eBatteryErr_t readTemperature(uint32_t *data);
    eBatteryErr_t readVoltage(float32_t *data);
    eBatteryErr_t readCurrent(int32_t *data);
    eBatteryErr_t readAverageCurrent(uint32_t *data);
    eBatteryErr_t readMaxError(uint32_t *data);
    eBatteryErr_t readRelativeStateOfCharge(uint32_t *data);
    eBatteryErr_t readAbsoluteStateOfCharge(uint32_t *data);
    eBatteryErr_t readRemainingCapacity(uint32_t *data);
    eBatteryErr_t readFullChargeCapacity(uint32_t *data);
    eBatteryErr_t readRunTimeToEmpty(uint32_t *data);
    eBatteryErr_t readAverageTimeToEmpty(uint32_t *data);
    eBatteryErr_t readAverageTimeToFull(uint32_t *data);
    eBatteryErr_t readChargingCurrent(uint32_t *data);
    eBatteryErr_t readChargingVoltage(uint32_t *data);
    eBatteryErr_t readBatteryStatus(uint32_t *data);
    eBatteryErr_t readCycleCount(uint32_t *data);
    eBatteryErr_t readDesignCapacity(uint32_t *data);
    eBatteryErr_t readDesignVoltage(uint32_t *data);
    eBatteryErr_t readSpecificationInfo(uint32_t *data);
    eBatteryErr_t readManufactureDate(uint32_t *data);
    eBatteryErr_t readSerialNumber(uint32_t *data);

    eBatteryErr_t readManufacturerName(uint32_t *data);
    eBatteryErr_t readDeviceName(uint32_t *data);
    eBatteryErr_t readDeviceChemistry(uint32_t *data);
    eBatteryErr_t readManufacturerData(uint32_t *data);

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
};

#endif /* _SMART_BATTERY_ */
