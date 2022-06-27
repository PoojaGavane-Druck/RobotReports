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

    /* Command execution function pointers */
    static sError_t fnSetParameter(void *instance, sParameter_t *parameterArray);
    static sError_t fnGetParameter(void *instance, sParameter_t *parameterArray);
#ifdef UNUSED_FUNCTIONS
    static sError_t fnRun(void *instance, sParameter_t *parameterArray);
#endif
    static sError_t fnStepClock(void *instance, sParameter_t *parameterArray);
#ifdef UNUSED_FUNCTIONS
    static sError_t fnMove(void *instance, sParameter_t *parameterArray);
    static sError_t fnGoTo(void *instance, sParameter_t *parameterArray);
    static sError_t fnGoToDir(void *instance, sParameter_t *parameterArray);
    static sError_t fnGoUntil(void *instance, sParameter_t *parameterArray);
    static sError_t fnReleaseSw(void *instance, sParameter_t *parameterArray);
    static sError_t fnGoHome(void *instance, sParameter_t *parameterArray);
    static sError_t fnGoMark(void *instance, sParameter_t *parameterArray);
    static sError_t fnResetPos(void *instance, sParameter_t *parameterArray);
#endif
    static sError_t fnResetDevice(void *instance, sParameter_t *parameterArray);
#ifdef UNUSED_FUNCTIONS
    static sError_t fnSoftStop(void *instance, sParameter_t *parameterArray);
    static sError_t fnHardStop(void *instance, sParameter_t *parameterArray);
    static sError_t fnSoftHiZ(void *instance, sParameter_t *parameterArray);
    static sError_t fnHardHiZ(void *instance, sParameter_t *parameterArray);
