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
#include "DCommsSerial.h"
#include "DErrorHandler.h"
#include "DCommsUSB.h"
#include "DCommsOwi.h"
#include "DPersistent.h"
#include "DRTC.h"
#include "DBattery.h"
#include "DPowerManager.h"
#include "DStepperController.h"
#include "DSensorTemperature.h"
/* Types ------------------------------------------------------------------------------------------------------------*/
class DPV624
{
public:

    DPV624(); //constructor

    //devices
    DPersistent *persistentStorage;
  

    //application objects
    DRtc *realTimeClock;
    DInstrument *instrument;
    DErrorHandler *errorHandler; //error indication shall have priority over all screen states.
    DKeyHandler *keyHandler;
    DUserInterface *userInterface;
   
    DCommsSerial *serialComms;
    DCommsUSB *commsUSB;
    DCommsOwi *commsOwi;
    


    DPowerManager *powerManager;
//    DDataLogger *dataLogger;		//data logger

    DStepperController *stepperController;
    DSensorTemperature *temperatureSensor;
    
    uint32_t mySerialNumber;
    float myTareValue;
    void handleError(error_code_t errorCode, int32_t param = -1, bool blocking = false);
    uint32_t getSerialNumber(void);
    bool setSerialNumber(uint32_t newSerialNumber);
    eRegionOfUse_t getRegion(void);
    bool setRegion(eRegionOfUse_t region);
    bool getTime(sTime_t *time);
    bool setTime(sTime_t *time);
    void validateApplicationObject(OS_ERR os_error);
    
    //For Production Testing
    uint32_t getKey(void);
    bool setKey(uint32_t key, uint32_t pressType);
    
    uint32_t getTestPoint(uint32_t index);
    uint32_t setTestPoint(uint32_t index, uint32_t parameter);
    
    int32_t queryEEPROMTest(void);
    void performEEPROMTest(void);
};

/* Variables -------------------------------------------------------------------------------------------------------*/
extern DPV624 *PV624;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

#endif /* __DPI610E_H */
