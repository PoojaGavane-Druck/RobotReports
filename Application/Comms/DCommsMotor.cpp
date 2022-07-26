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
* @file     DCommsMotor.c
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     04-09-2021
*
* @brief
*/

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <rtos.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

#include "DCommsMotor.h"
#include "DPV624.h"
#include "main.h"
#include "spi.h"
#include "DBinaryParser.h"

/* Error handler instance parameter starts from 801 to 900 */
/* Defines and constants ----------------------------------------------------*/

/* Types --------------------------------------------------------------------*/

/* Global Variables ---------------------------------------------------------*/

/* File Statics -------------------------------------------------------------*/
sCommand_t motorControlCommands[MAX_COMMANDS_SIZE];

/* User Code ----------------------------------------------------------------*/

/**
* @brief    DCommsMotor class constructor
* @param    void
* @retval   void
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
* @brief    DCommsMotor class destructor
* @param    void
* @retval   void
*/
DCommsMotor::~DCommsMotor()
{

}

/**
* @brief    sends command and receives response, parse the response
* @param    rxData  pointer to buffer to return the command exection response data
            stpError - pointer to hold if there is any returned error value from the stepper controller
* @retval   sError_t  command execution error status
*/
sError_t DCommsMotor::sendReceive(uint8_t *rxData, uint32_t *stpError)
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
                    drdyPin = HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_0);

                    if(GPIO_PIN_SET == (uint32_t)(drdyPin))
                    {
                        rxComplete = motorSpi->receive(&myRxData[0], (uint8_t)(commandLen));

                        if(1u == rxComplete)
                        {
                            /* Data is ready to be received */
                            rxComplete = 0u;
                            error = masterParser->parse(myRxData, commandLen, &error.value, 1u, rxData, stpError);
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
                        rxData[0] = 0xFFu;
                        rxData[1] = 0xFFu;
                        rxData[2] = 0xFFu;
                        rxData[3] = 0xFFu;
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
* @brief    resets the data buffers
* @param    void
* @retval   void
*/
sError_t DCommsMotor::resetDataBuffers(void)
{
    sError_t error;
    error.value = 0u;
    uint32_t counter = 0u;

    for(counter = 0u; counter < (uint32_t)(MAX_DATA_BUFFER); counter++)
    {
        myRxData[counter] = 0u;
        myTxData[counter] = 0u;
    }

    return error;
}

/**
* @brief    This function sends a command and receives the reposnse
* @param    cmd  command code
* @param    *txData  Pointer to the transmission data buffer
* @param    *rxData  pointer to the receive data buffer
* @retval   sError_t command execution error status
*/
sError_t DCommsMotor::query(uint8_t cmd, uint8_t *txData, uint8_t *rxData, uint32_t *stpError)
{
    sError_t error;

    error.value = 0u;

    error = sendCommand(cmd, txData, rxData, stpError);

    return error;
}

/**
* @brief This function sends a command and receives the reposnse
* @param cmd command code
* @param *txData Pointer to the transmission data buffer
* @param *rxData pointer to the receive data buffer
* @retval sError_t command execution error status
*/
sError_t DCommsMotor::query(uint8_t cmd,
                            uint8_t *txData,
                            uint8_t txDataLen,
                            uint8_t *rxData,
                            uint8_t rxLength,
                            uint32_t spiTimeoutQuery,
                            uint32_t *stpError)
{
    sError_t error;
    error.value = 0u;

    error = sendCommand(cmd, txData, txDataLen, rxData, rxLength, spiTimeoutQuery, stpError);

    return error;
}

/**
* @brief    This function sends a command and receives the reposnse
* @param    cmd  command code
* @param    *txData  Pointer to the transmission data buffer
* @param    *rxData  pointer to the receive data buffer
* @retval   sError_t command execution error status
*/
sError_t DCommsMotor::sendCommand(uint8_t cmd, uint8_t *data, uint8_t *rxData, uint32_t *stpError)
{
    sError_t error;
    uint8_t cmdDataSize = 4u;
    uint8_t txLen = 11u;

    error.value = 0u;
    commandLen = txLen;

    if(masterParser != NULL)
    {
        masterParser->prepareTxMessage(cmd, data, cmdDataSize, myTxData, (uint16_t)(txLen));
    }

    error = sendReceive(rxData, stpError);
    return error;
}

/**
* @brief    This function sends a command and receives the reposnse - Used for SPI firmware upgrade for secondary uC
* @param    cmd  command code
* @param    *txData  Pointer to the transmission data buffer
* @param    *rxData  pointer to the receive data buffer
* @retval   sError_t command execution error status
*/
sError_t DCommsMotor::sendCommand(uint8_t cmd,
                                  uint8_t *data,
                                  uint8_t dataLen,
                                  uint8_t *rxData,
                                  uint8_t rxLength,
                                  uint32_t spiTimeoutSendCmd,
                                  uint32_t *stpError)
{
    sError_t error;
    uint8_t cmdDataSize = dataLen;
    uint16_t txLen = 0u;        // This is used to get total length including header and crc from prepareTxMessage
    error.value = 0u;

    if(masterParser != NULL)
    {
        masterParser->prepareTxMessage(cmd, data, cmdDataSize, myTxData, &txLen);
    }

    error = sendReceive(rxData, rxLength, txLen, spiTimeoutSendCmd, stpError);
    return error;
}

/**
* @brief    sends command and receives response, parse the response
* @param    rxData  pointer to buffer to return the command exection response data
* @retval   sError_t  command execution error status
*/
sError_t DCommsMotor::sendReceive(uint8_t *rxData,
                                  uint8_t rxLength,
                                  uint16_t txLength,
                                  uint32_t spiTimeoutSendRcv,
                                  uint32_t *stpError)
{
    sError_t error;
    error.value = 0u;
    GPIO_PinState drdyPin = GPIO_PIN_RESET;

    /* Transmit data and wait for either timeout or pass */
    if((myTxData != NULL) && (rxData != NULL))
    {
        if(motorSpi != NULL)
        {
            txComplete = motorSpi->transmit(&myTxData[0], (uint8_t)(txLength));

            if(1u == txComplete)
            {
                txComplete = 0u;
                dataReady = motorSpi->getDataReady(spiTimeoutSendRcv);

                if(1u == dataReady)
                {
                    /* Data is ready to be received */
                    dataReady = 0u;
                    /* Check if data ready has become low
                    in this case, the master must not transmit the receive command */
                    drdyPin = HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_0);

                    if(GPIO_PIN_SET == (uint32_t)(drdyPin))
                    {
                        rxComplete = motorSpi->receive(&myRxData[0], (uint8_t)(rxLength), spiTimeoutSendRcv);

                        if(1u == rxComplete)
                        {
                            /* Data is ready to be received */
                            rxComplete = 0u;
                            error = masterParser->parse(myRxData, (uint32_t)rxLength, &error.value, 1u, rxData, stpError);
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
