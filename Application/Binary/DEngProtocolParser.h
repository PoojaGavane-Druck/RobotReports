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
* @file     DBinaryParser.h
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     27-Feb-2021
*
* @brief    Binary Parser for Stepper Motor Header File
*/
#ifndef __ENG_PROTOCOL_PARSE_H
#define __ENG_PROTOCOL_PARSE_H

#include "misra.h"

/* Includes -----------------------------------------------------------------*/
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
#endif/* End of external C language linkage */
MISRAC_ENABLE

#include "Types.h"
/* Constants & Defines ------------------------------------------------------*/


#define STEPPER_START_CONDITION 0xFFFF
#define ENG_PROTOCOL_MESSAGE_LENGTH_LIMIT 16

/* Type Defines -------------------------------------------------------------*/

typedef enum : uint8_t
{
    ENG_PROTOCOL_CMD_None = 0,                          // 0 - 0x00
    ENG_PROTOCOL_CMD_SetParameter,                      // 1 - 0x01
    ENG_PROTOCOL_CMD_GetParameter,                      // 2 - 0x02
    ENG_PROTOCOL_CMD_Run,                               // 3 - 0x03
    ENG_PROTOCOL_CMD_StepClock,                         // 4 - 0x04
    ENG_PROTOCOL_CMD_Move,                              // 5 - 0x05
    ENG_PROTOCOL_CMD_GoTo,                              // 6 - 0x06
    ENG_PROTOCOL_CMD_GoToDir,                           // 7 - 0x07
    ENG_PROTOCOL_CMD_GoUntil,                           // 8 - 0x08
    ENG_PROTOCOL_CMD_ReleaseSW,                         // 9 - 0x09
    ENG_PROTOCOL_CMD_GoHome,                            // 10 - 0x0A
    ENG_PROTOCOL_CMD_GoMark,                            // 11 - 0x0B
    ENG_PROTOCOL_CMD_ResetPos,                          // 12 - 0x0C
    ENG_PROTOCOL_CMD_ResetDevice,                       // 13 - 0x0D
    ENG_PROTOCOL_CMD_SoftStop,                          // 14 - 0x0E
    ENG_PROTOCOL_CMD_HardStop,                          // 15 - 0x0F
    ENG_PROTOCOL_CMD_SoftHiZ,                           // 16 - 0x10
    ENG_PROTOCOL_CMD_HardHiZ,                           // 17 - 0x11
    ENG_PROTOCOL_CMD_GetStatus,                         // 18 - 0x12
    ENG_PROTOCOL_CMD_MoveContinuous,                    // 19 - 0x13
    ENG_PROTOCOL_CMD_ReadStepCount,                     // 20 - 0x14
    ENG_PROTOCOL_CMD_WriteRegister,                     // 21 - 0x15
    ENG_PROTOCOL_CMD_ReadRegister,                      // 22 - 0x16
    ENG_PROTOCOL_CMD_WriteAcclAlpha,                    // 23 - 0x17
    ENG_PROTOCOL_CMD_WriteAcclBeta,                     // 24 - 0x18
    ENG_PROTOCOL_CMD_WriteDecclAlpha,                   // 25 - 0x19
    ENG_PROTOCOL_CMD_WriteDecclBeta,                    // 26 - 0x1A
    ENG_PROTOCOL_CMD_ReadAcclAlpha,                     // 27 - 0x1B
    ENG_PROTOCOL_CMD_ReadAcclBeta,                      // 28 - 0x1C
    ENG_PROTOCOL_CMD_ReadDecclAlpha,                    // 29 - 0x1D
    ENG_PROTOCOL_CMD_ReadDecclBeta,                     // 30 - 0x1E
    ENG_PROTOCOL_CMD_MinimumSpeed,                      // 31 - 0x1F
    ENG_PROTOCOL_CMD_MaximumSpeed,                      // 32 - 0x20
    ENG_PROTOCOL_CMD_WatchdogTime,                      // 33 - 0x21
    ENG_PROTOCOL_CMD_WatchdogEnable,                    // 34 - 0x22
    ENG_PROTOCOL_CMD_AccelerationTime,                  // 35 - 0x23
    ENG_PROTOCOL_CMD_DecelerationTime,                  // 36 - 0x24
    ENG_PROTOCOL_CMD_SetAbsPosition,                    // 37 - 0x25
    ENG_PROTOCOL_CMD_GetAbsPosition,                    // 38 - 0x26
    ENG_PROTOCOL_CMD_GetVersionInfo,                    // 39 - 0x27
    ENG_PROTOCOL_CMD_ResetController,                   // 40 - 0x28
    ENG_PROTOCOL_CMD_WriteHoldCurrent,                  // 41 - 0x29
    ENG_PROTOCOL_CMD_WriteRunCurrent,                   // 42 - 0x2A
    ENG_PROTOCOL_CMD_WriteAcclCurrent,                  // 43 - 0x2B
    ENG_PROTOCOL_CMD_WriteDecelCurrent,                 // 44 - 0x2C
    ENG_PROTOCOL_CMD_ReadHoldCurrent,                   // 45 - 0x2D
    ENG_PROTOCOL_CMD_ReadRunCurrent,                    // 46 - 0x2E
    ENG_PROTOCOL_CMD_ReadAcclCurrent,                   // 47 - 0x2F
    ENG_PROTOCOL_CMD_ReadDecelCurrent,                  // 48 - 0x30
    ENG_PROTOCOL_CMD_ReadSpeedAndCurrent,               // 49 - 0x31
    ENG_PROTOCOL_CMD_ReadOpticalSensor,                 // 50 - 0x32
    ENG_PROTOCOL_CMD_OpenValveOne,                      // 51 - 0x33
    ENG_PROTOCOL_CMD_CloseValveOne,                     // 52 - 0x34
    ENG_PROTOCOL_CMD_OpenValveTwo,                      // 53 - 0x35
    ENG_PROTOCOL_CMD_CloseValveTwo,                     // 54 - 0x36
    ENG_PROTOCOL_CMD_OpenValveThree,                    // 55 - 0x37
    ENG_PROTOCOL_CMD_CloseValveThree,                   // 56 - 0x38
    ENG_PROTOCOL_CMD_ControllerStatus,                  // 57 - 0x39
    ENG_PROTOCOL_CMD_GetFullScaleStatus,                // 58 - 0x3A
    ENG_PROTOCOL_CMD_GetBarometerReading,               // 59 - 0x3B
    ENG_PROTOCOL_CMD_GetPM620Reading,                   // 60 - 0x3C
    ENG_PROTOCOL_CMD_GetSensorType,                     // 61 - 0x3D
    ENG_PROTOCOL_CMD_GetSetPoint,                       // 62 - 0x3E
    ENG_PROTOCOL_CMD_GetControllingMode,                // 63 - 0x3F
    ENG_PROTOCOL_CMD_GetPositionSensor,                 // 64 - 0x40
    ENG_PROTOCOL_CMD_CheckSerialPort,                   // 65 - 0x41
    ENG_PROTOCOL_CMD_GetPMType,                         // 66 - 0x42
    ENG_PROTOCOL_CMD_DuciSwitch,                        // 67 - 0x43
    ENG_PROTOCOL_CMD_ValveTime                          // 68 - 0x44
} eEngProtocolCommand_t;

