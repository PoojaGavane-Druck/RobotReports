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
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

#include "DTask.h"
#include "DSlot.h"
#include "Types.h"
#include "DProcess.h"
//#include "PersistentFunctions.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

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
    //sFunctionSetting_t mySettings;          //user settings such as process/units

    virtual void createSlots(void);
    virtual void handleEvents(OS_FLAGS actualEvents);

    virtual void getSettingsData(void);
    virtual void validateSettings(void);
    virtual void applySettings();

    void runProcessing(void);

    
    void setReading (float32_t value);      //set processed measurement value
    float32_t getPosFullscale(void);        //get positive fullscale of function sensor
    void setPosFullscale(float32_t value);  //set positive fullscale of function sensor
    float32_t getNegFullscale(void);        //get negative fullscale of function sensor
    void setNegFullscale(float32_t value);  //set negative fullscale of function sensor
    float32_t getResolution(void);          //get resolution (accuracy of measurements)
    void setResolution(float32_t value);    //set resolution (accuracy of measurements)

  
    void updateSensorInformation(void);     //update sensor information

    

public:
    eFunction_t myFunction;                 //quick means of knowing own function type
    

    DFunction(void);

    virtual void start(void);               //initialisation of function before the 'while' loop runs
    virtual void runFunction(void);         //the 'while' loop
    virtual void cleanUp(void);             //graceful close

    float32_t getAbsPosFullscale(void);
    void setAbsPosFullscale(float32_t value);
    float32_t getAbsNegFullscale(void);
    void setAbsNegFullscale(float32_t value);
    eSensorType_t getSensorType(void);
    void getManufactureDate(sDate_t *date);     //Get Manufacturing Date
    void getCalDate(eSensorCalType_t caltype, sDate_t* date);
    //Note: Operations that read sensor values may go directly to sensor (bypassing the slot)
    virtual bool getOutput(uint32_t index, float32_t *value);   //read function output
    virtual bool getValue(eValueIndex_t index, float32_t *value);  //read function measured value
    virtual bool getValue(eValueIndex_t index, uint32_t *value);  //read param value
    bool setValue(eValueIndex_t index, float32_t value);
    virtual bool sensorRetry(void);
    virtual bool sensorContinue(void);
    virtual bool setFunction(eFunction_t func);
    virtual eFunction_t getFunction(void);
    virtual  bool getBarometerIdentity( uint32_t *identity);
    //Note: Operations that change sensor values must go through the slot and not directly to sensor
    virtual bool setOutput(uint32_t index, float32_t value);    //write function output
};

#endif // _DFUNCTION_H
