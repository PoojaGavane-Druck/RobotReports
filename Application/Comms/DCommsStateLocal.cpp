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

#include "DCommsStateDuci.h"
#include "DParseMasterSlave.h"
#include "DCommsStateLocal.h"
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
DCommsStateLocal::DCommsStateLocal(DDeviceSerial *commsMedium)
: DCommsStateDuci(commsMedium)
{
    OS_ERR os_error;

    myParser = new DParseMasterSlave((void *)this, &os_error);  //in local mode we don't know yet whether we will be master or slave

    createDuciCommands();
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateLocal::createDuciCommands(void)
{
    //create the common commands
    DCommsStateDuci::createCommands();

    //add those specific to this state instance
    myParser->addCommand("RI", "=s,s",  "",     fnSetRI,    NULL,       0xFFFFu);   //device identification
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
eCommOperationMode_t DCommsStateLocal::run(void)
{
    OS_ERR os_err;
    char *buffer;
    sInstrumentMode_t mask;
    mask.value = 0u;
//    mask.test = 1u;
    mask.remoteSerial = 1u;
//    mask.remoteUsb = 1u;

    //Entry
    PV624->userInterface->clearMode(mask);

    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;
    //DO
    nextOperationMode = E_COMMS_READ_OPERATION_MODE;

    while (nextState == E_COMMS_READ_OPERATION_MODE)
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
            if (query("#RI?:11", &buffer))
            {
                duciError = myParser->parse(buffer);

                errorStatusRegister.value |= duciError.value;

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


/* Static callback functions ----------------------------------------------------------------------------------------*/
sDuciError_t DCommsStateLocal::fnSetRI(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateLocal *myInstance = (DCommsStateLocal*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetRI(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/
sDuciError_t DCommsStateLocal::fnSetRI(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_REPLY)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //get first parameter, which should be "DKnnnn" where n is a digit 0-9
        char *str = parameterArray[1].charArray;
        size_t size = strlen(str);

        if (size != (size_t)6)
        {
            duciError.invalid_args = 1u;
        }
        else
        {
            //check that the start characters are "DK"
            if (strncmp(str, "DK", (size_t)2u) != 0)
            {
                duciError.invalid_args = 1u;
            }
            else
            {
                //skip over the two start characters
                str += 2;
                char *endptr;

                //expects exactly 4 digits after the "DK"
                duciError = myParser->getIntegerArg(str, (int32_t *)&externalDevice.dk, 4u, &endptr);

                //if first parameter is ok then check version parameter
                if (duciError.value == 0u)
                {
                    str = parameterArray[2].charArray;

                    size = strlen(str);

                    //expects exactly 9 characters: Vnn.nn.nn (where 'n' is a digit '0'-'9'
                    if (size != (size_t)9)
                    {
                        duciError.invalid_args = 1u;
                    }
                    else
                    {
                        //check that the next characters is 'V'
                        if (*str++ != 'V')
                        {
                            duciError.invalid_args = 1u;
                        }
                        else
                        {
                            int32_t intValue;

                            //expects exactly 2 digits next
                            duciError = myParser->getIntegerArg(str, &intValue, 2u, &endptr);

                            if (duciError.value == 0u)
                            {
                                externalDevice.version.major = (uint32_t)intValue;

                                str = endptr;

                                //check that the next characters is '.'
                                if (*str++ != '.')
                                {
                                    duciError.invalid_args = 1u;
                                }
                                else
                                {
                                    //expects exactly 2 digits next
                                    duciError = myParser->getIntegerArg(str, &intValue, 2u, &endptr);

                                    if (duciError.value == 0u)
                                    {
                                        externalDevice.version.minor = (uint32_t)intValue;

                                        str = endptr;

                                        //check that the next characters is '.'
                                        if (*str++ != '.')
                                        {
                                            duciError.invalid_args = 1u;
                                        }
                                        else
                                        {
                                            //expects exactly 2 digits next
                                            duciError = myParser->getIntegerArg(str, &intValue, 2u, &endptr);

                                            if (duciError.value == 0u)
                                            {
                                                externalDevice.version.build = (uint32_t)intValue;

                                                //all is well, so can go discover more about the device
                                                nextState = (eStateDuci_t)E_STATE_DUCI_DEVICE_DISCOVERY;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return duciError;
}

sDuciError_t DCommsStateLocal::fnGetKM(sDuciParameter_t * parameterArray)
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

sDuciError_t DCommsStateLocal::fnSetKM(sDuciParameter_t * parameterArray)
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
        switch(parameterArray[1].charArray[0])
        {
            case 'R':    //enter remote mde
                if(currentWriteMaster == (eCommMasterInterfaceType_t)E_COMMS_MASTER_NONE)
                {
                    nextOperationMode = E_COMMS_WRITE_OPERATION_MODE;
                    nextState = (eStateDuci_t)E_STATE_DUCI_REMOTE;
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



