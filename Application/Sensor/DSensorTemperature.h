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
* @file     DSensorTemperature.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    Temperature sensor header file
*/

#ifndef __DSENSOR_TEMPERATURE_SENSOR_H
#define __DSENSOR_TEMPERATURE_SENSOR_H

MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_def.h>
#include <os.h>
#include <Types.h>
MISRAC_ENABLE

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensor.h"
#include "i2c.h"
/* Types ------------------------------------------------------------------------------------------------------------*/
/* Variables -------------------------------------------------------------------------------------------------------*/

class DSensorTemperature : public DSensor
{   eSensorError_t readBytes(uint8_t RegAddr, uint8_t *data,uint16_t dataLen);
    eSensorError_t writeByte(uint8_t regAddr, uint8_t value);
private:
    uint32_t tempSensorIdentity;
     eI2CElement_t i2cn;
public:
    DSensorTemperature(void);
    eSensorError_t close();
    eSensorError_t initialise(void);
    uint16_t GetTemperatureSensorDeviceID();
    eSensorError_t getTemperature(float *tempInCelcius);   
};
#endif /* __DSENSOR_BAROMETER_H */
