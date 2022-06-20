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

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDuci.h"
#include "DCommsStateRemote.h"
#include "DParseSlave.h"
#include "DPV624.h"
#include "Utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/
/* Constants --------------------------------------------------------------------------------------------------------*/
const uint32_t E_REMOTE_PIN_NONE = 0u;              //remote PIN value for unprotected mode
const uint32_t E_REMOTE_PIN_CALIBRATION = 123u;     //remote PIN for calibration mode
const uint32_t E_REMOTE_PIN_CONFIGURATION = 777u;   //remote PIN for config mode
const uint32_t E_REMOTE_PIN_FACTORY = 800u;         //remote PIN for factory mode
const uint32_t E_REMOTE_PIN_ENGINEERING = 187u;     //remote PIN for engineering/diagnostics mode
const uint32_t E_REMOTE_PIN_UPGRADE = 548u;         //remote PIN for firmware upgrade mode

const uint32_t shutdownTime = 5u * 60u * 1000u;     // in miliseconds - 5 mins * 60s/mins * 1000ms/s
/* Defines ----------------------------------------------------------------------------------------------------------*/
#define SLAVE_REMOTE_COMMANDS_ARRAY_SIZE  96  //this is the maximum no of commands supported in DUCI remot eslave mode (can be increased if more needed)
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
    shutdownTimeout = shutdownTime / commandTimeoutPeriod;

    if(myParser != NULL)
    {
        //myParser->setAckFunction(acknowledge);
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
    myParser->addCommand("BT", "i=i,[i],[i],[i],[i]", "i?",    DCommsStateDuci::fnSetBT,   DCommsStateDuci::fnGetBT,   E_PIN_MODE_NONE, E_PIN_MODE_NONE); //Bluetooth test command
    /* C */
    myParser->addCommand("CA", "",             "",              fnSetCA,    NULL,      E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CB", "=i",           "",              fnSetCB,    NULL,      E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CD", "[i]=d",        "[i]?",          fnSetCD,    fnGetCD,   E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CI", "[i][i]=i",     "[i][i]?",           fnSetCI,    fnGetCI,   E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CM", "=i",            "?",            fnSetCM,    fnGetCM,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);   //serial number
    myParser->addCommand("CN", "=i",            "[i]?",            NULL,    fnGetCN,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("CP", "[i][i]=v",        "",              fnSetCP,    NULL,      E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CS", "",             "?",             fnSetCS,    fnGetCS,   E_PIN_MODE_CALIBRATION,   E_PIN_MODE_CALIBRATION);
    myParser->addCommand("CT", "[i]=i,[i]",    "",              fnSetCT,    NULL,      E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    myParser->addCommand("CX", "",             "",              fnSetCX,    NULL,      E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    /* D */
    myParser->addCommand("DK", "",             "[i]?",          NULL,       NULL,      E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* I */
    myParser->addCommand("IZ", "[i]=v",        "[i]?",          fnSetIZ,    fnGetIZ,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* K */
    myParser->addCommand("KP", "=i,[i]",       "?",             fnSetKP,    NULL,      E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* L */
    myParser->addCommand("LE", "=i",           "i?",            fnSetLE,    NULL,      E_PIN_MODE_ENGINEERING,   E_PIN_MODE_NONE);
    myParser->addCommand("LV", "=i",           "i?",            fnSetLV,    NULL,      E_PIN_MODE_ENGINEERING,   E_PIN_MODE_NONE);
    /* N */
    myParser->addCommand("ND", "[i]=d",        "[i]?",          fnSetND,    fnGetND,   E_PIN_MODE_CALIBRATION,   E_PIN_MODE_NONE);
    /* P */
    myParser->addCommand("PP", "=3i",          "?",             fnSetPP,    fnGetPP,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("PT",  "=i",          "?",             fnSetPT,    fnGetPT,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* R */
    myParser->addCommand("RB", "",             "?",             NULL,       NULL,      E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("RV", "",             "i?",            NULL,       NULL,      E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("RD", "=d",           "?",             fnSetRD,    fnGetRD,   E_PIN_MODE_FACTORY,       E_PIN_MODE_NONE);
    /* S */
    myParser->addCommand("SC", "[i]=i",        "[i]?",          fnSetSC,    fnGetSC,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("SD", "=d",            "?",            fnSetSD,    fnGetSD,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //Set/get system date
    myParser->addCommand("SE", "[i]=i",        "[i]?",          NULL,       NULL,      E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("SN", "[i]=i",        "[i]?",          fnSetSN,    fnGetSN,   E_PIN_MODE_FACTORY,       E_PIN_MODE_NONE);   //serial number
    myParser->addCommand("SP", "=v",           "?",             fnSetSP,    fnGetSP,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("ST", "=t",           "?",             fnSetST,    fnGetST,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //Set/get system time
    /* T */
    myParser->addCommand("TP", "i,[=][i]",     "[i]?",          NULL,       NULL,      E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* U */
    myParser->addCommand("UF", "[i]",           "[i]?",             fnSetUF,    fnGetUF,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    /* V */
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
    ePowerState_t powerState = E_POWER_STATE_OFF;
    uint32_t commandTimeout = 0u;

    //Entry
    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;

    //DO
    nextState = E_STATE_DUCI_REMOTE;


    while(E_STATE_DUCI_REMOTE == nextState)
    {
#ifdef TASK_HEALTH_MONITORING_IMPLEMENTED

        if(myTask != NULL)
        {
            PV624->keepAlive(myTask->getTaskId());
        }

#endif

        // Check power state before running
        powerState = PV624->getPowerState();

        if(E_POWER_STATE_OFF == powerState)
        {
            // Do nothing, but sleep and allow other tasks to run
            sleep(100u);
        }

        else
        {
            clearRxBuffer();

            if(receiveString(&buffer))
            {
                commandTimeout = 0u; // Reset the timeout as a command was received
                duciError = myParser->parse(buffer);

                errorStatusRegister.value |= duciError.value;
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
                    //PV624->shutdown();
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
                                                 130u,
                                                 false);
            }

            if(commModeStatus.remoteOwi)
            {
                PV624->errorHandler->handleError(E_ERROR_CODE_REMOTE_REQUEST_FROM_OWI_MASTER,
                                                 eClearError,
                                                 0u,
                                                 140u,
                                                 false);
            }

            else
            {
                /* do nothing*/
            }

            break;

        case 'S':    //enter production test mode
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
 * @brief   DUCI call back function for ST Command – Set time
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
 * @brief   DUCI handler for ST Command – Set time
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
 * @brief   DUCI call back function for RD Command – Set date
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
 * @brief   DUCI handler for SD Command – Set date
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
 * @brief   DUCI call back function for SD Command – Set date
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
 * @brief   DUCI handler for SD Command – Set date
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
        int32_t index = parameterArray[0].intNumber;

        if(0 == index)
        {
            if(PV624->setSerialNumber(parameterArray[2].uintNumber) == false)
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
        if(0u == (uint32_t)parameterArray[0].intNumber)
        {
            //save cal interval
            if(false == PV624->setCalInterval((uint32_t)parameterArray[1].intNumber, (uint32_t)parameterArray[3].intNumber))
            {

                duciError.commandFailed = 1u;
            }
        }

        else
        {
            duciError.invalid_args = 0u;
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
sDuciError_t DCommsStateRemote::fnSetSP(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetSP(parameterArray);
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
sDuciError_t DCommsStateRemote::fnSetSP(sDuciParameter_t *parameterArray)
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
        if(PV624->startCalSampling() == false)
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
        if(0 == parameterArray[0].intNumber)
        {
            //command format is <int><=><float>
            //called functions validates the parameters itself but we know it has to be greater than 0

            uint32_t calPoint = (uint32_t)parameterArray[1].intNumber;

            if(calPoint > 0)
            {
                //floating point value represents the user entered value
                if(PV624->setCalPoint(calPoint, parameterArray[3].floatValue) == false)
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
            duciError.invalid_args = 1u;
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
 * @brief   DUCI call back function for SC Command – Set Instrument Port Configuration
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
 * @brief   DUCI handler for SC Command – Set Instrument Port Configuration
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
 * @brief   DUCI handler for UF Command – Upgrade firmware
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

        if(UPGRADE_PV624_FIRMWARE == parameterArray[0].uintNumber)
        {
            ok = PV624->performUpgrade();

            if(!ok)
            {
                duciError.commandFailed = 1u;
            }
        }

        else if(UPGRADE_PM620_FIRMWARE == parameterArray[0].uintNumber)
        {
            ok = PV624->performPM620tUpgrade();

            if(!ok)
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
            duciError.invalid_args = 0u;
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
        int32_t index = parameterArray[0].intNumber;
        sDate_t date;

        switch(index)
        {
        case 1:
            date.day = parameterArray[2].date.day;
            date.month = parameterArray[2].date.month;
            date.year = parameterArray[2].date.year;

            //set cal date
            if(PV624->setCalDate(&date) == false)
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
 * @brief   DUCI call back function for PP Command – Get current PIN protection mode
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnGetPP(void *instance, sDuciParameter_t *parameterArray)   //* @note   =3i",          "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetPP(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for PP Command – Get current PIN protection mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnGetPP(sDuciParameter_t *parameterArray)
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
        uint32_t pinValue = 0u;

        switch(PV624->getPinMode())
        {
        case E_PIN_MODE_NONE:
            pinValue = E_REMOTE_PIN_NONE;
            break;

        case E_PIN_MODE_CALIBRATION:
            pinValue = E_REMOTE_PIN_CALIBRATION;
            break;

        case E_PIN_MODE_CONFIGURATION:
            pinValue = E_REMOTE_PIN_CONFIGURATION;
            break;

        case E_PIN_MODE_FACTORY:
            pinValue = E_REMOTE_PIN_FACTORY;
            break;

        case E_PIN_MODE_ENGINEERING:
            pinValue = E_REMOTE_PIN_ENGINEERING;
            break;

        case E_PIN_MODE_UPGRADE:
            pinValue = E_REMOTE_PIN_UPGRADE;
            break;

        default:
            duciError.invalidMode = 1u;
            break;
        }

        //reply only if all is well
        if(duciError.value == 0u)
        {
            snprintf(myTxBuffer, 16u, "!PP=%03u", pinValue);
            sendString(myTxBuffer);
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for PP Command – Set PIN protection mode
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
 * @brief   DUCI handler for PP Command – Set PIN protection mode
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
 * @brief   DUCI call back function for LE Command – Clear error log
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
 * @brief   DUCI call back function for LV Command – Clear event log
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
 * @brief   DUCI handler for LE Command â€“ Clear error log
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
        case 1:
            date.day = parameterArray[2].date.day;
            date.month = parameterArray[2].date.month;
            date.year = parameterArray[2].date.year;

            //set cal date
            if(PV624->setNextCalDate(&date) == false)
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

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")
