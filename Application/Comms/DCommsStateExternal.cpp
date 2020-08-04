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
* @file     DCommsStateExternal.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The communications external state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateExternal.h"
#include "DParseMaster.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateExternal class constructor
 * @param   commsMedium is pointer to
 * @retval  void
 */
DCommsStateExternal::DCommsStateExternal(DDeviceSerial *commsMedium)
: DCommsStateDuci(commsMedium)
{
    myParser = NULL;    //needs no parser as this state does no direct communication
}

/**
 * @brief   Run function for this class
 * @param   void
 * @retval  void
 */
eStateDuci_t DCommsStateExternal::run(void)
{
    OS_ERR osErr;

    //DO - just poll for serial number to (a) get he serial number and then (b) check that connection remains made
    nextState = E_STATE_DUCI_EXTERNAL;

    while (nextState == (eStateDuci_t)E_STATE_DUCI_EXTERNAL)
    {
        if (commsOwnership == (eStateComms_t)E_STATE_COMMS_RELINQUISHED) //means some other object has control of the serial port
        {
            //Wait 500 second and then check if still connected //TODO Wait for task message to hand over control
            OSTimeDlyHMSM(0u, 0u, 0u, 500u, OS_OPT_TIME_HMSM_STRICT, &osErr);
        }
        else
        {
            nextState = (eStateDuci_t)E_STATE_DUCI_DEVICE_DISCOVERY;
        }
    }

    //Exit

    return nextState;

//    DPI610E->userInterface->setMode((eUiMode_t)E_UI_MODE_LOCAL); //in this context the UI is still in local mode
//
//    //TODO: Send message to UI or instrument (TBD) to handover the serial port (myCommsMedium) for sensor comms
//    //the receiving end will use the port and send a message back to this state when finished (ie, sensor disconnected)
//
//    //TODO: Wait here forever until signalled by instrument that we no longer need external connection
//
//    //always return to local mode on return
//    return E_STATE_DUCI_LOCAL;
}
