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
* @file     DErrorHandler.cpp
* @version  1.00.00
* @author   Nageswara Rao P
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
{
    clearAllErrors();
    deviceStatus.bytes = 0u;
    // Set the PM 620 comms error at startup, as sensor will take 7 seconds to connect
    deviceStatus.bit.referenceSensorCommFail = 1u;
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
    PV624->logger->logError(errorCode,errStatus,paramValue,errInstance,isFatal);
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
    PV624->logger->logError(errorCode,errStatus,paramValue,errInstance,isFatal);
  }
  

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
 * @brief get device's current status.
 *
 * @param None
 * @return deviceStatus_t Complete PV624 device status
 */
deviceStatus_t DErrorHandler::getDeviceStatus(void)
{
    return deviceStatus;
}

/**
* @brief    updateDeviceStatus - updates the device status based on the error code
* @param    errorCode - enumerated erro code  value
* @param    eErrorStatus_t - error status 0: to clear error and 1:to Set Error
* @return   None
*/
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
        
        case E_ERROR_CHARGER_CONNECTED:
          deviceStatus.bit.chargingStatus = errStatus;
        break;
      
    default:
      break;
      
    }
    
}