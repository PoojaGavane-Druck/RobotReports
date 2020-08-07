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
* @file     DCommsStateRemote.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The communications remote state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDuci.h"
#include "DCommsStateRemote.h"
#include "DParseSlave.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
DCommsStateRemote *DCommsStateRemote::myInstance = NULL;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateRemote class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateRemote::DCommsStateRemote(DDeviceSerial *commsMedium)
: DCommsStateDuci(commsMedium)
{
    OS_ERR os_error;
    myParser = new DParseSlave((void *)this, &os_error);
    createCommands();
    commandTimeoutPeriod = 0u; //time in (ms) to wait for a response to a command (0 means wait forever)
}

/**
 * @brief   Get comms medium for this state
 * @param   void
 * @retval  commsMedium reference to comms medium
 */
DDeviceSerial *DCommsStateRemote::getCommsMedium(void)
{
    return myCommsMedium;
}

/**
 * @brief   Set comms medium for this state
 * @param   commsMedium reference to comms medium
 * @retval  flag - true if successful, false if already in use
 */
bool DCommsStateRemote::setCommsMedium(DDeviceSerial *commsMedium)
{
    bool flag = false;

    //setting to NULL always succeeds but any other value can only be accepted if currently NULL (ie free)
    //TODO: DO NOT NEED TO CHECK IF commsMedium is NULL because only exiting this state sets to NULL itself
    if ((commsMedium == NULL) || (myCommsMedium == NULL))
    {
        myCommsMedium = commsMedium;

        if (commsMedium != NULL)
        {
            myTxBuffer = myCommsMedium->getTxBuffer();
            myTxBufferSize = myCommsMedium->getTxBufferSize();
        }

        flag = true;
    }

    return flag;
}

/**
 * @brief   Create DUCI command set
 * @param   void
 * @return  void
 */
