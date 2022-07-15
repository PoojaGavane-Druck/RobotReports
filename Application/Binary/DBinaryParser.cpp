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
* @file     DBinaryParser.cpp
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     27-Feb-2021
*
* @brief    Binary Parser for stepper motor communications protocol source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DBinaryParser.h"
#include "string.h"
#include "DCommsMotor.h"

/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define OWI_HEADER_SIZE     1u
#define CHEECK_SUM_SIZE     1u
#define HEX_ASCII_FORMAT_FLOAT_SIZE         8u
#define HEX_ASCII_FORMAT_COEFFICIENTS_SIZE  8192u
#define HEX_FORMAT_COEFFICIENTS_SIZE  4096u
#define HEX_ASCII_FORMAT_CAL_DATA_SIZE      2048u
#define HEX_FORMAT_CAL_DATA_SIZE      1024u

#define LEN_START_CONDITION 2
#define LEN_COMMAND 1
#define LEN_LENGTH_OF_MESSAGE 1
#define LEN_CRC 1
#define LEN_HEADER (LEN_START_CONDITION + LEN_COMMAND + LEN_LENGTH_OF_MESSAGE)

#define LOC_START_CONDITION 0
#define LOC_COMMAND (LOC_START_CONDITION + LEN_START_CONDITION)
#define LOC_LENGTH_OF_MESSAGE (LOC_COMMAND + LEN_COMMAND)
#define LOC_DATA (LOC_LENGTH_OF_MESSAGE + LEN_LENGTH_OF_MESSAGE)
#define HEADER_BYTE 0XFF

#define DEFAULT_COMMANDS_NUM 16u

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Functions --------------------------------------------------------------------------------------------------------*/
/**
* @brief    Constructor
*
* @param    creator is owner of this instance
* @param    osErr is pointer to OS error
*
* @return   void
*/
DBinaryParser::DBinaryParser(void *creator, sCommand_t *commandArray, size_t maxCommands)
{
    //initialise the command set
    myParent = creator;
    commands = commandArray;
    capacity = maxCommands;
    numCommands = (size_t)(0);
    resetCrcTable();
    generateTableCrc8((uint8_t)(CRC_POLYNOMIAL_8BIT));
}

/**
* @brief    Destructor
* @param    void
* @return   void
*/
DBinaryParser::~DBinaryParser()
{

    commands = NULL;
    numCommands = (size_t)0;
    capacity = (size_t)0;
}

/**
 * @brief   This resets a CRC8 table
 * @param   void
 * @return  void
 */
void DBinaryParser::resetCrcTable(void)
{
    uint32_t index = 0u;

    for(index = 0u; index < 256u; index++)
    {
        tableCrc8[index] = (uint8_t)(0);
    }
}

#pragma diag_suppress=Pm031
/**
 * @brief   This generates a CRC8 table
 * @param   void
 * @return  void
 */
void DBinaryParser::generateTableCrc8(uint8_t polynomial)
{
    uint8_t crc = 0x80u;
    uint32_t i = 0u;
    uint32_t j = 0u;

    for(i = 1u; i < 256u; i <<= 1u)
    {
        if(crc & (uint8_t)(0x80))
        {
            crc = (crc << (uint8_t)(1)) ^ polynomial;
        }

        else
        {
            crc <<= (uint8_t)(1);
        }

        for(j = 0u; j < i; j++)
        {
            tableCrc8[(uint8_t)(i + j)] = crc ^ tableCrc8[j];
        }
    }
}
#pragma diag_default=Pm031
/**
 * @brief   Prepare message in specified transmit buffer
 * @param   str - is the character string to transmit
 * @param   buffer - is the buffer in which the string is prepared
 * @param   bufferSize is the size of the buffer
 * @return  true if completed successfully, else false
 */
