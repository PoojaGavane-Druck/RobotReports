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
#include "DBinaryParser.h"
#include "string.h"

/* Constants & Defines ------------------------------------------------------*/
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

/* Variables ----------------------------------------------------------------*/

/* Functions *****************************************************************/

/**
* @brief	Constructor
*
* @param    creator is owner of this instance
* @param    osErr is pointer to OS error
*
* @return   void
*/
DBinaryParser::DBinaryParser(void *creator,sCommand_t *commandArray, size_t maxCommands, OS_ERR *os_error)
{   
    //initialise the command set  
    myParent = creator;
    commands = commandArray;
    capacity = maxCommands;
    numCommands = (size_t)(0);
   
   
}

/**
* @brief	Destructor
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
 * @brief   Prepare message in specified transmit buffer
 * @param   str - is the character string to transmit
 * @param   buffer - is the buffer in which the string is prepared
 * @param   bufferSize is the size of the buffer
 * @return  true if completed successfully, else false
 */
bool DBinaryParser::prepareTxMessage(uint8_t cmd, 
                                     uint8_t* cmdData, 
                                     uint8_t cmdDataSize, 
                                     uint8_t *txBuffer, 
                                     uint16_t *txBufferLen)
{
    bool retStatus = false;
    uint8_t index = (uint8_t)0;
    uint8_t checkSum = (uint8_t)0;

    if( (txBuffer != NULL) && (txBufferLen != NULL))
    {
        txBuffer[index++] = (uint8_t)HEADER_BYTE;
        txBuffer[index++] = (uint8_t)HEADER_BYTE;

        txBuffer[index++] = cmd;

        txBuffer[index++] = cmdDataSize;
        if (NULL != cmdData)
        {
            for (uint8_t count = (uint8_t)0; count < cmdDataSize; count++)
            {
                txBuffer[index++] = cmdData[count];
            }
        }
        checkSum = CalculateCrc(txBuffer, index);
        txBuffer[index++] = checkSum;
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
                                fnPtrParam fnParam,
                                eDataType_t dataType,
                                uint32_t commandDataLength,
                                uint32_t responseDataLength)
{
     //  if array full, increase the array capacity (by arbitrarily adding another block of commands)
    if (numCommands >= capacity)
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
sError_t DBinaryParser::parse(uint8_t cmd, uint8_t* ptrBuffer, uint32_t msgSize, uint32_t* errorCode)
{
    sError_t error;
    uint32_t index = (uint32_t)(0);
    sCommand_t* commandSet = NULL;
    eDataType_t dataType = eDataTypeNone;
    bool statusFlag = false;
    uint8_t expectedMsgLength = (uint8_t)(0);
    sParameter_t param;
    error.value = 1u;
    
    /* Scan through the commands to find the command */
    for (index = (uint32_t)(0); index < numCommands; index++)
    {
        commandSet = &commands[index];

        if (cmd == commandSet->command)
        {
            dataType = commandSet->dataType;
            expectedMsgLength = (uint8_t)((uint8_t)commandSet->responseDataLength + (uint8_t)LEN_HEADER + (uint8_t)LEN_CRC);
            error.value = 0u;
            break;
        }
    }

    /* Check the start byte for the message */
    if ((uint32_t)(0) == error.value)
    {
        statusFlag = ValidateStartCondition(&ptrBuffer[LOC_START_CONDITION]);
    }
    if (false == statusFlag)
    {
        error.needsStart = 1u;
    }
    

    /* Validate response CRC */
    if ((uint32_t)(0) == error.value)
    {
       statusFlag = ValidateCrc(ptrBuffer,  (uint16_t)msgSize);
    }

    /* Validate length of the message */
    if (true == statusFlag)
    {
        if (msgSize < (uint32_t)(LEN_HEADER + LEN_CRC))
        {
            error.messageTooSmall = 1u;
            statusFlag = false;
        }

    }
    else
    {
        error.invalidChecksum = 1u;
    }
    if (true == statusFlag)
    {
        //Parse the parameters
        
        if ((uint8_t)cmd == (uint8_t)ptrBuffer[LOC_COMMAND])
        {
            GetValueFromBuffer((uint8_t*)&ptrBuffer[LOC_DATA], dataType, (sParameter_t*)&param);
            if (expectedMsgLength == msgSize)
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
            error.nackReceived = 1u;
            GetValueFromBuffer((uint8_t*)&ptrBuffer[LOC_DATA], eDataTypeUnsignedLong, (sParameter_t*)&param);
            *errorCode = param.uiValue;
        }
    }
    return error;
}



/**
 * @brief   This function gets the value data from buffer
 * @param   void
 * @return  void
 */
uint8_t DBinaryParser::CalculateCrc(uint8_t* data,  uint8_t length)
{
    /* for now calculate checksum until CRC table is available */
    uint8_t index = (uint8_t)(0);
   
    uint8_t checksum = (uint8_t)(0);

    for (index = (uint8_t)(0); index < length; index++)
    {
        checksum = checksum + (uint8_t)(data[index]);
    }

  

    return checksum;
}

/**
 * @brief   This function gets the value data from buffer
 * @param   void
 * @return  void
 */
bool DBinaryParser::ValidateCrc(uint8_t* data, uint16_t length)
{
    /* for now calculate checksum until CRC table is available */
    uint16_t index = (uint8_t)(0);
    bool status = false;
    uint8_t checksum = (uint8_t)(0);

    for (index = (uint16_t)(0); index < length - (uint16_t)1; index++)
    {
        checksum = checksum + (uint8_t)(data[index]);
    }

    if (data[index] == checksum)
    {
        status = true;
    }

    return status;
}
#if 0
/**
 * @brief   This function gets the value data from buffer
 * @param   void
 * @return  void
 */
bool DBinaryParser::ValidateCrc(uint8_t *data,  uint16_t length)
{
    /* for now calculate checksum until CRC table is available */
    uint8_t index = (uint8_t)(0);
    bool status = false;
    uint8_t checksum = (uint8_t)(0);

    for(index = (uint8_t)(0); index < length - (uint8_t)1; index++)
    {
        checksum = checksum + (uint8_t)(data[index]);
    }

    if(data[index] == checksum)
    {
        status = true;
    }

    return status;
}
#endif
/**
 * @brief   This function gets the value data from buffer
 * @param   void
 * @return  void
 */
bool DBinaryParser::ValidateStartCondition(uint8_t *data)
{
    uint16_t startCondition = (uint16_t)(0);
    bool statusFlg = false;

    startCondition = data[0];
    startCondition = (uint16_t)(startCondition  << (uint16_t)8) |  data[(uint8_t)1];

    if(startCondition == (uint16_t)(STEPPER_START_CONDITION))
    {
        statusFlg = true;
    }
    else
    {
        statusFlg = false;
    }

    return statusFlg;
}

/**
 * @brief   This function gets the value data from buffer
 * @param   void
 * @return  void
 */
bool DBinaryParser::GetValueFromBuffer(uint8_t* buffer, eDataType_t dataType, sParameter_t *ptrParam)
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
float DBinaryParser::GetFloatFromBuffer(uint8_t* buffer)
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

uint32_t DBinaryParser::GetUint32FromBuffer(uint8_t* buffer)
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

int32_t DBinaryParser::GetInt32FromBuffer(uint8_t* buffer)
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

uint16_t DBinaryParser::GetUint16FromBuffer(uint8_t* buffer)
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

uint8_t DBinaryParser::GetUint8FromBuffer(uint8_t* buffer)
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

int8_t DBinaryParser::GetInt8FromBuffer(uint8_t* buffer)
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
void DBinaryParser::GetBufferFromValue(float* value, uint8_t* buffer)
{
    GetBufferFromFloat(value, buffer);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DBinaryParser::GetBufferFromValue(uint32_t* value, uint8_t* buffer)
{
    GetBufferFromLong(value, buffer);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DBinaryParser::GetBufferFromValue(uint16_t* value, uint8_t* buffer)
{
    GetBufferFromShort(value, buffer);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DBinaryParser::GetBufferFromValue(uint8_t* value, uint8_t* buffer)
{
    GetBufferFromChar(value, buffer);
}

/**
 * @brief   This function converts a long integer (unsigned or signed)
 *          to a 4 byte buffer
 * @param   void
 * @return  void
 */
void DBinaryParser::GetBufferFromLong(uint32_t* value, uint8_t* buffer)
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
void DBinaryParser::GetBufferFromFloat(float* value, uint8_t* buffer)
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
void DBinaryParser::GetBufferFromShort(uint16_t* value, uint8_t* buffer)
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
void DBinaryParser::GetBufferFromChar(uint8_t* value, uint8_t* buffer)
{
    buffer[0] = *value;
}


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