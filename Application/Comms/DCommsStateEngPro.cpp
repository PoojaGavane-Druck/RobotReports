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
#include "DCommsStateEngPro.h"
#include "Utilities.h"
#include "DPV624.h"
#include "main.h"
#include "DBinaryParser.h"
#include "DCommsMotor.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/
#define ENG_PRTOCOL_SLAVE_COMMANDS_ARRAY_SIZE  64  //this is the maximum no of commands supported in DUCI master/slave mode (can be increased if more needed)
/* Variables --------------------------------------------------------------------------------------------------------*/
extern SPI_HandleTypeDef hspi2;
sEngProtcolCommand_t engProtocolSlaveLocalCommands[ENG_PRTOCOL_SLAVE_COMMANDS_ARRAY_SIZE];

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsState class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateEngPro::DCommsStateEngPro(DDeviceSerial *commsMedium, DTask* task)
:DCommsState(commsMedium, task)
{
    OS_ERR os_error;
    
    myParser = new DEngProtocolParser((void *)this, &engProtocolSlaveLocalCommands[0], (size_t)ENG_PRTOCOL_SLAVE_COMMANDS_ARRAY_SIZE, &os_error);
    createCommands();
    commandTimeoutPeriod = 500u; //default time in (ms) to wait for a response to a DUCI command
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateEngPro::createCommands(void)
{
    /* Commands */
  

    myParser->addCommand(ENG_PROTOCOL_CMD_SetParameter,
        eDataTypeUnsignedLong,
        fnSetParameter,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GetParameter,
        eDataTypeUnsignedLong,
        fnGetParameter,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);    
#ifdef USE_MOTOR_COMMANDS
    myParser->addCommand(ENG_PROTOCOL_CMD_Run,
        eDataTypeUnsignedLong,
        fnRun,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);
#endif
    myParser->addCommand(ENG_PROTOCOL_CMD_StepClock,
        eDataTypeUnsignedLong,
        fnStepClock,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH); 
    
#ifdef USE_MOTOR_COMMANDS
    myParser->addCommand(ENG_PROTOCOL_CMD_GoTo,
        eDataTypeUnsignedLong,
        fnGoTo,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GoToDir,
        eDataTypeUnsignedLong,
        fnGoToDir,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);     

    myParser->addCommand(ENG_PROTOCOL_CMD_GoUntil,
        eDataTypeUnsignedLong,
        fnGoUntil,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_ReleaseSW,
        eDataTypeUnsignedLong,
        fnReleaseSw,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);    

    myParser->addCommand(ENG_PROTOCOL_CMD_GoHome,
        eDataTypeUnsignedLong,
        fnGoHome,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GoMark,
        eDataTypeUnsignedLong,
        fnGoMark,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH); 

    myParser->addCommand(ENG_PROTOCOL_CMD_ResetPos,
        eDataTypeUnsignedLong,
        fnResetPos,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);
#endif
    myParser->addCommand(ENG_PROTOCOL_CMD_ResetDevice,
        eDataTypeUnsignedLong,
        fnResetDevice,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);                     

#ifdef USE_MOTOR_COMMANDS
    myParser->addCommand(ENG_PROTOCOL_CMD_SoftStop,
        eDataTypeUnsignedLong,
        fnSoftStop,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_HardStop,
        eDataTypeUnsignedLong,
        fnHardStop,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);    

    myParser->addCommand(ENG_PROTOCOL_CMD_SoftHiZ,
        eDataTypeUnsignedLong,
        fnSoftHiZ,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_HardHiZ,
        eDataTypeUnsignedLong,
        fnHardHiZ,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH); 
#endif
    
    myParser->addCommand(ENG_PROTOCOL_CMD_GetStatus,
        eDataTypeUnsignedLong,
        fnGetStatus,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_MoveContinuous,
        eDataTypeUnsignedLong,
        fnMove,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);                     

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadStepCount,
        eDataTypeUnsignedLong,
        fnReadSteps,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_WriteRegister,
        eDataTypeUnsignedLong,
        fnWriteRegister,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);    

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadRegister,
        eDataTypeUnsignedLong,
        fnReadRegister,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_WriteAcclAlpha,
        eDataTypeUnsignedLong,
        fnWriteAcclAlpha,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH); 

    myParser->addCommand(ENG_PROTOCOL_CMD_WriteAcclBeta,
        eDataTypeUnsignedLong,
        fnWriteAcclBeta,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_WriteDecclAlpha,
        eDataTypeUnsignedLong,
        fnWriteDecelAlpha,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);                     

    myParser->addCommand(ENG_PROTOCOL_CMD_WriteDecclBeta,
        eDataTypeUnsignedLong,
        fnWriteDecelBeta,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadAcclAlpha,
        eDataTypeUnsignedLong,
        fnReadAcclAlpha,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);    

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadAcclBeta,
        eDataTypeUnsignedLong,
        fnReadAcclBeta,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadDecclAlpha,
        eDataTypeUnsignedLong,
        fnReadDecelAlpha,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH); 

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadDecclBeta,
        eDataTypeUnsignedLong,
        fnReadDecelBeta,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_MinimumSpeed,
        eDataTypeUnsignedLong,
        fnMinSpeed,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);       
    
