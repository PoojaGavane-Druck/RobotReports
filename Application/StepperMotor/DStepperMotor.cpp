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
* @file     DComms.cpp
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     27-Feb-2021
*
* @brief    Binary Parser for Stepper Motor Source File
*/

/* Includes -----------------------------------------------------------------*/
#include "DStepperMotor.h"
#include "DBinaryParser.h"
#include "DDeviceSerialSlaveMicroController.h"

#define MAX_CURRENT 2.0f
/**
 * @brief   Constructor
 * @param   void
 * @return  void
 */
DStepperMotor::DStepperMotor()
{
    myComms = (DDeviceSerial*) new DDeviceSerialSlaveMicroController();
    myTxBuffer = (uint8_t*)myComms->getTxBuffer();
    myTxBufferSize = myComms->getTxBufferSize();
    /* Add all the commands here */
}

/**
 * @brief   Destructor
 * @param   void
 * @return  void
 */
DStepperMotor::~DStepperMotor()
{
    
}

/*
 * @brief   Send query command Owi sensor
 * @param   command string
 * @return  sensor error code
 */
eIbcError_t DStepperMotor::sendCommand(uint8_t cmd, uint8_t* cmdData, uint8_t cmdDataLength)
{
    eIbcError_t commError = E_IBC_ERROR_NONE;

    sError_t error;
    error.value = 0u;
    uint8_t* buffer;
    uint32_t responseLength = 0u;
    uint16_t cmdLength;
    uint32_t errorCode = 0u;
    //prepare the message for transmission
    myParser->prepareTxMessage(cmd, cmdData, cmdDataLength,(uint8_t*)myTxBuffer, &cmdLength);

    myParser->getResponseLength(cmd, &responseLength);

    if (myComms->query(myTxBuffer, cmdLength, &buffer, responseLength, commandTimeoutPeriod) == true)
    {
        if ((uint32_t)(0) == responseLength)
        {
            myComms->getRcvBufLength((uint16_t*)(&responseLength));
        }
        error = myParser->parse(cmd, buffer, responseLength, &errorCode);

        //if this transaction is ok, then we can use the received value
        if (error.value != 0u)
        {
            if (1u == error.nackReceived)
            {
                commError = (eIbcError_t)errorCode;
            }
            else
            {
                commError = E_IBC_ERROR_COMMAND;
            }
            
        }
    }
    else
    {
        commError = E_IBC_ERROR_COMMS;
    }

    return commError;
}



/**
 * @brief   This function reads the value of acceleration alpha
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeAcclAlpha(float32_t newAcclAlpha)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uFloat_t value;
    value.floatValue = newAcclAlpha;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_WriteAcclAlpha, &value.byteValue[0], (uint8_t)4);
    return errorStatus;
}
/**
 * @brief   This function writes the value of acceleration alpha 
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAcclAlpha(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor*instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetAcclAlpha(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the value of acceleration alpha 
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeAcclBeta(float32_t newAccBeta)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uFloat_t value;
    value.floatValue = newAccBeta;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_WriteAcclBeta, &value.byteValue[0], (uint8_t)4);
    return errorStatus;
}
/**
 * @brief   This function writes the value of acceleration beta  
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAcclBeta(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor*instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetAcclBeta(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the value of decceleration alpha 
 * @param   float
 * @return  void
 */
