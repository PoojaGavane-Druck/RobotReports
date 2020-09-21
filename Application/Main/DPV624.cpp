/**
* BHGE Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DPV624.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     12 April 2020
*
* @brief    The PV624 instrument class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <os.h>
MISRAC_ENABLE

#include "DPV624.h"
#include "DSlot.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
DPV624 *PV624;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
#include "Utilities.h"

DPV624::DPV624(void)
{
    OS_ERR os_error;
    
    //create devices
    /* Commenting these objects for testing commsOwi */
    
    /*
    persistentStorage = new DPersistent();
*/

    //create application objects
    realTimeClock = new DRtc();
    instrument = new DInstrument(&os_error);
    commsOwi = new DCommsOwi("commsOwi", &os_error);
    commsUSB = new DCommsUSB("commsUSB", &os_error);
    powerManager = new DPowerManager();
   
    errorHandler = new DErrorHandler(&os_error);
    keyHandler = new DKeyHandler(&os_error);


    /*
    errorHandler = new DErrorHandler(&os_error);
    keyHandler = new DKeyHandler(&os_error);
    userInterface = new DUserInterface(&os_error);
    serialComms = new DCommsSerial("commsSerial", &os_error);
    
    */
    
    //Todo Added for Testing by Nag
     mySerialNumber = 10101111u;
}
