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
* @file     DVoltageMonitor.cpp
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     28 January 2021
*
* @brief    Voltage monitor module
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DVoltageMonitor.h"
#include "main.h"
MISRAC_DISABLE
#include "stm32l4xx_hal_adc.h"
MISRAC_ENABLE
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DVoltageMonitor class constructor
 * @param   void
 * @retval  void
 */
DVoltageMonitor::DVoltageMonitor(void)
{
    /* Start the ADC in the constructor to read required
    number of voltage channels */
    HAL_GPIO_WritePin(P24V_EN_PA7_GPIO_Port, P24V_EN_PA7_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(P6V_EN_PB15_GPIO_Port, P6V_EN_PB15_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IR_SENS_DRIVE_PC0_GPIO_Port, IR_SENS_DRIVE_PC0_Pin, GPIO_PIN_SET);
    initConversionFactors();
    initVoltageLimits();
    measurementStart();
}

/**
 * @brief   Calculates the voltage conversion factors
 * @param   void
 * @retval  void
 */
void DVoltageMonitor::initConversionFactors(void)
{
    /* Calculate the conversion factors here to convert the ADC readings to voltage levels */
    /* Conversion formula
    Voltage = (ADC Count * reference voltage / ADC resolution) / conversion factor
    */
    /* Conversion factor = R2 / (R1 + R2) */
    conversionFactor[eVoltageLevelTwentyFourVolts] = (float)(ADC_REFERENCE_VOLTAGE) / (float)(ADC_RESOLUTION);
    conversionFactor[eVoltageLevelSixVolts] = (float)(ADC_REFERENCE_VOLTAGE) / (float)(ADC_RESOLUTION);
    conversionFactor[eVoltageLevelFiveVolts] = (float)(ADC_REFERENCE_VOLTAGE) / (float)(ADC_RESOLUTION);

    conversionFactor[eVoltageLevelTwentyFourVolts] = conversionFactor[eVoltageLevelTwentyFourVolts] /
            ((float)(POWER_RAIL_24V_R2) /
             (float)(POWER_RAIL_24V_R2 + POWER_RAIL_24V_R1));
    conversionFactor[eVoltageLevelSixVolts] = conversionFactor[eVoltageLevelSixVolts] /
            ((float)(POWER_RAIL_6V_R2) /
             (float)(POWER_RAIL_6V_R2 + POWER_RAIL_6V_R1));
    conversionFactor[eVoltageLevelFiveVolts] = conversionFactor[eVoltageLevelFiveVolts] /
            ((float)(POWER_RAIL_5V_R2) /
             (float)(POWER_RAIL_5V_R2 + POWER_RAIL_5V_R1));
}

/**
 * @brief   Calculates the high and low voltage limits
 * @param   void
 * @retval  void
 */
void DVoltageMonitor::initVoltageLimits(void)
{
    /* Calculate the conversion factors here to convert the ADC readings to voltage levels */
    /* Conversion formula
    Voltage = (ADC Count * reference voltage / ADC resolution) / conversion factor
    */
    /* Conversion factor = R2 / (R1 + R2) */
    voltageLimitHigh[eVoltageLevelTwentyFourVolts] = (float)(POWER_RAIL_24V) + ((float)(VOLTAGE_LIMIT_24V) *
            (float)(POWER_RAIL_24V));
    voltageLimitHigh[eVoltageLevelSixVolts] = (float)(POWER_RAIL_6V) + ((float)(VOLTAGE_LIMIT_6V) *
            (float)(POWER_RAIL_6V));
    voltageLimitHigh[eVoltageLevelFiveVolts] = (float)(POWER_RAIL_5V) + ((float)(VOLTAGE_LIMIT_5V) *
            (float)(POWER_RAIL_5V));

    voltageLimitLow[eVoltageLevelTwentyFourVolts] = (float)(POWER_RAIL_24V) + ((float)(VOLTAGE_LIMIT_24V) *
            (float)(POWER_RAIL_24V));
    voltageLimitLow[eVoltageLevelSixVolts] = (float)(POWER_RAIL_6V) + ((float)(VOLTAGE_LIMIT_6V) *
            (float)(POWER_RAIL_6V));
    voltageLimitLow[eVoltageLevelFiveVolts] = (float)(POWER_RAIL_5V) + ((float)(VOLTAGE_LIMIT_5V) *
            (float)(POWER_RAIL_5V));
}

/**
 * @brief   Starts the ADC in continuous mode to read voltages
 *          The ADC writes data directly to memory via DMA
 * @param   void
 * @retval  void
 */
void DVoltageMonitor::measurementStart(void)
{
    /* Provide pointer for the ADC DMA */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcCount, (uint32_t)(VOLTAGE_CHANNELS));
}

