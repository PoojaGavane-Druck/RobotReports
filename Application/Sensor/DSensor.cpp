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
* @file     DSensor.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     15 April 2020
*
* @brief    The sensor base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"
#include "DSensor.h"

MISRAC_DISABLE
#include <stdio.h>
#include <math.h>
#include <string.h>
MISRAC_ENABLE

#include "crc.h"
#include "Utilities.h"
#include "DPV624.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSensor class constructor
 * @param   void
 * @retval  void
 */
DSensor::DSensor()
{
       OS_ERR os_error = (OS_ERR)OS_ERR_NONE;

    resetStatus();                      //clean bill of health to start with

    myDevice = NULL;                    //no device

    myIdentity.value = 0u;              //arbitrary initialisation

    myType = E_SENSOR_TYPE_GENERIC;

    myLatency = 0u;                     //time (ms) to take a measurement, determined empirically for each measurement

    myFilter = NULL;                    //no input filtering by default

    myFsMaximum = 100.0f;               //arbitrary initialisation of positive fullscale
    myFsMinimum = 0.0f;                 //arbitrary initialisation of negative fullscale
    myAbsFsMaximum = 100.0f;            //arbitrary initialisation of absolute maximum value to be applied to this sensor
    myAbsFsMinimum = 0.0f;              //arbitrary initialisation of absolute minimum value to be applied to this sensor

    mySerialNumber = 0u;                //sensor serial number

    myCalData = NULL;                   //set pointer to calibration data
    myCalSampleCount = 0u;              //sample counter value (during calibration)
    myCalSamplesAccumulator = 0.0f;     //accumluation of sample values during calibration (used for averaging)
    myNumCalPoints = 2u;                //default is 2-point cal
    myCalSamplesRequired = 1u;          //number of cal samples at each cal point for averaging
    myCalInterval = DEFAULT_CAL_INTERVAL;

    myUserCalDate.day = 25u;             //sensor calibration date
    myUserCalDate.month = 12u;
    myUserCalDate.year = 2020u;

    myManufactureDate.day = 25u;         //sensor manufacture date
    myManufactureDate.month = 12u;
    myManufactureDate.year = 2020u;

    myMeasuredValue = 0.0f;
    //create mutex for resource locking
    char *name = "Sen";

    memset((void*)&myMutex, 0, sizeof(OS_MUTEX));
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &os_error);

    if (os_error != (OS_ERR)OS_ERR_NONE)
    {
        myStatus.fault = 1u;        //set error status bit
    }

    setMode(E_SENSOR_MODE_NORMAL);
}

/**
 * @brief   Create sensor ranges
 * @param   void
 * @retval  void
 */
void DSensor::createRanges(void)
{
}

/**
 * @brief   Initialisation function
 * @param   void
 * @retval  void
 */
