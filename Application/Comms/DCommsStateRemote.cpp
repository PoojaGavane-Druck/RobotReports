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
* @file     DCommsStateRemote.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The communications remote state class source file
*/
//*********************************************************************************************************************
#define __STDC_WANT_LIB_EXT1__ 1
/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE

#include <stdio.h>
#include <stdlib.h>
MISRAC_ENABLE
#include "string.h"
#include "DCommsStateDuci.h"
#include "DCommsStateRemote.h"
#include "DParseSlave.h"
#include "DPV624.h"
#include "Utilities.h"
#include "crc.h"

/* Error handler instance parameter starts from 1701 to 1800 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/
/* Constants --------------------------------------------------------------------------------------------------------*/



/* Defines ----------------------------------------------------------------------------------------------------------*/
#define SLAVE_REMOTE_COMMANDS_ARRAY_SIZE  51  //this is the maximum no of commands supported in DUCI remot eslave mode (can be increased if more needed)
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
sDuciCommand_t duciSlaveRemoteCommands[SLAVE_REMOTE_COMMANDS_ARRAY_SIZE];
DCommsStateRemote *DCommsStateRemote::ptrMyInstance = NULL;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateRemote class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateRemote::DCommsStateRemote(DDeviceSerial *commsMedium, DTask *task)
    : DCommsStateDuci(commsMedium, task)
{
    OS_ERR os_error = OS_ERR_NONE;
    myParser = new DParseSlave((void *)this,  &duciSlaveRemoteCommands[0], (size_t)SLAVE_REMOTE_COMMANDS_ARRAY_SIZE, &os_error);

    createCommands();
    commandTimeoutPeriod = 250u; //time in (ms) to wait for a response to a command (0 means wait forever)
    shutdownTimeout = (shutdownTime / commandTimeoutPeriod) * TASKS_USING_SHUTDOWN_TIMEOUT;
    remoteModeTimeout = (2u * 60u * 1000u / commandTimeoutPeriod) ;     // in miliseconds - 2 mins * 60s/mins * 1000ms/s
    lastDownloadNo = 0;
    remoteModeTimeoutCount = 0u;

    if(myParser != NULL)
    {
        //myParser->setAckFunction(acknowledge);
    }
}

/**
 * @brief   DCommsStateRemote class destructor
 * @param   void
 * @retval  void
 */
