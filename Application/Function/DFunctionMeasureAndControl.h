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
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

#include "DFunctionMeasure.h"
#include "Controller.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DFunctionMeasureAndControl : public DFunctionMeasure
{
  DSlot *myBarometerSlot;               //the slot (thread) that runs the sensor for this function
  float32_t myBarometerReading;         //Processed Barometer measureementValue
  float32_t myAbsoluteReading;
  float32_t myGaugeReading;
  controllerStatus_t myStatus;
  eControllerMode_t myMode;   // It tells about current mode
  eControllerMode_t myNewMode; // It tells about new mode which we received from DPI620G
  float32_t myCurrentPressureSetPoint; // It tells about current set point value

  DController *pressureController;
  bool getPressureInfo(pressureInfo_t *info);
  bool setPmSampleRate(void);
protected:
    
    virtual void createSlots(void);
    virtual void getSettingsData(void);
    virtual void runProcessing(void);
    virtual void handleEvents(OS_FLAGS actualEvents);

public:
    DFunctionMeasureAndControl();
    virtual void runFunction(void);             //the 'while' loop
    virtual bool setValue(eValueIndex_t index, uint32_t value);     //set specified integer function value
    virtual bool getValue(eValueIndex_t index, uint32_t *value);    //get specified integer function value
    virtual bool getValue(eValueIndex_t index, float32_t *value);  //read function measured value
    virtual bool setValue(eValueIndex_t index, float32_t value);
    virtual void takeNewReading(uint32_t rate);

    virtual bool setCalibrationType(int32_t calType, uint32_t range);
    virtual bool getRequiredNumCalPoints(uint32_t *numCalPoints);
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
    
    virtual bool getCalInterval(uint32_t *interval);
    virtual bool setCalInterval(uint32_t interval);
};

#endif // _DFUNCTION_MEASURE_ADD_EXT_BARO_H