#ifdef USE_MOTOR_COMMANDS
    myParser->addCommand(ENG_PROTOCOL_CMD_SetAbsPosition,
        eDataTypeUnsignedLong,
        fnSetAbsPosition,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);                     

    myParser->addCommand(ENG_PROTOCOL_CMD_GetAbsPosition,
        eDataTypeUnsignedLong,
        fnGetAbsPosition,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);
#endif
    myParser->addCommand(ENG_PROTOCOL_CMD_GetVersionInfo,
        eDataTypeUnsignedLong,
        fnGetVersionInfo,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);    

    myParser->addCommand(ENG_PROTOCOL_CMD_ResetController,
        eDataTypeUnsignedLong,
        fnResetController,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_WriteHoldCurrent,
        eDataTypeUnsignedLong,
        fnWriteHoldCurrent,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH); 

    myParser->addCommand(ENG_PROTOCOL_CMD_WriteRunCurrent,
        eDataTypeUnsignedLong,
        fnWriteRunCurrent,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_WriteAcclCurrent,
        eDataTypeUnsignedLong,
        fnWriteAcclCurrent,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);                     

    myParser->addCommand(ENG_PROTOCOL_CMD_WriteDecelCurrent,
        eDataTypeUnsignedLong,
        fnWriteDecelCurrent,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadHoldCurrent,
        eDataTypeUnsignedLong,
        fnReadHoldCurrent,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);    

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadRunCurrent,
        eDataTypeUnsignedLong,
        fnReadRunCurrent,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadAcclCurrent,
        eDataTypeUnsignedLong,
        fnReadAcclCurrent,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH); 
    
    myParser->addCommand(ENG_PROTOCOL_CMD_ReadDecelCurrent,
        eDataTypeUnsignedLong,
        fnReadDecelCurrent,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);     

    myParser->addCommand(ENG_PROTOCOL_CMD_ReadSpeedAndCurrent,
        eDataTypeUnsignedLong,
        fnReadSpeedAndCurrent,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);
                                                                                                    
    /* Old commands */
    myParser->addCommand(ENG_PROTOCOL_CMD_OpenValveOne,
        eDataTypeUnsignedLong,
        fnOpenValve1,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_CloseValveOne,
        eDataTypeUnsignedLong,
        fnCloseValve1,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_OpenValveTwo,
        eDataTypeUnsignedLong,
        fnOpenValve2,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_CloseValveTwo,
        eDataTypeUnsignedLong,
        fnCloseValve2,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_OpenValveThree,
        eDataTypeUnsignedLong,
        fnOpenValve3,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_CloseValveThree,
        eDataTypeUnsignedLong,
        fnCloseValve3,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_ControllerStatus,
        eDataTypeUnsignedLong,
        fnGetRE,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GetFullScaleStatus,
        eDataTypeUnsignedLong,
        fnGetFS,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GetBarometerReading,
        eDataTypeUnsignedLong,
        fnGetBR,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GetPM620Reading,
        eDataTypeUnsignedLong,
        fnGetIV,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GetSensorType,
        eDataTypeUnsignedLong,
        fnGetIS,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);
    myParser->addCommand(ENG_PROTOCOL_CMD_GetSetPoint,
        eDataTypeUnsignedLong,
        fnGetSP,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GetControllingMode,
        eDataTypeUnsignedLong,
        fnGetCM,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GetPositionSensor,
        eDataTypeUnsignedLong,
        fnGetPR,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);
    
    myParser->addCommand(ENG_PROTOCOL_CMD_CheckSerialPort,
        eDataTypeUnsignedLong,
        fnCheckSerial,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);
        
    myParser->addCommand(ENG_PROTOCOL_CMD_GetPMType,
        eDataTypeUnsignedLong,
        fnGetPmType,
        DEFAULT_CMD_DATA_LENGTH,
        DEFAULT_RESPONSE_DATA_LENGTH);    
 
}