#if 0
typedef enum
{
    eDataTypeNone = 0,
    eDataTypeBoolean,
    eDataTypeByte,
    eDataTypeUnsignedChar,
    eDataTypeSignedChar,
    eDataTypeUnsignedShort,
    eDataTypeSignedShort,
    eDataTypeUnsignedLong,
    eDataTypeSignedLong,
    eDataTypeFloat,
    eDataTypeDouble
} eEngProtocolDataType_t;
#endif
typedef union
{
    uint32_t value;

    struct
    {

        uint32_t unknownCommand : 1;
        uint32_t messageTooSmall : 1;
        uint32_t messageTooBig : 1;
        uint32_t hardwareError : 1;

        uint32_t invalidMode : 1;
        uint32_t TXtimeout : 1;
        uint32_t RXtimeout : 1;
        uint32_t unhandledMessage : 1;

        uint32_t cmdExecutionFailed : 1;
        uint32_t messageIsNotCmdType : 1;

        uint32_t reserved : 22;
    };

} sEngProError_t;

typedef union
{
    uint8_t byteArray[ENG_PROTOCOL_MESSAGE_LENGTH_LIMIT];
    uint8_t byteValue;
    int32_t iValue;
    uint32_t uiValue;
    float floatValue;

} sEngProtocolParameter_t;

