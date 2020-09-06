/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DSensor.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     15 April 2020
*
* @brief    The sensor base class header file
*/

#ifndef __DSENSOR_H
#define __DSENSOR_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
MISRAC_ENABLE

#include "DRange.h"
#include "DDevice.h"
#include "Types.h"
#include "DLock.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define MAX_SENSOR_RANGES   2

#define MIN_CAL_INTERVAL        7u
#define MAX_CAL_INTERVAL        548u
#define DEFAULT_CAL_INTERVAL    365u

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_SENSOR_MODE_NORMAL,
    E_SENSOR_MODE_CALIBRATION,
    E_SENSOR_MODE_TEST,
    E_SENSOR_MODE_SYNCHRONISED

} eSensorMode_t;

typedef enum
{
    E_SENSOR_CAL_TYPE_USER = 0,
    E_SENSOR_CAL_TYPE_FACTORY

} eSensorCalType_t;

typedef union
{
    uint32_t value;

    struct
    {
        uint32_t fault              : 1;    //1 = sensor not working properly, eg comms timeout or command error
        uint32_t rangeChanged       : 1;    //1 = range has auto-changed
        uint32_t inLimit            : 1;    //1 = measured vaue is with in tolerance limit of the setpoint
        uint32_t zeroError          : 1;    //1 = zero attempt was unsuccessful

        uint32_t calRejected        : 1;    //1 = calibration rejected
        uint32_t calDefault         : 1;    //1 = either cal was found to be invalid or sensor has not been calibrated
        uint32_t calOverdue         : 1;    //1 = calibration is overdue
        uint32_t calDateCheck       : 1;    //1 = calibration date was invalid or RTC not set

        uint32_t reserved           : 24;   //available for more bits as needed
    };

} sSensorStatus_t;

typedef enum : uint32_t
{
    E_SENSOR_ERROR_NONE         = 0x00000000u,		//no error
    E_SENSOR_ERROR_TIMEOUT      = 0x00000001u,		//function timed out
    E_SENSOR_ERROR_UNAVAILABLE  = 0x00000002u,		//function not implemented/available
    E_SENSOR_ERROR_FAULT        = 0x00000004u,		//function failed (unspecified cause)
    E_SENSOR_ERROR_COMMAND      = 0x00000008u,		//error in execution of a command
    E_SENSOR_ERROR_COMMS        = 0x00000010u,		//error in sensor comms tx/rx
    E_SENSOR_ERROR_HAL          = 0x00000020u,		//error returned by lower-level HAL function
    E_SENSOR_ERROR_MEASUREMENT  = 0x00000040u,		//error sensor failed to get measurement
    E_SENSOR_ERROR_PARAMETER    = 0x00000080u,		//paramter value error
    E_SENSOR_ERROR_OS           = 0x00000100u,		//OS function returned error
    E_SENSOR_ERROR_NCK          = 0x00000200u,		//Sensor Returns NCK
    E_SENSOR_SUPPLY_VOLAGE_LOW  = 0x00000400u           //sensor low supply voltage
} eSensorError_t;

/*sensor version*/
typedef union
{
    uint32_t value;

    struct
    {
        uint32_t build	: 8;	/*build number*/
        uint32_t minor	: 8;	/*minor version*/
        uint32_t major	: 4;	/*major version*/
        uint32_t dk     : 12;	/*dk number*/
    };

} uSensorIdentity_t;

/* Variables -------------------------------------------------------------------------------------------------------*/

class DSensor
{
private:
    float32_t myMeasuredValue;              //measured value
    float32_t myMeasuredRawValue;           //measured value before calibration applied

protected:
    //attributes ****************************************************************************************************
    OS_MUTEX myMutex;                       //resource sharing lock

    DDevice *myDevice;                      //pointer to device for this sensor (interface to hardware)
//    DFilter* myFilter;                      //measurement filter

    uSensorIdentity_t myIdentity;
    uint32_t mySerialNumber;                //sensor serial number
    eSensorType_t myType;                   //sensor type
    eSensorMode_t myMode;                   //sensor mode (eg, calibration mode)

    float32_t myFsMaximum;                  //positive fullscale
    float32_t myFsMinimum;                  //negative fullscale

    float32_t myAbsFsMaximum;               //absolute maximum value to be applied to this sensor
    float32_t myAbsFsMinimum;               //absolute minimum value to be applied to this sensor

    uint32_t myMaxCalPoints;                //calibration requires at most this many cal points
    uint32_t myMinCalPoints;                //calibration requires at least this many cal points