/**
 * @brief   Converts ADC count readings to voltage readings using conversion factors
 * @param   void
 * @retval  void
 */
void DVoltageMonitor::setVoltage(void)
{
    uint32_t counter = (uint32_t)(0);
    /* Voltage = (ADC Count * reference voltage / ADC resolution) / conversion factor  */

    for(counter = (uint32_t)(0); counter < (uint32_t)(VOLTAGE_CHANNELS); counter++)
    {
        voltage[counter] = (float)(adcCount[counter]) * conversionFactor[counter];
    }

    setVoltageStatus();
}

/**
 * @brief   Public function can be used to acquire voltage data from another task
 * @param   void
 * @retval  void
 */
void DVoltageMonitor::getVoltage(float *voltageReading)
{
    uint32_t counter = (uint32_t)(0);

    setVoltage();

    for(counter = (uint32_t)(0); counter < (uint32_t)(VOLTAGE_CHANNELS); counter++)
    {
        voltageReading[counter] = voltage[counter];
    }
}

/**
 * @brief   Public function can be used to acquire voltage data from another task
 * @param   void
 * @retval  void
 */
bool DVoltageMonitor::getVoltage(VOLTAGE_LEVELS_t VoltageChannel, float *voltageReading)
{
    bool retVal = false;

    setVoltage();

    if((VoltageChannel < (VOLTAGE_LEVELS_t)eVoltageLevelsEnumMax) && (voltageReading != NULL))
    {
        *voltageReading = voltage[VoltageChannel];
        retVal = true;
    }

    else
    {
        retVal = false;
    }

    return retVal;

}
/**
 * @brief   Sets the status of voltage levels OK or not OK
 * @param   void
 * @retval  void
 */
void DVoltageMonitor::setVoltageStatus(void)
{
    uint32_t channel = (uint32_t)(0);

    for(channel = (uint32_t)(0); channel < (uint32_t)(VOLTAGE_CHANNELS); channel++)
    {
        if((voltage[channel] >= voltageLimitLow[channel]) &&
                (voltage[channel] <= voltageLimitHigh[channel]))
        {
            voltageStatus[channel] = eVoltageStatusOK;
        }

        else
        {
            voltageStatus[channel] = eVoltageStatusNotOK;
        }
    }
}

/**
 * @brief   Public function can be used to acquire voltage status information from
 *          another task
 * @param   VOLTAGE_STATUS_t status
 * @retval  void
 */
void DVoltageMonitor::getVoltageStatus(VOLTAGE_STATUS_t *status)
{
    uint32_t counter = (uint32_t)(0);

    for(counter = (uint32_t)(0); counter < (uint32_t)(VOLTAGE_CHANNELS); counter++)
    {
        status[counter] = voltageStatus[counter];
    }
}
/**
 * @brief   Public function can be used to acquire voltage status information from
 *          another task
 * @param   VOLTAGE_STATUS_t status
 * @retval  void
 */
bool DVoltageMonitor::getVoltageStatus(VOLTAGE_LEVELS_t VoltageChannel, VOLTAGE_STATUS_t *status)
{
    bool retVal = false;

    if((VoltageChannel < (VOLTAGE_LEVELS_t)eVoltageLevelsEnumMax) && (status != NULL))
    {
        *status = voltageStatus[VoltageChannel];
        retVal = true;
    }

    else
    {
        retVal = false;
    }

    return retVal;
}


bool DVoltageMonitor::getAdcCounts(VOLTAGE_LEVELS_t VoltageChannel, uint32_t *adcCounts)
{
    bool retVal = false;

    if(NULL != adcCounts)
    {
        if((VoltageChannel < (VOLTAGE_LEVELS_t)eVoltageLevelsEnumMax) && (adcCounts != NULL))
        {
            *adcCounts = adcCount[VoltageChannel];
            retVal = true;
        }

        else
        {
            *adcCounts = 0u;
            retVal = false;
        }
    }

    return retVal;
}