eIbcError_t DStepperMotor::writeDecclAlpha(float32_t newDecclAlpha)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uFloat_t value;
    value.floatValue = newDecclAlpha;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_WriteDecclAlpha, &value.byteValue[0],  (uint8_t)4);
    return errorStatus;
}
/**
 * @brief   This function writes the value of Deceleration alpha 
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetDecclAlpha(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor*instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetDecclAlpha(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the value of acceleration beta 
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeDecclBeta(float32_t newDecclBeta)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uFloat_t value;
    value.floatValue = newDecclBeta;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_WriteDecclBeta, &value.byteValue[0],  (uint8_t)4);
    return errorStatus;
}
/**
 * @brief   This function writes the value of deceleration beta 
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetDecclBeta(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetDecclBeta(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   This function reads the value of acceleration alpha 
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readAcclAlpha(float32_t* acclAlpha)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadAcclAlpha, NULL,(uint8_t) 0);
    return errorStatus;
}
/**
 * @brief   This function reads the value of acceleration alpha 
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetAcclAlpha(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor*instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetAcclAlpha(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the value of acceleration Beta 
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readAcclBeta(float32_t* acclBeta)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadAcclBeta, NULL, (uint8_t)0);
    return errorStatus;
}
/**
 * @brief   This function reads the value of acceleration beta rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetAcclBeta(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetAcclBeta(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the value of deceleration alpha 
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readDecclAlpha(float32_t* decclAlpha)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadDecclAlpha, NULL, (uint8_t)0);
    return errorStatus;
}
/**
 * @brief   This function reads the value of deceleration alpha 
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetDecclAlpha(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor* instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetDecclAlpha(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the value of deceleration beta 
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readDecclBeta(float32_t* decclBeta)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadDecclBeta, NULL,  (uint8_t)0);
    return errorStatus;
}
/**
 * @brief   This function read the value of deceleration beta rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetDecclBeta(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor* instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetDecclBeta(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function moves the motor by a certain number of steps
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeMoveContinuous(uint32_t newCount)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uUint32_t value;
    value.uint32Value = newCount;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_MoveContinuous, &value.byteValue[0], (uint8_t) 4);
    return errorStatus;
}
/**
 * @brief   This function moves the motor by a certain number of steps
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetMoveContinuous(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor* instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetMoveContinuous(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the value of current step count
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readStepCount(uint32_t* stepCount)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadStepCount, NULL,  (uint8_t)0);
    return errorStatus;
}
/**
 * @brief   This function reads the steps count completed by the motor since the last command
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetStepCount(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor* instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetStepCount(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function writes the minimum speed at which the motor starts
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeMinimumSpeed(uint32_t newSpeed)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uUint32_t value;
    value.uint32Value = newSpeed;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_MinimumSpeed, &value.byteValue[0], (uint8_t) 4);
    return errorStatus;
}
/**
 * @brief   This function writes the minimum speed at which the motor starts
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetMinimumSpeed(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetMinimumSpeed(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function writes the maximum speed at which the motor starts
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeMaximumSpeed(uint32_t newSpeed)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uUint32_t value;
    value.uint32Value = newSpeed;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_MaximumSpeed, &value.byteValue[0], (uint8_t) 4);
    return errorStatus;
}
/**
 * @brief   This function writes the maximum speed at which the motor runs
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetMaximumSpeed(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetMaximumSpeed(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function write the value of the watchdog timeout
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeWatchdogTimeout(uint32_t newTimeOut)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uUint32_t value;
    value.uint32Value = newTimeOut;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_WatchdogTime, &value.byteValue[0], (uint8_t) 4);
    return errorStatus;
}
/**
 * @brief   This function write the value of the watchdog timeout
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetWatchdogTimeout(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetWatchdogTimeout(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function writes the value of the acceleration timer 
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeAccelerationTime(uint32_t newTime)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uUint32_t value;
    value.uint32Value = newTime;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_AccelerationTime, &value.byteValue[0], (uint8_t) 4);
    return errorStatus;
}
/**
 * @brief   This function writes the value of the acceleration timer
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAccelerationTime(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetAccelerationTime(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function writes the absolute position of the motor to the L6472 register
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeAbsolutePosition(uint32_t newPosition)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uUint32_t value;
    value.uint32Value = newPosition;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_SetAbsPosition, &value.byteValue[0], (uint8_t) 4);
    return errorStatus;
}
/**
 * @brief   This function writes the absolute position of the motor to the L6472 register
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAbsolutePosition(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetAbsolutePosition(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the absolute position of the motor from the L6472 register
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readAbsolutePosition(uint32_t* absPosition)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_GetAbsPosition, NULL, (uint8_t) 0);
    return errorStatus;
}
/**
 * @brief   This function reads the absolute position of the motor from the L6472 register
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetAbsolutePosition(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetAbsolutePosition(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}



/**
 * @brief   This function reads the version information of the application
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readVersionInfo(sVersion_t *ver)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_GetVersionInfo, NULL, (uint8_t) 0);
    return errorStatus;
}
/**
 * @brief   This function reads the version information of the application
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetVersionInfo(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetVersionInfo(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the hold current value
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readHoldCurrent(float32_t* holdCurrent)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadHoldCurrent, NULL, (uint8_t) 0);
    return errorStatus;
}
/**
 * @brief   This function reads the hold current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetHoldCurrent(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetHoldCurrent(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the run current value
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readRunCurrent(float32_t* runCurrent)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadRunCurrent, NULL, (uint8_t) 0);
    return errorStatus;
}
/**
 * @brief   This function reads the run current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetRunCurrent(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetRunCurrent(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the acceleration current value
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readAcclCurrent(float32_t* acclCurrent)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadAcclCurrent, NULL, (uint8_t) 0);
    return errorStatus;
}
/**
 * @brief   This function reads the acceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetAcclCurrent(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetAcclCurrent(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the deceleration current value
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readDecelCurrent(float32_t* decelCur)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadDecelCurrent, NULL, (uint8_t) 0);
    return errorStatus;
}
/**
 * @brief   This function reads the deceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetDecelCurrent(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetDecelCurrent(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   This function writes the hold current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetHoldCurrent(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetHoldCurrent(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   This function writes the run current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetRunCurrent(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetRunCurrent(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   This function writes the acceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAcclCurrent(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetAcclCurrent(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   This function writes the deceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetDecelCurrent(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnSetDecelCurrent(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}
/**
 * @brief   This function reads the value of current step count
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::readSpeedAndCurrent(uint32_t* speed, float32_t current)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_ReadSpeedAndCurrent, NULL, (uint8_t) 0);
    return errorStatus;
}
/**
 * @brief   This function reads the acceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetSpeedAndCurrent(void *parent, sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;

    DStepperMotor *instance = (DStepperMotor*)parent;

    if (instance != NULL)
    {
        error = instance->fnGetSpeedAndCurrent(ptrParam);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/* Callbacks for parent functions -------------------------------------------*/

