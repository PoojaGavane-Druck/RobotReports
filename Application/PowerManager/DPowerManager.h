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
* @file     DPowerManager.h
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     28 April 2020
*
* @brief    The DPowerManager class header file
*/

#ifndef _DPOWER_MANAGER_H
#define _DPOWER_MANAGER_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_def.h>
MISRAC_ENABLE
#include "Types.h"
#include "DTask.h"
#include "DVoltageMonitor.h"
#include "LTC4100.h"
#include "smartBattery.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define BATTERY_PRESENT 1
#define AC_PRESENT 1
#define BATTERY_NOT_AVAILABLE 0x55u
#define BATTERY_AVAILABLE 0xAAu

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    eBatteryDischarging = 0,
    eBatteryCharging
} eBattChargingStatus_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DPowerManager : public DTask
{
protected:
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond
    OS_MUTEX myMutex;

public:
    DPowerManager(SMBUS_HandleTypeDef *smbus, OS_ERR *osErr);       // Class constructor
    ~DPowerManager();                                               // Class destructor
    LTC4100 *ltc4100;                                               // Object to smart battery charger
    smartBattery *battery;                                          // Object to smart battery
    virtual void initialise(void);
    virtual void runFunction(void);
    virtual void cleanUp(void);

    bool getValue(eValueIndex_t index, float32_t *value);           // get specified floating point function value
    bool getValue(eValueIndex_t index, uint32_t *value);            // get specified integer function value
    void getBatLevelAndChargingStatus(float *pPercentCapacity,
                                      uint32_t *pChargingStatus);   // get only battery percentage and charging status

    void turnOnSupply(eVoltageLevels_t supplyLevel);                // To turn the supply voltage ON
    void turnOffSupply(eVoltageLevels_t supplyLevel);               // To turn the supply voltage OFF
    bool getBatTemperature(float *batteryTemperature);              // To read battery temperature
    void resetDataArray(uint8_t *source, uint32_t length);          // used for resetting data arrays
    bool checkBatteryComm(void);                 // returns battery communication status
    bool checkBatteryChargerComm(void);                 // returns battery communication status
    void getValue(eValueIndex_t index, int8_t *batteryManuf, uint32_t bufSize);

private:
    DVoltageMonitor *voltageMonitor;        // voltage monitor object

    uint32_t chargingStatus;                // Charging status flag
    uint32_t fullCapacity;                  // Full capacity value of battery
    uint32_t timeElapsed;                   // timeElapsed variable for performing actions at various times

    void handleChargerAlert(void);          // Handles the SMBUS alert interrupt actions
    void monitorBatteryParams(void);        // monitors battery parameters
    bool startCharging(void);        // Starts charging battery at set voltage and current levels
    bool stopCharging(void);         // Stops charging battey
    bool keepCharging(void);         // Keeps charging the battery by writing voltage and current values
    bool keepDischarging(void);      // Keeps discharging the battery

    uint8_t manufacturerName[LEN_MANUFACTURER_NAME];    // Holds the manufacturer name of the battery
    uint8_t batteryName[LEN_DEVICE_NAME];               // Battery name string
    uint8_t batteryChemistry[LEN_DEVICE_CHEM];          // Battery chemistry string

    uint32_t ltcIdentity;                               // LTC4100 battery charger identity

    void getPowerInfo(void);
    void checkVoltages(void);
    void checkRemainingBattery(void);

    void setPvIsCharging(uint32_t errInstance);
    void setPvIsDischarging(uint32_t errInstance);
    void clearAllBatteryErrors(uint32_t errInstance);
    void setBatteryWarningError(float32_t percentage, uint32_t errInstance);
    void setBatteryCriticalError(float32_t percentage, uint32_t errInstance);


};

#endif // _DPOWER_MANAGER_H