    DRange *myRanges[MAX_SENSOR_RANGES];    //supports up to two ranges per sensor
    uint32_t myNumRanges;                   //number of ranges
    uint32_t myRange;                       //current range
    bool myAutoRangingEnabled;              //autoranging permitted or not


    uint32_t myCalSampleCount;              //sample counter value (during calibration)
    float32_t myCalSamplesAccumulator;      //accumluation of sample values during calibration (used for averaging)
    uint32_t myCalSamplesRequired;          //number of cal samples required at each cal point (during calibration)

    uint32_t myCalInterval;                 //cal interval in days

    sDate_t myUserCalDate;                  //last user cal date
    sDate_t myManufactureDate;              //sensor manufacture date

    sSensorStatus_t myStatus;               //current sensor status value
    sSensorStatus_t myLastStatus;           //last value of current sensor status (used for change notification)
    sSensorStatus_t myStatusChanges;        //changes in sensor status since last read (cleared on every read)

    //functions ******************************************************************************************************
    virtual void createRanges(void);

    void calEnd();

    float32_t compensate(float32_t rawReading);

    bool validateCalData(sSensorData_t *sensorCalData);

    bool isCalibratable();
    void resetCalSampleCount();

    virtual bool performAutoRanging(float measurement);

public:
    DSensor();

    virtual eSensorError_t initialise(void);
    virtual eSensorError_t close();

    //data access function
    float32_t getResolution(void);

    sSensorStatus_t getStatus(void);
    void setStatus(sSensorStatus_t status);
    sSensorStatus_t getStatusChanges(void);

    bool getFilterEnabled();
    void setFilterEnabled(bool state);

    uint32_t getNumRanges();

    virtual eSensorError_t measure(void);  //perform measurement
    virtual eSensorError_t measure(uint32_t channelSelection);  //perform measurement

    virtual eSensorError_t setMode(eSensorMode_t mode);
    virtual eSensorError_t getMode(eSensorMode_t *mode);

    virtual uint32_t getRequiredNumCalPoints(void);
    virtual uint32_t getRequiredCalSamples(void);

    virtual uint32_t getSampleCount(void);
    void setSampleCount(uint32_t value);

    virtual eSensorError_t setCalPoint(float32_t value);
    virtual eSensorError_t calStartSampling(void);
    virtual eSensorError_t calAbort(void);
    virtual eSensorError_t calAccept(void);

    virtual void setCalDate(sDate_t date);
    virtual void getCalDate(eSensorCalType_t caltype, sDate_t* date);

    virtual void setRange(uint32_t range);
    virtual uint32_t getRange(void);

    virtual void setCalInterval(uint32_t interval);
    virtual uint32_t getCalInterval(void);

    virtual void setSerialNumber(uint32_t serialNumber);
    virtual uint32_t getSerialNumber(void);

    virtual void setIdentity(uSensorIdentity_t identity);
    virtual uSensorIdentity_t getIdentity(void);
    
    virtual void setIdentity(uint32_t identity);
    virtual void getIdentity(uint32_t* identity);
    
    virtual void setUserCalDate(sDate_t *date);
    virtual sDate_t *getUserCalDate(void);

    virtual void setManufactureDate(sDate_t *date);
    virtual void getManufactureDate(sDate_t  *date);

    virtual void setFullScaleMax(float32_t fullscale);
    virtual float32_t getFullScaleMax(void);

    virtual void setFullScaleMin(float32_t fullscale);
    virtual float32_t getFullScaleMin(void);

    virtual void setAbsFullScaleMax(float32_t fullscale);
    virtual float32_t getAbsFullScaleMax(void);

    virtual void setAbsFullScaleMin(float32_t fullscale);
    virtual float32_t getAbsFullScaleMin(void);

    virtual void setSensorType(eSensorType_t sensorType);
    virtual eSensorType_t getSensorType(void);

    virtual bool getValue(eValueIndex_t index, float32_t *value);    //get specified floating point function value
    virtual bool setValue(eValueIndex_t index, float32_t value);     //set specified floating point function value

    virtual bool getValue(eValueIndex_t index, uint32_t *value);    //get specified integer function value
    virtual bool setValue(eValueIndex_t index, uint32_t value);     //set specified integer function value

    virtual bool getValue(eValueIndex_t index, sDate_t *date);
    virtual float32_t getMeasurement(uint32_t index = 0u);  //get measured value variable value
    virtual void setMeasurement(float32_t value);           //set measured value variable value

    virtual float32_t getOutput(void);          //get setpoint
    virtual void setOutput(float32_t setpt);    //set setpoint
    
    virtual eSensorError_t getCoefficientsData(void);
    virtual eSensorError_t getCalibrationData(void);
};

#endif /* __DSENSOR_H */
