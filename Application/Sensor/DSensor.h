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
#include "DFilterAdaptive.h"
#include "DCalibration.h"
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
    E_SENSOR_MODE_SYNCHRONISED,
    E_SENSOR_MODE_FW_UPGRADE

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
        //lower 16 bits are used for general sensor status
        uint32_t fault              : 1;    //1 = sensor not working properly, e.g. comms timeout or command error
        uint32_t rangeChanged       : 1;    //1 = range has auto-changed
        uint32_t inLimit            : 1;    //1 = measured value is with in tolerance limit of the setpoint
        uint32_t zeroError          : 1;    //1 = zero attempt was unsuccessful

        uint32_t unused             : 1;    //    available for use
        uint32_t calRejected        : 1;    //1 = calibration rejected
        uint32_t calDefault         : 1;    //1 = either cal was found to be invalid or sensor has not been calibrated
        uint32_t calOverdue         : 1;    //1 = calibration is overdue

        uint32_t calDateCheck       : 1;    //1 = calibration date was invalid or RTC not set
        uint32_t calDataCrcFail     : 1;    //1 = cal data CRC check failed
        uint32_t calDateWriteFail   : 1;    //1 = cal date write failed
        uint32_t calSamplingDone    : 1;    //1 = cal sampling complete

        uint32_t reserved1          : 4;    //available for more bits as needed

        //higher 16 bits are used for calibration status
        uint32_t canSetCalType      : 1;    //set calibration type is allowed
        uint32_t canStartSampling   : 1;    //start sampling is allowed
        uint32_t canQuerySampling   : 1;    //query cal samples remaining is allowed
        uint32_t canSetCalPoint     : 1;    //set calibration point is allowed

        uint32_t canAcceptCal       : 1;    //calibration accept is allowed
        uint32_t canAbortCal        : 1;    //calibration abort is allowed
        uint32_t canSetCalDate      : 1;    //set calibration date is allowed
        uint32_t canSetCalInterval  : 1;    //set calibration interval is allowed

        uint32_t reserved2          : 8;    //available for more bits as needed
    };

} sSensorStatus_t;

#define SENSOR_STATUS_BITS_MASK     0x0000FFFFu //value used to mask off the cal status bits
typedef enum : uint32_t
{
    E_SENSOR_ERROR_NONE         = 0x00000000u,      //no error
    E_SENSOR_ERROR_TIMEOUT      = 0x00000001u,      //function timed out
    E_SENSOR_ERROR_UNAVAILABLE  = 0x00000002u,      //function not implemented/available
    E_SENSOR_ERROR_FAULT        = 0x00000004u,      //function failed (unspecified cause)
    E_SENSOR_ERROR_COMMAND      = 0x00000008u,      //error in execution of a command
    E_SENSOR_ERROR_COMMS        = 0x00000010u,      //error in sensor comms tx/rx
    E_SENSOR_ERROR_HAL          = 0x00000020u,      //error returned by lower-level HAL function
    E_SENSOR_ERROR_MEASUREMENT  = 0x00000040u,      //error sensor failed to get measurement
    E_SENSOR_ERROR_PARAMETER    = 0x00000080u,      //paramter value error
    E_SENSOR_ERROR_OS           = 0x00000100u,      //OS function returned error
    E_SENSOR_ERROR_NCK          = 0x00000200u,      //Sensor Returns NCK
    E_SENSOR_SUPPLY_VOLAGE_LOW  = 0x00000400u,           //sensor low supply voltage
    E_SENSOR_ERROR_CAL_COMMAND  = 0x00000800u,           //caibration command failed
    E_SENSOR_UPDATE_NACK_ERROR  = 0x00001000u           //sensor fails to accept firmware
} eSensorError_t;

typedef enum : uint8_t
{
    E_CHECKSUM_DISABLED       = 0x0u,       //Checksum Disabled
    E_CHECKSUM_ENABLED         = 0x1u,      //Checksum Enabled
} eCheckSumStatus_t;

