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
* @file     DParse.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The DUCI parser base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DParse.h"
#include "Utilities.h"
#include "DPV624.h"

MISRAC_DISABLE
#include <assert.h>
MISRAC_ENABLE

/* Error handler instance parameter starts from 2701 to 2800 */
/* Constants & Defines ----------------------------------------------------------------------------------------------*/
//const size_t defaultSize = 8u;

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Functions *********************************************************************************************************/
/**********************************************************************************************************************
 * DISABLE MISRA C 2004 Error[Pm100]: dynamic heap memory allocation shall not be used (MISRA C 2004 rule 20.4)
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm100")

/**
* @brief    Constructor
*
* @param    creator is owner of this instance
* @param    os_error is pointer to OS error
*
* @return   void
*/
DParse::DParse(void *creator, OS_ERR *os_error):
    myParent(NULL)
{
    myParent = creator;

    //initialise the command set
    numCommands = (size_t)0;

    checksumEnabled = true;        //by default, do not use checksum
    stripTrailingChecksum = true;   //by default, strip the trailing ":nn" characters from string (after validation)
    terminatorCrLf = true;          //by default, use CRLF terminator

    myAckFunction = NULL;           //by default, there is no acknowledge function
    acknowledgeCommand = false;     //by default, do not use acknowledgement

    commands = NULL;
    capacity = (size_t)0;
    messageType = (eDuciMessage_t)E_DUCI_UNEXPECTED;
}

/**
* @brief    Destructor
* @param    void
* @return   void
*/
DParse::~DParse()
{
    //free(commands);
    commands = NULL;
    numCommands = (size_t)0;
    capacity = (size_t)0;
}

/**
* @brief    Set Checksum Enabled
* @param    flag - true is checksum is used in message, else false
* @return   void
*/
void DParse::setChecksumEnabled(bool flag)
{
    checksumEnabled = flag;
}

/**
* @brief    Get Checksum Enabled
* @param    void
* @return   flag - true is checksum is used in message, else false
*/
bool DParse::getChecksumEnabled(void)
{
    return checksumEnabled;
}

/**
* @brief    Set CRLF Termination Enabled
* @param    flag - true means terminator is CRLF, false means terminator is LF only
* @return   void
*/
void DParse::setTerminatorCrLf(bool flag)
{
    terminatorCrLf = flag;
}

/**
* @brief    Get CRLF Termination Enabled
* @param    void
* @return   flag - true means terminator is CRLF, false means terminator is LF only
*/
bool DParse::getTerminatorCrLf(void)
{
    return terminatorCrLf;
}


/**
* @brief    Set CRLF Termination Enabled
* @param    ackFunction - acknowledgement function pointer
* @return   void
*/
void DParse::setAckFunction(fnPtrDuciCmd ackFunction)
{
    myAckFunction = ackFunction;
}

/**
* @brief    Set CRLF Termination Enabled
* @param    state - true means acknowledgement is enabled, false means disabled
* @return   void
*/
void DParse::setAcknowledgeMode(bool state)
{
    acknowledgeCommand = state;
}

/**
* @brief    Get CRLF Termination Enabled
* @param    void
* @return   flag - true means acknowledgement is enabled, false means disabled
*/
bool DParse::getAcknowledgeMode(void)
{
    return acknowledgeCommand;
}

/**
 * @brief   Add command to array
 *
 * @note    Each command may have up to DUCI_MESSAGE_MAX_PARAMETERS parameters only.
 *          Format specifiers for setFormat & getFormat:
 *
 *          =       assignment (set)
 *          ?       query (get)
 *          <n>i    'n' digits (eg, "2i" expects exactly 2 digits)
 *          <n>x    32-bit integer value: 'n' hex digits (eg, "4x" expects exactly 4 hex digits)
 *          <n>X    64-bit integer value: 'n' hex digits (eg, "8x" expects exactly 8 hex digits) - may be preceded by "0x"
 *          b       boolean (1 = true, 0 = false)
 *          c       ASCII character
 *          i       integer value
 *          v       floating point (with or without decimal places)
 *          d       date (always dd/mm/yyyy)
 *          t       time (always hh:mm:ss)
 *          [       indicates that following integer is optional - must have closing ']' after the specifier
 *          ]       indicates that preceding integer is optional - must have opening '[' before the specifier
 *          s       ASCII string
 *          $       whole string as parameter (for custom handling)
 *
 * @note    Only integer parameter type is allowed as optional
 *
 * @note    When specifying custom specifier, the setFunction and getFunction must be the same DIY function. The first
 *          one to be checked for will be the one called; safer to make both the same in case the order is ever changed.
 *
 * @param   command         - two command characters
 *          setFormat       - format specifier for the set command
 *          getFormat       - format specifier for the get command
 *          setFunction     - callback function for set command
 *          getFunction     - callback function for get command
 *          setPermissions  - required PIN mode for set function (access control, e.g. PIN modes required)
 *          getPermissions  - required PIN mode for get function (access control, e.g. PIN modes required)
 *
 * @return  void
 */
