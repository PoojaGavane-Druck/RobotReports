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
    ENG_PROTOCOL_CMD_MoveContinuous,                    // 1 - 0x01
    ENG_PROTOCOL_CMD_ReadStepCount,                     // 2 - 0x02
    ENG_PROTOCOL_CMD_GetVersionInfo,                    // 3 - 0x03
    ENG_PROTOCOL_CMD_ResetController,                   // 4 - 0x04
    ENG_PROTOCOL_CMD_ReadSpeedAndCurrent,               // 5 = 0x05
    ENG_PROTOCOL_CMD_OpenValveOne,                      // 6 - 0x06
    ENG_PROTOCOL_CMD_CloseValveOne,                     // 7 - 0x07
    ENG_PROTOCOL_CMD_OpenValveTwo,                      // 8 - 0x08
    ENG_PROTOCOL_CMD_CloseValveTwo,                     // 9 - 0x09
    ENG_PROTOCOL_CMD_OpenValveThree,                    // 10 - 0x0A
    ENG_PROTOCOL_CMD_CloseValveThree,                   // 11 - 0x0B
    ENG_PROTOCOL_CMD_ControllerStatus,                  // 12 - 0x0C
    ENG_PROTOCOL_CMD_GetFullScaleStatus,                // 13 - 0x0D
    ENG_PROTOCOL_CMD_GetBarometerReading,               // 14 - 0x0E
    ENG_PROTOCOL_CMD_GetPM620Reading,                   // 15 - 0x0F
    ENG_PROTOCOL_CMD_GetSensorType,                     // 16 - 0x10
    ENG_PROTOCOL_CMD_GetSetPoint,                       // 17 - 0x11
    ENG_PROTOCOL_CMD_GetControllingMode,                // 18 - 0x12
    ENG_PROTOCOL_CMD_GetPMType,                         // 19 - 0x13
    ENG_PROTOCOL_CMD_DuciSwitch,                        // 20 - 0x14
    ENG_PROTOCOL_CMD_ValveTime,                         // 21 - 0x15
    ENG_PROTOCOL_CMD_OptInterrupter,                    // 22 - 0x16
    ENG_PROTOCOL_CMD_ConfigValve,                       // 23 - 0x17
    ENG_PROTOCOL_CMD_GetRate                            // 24 - 0x18
} eEngProtocolCommand_t;

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
    float32_t floatValue;

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

protected:
    void *myParent;             //this can be set by the object instance that creates the parser (to be used as a callback parameter)


public:
    //constructor & destructor
    DEngProtocolParser(void *creator,
                       sEngProtcolCommand_t *commandArray,
                       size_t maxCommands,
                       OS_ERR *os_error);
    ~DEngProtocolParser();
    eEngProtocolMessage_t messageType;
    //methods
    sEngProError_t parse(uint8_t *daptrBufferta, uint32_t msgSize);


    void addCommand(uint8_t command,
                    eDataType_t dataType,
                    fnPtrEngProtoParam fnParam,
                    uint32_t expectedDataLength,
                    uint32_t responseDataLength);

    bool getValueFromBuffer(uint8_t *buffer, eDataType_t dataType,
                            sEngProtocolParameter_t *ptrParam);

    void getBufferFromValue(float32_t value, uint8_t *buffer);
    void getBufferFromValue(uint32_t value, uint8_t *buffer);
    void getBufferFromValue(uint16_t value, uint8_t *buffer);
    void getBufferFromValue(uint8_t value, uint8_t *buffer);

    float32_t getFloatFromBuffer(uint8_t *buffer);
    uint32_t getUint32FromBuffer(uint8_t *buffer);
    int32_t getInt32FromBuffer(uint8_t *buffer);
    uint16_t getUint16FromBuffer(uint8_t *buffer);
    uint8_t getUint8FromBuffer(uint8_t *buffer);
    int8_t getInt8FromBuffer(uint8_t *buffer);

    bool prepareResponse(sEngProtocolParameter_t *params,
                         uint8_t numOfParams,
                         uint8_t *txBuffer,
                         uint32_t *txBufferLen);
};
#endif