bool DBinaryParser::prepareTxMessage(uint8_t cmd,
                                     uint8_t *cmdData,
                                     uint8_t cmdDataSize,
                                     uint8_t *txBuffer,
                                     uint16_t txBufferLen)
{
    bool retStatus = false;
    uint8_t index = 0u;
    uint8_t crc = 0u;

    if((txBuffer != NULL) && (txBufferLen != (uint16_t)0))
    {
        // Add header
        txBuffer[index++] = (uint8_t)HEADER_BYTE;
        txBuffer[index++] = (uint8_t)HEADER_BYTE;
        // Add command
        txBuffer[index++] = cmd;
        // Add length of data in buffer
        txBuffer[index++] = cmdDataSize;

        // Add tx data into buffer
        if(NULL != cmdData)
        {
            for(uint8_t count = 0u; count < cmdDataSize; count++)
            {
                txBuffer[index++] = cmdData[count];
            }
        }

        // Add error dummy data into buffer
        txBuffer[index++] = 0u;
        txBuffer[index++] = 0u;

        calculateCrc(txBuffer, index, &crc);
        txBuffer[index++] = crc;
        //txBufferLen = index;
        retStatus = true;
    }

    else
    {
        retStatus = false;
    }

    return retStatus;
}
/**
 * @brief   Prepare message in specified transmit buffer - Used for SPI firmware upgrade for secondary uC
 * @param   str - is the character string to transmit
 * @param   buffer - is the buffer in which the string is prepared
 * @param   bufferSize is the size of the buffer
 * @return  true if completed successfully, else false
 */