void DParse::addCommand(const char *command,
                        const char *setFormat,
                        const char *getFormat,
                        fnPtrDuci setFunction,
                        fnPtrDuci getFunction,
                        uint32_t setPermissions,
                        uint32_t getPermissions)
{
    //  if array full, increase the array capacity (by arbitrarily adding another block of commands)
    if(numCommands >= capacity)
    {
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
//        capacity += defaultSize;
//        commands = (sDuciCommand_t *)realloc(commands, capacity * sizeof(sDuciCommand_t));
    }

    //add new command at current index
    sDuciCommand_t *element = &commands[numCommands];

    if(NULL != element)
    {
        element->command = command;

        formatToArgs(setFormat, &element->setArgs[0]);
        formatToArgs(getFormat, &element->getArgs[0]);

        element->setFunction = setFunction;
        element->getFunction = getFunction;

        element->permissions.set = setPermissions;
        element->permissions.get = getPermissions;
    }

    //increment no of commands in array
    numCommands++;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 Error[Pm100]: dynamic heap memory allocation shall not be used (MISRA C 2004 rule 20.4)
 **********************************************************************************************************************/
_Pragma("diag_default=Pm100")

/**
 * @brief   Parse and process message string
 * @param   pointer to null-terminated string to transmit
 * @return  duciError is the error status
 */
sDuciError_t DParse::parse(char *str)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char *pData = str;

    //handle the message
    uint32_t msgSize = strlen(pData);

    if((uint32_t)(0) != msgSize)
    {
        //replace '\r' and '\n' with '\0' in the string

        for(uint32_t i = 0u; i < msgSize; i++)
        {
            if((pData[i] == '\r') || (pData[i] == '\n'))
            {
                pData[i] = '\0';
            }
        }

        //message size has changed
        msgSize = strlen(pData);

        //make sure the command is within minimum and maximum size limits
        if(msgSize < DUCI_MESSAGE_MIN_SIZE)
        {
            duciError.messageTooSmall = 1u;     //error command size not valid
        }

        else if(msgSize > DUCI_MESSAGE_MAX_SIZE)
        {
            duciError.messageTooBig = 1u;       //error command size not valid
        }

        else
        {
            //check if there is checksum at the end of the message
            int32_t digit1 = (int)pData[msgSize - 2u];
            int32_t digit2 = (int)pData[msgSize - 1u];

            //check that the last three characters are 'colon', 'digit', 'digit'
            if((pData[msgSize - 3u] == ':') && (ASCII_IS_DIG(digit1) == DEF_YES) && (ASCII_IS_DIG(digit2) == DEF_YES))
            {
                //validate if enabled, else just discard
                if(checksumEnabled == true)
                {
                    int32_t expectedChecksum = ((digit1 - (int32_t)'0') * 10) + (digit2 - (int32_t)'0');

                    int32_t actualChecksum = 0;

                    for(uint32_t i = 0u; i < (msgSize - 2u); i++)
                    {
                        actualChecksum += (int32_t)pData[i];
                    }

                    actualChecksum %= 100;

                    //check for match
                    if(actualChecksum != expectedChecksum)
                    {
                        duciError.invalidChecksum = 1u;
                    }
                }

                if((stripTrailingChecksum == true) || (checksumEnabled == true))
                {
                    //whether used or not if checksum is present at the end of the message then can remove it now
                    pData[msgSize - 3u] = '\0'; //terminating string at the ':' does it
                }
            }

            else if(checksumEnabled == true)
            {
                //checksum was expected but not found
                duciError.missing_args = 1u;
            }

            else
            {
                //empty 'else' block required to comply with MISRA C rules
            }

            //only proceed if no errors at this point
            if(duciError.value == 0u)
            {
                if(isMyStartCharacter(*pData) == false)
                {
                    duciError.needsStart = 1u;    //error command start character not valid
                }

                else                                //only proceed if no error
                {
                    //look at next two characters
                    pData++;

                    int32_t foundCmdIndex = -1; //-1 indicates command not found

                    for(int32_t i = 0; i < (int32_t)numCommands; i++)
                    {
                        //compare 2 characters, ignoring case
                        if(strncasecmp(pData, commands[i].command, (size_t)2) == 0)
                        {
                            foundCmdIndex = i;

                            //end search as we have found a match
                            break;
                        }
                    }

                    if(foundCmdIndex < 0)
                    {
                        duciError.unknownCommand = 1u; //error command not found
                    }

                    else
                    {
                        //move the pointer along to start of the command parameters
                        pData += 2;
                        duciError = processCommand(foundCmdIndex, pData);
                    }
                }
            }
        }
    }

    return duciError;
}

