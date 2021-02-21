/**
* BHGE Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DSensorBarometer.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     05 June 2020
*
* @brief    The barometer sensor base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensorChipBarometer.h"
#include "utilities.h"
#include "i2c.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
//Regiter Addresses
#define INTERRUPT_CFG_ADDR              0x0B 
#define THS_P_L_ADDR		       0x0C 
#define THS_P_H_ADDR		       0x0D //R/W
#define IF_CTRL_ADDR		       0X0E //R/W
#define WHO_AM_I_ADDR		       0X0F
#define CTRL_REG1_ADDR		       0X10 //R/W
#define CTRL_REG2_ADDR		       0X11  //R/W
#define CTRL_REG3_ADDR		       0X12 //R/W
#define FIFO_CTRL_ADDR		       0X13 //R/W
#define FIFO_WTM_ADDR		       0X14
#define REF_P_L_ADDR		       0X15
#define REF_P_H_ADDR		       0X16
#define RPDS_L_ADDR				   0X18
#define RPDS_H_ADDR				   0X19
#define INT_SOURCE_ADDR		       0X24
#define FIFO_STATUS1_ADDR	       0X25
#define FIFO_STATUS2_ADDR	       0X26
#define STATUS_ADDR				   0X27
#define PRESSURE_OUT_XL_ADDR	   0X28
#define PRESSURE_OUT_L_ADDR	       0X29
#define PRESSURE_OUT_H_ADDR	       0x2A
#define TEMP_OUT_L_ADDR		       0X2B
#define TEMP_OUT_H_ADDR		       0X2C
#define FIFO_DATA_OUT_PRESS_XL_REG 0X78
#define FIFO_DATA_OUT_PRESS_L_REG  0X79
#define FIFO_DATA_OUT_PRESS_H_REG  0X7A
#define FIFO_DATA_OUT_TEMP_L_REG   0X7B
#define FIFO_DATA_OUT_TEMP_H_REG   0X7C

//Register values
#define INTERRUPT_CFG_CONFIG     0x00
#define THS_P_L_CONFIG	        0x00
#define THS_P_H_CONFIG		0x00
#define IF_CTRL_CONFIG		0x00
#define CTRL_REG1_CONFIG		0x00 //enabled one shot
#define CTRL_REG2_CONFIG		0x10 //triggers a single measurement of pressure and temperature
#define LPS22HH_ONE_SHOT		0x11
#define DATA_READY_STATUS	0X33

#define CTRL_REG3_CONFIG		0x04 //DRDY enabled
#define FIFO_CTRL_CONFIG		0X00
#define FIFO_WTM_CONFIG		0X00
#define RPDS_L_CONFIG		0X00
#define RPDS_H_CONFIG		0X00


#define PRESSURE_SENSITIVITY	4096.0f
#define TEMP_SENSITIVITY	        100.0f

//#define LPS22HH_I2C_ADD_H       0xB9U
#define LPS22HH_I2C_ADD_H       0xB8U
#define LPS22HH_I2C_ADD_L       0xB9U

#define LPS22HH_READ_WRITE_LEN  1
#define LPS22HH_DEVICE_ID       0XB3



//static OS_SEM LPS22HH_DRDY;
//static OS_ERR  err_Sen;
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSensorChipBarometer class constructor
 * @param   comms is the reference to the comms instance for this sensor
 * @retval  void
 */

DSensorChipBarometer::DSensorChipBarometer(): DSensor()
{
    //set up high-level sensor fullscale information
    myFsMinimum = 800.0f;    //stated calibrated -ve FS
    myFsMaximum = 1100.0f;   //stated calibrated +ve FS

    myAbsFsMinimum = myFsMinimum; //absolute minimum FS
    myAbsFsMaximum = myFsMaximum; //absolute maximum FS
    

#ifdef NUCLEO_BOARD    
    i2cn = I2Cn2;
#else
    i2cn = I2Cn4;
#endif

}

/**
 * @brief   Initialisation function
 * @param   void
 * @retval  void
 */

