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
* @file     DCommsFsmUsb.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The communications finite state machine class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsFsmUsb.h"

MISRAC_DISABLE
#include <lib_def.h>
MISRAC_ENABLE

#include "Utilities.h"
#include "DCommsStateUsbIdle.h"
#include "DCommsStateRemoteUsb.h"
#include "DCommsStateProdTest.h"
#include "DCommsStateEngPro.h"
#include "DCommsStateDump.h"

/* Error handler instance parameter starts from 701 to 800 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
//#define PRODUCTION_TEST_BUILD
//#define ENG_ALGO_TESTING
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsFsmUsb class constructor
 * @param   void
 * @retval  void
 */
DCommsFsmUsb::DCommsFsmUsb(void)
    : DCommsFsm()
{

}

/**
 * @brief   DCommsFsmUsb class destructor
 * @param   void
 * @retval  void
 */
DCommsFsmUsb::~DCommsFsmUsb(void)
{
    for(uint32_t index = (uint32_t)E_STATE_DUCI_LOCAL;
            index < (uint32_t)E_STATE_DUCI_SIZE;
            index++)
    {
        if(NULL != myStateArray[index])
        {
            delete  myStateArray[index];
        }
    }

    delete[] myStateArray;
}
/**
 * @brief   Create required states of the state machine
 * @param   commsMedium is pointer to serial comms medium
 * @param   task is pointer to own task
 * @retval  void
 */
void DCommsFsmUsb::createStates(DDeviceSerial *commsMedium, DTask *task)
{
    //create all the states of the 'finite state machine'
    if(E_STATE_DUCI_LOCAL < E_STATE_DUCI_SIZE)
    {
        myStateArray[E_STATE_DUCI_LOCAL] = new DCommsStateUsbIdle(commsMedium, task);
    }

    if(E_STATE_DUCI_REMOTE < E_STATE_DUCI_SIZE)
    {
        myStateArray[E_STATE_DUCI_REMOTE] = new DCommsStateRemoteUsb(commsMedium, task);
    }

    if(E_STATE_DUCI_ENG_TEST < E_STATE_DUCI_SIZE)
    {
        myStateArray[E_STATE_DUCI_ENG_TEST] = new DCommsStateEngPro(commsMedium, task);
    }

    if(E_STATE_DUCI_PROD_TEST < E_STATE_DUCI_SIZE)
    {
        myStateArray[E_STATE_DUCI_PROD_TEST] = new DCommsStateProdTest(commsMedium, task);
    }

    if(E_STATE_DUCI_DATA_DUMP < E_STATE_DUCI_SIZE)
    {
        myStateArray[E_STATE_DUCI_DATA_DUMP] = new DCommsStateDump(commsMedium, task);
    }

    //myInitialState = E_STATE_DUCI_DATA_DUMP;
    myInitialState = E_STATE_DUCI_LOCAL;

}