/**
* @brief    Get CRLF Termination Enabled
* @param    cmdIndex command structure array index
* @param    str pointer char array for return value to store command execution response
* @return   flag - true means acknowledgement is enabled, false means disabled
*/
sDuciError_t DParse::processCommand(int32_t cmdIndex, char *str)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    if(cmdIndex >= (int32_t)numCommands)
    {
        duciError.unknownCommand = 1u; //error command index not valid
    }

    else
    {
        sDuciCommand_t element = commands[cmdIndex];

        //only need to check the set command if there is a valid (non-NULL) callback function for it
        if(element.setFunction != NULL)
        {
            duciError = checkDuciString(&element.setArgs[0], str, element.setFunction, (ePinMode_t)element.permissions.set);
        }

        else
        {
            duciError.unhandledMessage = 1u;
        }

        //if 'no set command' or 'no match for it' then try the get command, if there is a non-NULL callback function
        if(((duciError.unhandledMessage == 1u) || (duciError.invalid_args == 1u)))
        {
            if(element.getFunction != NULL)
            {
                duciError = checkDuciString(&element.getArgs[0], str, element.getFunction, (ePinMode_t)element.permissions.get);
            }

            else
            {
                //duciError.unhandledMessage = 1u; <- TODO: don't need this as it will already be set to either invalid or unhandled
            }
        }
        else
        {
            //if acknowledgement of command is enabled then send back command characters
            if((acknowledgeCommand == true) && (myAckFunction != NULL))
            {
                if(NULL != myParent)
                {
                    myAckFunction(myParent, element.command);
                    //DCommsStateRemote::acknowledge(myParent, element.command);
                }
            }
        }
    }

    return duciError;
}

/**
 * @brief   Check instrument is in specified PIN mode (ie, protection level)
 * @param   pinMode is required PIN mode/permissions
 * @retval  true if instrument is in the required PIN mode
 */
bool DParse::checkPinMode(ePinMode_t pinMode)
{
    bool flag = false;

    //if PIN is required then need to check if we have permission
    if(pinMode == (ePinMode_t)E_PIN_MODE_NONE)
    {
        flag = true;
    }

    else
    {
        ePinMode_t currentPinMode = PV624->getPinMode();

        if(pinMode == currentPinMode)
        {
            flag = true;
        }
    }

    return flag;
}

/**
 * @brief   Check DUCI string and extecute callback function if parameters match found
 * @param   expectedArgs is the expected parameters for the command
 * @param   str is DUCI command string
 * @param   fnCallback is callback function if command syntax matches
 * @param   pinMode is required PIN mode/permissions
 * @retval  error status
 */
