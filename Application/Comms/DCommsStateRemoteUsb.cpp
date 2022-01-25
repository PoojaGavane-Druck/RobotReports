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
#ifdef USER_INTERFACE_ENABLED
        sInstrumentMode_t mask;
        mask.value = 0u;
        mask.remoteUsb = 1u;

        //Entry
        PV624->userInterface->setMode(mask);
#endif
        nextState = myRemoteCommsState->run();

        //Exit
#ifdef USER_INTERFACE_ENABLE
        PV624->userInterface->clearMode(mask);
#endif
    }

    return nextState;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")