/**
 * @brief   This function writes the value of acceleration alpha rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAcclAlpha(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.acclAlpha = ptrParam->floatValue;
    return error;

}

/**
 * @brief   This function writes the value of acceleration beta rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAcclBeta(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.acclBeta = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function writes the value of Deceleration alpha rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetDecclAlpha(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.decclAlpha = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function writes the value of deceleration beta rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetDecclBeta(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.decclBeta = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function reads the value of acceleration alpha rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetAcclAlpha(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.acclAlpha = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function reads the value of acceleration beta rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetAcclBeta(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.acclBeta = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function reads the value of deceleration alpha rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetDecclAlpha(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.decclAlpha = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function read the value of deceleration beta rcvd from PC
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetDecclBeta(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.decclBeta = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function moves the motor by a certain number of steps
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetMoveContinuous(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
#if 0
    uint32_t value = (uint32_t)(0);   

    value = GetUint32FromBuffer(buffer);
    
    if((uint32_t)(MOTOR_DIRECTION_BACKWARDS) == (value & (uint32_t)(MOTOR_DIRECTION_BACKWARDS)))
    {
        sApplication.sMotorParms.previousDirection = (uint32_t)(0);
        Motor_SetDirection(eMtrDirectionBackwards);
        sApplication.sMotorParms.steps = (uint32_t)(MAX_STEP_COUNT) - 
                                         (value & (uint32_t)(MAX_STEP_COUNT)) + 
                                         (uint32_t)(1);
    }
    else
    {
        sApplication.sMotorParms.previousDirection = (uint32_t)(1);
        Motor_SetDirection(eMtrDirectionForwards);
        sApplication.sMotorParms.steps = value & (uint32_t)(MAX_STEP_COUNT);
    }

    /* If there are any overrun steps, add them into the total count */

    if((uint32_t)(0) != sApplication.sMotorParms.steps)
    {
        sApplication.sMotorParms.steps = sApplication.sMotorParms.steps + sApplication.sMotorParms.overrunSteps;
        sApplication.sMotorParms.overrunSteps = (uint32_t)(0);
    }    
    sApplication.sMotorParms.motorMoveCommand = (uint32_t)(1);
#else
    motorParams.steps = ptrParam->uiValue;
    motorParams.motorMoveCommand = (uint32_t)(1);
#endif
    return error;
}

