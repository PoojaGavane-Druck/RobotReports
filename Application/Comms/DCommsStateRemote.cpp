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
#define SLAVE_REMOTE_COMMANDS_ARRAY_SIZE  96  //this is the maximum no of commands supported in DUCI remot eslave mode (can be increased if more needed)
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
sDuciCommand_t duciSlaveRemoteCommands[SLAVE_REMOTE_COMMANDS_ARRAY_SIZE];
DCommsStateRemote *DCommsStateRemote::myInstance = NULL;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateRemote class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateRemote::DCommsStateRemote(DDeviceSerial *commsMedium, DTask* task)
: DCommsStateDuci(commsMedium,task)
{
    OS_ERR os_error = OS_ERR_NONE;
    myParser = new DParseSlave((void *)this,  &duciSlaveRemoteCommands[0], (size_t)SLAVE_REMOTE_COMMANDS_ARRAY_SIZE, &os_error);
    createCommands();
    commandTimeoutPeriod = 250u; //time in (ms) to wait for a response to a command (0 means wait forever)
    
    if (myParser != NULL)
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
    if (myCommsMedium == NULL)
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
    DCommsStateDuci::createCommands();

    //Create DUCI command set
    //TODO: make PIN mode be a mask eg bit 0 = cal, 1 = config, 2 = factory, 3 = prod test/service
    //TODO:  factor out those commands that are common to all into base class

    //then set true (1) if that mode PIN is required
    myParser->addCommand("CA", "",             "",              fnSetCA,       NULL,      0xFFFFu);
    myParser->addCommand("CB", "=i",           "",              fnSetCB,       NULL,      0xFFFFu);
    myParser->addCommand("CD", "[i]=d",        "[i]?",          fnSetCD,       fnGetCD,      0xFFFFu);
    myParser->addCommand("CI", "=i",          "?",              fnSetCI,       fnGetCI,      0xFFFFu);
    myParser->addCommand("CT", "[i]=i,[i]",    "",              fnSetCT,     NULL,      0xFFFFu);           
    myParser->addCommand("CP", "[i]=v",        "",              fnSetCP,       NULL,      0xFFFFu);
    myParser->addCommand("CS", "",             "?",             fnSetCS,    fnGetCS,   0xFFFFu);
    myParser->addCommand("CX", "",             "",              fnSetCX,       NULL,      0xFFFFu);
    myParser->addCommand("DK", "",             "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("IP", "[i]=i,b",      "[i],[i]?",      NULL,       NULL,      0xFFFFu);
    myParser->addCommand("IZ", "[i],[=],[v]",  "[i]?",          fnSetIZ,    fnGetIZ,      0xFFFFu);
    myParser->addCommand("SP", "=v",           "?",             fnSetSP,    fnGetSP,    0xFFFFu);
    myParser->addCommand("SN", "=i",            "?",            fnSetSN,    fnGetSN,   0xFFFFu);   //serial number
    myParser->addCommand("CM", "=i",            "?",            fnSetCM,    fnGetCM,   0xFFFFu);   //serial number
    myParser->addCommand("CN", "=i",            "?",            fnSetCN,       fnGetCN,   0xFFFFu);  
    myParser->addCommand("KP", "=i,[i]",       "?",             fnSetKP,    NULL,      0xFFFFu);
    myParser->addCommand("LE", "=i",           "i?",            NULL,       NULL,      0xFFFFu);
    myParser->addCommand("LV", "=i",           "i?",            NULL,       NULL,      0xFFFFu);    
    myParser->addCommand("PP", "=3i",          "?",             NULL,       NULL,      0xFFFFu);
    myParser->addCommand("RB", "",             "?",             NULL,       NULL,      0xFFFFu);    
    myParser->addCommand("RV", "",             "i?",            NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SD", "=d",            "?",            fnSetSD,    fnGetSD,   0xFFFFu); //Set/get system date
    myParser->addCommand("SE", "[i]=i",        "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SF", "[i]=i,i",      "[i]?",          fnSetSF,    fnGetSF,      0xFFFFu);      
    myParser->addCommand("SR", "=i",           "?",             NULL,       NULL,      0xFFFFu);    
    myParser->addCommand("ST", "=t",           "?",             fnSetST,    fnGetST,   0xFFFFu); //Set/get system time
    myParser->addCommand("TP", "i,[=][i]",     "[i]?",          NULL,       NULL,      0xFFFFu);
    myParser->addCommand("SC", "[i]=i",        "[i]?",          fnSetSC,   fnGetSC,    0XFFFFu);
    myParser->addCommand("UF", "",             "",              fnSetUF,   NULL,       0XFFFFu);               
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
eStateDuci_t DCommsStateRemote::run(void)
{
   
    char *buffer;

    //Entry
    errorStatusRegister.value = 0u; //clear DUCI error status register
    externalDevice.status.all = 0u;

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;

    //DO
    nextState = E_STATE_DUCI_REMOTE;
    
    
    while (E_STATE_DUCI_REMOTE == nextState)
    {
        //OSTimeDlyHMSM(0u, 0u, 0u, 500u, OS_OPT_TIME_HMSM_STRICT, &os_err);

        clearRxBuffer();

        if (receiveString(&buffer))
        {
            duciError = myParser->parse(buffer);

            errorStatusRegister.value |= duciError.value;
        }
    }

    //Exit
    myCommsMedium = NULL; //mark the state as free

    return nextState;
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
        if ((keyId < E_BUTTON_1) || (keyId > E_BUTTON_2) || (pressType > 1u))
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
            

            switch (keyId)
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

sDuciError_t DCommsStateRemote::fnSetSF(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetSF(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateRemote::fnSetSF(sDuciParameter_t * parameterArray)
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
        if(0 == parameterArray[0].intNumber)
        {
            int32_t func = (int32_t)parameterArray[2].intNumber;
            int32_t pseudoType = (int32_t)parameterArray[3].intNumber;
            
            if ( (int32_t) 7 ==  func)
            {
              PV624->instrument->setFunction(E_FUNCTION_BAROMETER);
            }
            else if( (int32_t) 13 ==  func)
            {
              if( 0 == pseudoType)
              {
                PV624->instrument->setFunction(E_FUNCTION_PSEUDO_ABS);
              }
              else if( 1 == pseudoType)
              {
                PV624->instrument->setFunction(E_FUNCTION_PSEUDO_GAUGE);
              }
              else
              {
                PV624->instrument->setFunction(E_FUNCTION_EXT_PRESSURE);
              }
            }
            else 
            {
              PV624->instrument->setFunction(E_FUNCTION_EXT_PRESSURE);
            }

        }
        else
        {
          
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
sDuciError_t DCommsStateRemote::fnSetST(void *instance, sDuciParameter_t *parameterArray)   //* @note	=t",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
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
        if (PV624->setTime(&rtcTime) == false)
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
sDuciError_t DCommsStateRemote::fnSetSD(void *instance, sDuciParameter_t *parameterArray)   //* @note	=d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
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
        if (PV624->setDate(&date) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

sDuciError_t DCommsStateRemote::fnSetSN(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

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

sDuciError_t DCommsStateRemote::fnSetSN(sDuciParameter_t * parameterArray)
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
        if (PV624->setSerialNumber(parameterArray[1].uintNumber) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}


sDuciError_t DCommsStateRemote::fnSetCM(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetCM(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateRemote::fnSetCM(sDuciParameter_t * parameterArray)
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
        if (PV624->setControllerMode((eControllerMode_t)parameterArray[1].uintNumber) == false)
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}


/*
 * @brief   Handle cal interval reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DCommsStateRemote::fnSetCI(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
sDuciError_t DCommsStateRemote::fnSetCI(sDuciParameter_t * parameterArray)
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
        //save cal interval       
        if (false == PV624->setCalInterval((uint32_t)parameterArray[1].intNumber))
        {
          
            duciError.commandFailed = 1u;
        }
        
    }

    return duciError;
}


sDuciError_t DCommsStateRemote::fnSetSP(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetSP(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateRemote::fnSetSP(sDuciParameter_t * parameterArray)
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
        if (PV624->setPressureSetPoint((float32_t)parameterArray[1].floatValue) == false)
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
sDuciError_t DCommsStateRemote::fnSetCT(void *instance, sDuciParameter_t *parameterArray)   //* @note	[i]=i,[i]",    "",              NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //command format is <int><=><int><int>
        //validate the parameters

            //only accepting user calibration, so this parameter must always be 0
            if (parameterArray[2].intNumber == 0)
            {
                //set cal type 0 (user) for specified channel with the third parameter being the range
                if (PV624->setCalibrationType( 0, (uint32_t)parameterArray[3].intNumber) == false)
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
sDuciError_t DCommsStateRemote::fnSetCS(void *instance, sDuciParameter_t *parameterArray)   //* @note	",             "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //command has no parameters
        if (PV624->startCalSampling() == false)
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
sDuciError_t DCommsStateRemote::fnSetCP(void *instance, sDuciParameter_t *parameterArray)   //* @note	[i]=v",        "",              NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //command format is <int><=><float>
        //called functions validates the parameters itself but we know it has to be greater than 0
        uint32_t calPoint = (uint32_t)parameterArray[0].intNumber;

        if (calPoint > 0)
        {
            //floating point value represents the user entered value
            if (PV624->setCalPoint(calPoint, parameterArray[2].floatValue) == false)
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
 * @brief   DUCI call back function for command CA - Cal Accept
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateRemote::fnSetCA(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //command has no parameters
        if (PV624->acceptCalibration() == false)
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

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //command has no parameters
        if (PV624->abortCalibration() == false)
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
sDuciError_t DCommsStateRemote::fnSetCN(void *instance, sDuciParameter_t *parameterArray)   //* @note	",             "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        
        if (PV624->setRequiredNumCalPoints(parameterArray[1].uintNumber) == false)
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
sDuciError_t DCommsStateRemote::fnSetSC(void *instance, sDuciParameter_t *parameterArray)   //* @note	=d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        int32_t index = parameterArray[0].intNumber;
        int32_t mode = parameterArray[2].intNumber;
        if ((index == 0) && ((mode == 0) || (mode == 1)))
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

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {  
        bool ok = false;
        if( (uint32_t) 0 == parameterArray[1].uintNumber)
        {
          ok = PV624->performUpgrade();
        }
        else if( (uint32_t) 1 == parameterArray[1].uintNumber)
        {
          ok = PV624->performPM620tUpgrade();
        }
        else
        {
          /* Do Nothing */
        }
        if (!ok)
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
sDuciError_t DCommsStateRemote::fnSetIZ(void *instance, sDuciParameter_t *parameterArray)   //* @note	[i],[=],[v]",  "[i]?",          NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {     
            if (PV624->setZero(parameterArray[2].floatValue) == false)
            {
                duciError.commandFailed = 1u;
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

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        switch(parameterArray[1].intNumber)
        {
            case 1: //backup current cal
                if (PV624->backupCalDataSave() == false)
                {
                    duciError.commandFailed= 1u;
                }
                break;

            case 2: //restore cal from backup
                if (PV624->backupCalDataRestore() == false)
                {
                    duciError.commandFailed= 1u;
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

    DCommsStateRemote *myInstance = (DCommsStateRemote*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //command format is <int><=><date>
        //validate the parameters
          sDate_t date;

          date.day = parameterArray[2].date.day;
          date.month = parameterArray[2].date.month;
          date.year = parameterArray[2].date.year;

          //set cal date
          if (PV624->setCalDate( &date) == false)
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
_Pragma ("diag_default=Pm017,Pm128")
