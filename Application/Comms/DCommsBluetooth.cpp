/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DCommsBluetooth.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     13 November 2020
*
* @brief    The Bluetooth communications class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsBluetooth.h"
#include "DDeviceSerialBluetooth.h"
#include "DCommsFsmBluetooth.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsBluetooth class constructor
 * @param   medium name
 * @param   os error
 * @retval  void
 */
DCommsBluetooth::DCommsBluetooth(char *mediumName, OS_ERR *os_error)
    : DComms()
{
    myName = mediumName;

    //start the comms task
    start(mediumName, os_error);
}

/**
 * @brief   DCommsBluetooth initialisation function (overrides the base class)
 * @param   void
 * @retval  void
 */
void DCommsBluetooth::initialise(void)
{

    myTaskId = eCommunicationOverBluetoothTask;
    //specify the comms medium
    commsMedium = new DDeviceSerialBluetooth();

    //create the comms state machine- having setup the medium first
    myCommsFsm = new DCommsFsmBluetooth();
}

/**
 * @brief   Set (enter/exit) test mode
 * @param   state: value true = enter test mode, false = exit test mode
 * @retval  void
 */
void DCommsBluetooth::setTestMode(bool state)
{
    if(myCommsFsm != NULL)
    {
        if(state == true)
        {
            myCommsFsm->suspend();
        }

        else
        {
            myCommsFsm->resume();
        }
    }
}