eSensorError_t DSensor::initialise(void)
{
     resetStatus();

    setMode(E_SENSOR_MODE_NORMAL);

    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Close sensor
 * @param   void
 * @retval  void
 */
eSensorError_t DSensor::close(void)
{
    return E_SENSOR_ERROR_NONE;
}



/**
 * @brief   Generate compensated value (by applying calibration data to raw measured value)
 * @param   raw measurement
 * @retval  compensated value
 */
float32_t DSensor::compensate(float32_t rawReading)
{
    float32_t compValue = rawReading;

    //perform compensation here

    return compValue;
}

/**
 * @brief   Query if calibration is allowed on this sensor
 * @param   pointer to variable for return value
 * @retval  sensor error code
 */
bool DSensor::isCalibratable()
{
    return false;
}



/**
 * @brief   Get resolution for current sensor range
 * @param   void
 * @retval  resolution of measured value
 */
float32_t DSensor::getResolution()
{
   DLock is_on(&myMutex);
    return myResolution;
}
/**
 * @brief   Get sensor status
 * @param   void
 * @retval  sensor status (meaning is sensor specific)
 */
sSensorStatus_t DSensor::getStatus(void)
{
    DLock is_on(&myMutex);
    return myStatus;
}

/**
 * @brief   Reset (clear) sensor status
 * @param   void
 * @retval  void
 */
void DSensor::resetStatus(void)
{
    myStatus.value = 0u;              //clear all status bits
    myLastStatus.value = 0u;          //last value of current sensor status (used for change notification)
    myStatusChanges.value = 0u;       //changes in sensor status since last read (cleared on every read)
}

/**
 * @brief   Reset (clear) specified sensor status bits
 * @param   sensor status (meaning is sensor specific)
 * @retval  void
 */
void DSensor::resetStatus(sSensorStatus_t status)
{
    DLock is_on(&myMutex);
    myStatus.value &= ~status.value;                 //clear specified bits in current status field
    myLastStatus.value &= ~myLastStatus.value;       //clear specified bits in last value of current status field
    myStatusChanges.value &= ~myStatusChanges.value; //clear specified bits in sensor status changes field
}

/**
 * @brief   Set sensor status
 * @param   sensor status (meaning is sensor specific)
 * @retval  void
 */
void DSensor::setStatus(sSensorStatus_t status)
{
    DLock is_on(&myMutex);

    //update current sensor status value
    myStatus.value |= status.value;

    //compare with last value of sensor status and check for changes and add any new changes to other unread ones
    myStatusChanges.value |= (myLastStatus.value ^ myStatus.value);

    //not interested in the cal status bits, so mask those off, leaving only the sensor status bits
    myStatusChanges.value &= SENSOR_STATUS_BITS_MASK;

    //update for next time
    myLastStatus = status;
}

/**
 * @brief   Clear specified sensor status bits
 * @param   sensor status (meaning is sensor specific)
 * @retval  void
 */
void DSensor::clearStatus(sSensorStatus_t status)
{
    DLock is_on(&myMutex);

    //update current sensor status value
    myStatus.value &= ~status.value;

    //compare with last value of sensor status and check for changes and add any new changes to other unread ones
    myStatusChanges.value |= (myLastStatus.value ^ myStatus.value);

    //not interested in the cal status bits, so mask those off, leaving only the sensor status bits
    myStatusChanges.value &= SENSOR_STATUS_BITS_MASK;

    //update for next time
    myLastStatus = status;
}

/**
 * @brief   Get sensor status changes since last read
 * @note    The status is cleared after reading
 * @param   void
 * @retval  sensor status changed bits only
 */
sSensorStatus_t DSensor::getStatusChanges(void)
{
    DLock is_on(&myMutex);

    //get unreported changes
    sSensorStatus_t unReportedChanges = myStatusChanges;

    //changes in sensor status once last read (cleared on every read)
    myStatusChanges.value = 0u;

    return unReportedChanges;
}

/**
 * @brief   Set filter enabled state
 * @param   state: true = enabled; false = disabled
 * @retval  void
 */
void DSensor::setFilterEnabled(bool state)
{
    DLock is_on(&myMutex);

    if (myFilter != NULL)
    {
       myFilter->setEnabled(state);
   }

}

/**
 * @brief   Get filter enabled state
 * @param   void
 * @retval  state: true = enabled; false = disabled
 */
bool DSensor::getFilterEnabled()
{
    bool enabled = false;

    DLock is_on(&myMutex);

   if (myFilter != NULL)
    {
        enabled = myFilter->getEnabled();
    }

    return enabled;
}




/**
 * @brief   Perform sensor measurement
 * @param   none
 * @retval  sensor error code
 */
eSensorError_t DSensor::measure(void)
{
    DLock is_on(&myMutex);
    return E_SENSOR_ERROR_UNAVAILABLE;
}

eSensorError_t DSensor::measure(uint32_t channelSelection)
{
  return E_SENSOR_ERROR_UNAVAILABLE;
}
/**
 * @brief   Set sample count
 * @param   sample count
 * @retval  void
 */
void DSensor::setSampleCount(uint32_t value)
{
    DLock is_on(&myMutex);
    myCalSampleCount = value;
}

/**
 * @brief   Get number of samples to average over for each calibration point
 * @param   void
 * @retval  no of sample
 */
uint32_t DSensor::getRequiredCalSamples(void)
{
    return myCalSamplesRequired;
}

/**
 * @brief   Get sample count
 * @param   void
 * @retval  sample count
 */
uint32_t DSensor::getSampleCount(void)
{
    DLock is_on(&myMutex);
    return myCalSampleCount;
}

/**
 * @brief   Set sensor mode
 * @param   pointer to variable for return value
 * @retval  sensor error code
 */
void DSensor::setMode(eSensorMode_t mode)
{
    DLock is_on(&myMutex);

    //check if there is a change in mode
    if (myMode != mode)
    {
        myMode = mode;

        //no cal functions are allowed
        myStatus.canSetCalType = 1u;        //set calibration type is allowed
        myStatus.canStartSampling = 0u;     //start sampling is not allowed yet
        myStatus.canQuerySampling = 0u;     //query cal samples remaining is not allowed yet
        myStatus.canSetCalPoint = 0u;       //set calibration point is not allowed yet
        myStatus.canAcceptCal = 0u;         //calibration accept is not allowed yet
        myStatus.canAbortCal = 0u;          //calibration abort is not allowed yet

        myStatus.canSetCalDate = 0u;        //set calibration date is not allowed
        myStatus.canSetCalInterval = 0u;    //set calibration interval is not allowed
    }
}

/**
 * @brief   Get sensor mode
 * @param   pointer to variable for return value
 * @retval  sensor error code
 */
eSensorMode_t DSensor::getMode()
{
    DLock is_on(&myMutex);
    return myMode;
}


/**
 * @brief   Set calibration date
 * @param   date of calibration
 * @retval  void
 */
bool DSensor::setCalDate(sDate_t *date)
{
    bool flag = false;

    DLock is_on(&myMutex);

    //date is already validated - update it in sensor and/or persistent storage as well
    if (myCalData != NULL)
    {
        if (myCalData->hasCalData() == true)
        {
            myCalData->saveCalDate(date);  
            
            //reload calibration from persistent storage
            loadCalibrationData();
        }
        
    }

    return flag;
}
/**
 * @brief   get calibration date
 * @param   pointer to variable for return value
 * @retval  void
 */
bool DSensor::getCalDate(sDate_t* date)
{
    DLock is_on(&myMutex);
    date->day = myUserCalDate.day;
    date->month = myUserCalDate.month;
    date->year = myUserCalDate.year;

    return true;
}

/**
 * @brief   Set validated calibration interval (in number of days)
 * @param   interval value
 * @retval  void
 */
bool DSensor::setCalInterval(uint32_t interval)
{
    bool flag = false;

    DLock is_on(&myMutex);

    //date is already validated - update it in sensor and/or persistent storage as well
    if (myCalData != NULL)
    {
        if (myCalData->hasCalData() == true)
        {
          myCalData->saveCalInterval(interval);
 
          //reload calibration from persistent storage
          loadCalibrationData();
        }
    }

    return flag;
}
/**
 * @brief   Set calibration interval value (in number of days)
 * @param   interval value
 * @retval  void
 */
bool DSensor::setCalIntervalValue(uint32_t interval)
{
    DLock is_on(&myMutex);
    myCalInterval = interval;
    return true;
}

/**
 * @brief   Get calibration interval (in number of days)
 * @param   pointer to variable for return value
 * @retval  sensor error code
 */
bool DSensor::getCalInterval(uint32_t *interval)
{
    DLock is_on(&myMutex);
    *interval = myCalInterval;

    return true;
}



/**
 * @brief   Get sensor serial number
 * @param   void
 * @retval  sensor serial number
 */
uint32_t DSensor::getSerialNumber(void)
{
    DLock is_on(&myMutex);
    return mySerialNumber;
}

/**
 * @brief   Set sensor identity
 * @param   identity
 * @retval  void
 */
void DSensor::setIdentity(uSensorIdentity_t identity)
{
    DLock is_on(&myMutex);
    myIdentity = identity;
}

/**
 * @brief   Get sensor identity
 * @param   void
 * @retval  identity
 */
uSensorIdentity_t DSensor::getIdentity(void)
{
    DLock is_on(&myMutex);
    return myIdentity;
}



/**
 * @brief   Set user cal date
 * @param   user cal date
 * @retval  void
 */
void DSensor::setCalDateValue(sDate_t *date)
{
    DLock is_on(&myMutex);
    myUserCalDate.day = date->day;
    myUserCalDate.month = date->month;
    myUserCalDate.year = date->year;
}

/**
 * @brief   Get user cal date
 * @param   void
 * @retval  user cal date
 */
sDate_t *DSensor::getUserCalDate(void)
{
    DLock is_on(&myMutex);
    return &myUserCalDate;
}

/**
 * @brief   Set manufacture date
 * @param   manufacture date
 * @retval  void
 */
void DSensor::setManufactureDate(sDate_t *date)
{
    DLock is_on(&myMutex);
    myManufactureDate.day = date->day;
    myManufactureDate.month = date->month;
    myManufactureDate.year = date->year;
}

/**
 * @brief   Get manufacture date
 * @param   void
 * @retval  manufacture date
 */
void DSensor::getManufactureDate(sDate_t  *date)
{
    DLock is_on(&myMutex);
    *date = myManufactureDate;
}



/**
 * @brief   Set sensor type
 * @param   sensor type
 * @retval  void
 */
void DSensor::setSensorType(eSensorType_t sensorType)
{
    DLock is_on(&myMutex);
    myType = sensorType;
}

/**
 * @brief   Get sensor type
 * @param   void
 * @retval  sensor type
 */
eSensorType_t DSensor::getSensorType(void)
{
    DLock is_on(&myMutex);
    return myType;
}



/**
 * @brief   Get measured sensor value
 * @param   index of value to be read (default is 0)
 * @retval  last measured value
 */
eSensorError_t DSensor::getCoefficientsData(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Get measured sensor value
 * @param   index of value to be read (default is 0)
 * @retval  last measured value
 */
eSensorError_t DSensor::getCalibrationData(void)
{
    return E_SENSOR_ERROR_NONE;
}


/**
 * @brief   Validate sensor cal data
 * @param   void
 * @return  flag: true if cal data is valid, else false
 */
bool DSensor::validateCalData(sSensorData_t *sensorCalData)
{
    bool flag = false;
    sSensorStatus_t status;
    status.value = 0u;
 
    //last two bytes are always the CRC
    uint32_t crc = crc32((uint8_t *)&sensorCalData->data, sizeof(sSensorCal_t));

    if (crc == sensorCalData->crc)
    {
        flag = true;
        uint32_t days;

        if (daysSinceDate(&sensorCalData->data.date, &days) == false)
        {
            //date or RTC was not right
            status.calDateCheck = 1u;
        }
        else
        {
            //mark sensor cal as overdue if (one or more overdue range is sufficient to count as overdue)
            uint32_t interval = sensorCalData->data.calInterval;

            //check against valid range only (this allows cal interval = 0 setting to mean "don't use"
            if ((interval >= MIN_CAL_INTERVAL) && (interval <= MAX_CAL_INTERVAL) && (interval < days))
            {
                status.calOverdue = 1u;
            }
        }
        
    }
    else
    {
        status.calDataCrcFail = 1u;
    }

    if (status.value != 0u)
    {
        //bad cal data, so set sensor status as 'using default cal'
        status.calDefault = 1u;
    }
    else
    {
        //update interval in sensor class attributes
        //note: this is likely to be not used but implemented in case. we are not using cal interval info on a per-sensor
        //basis, however the cal date may be useful, eg, for service/diagnostics puposes
        setCalIntervalValue(sensorCalData->data.calInterval);

        //update the cal date too (can use either range, arbitrarily using the first range)
        myUserCalDate.day = sensorCalData->data.date.day;
        myUserCalDate.month = sensorCalData->data.date.month;
        myUserCalDate.year = sensorCalData->data.date.year;
    }

    //update status as a result of the checks above
    setStatus(status);

    return flag;
}
/**
 * @brief   Get floating point value
 * @param   index is function/sensor specific value identifier
 * @param   pointer to variable for return value
 * @return  true if successful, else false
 */
bool DSensor::getValue(eValueIndex_t index, float32_t *value)
{
    bool success = false;

    DLock is_on(&myMutex);
    success = true;

    switch (index)
    {
        case E_VAL_INDEX_VALUE:
            *value = myMeasuredValue;
           break;

        case E_VAL_INDEX_RAW_VALUE:
            *value = myMeasuredRawValue;
           break;

        case E_VAL_INDEX_POS_FS:
            *value = myFsMaximum;
            break;

        case E_VAL_INDEX_NEG_FS:
            *value = myFsMinimum;
            break;

        case E_VAL_INDEX_POS_FS_ABS:
            *value = myAbsFsMaximum;
            break;

        case E_VAL_INDEX_NEG_FS_ABS:
            *value = myAbsFsMinimum;
            break;

        case E_VAL_INDEX_RESOLUTION:
            *value = getResolution();
            break;

        default:
            success = false;
            break;
    }

    return success;
}

/**
 * @brief   Set floating point value
 * @param   index is function/sensor specific value identifier
 * @param   value to set
 * @return  true if successful, else false
 */
bool DSensor::setValue(eValueIndex_t index, float32_t value)
{
    bool success = false;

    DLock is_on(&myMutex);
    success = true;

    switch (index)
    {
        case E_VAL_INDEX_VALUE:
            myMeasuredValue = value;
           break;

        case E_VAL_INDEX_RAW_VALUE:
            myMeasuredRawValue = value;
           break;

        case E_VAL_INDEX_POS_FS:
            myFsMaximum = value;
            break;

        case E_VAL_INDEX_NEG_FS:
            myFsMinimum = value;
            break;

        case E_VAL_INDEX_POS_FS_ABS:
            myAbsFsMaximum = value;
            break;

        case E_VAL_INDEX_NEG_FS_ABS:
            myAbsFsMinimum = value;
            break;

        default:
            success = false;
            break;
    }

    return success;
}

/**
 * @brief   Get integer value
 * @param   index is sensor specific value identifier
 * @param   pointer to variable for return value of integer attribute
 * @return  true if successful, else false
 */
bool DSensor::getValue(eValueIndex_t index, uint32_t *value)
{
    bool successFlag = false;

    DLock is_on(&myMutex);
    successFlag = true;

    switch (index)
    {
        case E_VAL_INDEX_SERIAL_NUMBER:
            *value = mySerialNumber;
           break;
      
        case E_VAL_INDEX_SENSOR_TYPE:
          *value = (uint32_t) myType;
           break;
        case EVAL_INDEX_SENSOR_MANF_ID:
          *value = (uint32_t) myManfID;
           break;      
        default:
            successFlag = false;
            break;
    }

    return successFlag;
}

/**
 * @brief   Set calibration type
 * @param   calType - function specific calibration type (0 = user calibration)
 * @param   range - sensor range
 * @retval  true = success, false = failed
 */
bool DSensor::setCalibrationType(int32_t calType)
{
    bool flag = false;

    //can only calibrate sensors that have at least one cal point
    if (myNumCalPoints > 0u)
    {
        //TODO HSB: Should cal be allowed if RTC does not have a valid date? Should this be checked before starting cal (ie, CT command)

        //check range value is in bounds and cal type can only be 0 (ie, user cal)
        if (calType == 0) 
        {
            DLock is_on(&myMutex);

                if (myCalData != NULL)
                {
                    if (myCalData->hasCalData() == true)
                    {
                         //all set up, so can mark the sensor as 'in calibration' mode
                        setMode(E_SENSOR_MODE_CALIBRATION);

                        //stop applying existing cal data
                        myCalData->calInitialise();

                        //reset filtering because we don't want measurements from normal mode be including in cal mode ones

                        if (myFilter != NULL)
                        {
                            //TODO HSB: Should the filter be used at all in cal mode?
                            myFilter->reset();
                        }

                        myEnteredCalPoints = 0u;

                        //update allowed cal actions after cal type has been set
                        myStatus.canSetCalType = 0u;        //set calibration type is no longer allowed
                        myStatus.canStartSampling = 1u;     //start sampling is now allowed
                        myStatus.canAbortCal = 1u;          //calibration abort is allowed
                        myStatus.canSetCalDate = 1u;        //set calibration date is allowed as we may update just cal date and/or interval
                        myStatus.canSetCalInterval = 1u;    //set calibration interval is allowed as we may update just cal date and/or interval

                        flag = true;
                    }
                }           
        }
    }

    return flag;
}

/**
 * @brief   Get required number of calibration points
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSensor::getRequiredNumCalPoints(uint32_t *numCalPoints)
{
    DLock is_on(&myMutex);

    *numCalPoints = myNumCalPoints;

    return true;
}


/**
 * @brief   Start sampling at current cal point
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSensor::startCalSampling(void)
{
    bool flag = true;

    DLock is_on(&myMutex);

    myCalSampleCount = 0u;
    myCalSamplesAccumulator = 0.0f;

    //reset filtering because we don't want previous measurements to be including at new cal point

    if (myFilter != NULL)
    {
        //TODO HSB: Should the filter be used at all in cal mode?
        myFilter->reset();
    }

    //update allowed cal actions
    myStatus.canSetCalPoint = 0u;       //don't allow setting of cal point until sampling has completed
    myStatus.canQuerySampling = 1u;     //query cal samples remaining is allowed
    myStatus.canAcceptCal = 0u;         //don't allow accept cal while sampling
    myStatus.canSetCalDate = 0u;        //set calibration date is not allowed again until cal adjustment has been completed
    myStatus.canSetCalInterval = 0u;    //set calibration interval is not allowed again until cal adjustment has been completed

    setSamplingDoneStatus(0u);          //set status to indicate sampling not complete

    return flag;
}


/**
 * @brief   Set integer value
 * @param   index is sensor specific value identifier
 * @param   value to set for integer attribute
 * @return  true if successful, else false
 */
bool DSensor::setValue(eValueIndex_t index, uint32_t value)
{
    bool success = false;

    DLock is_on(&myMutex);
    success = true;

    switch (index)
    {
        case E_VAL_INDEX_SERIAL_NUMBER:
            mySerialNumber = value;
           break;

        default:
            success = false;
            break;
    }

    return success;
}

/**
 * @brief   Get Date value
 * @param   index is sensor specific value identifier
 * @param   pointer to variable for return value of date attribute
 * @return  true if successful, else false
 */
bool DSensor::getValue(eValueIndex_t index, sDate_t *date)
{
  bool successFlag = false;

    DLock is_on(&myMutex);
    successFlag = true;

    switch (index)
    {
        case E_VAL_INDEX_USER_CAL_DATE:
            *date = myUserCalDate;
           break;

        case E_VAL_INDEX_FACTORY_CAL_DATE:
            *date = myManufactureDate;
           break;
           
        case E_VAL_INDEX_MANUFACTURING_DATE:
            *date = myManufactureDate;
           break;
             
        default:
            successFlag = false;
            break;
    }

    return successFlag;
  
}

/**
 * @brief   Set cal sampling complete status
 * @param   samplingStatus: 1u = done; 0u = not done
 * @retval  void
 */
void DSensor::setSamplingDoneStatus(uint32_t samplingStatus)
{
    sSensorStatus_t status;
    status.value = 0u;
    status.calSamplingDone = samplingStatus;

    setStatus(status);
}

/**
 * @brief   Add measurement to cal sample accumulator
 * @param   sample value
 * @retval  void
 */
void DSensor::addCalSample(float32_t sample)
{
    DLock is_on(&myMutex);

    if (myCalSampleCount < myCalSamplesRequired)
    {
        myCalSampleCount++;
        myCalSamplesAccumulator += sample;

        //if all the cal samples we need have been accumluated then we set the status bit to indicate this
        if (myCalSampleCount == myCalSamplesRequired)
        {
            setSamplingDoneStatus(1u);      //set status to indicate sampling complete
        }
    }
}

/**
 * @brief   Get average value of cal samples in accumulator
 * @param   void
 * @retval  average sample value
 */
float32_t DSensor::getCalSampleAverage(void)
{
    float32_t sample = 0.0f;

    DLock is_on(&myMutex);

    if (myCalSampleCount > 0u)
    {
        sample = myCalSamplesAccumulator/((float32_t)myCalSampleCount);
    }

    return sample;
}

/**
 * @brief   Get remaining number of samples at current cal point
 * @param   pointer to variable for return value of remaining number of samples
 * @retval  true = success, false = failed
 */
bool DSensor::getCalSamplesRemaining(uint32_t *samples)
{
    DLock is_on(&myMutex);

    if (myCalSamplesRequired > 0u)
    {
        *samples = myCalSamplesRequired - myCalSampleCount;
    }
    else
    {
        *samples = 0u;
    }

    //if sampling has completed then allow setting of cal point
    if (*samples == 0u)
    {
        myStatus.canSetCalPoint = 1u;
        setSamplingDoneStatus(1u);      //set sampling complete, in case not already set by 'add sample' function
    }

    return true;
}


/**
 * @brief   Set calibration point
 * @param   point indicates the cal point number (1 - required no of points)
 * @param   user supplied calibration value
 * @retval  true = success, false = failed
 */
bool DSensor::setCalPoint(uint32_t calPoint, float32_t value)
{
    bool flag = false;

    DLock is_on(&myMutex);

    if (myCalData != NULL)
    {
        if (myCalData->hasCalData() == true)
        {
            float32_t sampleAverage = getCalSampleAverage();

            //only accept cal points if difference in applied and measured values is within 10% of fullscale
            float32_t calPointError = value - sampleAverage;

            if (fabsf(calPointError) < (0.1f * myFsMaximum))
            {
                //note: 'x' is what the instrument measures & 'y the applied value (ie, displayed value when measuring 'x')
                flag = myCalData->setCalPoint(calPoint, sampleAverage, value);

                if (flag == true)
                {
                    //TODO HSB: Use an array so don't get away with entering the same cal point multiple times!!
                    if (myEnteredCalPoints < myNumCalPoints)
                    {
                        myEnteredCalPoints++;
                    }

                    //if entered required number of cal points so can accept
                    //note that this does not take account of actual cal points - only the number of cal points
                    //so if a user may have entered the same cal point multiple times, in which case the cal
                    //would be rejected when an attempt is made to accept cal
                    if (myEnteredCalPoints >= myNumCalPoints)
                    {
                        myStatus.canAcceptCal = 1u;     //accept cal is now allowed
                    }

                    //update what is allowed after cal point
                    myStatus.canQuerySampling = 0u;     //query cal samples remaining is not allowed
                }
            }
        }
    }

    return flag;
}

/**
 * @brief   Cal accept
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSensor::acceptCalibration(void)
{
    bool flag = false;

    DLock is_on(&myMutex);

    //must supply required number of cal points to accept
    if (myEnteredCalPoints == myNumCalPoints)
    {
          if (myCalData != NULL)
          {
              if (myCalData->hasCalData() == true)
              {
                  //check that all required cal points have been entered
                  if (myCalData->getCalComplete(myNumCalPoints) == true)
                  {
                      //set cal date (RTC will have been checked already)
                      sDate_t date;
                      getSystemDate(&date);
                      myCalData->setDate(&date);

                      //TODO HSB: myCalData->data.calInterval = 0u;

                      flag = myCalData->setNumCalPoints(myNumCalPoints);

                      if (flag == true)
                      {
                          //calculate coefficients and apply if valid
                          flag = myCalData->validate(myNumCalPoints);

                          if (flag == true)
                          {
                              //save to persistent storage
                              flag = saveCalibrationData();
                          }
                      }
                  }
              }
          }
        endCalibration();
    }

    return flag;
}
/**
 * @brief   Abort calibration
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSensor::abortCalibration(void)
{
    endCalibration();
    return true;
}

/**
 * @brief   End calibration
 * @param   void
 * @retval  void
 */
void DSensor::endCalibration(void)
{
    DLock is_on(&myMutex);

    if (myCalData != NULL)
    {
        if (myCalData->hasCalData() == true)
        {
            //reload calibration from persistent storage
            loadCalibrationData();
        }
    }
    
    //all done
    setMode(E_SENSOR_MODE_NORMAL);

    //reset filtering because we don't want measurements from cal mode ones  be including in normal mode

    if (myFilter != NULL)
    {
        myFilter->reset();
    }

}
/**
 * @brief   Load calibration data from persistent storage
 * @param   void
 * @retval  true = success, false = failed
 */

bool DSensor::loadCalibrationData(void)
{
    bool flag = true; //if a sensor has no cal data then just return true

    //myCaldata is pointer to cal data for this sensor
    if (myCalData != NULL)
    {
         if (myCalData->hasCalData() == true)
        {
          flag = myCalData->loadCalibrationData();
    
          //sanity check the data - sets cal status for sensor as part of the check
          flag &= validateCalData(myCalData->getCalDataAddr());
        }
    }

    return flag;
}
/**
 * @brief   Save calibration to persistent storage
 * @param   void
 * @retval  true = success, false = failed
 */
bool DSensor::saveCalibrationData(void)
{
    bool flag = false;

    if (myCalData != NULL)
    {
        if (myCalData->hasCalData() == true)
        {
          flag = myCalData->saveCalibrationData();
        }
    }

    return flag;
}



 uint32_t DSensor::getManfIdentity(void)
 {
   return myManfID;
 }
 void DSensor::setManfIdentity(uint32_t manfIdentity)
 {
   myManfID = manfIdentity;
 }