sDuciError_t DParse::checkDuciString(sDuciArg_t *expectedArgs, char *str, fnPtrDuci fnCallback, ePinMode_t pinMode)
{
    //check how many parameters are expected
    uint32_t expectedNumParameters = 0u; //expected no of parameters

    for(uint32_t i = 0u; (i < DUCI_MESSAGE_MAX_PARAMETERS); i++)
    {
        if(expectedArgs[i].argFree == true)
        {
            //premature end of count
            break;
        }

        expectedNumParameters++;    //add one to the count for each in-use (ie, not free) array element
    }

    //command already checked so need to look at format to see if there is a match
    sDuciParameter_t parameters[DUCI_MESSAGE_MAX_PARAMETERS];

    sDuciError_t duciError;

    //TODO: Need to check pin modes (element->accessControl = accessControl;)
    char *pData = str;

    //Check against expected command parameters for the command
    uint32_t numParameters = 0u;
    char *endptr = NULL;

    duciError.value = 0u; //start error free

    //go through string checking for parameter - finish when end of string, expected parameter count reached or error
    for(uint32_t i = 0u; ((i < expectedNumParameters) && (*pData != '\0') && (duciError.value == 0u)); i++)
    {
        //check command
        switch(expectedArgs[i].argType)
        {
        case argInteger: //signed or unsigned integer value
            duciError = getIntegerArg(pData, &parameters[i].intNumber, expectedArgs[i].argFieldWidth, &endptr);

            //if integer not successfully found then if it is optional then set unspecified default argument to (0)
            if((duciError .value != 0u) && (expectedArgs[i].argOptional == true))
            {
                parameters[i].intNumber = 0;
                duciError .value = 0u;  //assume no error at this stage
            }

            break;

        case argValue:           //floating point value (with or without decimal point)
            duciError = getValueArg(pData, &parameters[i].floatValue, &endptr);
            break;

        case argHexadecimal:     //32-bit hexadecimal value
            duciError = getHexadecimalArg(pData, &parameters[i].intNumber, expectedArgs[i].argFieldWidth, &endptr);
            break;

        case argLongHexadecimal: //64-bit hexadecimal value
            duciError = getLongHexadecimalArg(pData, &parameters[i].hexNumber, expectedArgs[i].argFieldWidth, &endptr);
            break;

        case argString:          //ascii character string
            duciError = getStringArg(pData, parameters[i].charArray, &endptr);
            break;

        case argDate:           //forward slash - as a separator in date specification
            duciError = getDateArg(pData, &parameters[i].date, &endptr);
            break;

        case argTime:            //colon - as a separator in time specification
            duciError = getTimeArg(pData, &parameters[i].time, &endptr);
            break;

        case argBoolean:        //boolean flag value
            duciError.invalid_args = 0u;

            if(*pData == '1')
            {
                parameters[i].flagValue = true;
            }

            else if(*pData == '0')
            {
                parameters[i].flagValue = false;
            }

            else
            {
                duciError.invalid_args = 1u;
            }

            endptr = pData + 1;
            break;

        case argCustom:         //custom, ie callback function will handle the parsing/interpreting
            memset_s(parameters[i].charArray, sizeof(parameters[i].charArray), 0,  sizeof(parameters[i].charArray));
            strncpy_s(parameters[i].charArray, sizeof(parameters[i].charArray), pData,  sizeof(parameters[i].charArray));
            duciError.invalid_args = 0u;
            endptr = pData + (int32_t)strlen(pData);
            break;

        case argCharacter:      //ascii character (returns just the character)
            parameters[i].charArray[0] = *pData;
            endptr = pData + 1;
            duciError.invalid_args = 0u;
            break;

        case argQuery:          //question mark
            if(*pData == '?')
            {
                parameters[i].charArray[0] = *pData;
                endptr = pData + 1;
                duciError.invalid_args = 0u;
            }

            else
            {
                duciError.invalid_args = 1u;
            }

            break;

        case argAssignment:     //equals sign
            if(*pData == '=')
            {
                parameters[i].charArray[0] = *pData;
                endptr = pData + 1;
                duciError.invalid_args = 0u;
            }

            else
            {
                duciError.invalid_args = 1u;
            }

            break;

        default:
            duciError.invalid_args = 1u;
            break;
        }

        if(endptr != NULL)
        {
            pData = endptr;
            numParameters++;

            //skip over separator - if next character is ',' unless it is the last parameter
            if((*pData == ',') && (i < (expectedNumParameters - 1u)))
            {
                pData++;
            }
        }
    }

    //if no errors, have reached end of string and not seen expected no of parameters then check if for trailing optional parameters
    if((duciError.value == 0u) && (*pData == '\0') && (expectedNumParameters != numParameters))
    {
        //go through string checking for parameter - finish when end of string, expected parameter count reached or error
        for(uint32_t i = numParameters; ((i < expectedNumParameters) && (duciError.value == 0u)); i++)
        {
            if(expectedArgs[i].argOptional == true)
            {
                switch(expectedArgs[i].argType)
                {
                case argInteger:
                    parameters[i].intNumber = 0;
                    break;

                case argAssignment:
                    //parameters[i].charArray[0] = '=';
                    //NOTE: The optional assignment is substituted with a null to indicate that the following
                    //parameters are not specified for the command. The call back function will handle it appropriately.
                    parameters[i].charArray[0] = '\0';
                    break;

                case argValue:
                    parameters[i].floatValue = 0.0f;
                    break;

                case argCustom:
                    parameters[i].charArray[0] = '\0';
                    break;

                default:
                    duciError.invalid_args = 1u;
                    break;
                }

                numParameters++;
            }
        }
    }

    //if no of parameters is incorrect or there are more characters after the parameter check above then args invalid
    if((*pData != '\0') || (expectedNumParameters != numParameters))
    {
        duciError.invalid_args = 1u;
    }

    else if(duciError.value == 0u)  //if no error then can call the specified function
    {
        //check permission here
        if(checkPinMode(pinMode) == true)
        {
            if(NULL != myParent)
            {
                duciError = fnCallback(myParent, &parameters[0]);
            }

            else
            {
                duciError.commandFailed = 1u;
            }
        }

        else
        {
            duciError.invalidMode = 1u;
        }
    }

    else
    {
        //required to comply with MISRA C rules
    }

    return duciError;
}