void DCommsStateEngPro::initialise(void)
{

}

eStateDuci_t DCommsStateEngPro::run(void)
{
    uint32_t receivedLength = 0u;
    uint8_t* buffer;
    sEngProError_t engProError;

    nextState = (eStateDuci_t)E_STATE_DUCI_ENG_TEST;
    while ((eStateDuci_t)E_STATE_DUCI_ENG_TEST == nextState)
    {
        receivedLength = 0u;

        // sleep(50u);

         //listen for a command over USB comms
        if (receiveCmd((char**)&buffer, (uint32_t)5,&receivedLength))
        {
            engProError = myParser->parse(buffer, receivedLength);

            errorStatusRegister.value = engProError.value;

            if (errorStatusRegister.value != 0u)
            {
                /* Handle error by sending out the error response */
                sEngProtocolParameter_t buff[6];
                buff[0].uiValue = (uint32_t)(0xFFFFFFFFu);
                buff[1].uiValue = (uint32_t)(0xFFFFFFFFu);
                buff[2].uiValue = (uint32_t)(0xFFFFFFFFu);
                buff[3].uiValue = (uint32_t)(0xFFFFFFFFu);
                buff[4].uiValue = (uint32_t)(0xFFFFFFFFu);
                buff[5].uiValue = (uint32_t)(0xFFFFFFFFu);
                
                sendResponse(buff, 6u);
            }
            clearRxBuffer();
        }
    }
    return nextState;
}

bool DCommsStateEngPro::sendResponse(sEngProtocolParameter_t *params, uint8_t numOfParams)  //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;

    if (myCommsMedium != NULL)
    {
        successFlag = myParser->prepareResponse( params, numOfParams,(uint8_t*) myTxBuffer, &myTxBufferSize);

        if (successFlag == true)
        {
            successFlag = myCommsMedium->write((uint8_t*)myTxBuffer, myTxBufferSize);
        }
    }

    return successFlag;
}



bool DCommsStateEngPro::receiveCmd(char **pStr, uint32_t numbOfByteToRead, uint32_t* numOfBytesRead) //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;

    if (myCommsMedium != NULL)
    {
        successFlag = myCommsMedium->read((uint8_t**)pStr, numbOfByteToRead, numOfBytesRead, commandTimeoutPeriod);
    }

    return successFlag;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")


