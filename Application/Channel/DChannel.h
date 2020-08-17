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
* @file     DChannel.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DChannel base class header file
*/

#ifndef _DCHANNEL_H
#define _DCHANNEL_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

#include "Types.h"
#include "DFunctionMeasure.h"


/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DChannel
{
private:
    DFunction* myCurrentFunction;

protected:
    DFunctionMeasure *myMeasureFunctions[E_FUNCTION_MAX];
    

public:
    DChannel();

    virtual bool getFunction(eFunction_t *func, eFunctionDir_t *dir);
    virtual bool setFunction(eFunction_t func, eFunctionDir_t dir);

    virtual bool queryFunctionReady(void);

    virtual eUnits_t getUnits(void);
    virtual bool setUnits(eUnits_t units);

    virtual bool getOutput(uint32_t index, float32_t *value);
    virtual bool setOutput(uint32_t index, float32_t value);

    virtual bool getFunctionValue(uint32_t index, float32_t *value);
    bool getPosFullscale(float32_t *fs);
    bool getNegFullscale(float32_t *fs);
    virtual bool sensorRetry(void);
    virtual bool sensorContinue(void);

    virtual eSensorType_t getSensorType(void);

    virtual bool getProcessEnabled(eProcess_t process);
    virtual bool setProcessEnabled(eProcess_t process, bool state);
    virtual void resetProcess(eProcess_t process);

    virtual bool queryOutputInTolerance(void);

    virtual bool getSwitchState(void);
    virtual void resetSwitchTest(void);
    virtual float32_t getSwitchOpenAt(void);
    virtual float32_t getSwitchClosedAt(void);
    virtual float32_t getSwitchHysteresis(void);

    virtual void getMaxMin(float32_t *max, float32_t *min);
    virtual bool performZero(void);

    virtual bool getCalMode(void);
    virtual bool setCalMode(void);
    virtual bool abortCalMode(void);
    virtual bool setCalType(eCalType_t calType);
    virtual bool startCalSampling(void);
    virtual bool queryCalSamplingStatus(void);
    virtual bool setCalPoint(uint32_t point, float32_t value);
    virtual bool getCalDate(uint32_t range, sDate_t *date);
    virtual bool setCalDate(uint32_t range, sDate_t date);
    virtual bool getCalData(uint32_t range, sCalRange_t *calData);
    virtual bool getCalInterval(uint32_t range, sCalRange_t *calData);
    virtual uint32_t getNumCalPoints(uint32_t range);
    virtual bool getUserCalDate(sDate_t* caldate);
    virtual bool getManufactureDate( sDate_t *manfDate);
};

#endif // _DCHANNEL_H
