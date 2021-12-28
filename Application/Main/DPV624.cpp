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
#include "usb_device.h"
MISRAC_ENABLE

#include "DPV624.h"
#include "DStepperMotor.h"
#include "DSlot.h"
#include "i2c.h"
#include "uart.h"
#include "Utilities.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

//#define TEST_MOTOR

#define MOTOR_FREQ_CLK 12000000u

#define SP_ATMOSPHERIC 2u
#define SP_ABSOLUTE 1u
#define SP_GAUGE 0u

//#define   NUCLEO_BOARD 0
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
DPV624 *PV624;

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;
extern SMBUS_HandleTypeDef hsmbus1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern SPI_HandleTypeDef hspi2;

extern TIM_HandleTypeDef  htim4;
extern TIM_HandleTypeDef  htim1;

extern const unsigned int cAppDK;
extern const unsigned char cAppVersion[4];

#ifdef BOOTLOADER_IMPLEMENTED
/*  __FIXED_region_ROM_start__ in bootloader stm32l4r5xx_flash.icf */
const unsigned int bootloaderFixedRegionRomStart = 0x08000200u; 
__no_init const unsigned int cblDK @ bootloaderFixedRegionRomStart + 0u;
__no_init const unsigned char cblVersion[4] @ bootloaderFixedRegionRomStart + 4u;
//__no_init const char cblInstrument[16] @ bootloaderFixedRegionRomStart + 8u;
#else
const unsigned int cblDK = (uint32_t)498;
const unsigned char cblVersion[4] = {0, 99, 99, 99};
#endif

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
#include "Utilities.h"

DPV624::DPV624(void)
{
    OS_ERR os_error;   
    isEngModeEnable = true;
    
#ifdef NUCLEO_BOARD
    i2cInit(&hi2c2);
#else
    i2cInit(&hi2c3);
    i2cInit(&hi2c4);
#endif       
    persistentStorage = new DPersistent();
    
    uartInit(&huart2);
    uartInit(&huart3);
    uartInit(&huart4);  
    uartInit(&huart5);
#ifndef TEST_MOTOR
    
    extStorage = new DExtStorage(&os_error);
    validateApplicationObject(os_error);
    
    powerManager = new DPowerManager(&hsmbus1, &os_error);
    validateApplicationObject(os_error);
       
    logger = new DLogger(&os_error);
    validateApplicationObject(os_error);
    
    errorHandler = new DErrorHandler(&os_error);
    validateApplicationObject(os_error);

    keyHandler = new DKeyHandler(&os_error);
    validateApplicationObject(os_error);
#endif
    stepperMotor = new DStepperMotor();

#ifndef TEST_MOTOR
    instrument = new DInstrument(&os_error);
    validateApplicationObject(os_error);

    commsOwi = new DCommsOwi("commsOwi", &os_error);
    validateApplicationObject(os_error);
  
    commsUSB = new DCommsUSB("commsUSB", &os_error);
    validateApplicationObject(os_error);  
  
    valve1 = new DValve(&htim3, 
                        VALVE1_PWM_PE9_GPIO_Port, 
                        VALVE1_PWM_PE9_Pin,
                        VALVE1_DIR_PC5_GPIO_Port,
                        VALVE1_DIR_PC5_Pin);
    
    validateApplicationObject(os_error);

    valve2 = new DValve(&htim4,
                        VALVE2_PWM_PD14_GPIO_Port,
                        VALVE2_PWM_PD14_Pin,
                        VALVE2_DIR_PB1_GPIO_Port,
                        VALVE2_DIR_PB1_Pin);
    
    validateApplicationObject(os_error);
    
    valve3 = new DValve(&htim6,
                        VALVE3_PWM_PD15_GPIO_Port,
                        VALVE3_PWM_PD15_Pin,
                        VALVE3_DIR_PF11_GPIO_Port,
                        VALVE3_DIR_PF11_Pin);
    
    validateApplicationObject(os_error); 
    
    userInterface = new DUserInterface(&os_error);
    validateApplicationObject(os_error);
   //leds = new LEDS();
  
    isPrintEnable = false;
    
    /* Test motor */
#endif
#ifdef TEST_MOTOR
    uint32_t index = 0u;
    int32_t completed = (int32_t)(0);
    uint32_t speed = 0u;
    float current = 0.0f;
    uint8_t txBuff[4] = {0x00u, 0x00u, 0x00u, 0xC8u};
    uint8_t rxBuff[4] = {0x00u, 0x00u, 0x00u, 0x00u};
    
    for(index = 0u; index < 400u; index++)
    {
        stepperMotor->move((int32_t)(400), &completed);
        HAL_Delay(100u);
        stepperMotor->move((int32_t)(-400), &completed);
        HAL_Delay(100u);
        stepperMotor->readSpeedAndCurrent(&speed, &current);
        HAL_Delay(100u);
    }
#endif
}

