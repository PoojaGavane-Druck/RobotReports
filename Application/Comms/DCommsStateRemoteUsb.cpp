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
* @file     DCommsStateRemoteUsb.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     04 June 2020
*
* @brief    The USB communications remote state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateRemoteUsb.h"
#include "DUserInterface.h"
#include "DPV624.h"
/* Error handler instance parameter starts from 2001 to 2100 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateRemoteUsb class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateRemoteUsb::DCommsStateRemoteUsb(DDeviceSerial *commsMedium, DTask *task)
    : DCommsStateDuci(commsMedium, task)
{
    //get reference to the remote mode state (singleton) function
    myRemoteCommsState = DCommsStateRemote::getInstance();

}

/**
 * @brief   DCommsStateRemoteUsb class destructor
 * @param   void
 * @retval  void
 */
DCommsStateRemoteUsb::~DCommsStateRemoteUsb(void)
{
}
/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for this class
 * @param   void
 * @retval  next state
 */
eStateDuci_t DCommsStateRemoteUsb::run(void)
{
    //if already remote on another link then revert to local mode (unavailable if return false)
    if(myRemoteCommsState->setCommsMedium(myCommsMedium) == false)
    {
        nextState = E_STATE_DUCI_LOCAL;
    }

    else
    {

        PV624->setCommModeStatus(E_COMM_USB_INTERFACE, E_COMM_MODE_REMOTE);

        nextState = myRemoteCommsState->run();

        //Exit
        PV624->setCommModeStatus(E_COMM_USB_INTERFACE, E_COMM_MODE_LOCAL);

    }

    return nextState;
}


/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")
