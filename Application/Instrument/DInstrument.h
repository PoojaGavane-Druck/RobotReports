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
* @file     DInstrument.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The instrument base class header file
*/

#ifndef _DINSTRUMENT_H
#define _DINSTRUMENT_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DFunctionMeasure.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/


/* Variables --------------------------------------------------------------------------------------------------------*/

class DInstrument
{
private:

protected:
    DFunctionMeasure *myCurrentFunction;
public:
    DInstrument(OS_ERR *osErr);
    ~DInstrument();
    bool getFunction(eFunction_t *func);
    bool setFunction(eFunction_t func);
    bool getReading(eValueIndex_t index, float32_t *reading);
    bool getReading(eValueIndex_t index, int32_t *reading);
    bool getPosFullscale(float32_t *fs);
    bool getBarometerIdentity(uint32_t *identity);
    bool getNegFullscale(float32_t *fs);
    bool getManufactureDate(sDate_t *manfDate);
    bool getUserCalDate(sDate_t *caldate);
    bool getFactoryCalDate(sDate_t *caldate);
    bool getSensorType(eSensorType_t *pSenType);
    bool sensorContinue(void);
    bool sensorRetry(void);
    bool getExternalSensorAppIdentity(uSensorIdentity_t *identity);
    bool getExternalSensorBootLoaderIdentity(uSensorIdentity_t *identity);
    bool getControllerMode(eControllerMode_t *controllerMode);
    bool setControllerMode(eControllerMode_t newCcontrollerMode);
    bool getCalInterval(uint32_t *pInterval);
    bool setCalInterval(uint32_t sensor, uint32_t interval);
    void takeNewReading(uint32_t rate);
    bool getPressureSetPoint(float *setPoint);
    bool setPressureSetPoint(float newSetPointValue);
    bool getPM620Type(uint32_t *sensorType);
    bool reloadCalibration(void);

    bool setVentRate(float rate);
    bool getVentRate(float *rate);

    bool setFilterCoeff(float32_t filterCoeff);
    bool getFilterCoeff(float32_t *filterCoeff);
    bool resetDisplayFilter(void);

    bool getCalDate(sDate_t *date);
    bool setCalDate(sDate_t *date);

    bool setCalibrationType(int32_t calType, uint32_t range);
    bool getRequiredNumCalPoints(eSensor_t sensorType, uint32_t *numCalPoints);
    bool setRequiredNumCalPoints(uint32_t numCalPoints);
    bool startCalSampling(void);
    bool getCalSamplesRemaining(uint32_t *samples);
    bool setCalPoint(uint32_t calPoint, float32_t value);
    bool acceptCalibration(void);
    bool abortCalibration(void);
    bool getControllerStatus(uint32_t *controllerStatus);
    bool getSensorCalDate(sDate_t *date);
    bool getSensorSerialNumber(uint32_t *sn);
    bool getPressureReading(float *pressure);
    bool getPositiveFS(float *pressure);
    bool getNegativeFS(float *pressure);
    bool getSensorBrandMin(char *brandMin, uint32_t bufLen);
    bool getSensorBrandMax(char *brandMax, uint32_t bufLen);
    bool getSensorBrandType(char *brandType, uint32_t bufLen);
    bool getSensorBrandUnits(char *brandUnits, uint32_t bufLen);
    bool setControllerStatus(uint32_t controllerStatus);
    bool initController(void);
    bool setAquisationMode(eAquisationMode_t newAcqMode);
    bool upgradeSensorFirmware(void);
    bool opticalEvent(uint32_t eventNum);
    void startup(void);
    void shutdown(void);
    bool getBaroPosFullscale(float32_t  *fs);
    bool getBaroNegFullscale(float32_t  *fs);
    bool moveMotorTillForwardEndThenHome(void);
    bool moveMotorTillReverseEndThenHome(void);
    bool moveMotorTillForwardEnd(void);
    bool moveMotorTillReverseEnd(void);
    bool setSensorZeroValue(uint32_t sensor, float zeroVal);
    bool getSensorZeroValue(uint32_t sensor, float *zeroVal);
    bool shutdownPeripherals(void);

};

#endif // _DINSTRUMENT_H
