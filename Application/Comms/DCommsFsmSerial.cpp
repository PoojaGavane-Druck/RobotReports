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
* @file     DCommsFsmSerial.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The serial communications finite state machine class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsFsmSerial.h"

MISRAC_DISABLE
#include <lib_def.h>
MISRAC_ENABLE

#include "DCommsStateLocal.h"
#include "DCommsStateRemote.h"
//#include "DCommsStateExternal.h"
#include "DCommsStateRemoteSerial.h"
//#include "DCommsStateProdTest.h"
//#include "DCommsStateDevDiscovery.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsFsmSerialSerial class constructor
 * @param   void
 * @retval  void
 */
DCommsFsmSerial::DCommsFsmSerial(void)
: DCommsFsm()
{
}

/**
 * @brief   Create required states of the state machine
 * @param   commsMedium is pointer to serial comms medium
 * @retval  void
 */
void DCommsFsmSerial::createStates(DDeviceSerial *commsMedium)
{
    //create all the states of the 'finite state machine'
    myStateArray[E_COMMS_READ_OPERATION_MODE] = new DCommsStateLocal(commsMedium);
   // myStateArray[E_STATE_DUCI_EXTERNAL] = new DCommsStateExternal(commsMedium);
    myStateArray[E_COMMS_WRITE_OPERATION_MODE] = new DCommsStateRemote(commsMedium);
  //  myStateArray[E_STATE_DUCI_PROD_TEST] = new DCommsStateProdTest(commsMedium);
//myStateArray[E_STATE_DUCI_DEVICE_DISCOVERY] = new DCommsStateDevDiscovery(commsMedium);

    //always starts in local mode (DUCI master)
    myInitialMode = E_COMMS_READ_OPERATION_MODE;
}

