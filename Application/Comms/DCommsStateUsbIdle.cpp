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

#include "DParseSlave.h"
#include "DCommsStateUsbIdle.h"
// TEST - SCS - Removed
//#include "DuciSensorCommands.h"
#include "DPV624.h"
#include "Utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define MASTER_SLAVE_USB_COMMANDS_ARRAY_SIZE  23 //this is the maximum no of commands supported in DUCI master/slave mode (can be increased if more needed)

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
DCommsStateUsbIdle::DCommsStateUsbIdle(DDeviceSerial *commsMedium, DTask* task)
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
                           (uint16_t)36);
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
   
    myParser->addCommand("SN", "=i",    "?",              NULL,    fnGetSN,    E_PIN_MODE_NONE,         E_PIN_MODE_NONE);   //serial number
    myParser->addCommand("CM", "=i",    "?",              NULL,    fnGetCM,    E_PIN_MODE_NONE,         E_PIN_MODE_NONE);   //serial number
    myParser->addCommand("CI", "",      "?",              NULL,    fnGetCI,    E_PIN_MODE_NONE,         E_PIN_MODE_NONE);
    myParser->addCommand("SD", "=d",    "?",              NULL,    fnGetSD,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //Set/get system date
    myParser->addCommand("ST", "=t",    "?",              NULL,    fnGetST,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //Set/get system time
    myParser->addCommand("PT", "i",     "?",              NULL,    fnGetPT,      E_PIN_MODE_NONE,       E_PIN_MODE_NONE);
    myParser->addCommand("SP", "=i",      "?",           NULL,    fnGetSP,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);    
    myParser->addCommand("CN", "",       "?",            NULL,    fnGetCN,   E_PIN_MODE_NONE,       E_PIN_MODE_NONE);  
    myParser->addCommand("IZ", "[i],[=],[v]",  "[i]?",   NULL,    fnGetIZ,    E_PIN_MODE_NONE,      E_PIN_MODE_NONE);
    myParser->addCommand("CD", "[i]=d",        "[i]?",   NULL,    fnGetCD,      E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
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
eStateDuci_t DCommsStateUsbIdle::run(void)
{
    char *buffer;

    nextState = E_STATE_DUCI_LOCAL;
    //Entry
#ifdef USER_INTERFACE_ENABLED    
    sInstrumentMode_t mask;
    mask.value = 0u;
    mask.remoteUsb = 1u;

    //can't be in USB remote mode, so clear that bit

    PV624->userInterface->clearMode(mask);
#endif
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

    return nextState;
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
            nextState = (eStateDuci_t)E_STATE_DUCI_REMOTE;
#if 0
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
#endif
         break; 
         case 'S':    //enter service mode
           nextState = (eStateDuci_t)E_STATE_DUCI_PROD_TEST;
#if 0
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
#endif
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
sDuciError_t DCommsStateUsbIdle::fnGetKM(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        sendString("!KM=L");
    }

    return duciError;
}