bool DBinaryParser::prepareTxMessage(uint8_t cmd,
                                     uint8_t *cmdData,
                                     uint8_t cmdDataSize,
                                     uint8_t *txBuffer,
                                     uint16_t *txBufferLen)
{
    bool retStatus = false;
    uint8_t index = 0u;
    uint8_t crc = 0u;

    if((txBuffer != NULL) && (cmdDataSize != (uint16_t)0))
    {
        txBuffer[index++] = (uint8_t)HEADER_BYTE;
        txBuffer[index++] = (uint8_t)HEADER_BYTE;

        txBuffer[index++] = cmd;

        txBuffer[index++] = cmdDataSize;

        if(NULL != cmdData)
        {
            for(uint8_t count = 0u; count < cmdDataSize; count++)
            {
                txBuffer[index++] = cmdData[count];
            }
        }

        calculateCrc(txBuffer, index, &crc);
        txBuffer[index++] = crc;
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
void DBinaryParser::addCommand(uint8_t command,
                               eDataType_t dataType,
                               fnPtrParam fnParam,
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
    sCommand_t *cmdSet = &commands[numCommands];

    cmdSet->command = command;
    cmdSet->fnParam = fnParam;
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
sError_t DBinaryParser::parse(uint8_t *ptrBuffer,
                              uint32_t msgSize,
                              uint32_t *errorCode,
                              uint32_t enggProtoCommand,
                              uint8_t *rxData)
{
    sError_t error;
    uint32_t index = 0u;
    sCommand_t *commandSet = NULL;
    eDataType_t dataType = eDataTypeNone;
    bool statusFlag = false;
    uint8_t expectedMsgLength = 0u;
    sParameter_t param;
    error.value = 1u;

    /* Scan through the commands to find the command */
    if(0u == enggProtoCommand)
    {
        for(index = 0u; index < numCommands; index++)
        {
            commandSet = &commands[index];

            if(ptrBuffer[LOC_COMMAND] == commandSet->command)
            {
                dataType = commandSet->dataType;
                expectedMsgLength = (uint8_t)((uint8_t)commandSet->responseDataLength + (uint8_t)LEN_HEADER + (uint8_t)LEN_CRC);
                error.value = 0u;
                break;
            }
        }
    }

    else
    {
        error.value = 0u;
    }

    /* Check the start byte for the message */
    if(0u == error.value)
    {
        statusFlag = validateStartCondition(&ptrBuffer[LOC_START_CONDITION]);
    }

    if(false == statusFlag)
    {
        error.needsStart = 1u;
    }


    /* Validate response CRC */
    if(0u == error.value)
    {
        statusFlag = validateCrc(ptrBuffer, (uint16_t)msgSize);
    }

    /* Validate length of the message */
    if(true == statusFlag)
    {
        if(msgSize < (uint32_t)(LEN_HEADER + LEN_CRC))
        {
            error.messageTooSmall = 1u;
            statusFlag = false;
        }

    }

    else
    {
        error.invalidChecksum = 1u;
    }

    if(true == statusFlag)
    {
        if(0u == enggProtoCommand)
        {
            getValueFromBuffer((uint8_t *)&ptrBuffer[LOC_DATA], dataType, (sParameter_t *)&param);

            if(expectedMsgLength == msgSize)
            {
                commandSet->fnParam(myParent, &param);
                error.value = 0u;
                *errorCode = 0u;
            }

            else
            {
                error.messageTooSmall = 1u;
                statusFlag = false;
            }
        }

        else
        {
            /* only forward message */
            rxData[0] = ptrBuffer[4];
            rxData[1] = ptrBuffer[5];
            rxData[2] = ptrBuffer[6];
            rxData[3] = ptrBuffer[7];
        }

    }

    return error;
}



/**
 * @brief   This function computes the crc for a given data set
 * @param   void
 * @return  void
 */
uint32_t DBinaryParser::calculateCrc(uint8_t *data, uint8_t length, uint8_t *crc)
{
    uint8_t temp = 0u;
    uint32_t index = 0u;
    uint32_t status = 0u;

    if(tableCrc8 != NULL)
    {
        temp = *crc;

        /* Check the data length to not be larger than 256 */
        if(length > 254u)
        {
            status = 0u;
        }

        else
        {
            for(index = 0u; index < length; index++)
            {
                temp = temp ^ data[index];
                temp = tableCrc8[temp ^ tableCrc8[index]];
            }

            *crc = temp;

            status = 1u;
        }
    }

    return status;
}

/**
 * @brief   This function gets the value data from buffer
 * @param   void
 * @return  void
 */
bool DBinaryParser::validateCrc(uint8_t *data, uint16_t length)
{
    /* for now calculate checksum until CRC table is available */
    uint8_t crc = 0u;
    bool status = false;

    calculateCrc(data, (uint8_t)(length) - 1u, &crc);

    if(data[10] == crc)
    {
        status = true;
    }

    return status;
}

/**
 * @brief   This function gets the value data from buffer
 * @param   void
 * @return  void
 */
bool DBinaryParser::validateStartCondition(uint8_t *data)
{
    uint16_t startCondition = 0u;
    bool statusFlag = false;

    startCondition = data[0];
    startCondition = (uint16_t)(startCondition  << 8u) |  data[(uint8_t)1];

    if(startCondition == (uint16_t)(STEPPER_START_CONDITION))
    {
        statusFlag = true;
    }

    else
    {
        statusFlag = false;
    }

    return statusFlag;
}

/**
 * @brief   This function gets the value data from buffer
 * @param   void
 * @return  void
 */
bool DBinaryParser::getValueFromBuffer(uint8_t *buffer, eDataType_t dataType, sParameter_t *ptrParam)
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
        getUint32FromBuffer(buffer, &ptrParam->uiValue);
        break;

    case eDataTypeSignedLong:
        getInt32FromBuffer(buffer, &ptrParam->iValue);
        break;

    case eDataTypeFloat:
        getFloatFromBuffer(buffer, &ptrParam->floatValue);
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
sError_t DBinaryParser::getFloatFromBuffer(uint8_t *buffer, float *value)
{
    sError_t error;
    uFloat_t uFloatVal;

    error.value = 0u;

    uFloatVal.floatValue = 0.0f;

    uFloatVal.byteValue[0] = (uint8_t)(buffer[0]);
    uFloatVal.byteValue[1] = (uint8_t)(buffer[1]);
    uFloatVal.byteValue[2] = (uint8_t)(buffer[2]);
    uFloatVal.byteValue[3] = (uint8_t)(buffer[3]);

    *value = uFloatVal.floatValue;

    return error;
}

/**
 * @brief   This function converts a 4 byte buffer to a unsigned long integer
 * @param   void
 * @return  void
 */

sError_t DBinaryParser::getUint32FromBuffer(uint8_t *buffer, uint32_t *value)
{
    sError_t error;
    uUint32_t uValue;

    error.value = 0u;
    uValue.uint32Value = 0u;

    uValue.byteValue[0] = (uint8_t)(buffer[0]);
    uValue.byteValue[1] = (uint8_t)(buffer[1]);
    uValue.byteValue[2] = (uint8_t)(buffer[2]);
    uValue.byteValue[3] = (uint8_t)(buffer[3]);

    *value = uValue.uint32Value;

    return error;
}

/**
 * @brief   This function converts a 4 byte buffer to a unsigned long integer
 * @param   void
 * @return  void
 */

sError_t DBinaryParser::getInt32FromBuffer(uint8_t *buffer, int32_t *value)
{
    sError_t error;
    uSint32_t uIntVal;

    error.value = 0u;
    uIntVal.int32Value = 0;

    uIntVal.byteValue[0] = (uint8_t)(buffer[0]);
    uIntVal.byteValue[1] = (uint8_t)(buffer[1]);
    uIntVal.byteValue[2] = (uint8_t)(buffer[2]);
    uIntVal.byteValue[3] = (uint8_t)(buffer[3]);

    *value = uIntVal.int32Value;

    return error;
}
/**
 * @brief   This function converts a 4 byte buffer to a unsigned short
 * @param   void
 * @return  void
 */

sError_t DBinaryParser::getUint16FromBuffer(uint8_t *buffer, uint16_t *value)
{
    sError_t error;
    uUint16_t uShortVal;

    error.value = 0u;
    uShortVal.uint16Value = 0u;

    uShortVal.byteValue[0] = (uint8_t)(buffer[0]);
    uShortVal.byteValue[1] = (uint8_t)(buffer[1]);


    *value = uShortVal.uint16Value;

    return error;
}



/**
 * @brief   This function converts a 1 byte buffer to a signed short
 * @param   void
 * @return  void
 */

sError_t DBinaryParser::getUint8FromBuffer(uint8_t *buffer, uint8_t *value)
{
    sError_t error;
    uint8_t val = 0u;

    error.value = 0u;
    val = val | (uint8_t)(buffer[0]);

    *value = val;

    return error;
}

/**
 * @brief   This function converts a 1 byte buffer to a signed short
 * @param   void
 * @return  void
 */

sError_t DBinaryParser::getInt8FromBuffer(uint8_t *buffer, int8_t *value)
{
    sError_t error;
    int8_t val = (int8_t)(0);

    error.value = 0u;
    val = (int8_t)(buffer[0]);

    *value = val;

    return error;
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
sError_t DBinaryParser::getBufferFromValue(float *value, uint8_t *buffer)
{
    sError_t error;
    error.value = 0u;

    getBufferFromFloat(value, buffer);

    return error;
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
sError_t DBinaryParser::getBufferFromValue(uint32_t *value, uint8_t *buffer)
{
    sError_t error;
    error.value = 0u;

    getBufferFromLong(value, buffer);

    return error;
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
sError_t DBinaryParser::getBufferFromValue(uint16_t *value, uint8_t *buffer)
{
    sError_t error;
    error.value = 0u;

    getBufferFromShort(value, buffer);

    return error;
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
sError_t DBinaryParser::getBufferFromValue(uint8_t *value, uint8_t *buffer)
{
    sError_t error;
    error.value = 0u;

    getBufferFromChar(value, buffer);

    return error;
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DBinaryParser::getBufferFromLong(uint32_t *value, uint8_t *buffer)
{
    uUint32_t uLongVal;

    uLongVal.uint32Value = 0u;
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
void DBinaryParser::getBufferFromFloat(float *value, uint8_t *buffer)
{
    uFloat_t uFloatVal;

    uFloatVal.floatValue = 0.0f;
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
void DBinaryParser::getBufferFromShort(uint16_t *value, uint8_t *buffer)
{
    uUint16_t uShortVal;

    uShortVal.uint16Value = 0u;
    uShortVal.uint16Value = *value;
    buffer[0] = uShortVal.byteValue[0];
    buffer[1] = uShortVal.byteValue[1];
}

/**
 * @brief   Reads a single byte of data into a data buffer to be transmitter
 * @param   uint8_t *value - to read the character
 * @param   uint8_t *buffer - contains data to be transmitted
 * @return  void
 */
void DBinaryParser::getBufferFromChar(uint8_t *value, uint8_t *buffer)
{
    buffer[0] = *value;
}

/**
 * @brief   Gets the expected number of bytes from a command
 * @param   cmd - command
 * @param   *expectedLength - pointer to the number of expected bytes
 * @return  true - if command was found, false if not found
 */
bool DBinaryParser::getResponseLength(uint8_t cmd,
                                      uint32_t *expectedResponseLen)
{
    bool successFlag = false;

    uint32_t index = 0u;
    sCommand_t *element;

    for(index = 0u; index < numCommands; index++)
    {
        element = &commands[index];

        if(cmd == element->command)
        {
            *expectedResponseLen = element->responseDataLength +
                                   (uint32_t)LEN_HEADER + (uint32_t)LEN_CRC;

            successFlag = true;
            break;
        }
    }

    return successFlag;
}