/**
 * @brief   This function reads the steps count completed by the motor since the last command
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetStepCount(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.stepsCounter = ptrParam->iValue;
    return error;
}

/**
 * @brief   This function writes the minimum speed at which the motor starts
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetMinimumSpeed(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.minimumSpeed = ptrParam->uiValue;
    return error;
}

/**
 * @brief   This function writes the maximum speed at which the motor runs
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetMaximumSpeed(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.maximumSpeed = ptrParam->uiValue;
    return error;
}

/**
 * @brief   This function write the value of the watchdog timeout
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetWatchdogTimeout(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    watchdogTimeout = ptrParam->uiValue;
    return error;
}

/**
 * @brief   This function writes the value of the acceleration timer
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAccelerationTime(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    //motorParams.accelerationTime = ptrParam->uiValue;
    return error;
}

/**
 * @brief   This function writes the absolute position of the motor to the L6472 register
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAbsolutePosition(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.absoluteStepCounter = ptrParam->iValue;
    return error;
}

/**
 * @brief   This function reads the absolute position of the motor from the L6472 register
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetAbsolutePosition(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.absoluteStepCounter = ptrParam->iValue;
    return error;
}


/**
 * @brief   This function reads the version information of the application
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetVersionInfo(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    
    version.major = ptrParam->byteArray[0];
    version.minor = ptrParam->byteArray[1];
    version.build = ptrParam->byteArray[2];

    return error;
}

/**
 * @brief   This function reads the hold current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetHoldCurrent(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.currHold = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function reads the run current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetRunCurrent(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.currRun = ptrParam->floatValue;
    return error;

}

/**
 * @brief   This function reads the acceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetAcclCurrent(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.currAccl = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function reads the deceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetDecelCurrent(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.currDecel = ptrParam->floatValue;
    return error;
}

/**
 * @brief   This function writes the hold current value
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeHoldCurrent(float32_t value)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uFloat_t currValue;

    if ((float32_t)(value) >= (float32_t)(MAX_CURRENT))
    {
        currValue.floatValue = (float32_t)(MAX_CURRENT);
    }
    else
    {
        currValue.floatValue = (float32_t)(value);
    }
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_WriteHoldCurrent, &currValue.byteValue[0], (uint8_t)4);
    return errorStatus;

}
/**
 * @brief   This function writes the hold current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetHoldCurrent(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.currHold = ptrParam->floatValue;
    return error;
    
}

/**
 * @brief   This function writes the run current value
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeRunCurrent(float32_t value)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uFloat_t currValue;

    if ((float32_t)(value) >= (float32_t)(MAX_CURRENT))
    {
        currValue.floatValue = (float32_t)(MAX_CURRENT);
    }
    else
    {
        currValue.floatValue = (float32_t)(value);
    }
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_WriteRunCurrent, &currValue.byteValue[0], (uint8_t)4);
    return errorStatus;
    
}
/**
 * @brief   This function writes the run current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetRunCurrent(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.currRun = ptrParam->floatValue;
    return error;
}
/**
 * @brief   This function writes the acceleration current value
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeAcclCurrent(float32_t value)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uFloat_t currValue;

    if ((float32_t)(value) >= (float32_t)(MAX_CURRENT))
    {
        currValue.floatValue = (float32_t)(MAX_CURRENT);
    }
    else
    {
        currValue.floatValue = (float32_t)(value);
    }
    
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_WriteAcclCurrent, &currValue.byteValue[0], (uint8_t)4);
    return errorStatus;
}
/**
 * @brief   This function writes the acceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetAcclCurrent(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.currAccl = ptrParam->floatValue;
    return error;
}
/**
 * @brief   This function writes the deceleration current value
 * @param   void
 * @return  void
 */
eIbcError_t DStepperMotor::writeDecelCurrent(float32_t value)
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    uFloat_t currValue;

    if ((float32_t)(value) >= (float32_t)(MAX_CURRENT))
    {
        currValue.floatValue = (float32_t)(MAX_CURRENT);
    }
    else
    {
        currValue.floatValue = (float32_t)(value);
    }
    
    errorStatus = sendCommand(STEPPER_MOTOR_CMD_WriteDecelCurrent, &currValue.byteValue[0], (uint8_t)4);
    return errorStatus;
}

/**
 * @brief   This function writes the deceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnSetDecelCurrent(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    motorParams.currDecel = ptrParam->floatValue;
    return error;
}
/**
 * @brief   This function reads the acceleration current value
 * @param   void
 * @return  void
 */
sError_t DStepperMotor::fnGetSpeedAndCurrent(sParameter_t* ptrParam)
{
    sError_t error;
    error.value = 0u;
    
    motorParams.motorSpeed = myParser->GetUint32FromBuffer(&ptrParam->byteArray[0]);
    motorParams.motorCurrent = myParser->GetFloatFromBuffer(&ptrParam->byteArray[4]);
    return error;
}




