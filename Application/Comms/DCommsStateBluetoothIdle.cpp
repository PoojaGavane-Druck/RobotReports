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
* @file     DCommsStateBluetoothIdle.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     13 November 2020
*
* @brief    The Bluetooth communications idle (local) state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <assert.h>
extern "C"
{
#include <lib_ascii.h>
}
MISRAC_ENABLE

#include "DParseSlave.h"
#include "DCommsStateBluetoothIdle.h"
#include "DPV624.h"
#include "Utilities.h"


/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define MASTER_SLAVE_BT_COMMANDS_ARRAY_SIZE  8  //this is the maximum no of commands supported in Bluetooth DUCI master mode (can be increased if more needed)

/* Variables --------------------------------------------------------------------------------------------------------*/
sDuciCommand_t duciSlaveBtCommands[MASTER_SLAVE_BT_COMMANDS_ARRAY_SIZE];

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateBluetoothIdle class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateBluetoothIdle::DCommsStateBluetoothIdle(DDeviceSerial *commsMedium, DTask* task)
    : DCommsStateDuci(commsMedium, task)
{
    OS_ERR os_error = OS_ERR_NONE;

    myParser = new DParseSlave((void *)this, &duciSlaveBtCommands[0], (size_t)MASTER_SLAVE_BT_COMMANDS_ARRAY_SIZE, &os_error);

    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE));

    if(!ok)
    {
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
        
        PV624->handleError(E_ERROR_OS, 
                               eSetError,
                               (uint32_t)os_error,
                               (uint16_t)1);
    }

    createCommands();

    commandTimeoutPeriod = 250u; //default time in (ms) to wait for a response to a DUCI command
    commsOwnership = E_STATE_COMMS_RELINQUISHED;
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
eStateDuci_t DCommsStateBluetoothIdle::run(void)
{
    char *buffer;

    
    nextState = E_STATE_DUCI_LOCAL;

    //Entry
#if 0
    sInstrumentMode_t mask;
    mask.value = 0u;
    mask.remoteBluetooth = 1u;
#endif
    //can't be in USB remote mode, so clear that bit
   // PV624->userInterface->clearMode(mask);

    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;

    //clear buffer before we start
    clearRxBuffer();

     //DO
    
    
    while(E_STATE_DUCI_LOCAL == nextState)
    {
        if (myTask != NULL)
        {
#ifdef TASK_MONITOR_IMPLEMENTED
            myTask->keepAlive();
#endif
        }

        //check if any other part of application requires tshi task to stop interfering with comms
        if (commsOwnership == E_STATE_COMMS_REQUESTED)
        {
            commsOwnership = E_STATE_COMMS_RELINQUISHED;

            //wait and continue
            sleep(commandTimeoutPeriod);
        }
        else if (commsOwnership == E_STATE_COMMS_OWNED)
        {
            //listen for a command over BT
            if (receiveString(&buffer))
            {
                duciError = myParser->parse(buffer);
                clearRxBuffer();

                errorStatusRegister.value |= duciError.value;

                if (errorStatusRegister.value != 0u)
                {
                    //TODO: Handle Error
                }
            }
        }
        else
        {
            //just wait and continue
            sleep(commandTimeoutPeriod);
        }
    }

    //Exit

    return nextState;
}

/**
 * @brief   Suspend state machine
 * @param   void
 * @retval  void
 */
void DCommsStateBluetoothIdle::suspend(void)
{
    commsOwnership = E_STATE_COMMS_REQUESTED;

    while (commsOwnership == (eStateComms_t)E_STATE_COMMS_REQUESTED)
    {
        //wait until request has been processed
        sleep(50u);
    }
}

/**
 * @brief   Resume state machine
 * @param   void
 * @retval  void
 */
void DCommsStateBluetoothIdle::resume(void)
{
    commsOwnership = E_STATE_COMMS_OWNED;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")

/* Static callback functions ----------------------------------------------------------------------------------------*/
/**
 * @brief   DUCI handler for KM Command – Set front panel keypad operating mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateBluetoothIdle::fnSetKM(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted KM message in this state is a command type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        char ch = ASCII_ToUpper(parameterArray[1].charArray[0]);

        switch(ch)
        {
#ifndef PRODUCTION_TEST_BUILD
            case 'R':    //enter remote mde
                nextState = (eStateDuci_t)E_STATE_DUCI_REMOTE;
                break;
#endif

            case 'L':    //already in this mode so stay here - do nothing
                break;

            default:
                duciError.invalid_args = 1u;
                break;
        }
    }

    return duciError;
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateBluetoothIdle::createCommands(void)
{
    DCommsState::createCommands();
}


//TODO HSB Add BT0 command here (see Elvis)

