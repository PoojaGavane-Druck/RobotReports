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
* @file     DParse.h
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
#define OWI_STRING_LENGTH_LIMIT        4096
#define OWI_MESSAGE_MAX_PARAMETERS     8u

/* Types ------------------------------------------------------------------------------------------------------------*/
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
    char charArray[OWI_STRING_LENGTH_LIMIT];
    uint8_t byteValue;
    int32_t intNumber;    
    uint32_t uiValue;
    float32_t floatValue;
    sRawAdcCounts rqwAdcCounts;
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

typedef enum
{
    argInteger,         //signed or unsigned integer value
    argUnsignedInt,
    argByteValue,
    argHexadecimal,     //32-bit hexadecimal value
    argLongHexadecimal, //64-bit hexadecimal value
    argBoolean,         //boolean flag value
    argString,          //ASCII character string
    argCharacter,       //single ASCII character
    argValue,           //floating point value (with or without decimal point)
    argAssignment,      //equals sign (for assignment)
    argQuery,           //question mark
    argDate,            //date specifier (always dd/mm/yyyy)
    argTime,            //time specifier (always hh:mm:ss)
    argRawAdcCounts,    //Bridge differntial Counts and temperature counts
    argAmcCoefficientsInfo, //Coefficient information
    argAmcCalibrationInfo,  //Calibration information
    argCustom           //custom - whole string is passed as a single parameter

} eArgType_t;

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

typedef union
{
    
    uint8_t byteValue[4];    
    float32_t floatValue;
} uFloat_t;    
typedef sOwiError_t (*fnPtrOwi)(void *parent, sOwiParameter_t* parameterArray);
typedef enum
{
    E_OWI_ASCII = 0, 
    E_OWI_BYTE,
    E_OWI_HEX_ASCII

} eOwiDataFormat_t;

typedef enum
{
    E_OWI_COMMAND = 0, //a command string starting with '#' or '*'
    E_OWI_REPLY,       //a reply to a query command (that may look like a set command) but starts with a '!'
    E_OWI_UNEXPECTED   //a message with a start character that is not appropriate in current DUCI comms state

} eOwiMessage_t;
/* Variables ---------------------------------------------------------------*/
typedef struct
{
    uint8_t command;
    eArgType_t argType;
    eOwiDataFormat_t cmdDataFormat;
    eOwiDataFormat_t responseDataFormat;
    fnPtrOwi processCmdFunction;
    uint32_t  commandDataLength;
    uint32_t  responseDataLenght;
    uint32_t permissions;

} sOwiCommand_t;





class DOwiParse
{
private:
    //attributes
    sOwiCommand_t *commands;
    size_t numCommands;
    size_t capacity;

    //methods
    sOwiError_t processCommand(uint32_t cmdIndex, uint8_t *str);
   

    

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
    sOwiError_t parse(uint8_t * str, uint32_t msgSize);

    void setChecksumEnabled(bool flag);
    bool getChecksumEnabled(void);

    
    void addCommand( uint8_t command,
                     eOwiDataFormat_t cmdDataFormat,
                     eOwiDataFormat_t responseDataFormat,
                     fnPtrOwi processCmdFunction,
                     uint32_t expectedDataLength,
                     uint32_t responseDataLength,
                     uint32_t permissions);

  

    bool CalculateAndAppendCheckSum( uint8_t *cmdDataBuffer, 
                                     uint32_t cmdDataBufferSize,
                                     uint32_t &CommandLen);
    
    bool getResponseLength(uint8_t command, uint32_t &expectedResponseLen);

};

#endif /* __DPARSE_H */
