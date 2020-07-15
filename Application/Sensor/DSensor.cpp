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

    myDevice = NULL;

    //clear all ranges
    for (int32_t i = 0; i < MAX_SENSOR_RANGES; i++)
    {
        myRanges[i] = NULL;
    }

    //    myFilter = NULL;
    //	myCalDate = NULL;;

	myAutoRangingEnabled = true;
	myRange = 0u;

    myCalInterval = DEFAULT_CAL_INTERVAL;

	myMinCalPoints = 2u;              //default is 2-point cal
	myMaxCalPoints = myMinCalPoints;  //default is same as min calpoints
	myCalSamplesRequired = 1u;        //number of cal samples at each cal point for averaging

	myNumRanges = 1u;                 //default is single range

	myStatus.value = 0u;              //clear all status bits
    myLastStatus.value = 0u;          //last value of current sensor status (used for change notification)
    myStatusChanges.value = 0u;       //changes in sensor status since last read (cleared on every read)

	myMode = E_SENSOR_MODE_NORMAL;

    myType = E_SENSOR_TYPE_GENERIC;

    //create mutex for resource locking
    char *name = "Sen";
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &os_error);

    if (os_error != (OS_ERR)OS_ERR_NONE)
    {
        //Error handler?
    }
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
 * @brief   End calibration procedure
 * @param   void
 * @retval  void
 */
