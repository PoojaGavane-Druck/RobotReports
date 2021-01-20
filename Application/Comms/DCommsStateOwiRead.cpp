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
* @file     DCommsStateLocal.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The communications local state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <os.h>
#include <lib_def.h>
#include <stdint.h>
#include <stdlib.h>
MISRAC_ENABLE

#include "DParseMasterSlave.h"
#include "DCommsStateOwiRead.h"
//#include "DuciSensorCommands.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateLocal class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateOwiRead::DCommsStateOwiRead(DDeviceSerial *commsMedium, DTask* task)
: DCommsStateOwi(commsMedium,task)
{
    OS_ERR os_error;

    myParser = new DOwiParse((void *)this, &os_error);  //in local mode we don't know yet whether we will be master or slave

    createCommands();
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateOwiRead::createCommands(void)
{
    //create the common commands
    DCommsStateOwi::createCommands();

    //add those specific to this state instance
    
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for the local comms state (DUCI master)
 * @param   void
 * @retval  void
 */

eCommOperationMode_t DCommsStateOwiRead::run(void)
{
    OS_ERR os_err;
    uint8_t *buffer;
    sInstrumentMode_t mask;
    mask.value = 0u;
//    mask.test = 1u;
    mask.remoteSerial = 1u;
//    mask.remoteUsb = 1u;

    //Entry
#if 0
    /* This is commented as object is not initialised */
    PV624->userInterface->clearMode(mask);
#endif
    errorStatusRegister.value = 0u; //clear DUCI error status register

    externalDevice.status.all = 0u;

    sOwiError_t owiError;         //local variable for error status
    owiError.value = 0u;

    //DO
    nextState = E_STATE_OWI_READ;

    nextOperationMode = E_COMMS_READ_OPERATION_MODE;
    
    while (nextOperationMode == E_COMMS_READ_OPERATION_MODE)
    {
        if (commsOwnership == E_STATE_COMMS_REQUESTED)
        {
            commsOwnership = E_STATE_COMMS_RELINQUISHED;
        }

        //Polling for external connection every 500 ms.
        OSTimeDlyHMSM(0u, 0u, 0u, 500u, OS_OPT_TIME_HMSM_STRICT, &os_err);

        if (commsOwnership == E_STATE_COMMS_OWNED)
        {
            //add query command checksum explicitly here as this is only required for outgoing; reply may or may not have one
            if (waitForCommand( &buffer))
            {                
                errorStatusRegister.value |= owiError.value;

                if (errorStatusRegister.value != 0u)
                {
                    //TODO: Handle Error
                }
            }
        }
    }

    //Exit

    return nextOperationMode;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")





