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
* @file     DDPV624.cpp
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
    persistentStorage = new DPersistent();


    //create application objects
    instrument = new DInstrument(&os_error);
    errorHandler = new DErrorHandler(&os_error);
    keyHandler = new DKeyHandler(&os_error);
    userInterface = new DUserInterface(&os_error);
    serialComms = new DCommsSerial("commsSerial", &os_error);
    commsUSB = new DCommsUSB("commsUSB", &os_error);
}
