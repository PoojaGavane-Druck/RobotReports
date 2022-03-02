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
* @file     DBinaryParser.cpp
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     27-Feb-2021
*
* @brief    Binary Parser for Stepper Motor Source File
*/

/* Includes -----------------------------------------------------------------*/
#include "DEngProtocolParser.h"
#include "Utilities.h"
#include "DPV624.h"
#include "main.h"

/* Constants & Defines ------------------------------------------------------*/



#define LOC_COMMAND 0
#define LOC_DATA 1



#define DEFAULT_COMMANDS_NUM 16u

/* Variables ----------------------------------------------------------------*/

/* Functions *****************************************************************/

/**
* @brief    Constructor
*
* @param    creator is owner of this instance
* @param    osErr is pointer to OS error
*
* @return   void
*/
DEngProtocolParser::DEngProtocolParser(void *creator, sEngProtcolCommand_t *commandArray, size_t maxCommands, OS_ERR *os_error)
{
    //initialise the command set
    myParent = creator;
    commands = commandArray;
    capacity = maxCommands;
    numCommands = (size_t)(0);
    messageType = (eEngProtocolMessage_t)E_ENG_PROTOCOL_UNEXPECTED;

}

/**
* @brief    Destructor
* @param    void
* @return   void
*/
DEngProtocolParser::~DEngProtocolParser()
{

    commands = NULL;
    numCommands = (size_t)0;
    capacity = (size_t)0;
}
/**
 * @brief   Prepare message in specified transmit buffer
 * @param   str - is the character string to transmit
 * @param   buffer - is the buffer in which the string is prepared
 * @param   bufferSize is the size of the buffer
 * @return  true if completed successfully, else false
 */
bool DEngProtocolParser::prepareResponse(sEngProtocolParameter_t *params,
        uint8_t numOfParams,
        uint8_t *txBuffer,
        uint32_t *txBufferLen)
{
    bool retStatus = false;
    uint32_t index = (uint32_t)0;


    if((txBuffer != NULL) && (txBufferLen != NULL))
    {
        if(NULL != params)
        {
            for(uint8_t count = (uint8_t)0; count < numOfParams; count++)
            {
                for(uint8_t byteCount = (uint8_t)0; byteCount < (uint8_t)4; byteCount++)
                {
                    txBuffer[index++] = params[count].byteArray[byteCount];
                }
            }
        }

        *txBufferLen = index;
        retStatus = true;
    }

    else
    {
        retStatus = false;
    }

    return retStatus;
}
/**
 * @brief   Add command to array
 *
 * @param   command     - two command chracters
 *          setFormat   - format specifier for the set command
 *          getFormat   - format specifier for the get command
 *          setFunction - callback function for set command
 *          getFunction - callback function for get command
 *          permissions - permissions (access control, eg PIN modes required)
 *
 * @return  void
 */
void DEngProtocolParser::addCommand(uint8_t command,
                                    eDataType_t dataType,
                                    fnPtrEngProtoParam fnParam,
                                    uint32_t commandDataLength,
                                    uint32_t responseDataLength)
{
    //  if array full, increase the array capacity (by arbitrarily adding another block of commands)
    if(numCommands >= capacity)
    {
        capacity += (size_t)(DEFAULT_COMMANDS_NUM);
        //commands = (sCommand_t *)realloc(commands, capacity * sizeof(sCommand_t));
    }

    //add new command at current index
    sEngProtcolCommand_t *cmdSet = &commands[numCommands];

    cmdSet->command = command;
    cmdSet->fnParam = fnParam;
    cmdSet->dataType = dataType;
    cmdSet->commandDataLength = commandDataLength;
    cmdSet->responseDataLength = responseDataLength;

    //increment no of commands in array
    numCommands++;
}

/**
 * @brief   Parse and process message string
 * @param   pointer to null-terminated string to transmit
 * @return  duciError is the error status
 */
