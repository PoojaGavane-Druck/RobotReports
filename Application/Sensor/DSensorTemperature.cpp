/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <main.h>
#include <Types.h>
MISRAC_ENABLE

#include "DSensorTemperature.h"
#include "memory.h"
#include "i2c.h"

#define TEMPERATURE_REG         0X00u
#define CFGR_REG               0X01u
#define LOW_LIMIT_REG           0X02u       
#define HIGH_LIMIT_REG          0X03u
#define TEMP_DEVICE_ID_REG      0X0Fu
#define TEMP_DEVICE_ADDR        0X90u
#define TEMP_DEVICE_ID          0X75u
#define READ_WRITE_LEN          0X01u
#define TEMP_DATA_LEN           0x02u
#define ONE_SHOT_CONFIG         0x01u

DSensorTemperature::DSensorTemperature(void)
{
 
    i2cn = I2Cn3;
}

//This function reads temperature sensor ID and returns it.
uint8_t DSensorTemperature :: GetTemperatureSensorDeviceID()
{
    uint8_t value = 0u;  
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
    sensorError = readBytes(TEMP_DEVICE_ID_REG, &value, READ_WRITE_LEN);
    return value;
} 

//Initialise temperature sensor mode to  one shot 
eSensorError_t DSensorTemperature::initialise(void)
{ 
    uint8_t data = 0u;  
    uint8_t regData = 0u;
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
   
    /*Read device ID and verify it is TMP1075 temperature sensor */
    sensorError = readBytes(TEMP_DEVICE_ID_REG, &data, READ_WRITE_LEN);  
    if ((uint8_t)TEMP_DEVICE_ID == regData)
    {
      //write to config reg
      sensorError = writeByte(CFGR_REG, ONE_SHOT_CONFIG);  
    }
    else
    {
    }
    return sensorError;
}

eSensorError_t DSensorTemperature::getTemperature(float *tempInCelcius)
{
  eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
  uint8_t data[2] = {0u, 0u};
  uint16_t temperature = 0u;
  int16_t signedTemp = 0;
  uint16_t dataLen = 2u;

  sensorError = readBytes(TEMPERATURE_REG, data, dataLen);
  temperature = data[1];
  temperature <<= 8;
  temperature |= data[0];
  signedTemp = (int16_t)(temperature );
  *tempInCelcius = (float)(signedTemp) * 0.0625f /(float)(16);
  return sensorError;
}
eSensorError_t DSensorTemperature::close(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Read temperature data from TMP1075 Sensor
 * @param   Register address of I2C device
 * @param   Buffer to keep read value
 * @retval  sensor error code
 */
eSensorError_t DSensorTemperature::readBytes(uint8_t RegAddr, uint8_t *data, uint16_t length)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
    uint16_t devMemSize = 1u;
    HAL_StatusTypeDef I2CStatus = HAL_ERROR;  
    
    
    //uint8_t rxBuf[3];
   
    //uint8_t crc8 = 0u;
    //uint8_t len = 2u;
    
#ifdef NO_HARDWARE_AVAILABLE
    I2CStatus = I2C_ReadBuffer(i2cn, (uint8_t)TEMP_DEVICE_ADDR ,(uint16_t)RegAddr,devMemSize, data, len);
#else
    I2CStatus = I2C_ReadBuffer(i2cn, (uint8_t)TEMP_DEVICE_ADDR ,(uint16_t)RegAddr,devMemSize, data, length,DEF_NON_BLOCKING);
    // I2CStatus = SMBUS_I2C_ReadBuffer((eI2CElement_t) i2cn, TEMP_DEVICE_ADDR, RegAddr, (uint8_t*)&rxBuf[0], (uint8_t)length);
#endif
    if((uint8_t)I2CStatus == (uint8_t)HAL_OK)
     {
        sensorError = E_SENSOR_ERROR_NONE;
     }
    return sensorError;
}

/**
 * @brief   Write data to TMP1075 Sensor
 * @param   Register address of I2C device
 * @param   Value which has to be written
 * @retval  sensor error code
 */
eSensorError_t DSensorTemperature::writeByte(uint8_t regAddr, uint8_t value)
{   
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
    uint8_t readReg = 0u;
    uint16_t devMemSize = 1u;
    HAL_StatusTypeDef I2CStatus = HAL_ERROR;
    
    #ifdef NO_HARDWARE_AVAILABLE
        I2CStatus = I2C_WriteBuffer (i2cn, TEMP_DEVICE_ADDR , (uint16_t)regAddr, devMemSize, &value, (uint8_t)READ_WRITE_LEN);
    #else
        I2CStatus = I2C_WriteBuffer (i2cn, TEMP_DEVICE_ADDR , (uint16_t)regAddr, devMemSize, &value, (uint8_t)READ_WRITE_LEN, DEF_NON_BLOCKING );
    #endif
    if (static_cast<uint8_t>(I2CStatus) == static_cast<uint8_t>(HAL_OK))
    {
	sensorError = readBytes(regAddr,&readReg, (uint16_t)1u);
	if(value == readReg)
	{
		sensorError = E_SENSOR_ERROR_NONE;
	}
	else
	{
		sensorError = E_SENSOR_ERROR_HAL;
	}

    }
    return sensorError;
}