/*sensor version*/
typedef union
{
    uint32_t value;

    struct
    {
        uint32_t build  : 8;    /*build number*/
        uint32_t minor  : 8;    /*minor version*/
        uint32_t major  : 4;    /*major version*/
        uint32_t dk     : 12;   /*dk number*/
    };

} uSensorIdentity_t;

/* Variables -------------------------------------------------------------------------------------------------------*/

class DSensor
{


protected:
    //attributes ****************************************************************************************************
    OS_MUTEX myMutex;                       //resource sharing lock

    DDevice *myDevice;                      //pointer to device for this sensor (interface to hardware)

    DFilter *myFilter;                      //measurement filter

    uSensorIdentity_t myIdentity;           //Application Identity
    uSensorIdentity_t myBlIdentity;          // Bootloader Identity

    uint32_t myLatency;                     //this is the time in milliseconds that it takes to take a measurement
    float32_t myMeasuredAvgValue;           //measured value averaged for terps sensor
    float32_t myMeasuredValue;              //measured value after calibration applied
    float32_t myMeasuredRawValue;           //measured value before calibration applied
    float32_t myCapturedValue1;             //interpolated value at a given instant between two successive measurements
    float32_t myCapturedValue2;             //interpolated value at a given instant between two successive measurements

    uint32_t mySerialNumber;                //sensor serial number
    int8_t myBrandUnits[10];                // unit information
    int8_t myBrandMin[8];                   // min value supported by sensor
    int8_t myBrandMax[8];                   // max value supported by sensor
    int8_t myBrandType[8];                  //sensor type
    uint32_t mySampleRate;                  //speed of sampling the sensor
    eSensorType_t myType;                   //sensor type
    eSensorMode_t myMode;                   //sensor mode (eg, calibration mode)

    float32_t myFsMaximum;                  //positive fullscale
    float32_t myFsMinimum;                  //negative fullscale

    float32_t myAbsFsMaximum;               //absolute maximum value to be applied to this sensor
    float32_t myAbsFsMinimum;               //absolute minimum value to be applied to this sensor

    uint32_t myMaxCalPoints;                //calibration requires at most this many cal points
    uint32_t myMinCalPoints;                //calibration requires at least this many cal points

    DCalibration *myCalData;                //pointer to calibration data

    uint32_t myNumCalPoints;                //required number of calibration points
    uint32_t myEnteredCalPoints;            //entered number of calibration points

    uint32_t myCalSampleCount;              //sample counter value (during calibration)
    float32_t myCalSamplesAccumulator;      //accumluation of sample values during calibration (used for averaging)
    uint32_t myCalSamplesRequired;          //number of cal samples required at each cal point (during calibration)

    uint32_t myCalInterval;                 //cal interval in days
    float32_t myCalPointValue;

    sDate_t myUserCalDate;                  //last user cal date
    sDate_t myManufactureDate;              //sensor manufacture date

    sSensorStatus_t myStatus;               //current sensor status value
    sSensorStatus_t myLastStatus;           //last value of current sensor status (used for change notification)
    sSensorStatus_t myStatusChanges;        //changes in sensor status since last read (cleared on every read)

    uint32_t myManfID;                     //Manufacturer ID
    float32_t myResolution;                //resolution of the sensor

    int32_t pressRaw;
    int32_t tempRaw;
    int32_t tempRawFiltered;

    //functions ******************************************************************************************************

    virtual bool validateCalData(sSensorData_t *sensorCalData);
    virtual float32_t compensate(float32_t rawReading);
    virtual float32_t getResolution(void);

    void setSamplingDoneStatus(uint32_t samplingStatus);
    void addCalSample(float32_t sample);
    virtual float32_t getCalSampleAverage(void);

    bool isCalibratable();
    virtual void endCalibration(void);

public:
    DSensor();
    virtual ~DSensor();
    virtual eSensorError_t initialise(void);
    virtual eSensorError_t close();
    virtual eSensorError_t measure(void);  //perform measurement
    virtual eSensorError_t measure(uint32_t channelSelection);  //perform measurement


