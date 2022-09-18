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
* @file     DInstrument.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The instrument class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DInstrument.h"
#include "DFunctionMeasureAndControl.h"

/* Error handler instance parameter starts from 3501 to 3600 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DInstrument class constructor
 * @param   void
 * @retval  void
 */
DInstrument::DInstrument(OS_ERR *osErr)
{
    myCurrentFunction = new DFunctionMeasureAndControl();

    if(osErr != NULL)
    {
        *osErr = (OS_ERR)OS_ERR_NONE;
    }
}

/**
 * @brief   DInstrument class deconstructor
 * @param   void
 * @retval  void
 */
DInstrument::~DInstrument()
{
    if(NULL != myCurrentFunction)
    {
        delete myCurrentFunction;
    }
}
/**
 * @brief   Set Instrument function
 * @param   func is the function itself
 * @retval  true if activated successfully, else false
 */
bool DInstrument::setFunction(eFunction_t func)
{
    bool successFlag = false;

    if(func < (eFunction_t)E_FUNCTION_MAX)
    {
        successFlag = myCurrentFunction->setFunction(func);
    }

    return successFlag;
}

/**
 * @brief   get Instrument function
 * @param   func is the function itself
 * @retval  true if all's well, else false
 */
bool DInstrument::getFunction(eFunction_t *func)
{
    return  myCurrentFunction->getFunction(func);
}
/**
 * @brief   Get specified value of currently running function
 * @param   index is function specific meaning identified a specific output parameter
 * @param   pointer to variable for return of value
 * @retval  true if all's well, else false
 */
bool DInstrument::getReading(eValueIndex_t index, float32_t *reading)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(index, reading);
    }

    return successFlag;
}

/**
 * @brief   Signal sensor to continue
 * @param   void
 * @retval  true if all's well, else false
 */
bool DInstrument::sensorContinue(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->sensorContinue();
    }

    return successFlag;
}

/**
 * @brief   Signal sensor to retry after failure reported
 * @param   void
 * @retval  true if all's well, else false
 */
bool DInstrument::sensorRetry(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->sensorRetry();
    }

    return successFlag;
}
/**
 * @brief   Get positive fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getPosFullscale(float32_t *fs)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_SENSOR_POS_FS, fs);
    }

    return successFlag;
}

/**
 * @brief   Get negative fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getNegFullscale(float32_t *fs)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_SENSOR_NEG_FS, fs);
    }

    return successFlag;

}

/**
 * @brief   Get sensor type
 * @param   pSenType - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getSensorType(eSensorType_t *pSenType)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        if(NULL != pSenType)
        {
            myCurrentFunction->getValue(E_VAL_INDEX_SENSOR_TYPE, (uint32_t *)pSenType);
            successFlag = true;
        }
    }

    return successFlag;
}


/**
 * @brief   Get PM620 Type
 * @param   sensorType is pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getPM620Type(uint32_t *sensorType)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        if(NULL != sensorType)
        {
            myCurrentFunction->getValue(E_VAL_INDEX_PM620_TYPE, (uint32_t *)sensorType);
            successFlag = true;
        }
    }

    return successFlag;
}
/**
 * @brief   Get cal interval
 * @param   interval - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getCalInterval(uint32_t *interval)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        myCurrentFunction->getCalInterval(interval);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Set cal interval
 * @param   cal interval value
 * @retval  true = success, false = failed
 */
bool DInstrument::setCalInterval(uint32_t sensor, uint32_t interval)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        myCurrentFunction->setCalInterval(sensor, interval);
        successFlag = true;
    }

    return successFlag;

}

/**
 * @brief   Get sensor manufacturing date
 * @param   manfDate - pointer to date variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getManufactureDate(sDate_t *manfDate)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        myCurrentFunction->getManufactureDate(manfDate);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Get sensor calibration date
 * @param   caldate - pointer to variable for return value (cal date)
 * @retval  true = success, false = failed
 */
