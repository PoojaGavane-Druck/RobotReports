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
* @file		DCommsMotor.c
* @version	1.00.00
* @author	Makarand Deshmukh
* @date		04-09-2021
*
* @brief	
*/

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

#include "DCommsMotor.h"
#include "DPV624.h"
#include "main.h"
#include "spi.h"
#include "DBinaryParser.h"

/* Defines and constants ----------------------------------------------------*/

/* Types --------------------------------------------------------------------*/

/* Global Variables ---------------------------------------------------------*/

/* File Statics -------------------------------------------------------------*/
sCommand_t motorControlCommands[MAX_COMMANDS_SIZE];

/* User Code ----------------------------------------------------------------*/

/**
* @brief	DCommsMotor class constructor
* @param	void
* @retval	void
*/
DCommsMotor::DCommsMotor(SPI_HandleTypeDef *spiHandle)
{
    motorSpi = new spi(spiHandle);
    masterParser = new DBinaryParser((void *)this, &motorControlCommands[0], (size_t)MAX_COMMANDS_SIZE);

    resetDataBuffers();
    state = eCommStateNone;
    sendReceiveFlag = 0u;
    commandLen = 0u;
    txComplete = 0u;
    rxComplete = 0u;
    dataReady = 0u;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
DCommsMotor::~DCommsMotor()
{

}

/**
* @brief	Create motor commands
* @param	void
* @retval	void
*/
void DCommsMotor::createCommands(void)
{
    masterParser->addCommand(eCommandSetParameter,
                                eDataTypeUnsignedLong,
                                fnSetParameter,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandGetParameter,
                                eDataTypeUnsignedLong,
                                fnGetParameter,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);
#ifdef ENABLE_UNUSED_COMMANDS
    masterParser->addCommand(eCommandRun,
                                eDataTypeUnsignedLong,
                                fnRun,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);
#endif
    masterParser->addCommand(eCommandStepClock,
                                eDataTypeUnsignedLong,
                                fnStepClock,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);
#ifdef ENABLE_UNUSED_COMMANDS
    masterParser->addCommand(eCommandMove,
                                eDataTypeUnsignedLong,
                                fnMove,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandGoTo,
                                eDataTypeUnsignedLong,
                                fnGoTo,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandGoToDir,
                                eDataTypeUnsignedLong,
                                fnGoToDir,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandGoUntil,
                                eDataTypeUnsignedLong,
                                fnGoUntil,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReleaseSW,
                                eDataTypeUnsignedLong,
                                fnReleaseSw,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandGoHome,
                                eDataTypeUnsignedLong,
                                fnGoHome,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);   

    masterParser->addCommand(eCommandGoMark,
                                eDataTypeUnsignedLong,
                                fnGoMark,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandResetPos,
                                eDataTypeUnsignedLong,
                                fnResetPos,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);
#endif
    masterParser->addCommand(eCommandResetDevice,
                                eDataTypeUnsignedLong,
                                fnResetDevice,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

#ifdef ENABLE_UNUSED_COMMANDS
    masterParser->addCommand(eCommandSoftStop,
                                eDataTypeUnsignedLong,
                                fnSoftStop,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandHardStop,
                                eDataTypeUnsignedLong,
                                fnHardStop,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandSoftHiZ,
                                eDataTypeUnsignedLong,
                                fnSoftHiZ,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandHardHiZ,
                                eDataTypeUnsignedLong,
                                fnHardHiZ,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

#endif
    masterParser->addCommand(eCommandGetStatus,
                                eDataTypeUnsignedLong,
                                fnGetStatus,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandMoveContinuous,
                                eDataTypeSignedLong,
                                fnMoveContinuous,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadStepCount,
                                eDataTypeSignedLong,
                                fnReadStepCount,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH); 

    masterParser->addCommand(eCommandWriteRegister,
                                eDataTypeUnsignedLong,
                                fnWriteRegister,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadRegister,
                                eDataTypeUnsignedLong,
                                fnReadRegister,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandWriteAcclAlpha,
                                eDataTypeUnsignedLong,
                                fnWriteAcclAlpha,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandWriteAcclBeta,
                                eDataTypeUnsignedLong,
                                fnWriteAcclBeta,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandWriteDecelAlpha,
                                eDataTypeUnsignedLong,
                                fnWriteDecelAlpha,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandWriteDecelBeta,
                                eDataTypeUnsignedLong,
                                fnWriteDecelBeta,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadAcclAlpha,
                                eDataTypeUnsignedLong,
                                fnReadAcclAlpha,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadAcclBeta,
                                eDataTypeUnsignedLong,
                                fnReadAcclBeta,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadDecelAlpha,
                                eDataTypeUnsignedLong,
                                fnReadDecelAlpha,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadDecelBeta,
                                eDataTypeUnsignedLong,
                                fnReadDecelBeta,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandMinimumSpeed,
                                eDataTypeUnsignedLong,
                                fnWriteMinimumSpeed,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);
#ifdef ENABLE_UNUSED_COMMANDS
    masterParser->addCommand(eCommandMaximumSpeed,
                                eDataTypeUnsignedLong,
                                fnWriteMaximumSpeed,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandWatchdogEnable,
                                eDataTypeUnsignedLong,
                                fnWatchdogEnable,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadVersionInfo,
                                eDataTypeUnsignedLong,
                                fnReadVersionInfo,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);
#endif
    masterParser->addCommand(eCommandResetController,
                                eDataTypeUnsignedLong,
                                fnResetController,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

#ifdef DIFFERENT_CURRENTS
    masterParser->addCommand(eCommandWriteHoldCurrent,
                                eDataTypeUnsignedLong,
                                fnWriteHoldCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandWriteRunCurrent,
                                eDataTypeUnsignedLong,
                                fnWriteRunCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandWriteAcclCurrent,
                                eDataTypeUnsignedLong,
                                fnWriteAcclCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandWriteDecelCurrent,
                                eDataTypeUnsignedLong,
                                fnWriteDecelCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadHoldCurrent,
                                eDataTypeUnsignedLong,
                                fnReadHoldCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadRunCurrent,
                                eDataTypeUnsignedLong,
                                fnReadRunCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadAcclCurrent,
                                eDataTypeUnsignedLong,
                                fnReadAcclCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadDecelCurrent,
                                eDataTypeUnsignedLong,
                                fnReadDecelCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadSpeedAndCurrent,
                                eDataTypeUnsignedLong,
                                fnReadSpeedAndCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);
#endif
    
#ifndef DIFFERENT_CURRENTS
    masterParser->addCommand(eCommandWriteHoldCurrent,
                                eDataTypeUnsignedLong,
                                fnWriteCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);

    masterParser->addCommand(eCommandReadHoldCurrent,
                                eDataTypeUnsignedLong,
                                fnReadCurrent,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);    
#endif    
    masterParser->addCommand(eCommandFwUpgrade,
                                eDataTypeUnsignedLong,
                                fnFwUpgrade,
                                DEFAULT_CMD_DATA_LENGTH,
                                DEFAULT_RESPONSE_DATA_LENGTH);                                                                                                                                                                                                                                                                                                                                                                                                                             
}

/**
* @brief	sends command and receives response, parse the response 
* @param	rxData  pointer to buffer to return the command exection response data 
* @retval	sError_t  command execution error status
*/
sError_t DCommsMotor::sendReceive(uint8_t *rxData)
{
    sError_t error;
    error.value = 0u;
    GPIO_PinState drdyPin = GPIO_PIN_RESET;
      
    sendReceiveFlag = 1u;
    /* Transmit data and wait for either timeout or pass */
    if((myTxData != NULL) && (rxData != NULL))
    {
        if(motorSpi != NULL)
        {
            txComplete = motorSpi->transmit(&myTxData[0], (uint8_t)(commandLen));
            if(1u == txComplete)
            {
                txComplete = 0u;                
                dataReady = motorSpi->getDataReady();
                if(1u == dataReady)
                {
                    /* Data is ready to be received */
                    dataReady = 0u;
                    /* Check if data ready has become low 
                    in this case, the master must not transmit the receive command */
                    drdyPin = HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0);
                    
                    if(GPIO_PIN_SET == (uint32_t)(drdyPin))
                    {
                        rxComplete = motorSpi->receive(&myRxData[0], (uint8_t)(commandLen));
                        if(1u == rxComplete)
                        {
                            /* Data is ready to be received */
                            rxComplete = 0u;
                            masterParser->parse(myRxData, commandLen, &error.value, 1u, rxData);
                            state = eCommStateNone;
                        }
                        else
                        {
                            /* There was no response from slave */
                            state = eCommStateNone;
                        }
                    }
                    else
                    {
                        rxData[0] = (uint8_t)(0xFF);
                        rxData[1] = (uint8_t)(0xFF);
                        rxData[2] = (uint8_t)(0xFF);
                        rxData[3] = (uint8_t)(0xFF);
                    }
                }
                else
                {
                    /* There was no response from slave */
                    state = eCommStateNone;
                }
            }
            else
            {
                /* There was a timeout and data could not be sent */
                state = eCommStateNone;
            }  
        }
    }
    return error;
}

/**
* @brief	resets the data buffers
* @param	void
* @retval	void
*/
sError_t DCommsMotor::resetDataBuffers(void)
{
    sError_t error;
    error.value = 0u;
    uint32_t counter = 0u;

    for(counter = 0u; counter < (uint32_t)(MAX_DATA_BUFFER); counter++)
    {
        myRxData[counter] = (uint8_t)(0);
        myTxData[counter] = (uint8_t)(0);
    }
    
    return error;
}

/**
* @brief	This function sends a command and receives the reposnse
* @param	cmd  command code
* @param	*txData  Pointer to the transmission data buffer
* @param	*rxData  pointer to the receive data buffer
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::query(uint8_t cmd, uint8_t *txData, uint8_t *rxData)
{
    sError_t error;

    error.value = 0u;

    sendCommand(cmd, txData, rxData);

    return error;
}

/**
* @brief	This function sends a command and receives the reposnse
* @param	cmd  command code
* @param	*txData  Pointer to the transmission data buffer
* @param	*rxData  pointer to the receive data buffer
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::sendCommand(uint8_t cmd, uint8_t *data, uint8_t *rxData)
{
    sError_t error;
    uint8_t cmdDataSize = (uint8_t)(4);
    uint8_t txLen = (uint8_t)(9);

    error.value = 0u;
    commandLen = txLen;

    if(masterParser != NULL)
    {
        masterParser->prepareTxMessage(cmd, data, cmdDataSize, myTxData, (uint16_t)(txLen));
    }
    sendReceive(rxData);
    return error;
}

/**
* @brief	This function to set the requested motor parameter
 * @param       instance is a pointer to the DCommsMotor object
 * @param       parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnSetParameter(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnSetParameter(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for set parameter command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnSetParameter(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to get the requested motor parameter
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnGetParameter(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnGetParameter(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for get motor parameter command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnGetParameter(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

#ifdef UNUSED_FUNCTIONS
/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnRun(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnRun(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnRun(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}
#endif

/**
* @brief	This function to configure motor step clock
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnStepClock(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnStepClock(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for configure motor step clock command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnStepClock(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

#ifdef UNUSED_FUNCTIONS
/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnMove(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnMove(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnMove(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoTo(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnGoTo(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoTo(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoToDir(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnGoToDir(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoToDir(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoUntil(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnGoUntil(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoUntil(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnReleaseSw(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReleaseSw(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnReleaseSw(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoHome(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnGoHome(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoHome(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoMark(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnGoMark(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnGoMark(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnResetPos(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnResetPos(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnResetPos(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}
#endif

/**
* @brief	This function to reset the micro controller
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnResetDevice(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnResetDevice(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for resetting micro controller
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnResetDevice(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

#ifdef UNUSED_FUNCTIONS
/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnSoftStop(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnSoftStop(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnSoftStop(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnHardStop(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnHardStop(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnHardStop(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnSoftHiZ(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnSoftHiZ(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnSoftHiZ(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnHardHiZ(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnHardHiZ(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnHardHiZ(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}
#endif

/**
* @brief	This function to get the status
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnGetStatus(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnGetStatus(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for get status command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnGetStatus(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to move the motor
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnMoveContinuous(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnMoveContinuous(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for motor move command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnMoveContinuous(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}


/**
* @brief	This function to read the step count
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadStepCount(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadStepCount(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read step count command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadStepCount(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to write a value into requested register
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteRegister(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteRegister(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for writeRegister command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteRegister(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to read requested register
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadRegister(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadRegister(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read register command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadRegister(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to write acceleration alpha
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteAcclAlpha(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteAcclAlpha(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for writeAccelrationAlpha command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteAcclAlpha(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to write acceleration beta command
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteAcclBeta(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteAcclBeta(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for write acceleration beta command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteAcclBeta(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to write decelaration Alpha command
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteDecelAlpha(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteDecelAlpha(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for write Deceleration Alpha command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteDecelAlpha(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to write deceleration beta alue
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteDecelBeta(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteDecelBeta(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for write Decceleration beta  command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteDecelBeta(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to read Acceleration Alpha
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadAcclAlpha(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadAcclAlpha(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler to read acceleration alpha alue
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadAcclAlpha(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to read Acceleration beta
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadAcclBeta(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadAcclBeta(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for Read Acceleration beta command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadAcclBeta(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to read decceleration Alpha value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadDecelAlpha(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadDecelAlpha(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for readDeccelerationAlpha command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadDecelAlpha(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}


/**
* @brief	This function to read deceleration beta value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadDecelBeta(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadDecelBeta(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read Decceleration beta command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadDecelBeta(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to write motor minimumm speed value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteMinimumSpeed(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteMinimumSpeed(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for write motor minimum speed command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteMinimumSpeed(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

#ifdef UNUSED_FUNCTIONS
/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnWriteMaximumSpeed(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteMaximumSpeed(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnWriteMaximumSpeed(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnWatchdogEnable(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWatchdogEnable(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
* @brief	DCommsMotor class destructor
* @param	void
* @retval	void
*/
sError_t DCommsMotor::fnWatchdogEnable(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}
#endif

/**
* @brief	This function to read version information
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadVersionInfo(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadVersionInfo(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read version information command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadVersionInfo(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to reset controller
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnResetController(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnResetController(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for reset controller command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnResetController(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

#ifdef DIFFERENT_CURRENTS
/**
* @brief	This function to write hold current
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteHoldCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteHoldCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for write hold current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteHoldCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to write run current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteRunCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteRunCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for write run current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteRunCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to write acceleration current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteAcclCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteAcclCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for write acceleration current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteAcclCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to write decelration current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteDecelCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteDecelCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for writeDecceleration current coomand
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteDecelCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to read hold current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadHoldCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadHoldCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read hold current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadHoldCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}



/**
* @brief	This function to read run current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadRunCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadRunCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read run current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadRunCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to read acceleration current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadAcclCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadAcclCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read Acceleration current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadAcclCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to read deceleration current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadDecelCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadDecelCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read deceleration current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadDecelCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}
#endif

#ifndef DIFFERENT_CURRENTS

/**
* @brief	This function to write current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnWriteCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnWriteCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for write current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnWriteCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to read current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

#endif

/**
* @brief	This function to read speed and current value
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnReadSpeedAndCurrent(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnReadSpeedAndCurrent(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for read speed and current command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnReadSpeedAndCurrent(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}

/**
* @brief	This function to upgrade firmware
* @param        instance is a pointer to the DCommsMotor object
* @param        parameterArray is the array of received command parameters
* @retval	sError_t command execution error status
*/
sError_t DCommsMotor::fnFwUpgrade(void* instance, sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    DCommsMotor* myInstance = (DCommsMotor*)instance;

    if (myInstance != NULL)
    {
        error = myInstance->fnFwUpgrade(parameterArray);
    }
    else
    {
        error.unhandledMessage = 1u;
    }

    return error;
}

/**
 * @brief   handler for firmware upgrade command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sError_t DCommsMotor::fnFwUpgrade(sParameter_t* parameterArray)
{
    sError_t error;
    error.value = 0u;

    return error;
}