/**
 * @brief   Convert format specifier string to array of argument types
 * @param   pointer to null-terminated format string
 * @param   pointer to array of arguments
 * @return  numArgs is the number of arguments specified
 */
uint32_t DParse::formatToArgs(const char *formatSpec, sDuciArg_t *args)
{
    //up to DUCI_MESSAGE_MAX_PARAMETERS arguments
    const char *str = formatSpec;
    char *endptr = NULL;
    sDuciError_t argError;
    argError.value = 0u;

    uint32_t numArgs = 0u;

    //mark all argument 'slots' as free
    for(uint32_t i = 0u; i < DUCI_MESSAGE_MAX_PARAMETERS; i++)
    {
        args[i].argFree = true;
    }

    while((str != NULL) && (argError.value == 0u) && (numArgs < DUCI_MESSAGE_MAX_PARAMETERS))
    {
        argError = getArgument(str, &args[numArgs], (const char **)&endptr);
        str = endptr;
        numArgs++;
    }

    //TODO: This should return the error. numArgs is not used anywhere anyway!
    return numArgs;
}

/*
    @brief      Get argument from string
    @param      buffer - pointer to character string buffer
    @param[out] arg - pointer to argument
    @param[out] endptr - pointer to next character in buffer after the argument
    @return     error code
*/
sDuciError_t DParse::getArgument(const char *buffer, sDuciArg_t *arg, const char **endptr)
{
    sDuciError_t argError;
    argError.value = 0u;
    arg->argOptional = false; //assume argument is not optional by default

    if((NULL != buffer) && (NULL != arg) && (NULL != endptr))
    {
        //read first character
        char ch = *buffer;

        if(ch == '[')
        {
            arg->argOptional = true;

            //move past the square bracket
            buffer++;

            //read next character
            ch = *buffer;
        }

        //assume no specified field width (signified by value 0)
        arg->argFieldWidth = 0u;

        //if numeric then this is a field width specification (up to two digits max)
        if(ASCII_IS_DIG((int)ch) == DEF_YES)
        {
            //move pointer to next character, as it has already been seen
            buffer++;

            arg->argFieldWidth = (uint32_t)ch - (uint32_t)ASCII_CHAR_DIGIT_ZERO;

            for(int i = 0; i < 2; i++)
            {
                //check the next one
                ch = *buffer++;

                if(ASCII_IS_DIG((int)ch) == DEF_YES)
                {
                    arg->argFieldWidth = (arg->argFieldWidth * 10u) + (uint32_t)ch - (uint32_t)ASCII_CHAR_DIGIT_ZERO;
                }

                else
                {
                    break;
                }
            }
        }

        else if(arg->argOptional == true)
        {
            //we already have ch holding this character's value so can move pointer on to next character
            buffer++;
        }

        else
        {
            //all if, else if constructs should contain a final else clause (MISRA C 2004 rule 14.10)
        }

        //now check character that indicates type of argument, which has to be this character
        switch(ch)
        {
        case 'i': //integer number (can have leading minus sign)
            arg->argType = argInteger;
            break;

        case 's': //string of null-terminated characters
            arg->argType = argString;
            break;

        case 'x': //32-bit hexadecimal number (unsigned)
            arg->argType = argHexadecimal;
            break;

        case 'X': //64-bit hexadecimal number (unsigned)
            arg->argType = argLongHexadecimal;
            break;

        case 'b': //boolean - a single digit number 0 (false) or 1 (true)
            arg->argType = argBoolean;
            break;

        case 'c': //ASCII character
            arg->argType = argCharacter;
            break;

        case 'v': //value - a floating point number (with or without a decimal point)
            arg->argType = argValue;
            break;

        case '=': //equals sign
            arg->argType = argAssignment;
            break;

        case '?': //question mark
            arg->argType = argQuery;
            break;

        case '$': //dollar sign - custom parameter
            arg->argType = argCustom;
            break;

        case 'd': //date
            arg->argType = argDate;
            break;

        case 't': //time
            arg->argType = argTime;
            break;

        default:
            argError.invalid_args = 1u;
            break;
        }

        //only continue if no error
        if(argError.value == 0u)
        {
            //read next character - hold the pointer at this character
            ch = *buffer;

            //for optional parameters there must be a closing square bracket
            if(arg->argOptional == true)
            {
                if(ch != ']')
                {
                    argError.invalid_args = 1u;
                }
            }

            //only continue if no error
            if(argError.value == 0u)
            {
                //this arg slot is no longer free
                arg->argFree = false;

                //Check for special case of next character being question mark and the query parameter not already seen above
                if((ch == '?') && (arg->argType != (uint32_t)argQuery))
                {
                    //keep pointer here so character may be handled on next call
                }
                else
                {
                    //can move to check next character
                    buffer++;

                    //next character should be a separator (',') or end of string (null)
                    ch = *buffer;

                    if(ch == '\0')
                    {
                        buffer = NULL; //indicates end of string
                    }

                    else if(ch == ',')
                    {
                        //skip over the separator
                        buffer++;
                    }

                    else
                    {
                        //all if, else if constructs should contain a final else clause (MISRA C 2004 rule 14.10)
                    }
                }
            }

            //set pointer to next character in string
            *endptr = buffer;
        }
    }

    return argError;
}

