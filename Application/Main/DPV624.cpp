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
* @file     DPV624.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     12 April 2020
*
* @brief    The PV624 instrument class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include "main.h"
#include <os.h>
#include <assert.h>
MISRAC_ENABLE

#include "DPV624.h"
#include "DStepperMotor.h"
#include "DSlot.h"
#include "i2c.h"
#include "uart.h"
#include "Utilities.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

#define MOTOR_FREQ_CLK 12000000u
//#define   NUCLEO_BOARD 0
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
DPV624 *PV624;

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern TIM_HandleTypeDef  htim4;
extern TIM_HandleTypeDef  htim1;

extern const unsigned int cAppDK;
extern const unsigned char cAppVersion[4];


const unsigned int bootloaderFixedRegionRomStart = 0x08000200u; // __FIXED_region_ROM_start__ in bootloader stm32l4r5xx_flash.icf
__no_init const unsigned int cblDK @ bootloaderFixedRegionRomStart + 0u;
__no_init const unsigned char cblVersion[4] @ bootloaderFixedRegionRomStart + 4u;
//__no_init const char cblInstrument[16] @ bootloaderFixedRegionRomStart + 8u;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
#include "Utilities.h"

DPV624::DPV624(void)
{
    OS_ERR os_error;   
    

    //initialise I2C interface (must do this before accessing I2C devices) 


#ifdef NUCLEO_BOARD
    i2cInit(&hi2c2);
#else
    i2cInit(&hi2c1);
    i2cInit(&hi2c3);
    i2cInit(&hi2c4);
#endif    
    
    persistentStorage = new DPersistent();
    
    uartInit(&huart2);
    uartInit(&huart4);  
    uartInit(&huart5);
    
    //create application objects    
    instrument = new DInstrument(&os_error);
    validateApplicationObject(os_error);

    commsOwi = new DCommsOwi("commsOwi", &os_error);
    validateApplicationObject(os_error);

    commsUSB = new DCommsUSB("commsUSB", &os_error);
    validateApplicationObject(os_error);

    powerManager = new DPowerManager();
   
    errorHandler = new DErrorHandler(&os_error);
    validateApplicationObject(os_error);

    keyHandler = new DKeyHandler(&os_error);
    validateApplicationObject(os_error);
#if 0
    stepperController = new DStepperController(&htim2,
                                               TIM_CHANNEL_1,
                                               &htim3, 
                                               TIM_CHANNEL_2,
                                               MOTOR_FREQ_CLK);
#endif
    stepperMotor = new DStepperMotor();
    
    temperatureSensor =new DSensorTemperature();
   // sensorError = temperatureSensor->initialise();
   
    valve1 = new DValve(&htim1, TIM_CHANNEL_1,VALVE1_DIR_PC5_GPIO_Port,VALVE1_DIR_PC5_Pin);
    validateApplicationObject(os_error);

    valve2 = new DValve(&htim4,TIM_CHANNEL_3,VALVE2_DIR_PB1_GPIO_Port,VALVE2_DIR_PB1_Pin);
    validateApplicationObject(os_error);
    
    valve3 = new DValve(&htim4,TIM_CHANNEL_4,VALVE3_DIR_PF11_GPIO_Port,VALVE3_DIR_PF11_Pin);
    validateApplicationObject(os_error);


   /*
    errorHandler = new DErrorHandler(&os_error);
    keyHandler = new DKeyHandler(&os_error);
    userInterface = new DUserInterface(&os_error);
    serialComms = new DCommsSerial("commsSerial", &os_error);
    
    */
    

}

/**
* @brief    handleError - process messaages from the rest of the system
* @param    errorCode - enumerated error code identifier value
* @param    param -    optional parameter value TODO - Not currently used
* @param    blocking - optional parameter if true to stop here for showstopper faults TODO - blocking error won't cause watchdog to trip
* @return   None
*/
void DPV624::handleError(error_code_t errorCode, int32_t param, bool blocking)
{
    if (errorHandler != NULL)
    {
        errorHandler->handleError(errorCode, param, blocking);
    }
}
/**
 * @brief   validates the application object was created without error
 * @retval  None
 */
void DPV624::validateApplicationObject(OS_ERR os_error)
{
    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE));

    if (!ok)
    {
#ifdef ASSERT_IMPLEMENTED
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
#endif
        if ((PV624 != NULL) && (errorHandler != NULL))
        {
            error_code_t errorCode;
            errorCode.bit.osError = SET;
            PV624->handleError(errorCode, os_error);
        }
        else
        {
            // Resort to global error handler
            Error_Handler();
        }
    }
}

/**
 * @brief   Get serial number
 * @note    Instrument "serial number" if actually a free format ASCII character string
 * @param   void
 * @retval  character string
 */
uint32_t DPV624::getSerialNumber(void)
{    
    return persistentStorage->getSerialNumber();
}

