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
* @file     DVoltageMonitor.h
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     28 January 2021
*
* @brief    Voltage monitor class header file
*/
//*********************************************************************************************************************
#ifndef __D_VOTAGE_MONITOR_H
#define __D_VOTAGE_MONITOR_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <stdint.h>
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_def.h>
MISRAC_ENABLE
/* Defines ----------------------------------------------------------------------------------------------------------*/
#define VOLTAGE_CHANNELS 4u

#define POWER_RAIL_24V  24u         /* 24V Supply monitor */
#define POWER_RAIL_24V_R1 100e3     /* High side potential divider resistor value */
#define POWER_RAIL_24V_R2 11e3      /* Low side potential divider resistor value */
#define VOLTAGE_LIMIT_24V 0.1f      /* Percent */

#define POWER_RAIL_6V 5.5f            /* 6V Supply monitor */
#define POWER_RAIL_6V_R1 10e3       /* High side potential divider resistor value */
#define POWER_RAIL_6V_R2 11e3       /* Low side potential divider resistor value */
#define VOLTAGE_LIMIT_6V 0.05f      /* Percent */

#define POWER_RAIL_5V 5u            /* 5V Supply monitor */
#define POWER_RAIL_5V_R1 10e3       /* High side potential divider resistor value */
#define POWER_RAIL_5V_R2 10e3       /* Low side potential divider resistor value */
#define VOLTAGE_LIMIT_5V 0.05f      /* Percent */

#define ADC_REFERENCE_VOLTAGE 3.3F  /* ADC reference voltage */
#define ADC_RESOLUTION 4096         /* 12 bit ADC = 2^12 = 4096 */
#define ADC_ENOB 0x0F               /* Ignore 4 LSBs */

#define VOLTAGE_LIMIT 0.1f           /* Percent */
/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    eVoltageLevelNone = 0,
    eVoltageLevelTwentyFourVolts,
    eVoltageLevelSixVolts,
    eVoltageLevelFiveVolts,
    eVoltageLevelsEnumMax
} eVoltageLevels_t;

typedef enum
{
    eVoltageStatusNone = 0,
    eVoltageStatusOK,
    eVoltageStatusNotOK,
    eVoltageStatusMax
} eVoltageStatus_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DVoltageMonitor
{

private:
    uint16_t adcCount[VOLTAGE_CHANNELS];
    float conversionFactor[VOLTAGE_CHANNELS];
    float voltage[VOLTAGE_CHANNELS];
    float voltageLimitHigh[VOLTAGE_CHANNELS];
    float voltageLimitLow[VOLTAGE_CHANNELS];

    eVoltageStatus_t voltageStatus[VOLTAGE_CHANNELS];

    void setVoltage(void);
    void setVoltageStatus(void);
    void initConversionFactors(void);
    void initVoltageLimits(void);

public:
    DVoltageMonitor();
    ~DVoltageMonitor();

    void measurementStart(void);
    void getVoltage(float *voltageValue);
    void getVoltageStatus(eVoltageStatus_t *status);
    bool getVoltage(eVoltageLevels_t VoltageChannel, float *voltageReading);
    bool getVoltageStatus(eVoltageLevels_t voltageChannel, eVoltageStatus_t *status);
    bool getAdcCounts(eVoltageLevels_t voltageChannel, uint32_t *adcCounts);
    void turnOnSupply(eVoltageLevels_t supplyLevel);
    void turnOffSupply(eVoltageLevels_t supplyLevel);
};

#endif /* __DDEVICE_SERIAL_RS485_H */