/*
    @brief      Get integer value argument from string
    @param      buffer ia the pointer to character string buffer
    @param[out] intNumber is the location of return value
    @param      fieldWidth is the expected number of characters
    @param[out] endptr is the pointer to next character in buffer after the argument
    @return     error code
*/
sDuciError_t DParse::getIntegerArg(char *buffer, int32_t *intNumber, uint32_t fieldWidth, char **endptr)
{
    sDuciError_t argError;
    argError.value = 0u;
    bool minus = false;
    char *pData = buffer;
    char ch = *pData;

    //check first character which can be a minus sign
    if(ch == '-')
    {
        minus = true;
        pData++;
    }

    int32_t intValue = 0;

    //if fieldWidth is not specified (then no limit to length - use 11 as the biggest number can be -4294967296)
    uint32_t length = fieldWidth;

    if(length == 0u)
    {
        length = 11u;
    }

    for(uint32_t i = 0u; i < length; i++)
    {
        ch = *pData;

        if(ASCII_IsDig(ch) == DEF_TRUE)
        {
            intValue = (intValue * 10) + (int32_t)ch - (int32_t)'0';
            pData++; //move pointer to next character
        }

        else
        {
            //if premature exit then signal error; (fieldWidth > 0u) means that the no of digits must be specified fixed number
            if((i == 0u) || ((fieldWidth > 0u) && (i < fieldWidth)))
            {
                argError.invalid_args = 1u;
            }

            //exit at first non-digit character
            break;
        }
    }

    //error not checked here because value won't be used anyway if there is a problem
    if(minus == true)
    {
        intValue = -intValue;
    }

    *intNumber = intValue;

    *endptr = pData;

    return argError;
}

/*
    @brief      Get 32-bit hexadecimal value argument from string
    @param      buffer - pointer to character string buffer
    @param[out] intNumber - location of return value
    @param      fieldWidth - expected number of characters
    @param[out] endptr - pointer to next character in buffer after the argument
    @return     error code
*/
sDuciError_t DParse::getHexadecimalArg(char *buffer, int32_t *intNumber, uint32_t fieldWidth, char **endptr)
{
    sDuciError_t argError;
    argError.value = 0u;
    argError.value = 0u;

    uint32_t hexValue = 0u;

    //if fieldWidth is not specified (then no limit to length - use 4 as the biggest number can be FFFF)
    uint32_t length = fieldWidth;

    if(length == 0u)
    {
        length = 4u;
    }

    for(uint32_t i = 0u; i < length; i++)
    {
        char ch = *buffer;

        if(ASCII_IsDig(ch) == DEF_TRUE)
        {
            hexValue = (hexValue * 16u) + ch - '0';
            buffer++;
        }

        else
        {
            ch = ASCII_ToUpper(ch);

            if((ch >= 'A') && (ch <= 'F'))
            {
                hexValue = (hexValue * 16u) + 10u + ch - 'A';
                buffer++;
            }

            else
            {
                //if premature exit then signal error
                if(((i == 0u) || ((fieldWidth > 0u) && (i < (fieldWidth - 1u)))))
                {
                    argError.invalid_args = 1u;
                }

                break;
            }
        }
    }

    *intNumber = (int32_t)hexValue;

    *endptr = buffer;

    return argError;
}

