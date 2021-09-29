/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DOwiParse.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The DUCI message parser class header file
*/

#ifndef __DOWI_PARSE_H
#define __DOWI_PARSE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <os.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <lib_def.h>

#ifdef __cplusplus
extern "C" /* External C language linkage */
{  
   #include <lib_ascii.h>     
}
#endif/* End of external C language linkage */
MISRAC_ENABLE

#include "Types.h"

/* Defines  ---------------------------------------------------------------------------------------------------------*/
#define OWI_MESSAGE_MIN_SIZE           3u
#define OWI_MESSAGE_MAX_SIZE           80u
#define OWI_STRING_LENGTH_LIMIT        50u
#define OWI_MESSAGE_MAX_PARAMETERS     8u
#define OWI_ACK_LENGTH                  2u
#define OWI_NCK_LENGTH                  1u


/* Types ------------------------------------------------------------------------------------------------------------*/

typedef enum : uint8_t
{
    E_AMC_SENSOR_BRIDGE_COUNTS_CHANNEL     = 0X00,		
    E_AMC_SENSOR_TEMPERATURE_CHANNEL       = 0X01,
    E_AMC_SENSOR_RESERVERD1_CHANNEL        = 0X02,
    E_AMC_SENSOR_RESERVERD2_CHANNEL        = 0X03,
} eAmcSensorChannel_t;


typedef enum : uint8_t
{
    E_OWI_RESPONSE_ACC              = 0X86,		
    E_OWI_RESPONSE_NCK              = 0X95
} eOwiResponse_t;

typedef struct
{
    uint16_t    day;
    uint16_t    month;
    uint32_t    year;

} sOwiDate_t;

typedef struct
{
    uint16_t    hours;
    uint16_t    mins;
    uint32_t    secs;

} sOwiTime_t;

typedef struct
{
  uint32_t channel1AdcCounts;
  uint32_t channel2AdcCounts;
  
}sRawAdcCounts;
typedef union
{
    uint8_t byteArray[OWI_STRING_LENGTH_LIMIT];
    uint8_t byteValue;
    int32_t iValue;    
    uint32_t uiValue;
    float32_t floatValue;
    sRawAdcCounts rawAdcCounts;
    bool flagValue;
    sOwiDate_t date;
    sOwiTime_t time;

} sOwiParameter_t;

typedef enum
{
    owiMessageTypeCommand,
    owiMessageTypeResponse,
    owiMessageTypeQuery,
    owiMessageTypeUnknown

} eOwiMessageType_t;

typedef enum  : uint8_t
{
    owiArgNone = 0,
    owiArgInteger,         //signed or unsigned integer value
    owiArgUnsignedInt,
    owiArgByteValue,
    owiArgByteArray,
    owiArgHexadecimal,     //32-bit hexadecimal value    
    owiArgBoolean,         //boolean flag value
    owiArgString,          //ASCII character string
    owiArgCharacter,       //single ASCII character
    owiArgValue,           //floating point value (with or without decimal point)
    owiArgAssignment,      //equals sign (for assignment)
    owiArgQuery,           //question mark
    owiArgDate,            //date specifier (always dd/mm/yyyy)
    owiArgTime,            //time specifier (always hh:mm:ss)
    owiArgRawAdcCounts,    //Bridge differntial Counts and temperature counts
    owiArgAmcSensorCoefficientsInfo, //Coefficient information
    owiArgAmcSensorCalibrationInfo,  //Calibration information    
    owiArgCustom,           //custom - whole string is passed as a single parameter
    owiArgInvalid

} eOwiArgType_t;

typedef union
{
    uint32_t value;

    struct
    {
        uint32_t argFree        : 8;
        uint32_t argOptional    : 8;
        uint32_t argFieldWidth  : 8;
        uint32_t argType        : 8;
    };

} sOwiArg_t;

typedef union
{
    uint32_t value;

    struct
    {
        uint32_t needs_start            : 1;
        uint32_t unknownCommand         : 1;
        uint32_t invalid_args           : 1;
        uint32_t invalid_response       : 1;

        uint32_t missing_args           : 1;
        uint32_t number_not_in_sequence : 1;
        uint32_t writeToFlash           : 1;
        uint32_t bufferSize             : 1;

        uint32_t invalidMode            : 1;
        uint32_t TXtimeout              : 1;
        uint32_t RXtimeout              : 1;
        uint32_t FlashCrcError          : 1;

        uint32_t badReply               : 1;
        uint32_t invalidChecksum        : 1;
        uint32_t hardwareError          : 1;
        uint32_t calFailed              : 1;

        uint32_t messageTooSmall        : 1;
        uint32_t messageTooBig          : 1;
        uint32_t unexpectedMessage      : 1;
        uint32_t unhandledMessage       : 1;

        uint32_t reserved               : 12;
    };

} sOwiError_t;

  