bool DInstrument::getUserCalDate(sDate_t *caldate)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        myCurrentFunction->getCalDate((eSensorCalType_t)E_SENSOR_CAL_TYPE_USER,
                                      caldate);
        successFlag = true;
    }

    return successFlag;


}

/**
 * @brief   Get Barometer Manufacture ID
 * @param   identity - pointer to variable for return value (Barometer Identity )
 * @retval  true = success, false = failed
 */
bool DInstrument::getBarometerIdentity(uint32_t *identity)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(EVAL_INDEX_BAROMETER_ID,
                      identity);
    }

    return successFlag;
}

/**
 * @brief   Get PM620 sendor App Identity ID
 * @param   identity - pointer to variable for return value (Sensor Application identity ID)
 * @retval  true = success, false = failed
 */
bool DInstrument::getExternalSensorAppIdentity(uSensorIdentity_t *identity)
{
    bool successFlag = false;
    uint32_t val = 0u;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_PM620_APP_IDENTITY,
                      &val);

        if(true == successFlag)
        {
            identity->value = val;
        }

        else
        {
            identity->value = 0u;
        }
    }

    return successFlag;
}

/**
 * @brief   Get PM620 sendor Bootloader Identity ID
 * @param   identity -  - pointer to variable for return value (Sensor bootloader identity ID)
 * @retval  true = success, false = failed
 */
bool DInstrument::getExternalSensorBootLoaderIdentity(uSensorIdentity_t *identity)
{
    bool successFlag = false;
    uint32_t val = 0u;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_PM620_BL_IDENTITY,
                      &val);

        if(true == successFlag)
        {
            identity->value = val;
        }

        else
        {
            identity->value = 0u;
        }
    }

    return successFlag;
}

/**
 * @brief   Get controller mode
 * @param   controllerMode - pointer to variable for return value (controller  mode)
 * @retval  true = success, false = failed
 */
bool DInstrument::getControllerMode(eControllerMode_t *controllerMode)
{
    bool successFlag = false;
    uint32_t val = 0u;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_CONTROLLER_MODE,
                      &val);

        if(true == successFlag)
        {
            *controllerMode = (eControllerMode_t)val;
        }

        else
        {
            *controllerMode = (eControllerMode_t)E_CONTROLLER_MODE_NONE;
        }
    }

    return successFlag;
}

/**
 * @brief   Set controller mode
 * @param   controllerMode - new controller mode
 * @retval  true = success, false = failed
 */

bool DInstrument::setControllerMode(eControllerMode_t newCcontrollerMode)
{
    bool successFlag = false;
    uint32_t val = newCcontrollerMode;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->setValue(E_VAL_INDEX_CONTROLLER_MODE,
                      val);

    }

    return successFlag;
}

/**
 * @brief   Get controller mode
 * @param   controllerMode - pointer to variable for return value (controller  mode)
 * @retval  true = success, false = failed
 */
bool DInstrument::setVentRate(float rate)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->setValue(E_VAL_INDEX_VENT_RATE,
                      rate);

    }

    return successFlag;
}

/**
 * @brief   Get controller mode
 * @param   controllerMode - pointer to variable for return value (controller  mode)
 * @retval  true = success, false = failed
 */
bool DInstrument::getVentRate(float *rate)
{
    bool successFlag = false;
    float val = 0.0f;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_VENT_RATE,
                      &val);

        if(true == successFlag)
        {
            *rate = val;
        }
    }

    return successFlag;
}


/**
 * @brief   take readings at requested rate
 * @param   rate -
 * @retval  void
 */
void DInstrument::takeNewReading(uint32_t rate)
{
    myCurrentFunction->takeNewReading(rate);
}

/**
 * @brief   read controller pressure set point
 * @param   setPoint - pointer to variable for return value (controller pressure set point)
 * @retval  true = success, false = failed
 */