sEngProError_t DEngProtocolParser::parse(uint8_t *ptrBuffer, uint32_t msgSize)
{
    sEngProError_t error;
    uint32_t index = (uint32_t)(0);
    sEngProtcolCommand_t *commandSet = NULL;
    eDataType_t dataType = eDataTypeNone;
    bool statusFlag = false;
    uint8_t expectedMsgLength = (uint8_t)(0);
    sEngProtocolParameter_t param;
    error.value = 1u;

    /* Scan through the commands to find the command */
    if(NULL != ptrBuffer)
    {
        for(index = (uint32_t)(0); index < numCommands; index++)
        {
            commandSet = &commands[index];

            if(ptrBuffer[LOC_COMMAND] == commandSet->command)
            {
                dataType = commandSet->dataType;
                expectedMsgLength = (uint8_t)((uint8_t)commandSet->commandDataLength + 1u);
                error.value = 0u;
                statusFlag = true;
                break;
            }
        }



        /* Validate length of the message */
        if(true == statusFlag)
        {
            if(msgSize < (uint32_t)(expectedMsgLength))
            {
                error.messageTooSmall = 1u;
                statusFlag = false;
            }
        }

        else
        {
            /* Command not found */
            error.messageIsNotCmdType = 1u;
            statusFlag = false;
        }

        if(true == statusFlag)
        {
            if((uint8_t)ptrBuffer[LOC_COMMAND] == (uint8_t)ptrBuffer[LOC_COMMAND])
            {
                GetValueFromBuffer((uint8_t *)&ptrBuffer[LOC_DATA], dataType, (sEngProtocolParameter_t *)&param);

                if(expectedMsgLength == (uint8_t)msgSize)
                {
                    messageType = E_ENG_PROTOCOL_COMMAND;
                    commandSet->fnParam(myParent, &param);
                    error.value = 0u;
                }

                else
                {
                    error.messageTooSmall = 1u;
                    statusFlag = false;
                }
            }
        }

        else
        {

        }
    }

    return error;
}


/**
 * @brief   This function gets the value data from buffer
 * @param   void
 * @return  void
 */
bool DEngProtocolParser::GetValueFromBuffer(uint8_t *buffer, eDataType_t dataType, sEngProtocolParameter_t *ptrParam)
{
    bool statusFlag = true;

    switch(dataType)
    {
    case eDataTypeBoolean:
        break;

    case eDataTypeByte:
        break;

    case eDataTypeUnsignedChar:
        break;

    case eDataTypeSignedChar:
        break;

    case eDataTypeUnsignedShort:
        break;

    case eDataTypeSignedShort:
        break;

    case eDataTypeUnsignedLong:
        ptrParam->uiValue = GetUint32FromBuffer(buffer);
        break;

    case eDataTypeSignedLong:
        ptrParam->iValue = GetInt32FromBuffer(buffer);
        break;

    case eDataTypeFloat:
        ptrParam->floatValue = GetFloatFromBuffer(buffer);
        break;

    case eDataTypeDouble:
        break;

    default:
        statusFlag = false;
        break;
    }

    return statusFlag;
}

/**
 * @brief   This function converts a 4 byte buffer to a float value
 * @param   void
 * @return  void
 */
float DEngProtocolParser::GetFloatFromBuffer(uint8_t *buffer)
{
    uFloat_t uFloatVal;

    float value = (float)(0);

    uFloatVal.floatValue = (float)(0);

    uFloatVal.byteValue[0] = (uint8_t)(buffer[0]);
    uFloatVal.byteValue[1] = (uint8_t)(buffer[1]);
    uFloatVal.byteValue[2] = (uint8_t)(buffer[2]);
    uFloatVal.byteValue[3] = (uint8_t)(buffer[3]);

    value = uFloatVal.floatValue;

    return (value);
}

/**
 * @brief   This function converts a 4 byte buffer to a unsigned long integer
 * @param   void
 * @return  void
 */

uint32_t DEngProtocolParser::GetUint32FromBuffer(uint8_t *buffer)
{
    uUint32_t uValue;
    uValue.uint32Value = (uint32_t)(0);

    uValue.byteValue[0] = (uint8_t)(buffer[0]);
    uValue.byteValue[1] = (uint8_t)(buffer[1]);
    uValue.byteValue[2] = (uint8_t)(buffer[2]);
    uValue.byteValue[3] = (uint8_t)(buffer[3]);



    return (uValue.uint32Value);
}

