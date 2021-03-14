MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_def.h>
#include <os.h>
#include <Types.h>
MISRAC_ENABLE

#ifndef __DSENSOR_TEMPERATURE_SENSOR_H
#define __DSENSOR_TEMPERATURE_SENSOR_H

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