bool DInstrument::getPressureSetPoint(float *setPoint)
{
    bool successFlag = false;

    if((myCurrentFunction != NULL) && (NULL != setPoint))
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_PRESSURE_SETPOINT,
                      setPoint);
    }

    return successFlag;
}

/**
 * @brief   write controller pressure set point
 * @param   setPoint - controller new pressure set point)
 * @retval  true = success, false = failed
 */
bool DInstrument::setPressureSetPoint(float newSetPointValue)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->setValue(E_VAL_INDEX_PRESSURE_SETPOINT,
                      newSetPointValue);

    }

    return successFlag;
}


/**
 * @brief   Set calibration type
 * @param   calType - function specific calibration type (0 = user calibration)
 * @param   range - sensor range
 * @retval  true = success, false = failed
 */
bool DInstrument::setCalibrationType(int32_t calType, uint32_t range)
{
    bool success = false;

    if(myCurrentFunction != NULL)
    {
        success = myCurrentFunction->setCalibrationType(calType, range);
    }

    return success;
}

/**
 * @brief   Get required number of calibration points
 * @param   void
 * @retval  true = success, false = failed
 */
bool DInstrument::getRequiredNumCalPoints(eSensor_t sensorType, uint32_t *numCalPoints)
{
    bool success = false;

    if(myCurrentFunction != NULL)
    {
        success = myCurrentFunction->getRequiredNumCalPoints(sensorType, numCalPoints);
    }

    return success;
}


/**
 * @brief   set required number of calibration points
 * @param   uint32_t   Number of cal points
 * @retval  true = success, false = failed
 */
bool DInstrument::setRequiredNumCalPoints(uint32_t numCalPoints)
{
    bool success = false;

    if(myCurrentFunction != NULL)
    {
        success = myCurrentFunction->setRequiredNumCalPoints(numCalPoints);
    }

    return success;
}
/**
 * @brief   Start sampling at current cal point
 * @param   void
 * @retval  true = success, false = failed
 */
bool DInstrument::startCalSampling(void)
{
    bool success = false;

    if(myCurrentFunction != NULL)
    {
        success = myCurrentFunction->startCalSampling();
    }

    return success;
}

/**
 * @brief   Get remaining number of samples at current cal point
 * @param   pointer to variable for return value (remaining number of samples)
 * @retval  true = success, false = failed
 */
bool DInstrument::getCalSamplesRemaining(uint32_t *samples)
{
    bool success = false;

    if(myCurrentFunction != NULL)
    {
        success = myCurrentFunction->getCalSamplesRemaining(samples);
    }

    return success;
}

/**
 * @brief   Set calibration point
 * @param   point indicates the cal point number (1 - required no of points)
 * @param   user supplied calibration value
 * @retval  true = success, false = failed
 */
bool DInstrument::setCalPoint(uint32_t calPoint, float32_t value)
{
    bool success = false;

    if(myCurrentFunction != NULL)
    {
        success = myCurrentFunction->setCalPoint(calPoint, value);
    }

    return success;
}

/**
 * @brief   Cal accept
 * @param   void
 * @retval  true = success, false = failed
 */
bool DInstrument::acceptCalibration(void)
{
    bool success = false;

    if(myCurrentFunction != NULL)
    {
        success = myCurrentFunction->acceptCalibration();
    }

    return success;
}

/**
 * @brief   Abort calibration
 * @param   void
 * @retval  true = success, false = failed
 */
bool DInstrument::abortCalibration(void)
{
    bool success = false;

    if(myCurrentFunction != NULL)
    {
        success = myCurrentFunction->abortCalibration();
    }

    return success;
}

/**
 * @brief   Get Controller Status
 * @param   uint32_t* pointer to variable for return value --- controller status
 * @retval  true = success, false = failed
 */
bool DInstrument::getControllerStatus(uint32_t *controllerStatus)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_CONTROLLER_STATUS,
                      controllerStatus);

    }

    return successFlag;

}