void DSensor::calEnd()
{
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
 * @brief   Reset calibration sample count for current cal point
 * @param   void
 * @retval  void
 */
void DSensor::resetCalSampleCount()
{
    DLock is_on(&myMutex);
	myCalSampleCount = 0u;
	myCalSamplesAccumulator = 0.0f;
}

/**
 * @brief   Get resolution for current sensor range
 * @param   void
 * @retval  resolution of measured value
 */
float32_t DSensor::getResolution()
{
    DLock is_on(&myMutex);
    return myRanges[myRange]->getResolution();
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
 * @brief   Set sensor status
 * @param   sensor status (meaning is sensor specific)
 * @retval  void
 */
void DSensor::setStatus(sSensorStatus_t status)
{
    DLock is_on(&myMutex);

    //update current sensor status value
    myStatus = status;

    //compare with last value of sensor status and check for changes and add any new changes to other unread ones
    myStatusChanges.value |= (myLastStatus.value ^ status.value);

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
//    if (myFilter != NULL)
//    {
//        //myFilter->setEnabled(state);
//    }
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
//    if (myFilter != NULL)
//    {
//        //enabled = myFilter->getEnabled();
//    }

    return enabled;
}

/**
 * @brief   Get number of ranges
 * @param   void
 * @retval  number of ranges
 */
uint32_t DSensor::getNumRanges()
{
    DLock is_on(&myMutex);
    return myNumRanges;
}

/**
 * @brief   Set current sensor range
 * @param   range index (0 to (maxRanges -1))
 * @retval  void
 */
void DSensor::setRange(uint32_t range)
{
    //make sure it is a valid range index
    if ((range < myNumRanges) == true)
    {
        DLock is_on(&myMutex);
        myRange = range;
    }
}

/**
 * @brief   Get current sensor range
 * @param   void
 * @retval  sensor error code
 */
uint32_t DSensor::getRange(void)
{
    DLock is_on(&myMutex);
    return myRange;
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
eSensorError_t DSensor::setMode(eSensorMode_t mode)
{
    DLock is_on(&myMutex);
    myMode = mode;
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Get sensor mode
 * @param   pointer to variable for return value
 * @retval  sensor error code
 */
eSensorError_t DSensor::getMode(eSensorMode_t *mode)
{
    DLock is_on(&myMutex);
    *mode = myMode;
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Get number of cal point required
 * @param   void
 * @retval  number of cal points
 */
uint32_t DSensor::getRequiredNumCalPoints(void)
{
    DLock is_on(&myMutex);
    return myMinCalPoints;
}

/**
 * @brief   Set calibration point value
 * @param   applied value
 * @retval  sensor error code
 */
eSensorError_t DSensor::setCalPoint(float32_t value)
{
    DLock is_on(&myMutex);
    return E_SENSOR_ERROR_UNAVAILABLE;
}

/**
 * @brief   Start calibration point sampling
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensor::calStartSampling(void)
{
    return E_SENSOR_ERROR_UNAVAILABLE;
}

/**
 * @brief   Abort calibration procedure (revert to existing cal)
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensor::calAbort(void)
{
    DLock is_on(&myMutex);
    return E_SENSOR_ERROR_UNAVAILABLE;
}

/**
 * @brief   Accept calibration (apply new calibration)
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensor::calAccept(void)
{
    DLock is_on(&myMutex);
    return E_SENSOR_ERROR_UNAVAILABLE;
}

/**
 * @brief   Set calibration date
 * @param   date of calibration
 * @retval  void
 */
void DSensor::setCalDate(sDate_t date)
{
    DLock is_on(&myMutex);
}

/**
 * @brief   get calibration date
 * @param   type of calibration (user or factory)
 * @param   pointer to variable for return value
 * @retval  void
 */
void DSensor::getCalDate(eSensorCalType_t caltype, sDate_t* date)
{
    DLock is_on(&myMutex);
}

/**
 * @brief   Set calibration interval (in number of days)
 * @param   interval value
 * @retval  void
 */
void DSensor::setCalInterval(uint32_t interval)
{
    if ((interval >= MIN_CAL_INTERVAL) && (interval <= MAX_CAL_INTERVAL))
    {
        DLock is_on(&myMutex);
        myCalInterval = interval;
    }
}

/**
 * @brief   Get calibration interval (in number of days)
 * @param   pointer to variable for return value
 * @retval  sensor error code
 */
uint32_t DSensor::getCalInterval(void)
{
    DLock is_on(&myMutex);
    return myCalInterval;
}

/**
 * @brief   Set sensor serial number
 * @param   serial number
 * @retval  void
 */
void DSensor::setSerialNumber(uint32_t serialNumber)
{
    DLock is_on(&myMutex);
    mySerialNumber = serialNumber;
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
void DSensor::setUserCalDate(sDate_t *date)
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
sDate_t *DSensor::getManufactureDate(void)
{
    DLock is_on(&myMutex);
    return &myManufactureDate;
}

/**
 * @brief   Set measured sensor value
 * @param   value to set
 * @retval  void
 */
void DSensor::setMeasurement(float32_t value)
{
    DLock is_on(&myMutex);
    myMeasuredValue = value;
}

/**
 * @brief   Set absolute fullscale minimum value
 * @param   value to set
 * @retval  void
 */
void DSensor::setFullScaleMax(float32_t fullscale)
{
    DLock is_on(&myMutex);
    myFsMaximum = fullscale;
}

/**
 * @brief   Get absolute fullscale minimum value
 * @param   void
 * @retval  value
 */
float32_t DSensor::getFullScaleMax(void)
{
    DLock is_on(&myMutex);
    return myFsMaximum;
}

/**
 * @brief   Set absolute fullscale minimum value
 * @param   value to set
 * @retval  void
 */
void DSensor::setFullScaleMin(float32_t fullscale)
{
    DLock is_on(&myMutex);
    myFsMinimum = fullscale;
}

/**
 * @brief   Get absolute fullscale minimum value
 * @param   void
 * @retval  value
 */
float32_t DSensor::getFullScaleMin(void)
{
    DLock is_on(&myMutex);
    return myFsMinimum;
}

/**
 * @brief   Set absolute fullscale minimum value
 * @param   value to set
 * @retval  void
 */
void DSensor::setAbsFullScaleMax(float32_t fullscale)
{
    DLock is_on(&myMutex);
    myAbsFsMaximum = fullscale;
}

/**
 * @brief   Get absolute fullscale minimum value
 * @param   void
 * @retval  value
 */
float32_t DSensor::getAbsFullScaleMax(void)
{
    DLock is_on(&myMutex);
    return myAbsFsMaximum;
}

/**
 * @brief   Set absolute fullscale minimum value
 * @param   value to set
 * @retval  void
 */
void DSensor::setAbsFullScaleMin(float32_t fullscale)
{
    DLock is_on(&myMutex);
    myAbsFsMinimum = fullscale;
}

/**
 * @brief   Get absolute fullscale minimum value
 * @param   void
 * @retval  value
 */
float32_t DSensor::getAbsFullScaleMin(void)
{
    DLock is_on(&myMutex);
    return myAbsFsMinimum;
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
float32_t DSensor::getMeasurement(uint32_t index)
{
    DLock is_on(&myMutex);
    return myMeasuredValue;
}

/**
 * @brief   Check that the range is appropriate for the current measurement value
 * @param   compensated measurement
 * @retval  true if range changes as a result of this call, else false
 */
bool DSensor::performAutoRanging(float measurement)
{
    bool rangeChanged = false;

    //for autoranging use magnitude of value against threshold (assumption is that the ranges are symmetrical around 0)
    float32_t absValue = fabs(measurement);

    //must have more than 1 range and autoranging must be active for auto-ranging
    if ((myNumRanges > 1u) && (myAutoRangingEnabled == true))
    {
        //check value again current range thresholds to see if can step up or down in range
        if (absValue < myRanges[myRange]->getMinAutoRange())
        {
            //only switch if there is a lower range
            if (myRange > 0u)
            {
                //go down to lower range
                setRange(myRange - 1u);
                rangeChanged = true;
            }
        }
        else if (absValue > myRanges[myRange]->getMaxAutoRange())
        {
            //only switch if there is a higher range
            if (myRange < (myNumRanges - 1u))
            {
                //go up to bigger range
                setRange(myRange + 1u);
                rangeChanged = true;
            }
        }
        else
        {
            //all if, else if constructs should contain a final else clause (MISRA C 2004 rule 14.10)
        }
    }

    return rangeChanged;
}


/**
 * @brief   Get setpoint
 * @param   void
 * @retval  setpoint
 */
float32_t DSensor::getOutput(void)
{
    return 0.0f;
}

/**
 * @brief   Update setpoint
 * @param   void
 * @retval  void
 */
void DSensor::setOutput(float32_t setpt)
{
}

//TODO: These functions are to exchange infor with the UI, the ones above are for reading and writing from actual physical sensor!!
///**
// * @brief   Get measured sensor value
// * @param   void
// * @retval  last measured value
// */
//float32_t DSensor::getMeasurement()
//{
//    DLock is_on(&myMutex);
//    return myMeasuredValue;
//}
//
///**
// * @brief   Get cal interval
// * @param   void
// * @retval  cal interval
// */
//uint32_t DSensor::getCalInterval(void)
//{
//    DLock is_on(&myMutex);
//    return myCalInterval;
//}
//
///**
// * @brief   Set cal interval
// * @param   cal interval to set
// * @retval  void
// */
//void DSensor::setCalInterval(uint32_t interval)
//{
//}

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
    sCalRange_t *calRange = &sensorCalData->data.cal[0];

    //last two bytes are always the CRC
    uint32_t crc = crc32((uint8_t *)&sensorCalData->data, sizeof(sSensorCal_t));

    if (crc == sensorCalData->crc)
    {
       flag = true;
       uint32_t days;

       //set interval in sensor class attributes
       setCalInterval(sensorCalData->data.calInterval);

       //copy over the date too
       myUserCalDate.day = calRange->date.day;
       myUserCalDate.month = calRange->date.month;
       myUserCalDate.year = calRange->date.year;

       //check either range date for overdue cal
       for (uint32_t i = 0u; (i < myNumRanges) && (status.value == 0u); i++)
       {
           calRange = &sensorCalData->data.cal[i];

           if (daysSinceDate(&calRange->date, &days) == false)
           {
               //date or RTC was not right
               status.calDateCheck = 1u;
           }
           else
           {
               //mark sensor cal as overdue (one or more overdue range is sufficient to count as overdue)
               if (getCalInterval() < days)
               {
                   status.calOverdue = 1u;
               }
           }
       }
    }
    else
    {
       //bad cal data, so set sensor status as 'using default cal'
       status.calDefault = 1u;

       //clear all date to 0
       for (int32_t i = 0; i < MAX_CAL_RANGES; i++)
       {
           calRange = &sensorCalData->data.cal[i];

           //clearing to 0 ensures no attempt is made to use bad/invalid cal
           memset((void *)calRange, 0x00, sizeof(sCalRange_t));
       }
    }

    //update status as a result of the checks above
    setStatus(status);

    return flag;
}
