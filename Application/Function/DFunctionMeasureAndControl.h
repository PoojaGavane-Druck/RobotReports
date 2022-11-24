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
* @file     DFunctionMeasureAddExtBaro.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     02 June 2020
*
* @brief    The DFunctionMeasureAddExtBaro base class header file
*/

#ifndef _DFUNCTION_MEASURE_AND_CONTROL_H
#define _DFUNCTION_MEASURE_AND_CONTROL_H

/* Includes ---------------------------------------------------------------------------------------------------------*/


#include "DFunctionMeasure.h"
#include "DErrorHandler.h"
#include "Controller.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define VENTED 0x00000020u
#define PISTON_CENTERED 0x00001000u
#define MEASURE 0x00000400u

// Task timeout values for sensor tasks
#define TIMEOUT_COARSE_CONTROL_PM620 12u
#define TIMEOUT_COARSE_CONTROL_PM620T 22u

#define TIMEOUT_NON_COARSE_CONTROL 80u

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_STATE_SHUTDOWN = 0,
    E_STATE_RUNNING,
    E_STATE_SHUTTING_DOWN
} eFunctionStates_t;
/* Variables --------------------------------------------------------------------------------------------------------*/

class DFunctionMeasureAndControl : public DFunctionMeasure
{
    DSlot *myBarometerSlot;               //the slot (thread) that runs the sensor for this function
    float32_t myBarometerReading;         //Processed Barometer measureementValue
    float32_t myAbsoluteReading;
    float32_t myGaugeReading;

    float32_t myAbsFilteredReading;
    float32_t myGaugeFilteredReading;

    controllerStatus_t myStatus;
    controllerStatus_t myStatusPm;
    eControllerMode_t myMode;   // It tells about current mode
    eControllerMode_t myNewMode; // It tells about new mode which we received from DPI620G
    float32_t myCurrentPressureSetPoint; // It tells about current set point value
    eAquisationMode_t myAcqMode;

    uint32_t isMotorCentered;
    uint32_t isSensorConnected;
    uint32_t startCentering;
    uint32_t ventComplete;
    uint32_t wasVented;
    uint32_t ventIterations;

    int32_t rawTempValue;
    int32_t rawPressureValue;
    int32_t filteredTemperatureValue;

    deviceStatus_t controllerErrorMask;

    eFunctionStates_t myState;
    float32_t myVentRate;  // Required vent rate during switch testing
    float32_t myFilterCoeff;
    float32_t oldFilterValue;

    DController *pressureController;
    bool newSetPointReceivedFlag;
    bool getPressureInfo(pressureInfo_t *info);
    bool setPmSampleRate(void);
protected:

    virtual void createSlots(void);
    virtual void getSettingsData(void);
    virtual void runProcessing(void);
    virtual void handleEvents(OS_FLAGS actualEvents);

public:
    DFunctionMeasureAndControl();
    ~DFunctionMeasureAndControl();
    virtual void start(void);               //initialisation of function before the 'while' loop runs
    virtual void runFunction(void);             //the 'while' loop
    virtual bool setValue(eValueIndex_t index, uint32_t value);     //set specified integer function value
    virtual bool getValue(eValueIndex_t index, uint32_t *value);    //get specified integer function value
    virtual bool getValue(eValueIndex_t index, float32_t *value);  //read function measured value
    virtual bool setValue(eValueIndex_t index, float32_t value);
    virtual bool setValue(eValueIndex_t index, int32_t value);
    virtual bool getValue(eValueIndex_t index, int32_t *value);

    virtual void takeNewReading(uint32_t rate);

    virtual bool setCalibrationType(int32_t calType, uint32_t range);
    virtual bool getRequiredNumCalPoints(eSensor_t sensorType, uint32_t *numCalPoints);
    virtual bool setRequiredNumCalPoints(uint32_t numCalPoints);
    virtual bool startCalSampling(void);
    virtual bool getCalSamplesRemaining(uint32_t *samples);
    virtual bool setCalPoint(uint32_t calPoint, float32_t value);
    virtual bool acceptCalibration(void);
    virtual bool abortCalibration(void);
    virtual bool reloadCalibration(void);

    virtual bool supportsCalibration(void);

    virtual bool getCalDate(sDate_t *date);
    virtual bool setCalDate(sDate_t *date);

    //virtual bool getCalInterval(uint32_t *interval);
    virtual bool setCalInterval(uint32_t sensor, uint32_t interval);
    virtual bool initController(void);
    virtual bool setAquisationMode(eAquisationMode_t newAcqMode);
    virtual bool upgradeSensorFirmware(void);
    virtual void startUnit(void);
    virtual void shutdownUnit(void);
    virtual uint32_t shutdownSequence(void);
    virtual bool shutdownPeripherals(void);
    virtual bool moveMotorTillForwardEnd(void);
    virtual bool moveMotorTillReverseEnd(void);
    virtual bool moveMotorTillForwardEndThenHome(void);
    virtual bool moveMotorTillReverseEndThenHome(void);
    bool incrementAndLogSetPointInfo(void);
    virtual bool setSensorZeroValue(uint32_t sensor, float32_t zeroVal);
    virtual bool getSensorZeroValue(uint32_t sensor, float32_t *zeroVal);
    void refSensorDisconnectEventHandler(void);
    void baroSensorDisconnectEventHandler(void);
    bool isValidSetPoint(float32_t setPointValue);
    void runPressureSystem(void);
    uint32_t getOverPressureStatus(float32_t pressureG,
                                   float32_t pressureAbs,
                                   eSetPointType_t pressureType);

    void lowPassFilter(float32_t value, float32_t *filteredValue);
    virtual bool resetFilter(void);
    void logBistResults(void);
    virtual bool getCalibrationType(int32_t *calType, uint32_t *range);

};

#endif // _DFUNCTION_MEASURE_ADD_EXT_BARO_H