/**
* @brief    handleError - process messaages from the rest of the system
* @param    errorCode - enumerated error code identifier value
* @param    param -    optional parameter value TODO - Not currently used
* @param    blocking - optional parameter if true to stop here for showstopper faults TODO - blocking error won't cause watchdog to trip
* @return   None
*/
void DPV624::handleError(eErrorCode_t errorCode, 
                                   eErrorStatus_t errStatus,
                                   uint32_t paramValue,
                                   uint16_t errInstance, 
                                   bool isFatal)
{
    if (errorHandler != NULL)
    {
        errorHandler->handleError(errorCode, 
                                  errStatus,
                                  paramValue,
                                  errInstance, 
                                  isFatal);
    }
}

/**
* @brief    handleError - process messaages from the rest of the system
* @param    errorCode - enumerated error code identifier value
* @param    param -    optional parameter value TODO - Not currently used
* @param    blocking - optional parameter if true to stop here for showstopper faults TODO - blocking error won't cause watchdog to trip
* @return   None
*/
void DPV624::handleError(eErrorCode_t errorCode, 
                                   eErrorStatus_t errStatus,
                                   float paramValue,
                                   uint16_t errInstance, 
                                   bool isFatal)
{
    if (errorHandler != NULL)
    {
        errorHandler->handleError(errorCode, 
                                  errStatus,
                                  paramValue,
                                  errInstance, 
                                  isFatal);
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
          
            PV624->handleError(E_ERROR_OS, 
                               eSetError,
                               (uint32_t)os_error,
                               (uint16_t)1);
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
uint32_t DPV624::getSerialNumber(uint32_t snType)
{
    uint32_t sn = (uint32_t)(0);
    
    if((uint32_t)(0) == snType)
    {
        sn =  persistentStorage->getSerialNumber();
    }
    else
    {
        instrument->getSensorSerialNumber(&sn);
    }
    return sn;
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
 * @brief   Get stepper motor instance
 * @param   void
 * @retval  current setting for region of use
 */
DStepperMotor* DPV624::getStepperMotorInstance(void)
{
    return stepperMotor;
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
 * @brief   Reset stepper micro controller
 * @param   void
 * @retval  void
 */
void DPV624::resetStepperMicro(void)
{
    /* Reset the stepper controller micro */
  
    /* Generate a 100 ms reset */
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_Delay((uint16_t)(10));
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_Delay((uint16_t)(100));
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_SET);
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
 * @brief   Get DK of specified item as string
 * @param   item - value: 0 = Bootloader
 *                        1 = Application
 * @param   char dkStr[7] - pointer variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getDK(uint32_t item, uint32_t component, char dkStr[7])
{
    bool status = false;

    if(item <= 1u)
    {
      
      if( 0u == item)
      {
        switch(component)
        {
            case 0:
            {
                snprintf(dkStr, 7u, "%04d", cAppDK);
                status = true;
                break;
            }
            case 1:
            {
                snprintf(dkStr, 7u, "%04d", cblDK);
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
                    snprintf(dkStr, 7u, "%04d", identity.dk);
                }
                status = true;
                break;
            }
            case 1:
            {
                status = instrument->getExternalSensorBootLoaderIdentity(&identity);
                if (status)
                {
                    snprintf(dkStr, 7u, "%04d", identity.dk);
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
 * @brief   Get instrument name
 * @param   char nameStr[16] - pointer variable for return value
 * @retval  void
 */
void DPV624::getInstrumentName(char nameStr[13])
{
    // overrule stored cblInstrument and cAppInstrument values
    strncpy(nameStr,  "PV624HYBRID" , (size_t)13);
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
    return instrument->getSensorType(sensorType);
}

/**
 * @brief   Get sensor type for engineering protocol
 * @param   sensorType - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getPM620Type(uint32_t *sensorType)
{
    return instrument->getPM620Type(sensorType);
}

/**
 * @brief   Get controller mode
 * @param   controller mode - pointer to variable for return value
 * @retval  true = success, false = failed
*/
bool DPV624::getControllerMode(eControllerMode_t *controllerMode)
{
  return instrument->getControllerMode( controllerMode);
}

/**
 * @brief   Set controller mode
 * @param   controller mode - pointer to variable for return value
 * @retval  true = success, false = failed
*/
bool DPV624::setControllerMode(eControllerMode_t newCcontrollerMode)
{
  return instrument->setControllerMode( newCcontrollerMode);
}

/**
 * @brief   Get cal interval
 * @param   interval is pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getCalInterval( uint32_t *interval)
{
    bool flag = false;
    eFunction_t func = E_FUNCTION_GAUGE;
    //if function on specified channel is not being calibrated then we are setting the instrument's cal interval
    flag = instrument->getFunction((eFunction_t*)&func);
    if(true == flag)
    {
      if ((eFunction_t)E_FUNCTION_BAROMETER ==  func)
      {
          //"channel" can only be 0 if updating the instrument-wide cal interval
          if (NULL != interval)
          {                      
              *interval = persistentStorage->getCalInterval();
              flag = true;
          }
      }
      else
      {
          //set cal interval for the sensor being calibrated
          flag = instrument->getCalInterval( interval);
      }
    }
    return flag;
}

/**
 * @brief   set cal interval
 * @param   interval  value
 * @retval  true = success, false = failed
 */
bool DPV624::setCalInterval( uint32_t interval)
{
    bool flag = false;
     eFunction_t func = E_FUNCTION_GAUGE;
    //if function on specified channel is not being calibrated then we are setting the instrument's cal interval
    flag = instrument->getFunction((eFunction_t*)&func);
    if(true == flag)
    {
      if ((eFunction_t)E_FUNCTION_BAROMETER ==  func)      
      {
         flag = persistentStorage->setCalInterval(interval);   
         if(true == flag)
         {
           flag = instrument->setCalInterval( interval);
         }
          
      }
      else
      {
          //Not allowed to write PM620 Calibration interval
          flag = false;
      }
    }

    return flag;
}

bool DPV624::getPressureSetPoint(float *pSetPoint)
{
    bool successFlag = false;
    
    if (NULL != pSetPoint)
    {
        successFlag = instrument->getPressureSetPoint(pSetPoint);
    }
   else
   {
     successFlag=  false;
   }
    return successFlag; 
}

bool DPV624::setPressureSetPoint(float newSetPointValue)
{
    bool successFlag = false;
    
    successFlag = instrument->setPressureSetPoint(newSetPointValue);      
    
    return successFlag; 
}


/**
 * @brief   get Instrument function
 * @param   func is the function itself
 * @retval  true if activated successfully, else false
 */
 bool DPV624::getFunction( eFunction_t *pFunc)
{
  return instrument->getFunction(pFunc);
}

void DPV624::takeNewReading(uint32_t rate)
{
  instrument->takeNewReading(rate);
}

bool DPV624::setControllerStatusPm(uint32_t status)
{
    return instrument->setControllerStatusPm(status);
}

bool DPV624::getControllerStatusPm(uint32_t *status)
{
    return instrument->getControllerStatusPm(status);
}


bool DPV624::setControllerStatus(uint32_t newStatus)
{
    return instrument->setControllerStatus(newStatus);
}


#if 1
/**
 * @brief   Get Controller Status
 * @param   void
 * @retval  uint32_t controller status
 */
bool DPV624::getControllerStatus(uint32_t *controllerStatus)
{
    return instrument->getControllerStatus(controllerStatus);
}
#endif


/**
 * @brief   Get current PIN mode (ie, protection level)
 * @param   void
 * @retval  PIN mode
 */
ePinMode_t DPV624::getPinMode(void)
{
    return myPinMode;
}


/**
 * @brief   Get current PIN mode (ie, protection level)
 * @param   mode - new PIN mode
 * @retval  true = success, false = failed
 */
bool DPV624::setPinMode(ePinMode_t mode)
{
    bool flag = false;

    //can only change mode if either current mode or new mode is E_PIN_MODE_NONE;
    //ie cannot switch modes without going through 'no pin' mode
    if ((myPinMode == (ePinMode_t)E_PIN_MODE_NONE) || (mode == (ePinMode_t)E_PIN_MODE_NONE))
    {
        myPinMode = mode;
        flag = true;
    }

    return flag;
}

/**
 * @brief   Set Instrument Port Configuration for USB
 * @param   mode - 0 for communications mode; 1 for storage mode
 * @retval  none
 */
void DPV624::setUsbInstrumentPortConfiguration(int32_t mode)
{
//#ifdef PORT_SWITCHING_IMPLEMENTED
    MX_USB_DEVICE_SetUsbMode((eUsbMode_t)mode);
//#endif
}

/**
 * @brief   Get Instrument Port Configuration for USB
 * @param   none
 * @retval  mode - 0 for communications mode; 1 for storage mode
 */
int32_t DPV624::getUsbInstrumentPortConfiguration()
{
//#ifdef PORT_SWITCHING_IMPLEMENTED
     return (int32_t)MX_USB_DEVICE_GetUsbMode();
//#else
  //   return (int32_t)0;
//#endif
   
}

/**
 * @brief   Get cal date
 * @param   instrument channel
 * @param   pointer to date structure for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getManufactureDate( sDate_t *date)
{
    bool flag = false;

    if(NULL != date)
    {
        //get address of manufacure date structure in persistent storage TODO
        //sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        date->day = manufactureDate.day;
        date->month = manufactureDate.month;
        date->year = manufactureDate.year;

        flag = true;
    }

    return flag;
}

/**
 * @brief   Set cal date
 * @param   instrument channel
 * @param   pointer to date structure
 * @retval  true = success, false = failed
 */
bool DPV624::setCalDate( sDate_t *date)
{
    bool flag = false;


    if(NULL != date)
    {
      //get address of calibration data structure in persistent storage
      sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

      calDataBlock->calDate.day = date->day;
      calDataBlock->calDate.month = date->month;
      calDataBlock->calDate.year = date->year;

      flag = persistentStorage->saveCalibrationData();
    }
    return flag;
}

/**
 * @brief   Set cal date
 * @param   instrument channel
 * @param   pointer to date structure
 * @retval  true = success, false = failed
 */
bool DPV624::setManufactureDate( sDate_t *date)
{
    bool flag = false;


    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
        //sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();
        
        manufactureDate.day = date->day;
        manufactureDate.month = date->month;
        manufactureDate.year = date->year;
        
        /*
        calDataBlock->calDate.day = date->day;
        calDataBlock->calDate.month = date->month;
        calDataBlock->calDate.year = date->year;

        flag = persistentStorage->saveCalibrationData();
        */
        flag = true;
    }
    return flag;
}

bool DPV624::getSensorBrandUnits(char *brandUnits)
{
    instrument->getSensorBrandUnits(brandUnits);
    
    return true;
}

/**
 * @brief   Set calibration type
 * @param   calType - function specific calibration type (0 = user calibration)
 * @param   range - sensor range
 * @retval  true = success, false = failed
 */
bool DPV624::setCalibrationType(int32_t calType, uint32_t range)
{
  bool retStatus = false;
  
  retStatus = instrument->setCalibrationType( calType, range);
  

  
  return retStatus;
}

/**
 * @brief   Start sampling at current cal point
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::startCalSampling(void)
{
    return instrument->startCalSampling();
}

/**
 * @brief   Set calibration point
 * @param   point indicates the cal point number (1 - required no of points)
 * @param   user supplied calibration value
 * @retval  true = success, false = failed
 */
bool DPV624::setCalPoint(uint32_t calPoint, float32_t value)
{
    return instrument->setCalPoint(calPoint, value);
}

/**
 * @brief   Cal accept
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::acceptCalibration(void)
{
    return instrument->acceptCalibration();
}

/**
 * @brief   Abort calibration
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::abortCalibration(void)
{
    return instrument->abortCalibration();
}

/**
 * @brief   Sets required number of calibration points
 * @param   uint32_t number of calpoints
 * @retval  true = success, false = failed
 */
bool DPV624::setRequiredNumCalPoints(uint32_t numCalPoints)
{
    return instrument->setRequiredNumCalPoints(numCalPoints);
}

/**
 * @brief   Perform application firmware upgrade
 * @param   void
 * @retval  flag: true if ok, else false
 */
bool DPV624::performUpgrade(void)
{
    bool ok = true;


    return ok;
}

/**
 * @brief   Perform application firmware upgrade
 * @param   void
 * @retval  flag: true if ok, else false
 */
bool DPV624::performPM620tUpgrade(void)
{
    bool ok = true;
    ok = instrument->upgradeSensorFirmware();    
    return ok;
}

/*****************************************************************************/
/* SUPPRESS: floating point values shall not be tested for exact equality or 
inequality (MISRA C 2004 rule 13.3) */
/*****************************************************************************/
_Pragma ("diag_suppress=Pm046")
/**
 * @brief   Set channel sensor zero value
 * @param   value - user provided value to set as the zero
 * @retval  true = success, false = failed
 */
bool DPV624::setZero( float32_t value)
{
    /* Test of zero val setting, this has to be written to mem ? */
    float pressure = 0.0f;
    float fsPressure = 0.0f;
    float zeroPc = 0.0f;
    
    bool status = false;
    
    if(0.0f == value)
    {
        /* read original zero */        
        instrument->getPressureReading(&value);
        value = value + pressure;
    }
    if(0.0f <= value)
    {
        instrument->getPositiveFS((float*)&fsPressure);
    }
    else
    {
        instrument->getNegativeFS((float*)&fsPressure);
    }
    
    zeroPc = value * 100.0f / fsPressure; 
    
    if(1.0f > zeroPc)
    {
        zeroVal = value;
        status = true;
    }
    
    return status;
}
/*****************************************************************************/
/* SUPPRESS: floating point values shall not be tested for exact equality or 
inequality (MISRA C 2004 rule 13.3) */
/*****************************************************************************/
_Pragma ("diag_default=Pm046")




/**
 * @brief   Get battery status from Coulomb handler
 * @param   pointer to sBatteryStatus
 * @retval  none
 */
void DPV624::getBatteryStatus(sBatteryStatus_t *sBatteryStatus)
{
   
}

/**
 * @brief   Save current cal as backup
 * @param   void
 * @retval  flag: true = success, false = failed
 */
bool DPV624::backupCalDataSave(void)
{
    bool flag = false;

    if (persistentStorage != NULL)
    {
        flag = persistentStorage->saveAsBackupCalibration();
    }

    return flag;
}


/**
 * @brief   Restore backup cal as current
 * @param   void
 * @retval  flag: true = success, false = failed
 */
bool DPV624::backupCalDataRestore(void)
{
    bool flag = false;

    if (persistentStorage != NULL)
    {
        flag = persistentStorage->loadBackupCalibration();

        if (instrument != NULL)
        {
            flag &= instrument->reloadCalibration();
        }
    }

    return flag;
}

/**
 * @brief   Query if instrument variant is intrinsically variant
 * @param   void
 * @retval  true if IS variant, else false (commercial)
 */
int32_t DPV624::queryInvalidateCalOpeResult(void)
{
    int32_t result = 0;

    sPersistentDataStatus_t status = persistentStorage->getStatus();

    switch (status.invalidateCalOperationResult)
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
 * @brief   Get remaining number of samples at current cal point
 * @param   pointer to variable for return value of remaining number of samples
 * @retval  true = success, false = failed
 */
bool DPV624::getCalSamplesRemaining(uint32_t *samples)
{
    return instrument->getCalSamplesRemaining(samples);
}

/**
 * @brief   Get required number of calibration points
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::getRequiredNumCalPoints(uint32_t *numCalPoints)
{
    return instrument->getRequiredNumCalPoints(numCalPoints);
}

/**
 * @brief   Get channel sensor zero value
 * @param   channel - instrument channel
 * @param   value - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getZero(float32_t *value)
{
    //TODO HSB:
    *value = zeroVal;
    return true;
}

/**
 * @brief   Get cal date
 * @param   instrument channel
 * @param   pointer to date structure for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getCalDate( sDate_t *date)
{
    bool flag = false;

   if(NULL != date)
   {
    //get address of calibration data structure in persistent storage
    sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

    date->day = calDataBlock->calDate.day;
    date->month = calDataBlock->calDate.month;
    date->year = calDataBlock->calDate.year;

    flag = true;
   }

    return flag;
}

/**
 * @brief   Invalidate Calibratin Data
 * @param   void
 * @retval  void
 */
bool DPV624::invalidateCalibrationData(void)
{
    return persistentStorage->invalidateCalibrationData();
}

/**
 * @brief   writes data over USB
 * @param   buf - pointer to null-terminated character string to transmit
* @param   bufSIze - number of bytes to write
 * @retval  flag - true = success, false = failed
 */
 bool DPV624::print(uint8_t* buf, uint32_t bufSize)
 {
   bool retStatus = false;
   DDeviceSerial *myCommsMedium;
   myCommsMedium = commsUSB->getMedium();
   if(NULL != buf)      
   {
      if(true == isPrintEnable)
      {
        retStatus = myCommsMedium->write(buf, bufSize);
      }
      else
      {
        retStatus = true;
      }
   }
   return retStatus;
 }

 /**
 * @brief   sets isPrintEnable status flag
 * @param   newState - true - enable print, flase disable print
 * @retval  flag - true = success, false = failed
 */
void DPV624::setPrintEnable(bool newState)
{
  isPrintEnable = newState;
}

 /**
 * @brief   returns eng mode status
 * @param   void
 * @retval  true = if  eng mode enable , false = if engmode not enabled
 */
bool DPV624::engModeStatus(void)
{
    return isEngModeEnable;
}
/**
 * @brief   Sets aquisation mode of pressure slot and barometer slot
 * @param   newAcqMode : new Aquisation mode
 * @retval  void
 */
bool DPV624::setAquisationMode(eAquisationMode_t newAcqMode)
{
     bool retStatus = false;
     
     retStatus = instrument->setAquisationMode(newAcqMode);
     if( true == retStatus)
     {
       if((eAquisationMode_t)E_REQUEST_BASED_ACQ_MODE == newAcqMode)
       {
          isEngModeEnable = true;
       }
       else
       {
          isEngModeEnable = false;
       }
     }
     return retStatus;
}

/**
 * @brief   increments the setpoint count and saves into eeprom
 * @param   void
 * @retval  true if saved  sucessfully false if save fails
 */
bool DPV624::incrementSetPointCount()
{
     bool successFlag = false;   
     uint32_t pNewSetPointCount;
     //increment set point counts
     if (persistentStorage->incrementSetPointCount(&pNewSetPointCount) == true)
     {
        successFlag = true;
        //Todo Log SetPoint Count into Logger event file 
     }     
    
    return successFlag; 
}

/**
 * @brief   retruns set point count
 * @param   void
 * @retval  set Point Count
 */
uint32_t DPV624::getSetPointCount(void)
{
  return persistentStorage->getSetPointCount();
}

/**
 * @brief   get the battery percentage and charginging status
 * @param   *pPercentCapacity    to return percentage capacity
 * @return  *pChargingStatus     to return charging Status
 */
void DPV624::getBatLevelAndChargingStatus(float *pPercentCapacity,
                                          uint32_t *pChargingStatus)
{

   powerManager->getBatLevelAndChargingStatus(pPercentCapacity,
                                              pChargingStatus);
}

/**
 * @brief   updates battery status
 * @param   void
 * @return  void
 */
void DPV624::updateBatteryStatus(void)
{
  userInterface->updateBatteryStatus(BATTERY_LEDS_DISPLAY_TIME,BATTERY_LED_UPDATE_RATE);
}