/*check return error TODO*/
eSensorError_t DSensorChipBarometer::initialise(void)
{
    uint8_t regData = 0u;
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
    /*Read device ID and verify it is LPS22HH barometer sensor */
    sensorError = readByte((uint8_t)WHO_AM_I_ADDR, &regData);
    
	if ((uint8_t)LPS22HH_DEVICE_ID == regData)
	{
              
              setIdentity((uint32_t)(regData));
              /*Unused register configured to default values*/
              sensorError = writeByte((uint8_t)INTERRUPT_CFG_ADDR, (uint8_t)INTERRUPT_CFG_CONFIG);
	
              if (sensorError == E_SENSOR_ERROR_NONE)
              {
                      sensorError = writeByte((uint8_t)THS_P_L_ADDR, (uint8_t)THS_P_L_CONFIG);
              }
              if (sensorError == E_SENSOR_ERROR_NONE)
              {
                      sensorError = writeByte((uint8_t)THS_P_H_ADDR, (uint8_t)THS_P_H_CONFIG);
              }
              if (sensorError == E_SENSOR_ERROR_NONE)
              {
                      sensorError = writeByte((uint8_t)FIFO_CTRL_ADDR, (uint8_t)FIFO_CTRL_CONFIG);
              }
              if (sensorError == E_SENSOR_ERROR_NONE)
              {
                      sensorError = writeByte((uint8_t)FIFO_WTM_ADDR, (uint8_t)FIFO_WTM_CONFIG);
              }
              if (sensorError == E_SENSOR_ERROR_NONE)
              {
                      sensorError = writeByte((uint8_t)RPDS_L_ADDR, (uint8_t)RPDS_L_CONFIG);
              }
              if (sensorError == E_SENSOR_ERROR_NONE)
              {
                      sensorError = writeByte((uint8_t)RPDS_L_ADDR, (uint8_t)RPDS_L_CONFIG);
              }
              if (sensorError == E_SENSOR_ERROR_NONE)
              {
                      sensorError = writeByte((uint8_t)IF_CTRL_ADDR, (uint8_t)IF_CTRL_CONFIG);
              }
              if (sensorError == E_SENSOR_ERROR_NONE)
              {
                      /**No SPI and one shot configuration selected = 0x00 */
                      sensorError = writeByte((uint8_t)CTRL_REG1_ADDR, (uint8_t)CTRL_REG1_CONFIG);
              }
              if (sensorError == E_SENSOR_ERROR_NONE)
              {
                      /* LOW_NOISE_EN enables for data rate less than 100KHz*/
                      sensorError = writeByte((uint8_t)CTRL_REG2_ADDR, (uint8_t)CTRL_REG2_CONFIG);
              }
              if (sensorError == E_SENSOR_ERROR_NONE)
              {	/*Data ready enabled*/
                      sensorError = writeByte((uint8_t)CTRL_REG3_ADDR, (uint8_t)CTRL_REG3_CONFIG);
              }
		
		
	}
 return sensorError;
}

/**
 * @brief   Close sensor
 * @param   void
 * @retval  void
 */
eSensorError_t DSensorChipBarometer::close(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Perform sensor measurement
 * @param   none
 * @retval  sensor error code
 */
eSensorError_t DSensorChipBarometer::measure()
{
    uint8_t regData = 0u;
    float32_t pressure, temperature;
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;

    /*commented os function as DRDY interrupt is not used for now
    set semaphore for DRDY
    OSSemSet(&LPS22HH_DRDY, (OS_SEM_CTR)0, &err_Sen);
    */
    sensorError = readByte((uint8_t)CTRL_REG2_ADDR, &regData);

    sensorError = writeByte((uint8_t)CTRL_REG2_ADDR, (uint8_t)LPS22HH_ONE_SHOT);
    //OSSemPend(&LPS22HH_DRDY, (OS_TICK)500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &err_Sen);

    //if ((OS_ERR)OS_ERR_NONE == (OS_ERR) err_Sen)
    sensorError = readByte((uint8_t)STATUS_ADDR, &regData);

    if ((E_SENSOR_ERROR_NONE == sensorError) && ((uint8_t)DATA_READY_STATUS == regData))
	{
            //INT_SOURCE HAS TO BE CHECKED  when using data ready interrupt
            //Reconfirm INT_SOURCE source register //typecast all errors
            //sensorError = readByte((uint8_t)INT_SOURCE_ADD, &regData);//TODO
            //if ((sensorError != E_SENSOR_ERROR_NONE) && (regData == uint8_t(0x04)))//(regData & INT_EN4 == INT_EN4)
            //{
            //}

                    
            sensorError = readPresureAndTemp(&pressure, &temperature);
                    
            if (E_SENSOR_ERROR_NONE == sensorError)
             {
              setMeasurement(pressure);
              sensorError = writeByte((uint8_t)CTRL_REG2_ADDR, (uint8_t)CTRL_REG2_CONFIG);// This could be written in interrupt handler
              //sensorError = writeByte((uint8_t)CTRL_REG2, (uint8_t)CTRL_REG2_RESET only bit);// This could be written in interrupt handler
             }
                                            
	}
	return sensorError;
}

/**
 * @brief   Read data pressure and temperature data from LPS22HH Sensor
 * @param   Register address of I2C device
 * @param   Buffer to keep read value
 * @retval  sensor error code
 */
eSensorError_t DSensorChipBarometer::readByte(uint8_t RegAddr, uint8_t *value)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
    uint16_t devMemSize = 1u;
    HAL_StatusTypeDef I2CStatus = HAL_ERROR;
  
#ifdef NO_HARDWARE_AVAILABLE
    I2CStatus = I2C_ReadBuffer(i2cn, (uint8_t)LPS22HH_I2C_ADD_H ,(uint16_t)RegAddr, devMemSize, value, (uint8_t)LPS22HH_READ_WRITE_LEN);
#else
    I2CStatus = I2C_ReadBuffer(i2cn, (uint8_t)LPS22HH_I2C_ADD_H ,(uint16_t)RegAddr, devMemSize, value, (uint8_t)LPS22HH_READ_WRITE_LEN, DEF_NON_BLOCKING);
#endif
    
    if((uint8_t)I2CStatus == (uint8_t)HAL_OK)
     {
        sensorError = E_SENSOR_ERROR_NONE;
     }
    return sensorError;
}

