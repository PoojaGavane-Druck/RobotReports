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
* @file     DErrorHandler.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 April 2020
*
* @brief    The error handler source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DErrorHandler.h"

MISRAC_DISABLE
#include "main.h"
MISRAC_ENABLE

#include "DPV624.h"
#include "Types.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
//#define SIMON_SAYS
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
const uint32_t errorBitMaskForLogging = (uint32_t)0x3ECFFF;
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DErrorHandler class constructor
 * @param   comms is the reference to the comms instance for this sensor
 * @retval  void
 */
DErrorHandler::DErrorHandler(OS_ERR *os_error)
    : DTask()
{
    clearAllErrors();
    deviceStatus.bytes = 0u;
}

/**
* @brief    handleError - process messaages from the rest of the system
* @param    errorCode - enumerated erro code identifier value
* @param    param -    optional parameter value TODO - Not currently used
* @param    blocking - optional parameter if true to stop here for showstopper faults TODO - blocking error won't cause watchdog to trip
* @return   None
*/
void DErrorHandler::handleError(eErrorCode_t errorCode, 
                               eErrorStatus_t errStatus,
                               uint32_t paramValue,
                               uint16_t errInstance, 
                               bool isFatal)
{
  deviceStatus_t prevDeviceStatus;
  prevDeviceStatus.bytes = (uint32_t)0;
  
  prevDeviceStatus.bytes = deviceStatus.bytes;
 
  updateDeviceStatus(errorCode, errStatus);
 

  if((prevDeviceStatus.bytes & (errorBitMaskForLogging)) != (deviceStatus.bytes & (errorBitMaskForLogging)))
  {
    //PV624->logger->logError(errorCode,errStatus,paramValue,errInstance,isFatal);
  }
  

}

                               
/**
* @brief    handleError - process messaages from the rest of the system
* @param    errorCode - enumerated erro code identifier value
* @param    param -    optional parameter value TODO - Not currently used
* @param    blocking - optional parameter if true to stop here for showstopper faults TODO - blocking error won't cause watchdog to trip
* @return   None
*/
void DErrorHandler::handleError(eErrorCode_t errorCode, 
                               eErrorStatus_t errStatus,
                               float paramValue,
                               uint16_t errInstance, 
                               bool isFatal)
{
  deviceStatus_t prevDeviceStatus;
  prevDeviceStatus.bytes = (uint32_t)0;
  
  prevDeviceStatus.bytes = deviceStatus.bytes;
 
  updateDeviceStatus(errorCode, errStatus);
 

  if((prevDeviceStatus.bytes & (errorBitMaskForLogging)) != (deviceStatus.bytes & (errorBitMaskForLogging)))
  {
    //PV624->logger->logError(errorCode,errStatus,paramValue,errInstance,isFatal);
  }
  

}                               
/**
 * @brief Clears the specified error
 *
 * @param error_code_t errorCode
 * @return None
 */
void DErrorHandler::clearError(eErrorCode_t errorCode)
{
#if 0
    switch(errorCode)
    {
        case E_ERROR_LOW_REFERENCE_SENSOR_VOLTAGE:
          currentDeviceStatus.lowReferenceSensorVoltage = 0;
        break;
      
        case E_ERROR_REFERENCE_SENSOR_COM:
          currentDeviceStatus.referenceSensorCommFail = 0;
        break;
      
        case E_ERROR_BAROMETER_SENSOR:
          currentDeviceStatus.barometerSensorFail = 0;
        break;
      
        case E_ERROR_STEPPER_CONTROLLER:
          currentDeviceStatus.stepperControllerFail = 0;
        break;
      
        case E_ERROR_MOTOR_VOLTAGE:
          currentDeviceStatus.motorVoltageFail = 0;
        break;
      
        case E_ERROR_STEPPER_DRIVER:
          currentDeviceStatus.stepperDriverFail = 0;
        break;
      
        case E_ERROR_VALVE:
          currentDeviceStatus.vlaveFail = 0;
        break;
      
        case E_ERROR_EEPROM:
          currentDeviceStatus.persistentMemoryFail = 0;
        break;
      
        case E_ERROR_BATTERY_WARNING_LEVEL:
          currentDeviceStatus.batteryWarningLevel = 0;
        break;
      
        case E_ERROR_BATTERY_CRITICAL_LEVEL:
          currentDeviceStatus.batteryCriticalLevel = 0;
        break;
      
        case E_ERROR_EXTERNAL_FLASH_CORRUPT:
          currentDeviceStatus.extFlashCorrupt = 0;
        break;
      
        case E_ERROR_EXTERNAL_FLASH_WRITE:
          currentDeviceStatus.extFlashWriteFailure = 0;
        break;
        
        case E_ERROR_ON_BOARD_FLASH:
          currentDeviceStatus.onboardFlashFail = 0;
        break;
      
        case E_ERROR_OVER_TEMPERATURE:
          currentDeviceStatus.overTemperature = 0;
        break;
            
        case E_ERROR_BATTERY_COMM:
          currentDeviceStatus.smBusBatteryComFailed = 0;
        break;
      
        case E_ERROR_BATTERY_CHARGER_COMM:
          currentDeviceStatus.smBusBatChargerComFailed = 0;
        break;
      
        case E_ERROR_OS:
          currentDeviceStatus.osError = 0;
        break;
        
        case E_ERROR_REFERENCE_SENSOR_OUT_OF_CAL:
          currentDeviceStatus.lowReferenceSensorVoltage = 0;
        break;
      
        case E_ERROR_BAROMETER_OUT_OF_CAL:
          currentDeviceStatus.lowReferenceSensorVoltage = 0;
        break;
        
        case E_ERROR_BAROMETER_SENSOR_MODE:
          currentDeviceStatus.barometerSensorMode = 0;
        break;
      
        case E_ERROR_BAROMETER_SENSOR_CAL_STATUS:
          currentDeviceStatus.barometerSensorCalStatus = 0;
        break;
      
        case E_ERROR_BAROMETER_NOT_ENABLED:
          currentDeviceStatus.barometerNotEnabled = 0;
        break;
      
    default:
      break;
      
    }
#endif
}

