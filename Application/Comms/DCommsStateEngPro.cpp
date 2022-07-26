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

/* Error handler instance parameter starts from 1501 to 1600 */
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
DCommsStateEngPro::DCommsStateEngPro(DDeviceSerial *commsMedium, DTask *task)
    : DCommsState(commsMedium, task)
{
    OS_ERR os_error;

    myParser = new DEngProtocolParser((void *)this, &engProtocolSlaveLocalCommands[0], (size_t)ENG_PRTOCOL_SLAVE_COMMANDS_ARRAY_SIZE, &os_error);
    createCommands();
    commandTimeoutPeriod = 500u; //default time in (ms) to wait for a response to a DUCI command
}
/**
 * @brief   DCommsState class destructor
 * @param   void
 * @retval  void
 */
DCommsStateEngPro::~DCommsStateEngPro()
{

}
/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateEngPro::createCommands(void)
{
    /* Commands */
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

    myParser->addCommand(ENG_PROTOCOL_CMD_GetPMType,
                         eDataTypeUnsignedLong,
                         fnGetPmType,
                         DEFAULT_CMD_DATA_LENGTH,
                         DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_DuciSwitch,
                         eDataTypeUnsignedLong,
                         fnSwitchToDuci,
                         DEFAULT_CMD_DATA_LENGTH,
                         DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_ValveTime,
                         eDataTypeUnsignedLong,
                         fnSetValveTimer,
                         DEFAULT_CMD_DATA_LENGTH,
                         DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_OptInterrupter,
                         eDataTypeUnsignedLong,
                         fnGetOptInterrupt,
                         DEFAULT_CMD_DATA_LENGTH,
                         DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_ConfigValve,
                         eDataTypeUnsignedLong,
                         fnConfigValve,
                         DEFAULT_CMD_DATA_LENGTH,
                         DEFAULT_RESPONSE_DATA_LENGTH);

    myParser->addCommand(ENG_PROTOCOL_CMD_GetRate,
                         eDataTypeUnsignedLong,
                         fnGetRate,
                         DEFAULT_CMD_DATA_LENGTH,
                         DEFAULT_RESPONSE_DATA_LENGTH);

}

void DCommsStateEngPro::initialise(void)
{

}

eStateDuci_t DCommsStateEngPro::run(void)
{
    uint32_t receivedLength = 0u;
    uint8_t *buffer;
    sEngProError_t engProError;

    nextState = (eStateDuci_t)E_STATE_DUCI_ENG_TEST;

    while((eStateDuci_t)E_STATE_DUCI_ENG_TEST == nextState)
    {
        receivedLength = 0u;

        // sleep(50u);

        //listen for a command over USB comms
        if(receiveCmd((char **)&buffer, (uint32_t)5, &receivedLength))
        {
            engProError = myParser->parse(buffer, receivedLength);

            errorStatusRegister.value = engProError.value;

            if(errorStatusRegister.value != 0u)
            {
                /* Handle error by sending out the error response */
                sEngProtocolParameter_t buff[6];
                buff[0].uiValue = 0xFFFFFFFFu;
                buff[1].uiValue = 0xFFFFFFFFu;
                buff[2].uiValue = 0xFFFFFFFFu;
                buff[3].uiValue = 0xFFFFFFFFu;
                buff[4].uiValue = 0xFFFFFFFFu;
                buff[5].uiValue = 0xFFFFFFFFu;

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

    if(myCommsMedium != NULL)
    {
        successFlag = myParser->prepareResponse(params, numOfParams, (uint8_t *) myTxBuffer, &myTxBufferSize);

        if(successFlag == true)
        {
            successFlag = myCommsMedium->write((uint8_t *)myTxBuffer, myTxBufferSize);
        }
    }

    return successFlag;
}



bool DCommsStateEngPro::receiveCmd(char **pStr, uint32_t numbOfByteToRead, uint32_t *numOfBytesRead) //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;

    if(myCommsMedium != NULL)
    {
        successFlag = myCommsMedium->read((uint8_t **)pStr, numbOfByteToRead, numOfBytesRead, commandTimeoutPeriod);
    }

    return successFlag;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")


/* Motor control function definitions */
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnMove(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnMove(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        //sEngProtocolParameter_t responseData[1];
        uint8_t rxBuff[4] = {0x00, 0x00, 0x00, 0x00};
        cmd = (eEngProtocolCommand_t)0x13;

        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        if((PV624->stepperMotor != NULL) && (rxBuff != NULL))
        {
            /* check if any optical sensor is triggered */
            uint32_t opt1 = 0u;
            uint32_t opt2 = 0u;

            opt1 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
            opt2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);

            if((1u == opt1) && (1u == opt2))
            {
                PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, rxBuff);
            }

            else if(0u == opt1)
            {
                // Sensor one has triggered, accept only negative steps
                if(parameterArray->iValue > 0)
                {
                    parameterArray->uiValue = 0u;
                    PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, rxBuff);
                }

                else
                {
                    PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, rxBuff);
                }
            }

            else if(0u == opt2)
            {
                // Sensor one has triggered, accept only positive steps
                if(parameterArray->iValue < 0)
                {
                    parameterArray->uiValue = 0u;
                    PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, rxBuff);
                }

                else
                {
                    PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, rxBuff);
                }
            }

            else
            {
                parameterArray->uiValue = 0u;
                PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, rxBuff);
            }
        }

        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(rxBuff, 4u);
    }

    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadSteps(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnReadSteps(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadStepCount;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u);
    }

    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnGetVersionInfo(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnGetVersionInfo(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
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
            PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, rxBuff);
        }

        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(rxBuff, 4u);
        //sendResponse(rxBuff, 1u);
    }

    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnResetController(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnResetController(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ResetController;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u);
    }

    return engProError;
}