/**
 * @brief   This function converts a 4 byte buffer to a unsigned long integer
 * @param   void
 * @return  void
 */

int32_t DEngProtocolParser::GetInt32FromBuffer(uint8_t *buffer)
{
    uSint32_t value;
    value.int32Value = (int32_t)(0);

    value.byteValue[0] = (uint8_t)(buffer[0]);
    value.byteValue[1] = (uint8_t)(buffer[1]);
    value.byteValue[2] = (uint8_t)(buffer[2]);
    value.byteValue[3] = (uint8_t)(buffer[3]);



    return (value.int32Value);
}
/**
 * @brief   This function converts a 4 byte buffer to a unsigned short
 * @param   void
 * @return  void
 */

uint16_t DEngProtocolParser::GetUint16FromBuffer(uint8_t *buffer)
{
    uUint16_t uShortVal;

    uShortVal.uint16Value = (uint16_t)(0);

    uShortVal.byteValue[0] = (uint8_t)(buffer[0]);
    uShortVal.byteValue[1] = (uint8_t)(buffer[1]);

    return uShortVal.uint16Value;
}



/**
 * @brief   This function converts a 1 byte buffer to a signed short
 * @param   void
 * @return  void
 */

uint8_t DEngProtocolParser::GetUint8FromBuffer(uint8_t *buffer)
{
    uint8_t value = (uint8_t)(0);

    value = value | (uint8_t)(buffer[0]);

    return (value);
}

/**
 * @brief   This function converts a 1 byte buffer to a signed short
 * @param   void
 * @return  void
 */

int8_t DEngProtocolParser::GetInt8FromBuffer(uint8_t *buffer)
{
    int8_t value = (int8_t)(0);

    value = (int8_t)(buffer[0]);

    return (value);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DEngProtocolParser::GetBufferFromValue(float *value, uint8_t *buffer)
{
    GetBufferFromFloat(value, buffer);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DEngProtocolParser::GetBufferFromValue(uint32_t *value, uint8_t *buffer)
{
    GetBufferFromLong(value, buffer);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DEngProtocolParser::GetBufferFromValue(uint16_t *value, uint8_t *buffer)
{
    GetBufferFromShort(value, buffer);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DEngProtocolParser::GetBufferFromValue(uint8_t *value, uint8_t *buffer)
{
    GetBufferFromChar(value, buffer);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DEngProtocolParser::GetBufferFromLong(uint32_t *value, uint8_t *buffer)
{
    uUint32_t uLongVal;

    uLongVal.uint32Value = (uint32_t)(0);
    uLongVal.uint32Value = *value;
    buffer[0] = uLongVal.byteValue[0];
    buffer[1] = uLongVal.byteValue[1];
    buffer[2] = uLongVal.byteValue[2];
    buffer[3] = uLongVal.byteValue[3];
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DEngProtocolParser::GetBufferFromFloat(float *value, uint8_t *buffer)
{
    uFloat_t uFloatVal;

    uFloatVal.floatValue = (float32_t)(0.0);
    uFloatVal.floatValue = *value;
    buffer[0] = uFloatVal.byteValue[0];
    buffer[1] = uFloatVal.byteValue[1];
    buffer[2] = uFloatVal.byteValue[2];
    buffer[3] = uFloatVal.byteValue[3];
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DEngProtocolParser::GetBufferFromShort(uint16_t *value, uint8_t *buffer)
{
    uUint16_t uShortVal;

    uShortVal.uint16Value = (uint16_t)(0);
    uShortVal.uint16Value = *value;
    buffer[0] = uShortVal.byteValue[0];
    buffer[1] = uShortVal.byteValue[1];

}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DEngProtocolParser::GetBufferFromChar(uint8_t *value, uint8_t *buffer)
{
    buffer[0] = *value;
}