typedef sEngProError_t(*fnPtrEngProtoParam)(void *parent, sEngProtocolParameter_t *ptrParam);




typedef struct
{
    uint8_t command;
    eDataType_t dataType;
    fnPtrEngProtoParam fnParam;
    uint32_t  commandDataLength;
    uint32_t  responseDataLength;
} sEngProtcolCommand_t;
typedef enum
{
    E_ENG_PROTOCOL_COMMAND = 0, //a command string starting with '#' or '*'
    E_ENG_PROTOCOL_REPLY,       //a reply to a query command (that may look like a set command) but starts with a '!'
    E_ENG_PROTOCOL_UNEXPECTED   //a message with a start character that is not appropriate in current DUCI comms state

} eEngProtocolMessage_t;



/* Class --------------------------------------------------------------------*/
class DEngProtocolParser
{
private:
    //attributes
    sEngProtcolCommand_t *commands;
    size_t numCommands;
    size_t capacity;



    void GetBufferFromLong(uint32_t *value, uint8_t *buffer);
    void GetBufferFromFloat(float *value, uint8_t *buffer);
    void GetBufferFromShort(uint16_t *value, uint8_t *buffer);
    void GetBufferFromChar(uint8_t *value, uint8_t *buffer);

protected:
    void *myParent;             //this can be set by the object instance that creates the parser (to be used as a callback parameter)


public:
    //constructor & destructor
    DEngProtocolParser(void *creator, sEngProtcolCommand_t *commandArray, size_t maxCommands, OS_ERR *os_error);
    ~DEngProtocolParser();
    eEngProtocolMessage_t messageType;
    //methods
    sEngProError_t parse(uint8_t *daptrBufferta, uint32_t msgSize);


    void addCommand(uint8_t command,
                    eDataType_t dataType,
                    fnPtrEngProtoParam fnParam,
                    uint32_t expectedDataLength,
                    uint32_t responseDataLength);


    bool GetValueFromBuffer(uint8_t *buffer, eDataType_t dataType, sEngProtocolParameter_t *ptrParam);

    void GetBufferFromValue(float *value, uint8_t *buffer);
    void GetBufferFromValue(uint32_t *value, uint8_t *buffer);
    void GetBufferFromValue(uint16_t *value, uint8_t *buffer);
    void GetBufferFromValue(uint8_t *value, uint8_t *buffer);

    float GetFloatFromBuffer(uint8_t *buffer);
    uint32_t GetUint32FromBuffer(uint8_t *buffer);
    int32_t GetInt32FromBuffer(uint8_t *buffer);
    uint16_t GetUint16FromBuffer(uint8_t *buffer);
    uint8_t GetUint8FromBuffer(uint8_t *buffer);
    int8_t GetInt8FromBuffer(uint8_t *buffer);

    bool prepareResponse(sEngProtocolParameter_t *params,
                         uint8_t numOfParams,
                         uint8_t *txBuffer,
                         uint32_t *txBufferLen);

};
#endif