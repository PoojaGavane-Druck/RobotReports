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
#include <rtos.h>
#include <lib_def.h>
#include <stdint.h>
#include <stdlib.h>
MISRAC_ENABLE

#include "DParseSlave.h"
#include "DCommsStateUsbIdle.h"
// TEST - SCS - Removed
//#include "DuciSensorCommands.h"
#include "DPV624.h"
#include "Utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define MASTER_SLAVE_USB_COMMANDS_ARRAY_SIZE  26 //this is the maximum no of commands supported in DUCI master/slave mode (can be increased if more needed)

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
sDuciCommand_t duciSlaveUsbCommands[MASTER_SLAVE_USB_COMMANDS_ARRAY_SIZE]; //TODO HSB: This needs to be on a per-instance basis!!!! there are multiple instances
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateUsbIdle class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateUsbIdle::DCommsStateUsbIdle(DDeviceSerial *commsMedium, DTask *task)
    : DCommsStateDuci(commsMedium, task)
{
    OS_ERR os_error;

    myParser = new DParseSlave((void *)this, &duciSlaveUsbCommands[0], (size_t)MASTER_SLAVE_USB_COMMANDS_ARRAY_SIZE, &os_error);

    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE));

    if(!ok)
    {
#ifdef ASSERT_IMPLEMENTED
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
#endif

        PV624->handleError(E_ERROR_OS,
                           eSetError,
                           (uint32_t)os_error,
                           36u);
    }

    createCommands();

    commandTimeoutPeriod = 200u; //default time in (ms) to wait for a response to a DUCI command
}


/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateUsbIdle::createCommands(void)
{
    //create the common commands
    DCommsStateDuci::createCommands();

    //add those specific to this state instance

    myParser->addCommand("SN", "[i]=i",    "[i]?",             NULL,    fnGetSN,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);   //serial number
    myParser->addCommand("CM", "=i",    "?",             NULL,    fnGetCM,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);   //serial number
    myParser->addCommand("CI", "",      "[i][i]?",             NULL,    fnGetCI,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);
    myParser->addCommand("SD", "=d",    "?",             NULL,    fnGetSD,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE); //Set/get system date
    myParser->addCommand("ST", "=t",    "?",             NULL,    fnGetST,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE); //Set/get system time
    myParser->addCommand("PT", "i",     "?",             NULL,    fnGetPT,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);
    myParser->addCommand("SP", "=i",      "?",           NULL,    fnGetSP,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);
    myParser->addCommand("CN", "",       "[i]?",            NULL,    fnGetCN,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);
    myParser->addCommand("IZ", "[i],[=],[v]",  "[i]?",   NULL,    fnGetIZ,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);
    myParser->addCommand("CD", "[i]=d",        "[i]?",   NULL,    fnGetCD,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);
    // N
    myParser->addCommand("ND",  "[i]=d",    "[i]?", NULL,   fnGetND,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
}
/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for the local comms state (DUCI master)
 * @param   void
 * @retval  eStateDuci_t  Duci state
 */
eStateDuci_t DCommsStateUsbIdle::run(void)
{
    char *buffer;

    nextState = E_STATE_DUCI_LOCAL;
    //Entry

    PV624->setCommModeStatus(E_COMM_USB_INTERFACE, E_COMM_MODE_LOCAL);

    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;

    //DO
    clearRxBuffer();

    while(E_STATE_DUCI_LOCAL == nextState)
    {
        //sleep(500u);

        // sleep(50u);

        //listen for a command over USB comms
#ifdef TASK_HEALTH_MONITORING_IMPLEMENTED
        if(myTask != NULL)
        {
            PV624->keepAlive(myTask->getTaskId());
        }

#endif

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

    if(E_STATE_DUCI_DATA_DUMP == nextState)
    {
        PV624->setPrintEnable(true);
    }

    //Exit

    return nextState;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")

/**
 * @brief   DUCI call back function for KM Command ? Change Mode
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateUsbIdle::fnSetKM(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    bool retStatus = false;

    //only accepted KM message in this state is a command type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        switch(parameterArray[1].charArray[0])
        {
        case 'E':    //enter Eng mode

            retStatus = PV624->setAquisationMode(E_REQUEST_BASED_ACQ_MODE);

            if(true == retStatus)
            {
                PV624->setPrintEnable(false);
                nextState = (eStateDuci_t)E_STATE_DUCI_ENG_TEST;
            }

            else
            {

            }

            break;

        case 'D':    //enter Eng mode


            retStatus = PV624->setAquisationMode(E_CONTINIOUS_ACQ_MODE);

            if(true == retStatus)
            {
                //PV624->setPrintEnable(true);
                nextState = (eStateDuci_t)E_STATE_DUCI_DATA_DUMP;
            }

            else
            {

            }

            break;

        case 'R':    //enter remote mode
            nextState = (eStateDuci_t)E_STATE_DUCI_REMOTE;
            break;

        case 'S':    //enter service mode
            nextState = (eStateDuci_t)E_STATE_DUCI_PROD_TEST;
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
 * @brief   DUCI handler for KM Command – Get front panel keypad operating mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateUsbIdle::fnGetKM(sDuciParameter_t *parameterArray)
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