/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnReadSpeedAndCurrent(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnReadSpeedAndCurrent(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    eEngProtocolCommand_t cmd = ENG_PROTOCOL_CMD_None;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        sEngProtocolParameter_t responseData[4];
        cmd = ENG_PROTOCOL_CMD_ReadSpeedAndCurrent;
        /* Forward message to motor controller and get response
        then return the response from motor controller to PC */
        PV624->stepperMotor->sendEnggCommand((uint8_t)(cmd), parameterArray->byteArray, (uint8_t *)(responseData[0].byteArray));
        //sendResponse(&responseData[0], 1u);
        myCommsMedium->write(&responseData[0].byteArray[0], 4u);
    }

    return engProError;
}

/* Non motor control functions */
/* Static callback functions ----------------------------------------------------------------------------------------*/
sEngProError_t DCommsStateEngPro::fnOpenValve1(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnOpenValve1(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        //PV624->valve1->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_FORWARD);
        PV624->valve1->triggerValve((eValveState_t)VALVE_STATE_ON);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnOpenValve2(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnOpenValve2(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {

        //PV624->valve2->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_FORWARD);
        PV624->valve2->triggerValve((eValveState_t)VALVE_STATE_ON);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnOpenValve3(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnOpenValve3(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {

        //PV624->valve3->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_FORWARD);
        PV624->valve3->triggerValve((eValveState_t)VALVE_STATE_ON);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnCloseValve1(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnCloseValve1(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        //PV624->valve1->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_REVERSE);
        PV624->valve1->triggerValve((eValveState_t)VALVE_STATE_OFF);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnCloseValve2(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnCloseValve2(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {

        //PV624->valve2->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_REVERSE);
        PV624->valve2->triggerValve((eValveState_t)VALVE_STATE_OFF);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnCloseValve3(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnCloseValve3(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {

        //PV624->valve3->valveTest((eValveFunctions_t)E_VALVE_FUNCTION_REVERSE);
        PV624->valve3->triggerValve((eValveState_t)VALVE_STATE_OFF);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetRE(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
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

sEngProError_t DCommsStateEngPro::fnGetRE(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        PV624->setControllerStatus(parameterArray->uiValue);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        bool statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetIV(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnGetIV(parameterArray);
    }

    else
    {

        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetIV(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    uint32_t rate = 0u;

    float measVal = 35000.0f;
    bool statusFlag = false;

    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        rate = parameterArray->uiValue;

        PV624->takeNewReading(rate);

        statusFlag = PV624->commsUSB->waitForEvent(EV_FLAG_TASK_NEW_VALUE, 1000u);

        sEngProtocolParameter_t pressure;
        sEngProtocolParameter_t baro;
        sEngProtocolParameter_t sp;
        sEngProtocolParameter_t mode;
        sEngProtocolParameter_t pressureG;
        sEngProtocolParameter_t spType;

        eSensorType_t sensorType = (eSensorType_t)(0);
        eFunction_t function = E_FUNCTION_GAUGE;

        float32_t setPoint = 0.0f;
        sEngProtocolParameter_t buff[6];
        eControllerMode_t controllerMode;

        if(true == statusFlag)
        {
            measVal = 0.0f;
            PV624->instrument->getReading((eValueIndex_t)E_VAL_INDEX_VALUE, &measVal);
            pressure.floatValue = measVal;
        }

        if(true == PV624->instrument->getReading(E_VAL_INDEX_BAROMETER_VALUE, &measVal))
        {
            baro.floatValue = measVal;
        }

        PV624->getSensorType(&sensorType);
        PV624->getFunction(&function);

        if(2u == sensorType)
        {
            if(function == (eFunction_t)(E_FUNCTION_ABS))
            {
                pressureG.floatValue = pressure.floatValue - baro.floatValue;
            }

            else if(function == (eFunction_t)(E_FUNCTION_GAUGE))
            {
                pressureG.floatValue = pressure.floatValue;
                pressure.floatValue = pressureG.floatValue + baro.floatValue;
            }

            else
            {
                pressureG.floatValue = pressure.floatValue;
                pressure.floatValue = pressureG.floatValue + baro.floatValue;
            }
        }

        else if(1u == sensorType)
        {
            if(function == (eFunction_t)(E_FUNCTION_ABS))
            {
                pressureG.floatValue = pressure.floatValue - baro.floatValue;
            }

            else if(function == (eFunction_t)(E_FUNCTION_GAUGE))
            {
                pressureG.floatValue = pressure.floatValue;
                pressure.floatValue = pressureG.floatValue + baro.floatValue;
            }

            else
            {
                pressureG.floatValue = pressure.floatValue - baro.floatValue;
            }
        }

        else
        {
        }

        if(true == PV624->getPressureSetPoint(&setPoint))
        {
            sp.floatValue = setPoint;
        }

        if(true == PV624->getControllerMode(&controllerMode))
        {
            mode.uiValue = (uint32_t)controllerMode;
        }

        PV624->getFunction((eFunction_t *)&spType.uiValue);

        buff[0] = pressure;
        buff[1] = baro;
        buff[2] = sp;
        buff[3] = mode;
        buff[4] = pressureG;
        buff[5] = spType;

        statusFlag = sendResponse(buff, 6u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetOptInterrupt(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnGetOptInterrupt(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetOptInterrupt(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        uint32_t opt1 = 0u;
        uint32_t opt2 = 0u;
        sEngProtocolParameter_t param;

        opt1 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
        opt2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
        param.uiValue = 0u;
        param.byteArray[0] = (uint8_t)(opt1);
        param.byteArray[1] = (uint8_t)(opt2);


        bool statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetIS(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnGetIS(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetIS(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    uint32_t senType;

    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        PV624->getPM620Type((uint32_t *)&senType);
        sEngProtocolParameter_t param;
        param.uiValue = (uint32_t)senType;
        bool statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetFS(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnGetFS(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetFS(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    float minPressure = 0.0f;
    float maxPressure = 0.0f;

    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        PV624->getPosFullscale((float *)&maxPressure);
        PV624->getNegFullscale((float *)&minPressure);
        sEngProtocolParameter_t param;
        param.floatValue = maxPressure;
        bool statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetCM(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnGetCM(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetCM(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

//only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        eControllerMode_t controllerMode = E_CONTROLLER_MODE_NONE;

        if(true == PV624->getControllerMode(&controllerMode))
        {
            sEngProtocolParameter_t param;
            param.uiValue = (uint32_t)controllerMode;
            bool statusFlag = sendResponse(&param, 1u);

            if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetSP(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnGetSP(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetSP(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        float32_t setPoint = 0.0f;

        if(true == PV624->getPressureSetPoint(&setPoint))
        {
            sEngProtocolParameter_t param;
            param.floatValue = setPoint;
            bool statusFlag = sendResponse(&param, 1u);

            if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetBR(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnGetBR(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetBR(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        float measValue = 0.0f;

        if(true == PV624->instrument->getReading(E_VAL_INDEX_BAROMETER_VALUE, &measValue))
        {
            sEngProtocolParameter_t param;
            param.floatValue = measValue;
            bool statusFlag = sendResponse(&param, 1u);

            if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetPmType(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnGetPmType(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetPmType(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        sEngProtocolParameter_t param;
        uint32_t sensorType = 0u;
        PV624->getPM620Type(&sensorType);
        param.uiValue = (uint32_t)(sensorType);
        bool statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnSetValveTimer(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnSetValveTimer(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnSetValveTimer(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        uint32_t valveTimer = 0u;
        valveTimer = parameterArray->uiValue;
        PV624->valve3->setValveTime(valveTimer);
        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        bool statusFlag = sendResponse(&param, 1u);
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnSwitchToDuci(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnSwitchToDuci(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnSwitchToDuci(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {

    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnConfigValve(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnConfigValve(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

/* instance versions of callback functions --------------------------------------------------------------------------*/

sEngProError_t DCommsStateEngPro::fnConfigValve(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;
    bool statusFlag = false;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        uint32_t valveNum = parameterArray->byteArray[0];
        uint32_t valveConfig = parameterArray->byteArray[1];

        if(1u == valveNum)
        {
            PV624->valve1->reConfigValve(valveConfig);
        }

        else if(2u == valveNum)
        {
            PV624->valve2->reConfigValve(valveConfig);
        }

        else if(3u == valveNum)
        {
            PV624->valve3->reConfigValve(valveConfig);
        }

        else
        {
        }

        sEngProtocolParameter_t param;
        param.uiValue = 1u;
        statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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

sEngProError_t DCommsStateEngPro::fnGetRate(void *instance, sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    DCommsStateEngPro *myInstance = (DCommsStateEngPro *)instance;

    if(myInstance != NULL)
    {
        engProError = myInstance->fnGetRate(parameterArray);
    }

    else
    {
        engProError.unhandledMessage = 1u;
    }

    return engProError;
}

sEngProError_t DCommsStateEngPro::fnGetRate(sEngProtocolParameter_t *parameterArray)
{
    sEngProError_t engProError;
    engProError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eEngProtocolMessage_t)E_ENG_PROTOCOL_COMMAND)
    {
        engProError.messageIsNotCmdType = 1u;
    }

    else
    {
        sEngProtocolParameter_t param;
        float rate = 0.0f;
        PV624->getVentRate(&rate);
        param.floatValue = rate;
        bool statusFlag = sendResponse(&param, 1u);

        if(true == statusFlag)
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
_Pragma("diag_default=Pm017,Pm128")