#endif
    static sError_t fnGetStatus(void *instance, sParameter_t *parameterArray);
    static sError_t fnMoveContinuous(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadStepCount(void *instance, sParameter_t *parameterArray);
    static sError_t fnWriteRegister(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadRegister(void *instance, sParameter_t *parameterArray);
    static sError_t fnWriteAcclAlpha(void *instance, sParameter_t *parameterArray);
    static sError_t fnWriteAcclBeta(void *instance, sParameter_t *parameterArray);
    static sError_t fnWriteDecelAlpha(void *instance, sParameter_t *parameterArray);
    static sError_t fnWriteDecelBeta(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadAcclAlpha(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadAcclBeta(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadDecelAlpha(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadDecelBeta(void *instance, sParameter_t *parameterArray);
    static sError_t fnWriteMinimumSpeed(void *instance, sParameter_t *parameterArray);
#ifdef UNUSED_FUNCTIONS
    static sError_t fnWriteMaximumSpeed(void *instance, sParameter_t *parameterArray);
    static sError_t fnWatchdogEnable(void *instance, sParameter_t *parameterArray);
#endif
    static sError_t fnReadVersionInfo(void *instance, sParameter_t *parameterArray);
    static sError_t fnResetController(void *instance, sParameter_t *parameterArray);

#ifdef DIFFERENT_CURRENTS
    static sError_t fnWriteHoldCurrent(void *instance, sParameter_t *parameterArray);
    static sError_t fnWriteRunCurrent(void *instance, sParameter_t *parameterArray);
    static sError_t fnWriteAcclCurrent(void *instance, sParameter_t *parameterArray);
    static sError_t fnWriteDecelCurrent(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadHoldCurrent(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadRunCurrent(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadAcclCurrent(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadDecelCurrent(void *instance, sParameter_t *parameterArray);
#endif
    static sError_t fnReadSpeedAndCurrent(void *instance, sParameter_t *parameterArray);
    static sError_t fnFwUpgrade(void *instance, sParameter_t *parameterArray);
#ifndef DIFFERENT_CURRENTS
    static sError_t fnWriteCurrent(void *instance, sParameter_t *parameterArray);
    static sError_t fnReadCurrent(void *instance, sParameter_t *parameterArray);
#endif
    sError_t resetDataBuffers(void);
    sError_t stateMachine(void);
    void createCommands(void);

protected:

public:
    DCommsMotor(SPI_HandleTypeDef *spiHandle);
    ~DCommsMotor();
    sError_t sendCommand(uint8_t cmd, uint8_t *data,  uint8_t *rxData);
    sError_t sendCommand(uint8_t cmd, uint8_t *data, uint8_t dataLen, uint8_t *rxData, uint8_t rxLength, uint32_t spiTimeoutSendCmd);

    sError_t sendReceive(uint8_t *rxData);

    /* Callbacks for function pointers */
    sError_t fnSetParameter(sParameter_t *parameterArray);
    sError_t fnGetParameter(sParameter_t *parameterArray);
#ifdef UNUSED_FUNCTIONS
    sError_t fnRun(sParameter_t *parameterArray);
#endif
    sError_t fnStepClock(sParameter_t *parameterArray);
#ifdef UNUSED_FUNCTIONS
    sError_t fnMove(sParameter_t *parameterArray);
    sError_t fnGoTo(sParameter_t *parameterArray);
    sError_t fnGoToDir(sParameter_t *parameterArray);
    sError_t fnGoUntil(sParameter_t *parameterArray);
    sError_t fnReleaseSw(sParameter_t *parameterArray);
    sError_t fnGoHome(sParameter_t *parameterArray);
    sError_t fnGoMark(sParameter_t *parameterArray);
    sError_t fnResetPos(sParameter_t *parameterArray);
#endif
    sError_t fnResetDevice(sParameter_t *parameterArray);
#ifdef UNUSED_FUNCTIONS
    sError_t fnSoftStop(sParameter_t *parameterArray);
    sError_t fnHardStop(sParameter_t *parameterArray);
    sError_t fnSoftHiZ(sParameter_t *parameterArray);
    sError_t fnHardHiZ(sParameter_t *parameterArray);
#endif
    sError_t fnGetStatus(sParameter_t *parameterArray);
    sError_t fnMoveContinuous(sParameter_t *parameterArray);
    sError_t fnReadStepCount(sParameter_t *parameterArray);
    sError_t fnWriteRegister(sParameter_t *parameterArray);
    sError_t fnReadRegister(sParameter_t *parameterArray);
    sError_t fnWriteAcclAlpha(sParameter_t *parameterArray);
    sError_t fnWriteAcclBeta(sParameter_t *parameterArray);
    sError_t fnWriteDecelAlpha(sParameter_t *parameterArray);
    sError_t fnWriteDecelBeta(sParameter_t *parameterArray);
    sError_t fnReadAcclAlpha(sParameter_t *parameterArray);
    sError_t fnReadAcclBeta(sParameter_t *parameterArray);
    sError_t fnReadDecelAlpha(sParameter_t *parameterArray);
    sError_t fnReadDecelBeta(sParameter_t *parameterArray);
    sError_t fnWriteMinimumSpeed(sParameter_t *parameterArray);
#ifdef UNUSED_FUNCTIONS
    sError_t fnWriteMaximumSpeed(sParameter_t *parameterArray);
    sError_t fnWatchdogEnable(sParameter_t *parameterArray);
#endif
    sError_t fnReadVersionInfo(sParameter_t *parameterArray);
    sError_t fnResetController(sParameter_t *parameterArray);
#ifdef DIFFERENT_CURRENTS
    sError_t fnWriteHoldCurrent(sParameter_t *parameterArray);
    sError_t fnWriteRunCurrent(sParameter_t *parameterArray);
    sError_t fnWriteAcclCurrent(sParameter_t *parameterArray);
    sError_t fnWriteDecelCurrent(sParameter_t *parameterArray);
    sError_t fnReadHoldCurrent(sParameter_t *parameterArray);
    sError_t fnReadRunCurrent(sParameter_t *parameterArray);
    sError_t fnReadAcclCurrent(sParameter_t *parameterArray);
    sError_t fnReadDecelCurrent(sParameter_t *parameterArray);
#endif
#ifndef DIFFERENT_CURRENTS
    sError_t fnWriteCurrent(sParameter_t *parameterArray);
    sError_t fnReadCurrent(sParameter_t *parameterArray);
#endif
    sError_t fnReadSpeedAndCurrent(sParameter_t *parameterArray);
    sError_t fnFwUpgrade(sParameter_t *parameterArray);
    sError_t sendReceive(uint8_t *rxData, uint8_t rxLength, uint16_t txLength, uint32_t spiTimeoutSendRcv);

    /* Public communications functions */
    sError_t query(uint8_t cmd, uint8_t *txData, uint8_t *rxData);
    sError_t query(uint8_t cmd, uint8_t *txData, uint8_t txDataLen, uint8_t *rxData, uint8_t rxLength, uint32_t spiTimeoutQuery);
};

#endif /* DCommsMotor.h*/
