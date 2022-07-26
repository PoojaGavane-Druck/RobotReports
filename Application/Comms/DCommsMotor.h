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

#ifndef __DCOMMSMOTOR_H__
#define __DCOMMSMOTOR_H__

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <rtos.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

#include "spi.h"
#include "DBinaryParser.h"

/* Defines and constants ----------------------------------------------------*/
#define MAX_DATA_BUFFER 255u

/* Types --------------------------------------------------------------------*/
typedef enum
{
    eCommStateNone = 0,
    eCommStateTransmit,
    eCommStateWaitForDrdy,
    eCommStateReceive,
    eCommStateParse
} eCommStateMotor_t;

typedef enum : uint32_t
{
    eAcclAlpha = 0,
    eAcclBeta,
    eDecelAlpha,
    eDecelBeta
} eAcclDecelParam_t;

typedef enum : uint32_t
{
    eHoldCurrent = 0,
    eRunCurrent,
    eAcclCurrent,
    eDecelCurrent
} eCurrentParam_t;

/* Variables ----------------------------------------------------------------*/

class DCommsMotor
{
private:

    spi *motorSpi;
    DBinaryParser *masterParser;
    eCommStateMotor_t state;
    uint32_t sendReceiveFlag;
    uint32_t txComplete;
    uint32_t rxComplete;
    uint32_t dataReady;
    uint32_t commandLen;
    uint8_t myTxData[MAX_DATA_BUFFER];
    uint8_t myRxData[MAX_DATA_BUFFER];

    sError_t resetDataBuffers(void);

protected:

public:
    DCommsMotor(SPI_HandleTypeDef *spiHandle);
    ~DCommsMotor();
    // Used for functional transactions
    sError_t sendCommand(uint8_t cmd, uint8_t *data,  uint8_t *rxData, uint32_t *stpError);
    // Overloaded for FW upgrade data transacations
    sError_t sendCommand(uint8_t cmd,
                         uint8_t *data,
                         uint8_t dataLen,
                         uint8_t *rxData,
                         uint8_t rxLength,
                         uint32_t spiTimeoutSendCmd,
                         uint32_t *stpError);

    sError_t sendReceive(uint8_t *rxData, uint32_t *stpError);

    sError_t sendReceive(uint8_t *rxData,
                         uint8_t rxLength,
                         uint16_t txLength,
                         uint32_t spiTimeoutSendRcv,
                         uint32_t *stpError);

    /* Public communications functions */
    sError_t query(uint8_t cmd, uint8_t *txData, uint8_t *rxData, uint32_t *stpError);
    sError_t query(uint8_t cmd,
                   uint8_t *txData,
                   uint8_t txDataLen,
                   uint8_t *rxData,
                   uint8_t rxLength,
                   uint32_t spiTimeoutQuery,
                   uint32_t *stpError);
};

#endif /* DCommsMotor.h*/
