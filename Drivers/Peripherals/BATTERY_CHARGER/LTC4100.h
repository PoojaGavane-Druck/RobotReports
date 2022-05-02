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
* @file     LTC4100.h
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     25 May 2021
*
* @brief    LTC4100 Battery Charger Library Header File
*/

#ifndef __LTC4100_H
#define __LTC4100_H

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

#include "smbus.h"

/* Defines and constants  ---------------------------------------------------*/
/* Defines for charger status bits */
#define CSTATUS_CHARGE_INHIBITED 0x0001
#define CSTATUS_POLLING_ENABLED 0x0002
#define CSTATUS_VOLTAGE_NOTREG 0x0004
#define CSTATUS_CURRENT_NOTREG 0x0008
#define CSTATUS_LEVEL 0x0030
#define CSTATUS_CURRENT_OR 0x0040
#define CSTATUS_VOLTAGE_OR 0x0080
#define CSTATUS_RES_OR 0x0100
#define CSTATUS_RES_COLD 0x0200
#define CSTATUS_RES_HOT 0x0400
#define CSTATUS_RES_UR 0x0800
#define CSTATUS_ALARM_INHIBITED 0x1000
#define CSTATUS_POWER_FAIL 0x2000
#define CSTATUS_BATTERY_PRESENT 0x4000
#define CSTATUS_AC_PRESENT 0x8000

/* Constants for LTC4100 */
#define LTC_CHARGER_LEVEL 0x01
#define LTC_COMM_TIMEOUT_MS 50 /* MS */

/* Charger enable / disable */
#define CHARGE_START 0x00
#define CHARGE_INHIBIT 0x01

/* Types --------------------------------------------------------------------*/
typedef enum
{
    eChargerSpecInfo = 0x11,
    eChargerMode = 0x12,
    eChargerStatus = 0x13,
    eChargingCurrent = 0x14,
    eChargingVoltage = 0x15,
    eAlarmWarning = 0x16,
    eLTCO = 0x3C
} eCommandCode_t;

typedef enum
{
    eLtcOwnAddress = 0x12,
    eLtcArAddress = 0x18
} eLTC4100_Address_t;

typedef enum
{
    eNoError = 0x0000,
    eChargeInhibited = 0x0001,
    eCurrentOverRange = 0x0002,
    eVoltageOverRange = 0x0004,
    eResOverRange = 0x0008,
    eResCold = 0x0010,
    eResHot = 0x0020,
    eResUnderRange = 0x0040,
    eAlarmInhibited = 0x0080,
    ePowerFail = 0x0100,
    eBatteryAbsent = 0x0200,
    eAcAbsent = 0x0400
} eLtcErrors_t;

typedef enum
{
    eChargingVoltage8796mV = 0x225F,
    eChargingVoltage13100mV = 0x332F,
    eChargingVoltage17404mV = 0x43FF,
    eChargingVoltage21708mV = 0x54CF,
    eChargingVoltage27996mV = 0x6D5F
} eChargingVoltage_t;

typedef enum
{
    eChargeCurrent1023mA = 1023,
    eChargeCurrent2047mA = 2047,
    eChargeCurrent3071mA = 3071,
    eChargeCurrent4095mA = 4095
} eChargingCurrent_t;

typedef enum : uint32_t
{
    eLtcErrorNone = 0,
    eLtcErrorInvalidData,
    eLtcErrorSmbus,
    eLtcSuccess
} eLtcError_t;

typedef enum
{
    eChargerPinReset = 0,
    eChargerPinSet
} eChargerPin_t;

/* Variables ----------------------------------------------------------------*/
class LTC4100
{
public:
    LTC4100(SMBUS_HandleTypeDef *smbus);
    ~LTC4100();

    SMBUS *ltcSmbus;

    eLtcError_t getChargerSpecInfo(uint32_t *info);
    eLtcError_t getChargerMode(uint32_t *mode);
    eLtcError_t setChargerMode(uint32_t mode);
    eLtcError_t getChargerStatus(uint32_t *status);
    eLtcError_t setChargingCurrent(uint32_t current);
    eLtcError_t getChargingCurrent(uint32_t *current);
    eLtcError_t setChargingVoltage(uint32_t voltage);
    eLtcError_t getChargingVoltage(uint32_t *voltage);
    eLtcError_t getAlarmWarning(uint32_t *alarmWarning);
    eLtcError_t getLtcVersionInfo(uint32_t *version);
    eLtcError_t getMaxChargeCurrent(eChargingCurrent_t *current);
    eLtcError_t getMaxChargeVoltage(eChargingVoltage_t *voltage);
    eLtcError_t getIsChargeInhibited(uint32_t *status);
    eLtcError_t getIsCurrentOr(uint32_t *status);
    eLtcError_t getIsVoltageOr(uint32_t *status);
    eLtcError_t getIsResOr(uint32_t *status);
    eLtcError_t getIsResCold(uint32_t *status);
    eLtcError_t getIsResHot(uint32_t *status);
    eLtcError_t getIsResUr(uint32_t *status);
    eLtcError_t getIsAlarmInhibited(uint32_t *status);
    eLtcError_t getIsPowerFail(uint32_t *status);
    eLtcError_t getIsBatteryPresent(uint32_t *status);
    eLtcError_t getIsAcPresent(uint32_t *status);
    eLtcError_t startCharging(void);
    eLtcError_t stopCharging(void);
    eLtcError_t keepCharging(void);
    eLtcError_t continueCharging(void);

private:
    uint32_t chargeInhibited;
    uint32_t currentOverRange;
    uint32_t voltageOverRange;
    uint32_t tempOverRange;
    uint32_t tempCold;
    uint32_t tempHot;
    uint32_t tempUnderRange;
    uint32_t alarmInhibited;
    uint32_t powerFailure;
    uint32_t batteryPresent;
    uint32_t acPresent;

    GPIO_TypeDef *chargerPort;
    uint16_t chargerPin;

    eLtcError_t setMaxChargeCurrent(eChargingCurrent_t current);
    eLtcError_t setMaxChargeVoltage(eChargingVoltage_t voltage);
    eLtcError_t setChargerStatusSignals(uint32_t status);
    eLtcError_t setIsChargeInhibited(uint32_t status);
    eLtcError_t setIsCurrentOr(uint32_t status);
    eLtcError_t setIsVoltageOr(uint32_t status);
    eLtcError_t setIsResOr(uint32_t status);
    eLtcError_t setIsResCold(uint32_t status);
    eLtcError_t setIsResHot(uint32_t status);
    eLtcError_t setIsResUr(uint32_t status);
    eLtcError_t setIsAlarmInhibited(uint32_t status);
    eLtcError_t setIsPowerFail(uint32_t status);
    eLtcError_t setIsBatteryPresent(uint32_t status);
    eLtcError_t setIsAcPresent(uint32_t status);
    eLtcError_t setCommand(eCommandCode_t commandCode, uint32_t data);
    eLtcError_t getCommand(eCommandCode_t commandCode, uint32_t *data);
    eLtcError_t enableChargerAlert(void);
    eLtcError_t setChargerPin(eChargerPin_t pinState);

protected:
    eChargingVoltage_t maxChargingVoltage;
    eChargingCurrent_t maxChargingCurrent;
    uint32_t chargerStatus;
    uint32_t ltcCommTimeout;
};

#endif /* __LTC4100_H */