/* Motor control function definitions */
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnSetParameter(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnSetParameter(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnSetParameter(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;
    
    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_SetParameter;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGetParameter(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetParameter(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGetParameter(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;
    
    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_GetParameter;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

#ifdef USE_MOTOR_COMMANDS
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnRun(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnRun(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnRun(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;
    
    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_Run;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}
#endif
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnStepClock(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnStepClock(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnStepClock(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;
    
    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_StepClock;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

#ifdef USE_MOTOR_COMMANDS
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGoTo(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGoTo(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGoTo(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_GoTo;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGoToDir(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGoToDir(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGoToDir(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_GoToDir;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGoUntil(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGoUntil(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGoUntil(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_GoUntil;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReleaseSw(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReleaseSw(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReleaseSw(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReleaseSW;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGoHome(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGoHome(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGoHome(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_GoHome;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGoMark(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGoMark(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGoMark(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_GoMark;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnResetPos(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnResetPos(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnResetPos(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ResetPos;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}
#endif
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnResetDevice(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnResetDevice(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnResetDevice(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ResetDevice;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

#ifdef USE_MOTOR_COMMANDS
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnSoftStop(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnSoftStop(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnSoftStop(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_SoftStop;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnHardStop(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnHardStop(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnHardStop(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_HardStop;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnSoftHiZ(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnSoftHiZ(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnSoftHiZ(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_SoftHiZ;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnHardHiZ(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnHardHiZ(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnHardHiZ(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_HardHiZ;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}
#endif
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGetStatus(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetStatus(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}


/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGetStatus(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_GetStatus;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnMove(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnMove(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnMove(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        //sEngProtocolParameter_t responseData[1];
        uint8_t rxBuff[4] = {0x00, 0x00, 0x00, 0x00};
        cmd = ENG_PROTOCOL_CMD_MoveContinuous;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        if((PV624->stepperMotor != NULL) && (rxBuff != NULL))
        {
            PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, rxBuff);
        }
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(rxBuff, 4u);
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadSteps(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadSteps(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadSteps(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadStepCount;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWriteRegister(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWriteRegister(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWriteRegister(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WriteRegister;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadRegister(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadRegister(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadRegister(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadRegister;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWriteAcclAlpha(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWriteAcclAlpha(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWriteAcclAlpha(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WriteAcclAlpha;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWriteAcclBeta(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWriteAcclBeta(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWriteAcclBeta(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WriteAcclBeta;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWriteDecelAlpha(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWriteDecelAlpha(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWriteDecelAlpha(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WriteDecclAlpha;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWriteDecelBeta(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWriteDecelBeta(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWriteDecelBeta(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WriteDecclBeta;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadAcclAlpha(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadAcclAlpha(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadAcclAlpha(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadAcclAlpha;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadAcclBeta(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadAcclBeta(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadAcclBeta(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadAcclBeta;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadDecelAlpha(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadDecelAlpha(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadDecelAlpha(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadDecclAlpha;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadDecelBeta(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadDecelBeta(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadDecelBeta(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadDecclBeta;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnMinSpeed(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnMinSpeed(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnMinSpeed(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_MinimumSpeed;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u);  
    }
    return engProError;
}

#ifdef USE_MOTOR_COMMANDS
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnMaxSpeed(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnMaxSpeed(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnMaxSpeed(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_MaximumSpeed;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u);  
    }
    return engProError;
}


/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWdTime(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWdTime(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWdTime(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WatchdogTime;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u);  
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWdEnable(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWdEnable(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWdEnable(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WatchdogEnable;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnAcclTime(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnAcclTime(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnAcclTime(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_AccelerationTime;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnDecelTime(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnDecelTime(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnDecelTime(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_DecelerationTime;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnSetAbsPosition(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnSetAbsPosition(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnSetAbsPosition(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_SetAbsPosition;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGetAbsPosition(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetAbsPosition(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGetAbsPosition(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_GetAbsPosition;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}
#endif
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGetVersionInfo(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetVersionInfo(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGetVersionInfo(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        //sEngProtocolParameter_t responseData[1];
        uint8_t rxBuff[4] = {0x00, 0x00, 0x00, 0x00};
        cmd = ENG_PROTOCOL_CMD_GetVersionInfo;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        if((PV624->stepperMotor != NULL) && (rxBuff != NULL))
        {
            PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, rxBuff);
        }
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(rxBuff, 4u);
        //sendResponse(rxBuff, 1u);
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnResetController(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnResetController(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnResetController(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ResetController;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWriteHoldCurrent(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWriteHoldCurrent(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWriteHoldCurrent(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WriteHoldCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWriteRunCurrent(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWriteRunCurrent(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWriteRunCurrent(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WriteRunCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWriteAcclCurrent(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWriteAcclCurrent(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWriteAcclCurrent(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WriteAcclCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnWriteDecelCurrent(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnWriteDecelCurrent(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnWriteDecelCurrent(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_WriteDecelCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadHoldCurrent(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadHoldCurrent(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadHoldCurrent(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadHoldCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadRunCurrent(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadRunCurrent(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadRunCurrent(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadRunCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadAcclCurrent(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadAcclCurrent(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadAcclCurrent(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadAcclCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadDecelCurrent(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadDecelCurrent(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadDecelCurrent(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;
   
    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadDecelCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadSpeedAndCurrent(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnReadSpeedAndCurrent(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnReadSpeedAndCurrent(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadSpeedAndCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u); 
    }
    return engProError;
}

/* Non motor control functions */
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnOpenValve1(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnOpenValve1(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnOpenValve1(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
       
        PV624->valve1->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_FORWARD);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;
            
        }
        
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnOpenValve2(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnOpenValve2(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnOpenValve2(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
 
        PV624->valve2->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_FORWARD);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;

        }

    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnOpenValve3(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnOpenValve3(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnOpenValve3(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {

        PV624->valve3->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_FORWARD);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;

        }

    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnCloseValve1(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnCloseValve1(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnCloseValve1(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
  
        PV624->valve1->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_REVERSE);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;

        }

    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnCloseValve2(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnCloseValve2(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnCloseValve2(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {

        PV624->valve2->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_REVERSE);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;

        }

    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnCloseValve3(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnCloseValve3(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}
/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnCloseValve3(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {

        PV624->valve3->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_REVERSE);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;

        }

    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetRE(void *instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro*myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetRE(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnGetRE(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
  
    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        PV624->setControllerStatus(parameterArray->uiValue);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        bool statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;         
        }
    }

    return engProError;
}




sEngProError_t DCommsStateEngPro::fnGetIV(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetIV(parameterArray);
    }
    else
    {
       
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}


sEngProError_t DCommsStateEngPro::fnGetIV(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    uint32_t rate = (uint32_t)(0);
   
    float measVal = 35000.0f;
    bool statusFlag = false;
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        rate = parameterArray->uiValue;
        
        PV624->takeNewReading(rate);
        
        //statusFlag = PV624->commsSerial->waitForEvent(EV_FLAG_TASK_NEW_VALUE, 1000u);
        
        statusFlag = PV624->commsUSB->waitForEvent(EV_FLAG_TASK_NEW_VALUE, 1000u);
        
        sEngProtocolParameter_t pressure;
        sEngProtocolParameter_t baro;
        sEngProtocolParameter_t sp;
        sEngProtocolParameter_t mode;
        sEngProtocolParameter_t pressureG;
        sEngProtocolParameter_t spType;
        
        eSensorType_t sensorType = (eSensorType_t)(0);
        eFunction_t function = E_FUNCTION_GAUGE;
        
        float setPoint = (float)(0);
        sEngProtocolParameter_t buff[6];
        eControllerMode_t controllerMode;
        
        if (true == statusFlag)
        {
            measVal = 0.0f;
            PV624->instrument->getReading((eValueIndex_t)E_FUNCTION_ABS, (float*)&measVal);
            pressure.floatValue = measVal;
            
            measVal = 0.0f;
            PV624->instrument->getReading((eValueIndex_t)E_FUNCTION_GAUGE, (float*)&measVal);
            pressureG.floatValue = measVal;
        }
        
        if (true == PV624->instrument->getReading(E_VAL_INDEX_BAROMETER_VALUE ,&measVal))
        {
            baro.floatValue = measVal;
        }
        
        PV624->getSensorType(&sensorType);
        PV624->getFunction(&function);
        
        
        if (true == PV624->getPressureSetPoint(&setPoint))
        {
            sp.floatValue = setPoint;
        }
        
        if (true == PV624->getControllerMode(&controllerMode))
        {            
            mode.uiValue = (uint32_t)controllerMode;        
        }

        PV624->getFunction((eFunction_t*)&spType.uiValue);
        
        buff[0] = pressure;
        buff[1] = baro;
        buff[2] = sp;
        buff[3] = mode;
        buff[4] = pressureG;
        buff[5] = spType;
        
        statusFlag = sendResponse(buff, 6u);
        
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;            

        }    
        
    }
    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetIS(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetIS(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetIS(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    uint32_t senType;
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {        
        PV624->getPM620Type((uint32_t*)&senType);
        sEngProtocolParameter_t param;
        param.uiValue = (uint32_t)senType;
        bool statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;
        }

        
    }
    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetFS(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetFS(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetFS(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;    
    float minPressure = 0.0f;
    float maxPressure = 0.0f;
   
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        PV624->getPosFullscale((float*)&maxPressure);
        PV624->getNegFullscale((float*)&minPressure);
        sEngProtocolParameter_t param;
        param.floatValue = maxPressure;
        bool statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;

        }  
    }
    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetCM(void *instance, sEngProtocolParameter_t * parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro*myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetCM(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetCM(sEngProtocolParameter_t * parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

//only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        eControllerMode_t controllerMode = E_CONTROLLER_MODE_NONE;
        if (true == PV624->getControllerMode(&controllerMode))
        {            
            sEngProtocolParameter_t param;
            param.uiValue = (uint32_t)controllerMode;
            bool statusFlag = sendResponse(&param, 1u);
            if (true == statusFlag)
            {
                errorStatusRegister.value = 0u; //clear error status register as it has been read now
            }
            else
            {
                engProError.TXtimeout = 1u;
                errorStatusRegister.TXtimeout = 1u; 
            }
        }
        else
        {
            engProError.cmdExecutionFailed = 1u;
        }
    }

    return engProError;
}



sEngProError_t DCommsStateEngPro::fnGetSP(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetSP(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetSP(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        float32_t setPoint = 0.0f;
        if (true == PV624->getPressureSetPoint(&setPoint))
        {
            sEngProtocolParameter_t param;
            param.floatValue = setPoint;
            bool statusFlag = sendResponse(&param, 1u);
            if (true == statusFlag)
            {
                errorStatusRegister.value = 0u; //clear error status register as it has been read now
            }
            else
            {
                engProError.TXtimeout = 1u;
            }
        }
        else
        {
            engProError.cmdExecutionFailed = 1u;
        }
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetBR(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetBR(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetBR(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        float measValue = 0.0f;
        if (true == PV624->instrument->getReading(E_VAL_INDEX_BAROMETER_VALUE ,&measValue))
        {
            sEngProtocolParameter_t param;
            param.floatValue = measValue;
            bool statusFlag = sendResponse(&param, 1u);
            if (true == statusFlag)
            {
                errorStatusRegister.value = 0u; //clear error status register as it has been read now
            }
            else
            {
                engProError.TXtimeout = 1u;
            }
        }
        else
        {
            engProError.cmdExecutionFailed = 1u;
        }
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetPR(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetPR(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetPR(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        uint32_t val = 0u;
        if (true == PV624->powerManager->getValue(EVAL_INDEX_IR_SENSOR_ADC_COUNTS,
            (uint32_t*)&val))
        {
            sEngProtocolParameter_t param;
            param.uiValue = (uint32_t)val;
            bool statusFlag = sendResponse(&param, 1u);
            if (true == statusFlag)
            {
                errorStatusRegister.value = 0u; //clear error status register as it has been read now
            }
            else
            {
                engProError.TXtimeout = 1u;
            }
        }
        else
        {
            engProError.cmdExecutionFailed = 1u;
        }
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnCheckSerial(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnCheckSerial(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnCheckSerial(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t param;
        param.uiValue = (uint32_t)(0x13572468);
        bool statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;
        }
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetPmType(void* instance, sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro* myInstance = (DCommsStateEngPro*)instance;

    if (myInstance != NULL)
    {
        engProError = myInstance->fnGetPmType(parameterArray);
    }
    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetPmType(sEngProtocolParameter_t* parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }
    else
    {
        sEngProtocolParameter_t param;
        uint32_t sensorType = (uint32_t)(0);        
        PV624->getPM620Type(&sensorType);
        param.uiValue = (uint32_t)(sensorType);
        bool statusFlag = sendResponse(&param, 1u);
        if (true == statusFlag)
        {
            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }
        else
        {
            engProError.TXtimeout = 1u;
        }
    }

    return engProError;
}
/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")


