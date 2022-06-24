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
const uint32_t errorBitMaskForLogging = 0x3ECFFFu;
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
    prevDeviceStatus.bytes = 0u;

    prevDeviceStatus.bytes = deviceStatus.bytes;

    updateDeviceStatus(errorCode, errStatus);


    if((prevDeviceStatus.bytes & (errorBitMaskForLogging)) != (deviceStatus.bytes & (errorBitMaskForLogging)))
    {
        PV624->logger->logError(errorCode, errStatus, paramValue, errInstance, isFatal);
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
    prevDeviceStatus.bytes = 0u;

    prevDeviceStatus.bytes = deviceStatus.bytes;

    updateDeviceStatus(errorCode, errStatus);


    if((prevDeviceStatus.bytes & (errorBitMaskForLogging)) != (deviceStatus.bytes & (errorBitMaskForLogging)))
    {
        performActionOnError(errorCode, errStatus);
#ifdef ENABLE_LOGGER
        PV624->logger->logError(errorCode, errStatus, paramValue, errInstance, isFatal);
#endif
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
    uint32_t updateErrorLed = 0u;

    switch(errorCode)
    {
    case E_ERROR_LOW_REFERENCE_SENSOR_VOLTAGE:
        deviceStatus.bit.lowReferenceSensorVoltage = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_REFERENCE_SENSOR_COM:
        deviceStatus.bit.referenceSensorCommFail = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_BAROMETER_SENSOR:
        deviceStatus.bit.barometerSensorFail = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_STEPPER_CONTROLLER:
        deviceStatus.bit.stepperControllerFail = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_MOTOR_VOLTAGE:
        deviceStatus.bit.motorVoltageFail = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_STEPPER_DRIVER:
        deviceStatus.bit.stepperDriverFail = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_VALVE:
        deviceStatus.bit.vlaveFail = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_EEPROM:
        deviceStatus.bit.persistentMemoryFail = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_BATTERY_WARNING_LEVEL:
        deviceStatus.bit.batteryWarningLevel = errStatus;
        break;

    case E_ERROR_BATTERY_CRITICAL_LEVEL:
        deviceStatus.bit.batteryCriticalLevel = errStatus;
        break;

    case E_ERROR_EXTERNAL_FLASH_CORRUPT:
        deviceStatus.bit.extFlashCorrupt = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_EXTERNAL_FLASH_WRITE:
        deviceStatus.bit.extFlashWriteFailure = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_ON_BOARD_FLASH:
        deviceStatus.bit.onboardFlashFail = errStatus;
        break;

    case E_ERROR_OVER_TEMPERATURE:
        deviceStatus.bit.overTemperature = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_BATTERY_COMM:
        deviceStatus.bit.smBusBatteryComFailed = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_BATTERY_CHARGER_COMM:
        deviceStatus.bit.smBusBatChargerComFailed = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_OS:
        deviceStatus.bit.osError = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

#if 0

    case E_ERROR_REFERENCE_SENSOR_OUT_OF_CAL:
        deviceStatus.bit.referenceSensorOutOfCal = errStatus;
        break;

    case E_ERROR_BAROMETER_OUT_OF_CAL:
        deviceStatus.bit.barometerOutOfCal = errStatus;
        break;

    case E_ERROR_BAROMETER_SENSOR_MODE:
        deviceStatus.bit.barometerSensorMode = errStatus;
        break;
#endif

    case E_ERROR_BAROMETER_SENSOR_CAL_STATUS:
        deviceStatus.bit.barometerSensorCalStatus = errStatus;
        break;
#if 0

    case E_ERROR_BAROMETER_NOT_ENABLED:
        deviceStatus.bit.barometerNotEnabled = errStatus;
        break;
#endif

    case E_ERROR_CHARGER_CONNECTED:
        deviceStatus.bit.chargingStatus = errStatus;
        break;

    case E_ERROR_CODE_REMOTE_REQUEST_FROM_BT_MASTER:
        deviceStatus.bit.remoteRequestFromBtMaster = errStatus;
        break;

    case E_ERROR_CODE_REMOTE_REQUEST_FROM_OWI_MASTER:
        deviceStatus.bit.remoteRequestFromOwiMaster = errStatus;
        break;

    default:
        break;

    }

    if(1u == updateErrorLed)
    {
        PV624->userInterface->statusLedControl(eStatusError,
                                               E_LED_OPERATION_SWITCH_ON,
                                               65535u,
                                               E_LED_STATE_SWITCH_ON,
                                               1u);
    }
}

/**
* @brief    updateDeviceStatus - updates the device status based on the error code
* @param    errorCode - enumerated erro code  value
* @param    eErrorStatus_t - error status 0: to clear error and 1:to Set Error
* @return   None
*/
void DErrorHandler::performActionOnError(eErrorCode_t errorCode,
        eErrorStatus_t errStatus)
{
    switch(errorCode)
    {
    case E_ERROR_LOW_REFERENCE_SENSOR_VOLTAGE:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->stopMotor();
            PV624->ventSystem();
        }

        break;

    case E_ERROR_REFERENCE_SENSOR_COM:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->stopMotor();
            PV624->ventSystem();
        }

        break;

    case E_ERROR_BAROMETER_SENSOR:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->stopMotor();
            PV624->ventSystem();
        }

        break;

    case E_ERROR_STEPPER_CONTROLLER:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->resetStepperMicro();
            PV624->ventSystem();
        }

        break;

    case E_ERROR_MOTOR_VOLTAGE:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->holdStepperMicroInReset();
            PV624->ventSystem();
        }

        else
        {

        }

        break;

    case E_ERROR_STEPPER_DRIVER:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->resetStepperMicro();
            PV624->ventSystem();
        }

        break;

    case E_ERROR_VALVE:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->stopMotor();
            PV624->ventSystem();
        }

        break;

    case E_ERROR_EEPROM:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->stopMotor();
            PV624->ventSystem();
        }

        break;


    case E_ERROR_BATTERY_CRITICAL_LEVEL:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->stopMotor();
            PV624->ventSystem();
        }

        break;

    case E_ERROR_ON_BOARD_FLASH:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->stopMotor();
            PV624->ventSystem();
        }

        break;



    case E_ERROR_BATTERY_COMM:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->stopMotor();
            PV624->ventSystem();
        }

        break;

    case E_ERROR_BATTERY_CHARGER_COMM:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->stopMotor();
            PV624->ventSystem();
        }

        break;


    default:
        break;

    }
}