/**
 * @brief   Set Controller Status
 * @param   uint32_t controller status
 * @retval  true = success, false = failed
 */
bool DInstrument::setControllerStatus(uint32_t controllerStatus)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->setValue(E_VAL_INDEX_CONTROLLER_STATUS,
                      controllerStatus);

    }

    return successFlag;
}
/**
 * @brief   Reload calibration data
 * @param   void
 * @retval  flag: true = success, false = failed
 */
bool DInstrument::reloadCalibration(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->reloadCalibration();

    }

    return successFlag;

}


/**
 * @brief   Get cal date
 * @param   pointer to date structure for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getCalDate(sDate_t *date)
{

    bool successFlag = false;

    if((myCurrentFunction != NULL) && (date != NULL))
    {
        successFlag = myCurrentFunction->getCalDate(date);

    }

    return successFlag;

}

/**
 * @brief   Set cal date
 * @param   pointer to date structure
 * @retval  true = success, false = failed
 */
bool DInstrument::setCalDate(sDate_t *date)
{

    bool successFlag = false;

    if((myCurrentFunction != NULL) && (date != NULL))
    {
        successFlag = myCurrentFunction->setCalDate(date);

    }

    return successFlag;
}

/**
 * @brief   Set cal date
 * @param   pointer to date structure for return value --- Sensor calibration date
 * @retval  true = success, false = failed
 */
bool DInstrument::getSensorCalDate(sDate_t *date)
{
    bool successFlag = false;

    if((myCurrentFunction != NULL) && (date != NULL))
    {
        successFlag = myCurrentFunction->getSensorCalDate(date);

    }

    return successFlag;
}

/**
 * @brief   get Sensor serial number
 * @param   pointer to variable for return value --- Sensor serial nuber
 * @retval  true = success, false = failed
 */
bool DInstrument::getSensorSerialNumber(uint32_t *sn)
{
    bool successFlag = false;

    if((myCurrentFunction != NULL) && (sn != NULL))
    {
        successFlag = myCurrentFunction->getSensorSerialNumber(sn);

    }

    return successFlag;
}

/**
 * @brief   get pressure reading
 * @param   pointer to variable for return value --- measured pressure value
 * @retval  true = success, false = failed
 */
bool DInstrument::getPressureReading(float *pressure)
{
    bool successFlag = false;

    if((myCurrentFunction != NULL) && (pressure != NULL))
    {
        successFlag = myCurrentFunction->getPressureReading(pressure);

    }

    return successFlag;
}

/**
 * @brief   get positive full scale value
 * @param   pointer to variable for return value --- measured pressure value
 * @retval  true = success, false = failed
 */
bool DInstrument::getPositiveFS(float *pressure)
{
    bool successFlag = false;

    if((myCurrentFunction != NULL) && (pressure != NULL))
    {
        successFlag = myCurrentFunction->getPositiveFS(pressure);

    }

    return successFlag;
}

/**
 * @brief   get negatvie full scale value
 * @param   pointer to variable for return value --- measured pressure value
 * @retval  true = success, false = failed
 */
bool DInstrument::getNegativeFS(float *pressure)
{
    bool successFlag = false;

    if((myCurrentFunction != NULL) && (pressure != NULL))
    {
        successFlag = myCurrentFunction->getNegativeFS(pressure);

    }

    return successFlag;
}

/**
 * @brief   get sensor brand units
 * @param   pointer to variable for return value --- brand unit value
 * @retval  true = success, false = failed
 */
bool DInstrument::getSensorBrandInfo(char *brandMin, char *brandMax, char *brandType, char *brandUnits)
{
    bool successFlag = false;

    if((myCurrentFunction != NULL) && (brandUnits != NULL))
    {
        successFlag = myCurrentFunction->getSensorBrandInfo(brandMin, brandMax, brandType, brandUnits);
    }

    return successFlag;
}

