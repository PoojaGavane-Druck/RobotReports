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

#ifndef __DPARSE_H
#define __DPARSE_H

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
#define DUCI_MESSAGE_MIN_SIZE           3u
#define DUCI_MESSAGE_MAX_SIZE           80u
#define DUCI_STRING_LENGTH_LIMIT        64
#define DUCI_MESSAGE_MAX_PARAMETERS     8u

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef struct
{
    uint16_t    day;
    uint16_t    month;
    uint32_t    year;

} sDuciDate_t;

typedef struct
{
    uint16_t    hours;
    uint16_t    mins;
    uint32_t    secs;

} sDuciTime_t;

typedef union
{
    char charArray[DUCI_STRING_LENGTH_LIMIT];
    int32_t intNumber;
    uint64_t hexNumber;
    float32_t floatValue;
    bool flagValue;
    sDuciDate_t date;
    sDuciTime_t time;

} sDuciParameter_t;

typedef enum
{
    duciMessageTypeCommand,
    duciMessageTypeResponse,
    duciMessageTypeQuery,
    duciMessageTypeUnknown

} eDuciMessageType_t;

typedef enum
{
    argInteger,         //signed or unsigned integer value
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

} sDuciArg_t;

typedef union
{
    uint32_t value;

    struct
    {
        uint32_t needsStart             : 1;
        uint32_t unknownCommand         : 1;
        uint32_t invalid_args           : 1;
        uint32_t invalid_response       : 1;

        uint32_t missing_args           : 1;
        uint32_t numberNotInSequence    : 1;
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

        uint32_t commandFailed          : 1;
        uint32_t badData                : 1;

        uint32_t reserved               : 10;
    };

} sDuciError_t;

typedef sDuciError_t (*fnPtrDuci)(void *parent, sDuciParameter_t * parameterArray);

typedef struct
{
    const char *command;

    sDuciArg_t setArgs[DUCI_MESSAGE_MAX_PARAMETERS];
    sDuciArg_t getArgs[DUCI_MESSAGE_MAX_PARAMETERS];

    fnPtrDuci setFunction;
    fnPtrDuci getFunction;

    uint32_t permissions;

} sDuciCommand_t;

typedef enum
{
    E_DUCI_COMMAND = 0, //a command string starting with '#' or '*'
    E_DUCI_REPLY,       //a reply to a query command (that may look like a set command) but starts with a '!'
    E_DUCI_UNEXPECTED   //a message with a start character that is not appropriate in current DUCI comms state

} eDuciMessage_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DParse
{
private:
    //attributes
    size_t numCommands;
   
    //methods
    sDuciError_t processCommand(int32_t cmdIndex, char *str);
    uint32_t formatToArgs(const char *formatSpec, sDuciArg_t *args);
    sDuciError_t checkDuciString(sDuciArg_t *expectedArgs, char *str, fnPtrDuci fnCallback);

    sDuciError_t getArgument(const char *buffer, sDuciArg_t *arg, const char **endptr);

protected:
    //attributes
    sDuciCommand_t *commands;
    size_t capacity;
    
    bool echoCommand;   //flag that indicates whether a command should be echoed back
    bool checksumEnabled;   //true is checksum is used in messages
    bool stripTrailingChecksum; //strip the trailing ":nn" characters from string (after validation)
    bool terminatorCrLf;    //true if terminator is CRLF, else use on LF
    void *myParent;     //this can be set by the object instance that creates the parser (to be used as a callback parameter)

    virtual bool isMyStartCharacter(char ch);

public:
    //constructor & destructor
    DParse(void *creator, OS_ERR *osErr);
    ~DParse();

    //attributes
    eDuciMessage_t messageType;

    //methods
    sDuciError_t parse(char * str);

    void setChecksumEnabled(bool flag);
    bool getChecksumEnabled(void);

    void setTerminatorCrLf(bool flag);
    bool getTerminatorCrLf(void);

    void addCommand(const char *command,
                    const char *setFormat,
                    const char *getFormat,
                    fnPtrDuci setFunction,
                    fnPtrDuci getFunction,
                    uint32_t permissions);

    sDuciError_t getIntegerArg(char *buffer, int32_t *intNumber, uint32_t fieldWidth, char **endptr);
    sDuciError_t getValueArg(char *buffer, float32_t *floatValue, char **endptr);
    sDuciError_t getHexadecimalArg(char *buffer, int32_t *intNumber, uint32_t fieldWidth, char **endptr);
    sDuciError_t getLongHexadecimalArg(char *buffer, uint64_t *hexNumber, uint32_t fieldWidth, char **endptr);
    sDuciError_t getStringArg(char *buffer, char *str, char **endptr);
    sDuciError_t getDateArg(char *buffer, sDuciDate_t *pDate, char **endptr);
    sDuciError_t getTimeArg(char *buffer, sDuciTime_t *ptime, char **endptr);

    virtual bool prepareTxMessage(char *str, char *buffer, uint32_t bufferSize);

};

#endif /* __DPARSE_H */