/**
 * @brief Clears all current errors
 *
 * @param None
 * @return error_code_t
 */
void DErrorHandler::clearAllErrors(void)
{


    deviceStatus.bytes = RESET;



}

/**
 * @brief: Get all current errors
 *
 * @param None
 * @return error_code_t
 */
deviceStatus_t DErrorHandler::getDeviceStatus(void)
{
    return deviceStatus;
}

void DErrorHandler::updateDeviceStatus(eErrorCode_t errorCode, 
                                       eErrorStatus_t errStatus)
{
    switch(errorCode)
    {
        case E_ERROR_LOW_REFERENCE_SENSOR_VOLTAGE:
          deviceStatus.bit.lowReferenceSensorVoltage = errStatus;
        break;
      
        case E_ERROR_REFERENCE_SENSOR_COM:
          deviceStatus.bit.referenceSensorCommFail = errStatus;
        break;
      
        case E_ERROR_BAROMETER_SENSOR:
          deviceStatus.bit.barometerSensorFail = errStatus;
        break;
      
        case E_ERROR_STEPPER_CONTROLLER:
          deviceStatus.bit.stepperControllerFail = errStatus;
        break;
      
        case E_ERROR_MOTOR_VOLTAGE:
          deviceStatus.bit.motorVoltageFail = errStatus;
        break;
      
        case E_ERROR_STEPPER_DRIVER:
          deviceStatus.bit.stepperDriverFail = errStatus;
        break;
      
        case E_ERROR_VALVE:
          deviceStatus.bit.vlaveFail = errStatus;
        break;
      
        case E_ERROR_EEPROM:
          deviceStatus.bit.persistentMemoryFail = errStatus;
        break;
      
        case E_ERROR_BATTERY_WARNING_LEVEL:
          deviceStatus.bit.batteryWarningLevel = errStatus;
        break;
      
        case E_ERROR_BATTERY_CRITICAL_LEVEL:
          deviceStatus.bit.batteryCriticalLevel = errStatus;
        break;
      
        case E_ERROR_EXTERNAL_FLASH_CORRUPT:
          deviceStatus.bit.extFlashCorrupt = errStatus;
        break;
      
        case E_ERROR_EXTERNAL_FLASH_WRITE:
          deviceStatus.bit.extFlashWriteFailure = errStatus;
        break;
        
        case E_ERROR_ON_BOARD_FLASH:
          deviceStatus.bit.onboardFlashFail = errStatus;
        break;
      
        case E_ERROR_OVER_TEMPERATURE:
          deviceStatus.bit.overTemperature = errStatus;
        break;
            
        case E_ERROR_BATTERY_COMM:
          deviceStatus.bit.smBusBatteryComFailed = errStatus;
        break;
      
        case E_ERROR_BATTERY_CHARGER_COMM:
          deviceStatus.bit.smBusBatChargerComFailed = errStatus;
        break;
      
        case E_ERROR_OS:
          deviceStatus.bit.osError = errStatus;
        break;
        
        case E_ERROR_REFERENCE_SENSOR_OUT_OF_CAL:
          deviceStatus.bit.referenceSensorOutOfCal = errStatus;
        break;
      
        case E_ERROR_BAROMETER_OUT_OF_CAL:
          deviceStatus.bit.barometerOutOfCal = errStatus;
        break;
        
        case E_ERROR_BAROMETER_SENSOR_MODE:
          deviceStatus.bit.barometerSensorMode = errStatus;
        break;
      
        case E_ERROR_BAROMETER_SENSOR_CAL_STATUS:
          deviceStatus.bit.barometerSensorCalStatus = errStatus;
        break;
      
        case E_ERROR_BAROMETER_NOT_ENABLED:
          deviceStatus.bit.barometerNotEnabled = errStatus;
        break;
      
    default:
      break;
      
    }
    
}