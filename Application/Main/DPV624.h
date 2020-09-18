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

    //ToDo: Added for Testing
    uint32_t mySerialNumber;
    float myTareValue;
};

/* Variables -------------------------------------------------------------------------------------------------------*/
extern DPV624 *PV624;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

#endif /* __DPI610E_H */
