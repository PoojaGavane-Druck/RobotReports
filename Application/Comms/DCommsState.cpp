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
* @file     DCommsState.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The communications class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsState.h"
#include "Utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
sExternalDevice_t DCommsState::externalDevice = { 0 };
eStateComms_t DCommsState::commsOwnership = E_STATE_COMMS_OWNED;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsState class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsState::DCommsState(DDeviceSerial *commsMedium)
{
    myCommsMedium = commsMedium;

    if (commsMedium != NULL)
    {
        myTxBuffer = myCommsMedium->getTxBuffer();
        myTxBufferSize = myCommsMedium->getTxBufferSize();
    }

    commandTimeoutPeriod = 500u; //default time in (ms) to wait for a response to a DUCI command

    commsOwnership = E_STATE_COMMS_OWNED;
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsState::createDuciCommands(void)
{
    myParser->addCommand("KM", "=c",    "?",    fnSetKM,    fnGetKM,    0xFFFFu);   //UI (key) mode
    myParser->addCommand("RE", "",      "?",    NULL,       fnGetRE,    0xFFFFu);   //error status
    myParser->addCommand("SN", "=i",    "?",    fnSetSN,    fnGetSN,    0xFFFFu);   //serial number
}

void DCommsState::initialise(void)
{
}

/**
 * @brief   Suspend state machine
 * @param   void
 * @retval  void
 */
void DCommsState::suspend(void)
{
    commsOwnership = E_STATE_COMMS_REQUESTED;

    while (commsOwnership == (eStateComms_t)E_STATE_COMMS_REQUESTED)
    {
        //wait until request has been processed
        sleep(100u);
    }
}

/**
 * @brief   Resume state machine
 * @param   void
 * @retval  void
 */
void DCommsState::resume(void)
{
    commsOwnership = E_STATE_COMMS_OWNED;
}

eStateDuci_t DCommsState::run(void)
{
    return E_STATE_DUCI_LOCAL;
}

void DCommsState::cleanup(void)
{
}

void DCommsState::clearRxBuffer(void) //Temporarily overriden - all comms has own buffer which base class could clear
{
    if (myCommsMedium != NULL)
    {
        myCommsMedium->clearRxBuffer();
    }
}

///**********************************************************************************************************************
// * DISABLE MISRA C 2004 CHECK for Rule 10.3. Ignoring this - explicit conversion from 'signed int' to 'char' is safe
// **********************************************************************************************************************/
//_Pragma ("diag_suppress=Pm136")
//
////prepare message in txBuffer
//bool DCommsState::prepareMessage(char *str)
//{
//    bool successFlag = false;
//
//    uint32_t size = strlen(str);
//
//    if (size < (TX_BUFFER_SIZE - 6u))
//    {
//        int32_t checksum = 0;
//
//        //checksum only necessary if enabled
//        if (myParser->getChecksumEnabled() == true)
//        {
//            for (uint32_t i = 0u; i < size; i++)
//            {
//                txBuffer[i] = str[i];
//                checksum += (int32_t)str[i];
//            }
//
//            //checksum include the semi-colon
//            txBuffer[size++] = ':';
//            checksum += (int32_t)':';
//
//            checksum %= 100;
//
//            txBuffer[size++] = '0' + (char)(checksum / 10);
//            txBuffer[size++] = '0' + (char)(checksum % 10);
//        }
//        else
//        {
//            strncpy(txBuffer, str, size);
//        }
//
//        if (myParser->getTerminatorCrLf() == true)
//        {
//            txBuffer[size++] = '\r';    //CR sent only if enabled
//        }
//
//        txBuffer[size++] = '\n';         //always send LF
//        txBuffer[size] = '\0';           //always null terminate
//
//        successFlag = true;
//    }
//
//    return successFlag;
//}
//
///**********************************************************************************************************************
// * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
// **********************************************************************************************************************/
//_Pragma ("diag_default=Pm136")

bool DCommsState::sendString(char *str)  //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;

    if (myCommsMedium != NULL)
    {
        successFlag = myParser->prepareTxMessage(str, myTxBuffer, myTxBufferSize);

        if (successFlag == true)
        {
            successFlag = myCommsMedium->sendString(myTxBuffer);
        }
    }

    return successFlag;
}

bool DCommsState::query(char *str, char **pStr)
{
    bool successFlag = false;

    if (myCommsMedium != NULL)
    {
        successFlag = myParser->prepareTxMessage(str, myTxBuffer, myTxBufferSize);

        if (successFlag == true)
        {
            successFlag = myCommsMedium->query(myTxBuffer, pStr, commandTimeoutPeriod);
        }
    }

    return successFlag;
}

bool DCommsState::receiveString(char **pStr) //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;

    if (myCommsMedium != NULL)
    {
        successFlag = myCommsMedium->receiveString(pStr, commandTimeoutPeriod);
    }

    return successFlag;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")


/* Static callback functions ----------------------------------------------------------------------------------------*/
sDuciError_t DCommsState::fnGetKM(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsState *myInstance = (DCommsState*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetKM(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsState::fnSetKM(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsState *myInstance = (DCommsState*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetKM(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsState::fnGetRE(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsState *myInstance = (DCommsState*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetRE(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsState::fnGetSN(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsState *myInstance = (DCommsState*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetSN(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsState::fnSetSN(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsState *myInstance = (DCommsState*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetSN(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/
sDuciError_t DCommsState::fnGetKM(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    duciError.unhandledMessage = 1u;
    return duciError;
}

sDuciError_t DCommsState::fnSetKM(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    duciError.unhandledMessage = 1u;
    return duciError;
}

sDuciError_t DCommsState::fnGetRE(sDuciParameter_t * parameterArray)
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
        char buffer[32];
        sprintf(buffer, "!RE=%08X", errorStatusRegister.value);
        sendString(buffer);

        errorStatusRegister.value = 0u; //clear error status register as it has been read now
    }

    return duciError;
}

sDuciError_t DCommsState::fnGetSN(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    duciError.unhandledMessage = 1u;
    return duciError;
}

sDuciError_t DCommsState::fnSetSN(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    duciError.unhandledMessage = 1u;
    return duciError;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")


