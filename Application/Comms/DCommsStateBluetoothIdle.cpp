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
DCommsStateBluetoothIdle::DCommsStateBluetoothIdle(DDeviceSerial *commsMedium, DTask *task)
    : DCommsStateDuci(commsMedium, task)
{
    OS_ERR os_error = OS_ERR_NONE;


    myParser = new DParseSlave((void *)this, &duciSlaveBtCommands[0], (size_t)MASTER_SLAVE_BT_COMMANDS_ARRAY_SIZE, &os_error);
    handleOSError(&os_error);
    createCommands();

    commandTimeoutPeriod = 250u; //default time in (ms) to wait for a response to a DUCI command
    commsOwnership = E_STATE_COMMS_RELINQUISHED;
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for the local comms state (DUCI master)
 * @param   void
 * @retval  eStateDuci_t returns latest DUCI state
 */
eStateDuci_t DCommsStateBluetoothIdle::run(void)
{
    char *buffer;

    ePowerState_t powerState = E_POWER_STATE_OFF;

    nextState = E_STATE_DUCI_LOCAL;

    //Entry
    PV624->setCommModeStatus(E_COMM_BLUETOOTH_INTERFACE, E_COMM_MODE_LOCAL);



    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;

    //clear buffer before we start
    clearRxBuffer();

    //DO


    while(E_STATE_DUCI_LOCAL == nextState)
    {
#ifdef TASK_HEALTH_MONITORING_IMPLEMENTED

        if(myTask != NULL)
        {
            PV624->keepAlive(myTask->getTaskId());
        }

#endif
        powerState = PV624->getPowerState();

        if(E_POWER_STATE_OFF == powerState)
        {
            // Do nothing, but sleep and allow other tasks to run
            sleep(100u);
        }

        else
        {
            clearRxBuffer();

            //listen for a command over BT
            if(receiveString(&buffer))
            {
                duciError = myParser->parse(buffer);

                errorStatusRegister.value |= duciError.value;

                if(errorStatusRegister.value != 0u)
                {
                    //TODO: Handle Error
                }
            }
            else
            {
                /* ToDo for 5 min time out period */
            }
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

    while(commsOwnership == (eStateComms_t)E_STATE_COMMS_REQUESTED)
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
_Pragma("diag_default=Pm017,Pm128")

/* Static callback functions ----------------------------------------------------------------------------------------*/

/**
 * @brief   DUCI handler for KM Command � Get front panel keypad operating mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateBluetoothIdle::fnGetKM(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        sendString("!KM=L");
    }

    return duciError;
}

/**
 * @brief   DUCI handler for KM Command � Set front panel keypad operating mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateBluetoothIdle::fnSetKM(sDuciParameter_t *parameterArray)
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
        case 'E':
            PV624->commsUSB->setState(E_STATE_DUCI_ENG_TEST);
            bool retStatus = false;
            retStatus = PV624->setAquisationMode(E_REQUEST_BASED_ACQ_MODE);

            break;

        case 'R':    //enter remote mde
            nextState = (eStateDuci_t)E_STATE_DUCI_REMOTE;
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

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateBluetoothIdle::createCommands(void)
{
    DCommsState::createCommands();

    myParser->addCommand("CD",  "[i]=d",    "[i]?", NULL,   fnGetCD,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    myParser->addCommand("CI",  "",         "?",    NULL,   fnGetCI,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    myParser->addCommand("CM",  "=i",       "?",    NULL,   fnGetCM,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    myParser->addCommand("CN",  "",         "?",    NULL,   fnGetCN,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    // I
    myParser->addCommand("IZ", "[i],[=],[v]",  "[i]?",   NULL,       fnGetIZ,    E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    // N
    myParser->addCommand("ND",  "[i]=d",    "[i]?", NULL,   fnGetND,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    // P
    myParser->addCommand("PT", "=i",    "?",   NULL,    fnGetPT,      E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    // R
    myParser->addCommand("RD", "=d",    "?",   NULL,    fnGetRD,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    // S
    myParser->addCommand("SD", "=d",    "?",    NULL,    fnGetSD,   E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    myParser->addCommand("SN", "=i",    "?",    NULL,    fnGetSN,    E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    myParser->addCommand("SP", "",      "?",     NULL,   fnGetSP,    E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    myParser->addCommand("ST", "=t",    "?",    NULL,    fnGetST,   E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    //U
    myParser->addCommand("UF", "=i",           "?",     NULL,    fnGetUF,       E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
}


//TODO HSB Add BT0 command here (see Elvis)