typedef sOwiError_t (*fnPtrOwiParam)(void *parent, sOwiParameter_t* ptrOwiParam);
typedef sOwiError_t (*fnPtrChar)(void *parent, uint8_t* ptrChar, uint32_t* size);


typedef enum: uint8_t
{
    E_OWI_ASCII = 0, 
    E_OWI_BYTE,
    E_OWI_HEX_ASCII

} eOwiDataFormat_t;

typedef enum: uint8_t
{
    E_OWI_COMMAND = 0, 
    E_OWI_REPLY, 
    E_OWI_INVALID_REPLY, //If Check sum not matches or not enough number of bytes 
    E_OWI_UNEXPECTED   
} eOwiMessage_t;
/* Variables ---------------------------------------------------------------*/
typedef struct
{
    uint8_t command;
    eOwiArgType_t argType;
    eOwiDataFormat_t cmdDataFormat;
    eOwiDataFormat_t responseDataFormat;
    fnPtrOwiParam fnOwiParam;
    fnPtrChar fnCharParam;
    uint32_t  commandDataLength;
    uint32_t  responseDataLenght;
    bool checksumAvailableStatusInResponse; //Some of responses does not have checksum
    uint32_t permissions;

} sOwiCommand_t;

typedef enum :uint8_t
{
    E_OWI_CMD_NONE = 0, 
    E_OWI_CMD_READ, 
    E_OWI_CMD_WRITE, 
} eOwiCommandType_t;



class DOwiParse
{
private:
    //attributes
    sOwiCommand_t *commands;
    size_t numCommands;
    size_t capacity;

    //methods
     bool getCoefficientsArg(uint8_t* pBinaryBuffer,
                                    uint8_t* pAsciiString, 
                                    uint32_t msgSize);
     
    bool getCalibrationDataArg(uint8_t* pBinaryBuffer,
                                    uint8_t* pAsciiString, 
                                    uint32_t msgSize);
    bool getValueArg(float* pfValue, 
                               uint8_t* pSrcBuffer, 
                               eOwiDataFormat_t srcDataFormat, 
                               uint32_t msgSize
                               );
    
   bool getRawCountsArg(sRawAdcCounts* prtRawAdcCounts,
                        uint8_t* pSrcBuffer,
                        uint32_t srcBufferSize) ;
   
   bool dataToAsciiHex(uint8_t* pAsciiString, uint8_t* pBinaryBuffer, 
                                uint32_t iNumberOfBinaryBytes);
   
   bool asciiHexToData(uint8_t* pBinaryBuffer, uint8_t* pAsciiString, 
                                uint32_t iNumberOfBinaryBytes);

protected:
    
    bool checksumEnabled;   //true is checksum is used in messages
   
    void *myParent;     //this can be set by the object instance that creates the parser (to be used as a callback parameter)

    
    
public:
    //constructor & destructor
    DOwiParse(void *creator, OS_ERR *osErr);
    ~DOwiParse();

    //attributes
    eOwiMessage_t messageType;

    //methods
    sOwiError_t parse(uint8_t cmd, uint8_t *str, uint32_t msgSize = 0);
    sOwiError_t slaveParse(uint8_t cmd, uint8_t *str, uint32_t *msgSize );

    void setChecksumEnabled(bool flag);
    bool getChecksumEnabled(void);

    
    void addCommand( uint8_t command,
                     eOwiArgType_t argType,
                     eOwiDataFormat_t cmdDataFormat,
                     eOwiDataFormat_t responseDataFormat,
                     fnPtrOwiParam fnOwiParam,
                     fnPtrChar fnCharParam,
                     uint32_t expectedDataLength,
                     uint32_t responseDataLength,
                     bool responseCheckSumStatus,
                     uint32_t permissions);

  

    bool CalculateAndAppendCheckSum( uint8_t *cmdDataBuffer, 
                                     uint32_t cmdDataBufferSize,
                                     uint32_t *CommandLen);
    
    bool getResponseLength(uint8_t command, uint32_t *expectedResponseLen);
    
    bool getCommandDataLength(uint8_t cmd, uint32_t *expectedDataLength);

    bool parseAcknowledgement(uint8_t cmd, uint8_t* ptrBuffer);
    
    bool ValidateCheckSum(uint8_t *cmdDataBuffer, 
                                     uint32_t cmdDataBufferSize);
    
    eOwiCommandType_t getCommandType(uint8_t cmd);
    //bool getHandleToCommandProperties(uint8_t cmd, sOwiCommand_t *ptrToCmd );
    
    uint8_t getHandleToCommandProperties(uint8_t cmd, sOwiCommand_t **ptrToCmd );
    
};

#endif /* __DPARSE_H */