/**
 * @brief   Write data to LPS22HH Sensor
 * @param   Register address of I2C device
 * @param   Value which has to be written
 * @retval  sensor error code
 */
eSensorError_t DSensorChipBarometer::writeByte(uint8_t regAddr, uint8_t value)
{   
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
    uint8_t readReg = 0u;
    uint16_t devMemSize = 1u;
    HAL_StatusTypeDef I2CStatus = HAL_ERROR;
  
    
    #ifdef NO_HARDWARE_AVAILABLE
        I2CStatus = I2C_WriteBuffer (i2cn, LPS22HH_I2C_ADD_H , (uint16_t)regAddr, devMemSize, &value, (uint8_t)LPS22HH_READ_WRITE_LEN);
    #else
        I2CStatus = I2C_WriteBuffer (i2cn, LPS22HH_I2C_ADD_H , (uint16_t)regAddr, devMemSize, &value, (uint8_t)LPS22HH_READ_WRITE_LEN, DEF_NON_BLOCKING );
    #endif
    if (static_cast<uint8_t>(I2CStatus) == static_cast<uint8_t>(HAL_OK))
    {
	sensorError = readByte(regAddr,&readReg);
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

/**
 * @brief   Read pressure and temperature data from LPS22HH and convert it to hpa and celcius respectively
 * @param   Address of  pressure value
 * @param   address of temperature value
 * @retval  sensor error code
 */

eSensorError_t DSensorChipBarometer::readPresureAndTemp(float32_t *pressure_hpa, float32_t *temp_Celcius)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    uint8_t pressOutXL = 0u; 
    uint8_t pressOutL = 0u; 
    uint8_t pressOutH = 0u;
    uint8_t tempOutH = 0u;
    uint8_t tempOutL = 0u;

    uint32_t pressRegData = 0u;
    uint16_t tempRegData = 0u;

    sensorError= readByte((uint8_t)PRESSURE_OUT_XL_ADDR, &pressOutXL);
    sensorError = readByte((uint8_t)PRESSURE_OUT_L_ADDR, &pressOutL);
    sensorError = readByte((uint8_t)PRESSURE_OUT_H_ADDR, &pressOutH);
/*
    pressRegData <<= 8u;
    pressRegData |= pressOutL;
    pressRegData <<= 8u;
    pressRegData |= pressOutXL;
*/
    pressRegData = (uint32_t)pressOutH << ((uint32_t)16) | (uint32_t)pressOutL << ((uint32_t)8) | (uint32_t)pressOutXL; 
    *pressure_hpa = ((float32_t)pressRegData / (float32_t)PRESSURE_SENSITIVITY);

    sensorError= readByte((uint8_t)TEMP_OUT_L_ADDR, &tempOutL);
    sensorError = readByte((uint8_t)TEMP_OUT_H_ADDR, &tempOutH);

    tempRegData = tempOutH;
    tempRegData <<= 8;
    tempRegData |= tempOutL;
  
    //tempRegData = (uint16_t)pressOutL << ((uint16_t)8) | (uint16_t)tempOutH; 
    *temp_Celcius = (float32_t)tempRegData/(float32_t)TEMP_SENSITIVITY;
     return sensorError;
}

/**
 * @brief   Set the identity value of the barometer sensor installed
 * @param   Address of  pressure value
 * @param   address of temperature value
 * @retval  sensor error code
 */
void DSensorChipBarometer::setIdentity(uint32_t identity)
{
    myManfID = identity;
}

/**
 * @brief   Get the identification value of the barometer sensor installed
 * @param   Address of  pressure value
 * @param   address of temperature value
 * @retval  sensor error code
 */
void DSensorChipBarometer::getIdentity(uint32_t *identity)
{
    *identity = myManfID;
}




