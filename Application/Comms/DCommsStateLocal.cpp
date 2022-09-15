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
* @file     DCommsStateLocal.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The communications local state class source file
*/
//*********************************************************************************************************************
#define __STDC_WANT_LIB_EXT1__ 1
/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <assert.h>
#include <rtos.h>
#include <lib_def.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
MISRAC_ENABLE

#include "DCommsStateDuci.h"
#include "DParseSlave.h"
#include "DCommsStateLocal.h"
//#include "DuciSensorCommands.h"
#include "DPV624.h"
#include "Utilities.h"

/* Error handler instance parameter starts from 6501 to 6600 */


/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define MASTER_SLAVE_LOCAL_COMMANDS_ARRAY_SIZE  35  //this is the maximum no of commands supported in DUCI master/slave mode (can be increased if more needed)
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
sDuciCommand_t duciSlaveLocalCommands[MASTER_SLAVE_LOCAL_COMMANDS_ARRAY_SIZE];
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateLocal class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateLocal::DCommsStateLocal(DDeviceSerial *commsMedium, DTask *task)
    : DCommsStateDuci(commsMedium, task)
{
    OS_ERR os_error = OS_ERR_NONE;

    //in local mode we don't know yet whether we will be master or slave
    myParser = new DParseSlave((void *)this,
                               &duciSlaveLocalCommands[0],
                               (size_t)MASTER_SLAVE_LOCAL_COMMANDS_ARRAY_SIZE,
                               &os_error);

    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE));

    if(!ok)
    {
#ifdef ASSERT_ENABLED
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
#endif
        PV624->handleError(E_ERROR_OS,
                           eSetError,
                           (uint32_t)os_error,
                           6501u);
    }

    shutdownTimeout = shutdownTime / commandTimeoutPeriod;
    remoteRequestTimeOut = 0u;
    createCommands();
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateLocal::createCommands(void)
{
    //create the common commands
    DCommsStateDuci::createCommands();

    //add those specific to this state instance

    /* C */
    myParser->addCommand("CA", "",          "?",    NULL,   fnGetCA,    E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CD",  "[i]=d",    "[i]?", NULL,   fnGetCD,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    myParser->addCommand("CI",  "",         "[i][i]?",    NULL,   fnGetCI,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    myParser->addCommand("CM",  "=i",       "?",    NULL,   fnGetCM,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    myParser->addCommand("CN",  "",         "[i]?",    NULL,   fnGetCN,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    /* I */
    myParser->addCommand("IZ", "[i],[=],[v]",  "[i]?",   NULL,       fnGetIZ,    E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    /* N */
    myParser->addCommand("ND",  "[i]=d",    "[i]?", NULL,   fnGetND,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    /* P */
    myParser->addCommand("PP", "=3i",          "?", NULL,    fnGetPP,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("PT", "=i",    "?",   NULL,    fnGetPT,      E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    /* R  */
    myParser->addCommand("RD", "=d",    "?",   NULL,    fnGetRD,    E_PIN_MODE_NONE,    E_PIN_MODE_NONE);
    /* S */
    myParser->addCommand("SC", "[i]=i",        "[i]?",          NULL,    fnGetSC,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("SD", "=d",    "?",    NULL,    fnGetSD,   E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    myParser->addCommand("SN", "[i]=i",    "[i]?",    NULL,    fnGetSN,    E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    myParser->addCommand("SP", "",      "?",     NULL,   fnGetSP,    E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    myParser->addCommand("ST", "=t",    "?",    NULL,    fnGetST,   E_PIN_MODE_NONE, E_PIN_MODE_NONE);
    myParser->addCommand("UF", "[i]",           "[i]?",     NULL,    fnGetUF,       E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* V */
    myParser->addCommand("VR", "=v",           "?",             NULL,    fnGetVR,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
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
eStateDuci_t DCommsStateLocal::run(void)
{

    char *buffer;
    uint32_t commandTimeout = 0u;
    eSysMode_t sysMode = E_SYS_MODE_NONE;

    PV624->setCommModeStatus(E_COMM_OWI_INTERFACE, E_COMM_MODE_LOCAL);

    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;
    //DO
    nextState = E_STATE_DUCI_LOCAL;

    while(E_STATE_DUCI_LOCAL == nextState)
    {
        if(myTask != NULL)
        {
            PV624->keepAlive(myTask->getTaskId());
        }

        sysMode = PV624->getSysMode();

        if(E_SYS_MODE_RUN != sysMode)
        {
            // Do nothing, but sleep and allow other tasks to run
            sleep(100u);
        }

        else
        {
            clearRxBuffer();

            if(receiveString(&buffer))
            {
                commandTimeout = 0u;
                duciError = myParser->parse(buffer);

                errorStatusRegister.value |= duciError.value;

                if(errorStatusRegister.value != 0u)
                {
                    //TODO: Handle Error
                }
                else
                {

                    if(remoteRequestTimeOut)
                    {
                        remoteRequestTimeOut--;

                        if(!remoteRequestTimeOut)
                        {
                            PV624->errorHandler->handleError(E_ERROR_CODE_REMOTE_REQUEST_FROM_OWI_MASTER,
                                                             eClearError,
                                                             0u,
                                                             6502u,
                                                             false);
                        }
                    }
                }

            }

            else
            {
                // Increment command timeout if no command
                // The timeout runs at 250ms
                // If total time reaches higher than 5 minutes, start the shutdown procedure
                commandTimeout = commandTimeout + 1u;

                if(shutdownTimeout < commandTimeout)
                {
                    // Initiate PV 624 shutdown
                    PV624->shutdown();
                }
            }
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


/**
* @brief    DUCI call back function for command RI ---  set instrument ID
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateLocal::fnSetRI(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateLocal *myInstance = (DCommsStateLocal *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetRI(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   handler for set RI command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */sDuciError_t DCommsStateLocal::fnSetRI(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_REPLY)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //get first parameter, which should be "DKnnnn" where n is a digit 0-9
        char *str = parameterArray[1].charArray;
        size_t size = strnlen_s(str, sizeof(parameterArray[1].charArray));

        if(size != (size_t)6)
        {
            duciError.invalid_args = 1u;
        }

        else
        {
            //check that the start characters are "DK"
            if(strncmp(str, "DK", (size_t)2u) != 0)
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
                if(duciError.value == 0u)
                {
                    str = parameterArray[2].charArray;

                    size = strnlen_s(str, sizeof(parameterArray[2].charArray));

                    //expects exactly 9 characters: Vnn.nn.nn (where 'n' is a digit '0'-'9'
                    if(size != (size_t)9)
                    {
                        duciError.invalid_args = 1u;
                    }

                    else
                    {
                        //check that the next characters is 'V'
                        if(*str++ != 'V')
                        {
                            duciError.invalid_args = 1u;
                        }

                        else
                        {
                            int32_t intValue;

                            //expects exactly 2 digits next
                            duciError = myParser->getIntegerArg(str, &intValue, 2u, &endptr);

                            if(duciError.value == 0u)
                            {
                                externalDevice.version.major = (uint32_t)intValue;

                                str = endptr;

                                //check that the next characters is '.'
                                if(*str++ != '.')
                                {
                                    duciError.invalid_args = 1u;
                                }

                                else
                                {
                                    //expects exactly 2 digits next
                                    duciError = myParser->getIntegerArg(str, &intValue, 2u, &endptr);

                                    if(duciError.value == 0u)
                                    {
                                        externalDevice.version.minor = (uint32_t)intValue;

                                        str = endptr;

                                        //check that the next characters is '.'
                                        if(*str++ != '.')
                                        {
                                            duciError.invalid_args = 1u;
                                        }

                                        else
                                        {
                                            //expects exactly 2 digits next
                                            duciError = myParser->getIntegerArg(str, &intValue, 2u, &endptr);

                                            if(duciError.value == 0u)
                                            {
                                                externalDevice.version.build = (uint32_t)intValue;

                                                //all is well, so can go discover more about the device
                                                //nextState = (eStateDuci_t)E_STATE_DUCI_DEVICE_DISCOVERY;
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

/**
 * @brief   DUCI handler for KM Command – Get front panel keypad operating mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateLocal::fnGetKM(sDuciParameter_t *parameterArray)
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
 * @brief   DUCI handler for KM Command – Set front panel keypad operating mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateLocal::fnSetKM(sDuciParameter_t *parameterArray)
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
        case 'E':
            PV624->commsUSB->setState(E_STATE_DUCI_ENG_TEST);
            retStatus = PV624->setAquisationMode(E_REQUEST_BASED_ACQ_MODE);

            if(false == retStatus)
            {
                duciError.commandFailed = 1u;
            }

            break;

        case 'R':    //enter remote mde
            sInstrumentMode_t commModeStatus;
            commModeStatus.value = 0u;
            commModeStatus = PV624->getCommModeStatus();

            if(commModeStatus.remoteBluetooth)
            {
                PV624->errorHandler->handleError(E_ERROR_CODE_REMOTE_REQUEST_FROM_OWI_MASTER,
                                                 eSetError,
                                                 0u,
                                                 6503u,
                                                 false);
            }

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