/*
    @brief      Get 64-bit hexadecimal value argument from string (may be preceded by 0x)
    @param      buffer - pointer to character string buffer
    @param[out] hexNumber - location of return value
    @param      fieldWidth - expected number of characters
    @param[out] endptr - pointer to next character in buffer after the argument
    @return     error code
*/
sDuciError_t DParse::getLongHexadecimalArg(char *buffer, uint64_t *hexNumber, uint32_t fieldWidth, char **endptr)
{
    sDuciError_t argError;
    argError.value = 0u;
    argError.value = 0u;

    uint64_t hexValue = 0u;

    //check for possible preceding "0x"
    if((buffer[0] == '0') && (buffer[1] == 'x'))
    {
        //skip over these two characters
        buffer += 2;
    }

    //if fieldWidth is not specified (then no limit to length - use 8 as the biggest number can be FFFFFFFF)
    uint32_t length = fieldWidth;

    if(length == 0u)
    {
        length = 8u;
    }

    for(uint32_t i = 0u; i < length; i++)
    {
        char ch = *buffer;

        if(ASCII_IsDig(ch) == DEF_TRUE)
        {
            hexValue = (hexValue * 16u) + ch - '0';
            buffer++;
        }

        else
        {
            ch = ASCII_ToUpper(ch);

            if((ch >= 'A') && (ch <= 'F'))
            {
                hexValue = (hexValue * 16u) + 10u + ch - 'A';
                buffer++;
            }

            else
            {
                //if premature exit then signal error
                if(((i == 0u) || ((fieldWidth > 0u) && (i < (fieldWidth - 1u)))))
                {
                    argError.invalid_args = 1u;
                }

                break;
            }
        }
    }

    *hexNumber = hexValue;

    *endptr = buffer;

    return argError;
}

/*
    @brief      Get argument from string
    @param      buffer - pointer to character string buffer
    @param[out] floatValue - location of return value
    @param[out] endptr - pointer to next character in buffer after the argument
    @return     error code
*/
sDuciError_t DParse::getValueArg(char *buffer, float32_t *floatValue, char **endptr)
{
    sDuciError_t argError;
    argError.value = 0u;

    *floatValue = strtof(buffer, endptr);

    return argError;
}

/*
    @brief      Get string value argument from string
    @param      buffer - pointer to character string buffer
    @param[out] str - location of return value
    @param[out] endptr - pointer to next character in buffer after the argument
    @return     error code
*/
sDuciError_t DParse::getStringArg(char *buffer, char *str, char **endptr)
{
    sDuciError_t argError;
    argError.value = 0u;
    char *pSrc = buffer;
    char *pDest = str;

    //TODO: strings containing commas? use quotation marks?
    //TODO: bounds checking?
    while((*pSrc != '\0') && (*pSrc != ','))
    {
        *pDest++ = *pSrc++;
    }

    //null terminate
    *pDest = '\0';

    *endptr = pSrc;

    return argError;
}

/**
 * @brief   Check if the character is my start character
 * @note    The base class function accepts master or slave start characters
 * @param   ch - is the character
 * @return  true if ch is a start character for parser, else false
 */
sDuciError_t DParse::getDateArg(char *buffer, sDate_t *pDate, char **endptr)
{
    //TODO: Factor this out to not repeat code in date and time functions
    sDuciError_t argError;
    argError.value = 0u;
    char *pData = buffer;
    uint32_t intValue = 0u;
    char separator = '/';
    char ch;

    int32_t len = (int32_t)strlen(buffer);

    //string must be at least 10 bytes in size (required for dd/mm/yyyy)
    if(len < 10)
    {
        argError.invalid_args = 1u;
    }

    else
    {
        for(uint32_t i = 0u; i < 2u; i++)
        {
            ch = *pData++;

            if(ASCII_IsDig(ch) == DEF_TRUE)
            {
                intValue = (intValue * 10u) + (uint32_t)ch - (uint32_t)'0';
            }

            else
            {
                argError.invalid_args = 1u;
            }
        }

        //continue if no error
        if(argError.value == 0u)
        {
            ch = *pData++;

            if(ch != separator)
            {
                argError.invalid_args = 1u;
            }

            else
            {
                pDate->day = (uint16_t)intValue;
                intValue = 0u;

                for(uint32_t i = 0u; i < 2u; i++)
                {
                    ch = *pData++;

                    if(ASCII_IsDig(ch) == DEF_TRUE)
                    {
                        intValue = (intValue * 10u) + (uint32_t)ch - (uint32_t)'0';
                    }

                    else
                    {
                        argError.invalid_args = 1u;
                    }
                }

                //continue if no error
                if(argError.value == 0u)
                {
                    ch = *pData++;

                    if(ch != separator)
                    {
                        argError.invalid_args = 1u;
                    }

                    else
                    {
                        pDate->month = (uint16_t)intValue;
                        intValue = 0u;

                        for(uint32_t i = 0u; i < 4u; i++)
                        {
                            ch = *pData++;

                            if(ASCII_IsDig(ch) == DEF_TRUE)
                            {
                                intValue = (intValue * 10u) + (uint32_t)ch - (uint32_t)'0';
                            }

                            else
                            {
                                argError.invalid_args = 1u;
                            }
                        }

                        //continue of no error
                        if(argError.value == 0u)
                        {
                            pDate->year = intValue;
                        }

                        //validate date
                        if(isDateValid(pDate->day, (uint32_t)pDate->month, (uint32_t)pDate->year) == false)
                        {
                            argError.invalid_args = 1u;
                        }
                    }
                }
            }
        }
    }

    *endptr = pData;

    return argError;
}

