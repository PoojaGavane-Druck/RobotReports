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
#include "Utilities.h"
/* Error handler instance parameter starts from 3001 to 3100 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

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

    deviceStatus_t mask;

    mask.bytes = 0u;

    // mask the following errors to not be logged
    mask.bit.batteryWarningLevel = 1u;
    mask.bit.overTemperature = 1u;
    mask.bit.chargingStatus = 1u;
    mask.bit.barometerSensorCalStatus = 1u;
    mask.bit.remoteRequestFromBtMaster = 1u;
    mask.bit.remoteRequestFromOwiMaster = 1u;
    mask.bit.dueForService = 1u;

    // Mask reserved bits also

    mask.bit.Reserved1 = 1u;
    mask.bit.Reserved2 = 1u;

    errorBitMaskForLogging = ~(mask.bytes);

    // Set the PM 620 comms error at startup, as sensor will take 7 seconds to connect
    deviceStatus.bit.referenceSensorCommFail = 1u;

    //check if device is due for service if yes set dueFOrServiceBit
    if(PV624->isDeviceDueForService())
    {
        deviceStatus.bit.dueForService = 1u;
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

    uint32_t testVal = 0u;
    uint32_t testVal2 = 0u;

    prevDeviceStatus.bytes = deviceStatus.bytes;

    updateDeviceStatus(errorCode, errStatus);

    testVal = prevDeviceStatus.bytes & (errorBitMaskForLogging);
    testVal2 = deviceStatus.bytes & (errorBitMaskForLogging);

    if((prevDeviceStatus.bytes & (errorBitMaskForLogging)) != (deviceStatus.bytes & (errorBitMaskForLogging)))
    {

        PV624->logger->logError(errorCode, errStatus, paramValue, errInstance, isFatal);
        performActionOnError(errorCode, errStatus);
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

    case E_ERROR_BAROMETER_SENSOR_COM:
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

    case E_ERROR_OVER_PRESSURE:
        deviceStatus.bit.overPressure = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_VALVE:
        deviceStatus.bit.valveFail = errStatus;
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

    case E_ERROR_BAROMETER_OUT_OF_CAL:
        deviceStatus.bit.barometerOutOfCal = errStatus;
        break;

    case E_ERROR_CODE_EXTERNAL_STORAGE:
        deviceStatus.bit.extFlashFailure = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_ON_BOARD_FLASH:
        deviceStatus.bit.onboardFlashFail = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
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

    case E_ERROR_BAROMETER_SENSOR_CAL_STATUS:
        deviceStatus.bit.barometerSensorCalStatus = errStatus;
        break;


    case E_ERROR_CHARGER_CONNECTED:
        deviceStatus.bit.chargingStatus = errStatus;
        break;

    case E_ERROR_CODE_REMOTE_REQUEST_FROM_BT_MASTER:
        deviceStatus.bit.remoteRequestFromBtMaster = errStatus;
        break;

    case E_ERROR_CODE_REMOTE_REQUEST_FROM_OWI_MASTER:
        deviceStatus.bit.remoteRequestFromOwiMaster = errStatus;
        break;

    case E_ERROR_DEVICE_DUE_FOR_SERVICE:
        deviceStatus.bit.dueForService = errStatus;
        break;

    case E_ERROR_OPTICAL_BOARD_NOT_FOUND:
        deviceStatus.bit.opticalBoardFail = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_BAROMETER_CAL_REJECT:
        deviceStatus.bit.barometerSensorCalRejected = errStatus;
        break;

    case E_ERROR_BAROMETER_CAL_DEFAULT:
        deviceStatus.bit.barometerCalDefault = errStatus;
        break;

    case E_ERROR_CODE_DRIVER_BLUETOOTH:
        deviceStatus.bit.bl652CommFailure = errStatus;
        break;

    case E_ERROR_CODE_FIRMWARE_UPGRADE_FAILED:
        deviceStatus.bit.upgradeFailed = errStatus;
        break;

    case E_ERROR_CPU_AND_STACK_AND_CLOCK_TEST_FAILED:
        deviceStatus.bit.cpuAndStackClockFailed = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
        break;

    case E_ERROR_CODE_RAM_FAILED:
        deviceStatus.bit.ramFailed = errStatus;
        updateErrorLed = (uint32_t)(errStatus);
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
            PV624->ventSystem();
        }

        break;

    case E_ERROR_REFERENCE_SENSOR_COM:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            PV624->ventSystem();
        }

        break;

    case E_ERROR_BAROMETER_SENSOR_COM:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
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
        }


        break;

    case E_ERROR_OVER_PRESSURE:
        /* FunctionMeasureAndCOntrol task will change the mode to measure mode
          to isolate the valves from hand pump */
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
            PV624->ventSystem();
            // Handle shutdown event here
            sleep(DELAY_BEFORE_SHUTDOWN);
            PV624->shutdown();
        }

        break;

    case E_ERROR_ON_BOARD_FLASH:
        if(errStatus == (eErrorStatus_t)eSetError)

        {
            PV624->ventSystem();
        }


        break;


    case E_ERROR_OPTICAL_BOARD_NOT_FOUND:
        if(errStatus == (eErrorStatus_t)eSetError)

        {
            PV624->ventSystem();
        }


        break;

    case E_ERROR_BATTERY_COMM:
        if(errStatus == (eErrorStatus_t)eSetError)
        {
            /* Conforming hardware team if operating PV624 on only charger is allowed
               if it is allowed we need to remove this action */
            PV624->ventSystem();
        }

        break;

    case E_ERROR_BATTERY_CHARGER_COMM:
        /* whenever  battery enters into critical low level it calls vent system.
          that will ctake care by CRITICAL_LOW_BATTERY error */

        break;


    default:
        break;

    }
}