DCommsStateRemote::~DCommsStateRemote(void)
{
    if(NULL != myParser)
    {
        delete myParser;
    }
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


void DCommsStateRemote::setMyTask(DTask *task)
{
    myTask = task;
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
    //do not need to check if commsMedium is NULL because exiting this state sets to NULL anyway
    if(myCommsMedium == NULL)
    {
        myCommsMedium = commsMedium;

        if(commsMedium != NULL)
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
    DCommsStateDuci::createCommands();

    //Create DUCI command set
    //TODO: make PIN mode be a mask eg bit 0 = cal, 1 = config, 2 = factory, 3 = prod test/service
    //TODO:  factor out those commands that are common to all into base class

    //then set true (1) if that mode PIN is required
    /* B */
    myParser->addCommand("BS", "=i",            "?",            fnSetBS,    fnGetBS,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("BT", "i=i,[i],[i],[i],[i]", "i?",     DCommsStateDuci::fnSetBT,   DCommsStateDuci::fnGetBT,   E_PIN_MODE_NONE, E_PIN_MODE_NONE); //Bluetooth test command
    myParser->addCommand("BD", "",            "?",              NULL,       fnGetBD,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* C */
    myParser->addCommand("CA", "",             "?",             fnSetCA,    fnGetCA,      E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CB", "=i",           "",              fnSetCB,    NULL,      E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CD", "[i]=d",      "[i]?",            fnSetCD,    fnGetCD,   E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CI", "[i]=i",     "[i]?",             fnSetCI,    fnGetCI,   E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CM", "=i",            "?",            fnSetCM,    fnGetCM,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);   //serial number

    myParser->addCommand("CP", "i=v",        "",                fnSetCP,    NULL,      E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CS", "",             "?",             fnSetCS,    fnGetCS,   E_PIN_MODE_CALIBRATION,   E_PIN_MODE_CALIBRATION);
    myParser->addCommand("CT", "[i]=i,[i]",    "[i]?",          fnSetCT,    fnGetCT,   E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CX", "",             "",              fnSetCX,    NULL,      E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    /* D */
    myParser->addCommand("DF", "=v",           "?",             fnSetDF,    fnGetDF,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* F */
    myParser->addCommand("FF", "",             "[i]?",          fnSetFF,    fnGetFF,   E_PIN_MODE_FACTORY,       E_PIN_MODE_NONE); //Configure External Flash Memory
    /* I */
    myParser->addCommand("IZ", "[i]=v",        "[i]?",          fnSetIZ,    fnGetIZ,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* K */
    myParser->addCommand("KP", "=i,[i]",       "?",             fnSetKP,    NULL,      E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* L */
    myParser->addCommand("LE", "=i",           "i?",            fnSetLE,    NULL,      E_PIN_MODE_ENGINEERING,   E_PIN_MODE_NONE);
    myParser->addCommand("LV", "=i",           "i?",            fnSetLV,    NULL,      E_PIN_MODE_ENGINEERING,   E_PIN_MODE_NONE);
    /* M */
    myParser->addCommand("ME", "i=s",           "",             fnSetME,    NULL,      E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //Memory Erase File #ME1=%s%s      // TODO: check same as Genii, for 4Sight
    myParser->addCommand("MF", "i=s,i,i,i,F",     "",           fnSetMF,    NULL,      E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //Download raw application image file
    /* N */
    myParser->addCommand("ND", "[i]=d",        "[i]?",          fnSetND,    fnGetND,   E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    /* O */
    myParser->addCommand("OE", "=i",            "?",            fnSetOE,    fnGetOE,   E_PIN_MODE_NONE,   E_PIN_MODE_NONE);
    /* P */
    myParser->addCommand("PP", "=3i",          "?",             fnSetPP,    DCommsStateDuci::fnGetPP,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("PT",  "=i",          "?",             fnSetPT,    fnGetPT,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* R */
    myParser->addCommand("RD", "=d",           "?",             fnSetRD,    fnGetRD,   E_PIN_MODE_FACTORY,       E_PIN_MODE_NONE);
    /* S */
    myParser->addCommand("SC", "[i]=i",        "[i]?",          fnSetSC,    fnGetSC,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("SD", "=d",            "?",            fnSetSD,    fnGetSD,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //Set/get system date
    myParser->addCommand("SN", "=i",            "?",          fnSetSN,    fnGetSN,   E_PIN_MODE_FACTORY,       E_PIN_MODE_NONE);   //serial number
    myParser->addCommand("ST", "=t",           "?",             fnSetST,    fnGetST,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //Set/get system time
    /* T */
    /* U */
    myParser->addCommand("UF", "",           "?",         fnSetUF,    fnGetUF,   E_PIN_MODE_UPGRADE,          E_PIN_MODE_NONE);
    myParser->addCommand("UT", "",           "?",         fnSetUT,    fnGetUT,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* V */
    myParser->addCommand("VP", "=v",           "?",             fnSetVP,    fnGetVP,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("VR", "=v",           "?",             fnSetVR,    fnGetVR,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for this class
 * @param   void
 * @retval  void
 */
eStateDuci_t DCommsStateRemote::run(void)
{

    char *buffer;
    eSysMode_t sysMode = E_SYS_MODE_NONE;

    //Entry
    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;
    sInstrumentMode_t commModeStatus;
    commModeStatus.value = 0u;

    //DO
    nextState = E_STATE_DUCI_REMOTE;


    while(E_STATE_DUCI_REMOTE == nextState)
    {
        commModeStatus = PV624->getCommModeStatus();

        if(myTask != NULL)
        {
            PV624->keepAlive(myTask->getTaskId());
        }

        // Check power state before running
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
                commsTimeout = 0u; // Reset the timeout as a command was received

                if(commModeStatus.remoteBluetooth)
                {
                    remoteModeTimeoutCount = 0u;
                }

                duciError = myParser->parse(buffer);

                errorStatusRegister.value |= duciError.value;
            }

            else
            {
                // Increment command timeout if no command
                // The timeout runs at 250ms
                // If total time reaches higher than 5 minutes, start the shutdown procedure
                commsTimeout = commsTimeout + 1u;


                if(commModeStatus.remoteBluetooth)
                {
                    remoteModeTimeoutCount = remoteModeTimeoutCount + 1u;

                    if(remoteModeTimeout < remoteModeTimeoutCount)
                    {
                        remoteModeTimeoutCount = 0u;
                        nextState = E_STATE_DUCI_LOCAL;
                        PV624->manageBlueToothConnection(BL_STATE_NO_COMMUNICATION);
                    }
                }

                if(shutdownTimeout < commsTimeout)
                {
                    // Initiate PV 624 shutdown
                    PV624->shutdown();
                }
            }
        }
    }

    //Exit
    myCommsMedium = NULL; //mark the state as free

    return nextState;
}


/**
 * @brief   DUCI call back function for KP Command - Key Press Emulation
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetKP(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetKP(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for KP Command - Key Press Emulation
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetKP(sDuciParameter_t *parameterArray)
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
        uint32_t keyId = (uint32_t)parameterArray[1].intNumber;     //first parameter (after '=' sign) is key code
        uint32_t pressType = (uint32_t)parameterArray[2].intNumber; //second paramter is the press type (short or long)

        //check for validity
        if((keyId < E_BUTTON_1) || (keyId > E_BUTTON_2) || (pressType > 1u))
        {
            duciError.invalid_args = 1u;
        }

        else
        {
            gpioButtons_t keycode;
            pressType_t keyPressType;
            keyPressType.bytes = 0;
            keycode.bytes = 0u;
            keycode.bit.remote = 1u;

            switch(keyId)
            {
            case 1:
                keycode.bit.powerOnOff = 1u;

                if(1u == pressType)
                {
                    keyPressType.bit.updateBattery = 1u;
                }

                else if(2u == pressType)
                {
                    keyPressType.bit.powerOnOff = 1u;
                }

                else
                {
                    /* Do Nothing */
                }

                break;

            case 2:
                keycode.bit.blueTooth = 1u;
                break;



            default:
                break;
            }

            PV624->keyHandler->sendKey(keycode, keyPressType);
        }
    }

    return duciError;
}

/**
 * @brief   handler for get KM command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnGetKM(sDuciParameter_t *parameterArray)
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
        sendString("!KM=R");
    }

    return duciError;
}

/**
 * @brief   handler for set KM command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetKM(sDuciParameter_t *parameterArray)
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

        case 'L':    //enter local mode
            nextState = (eStateDuci_t)E_STATE_DUCI_LOCAL;
            sInstrumentMode_t commModeStatus;
            commModeStatus.value = 0u;
            commModeStatus = PV624->getCommModeStatus();

            if(commModeStatus.remoteBluetooth)
            {
                PV624->errorHandler->handleError(E_ERROR_CODE_REMOTE_REQUEST_FROM_BT_MASTER,
                                                 eClearError,
                                                 0u,
                                                 1701u,
                                                 false);
            }

            if(commModeStatus.remoteOwi)
            {
                PV624->errorHandler->handleError(E_ERROR_CODE_REMOTE_REQUEST_FROM_OWI_MASTER,
                                                 eClearError,
                                                 0u,
                                                 1702u,
                                                 false);
            }

            else
            {
                /* do nothing*/
            }

            break;

        case 'S':    //enter production test mode
            PV624->ventSystem();
            PV624->setSysMode(E_SYS_MODE_PRODUCTION_TEST);
            sleep(2000u);
            nextState = (eStateDuci_t)E_STATE_DUCI_PROD_TEST;
            // duciError.invalidMode = 1u;
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

/**
* @brief    DUCI call back function for command PT --- Set Pressure type
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/

sDuciError_t DCommsStateRemote::fnSetPT(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetPT(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command PT - Set pressure type
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetPT(sDuciParameter_t *parameterArray)
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
        eFunction_t func = (eFunction_t)parameterArray[1].uintNumber;

        if(func < (eFunction_t)E_FUNCTION_MAX)
        {
            PV624->instrument->resetDisplayFilter();
            PV624->instrument->setFunction(func);
        }

        else
        {
            duciError.invalid_args = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for ST Command ? Set time
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetST(void *instance, sDuciParameter_t *parameterArray)   //* @note   =t",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetST(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for ST Command ? Set time
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetST(sDuciParameter_t *parameterArray)
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
        sTime_t rtcTime;

        rtcTime.hours = parameterArray[1].time.hours;
        rtcTime.minutes = parameterArray[1].time.minutes;
        rtcTime.seconds = parameterArray[1].time.seconds;

        //set RTC time
        if(PV624->setTime(&rtcTime) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for RD Command ? Set date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetRD(void *instance, sDuciParameter_t *parameterArray)   //* @note   =d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetRD(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for SD Command ? Set date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetRD(sDuciParameter_t *parameterArray)
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
        sDate_t date;

        date.day = parameterArray[1].date.day;
        date.month = parameterArray[1].date.month;
        date.year = parameterArray[1].date.year;

        //set RTC date
        if(PV624->setManufactureDate(&date) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SD Command ? Set date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetSD(void *instance, sDuciParameter_t *parameterArray)   //* @note   =d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetSD(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for SD Command ? Set date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetSD(sDuciParameter_t *parameterArray)
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
        sDate_t date;

        date.day = parameterArray[1].date.day;
        date.month = parameterArray[1].date.month;
        date.year = parameterArray[1].date.year;

        //set RTC date
        if(PV624->setDate(&date) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
* @brief    DUCI call back function for command SN ---  Set instrument serial number
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateRemote::fnSetSN(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetSN(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   handler for set SN command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetSN(sDuciParameter_t *parameterArray)
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
        if(PV624->setSerialNumber(parameterArray[1].uintNumber) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
* @brief    DUCI call back function for command CM --- change controller mode
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateRemote::fnSetCM(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCM(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   handler for CM command --- Set Controller mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCM(sDuciParameter_t *parameterArray)
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
        if(PV624->setControllerMode((eControllerMode_t)parameterArray[1].uintNumber) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}


/**
 * @brief   DUCI call back function for command CI - Set Cal Interval
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCI(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCI(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/*
 * @brief   Handle cal interval reply for this sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DCommsStateRemote::fnSetCI(sDuciParameter_t *parameterArray)
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
        //save cal interval
        if(false == PV624->setCalInterval((uint32_t)parameterArray[0].intNumber, (uint32_t)parameterArray[2].intNumber))
        {

            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SP Command ? Set controller set point
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetVP(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetVP(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   handler for set SP command --- set controller pressure set point
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetVP(sDuciParameter_t *parameterArray)
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
        if(PV624->setPressureSetPoint((float32_t)parameterArray[1].floatValue) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SP Command ? Set controller set point
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetVR(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetVR(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   handler for set SP command --- set controller pressure set point
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetVR(sDuciParameter_t *parameterArray)
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
        if(PV624->setVentRate((float32_t)parameterArray[1].floatValue) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SP Command ? Set controller set point
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetDF(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetDF(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   handler for set SP command --- set controller pressure set point
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetDF(sDuciParameter_t *parameterArray)
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
        if(PV624->setFilterCoeff((float32_t)parameterArray[1].floatValue) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CT - Set Calibration Type
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCT(void *instance, sDuciParameter_t *parameterArray)   //* @note   [i]=i,[i]",    "",              NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCT(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for command CT - Set Calibration Type
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCT(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a command type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //command format is <int><=><int><int>
        //validate the parameters

        //only accepting user calibration, so this parameter must always be 0
        if(parameterArray[2].intNumber == 0)
        {
            //set cal type 0 (user) for specified channel with the third parameter being the range
            if(PV624->setCalibrationType(0, (uint32_t)parameterArray[3].intNumber) == false)
            {
                duciError.commandFailed = 1u;
            }
        }

        else
        {
            duciError.invalid_args = 1u;
        }


    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CS - Start sampling at cal point
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCS(void *instance, sDuciParameter_t *parameterArray)   //* @note   ",             "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCS(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CS - Start sampling at cal point
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCS(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a command type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //command has no parameters
        eFunction_t curfunc = (eFunction_t)E_FUNCTION_NONE;

        //get cal interval
        if(PV624->getFunction(&curfunc) == true)
        {
            if((eFunction_t)E_FUNCTION_BAROMETER == curfunc)
            {
                if(PV624->startCalSampling() == false)
                {
                    duciError.commandFailed = 1u;
                }
            }

            else
            {
                duciError.invalidMode = 1u;
            }
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}


/**
 * @brief   DUCI call back function for command CP - Set calibration point value
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCP(void *instance, sDuciParameter_t *parameterArray)   //* @note   [i]=v",        "",              NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCP(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CP - Set calibration point value
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCP(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a command type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        eFunction_t curfunc = (eFunction_t)E_FUNCTION_NONE;

        //get cal interval
        if(PV624->getFunction(&curfunc) == true)
        {
            if((eFunction_t)E_FUNCTION_BAROMETER == curfunc)
            {
                //command format is <int><=><float>
                //called functions validates the parameters itself but we know it has to be greater than 0

                uint32_t calPoint = (uint32_t)parameterArray[0].intNumber;

                if(calPoint > 0)
                {
                    //floating point value represents the user entered value
                    if(PV624->setCalPoint(calPoint, parameterArray[2].floatValue) == false)
                    {
                        duciError.commandFailed = 1u;
                    }
                }

                else
                {
                    duciError.invalid_args = 1u;
                }
            }

            else
            {
                duciError.invalidMode = 1u;
            }
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CA - Cal Accept
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCA(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCA(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CA - Cal Accept
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCA(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a command type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //command has no parameters
        if(PV624->acceptCalibration() == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}


/**
 * @brief   DUCI call back function for command CX - Abort calibration
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCX(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCX(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CX - Abort calibration
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCX(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a command type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //command has no parameters
        if(PV624->abortCalibration() == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CN - Get number of cal points required
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCN(void *instance, sDuciParameter_t *parameterArray)   //* @note   ",             "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCN(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CN - Get number of cal points required
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCN(sDuciParameter_t *parameterArray)
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

        if(PV624->setRequiredNumCalPoints(parameterArray[1].uintNumber) == false)
        {
            //we only have fixed no of cal points so always min = max number
            duciError.commandFailed = 1u;
        }

    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SC Command ? Set Instrument Port Configuration
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetSC(void *instance, sDuciParameter_t *parameterArray)   //* @note   =d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetSC(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for SC Command ? Set Instrument Port Configuration
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetSC(sDuciParameter_t *parameterArray)
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
        int32_t index = parameterArray[0].intNumber;
        int32_t mode = parameterArray[2].intNumber;

        if((index == 0) && ((mode == 0) || (mode == 1)))
        {
            PV624->setUsbInstrumentPortConfiguration(mode);
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command UF - Upgrade firmware
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetUF(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetUF(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for UF Command ? Upgrade PV624 firmware
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetUF(sDuciParameter_t *parameterArray)
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
        bool ok = false;

        ok = PV624->performUpgrade();

        if(!ok)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**

* @brief   DUCI call back function for command UT - Upgrade PM620T firmware
* @param   instance is a pointer to the FSM state instance
* @param   parameterArray is the array of received command parameters
* @retval  error status
*/
sDuciError_t DCommsStateRemote::fnSetUT(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetUT(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for UT Command ? Upgrade PM620 Terps firmware
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetUT(sDuciParameter_t *parameterArray)
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
        bool ok = false;

        ok = PV624->performPM620tUpgrade();

        if(!ok)
        {
            duciError.commandFailed = 1u;
        }

    }

    return duciError;
}

/**
 * @brief   DUCI call back function for IZ Command - Zero input reading
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetIZ(void *instance, sDuciParameter_t *parameterArray)   //* @note   [i],[=],[v]",  "[i]?",          NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetIZ(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for IZ Command - Zero input reading
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetIZ(sDuciParameter_t *parameterArray)
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
        if(0 == parameterArray[0].intNumber)
        {
            if(PV624->setZero(0u, parameterArray[2].floatValue) == false)
            {
                duciError.commandFailed = 1u;
            }
        }

        else
        {
            duciError.invalid_args = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CB - Backup/Restore cal data
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCB(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCB(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CB - Save/restore backup cal
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCB(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a command type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        switch(parameterArray[1].intNumber)
        {
        case 1: //backup current cal
            if(PV624->backupCalDataSave() == false)
            {
                duciError.commandFailed = 1u;
            }

            break;

        case 2: //restore cal from backup
            if(PV624->backupCalDataRestore() == false)
            {
                duciError.commandFailed = 1u;
            }

            break;

        default:
            duciError.invalid_args = 1u;
            break;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CD- Set Calibration Date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCD(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetCD(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for command CD- Set Calibration Date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCD(sDuciParameter_t *parameterArray)
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
        //command format is <int><=><date>
        //validate the parameters

        int32_t item = parameterArray[0].intNumber;
        sDate_t date;

        switch(item)
        {
        case E_BAROMETER_SENSOR:
            date.day = parameterArray[2].date.day;
            date.month = parameterArray[2].date.month;
            date.year = parameterArray[2].date.year;

            if(PV624->getBarometerCalStatus())
            {
                //set cal date
                if(false == PV624->setCalDate(&date))
                {
                    duciError.commandFailed = 1u;
                }
            }

            else
            {
                duciError.commandFailed = 1u;
            }

            break;

        default:
            duciError.invalid_args = 1u;
            break;
        }

    }

    return duciError;
}

/**
 * @brief   DUCI call back function for OE command - overshoot enable disable
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetOE(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetOE(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for overshoot enable / disabled command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetOE(sDuciParameter_t *parameterArray)
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
        if(false == PV624->setOvershootState(parameterArray[1].uintNumber))
        {
            duciError.invalidMode = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for PP Command ? Set PIN protection mode
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetPP(void *instance, sDuciParameter_t *parameterArray)   //* @note   =3i",          "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetPP(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for PP Command ? Set PIN protection mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetPP(sDuciParameter_t *parameterArray)
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
        switch(parameterArray[1].intNumber)
        {
        case E_REMOTE_PIN_NONE:
            if(PV624->setPinMode(E_PIN_MODE_NONE) == false)
            {
                duciError.invalidMode = 1u;
            }

            break;

        case E_REMOTE_PIN_CALIBRATION:
            if(PV624->setPinMode(E_PIN_MODE_CALIBRATION) == false)
            {
                duciError.invalidMode = 1u;
            }

            break;

        case E_REMOTE_PIN_CONFIGURATION:
            if(PV624->setPinMode(E_PIN_MODE_CONFIGURATION) == false)
            {
                duciError.invalidMode = 1u;
            }

            break;

        case E_REMOTE_PIN_FACTORY:
            if(PV624->setPinMode(E_PIN_MODE_FACTORY) == false)
            {
                duciError.invalidMode = 1u;
            }

            break;

        case E_REMOTE_PIN_ENGINEERING:
            if(PV624->setPinMode(E_PIN_MODE_ENGINEERING) == false)
            {
                duciError.invalidMode = 1u;
            }

            break;

        case E_REMOTE_PIN_UPGRADE:
            if(PV624->setPinMode(E_PIN_MODE_UPGRADE) == false)
            {
                duciError.invalidMode = 1u;
            }

            break;

        default:
            duciError.invalid_args = 1u;
            break;
        }
    }

    return duciError;
}
/**
 * @brief   DUCI call back function for LE Command ? Clear error log
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetLE(void *instance, sDuciParameter_t *parameterArray)   //* @note   =i",           "i?",            NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetLE(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI call back function for LV Command ? Clear event log
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetLV(void *instance, sDuciParameter_t *parameterArray)   //* @note   =i",           "i?",            NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetLV(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for LE Command � Clear error log
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetLE(sDuciParameter_t *parameterArray)
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
        if(parameterArray[1].intNumber == 0)
        {
            bool statusFlag = PV624->clearErrorLog();

            if(false == statusFlag)
            {
                duciError.commandFailed = 1u;
            }
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}
/**
 * @brief   DUCI handler for LV Command - Clear event log
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetLV(sDuciParameter_t *parameterArray)
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
        if(parameterArray[1].intNumber == 0)
        {

            bool statusFlag = PV624->clearServiceLog();

            if(statusFlag)
            {
                statusFlag = PV624->clearMaintainceData();

                if(statusFlag)
                {
                    PV624->errorHandler->handleError(E_ERROR_DEVICE_DUE_FOR_SERVICE,
                                                     eClearError,
                                                     0u,
                                                     1703u,
                                                     false);
                }
            }


            if(false == statusFlag)
            {
                duciError.commandFailed = 1u;
            }
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command ND- Set Next Calibration Date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetND(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetND(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for command ND- Set next Calibration Date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetND(sDuciParameter_t *parameterArray)
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
        //command format is <int><=><date>
        //validate the parameters
        int32_t index = parameterArray[0].intNumber;
        sDate_t date;

        switch(index)
        {
        case E_BAROMETER_SENSOR:
            if(PV624->getBarometerCalStatus())
            {
                date.day = parameterArray[2].date.day;
                date.month = parameterArray[2].date.month;
                date.year = parameterArray[2].date.year;

                //set cal date
                if(PV624->setNextCalDate(&date) == false)
                {
                    duciError.commandFailed = 1u;
                }
            }

            else
            {
                duciError.commandFailed = 1u;
            }

            break;

        default:
            duciError.invalid_args = 1u;
            break;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for ME Command - Memory Erase File
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetME(void *instance, sDuciParameter_t *parameterArray)   //* @note   [i],[=],[v]",  "[i]?",          NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetME(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for ME Command - Memory Erase File
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetME(sDuciParameter_t *parameterArray)
{
    // Erase all files of name, regardless of extension
    // ME<area>=<directory><filename>
    // <area> the type of documenting file to erase, 1 = remote, 2 = local
    // <directory> the directory the file is located in
    // <filename> the name of the file to erase

    sDuciError_t duciError;
    duciError.value = 0u;
    bool eraseResult = false;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //validate the parameters - ME1 or ME2 is always used for downloads via the CommsServer
        if(FW_UPGRADE_OPTION == parameterArray[0].intNumber)
        {
            char *filename = parameterArray[2].charArray;
            char filePath [MAX_ITP_FILEPATH_LENGTH + 1] = {0};
            // erase root directory file
            // for use before #MF command to erase previous DK0492.raw file
            snprintf_s(filePath, (size_t)MAX_ITP_FILEPATH_LENGTH, "%s", filename);

            if(PV624->extStorage->exists(filePath))
            {
                eraseResult = PV624->extStorage->erase(filePath);

                if(!eraseResult)
                {
                    duciError.invalid_response = 1u;
                }
            }
        }

        else
        {
            duciError.invalid_args = 1u;
        }
    }

    return duciError;
}
/**
 * @brief   DUCI call back function for MF Command - Memory File
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetMF(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetMF(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for MF Command - Memory File
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetMF(sDuciParameter_t *parameterArray)
{
    // Command used to download a raw application file to the FS
    //
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        char *fileName = parameterArray[2].charArray;
        int32_t downloadNo = parameterArray[3].intNumber;
        // Note: CRC value is validated within the parser before the data is de-coded.
        uint32_t dataSize = parameterArray[5].intNumber;
        char *fileData = parameterArray[6].fileStringBuffer;

        char testFilePath [MAX_ITP_FILEPATH_LENGTH + 1] = {0};

        // validate the area parameter
        if(FW_UPGRADE_OPTION == parameterArray[0].intNumber)
        {
            snprintf_s(testFilePath, (size_t)MAX_ITP_FILEPATH_LENGTH, "%s", fileName);
        }

        else
        {
            duciError.invalid_args = 1u;
        }

        // check the received buffer size is valid
        if(dataSize > DUCI_FILE_STRING_LENGTH_LIMIT)
        {
            duciError.bufferSize = 1u;
        }

        else
        {
            if(duciError.value == 0u)
            {
                bool fileExists = false;

                // first line of file, check to see if a file with the same name already exists
                if(1 == downloadNo)
                {
                    fileExists = PV624->extStorage->exists(testFilePath);

                    if(fileExists)
                    {
                        duciError.invalidMode = 1u;
                    }

                    else
                    {
                        duciError = detectExtendedAsciiCharSet(testFilePath);

                        if(duciError.value == 0)
                        {
                            fileExists = PV624->extStorage->openFile(testFilePath, true);
                            lastDownloadNo = 0;
                        }
                    }
                }

                else
                {
                    fileExists = PV624->extStorage->openFile(testFilePath, true);

                    if(!fileExists)
                    {
                        duciError.commandFailed = 1u;
                    }
                }

                // check the file exists if it has been written to peviously
                if(fileExists && (duciError.value == 0))
                {
                    bool writeResult = false;

                    // check the downloadNo is the value expected
                    if(downloadNo == (lastDownloadNo + 1))
                    {
                        writeResult = PV624->extStorage->write(fileData, dataSize, dataSize);

                        if(!writeResult)
                        {
                            duciError.writeToFlash = 1u;
                        }

                        else
                        {
                            lastDownloadNo = downloadNo;
                        }

                    }

                    else
                    {
                        // line no invalid - something wrong in sequence
                        duciError.numberNotInSequence = 1u;
                    }
                }

                PV624->extStorage->close();
            }
        }

    }

    return duciError;
}
/**
* @brief    DUCI call back function for command DR ---  run diagnostics and get result
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateRemote::fnGetBD(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetBD(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief    DUCI call back function for command DR ---  run diagnostics and get result
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnGetBD(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    uint32_t diagResult = 0u;

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        diagResult = PV624->runDiagnostics();

        snprintf_s(myTxBuffer, 20u, "!BD=%d", diagResult);
        sendString(myTxBuffer);
    }

    return duciError;
}
/**
 * @brief   Local Function to detect the use of the extended ascii char set which is currently not supported for FS filenames
 * @param   const char *filePath the filename of the file to create
 * @retval  sDuciError_t error status
 */
sDuciError_t DCommsStateRemote::detectExtendedAsciiCharSet(const char *filePath)
{
    sDuciError_t duciError;
    duciError.value = 0;

    // search for un-supported characters in the filename
    // asciis extended char characters 128 to 255
    const char *invalidCharSet = "������������������������������������������������񲳴��������������������������������������������������������������������������";

    size_t valid_len = strcspn(filePath, invalidCharSet);

    if(valid_len != strlen(filePath))
    {
        duciError.unexpectedMessage = 1u;
    }

    return duciError;
}
/**
* @brief    Set Checksum Enabled
* @param    flag - true is checksum is used in message, else false
* @return   void
*/
void DCommsStateRemote::setChecksumEnabled(bool flag)
{
    myParser->setChecksumEnabled(flag);
}

/**
 * @brief   DUCI call back function for command FF - Configure External Flash Memory
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetFF(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetFF(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for FF Command - Configure External Flash Memory
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetFF(sDuciParameter_t *parameterArray)
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
        bool ok = PV624->configureExternalFlashMemory();

        if(!ok)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")