/**
 * @brief   Check if the character is my start character
 * @note    The base class function accepts master or slave start characters
 * @param   ch - is the character
 * @return  true if ch is a start character for parser, else false
 */
sDuciError_t DParse::getTimeArg(char *buffer, sTime_t *pTime, char **endptr)
{
    //TODO: Factor this out to not repeat code in date and time functions
    sDuciError_t argError;
    argError.value = 0u;
    char *pData = buffer;
    uint32_t intValue = 0u;
    char ch;
    char separator = ':';

    int32_t len = (int32_t)strlen(buffer);

    //string must be at least 8 bytes in size (required for hh:mm:ss)
    if(len < 8)
    {
        argError.invalid_args = 1u;
    }

    else
    {
        for(uint32_t i = 0u; i < 2u; i++)
        {
            ch = *pData++;

            if(ASCII_IsDig(ch) == DEF_TRUE)
            {
                intValue = (intValue * 10u) + (uint32_t)ch - (uint32_t)'0';
            }

            else
            {
                argError.invalid_args = 1u;
            }
        }

        //continue if no error
        if(argError.value == 0u)
        {
            ch = *pData++;

            if(ch != separator)
            {
                argError.invalid_args = 1u;
            }

            else
            {
                pTime->hours = (uint16_t)intValue;
                intValue = 0u;

                for(uint32_t i = 0u; i < 2u; i++)
                {
                    ch = *pData++;

                    if(ASCII_IsDig(ch) == DEF_TRUE)
                    {
                        intValue = (intValue * 10u) + (uint32_t)ch - (uint32_t)'0';
                    }

                    else
                    {
                        argError.invalid_args = 1u;
                    }
                }

                //continue if no error
                if(argError.value == 0u)
                {
                    ch = *pData++;

                    if(ch != separator)
                    {
                        argError.invalid_args = 1u;
                    }

                    else
                    {
                        pTime->minutes = (uint16_t)intValue;
                        intValue = 0u;

                        for(uint32_t i = 0u; i < 2u; i++)
                        {
                            ch = *pData++;

                            if(ASCII_IsDig(ch) == DEF_TRUE)
                            {
                                intValue = (intValue * 10u) + (uint32_t)ch - (uint32_t)'0';
                            }

                            else
                            {
                                argError.invalid_args = 1u;
                            }
                        }

                        //continue of no error
                        if(argError.value == 0u)
                        {
                            pTime->seconds = intValue;
                        }
                    }
                }
            }
        }
    }

    *endptr = pData;

    return argError;
}

/**
 * @brief   Check if the character is my start character
 * @note    The base class function accepts master or slave start characters
 * @param   ch - is the character
 * @return  true if ch is a start character for parser, else false
 */
bool DParse::isMyStartCharacter(char ch)
{
    bool isMyStart = true;
    messageType = E_DUCI_COMMAND;

    switch(ch)
    {
    case '*':
    case '#':
        break;

    case '!':
        messageType = E_DUCI_REPLY;
        break;

    default:
        isMyStart = false;
        messageType = E_DUCI_UNEXPECTED;
        break;
    }

    return isMyStart;
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 10.3. Ignoring this - explicit conversion from 'signed int' to 'char' is safe
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm136")

/**
 * @brief   Prepare message in specified transmit buffer
 * @param   str - is the character string to transmit
 * @param   buffer - is the buffer in which the string is prepared
 * @param   bufferSize is the size of the buffer
 * @return  true if completed successfully, else false
 */
bool DParse::prepareTxMessage(char *str, char *buffer, uint32_t bufferSize)
{
    bool successFlag = false;

    uint32_t size = strlen(str);

    if(size < (bufferSize - 6u))
    {
        int32_t checksum = 0;

        //checksum only necessary if enabled
        if(getChecksumEnabled() == true)
        {
            for(uint32_t i = 0u; i < size; i++)
            {
                buffer[i] = str[i];
                checksum += (int32_t)str[i];
            }

            //checksum include the semi-colon
            buffer[size++] = ':';
            checksum += (int32_t)':';

            checksum %= 100;

            buffer[size++] = '0' + (char)(checksum / 10);
            buffer[size++] = '0' + (char)(checksum % 10);
        }

        else
        {
            strncpy_s(buffer, bufferSize, str, size);
        }

        if(getTerminatorCrLf() == true)
        {
            buffer[size++] = '\r';    //CR sent only if enabled
        }

        buffer[size++] = '\n';         //always send LF
        buffer[size] = '\0';           //always null terminate

        successFlag = true;
    }

    return successFlag;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma("diag_default=Pm136")