void DCommsStateRemote::createCommands(void)
{
    //create common commands - that apply to all states
    DCommsState::createCommands();

    //Create DUCI command set
    //TODO: make PIN mode be a mask eg bit 0 = cal, 1 = config, 2 = factory, 3 = prod test/service
    //TODO:  factor out those commands that are common to all into base class

    //then set true (1) if that mode PIN is required
    myParser->addCommand("AA", "=2i",          "",              NULL,       NULL,      0xFFFFu);
    myParser->addCommand("AE", "=4x",          "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("AK", "=b",           "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("CA", "",             "",              NULL,       NULL,      0xFFFFu);
    myParser->addCommand("CD", "[i]=d",        "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("CI", "=2i",          "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("CN", "",             "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("CP", "[i]=v",        "",              NULL,       NULL,      0xFFFFu);
    myParser->addCommand("CS", "",             "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("CT", "[i]=i,[i]",    "",              NULL,       NULL,      0xFFFFu);
    myParser->addCommand("CX", "",             "",              NULL,       NULL,      0xFFFFu);
    myParser->addCommand("DK", "",             "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("DV", "",             "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("FA", "=b",           "",              NULL,       NULL,      0xFFFFu);
    myParser->addCommand("FC", "=b",           "",              NULL,       NULL,      0xFFFFu);
    myParser->addCommand("IB", "",             "[i],[i]?",      NULL,       NULL,      0xFFFFu);
    myParser->addCommand("IP", "[i]=i,b",      "[i],[i]?",      NULL,       NULL,      0xFFFFu);
    myParser->addCommand("IR", "",             "[i],[i]?",      NULL,       NULL,      0xFFFFu);
    myParser->addCommand("IS", "",             "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("IV", "",             "[i],[i]?",      NULL,       NULL,      0xFFFFu);
    myParser->addCommand("IZ", "[i],[=],[v]",  "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("KP", "=i,[i]",       "?",             fnSetKP,    NULL,      0xFFFFu);
    myParser->addCommand("LE", "=i",           "i?",            NULL,       NULL,      0xFFFFu);
    myParser->addCommand("LV", "=i",           "i?",            NULL,       NULL,      0xFFFFu);
    myParser->addCommand("PM", "i=4x",         "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("PP", "=3i",          "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("RB", "",             "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("RI", "",              "?",            NULL,       fnGetRI,   0xFFFFu);
    myParser->addCommand("RV", "",             "i?",            NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SD", "=d",           "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SE", "[i]=i",        "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SF", "[i]=i,i",      "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SG", "",             "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SN", "=i",           "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SR", "=i",           "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SS", "=v",           "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("ST", "=t",           "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SU", "[i]=i,[i]",    "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("TM", "[=][s]",       "",              NULL,       NULL,      0xFFFFu);
    myParser->addCommand("TP", "i,[=][i]",     "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("UI", "",             "?",             NULL,       NULL,      0xFFFFu);
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for this class
 * @param   void
 * @retval  void
 */
eCommOperationMode_t DCommsStateRemote::run(void)
{
    OS_ERR os_err;
    char *buffer;

    //Entry
    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;

    //DO
    //nextOperationMode = E_STATE_DUCI_REMOTE;

    while (nextOperationMode == E_COMMS_WRITE_OPERATION_MODE)
    {
        OSTimeDlyHMSM(0u, 0u, 0u, 500u, OS_OPT_TIME_HMSM_STRICT, &os_err);

        clearRxBuffer();

        if (receiveString(&buffer))
        {
            duciError = myParser->parse(buffer);

            errorStatusRegister.value |= duciError.value;
        }
    }

    //Exit
    myCommsMedium = NULL; //mark the state as free

    return nextOperationMode;
}

//call back functions - each calls an instance method
sDuciError_t DCommsStateRemote::fnGetRI(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetRI(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateRemote::fnSetKP(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetKP(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/
sDuciError_t DCommsStateRemote::fnSetKP(sDuciParameter_t * parameterArray)
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
        uint32_t keyId = (uint32_t)parameterArray[1].intNumber;     //first parameter (after '=' sign) is key code
        uint32_t pressType = (uint32_t)parameterArray[2].intNumber; //second paramter is the press type (short or long)

        //check for validity
        if ((keyId < E_BUTTON_1) || (keyId > E_BUTTON_6) || (pressType > 1u))
        {
            duciError.invalid_args = 1u;
        }
        else
        {
            gpioButtons_t keycode;
            keycode.bytes = 0u;
            keycode.bit.remote = 1u;
            keycode.bit.LongPress = pressType;

            switch (keyId)
            {
                case 1:
                    keycode.bit.TopLeft = 1u;
                    break;

                case 2:
                    keycode.bit.TopMiddle = 1u;
                    break;

                case 3:
                    keycode.bit.TopRight = 1u;
                    break;

                case 4:
                    keycode.bit.BottomLeft = 1u;
                    break;

                case 5:
                    keycode.bit.BottomMiddle = 1u;
                    break;

                case 6:
                    keycode.bit.BottomRight = 1u;
                    break;

                default:
                    break;
            }

            PV624->keyHandler->sendKey(keycode);
        }
    }

    return duciError;
}

sDuciError_t DCommsStateRemote::fnGetKM(sDuciParameter_t * parameterArray)
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
        sendString("!KM=R");
    }

    return duciError;
}

sDuciError_t DCommsStateRemote::fnSetKM(sDuciParameter_t * parameterArray)
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
            case 'L':    //enter local mode
                nextState = (eStateDuci_t)E_STATE_DUCI_LOCAL;
                nextOperationMode = E_COMMS_READ_OPERATION_MODE;
                currentWriteMaster = (eCommMasterInterfaceType_t)E_COMMS_MASTER_NONE;
                break;

            case 'S':    //enter production test mode
//                nextState = (eStateDuci_t)E_STATE_DUCI_PROD_TEST;
                duciError.invalidMode = 1u;
                break;

            case 'R':    //already in this mode so stay here - do nothing
                break;

            default:
                duciError.invalid_args = 1u;
                break;
        }
    }

    return duciError;
}

sDuciError_t DCommsStateRemote::fnGetRI(sDuciParameter_t * parameterArray)
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
        sprintf(buffer, "!RI=DK0492,V%02d.%02d.%02d", 1, 0, 0); //TODO: Get version form the right place
        sendString(buffer);
    }

    return duciError;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")
