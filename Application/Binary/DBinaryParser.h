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
#ifndef __DBINARY_PARSE_H
#define __DBINARY_PARSE_H

#include "misra.h"

/* Includes -----------------------------------------------------------------*/
MISRAC_DISABLE
#include <os.h>
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
#define OWI_MESSAGE_MIN_SIZE           3u
#define OWI_MESSAGE_MAX_SIZE           80u
#define MESSAGE_LENGTH_LIMIT           255u
#define OWI_MESSAGE_MAX_PARAMETERS     8u
#define OWI_ACK_LENGTH                 2u
#define OWI_NCK_LENGTH                 1u

#define STEPPER_START_CONDITION 0xFFFF

/* Type Defines -------------------------------------------------------------*/
typedef enum
{
    E_CMD_NONE = 0, 
    E_CMD_READ, 
    E_CMD_WRITE
}eCommandType_t;

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
}eDataType_t;

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







typedef sError_t (*fnPtrParam)(void *parent, sParameter_t* ptrParam);

typedef struct
{
    uint8_t command;
    fnPtrParam fnParam;
    eDataType_t dataType;
    uint32_t  commandDataLength;
    uint32_t  responseDataLength;
}sCommand_t;

/* Class --------------------------------------------------------------------*/
class DBinaryParser
{
private:
    //attributes
    sCommand_t *commands;
    size_t numCommands;
    size_t capacity;

   

    void GetBufferFromLong(uint32_t* value, uint8_t* buffer);
    void GetBufferFromFloat(float* value, uint8_t* buffer);
    void GetBufferFromShort(uint16_t* value, uint8_t* buffer);
    void GetBufferFromChar(uint8_t* value, uint8_t* buffer);
    bool ValidateCrc(uint8_t *data, uint16_t length);
    bool ValidateStartCondition(uint8_t *data);
    uint8_t CalculateCrc(uint8_t* data,  uint8_t length);
protected:
       void *myParent;             //this can be set by the object instance that creates the parser (to be used as a callback parameter)


public:
    //constructor & destructor
    DBinaryParser(void *creator, sCommand_t *commandArray, size_t maxCommands, OS_ERR *os_error);
    ~DBinaryParser();

    //methods
    sError_t parse(uint8_t cmd, uint8_t *data, uint32_t msgSize, uint32_t* errorCode);
    sError_t slaveParse(uint8_t cmd, uint8_t *data, uint32_t *msgSize);

    void setChecksumEnabled(bool flag);
    bool getChecksumEnabled(void);

    void addCommand(uint8_t command,
                    fnPtrParam fnParam,
                    eDataType_t dataType,
                    uint32_t expectedDataLength,
                    uint32_t responseDataLength);

    bool CalculateAndAppendCrc(uint8_t *cmdDataBuffer, 
                                     uint32_t cmdDataBufferSize,
                                     uint32_t *CommandLen);
    
    bool getResponseLength(uint8_t command, 
                            uint32_t *expectedResponseLen);
    
    bool getCommandDataLength(uint8_t cmd, uint32_t *expectedDataLength);

    bool parseAcknowledgement(uint8_t cmd, uint8_t* ptrBuffer, uint32_t* errorCode);
    
    //bool ValidateCrc(uint8_t *cmdDataBuffer,uint32_t cmdDataBufferSize);
    
    eCommandType_t getCommandType(uint8_t cmd);
    
    uint8_t getHandleToCommandProperties(uint8_t cmd, sCommand_t **ptrToCmd);

    bool GetValueFromBuffer(uint8_t* buffer, eDataType_t dataType, sParameter_t *ptrParam);

    void GetBufferFromValue(float* value, uint8_t* buffer);
    void GetBufferFromValue(uint32_t* value, uint8_t* buffer);
    void GetBufferFromValue(uint16_t* value, uint8_t* buffer);
    void GetBufferFromValue(uint8_t* value, uint8_t* buffer);
    
    float GetFloatFromBuffer(uint8_t* buffer);
    uint32_t GetUint32FromBuffer(uint8_t* buffer);
    int32_t GetInt32FromBuffer(uint8_t* buffer);
    uint16_t GetUint16FromBuffer(uint8_t* buffer);
    int16_t GetInt16FromBuffer(uint8_t* buffer);
    uint8_t GetUint8FromBuffer(uint8_t* buffer);
    int8_t GetInt8FromBuffer(uint8_t* buffer);  
   
    bool prepareTxMessage(uint8_t cmd, 
                                     uint8_t* cmdData, 
                                     uint8_t cmdDataSize, 
                                     uint8_t *txBuffer, 
                                     uint16_t *txBufferLen);
    
};
#endif