    //data access functions
    void resetStatus(void);
    void resetStatus(sSensorStatus_t status);
    void clearStatus(sSensorStatus_t status);
    sSensorStatus_t getStatus(void);
    sSensorStatus_t getStatusChanges(void);
    void setStatus(sSensorStatus_t status);

    virtual void setIdentity(uSensorIdentity_t identity);
    virtual uSensorIdentity_t getIdentity(void);

    bool getFilterEnabled();
    void setFilterEnabled(bool state);

    virtual eSensorMode_t getMode(void);
    virtual void setMode(eSensorMode_t mode);

    virtual eSensorType_t getSensorType(void);
    virtual void setSensorType(eSensorType_t sensorType);

    virtual bool getValue(eValueIndex_t index, float32_t *value);    //get specified floating point function value
    virtual bool setValue(eValueIndex_t index, float32_t value);     //set specified floating point function value

    virtual bool getValue(eValueIndex_t index, uint32_t *value);    //get specified integer function value
    virtual bool setValue(eValueIndex_t index, uint32_t value);     //set specified integer function value

    virtual bool getValue(eValueIndex_t index, sDate_t *date);

    virtual bool getValue(eValueIndex_t index, int32_t *value);
    virtual bool setValue(eValueIndex_t index, int32_t value);

    virtual uint32_t getRequiredCalSamples(void);

    virtual uint32_t getSampleCount(void);
    void setSampleCount(uint32_t value);

    virtual bool getCalInterval(uint32_t *interval);
    virtual bool setCalInterval(uint32_t interval);
    virtual bool setCalIntervalValue(uint32_t interval);

    virtual bool getCalDate(sDate_t *date);
    virtual bool setCalDate(sDate_t *date);

    virtual void setCalDateValue(sDate_t *date);
    virtual sDate_t *getUserCalDate(void);

    virtual void getManufactureDate(sDate_t  *date);
    virtual void setManufactureDate(sDate_t *date);

    virtual bool setCalibrationType(int32_t calType);
    virtual bool getRequiredNumCalPoints(uint32_t *numCalPoints);
    virtual bool setRequiredNumCalPoints(uint32_t numCalPoints);
    virtual bool startCalSampling(void);
    virtual bool getCalSamplesRemaining(uint32_t *samples);
    virtual bool setCalPoint(uint32_t calPoint, float32_t value);
    virtual bool acceptCalibration(void);
    virtual bool abortCalibration(void);
    virtual bool saveCalibrationData(void);
    virtual bool loadCalibrationData(void);


    virtual eSensorError_t setCheckSum(eCheckSumStatus_t checksumStatus);
    virtual uint32_t getSerialNumber(void);

    virtual eSensorError_t readCoefficientsData(void);
    virtual eSensorError_t readCalibrationData(void);
    virtual eSensorError_t readZeroData(void);

    virtual uint32_t getManfIdentity(void);
    virtual void setManfIdentity(uint32_t manfIdentity);
    virtual bool getBrandUnits(int8_t *brandUnits, uint32_t bufLen);
    virtual bool getBrandMin(int8_t *brandMin, uint32_t bufLen);
    virtual bool getBrandMax(int8_t *brandMax, uint32_t bufLen);
    virtual bool getBrandType(int8_t *brandType, uint32_t bufLen);
    virtual eSensorError_t upgradeFirmware(void);
    bool isZeroable(void);
    bool getCalPoint(uint32_t range,
                     uint32_t calPoint,
                     float32_t *measured,
                     float32_t *applied);
    bool validateCalPointValue(float32_t value, float32_t *sampleAverage);
    virtual eSensorError_t setZeroData(float32_t zeroVal);
    virtual eSensorError_t getZeroData(float32_t *zeroVal);
    virtual void initializeSensorInfo(void);
    virtual eSensorError_t restoreChecksumStatus(eCheckSumStatus_t checksumStatus);
    virtual eSensorError_t hardControlChecksum(eCheckSumStatus_t checksumStatus);
};

#endif /* __DSENSOR_H */