/**
 * @brief   get sensor brand units
 * @param   pointer to variable for return value --- brand unit value
 * @retval  true = success, false = failed
 */
bool DInstrument::initController(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->initController();
    }

    return successFlag;
}

/**
 * @brief   Sets aquisation mode of pressure slot and barometer slot
 * @param   newAcqMode : new Aquisation mode
 * @retval  void
 */
bool DInstrument::setAquisationMode(eAquisationMode_t newAcqMode)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->setAquisationMode(newAcqMode);
    }

    return successFlag;
}

/**
 * @brief   Stop the instrument function
 * @param   pointer to variable for return value --- brand unit value
 * @retval  true = success, false = failed
 */
void DInstrument::startup(void)
{
    if(myCurrentFunction != NULL)
    {
        myCurrentFunction->startUnit();
    }
}

/**
 * @brief   Start the instrument function
 * @param   pointer to variable for return value --- brand unit value
 * @retval  true = success, false = failed
 */
void DInstrument::shutdown(void)
{
    if(myCurrentFunction != NULL)
    {
        myCurrentFunction->shutdownUnit();
    }
}

/**
 * @brief   upgrades sensor firmware
 * @param   void
 * @retval  true = success, false = failed
 */
bool DInstrument::upgradeSensorFirmware(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->upgradeSensorFirmware();
    }

    return successFlag;
}

/**
 * @brief   upgrades sensor firmware
 * @param   void
 * @retval  true = success, false = failed
 */
bool DInstrument::opticalEvent(uint32_t eventNum)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        if(1u == eventNum)
        {
            myCurrentFunction->postEvent(EV_FLAG_OPT_INTERRUPT_1);
        }

        if(2u == eventNum)
        {
            myCurrentFunction->postEvent(EV_FLAG_OPT_INTERRUPT_2);
        }
    }

    return successFlag;
}

/**
 * @brief   Get positive fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getBaroPosFullscale(float32_t *fs)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_BARO_SENSOR_POS_FS, fs);
    }

    return successFlag;
}

/**
 * @brief   Get negative fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getBaroNegFullscale(float32_t *fs)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_BARO_SENSOR_NEG_FS, fs);
    }

    return successFlag;

}

/**
 * @brief   moves the motor till forward end and then return to home position
 * @param   none
 * @retval  true = success, false = failed
 */
bool DInstrument::moveMotorTillForwardEnd(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->moveMotorTillForwardEnd();
    }

    return successFlag;
}

/**
 * @brief   moves the motor till reverse end and then return to home position
 * @param   none
 * @retval  true = success, false = failed
 */
bool DInstrument::moveMotorTillReverseEnd(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->moveMotorTillReverseEnd();
    }

    return successFlag;
}

/**
 * @brief   moves the motor till forward end and then return to home position
 * @param   none
 * @retval  true = success, false = failed
 */
bool DInstrument::moveMotorTillForwardEndThenHome(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->moveMotorTillForwardEndThenHome();
    }

    return successFlag;
}

/**
 * @brief   moves the motor till reverse end and then return to home position
 * @param   none
 * @retval  true = success, false = failed
 */
bool DInstrument::moveMotorTillReverseEndThenHome(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->moveMotorTillReverseEndThenHome();
    }

    return successFlag;
}

bool DInstrument::setSensorZeroValue(uint32_t sensor, float zeroVal)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->setSensorZeroValue(sensor, zeroVal);
    }

    return successFlag;
}

bool DInstrument::getSensorZeroValue(uint32_t sensor, float *zeroVal)
{
    bool successFlag = false;

    if((myCurrentFunction != NULL) && (zeroVal != NULL))
    {
        successFlag = myCurrentFunction->getSensorZeroValue(sensor, zeroVal);
    }

    return successFlag;
}

bool DInstrument::shutdownPeripherals(void)
{
    bool successFlag = false;

    if(myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->shutdownPeripherals();
    }

    return successFlag;
}