/**
 * @brief   Set serial number
 * @note    Instrument "serial number" if actually a free format ASCII character string
 * @param   str - string
 * @retval  true = success, false = failed
 */
bool DPV624::setSerialNumber(uint32_t newSerialNumber)
{
    bool flag = false;

    if (newSerialNumber != 0XFFFFFFFFu) //one byte less for null terminator
    {
        //update string value
        sConfig_t *configData = persistentStorage->getConfigDataAddr();
        configData->serialNumber = newSerialNumber;
        //save in persistent storage
        if (persistentStorage->saveConfigData() == true)
        {
            flag = true;
        }

    }

    return flag;
}

/**
 * @brief   Get region of use
 * @param   void
 * @retval  current setting for region of use
 */
eRegionOfUse_t DPV624::getRegion(void)
{
    return (eRegionOfUse_t)0u;
}

/**
 * @brief   Set region of use
 * @param   region - are of use
 * @retval  true = success, false = failed
 */
bool DPV624::setRegion(eRegionOfUse_t region)
{
    return false;
}


/**
 * @brief   Query if instrument variant is intrinsically variant
 * @param   void
 * @retval  true if IS variant, else false (commercial)
 */
int32_t DPV624::queryEEPROMTest(void)
{
    int32_t result = 0;

    sPersistentDataStatus_t status = persistentStorage->getStatus();

    switch (status.selfTestResult)
    {
        case 2:
            result = 1;
            break;

        case 3:
            result = -1;
            break;

        default:
            break;
    }

    return result;
}

/**
 * @brief   Perform EEPROM test
 * @param   void
 * @retval  void
 */
void DPV624::performEEPROMTest(void)
{
    return persistentStorage->selfTest();
}

/**
 * @brief   Get pressed key
 * @param   void
 * @retval  true = success, false = failed
 */
uint32_t DPV624::getKey(void)
{
    return 0u;
}

/**
 * @brief   Emulate key press
 * @param   key - key id
 * @param   pressType - 0 - short press, 1 = long press
 * @retval  true = success, false = failed
 */
bool DPV624::setKey(uint32_t key, uint32_t pressType)
{
    return false;
}

/**
 * @brief   Get Date in RTC
 * @param   date - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getDate(sDate_t *date)
{
    return getSystemDate(date);
}

/**
 * @brief   Set Date in RTC
 * @param   date - user specified date to set
 * @retval  true = success, false = failed
 */
bool DPV624::setDate(sDate_t *date)
{
    return setSystemDate(date);
}

/**
 * @brief   Get system time from RTC
 * @param   time - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getTime(sTime_t *time)
{
    return getSystemTime(time);
}

/**
 * @brief   Set system time in RTC
 * @param   time - pointer to system time structure containing time to set the RTC
 * @retval  true = success, false = failed
 */
bool DPV624::setTime(sTime_t *time)
{
    return setSystemTime(time);
}

/**
 * @brief   Get version of specified item
 * @param   item - value: 0 = PV624
 *                        1 = PM620
 * @param   component - value: 0 = Application
 *                        1 = Bootloader
 *
 * @param   itemver - pointer variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getVersion(uint32_t item, uint32_t component, char itemverStr[10])
{
    bool status = false;
    itemverStr[0] = '\0';
      
    if(item <= 1u)
    {
      
      if( 0u == item)
      {
        switch(component)
        {
            case 0:
            {
                snprintf(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)cAppVersion[1],(uint8_t)cAppVersion[2], (uint8_t)cAppVersion[3]);
                status = true;
                break;
            }
            case 1:
            {
                snprintf(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)cblVersion[1],(uint8_t)cblVersion[2], (uint8_t)cblVersion[3]);
                status = true;
                break;
            }
            default:
            {
               status = false;
                break;
            }
        }
      }
      else
      {
        uSensorIdentity_t identity;
    
        switch(component)
        {
            case 0:
            {
                status = instrument->getExternalSensorAppIdentity(&identity);
                if (status)
                {
                    snprintf(itemverStr, 10u, "%02d.%02d.%02d", identity.major, identity.minor, identity.build);
                }
                status = true;
                break;
            }
            case 1:
            {
                status = instrument->getExternalSensorBootLoaderIdentity(&identity);
                if (status)
                {
                    snprintf(itemverStr, 10u, "%02d.%02d.%02d", identity.major, identity.minor, identity.build);
                }
                status = true;
                break;
            }
            default:
            {
                status = false;
                break;
            }
        }
      }
    }
      
    

    return status;
}


/**
 * @brief   Get positive fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getPosFullscale( float32_t  *fs)
{
    return instrument->getPosFullscale( fs);
}

/**
 * @brief   Get negative fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getNegFullscale( float32_t  *fs)
{
    return instrument->getNegFullscale( fs);
}

/**
 * @brief   Get sensor type
 * @param   sensorType - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getSensorType( eSensorType_t *sensorType)
{
    return instrument->getSensorType( sensorType);
}