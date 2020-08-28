/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DSlot.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlot base class header file
*/

#ifndef _DBATTERY_H
#define _DBATTERY_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

#include "DTask.h"


/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum : uint32_t
{
    E_BATTERY_ERROR_NONE = 0u,
    E_BATTERY_ERROR_HAL,
    E_BATTERY_ERROR_OS

} eBatteryError_t;

typedef enum : uint8_t
{
    E_BATTERY_CMD_MANUFACTURE_ACCESS            = 0X00,		
    E_BATTERY_CMD_REMAINING_CAPACITY_ALARM      = 0X01,
    E_BATTERY_CMD_REMAINING_TIME_ALARM          = 0X02,		
    E_BATTERY_CMD_BATTERY_MODE                  = 0X03,
    E_BATTERY_CMD_AT_RATE                       = 0X04,		
    E_BATTERY_CMD_AT_RATE_TIME_TO_FULL          = 0X05,
    E_BATTERY_CMD_AT_RATE_TIME_TO_EMPTY         = 0X06,
    E_BATTERY_CMD_AT_RATE_OK                    = 0X07,		
    E_BATTERY_CMD_TEMPERATURE                   = 0X08,
    E_BATTERY_CMD_VOLTAGE                       = 0X09,
    E_BATTERY_CMD_CURRENT                       = 0X0A,
    E_BATTERY_CMD_AVERAGE_CURRENT               = 0X0B,
    E_BATTERY_CMD_MAX_ERROR                     = 0X0C,
    E_BATTERY_CMD_RELATIVE_STATE_OF_CHARGE      = 0X0D,
    E_BATTERY_CMD_ABSOLUTE_STATE_OF_CHARGE      = 0X0E,
    E_BATTERY_CMD_REMAINING_CAPACITY            = 0X0F,
    E_BATTERY_CMD_FULL_CHARGE_CAPACITY          = 0X10,
    E_BATTERY_CMD_RUN_TIME_TO_EMPTY             = 0X11,
    E_BATTERY_CMD_AVERAGE_TIME_TO_EMPTY         = 0X12,
    E_BATTERY_CMD_AVERAGE_TIME_TO_FULL          = 0X13,
    E_BATTERY_CMD_CHARGING_CURRENT              = 0X14,
    E_BATTERY_CMD_CHARGING_VOLTAGE              = 0X15,
    E_BATTERY_CMD_BATTERY_STATUS                = 0X16,
    E_BATTERY_CMD_CYCLE_COUNT                   = 0X17,
    E_BATTERY_CMD_DESIGN_CAPACITY               = 0X18,
    E_BATTERY_CMD_DESIGN_VOLTAGE                = 0X19,
    E_BATTERY_CMD_SPECIFICATION_INFO            = 0X1A,
    E_BATTERY_CMD_MANUFACTURER_DATE             = 0X1B,
    E_BATTERY_CMD_SERIAL_NUMBER                 = 0X1C,	
    E_BATTERY_CMD_MANUFACTURER_NAME             = 0X20,
    E_BATTERY_CMD_DEVICE_NAME                   = 0X21,
    E_BATTERY_CMD_DEVICE_CHEMISTRY              = 0X22,
    E_BATTERY_CMD_MANUFACTURER_DATA             = 0X23
} eSmartBatteryCommand_t;
/* Variables --------------------------------------------------------------------------------------------------------*/

class DBattery : public DTask
{
    uint16_t remainingCapacityAlarm;
    uint16_t remainingTimeAlarm;
    uint16_t batteryMode;
    uint16_t rateForTimeToFull;
    uint16_t rateForTimeToEmpty;
    uint16_t isValidrate;
    uint16_t internalTemperature;
    uint16_t myVoltage;
    uint16_t myCurrent;
    uint16_t myAverageCurrent;
    uint16_t expectedMarginOfErr;
    uint16_t relativeStateOfCharge;
    uint16_t absoluteStateOfCharge;
    uint16_t remainingCapacity;
    uint16_t fullChargeCapacity;
    uint16_t remainingBatteryLife;
    uint16_t averageTimeToEmpty;
    uint16_t averageTimeToFull;
    uint16_t chargingCurrent;
    uint16_t chargingVoltage;
    uint16_t batteryStatus;
    uint16_t cycleCount;
    uint16_t designCapacity;
    uint16_t designVoltage;
    uint16_t specificationInfo;
    uint16_t manufactureDate;
    uint16_t serialNumber;
    uint8_t manufactureName[50];
    uint8_t deviceName[50];
    uint8_t battryChemistry[50];
    eBatteryError_t readParam(uint8_t cmdCode, uint16_t *value);
    eBatteryError_t writeParam(uint8_t cmdCode, uint16_t value);
    
protected:
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond
    OS_MUTEX myMutex;

    eBatteryError_t readBatteryInfo(void); 
    eBatteryError_t readBatteryParams(void);

public:
    DBattery(void);
  
    virtual void runFunction(void);
    virtual void cleanUp(void);

    

    

  

};

#endif // _DSLOT_H
