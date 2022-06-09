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
* @file     DBinaryParser.h
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     27-Feb-2021
*
* @brief    Binary Parser for stepper motor communications binary protocol header file
*/
//*********************************************************************************************************************
#ifndef __DBINARY_PARSE_H
#define __DBINARY_PARSE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <rtos.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" /* External C language linkage */
{
#include <lib_ascii.h>
}
#endif /* End of external C language linkage */
MISRAC_ENABLE

#include "Types.h"

/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define MAX_COMMANDS_SIZE 64u
#define DEFAULT_CMD_DATA_LENGTH 4u
#define DEFAULT_RESPONSE_DATA_LENGTH 4u
#define MESSAGE_LENGTH_LIMIT 10u

#define CRC_POLYNOMIAL_8BIT 0x07

#define STEPPER_START_CONDITION 0xFFFF

/* Type Defines -----------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_CMD_NONE = 0,
    E_CMD_READ,
    E_CMD_WRITE
} eCommandType_t;

typedef enum : uint8_t
{
    eCommandNone = 0,                     // 0 - 0x00
    eCommandSetParameter,                 // 1 - 0x01
    eCommandGetParameter,                 // 2 - 0x02
    eCommandRun,                          // 3 - 0x03
    eCommandStepClock,                    // 4 - 0x04
    eCommandMove,                         // 5 - 0x05
    eCommandGoTo,                         // 6 - 0x06
    eCommandGoToDir,                      // 7 - 0x07
    eCommandGoUntil,                      // 8 - 0x08
    eCommandReleaseSW,                    // 9 - 0x09
    eCommandGoHome,                       // 10 - 0x0A
    eCommandGoMark,                       // 11 - 0x0B
    eCommandResetPos,                     // 12 - 0x0C
    eCommandResetDevice,                  // 13 - 0x0D
    eCommandSoftStop,                     // 14 - 0x0E
    eCommandHardStop,                     // 15 - 0x0F
    eCommandSoftHiZ,                      // 16 - 0x10
    eCommandHardHiZ,                      // 17 - 0x11
    eCommandGetStatus,                    // 18 - 0x12
    eCommandMoveContinuous,               // 19 - 0x13
    eCommandReadStepCount,                // 20 - 0x14
    eCommandWriteRegister,                // 21 - 0x15
    eCommandReadRegister,                 // 22 - 0x16
    eCommandWriteAcclAlpha,               // 23 - 0x17
    eCommandWriteAcclBeta,                // 24 - 0x18
    eCommandWriteDecelAlpha,              // 25 - 0x19
    eCommandWriteDecelBeta,               // 26 - 0x1A
    eCommandReadAcclAlpha,                // 27 - 0x1B
    eCommandReadAcclBeta,                 // 28 - 0x1C
    eCommandReadDecelAlpha,               // 29 - 0x1D
    eCommandReadDecelBeta,                // 30 - 0x1E
    eCommandMinimumSpeed,                 // 31 - 0x1F
    eCommandMaximumSpeed,                 // 32 - 0x20
    eCommandWatchdogEnable,               // 33 - 0x21
    eCommandReadVersionInfo,               // 34 - 0x22
    eCommandResetController,              // 35 - 0x23
    eCommandWriteHoldCurrent,             // 36 - 0x24
    eCommandWriteRunCurrent,              // 37 - 0x25
    eCommandWriteAcclCurrent,             // 38 - 0x26
    eCommandWriteDecelCurrent,            // 39 - 0x27
    eCommandReadHoldCurrent,              // 40 - 0x28
    eCommandReadRunCurrent,               // 41 - 0x28
    eCommandReadAcclCurrent,              // 42 - 0x2A
    eCommandReadDecelCurrent,             // 43 - 0x2B
    eCommandReadSpeedAndCurrent,          // 44 - 0x2C
    eCommandFwUpgrade,                     // 45 - 0x2D
    eCommandReadBootVersionInfo          // 46 - 0x2E
} eCommands_t;

typedef union
{
    uint32_t value;

    struct
    {
        uint32_t needsStart             : 1;
        uint32_t unknownCommand         : 1;
        uint32_t invalidResponse        : 1;
        uint32_t nackReceived           : 1;

        uint32_t invalidChecksum        : 1;
        uint32_t messageTooSmall        : 1;
        uint32_t messageTooBig          : 1;
        uint32_t hardwareError          : 1;

        uint32_t invalidMode            : 1;
        uint32_t TXtimeout              : 1;
        uint32_t RXtimeout              : 1;
        uint32_t unhandledMessage      :  1;

        uint32_t reserved               : 20;
    };

} sError_t;

typedef union
{
    uint8_t byteArray[MESSAGE_LENGTH_LIMIT];
    uint8_t byteValue;
    int32_t iValue;
    uint32_t uiValue;
    float floatValue;
} sParameter_t;

typedef sError_t (*fnPtrParam)(void *parent, sParameter_t *ptrParam);

typedef struct
{
    uint8_t command;
    fnPtrParam fnParam;
    eDataType_t dataType;
    uint32_t  commandDataLength;
    uint32_t  responseDataLength;
} sCommand_t;

/* Class ------------------------------------------------------------------------------------------------------------*/
class DBinaryParser
{
private:
    //attributes
    sCommand_t *commands;
    size_t numCommands;
    size_t capacity;
    uint8_t tableCrc8[256];

