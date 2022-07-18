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

/* Error handler instance parameter starts from 101 to 200 */
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
    uint32_t index = 0u;


    if((txBuffer != NULL) && (txBufferLen != NULL))
    {
        if(NULL != params)
        {
            for(uint8_t count = 0u; count < numOfParams; count++)
            {
                for(uint8_t byteCount = 0u; byteCount < 4u; byteCount++)
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
 * @return  sEngProError_t is the error status
 */
sEngProError_t DEngProtocolParser::parse(uint8_t *ptrBuffer, uint32_t msgSize)
{
    sEngProError_t error;
    uint32_t index = 0u;
    sEngProtcolCommand_t *commandSet = NULL;
    eDataType_t dataType = eDataTypeNone;
    bool statusFlag = false;
    uint8_t expectedMsgLength = 0u;
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
                getValueFromBuffer((uint8_t *)&ptrBuffer[LOC_DATA], dataType,
                                   (sEngProtocolParameter_t *)&param);

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
 * @brief   This function gets the value  from buffer
 * @param   eDataType_t dataType  Tells the data type of parameter
 * @param  sEngProtocolParameter_t *ptrParam pointer to the parameter union which contains data
 * @return  true if success or false if it fails
 */
bool DEngProtocolParser::getValueFromBuffer(uint8_t *buffer,
        eDataType_t dataType,
        sEngProtocolParameter_t *ptrParam)
{
    bool statusFlag = true;

    switch(dataType)
    {
    case eDataTypeBoolean:
    case eDataTypeByte:
    case eDataTypeUnsignedChar:
    case eDataTypeSignedChar:
    case eDataTypeUnsignedShort:
    case eDataTypeSignedShort:
    case eDataTypeDouble:
        statusFlag = false;
        break;

    case eDataTypeUnsignedLong:
        ptrParam->uiValue = getUint32FromBuffer(buffer);
        break;

    case eDataTypeSignedLong:
        ptrParam->iValue = getInt32FromBuffer(buffer);
        break;

    case eDataTypeFloat:
        ptrParam->floatValue = getFloatFromBuffer(buffer);
        break;



    default:
        statusFlag = false;
        break;
    }

    return statusFlag;
}

/**
 * @brief   This function converts a 4 byte buffer to a float value
 * @param   uint8_t *buffer -- pointer to unsigned char which contains float
            value in byte format
 * @return  returns converted float value
 */
float DEngProtocolParser::getFloatFromBuffer(uint8_t *buffer)
{
    uFloat_t uFloatVal;

    float value = 0.0f;

    uFloatVal.floatValue = 0.0f;

    uFloatVal.byteValue[0] = (uint8_t)(buffer[0]);
    uFloatVal.byteValue[1] = (uint8_t)(buffer[1]);
    uFloatVal.byteValue[2] = (uint8_t)(buffer[2]);
    uFloatVal.byteValue[3] = (uint8_t)(buffer[3]);

    value = uFloatVal.floatValue;

    return (value);
}

/**
 * @brief   This function converts a 4 byte buffer to a unsigned long integer
 * @param   uint8_t *buffer -- pointer to unsigned char  which contains usigned
            long integer value in byte format
 * @return  returns converted unsinged long integer value
 */

uint32_t DEngProtocolParser::getUint32FromBuffer(uint8_t *buffer)
{
    uUint32_t uValue;
    uValue.uint32Value = 0u;

    uValue.byteValue[0] = (uint8_t)(buffer[0]);
    uValue.byteValue[1] = (uint8_t)(buffer[1]);
    uValue.byteValue[2] = (uint8_t)(buffer[2]);
    uValue.byteValue[3] = (uint8_t)(buffer[3]);



    return (uValue.uint32Value);
}

/**
 * @brief   This function converts a 4 byte buffer to a signed long integer
 * @param   uint8_t *buffer -- pointer to unsigned char which contains
            signed long integer value in byte format
 * @return  returns converted singed long integer value
 */

int32_t DEngProtocolParser::getInt32FromBuffer(uint8_t *buffer)
{
    uSint32_t value;
    value.int32Value = 0;

    value.byteValue[0] = (uint8_t)(buffer[0]);
    value.byteValue[1] = (uint8_t)(buffer[1]);
    value.byteValue[2] = (uint8_t)(buffer[2]);
    value.byteValue[3] = (uint8_t)(buffer[3]);



    return (value.int32Value);
}

/**
 * @brief   This function converts a 4 byte buffer to a unsigned short
 * @param   uint8_t *buffer -- pointer to unsigned char which contains
            unsigned short value in byte format
 * @return  returns converted unsigned short value
 */

uint16_t DEngProtocolParser::getUint16FromBuffer(uint8_t *buffer)
{
    uUint16_t uShortVal;

    uShortVal.uint16Value = 0u;

    uShortVal.byteValue[0] = (uint8_t)(buffer[0]);
    uShortVal.byteValue[1] = (uint8_t)(buffer[1]);

    return uShortVal.uint16Value;
}



/**
 * @brief   This function converts a 1 byte buffer to a unsigned char
 * @param   uint8_t *buffer -- pointer to unsigned char  which contains
            unsigned char value
 * @return  uint8_t  returns converted unsigned char value
 */

uint8_t DEngProtocolParser::getUint8FromBuffer(uint8_t *buffer)
{
    uint8_t value = 0u;

    value = value | (uint8_t)(buffer[0]);

    return (value);
}


/**
 * @brief   This function converts a 1 byte buffer to a signed char
 * @param   uint8_t *buffer -- pointer to unsigned char  which contains
            signed char value
 * @return  int8_t  returns converted signed char value
 */

int8_t DEngProtocolParser::getInt8FromBuffer(uint8_t *buffer)
{
    int8_t value = (int8_t)(0);

    value = (int8_t)(buffer[0]);

    return (value);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   float -- float value
 * @param   uint8_t *buffer -- pointer to char  to return float value in byte format
 * @return  void
 */
void DEngProtocolParser::getBufferFromValue(float value, uint8_t *buffer)
{
    uFloat_t uFloatVal;

    uFloatVal.floatValue = 0.0f;
    uFloatVal.floatValue = value;
    buffer[0] = uFloatVal.byteValue[0];
    buffer[1] = uFloatVal.byteValue[1];
    buffer[2] = uFloatVal.byteValue[2];
    buffer[3] = uFloatVal.byteValue[3];
}

/**
 * @brief   This function converts a long integer (unsigned)
 *          to a 4 byte buffer
 * @param   uint32_t -- unsigned int value
 * @param   uint8_t *buffer -- pointer to unsigned char  to return
            unsigned int value in byte format
 * @return  void
 */
void DEngProtocolParser::getBufferFromValue(uint32_t value, uint8_t *buffer)
{
    uUint32_t uLongVal;

    uLongVal.uint32Value = 0u;
    uLongVal.uint32Value = value;
    buffer[0] = uLongVal.byteValue[0];
    buffer[1] = uLongVal.byteValue[1];
    buffer[2] = uLongVal.byteValue[2];
    buffer[3] = uLongVal.byteValue[3];
}

/**
 * @brief   This function converts a unsigned short integer (unsigned )
 *          to a 4 byte buffer
 * @param   uint16_t -- unsigned short value
 * @param   uint8_t *buffer -- pointer to unsigned char  to return
            unsigned short value in byte format
 * @return  void
 */
void DEngProtocolParser::getBufferFromValue(uint16_t value, uint8_t *buffer)
{
    uUint16_t uShortVal;

    uShortVal.uint16Value = 0u;
    uShortVal.uint16Value = value;
    buffer[0] = uShortVal.byteValue[0];
    buffer[1] = uShortVal.byteValue[1];

}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   uint8_t -- unsigned har value
 * @param   uint8_t *buffer -- pointer to unsigned char  to return
             unsigned char value in byte
 * @return  void
 */
void DEngProtocolParser::getBufferFromValue(uint8_t value, uint8_t *buffer)
{
    buffer[0] = value;
}

