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
* @file     DPV624.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     12 April 2020
*
* @brief    The DPI610E instrument class header file
*/

#ifndef __DPV624_H
#define __DPV624_H

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DInstrument.h"
#include "DKeyHandler.h"
#include "DUserInterface.h"
#include "DErrorHandler.h"
#include "DCommsUSB.h"
#include "DCommsSerial.h"
#include "DCommsOwi.h"
#include "DCommsMotor.h"
#include "DPersistent.h"
#include "DBattery.h"
#include "DPowerManager.h"
#include "DStepperMotor.h"
#include "DSensorTemperature.h"
#include "DValve.h"
#include "leds.h"
/* Types ------------------------------------------------------------------------------------------------------------*/
class DPV624
{
  
private:
    ePinMode_t myPinMode;
public:

    DPV624(); //constructor

    //devices
    DPersistent *persistentStorage;
  
    //uint32_t controllerStatus;
    
    //application objects
    
    DInstrument *instrument;
    DErrorHandler *errorHandler; //error indication shall have priority over all screen states.
    DKeyHandler *keyHandler;
    DUserInterface *userInterface;
     
    DCommsUSB *commsUSB;
    DCommsOwi *commsOwi;
    DCommsSerial *commsSerial;
    //DCommsMotor *commsMotor;
    
    DValve *valve1;
    DValve *valve2;
    DValve *valve3;

    LEDS *leds;

    DPowerManager *powerManager;
    DStepperMotor *stepperMotor;
    DSensorTemperature *temperatureSensor;
    
    float myTareValue;
	sDate_t manufactureDate;
	float zeroVal;
    void handleError(error_code_t errorCode, int32_t param = -1, bool blocking = false);
    uint32_t getSerialNumber(uint32_t snType);
    bool setSerialNumber(uint32_t newSerialNumber);
    eRegionOfUse_t getRegion(void);
    bool setRegion(eRegionOfUse_t region);
    bool getDate(sDate_t *date);
    bool setDate(sDate_t *date);
    bool getTime(sTime_t *timeNow);
    bool setTime(sTime_t *timeNow);
    void validateApplicationObject(OS_ERR os_error);
    
    //For Production Testing
    uint32_t getKey(void);
    bool setKey(uint32_t key, uint32_t pressType);
    
    uint32_t getTestPoint(uint32_t index);
    uint32_t setTestPoint(uint32_t index, uint32_t parameter);
    
    int32_t queryEEPROMTest(void);
    void performEEPROMTest(void);
    bool getVersion(uint32_t item, uint32_t component, char itemverStr[10]); 
    bool getPosFullscale( float32_t  *fs);
    bool getNegFullscale( float32_t  *fs);
    bool getSensorType( eSensorType_t *sensorType);
    bool getControllerMode(eControllerMode_t *controllerMode);
    bool setControllerMode(eControllerMode_t newCcontrollerMode);
    bool getDK(uint32_t item, uint32_t component, char dkStr[7]);
    void getInstrumentName(char nameStr[13]);
    bool getCalInterval( uint32_t *interval);
    bool setCalInterval( uint32_t interval);
    bool getFunction( eFunction_t *func);
    void takeNewReading(uint32_t rate);
    bool setControllerStatus(uint32_t newStatus);
    bool getControllerStatus(uint32_t *newStatus);
    bool getPressureSetPoint(float *setPoint);
    bool setPressureSetPoint(float newSetPointValue);
    void resetStepperMicro(void);
    bool getPM620Type(uint32_t *sensorType);
    ePinMode_t getPinMode(void);
    bool setPinMode(ePinMode_t mode);
    bool getSensorBrandUnits(char *brandUnits);
    bool setManufactureDate( sDate_t *date);
    bool getManufactureDate( sDate_t *date);
    int32_t getUsbInstrumentPortConfiguration();
    void setUsbInstrumentPortConfiguration(int32_t mode);
    bool backupCalDataRestore(void);
    bool backupCalDataSave(void);
    bool setZero( float32_t value);
    bool performPM620tUpgrade(void);
    bool performUpgrade(void);
    bool setRequiredNumCalPoints(uint32_t numCalPoints);
    bool abortCalibration(void);
    bool acceptCalibration(void);
    bool setCalPoint(uint32_t calPoint, float32_t value);
    bool startCalSampling(void);
    bool setCalibrationType(int32_t calType, uint32_t range);
    bool setCalDate( sDate_t *date);
    int32_t queryInvalidateCalOpeResult(void);
    bool getZero(float32_t *value);
    bool getRequiredNumCalPoints(uint32_t *numCalPoints);
    bool getCalSamplesRemaining(uint32_t *samples);
    bool getCalDate( sDate_t *date);
	bool invalidateCalibrationData(void);
	DStepperMotor* getStepperMotorInstance(void);

             
    void getBatteryStatus(sBatteryStatus_t *sBatteryStatus);
    //bool getControllerStatus(uint32_t *controllerStatus);
   
    
    
    
};

/* Variables -------------------------------------------------------------------------------------------------------*/
extern DPV624 *PV624;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

#endif /* __DPI610E_H */
