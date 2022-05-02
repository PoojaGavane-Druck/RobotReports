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
* @file     DFunction.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DFunction base class header file
*/

#ifndef _DFUNCTION_H
#define _DFUNCTION_H

/* Includes ---------------------------------------------------------------------------------------------------------*/


#include "DTask.h"
#include "DSlot.h"
#include "Types.h"
#include "DProcess.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef union
{
    uint32_t all;
    struct
    {
        uint32_t calibrate      : 1;
        uint32_t leakTest       : 1;
        uint32_t switchTest     : 1;

        uint32_t reserved       : 29;
    };

} sCapabilities_t;
/* Variables --------------------------------------------------------------------------------------------------------*/

class DFunction : public DTask
{
protected:
    OS_MUTEX myMutex;                       //mutex for resource locking
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond
    DSlot *mySlot;                          //the slot (thread) that runs the sensor for this function

    // DProcess *processes[E_PROCESS_NUMBER];



    float32_t myReading;                    //processed measurement value
    float32_t myAbsPosFullscale;            //Absolute Positive fullscale of function sensor
    float32_t myAbsNegFullscale;            //Absolute Negative fullscale of function sensor
    float32_t myPosFullscale;               //Positive fullscale of function sensor
    float32_t myNegFullscale;               //Negative fullscale of function sensor
    float32_t myResolution;                 //Resolution (accuracy of measurements)
    eSensorType_t myType;                   //My Sensor Type
    sDate_t myManufactureDate;                //Manufacturing Date
    sDate_t myUserCalibrationDate;            //User Calibration Date
    sDate_t myFactoryCalibrationDate;         //Factory Calibration Date
    sCapabilities_t capabilities;
    //sFunctionSetting_t mySettings;          //user settings such as process/units

    virtual void createSlots(void);
    virtual void handleEvents(OS_FLAGS actualEvents);

    virtual void getSettingsData(void);
    virtual void validateSettings(void);
    virtual void applySettings();

    void runProcessing(void);


    void setReading(float32_t value);       //set processed measurement value
    float32_t getPosFullscale(void);        //get positive fullscale of function sensor
    void setPosFullscale(float32_t value);  //set positive fullscale of function sensor
    float32_t getNegFullscale(void);        //get negative fullscale of function sensor
    void setNegFullscale(float32_t value);  //set negative fullscale of function sensor
    float32_t getResolution(void);          //get resolution (accuracy of measurements)
    void setResolution(float32_t value);    //set resolution (accuracy of measurements)


    void updateSensorInformation(void);     //update sensor information
    float32_t myVentRate;


public:
    eFunction_t myFunction;                 //quick means of knowing own function type


    DFunction(void);
    virtual ~DFunction();
    virtual void start(void);               //initialisation of function before the 'while' loop runs
    virtual void runFunction(void);         //the 'while' loop
    virtual void cleanUp(void);             //graceful close

    float32_t getAbsPosFullscale(void);
    void setAbsPosFullscale(float32_t value);
    float32_t getAbsNegFullscale(void);
    void setAbsNegFullscale(float32_t value);
    eSensorType_t getSensorType(void);
    void getManufactureDate(sDate_t *date);     //Get Manufacturing Date
    void getCalDate(eSensorCalType_t caltype, sDate_t *date);
    //Note: Operations that read sensor values may go directly to sensor (bypassing the slot)
    virtual bool getOutput(uint32_t index, float32_t *value);   //read function output
    virtual bool getValue(eValueIndex_t index, float32_t *value);  //read function measured value
    virtual bool getValue(eValueIndex_t index, uint32_t *value);  //read param value
    virtual bool setValue(eValueIndex_t index, float32_t value);
    virtual bool setValue(eValueIndex_t index, uint32_t value);
    virtual bool sensorRetry(void);
    virtual bool sensorContinue(void);
    virtual bool setFunction(eFunction_t func);
    virtual bool getFunction(eFunction_t *func);
    virtual  bool getBarometerIdentity(uint32_t *identity);
    //Note: Operations that change sensor values must go through the slot and not directly to sensor
    virtual bool setOutput(uint32_t index, float32_t value);    //write function output

    virtual void takeNewReading(uint32_t rate);

    virtual bool setCalibrationType(int32_t calType, uint32_t range);
    virtual bool getRequiredNumCalPoints(uint32_t *numCalPoints);
    virtual bool setRequiredNumCalPoints(uint32_t numCalPoints);
    virtual bool startCalSampling(void);
    virtual bool getCalSamplesRemaining(uint32_t *samples);
    virtual bool setCalPoint(uint32_t calPoint, float32_t value);
    virtual bool acceptCalibration(void);
    virtual bool abortCalibration(void);

    virtual void suspendProcesses(bool state);
    virtual bool reloadCalibration(void);

    virtual bool supportsCalibration(void);

    virtual bool getCalDate(sDate_t *date);
    virtual bool setCalDate(sDate_t *date);

    virtual bool getCalInterval(uint32_t *interval);
    virtual bool setCalInterval(uint32_t interval);

    virtual bool getSensorCalDate(sDate_t *date);

    virtual bool getSensorSerialNumber(uint32_t *sn);
    virtual bool getPressureReading(float *pressure);
    virtual bool getNegativeFS(float *pressure);
    virtual bool getPositiveFS(float *pressure);
    virtual bool getSensorBrandUnits(char *brandUnits);
    virtual bool initController(void);
    virtual bool setAquisationMode(eAquisationMode_t newAcqMode);
    virtual bool upgradeSensorFirmware(void);
    virtual void startUnit(void);
    virtual void shutdownUnit(void);
    virtual bool moveMotorTillForwardEndThenHome(void);
    virtual bool moveMotorTillReverseEndThenHome(void);

};

#endif // _DFUNCTION_H