    void getBufferFromLong(uint32_t *value, uint8_t *buffer);
    void getBufferFromFloat(float *value, uint8_t *buffer);
    void getBufferFromShort(uint16_t *value, uint8_t *buffer);
    void getBufferFromChar(uint8_t *value, uint8_t *buffer);
    bool validateCrc(uint8_t *data, uint16_t length);
    bool validateStartCondition(uint8_t *data);
    uint32_t calculateCrc(uint8_t *data,  uint8_t length, uint8_t *crc);
    void generateTableCrc8(uint8_t polynomial);
    void resetCrcTable(void);

protected:
    void *myParent;

public:
    //constructor & destructor
    DBinaryParser(void *creator, sCommand_t *commandArray, size_t maxCommands);
    ~DBinaryParser();

    sError_t parse(uint8_t *data,
                   uint32_t msgSize,
                   uint32_t *errorCode,
                   uint32_t enggProtoCommand,
                   uint8_t *rxData);

    void addCommand(uint8_t command,
                    eDataType_t dataType,
                    fnPtrParam fnParam,
                    uint32_t commandDataLength,
                    uint32_t responseDataLength);

    bool calculateAndAppendCrc(uint8_t *cmdDataBuffer,
                               uint32_t cmdDataBufferSize,
                               uint32_t *CommandLen);

    bool getResponseLength(uint8_t command, uint32_t *expectedResponseLen);
    bool getCommandDataLength(uint8_t cmd, uint32_t *expectedDataLength);

    bool parseAcknowledgement(uint8_t cmd, uint8_t *ptrBuffer, uint32_t *errorCode);

    eCommandType_t getCommandType(uint8_t cmd);

    uint8_t getHandleToCommandProperties(uint8_t cmd, sCommand_t **ptrToCmd);

    bool getValueFromBuffer(uint8_t *buffer, eDataType_t dataType, sParameter_t *ptrParam);
    sError_t getBufferFromValue(float *value, uint8_t *buffer);
    sError_t getBufferFromValue(uint32_t *value, uint8_t *buffer);
    sError_t getBufferFromValue(uint16_t *value, uint8_t *buffer);
    sError_t getBufferFromValue(uint8_t *value, uint8_t *buffer);
    sError_t getFloatFromBuffer(uint8_t *buffer, float *value);
    sError_t getUint32FromBuffer(uint8_t *buffer, uint32_t *value);
    sError_t getInt32FromBuffer(uint8_t *buffer, int32_t *value);
    sError_t getUint16FromBuffer(uint8_t *buffer, uint16_t *value);
    sError_t getInt16FromBuffer(uint8_t *buffer, int16_t *value);
    sError_t getUint8FromBuffer(uint8_t *buffer, uint8_t *value);
    sError_t getInt8FromBuffer(uint8_t *buffer, int8_t *value);

    bool prepareTxMessage(uint8_t cmd,
                          uint8_t *cmdData,
                          uint8_t cmdDataSize,
                          uint8_t *txBuffer,
                          uint16_t txBufferLen);

};

#endif /* DBinary Parser.h */