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
* @file     DCommsStateUsbIdle.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     04 June 2020
*
* @brief    The USB communications idle (local) state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <assert.h>
#include <os.h>
#include <lib_def.h>
#include <stdint.h>
#include <stdlib.h>
MISRAC_ENABLE

#include "DParseMasterSlave.h"
#include "DCommsStateUsbIdle.h"
// TEST - SCS - Removed
//#include "DuciSensorCommands.h"
#include "DPV624.h"
#include "Utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define MASTER_SLAVE_USB_COMMANDS_ARRAY_SIZE  8  //this is the maximum no of commands supported in DUCI master/slave mode (can be increased if more needed)

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
sDuciCommand_t duciMasterSlaveUsbCommands[MASTER_SLAVE_USB_COMMANDS_ARRAY_SIZE]; //TODO HSB: This needs to be on a per-instance basis!!!! there are multiple instances
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateUsbIdle class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateUsbIdle::DCommsStateUsbIdle(DDeviceSerial *commsMedium, DTask* task)
    : DCommsStateDuci(commsMedium, task)
{
    OS_ERR os_error;

    myParser = new DParseMasterSlave((void *)this, &duciMasterSlaveUsbCommands[0], (size_t)MASTER_SLAVE_USB_COMMANDS_ARRAY_SIZE, &os_error);

    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE));

    if(!ok)
    {
      #ifdef ASSERT_IMPLEMENTED
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
#endif
        error_code_t errorCode;
        errorCode.bit.osError = SET;
        PV624->handleError(errorCode, os_error);
    }
    createCommands();

    commandTimeoutPeriod = 900u; //default time in (ms) to wait for a response to a DUCI command
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for the local comms state (DUCI master)
 * @param   void
 * @retval  void
 */
eCommOperationMode_t DCommsStateUsbIdle::run(void)
{
    char *buffer;

    nextOperationMode = E_COMMS_READ_OPERATION_MODE;

    //Entry
    sInstrumentMode_t mask;
    mask.value = 0u;
    mask.remoteUsb = 1u;

    //can't be in USB remote mode, so clear that bit
#ifdef UI_ENABLED
    PV624->userInterface->clearMode(mask);
#endif
    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;

    //DO
    clearRxBuffer();
    while(nextOperationMode == E_COMMS_READ_OPERATION_MODE)
    {
       // sleep(500u);

        sleep(50u);

        //listen for a command over USB comms
        if(receiveString(&buffer))
        {
            duciError = myParser->parse(buffer);

            errorStatusRegister.value |= duciError.value;

            if(errorStatusRegister.value != 0u)
            {
                //TODO: Handle Error
            }
            clearRxBuffer();
        }
    }

    //Exit

    return nextOperationMode;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")

/* Static callback functions ----------------------------------------------------------------------------------------*/
sDuciError_t DCommsStateUsbIdle::fnSetKM(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted KM message in this state is a command type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        switch(parameterArray[1].charArray[0])
        {
         case 'R':    //enter remote mode
            if(currentWriteMaster == (eCommMasterInterfaceType_t)E_COMMS_MASTER_NONE)
            {
                nextOperationMode = E_COMMS_WRITE_OPERATION_MODE;
                nextState = (eStateDuci_t)E_STATE_DUCI_REMOTE;
                currentWriteMaster = (eCommMasterInterfaceType_t)E_COMMS_DUCI_OVER_USB;
            }
            else
            {
              duciError.invalid_args = 1u;
            }
         break; 
         case 'S':    //enter service mode
            if(currentWriteMaster == (eCommMasterInterfaceType_t)E_COMMS_MASTER_NONE)
            {
                nextOperationMode = E_COMMS_PRODUCTION_OPERATION_MODE;
                nextState = (eStateDuci_t)E_STATE_DUCI_REMOTE;
                currentWriteMaster = (eCommMasterInterfaceType_t)E_COMMS_DUCI_OVER_USB;
            }
            else if((currentWriteMaster == (eCommMasterInterfaceType_t)E_COMMS_MASTER_NONE)||(nextOperationMode == (eCommOperationMode_t)E_COMMS_WRITE_OPERATION_MODE))
            {
                nextOperationMode = E_COMMS_PRODUCTION_OPERATION_MODE;
                currentWriteMaster = (eCommMasterInterfaceType_t)E_COMMS_DUCI_OVER_USB;
            }
            else
            {
              duciError.invalid_args = 1u;
            }
         break; 
         

         case 'L':    //already in this mode so stay here - do nothing
            break;

        default:
            duciError.invalid_args = 1u;
            break;
        }
    }

    return duciError;
}

