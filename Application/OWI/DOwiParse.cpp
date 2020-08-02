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
* @file     DOwiParse.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The DUCI parser base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DOwiParse.h"
#include "string.h"
/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define OWI_HEADER_SIZE     1u
#define CHEECK_SUM_SIZE     1u
#define HEX_ASCII_FORMAT_FLOAT_SIZE         8u
#define HEX_ASCII_FORMAT_COEFFICIENTS_SIZE  8192u
#define HEX_FORMAT_COEFFICIENTS_SIZE  4096u
#define HEX_ASCII_FORMAT_CAL_DATA_SIZE      2048u
#define HEX_FORMAT_CAL_DATA_SIZE      1024u
const size_t defaultSize = 8u;

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Functions *********************************************************************************************************/
/**********************************************************************************************************************
 * DISABLE MISRA C 2004 Error[Pm100]: dynamic heap memory allocation shall not be used (MISRA C 2004 rule 20.4)
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm100")

/**
* @brief	Constructor
*
* @param    creator is owner of this instance
* @param    osErr is pointer to OS error
*
* @return   void
*/
DOwiParse::DOwiParse(void *creator, OS_ERR *osErr)
{
    myParent = creator;

    //initialise the command set
    commands = (sOwiCommand_t *)malloc(defaultSize * sizeof(sOwiCommand_t));
    numCommands = (size_t)0;
    capacity = defaultSize;

    //no echo by default    
    checksumEnabled = false;        //by default, do not use checksum
}

/**
* @brief	Destructor
* @param    void
* @return   void
*/
DOwiParse::~DOwiParse()
{
  free(commands);
  commands = NULL;
  numCommands = (size_t)0;
  capacity = (size_t)0;
}

/**
* @brief	Set Checksum Enabled
* @param    flag - true is checksum is used in message, else false
* @return   void
*/
void DOwiParse::setChecksumEnabled(bool flag)
{
    checksumEnabled = flag;
}

/**
* @brief	Get Checksum Enabled
* @param    void
* @return   flag - true is checksum is used in message, else false
*/
bool DOwiParse::getChecksumEnabled(void)
{
    return checksumEnabled;
}

/**
 * @brief   Add command to array
 *
 * @note    Each command may have up to DUCI_MESSAGE_MAX_PARAMETERS parameters only.
 *          Format specifiers for setFormat & getFormat:
 *
 *          =		assignment (set)
 *          ?		query (get)
 *          <n>i	'n' digits (eg, "2i" expects exactly 2 digits)
 *          <n>x	32-bit integer value: 'n' hex digits (eg, "4x" expects exactly 4 hex digits)
 *          <n>X	64-bit integer value: 'n' hex digits (eg, "8x" expects exactly 8 hex digits) - may be preceded by "0x"
 *          b		boolean (1 = true, 0 = false)
 *          c		ASCII character
 *          i		integer value
 *          v		floating point (with or without decimal places)
 *          d		date (always dd/mm/yyyy)
 *          t       time (always hh:mm:ss)
 *          [		inidicates that following integer is optional - must have closing ']' after the specifier
 *          ]		inidicates that preceding interger is optional - must have opening '[' before the specifier
 *          s		ASCII string
 *          $		whole string as parameter (for custom handling)
 *
 * @note    Only integer parameter type is allowed as optional
 *
 * @note    When specifying custom specifier, the setFunction and getFunction must be the same DIY function. The first
 *          one to be checked for will be the one called; safer to make both the same in case the order is ever changed.
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
void DOwiParse::addCommand(uint8_t cmd,
                           eOwiArgType_t argType,
                           eOwiDataFormat_t cmdDataFormat,
                           eOwiDataFormat_t responseDataFormat,
                           fnPtrOwiParam fnOwiParam,
                           fnPtrChar fnCharParam,
                           uint32_t commandDataLength,
                           uint32_t responseDataLength,
                           bool responseCheckSumStatus,
                           uint32_t permissions)
{
     //  if array full, increase the array capacity (by arbitrarily adding another block of commands)
    if (numCommands >= capacity)
    {
        capacity += defaultSize;
        commands = (sOwiCommand_t *)realloc(commands, capacity * sizeof(sOwiCommand_t));
    }

    //add new command at current index
    sOwiCommand_t *element = &commands[numCommands];

    element->command = cmd;

    element->cmdDataFormat = cmdDataFormat;
    element->responseDataFormat = responseDataFormat;

    element->fnOwiParam = fnOwiParam;   
    element->fnCharParam = fnCharParam;   
    element->commandDataLength = commandDataLength;
    element->responseDataLenght = responseDataLength;
    element->checksumAvailableStatusInResponse = responseCheckSumStatus;
    element->permissions = permissions;

    //increment no of commands in array
    numCommands++;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 Error[Pm100]: dynamic heap memory allocation shall not be used (MISRA C 2004 rule 20.4)
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm100")

/**
 * @brief   Parse and process message string
 * @param   pointer to null-terminated string to transmit
 * @return  duciError is the error status
 */
sOwiError_t DOwiParse::parse(uint8_t cmd, uint8_t *str, uint32_t msgSize )
{
    sOwiError_t owiError;
    owiError.value = 0u;
    bool statusFlag = false;
    sOwiCommand_t *element;
    eOwiArgType_t argType = owiArgInvalid;
    owiError.unknownCommand  =  1u;
    for(uint8_t index = 0u; index < numCommands; index++)
    {
      element = &commands[index];
      if(cmd == element->command)
      {
        argType = element->argType;  
        owiError.value = 0u;        
      }
      
    }
    
    //Step1: Calcualte checksum and verify it with the checksum inside the msg

    if(0u == owiError.value)
    {
        if(true ==element->checksumAvailableStatusInResponse)
        {
          statusFlag = ValidateCheckSum( str, msgSize);
        }
        else
        {
          statusFlag = true;
        }
    }
       
    
    if(false == statusFlag)
    {
      owiError.invalid_response = 1u;
    }
   
    //Step2: Parse the received data
    if(0u == owiError.value)
    {
       if((eOwiArgType_t)owiArgAmcSensorCoefficientsInfo == argType ) //Coefficient information
       {
         uint8_t coeffbuffer[HEX_FORMAT_COEFFICIENTS_SIZE];
         statusFlag = getCoefficientsArg(coeffbuffer, str,  msgSize);
           // Step3 : Process the command
          if(true == statusFlag)
          {
            owiError = element->fnCharParam(myParent,
                                                  (uint8_t*)coeffbuffer, (uint8_t*)&msgSize);
          }
       }
       else if((eOwiArgType_t)owiArgAmcSensorCalibrationInfo == argType) //Calibration information
       {
         uint8_t calbuffer[HEX_FORMAT_CAL_DATA_SIZE];
         statusFlag =  getCalibrationDataArg(calbuffer, str,  msgSize);
          if(true == statusFlag)
          {
            owiError = element->fnCharParam(myParent,
                                                  calbuffer,(uint8_t*)&msgSize);
          }
       }
       else
       {
          sOwiParameter_t owiParam;
          switch(argType)
          {            
            case owiArgString:
              strcpy((char*)&owiParam.byteArray[0],(char const*)str);
            break;
           
            case owiArgRawAdcCounts:
             statusFlag =  getRawCountsArg(&owiParam.rawAdcCounts,str,msgSize);
            break;
            
            case owiArgValue:
             statusFlag =   getValueArg(&owiParam.floatValue,
                                        str,
                                        element->responseDataFormat, 
                                        msgSize);
            break;
            
            default:
              statusFlag = false;
            break;
          }
          // Step3 : Process the command
          if(true == statusFlag)
          {
            owiError = element->fnOwiParam(myParent,
                                                  &owiParam);
          }
       }
     
    }    
    return owiError;
}



sOwiError_t DOwiParse::slaveParse(uint8_t cmd, uint8_t *str, uint32_t *msgSize )
{
   sOwiError_t owiError;
   owiError.value = 0u;
   bool statusFlag = false;
   sOwiCommand_t *element = NULL;
   statusFlag = getHandleToCommandProperties(cmd,element);
   if(true == statusFlag)
   {
     element->fnCharParam(myParent, str,(uint8_t*)msgSize);
   }
   return owiError;
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 10.3. Ignoring this - explicit conversion from 'signed int' to 'char' is safe
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm136")

/**
 * @brief   Prepare message in specified transmit buffer
 * @param   str - is the character string to transmit
 * @param   buffer - is the buffer in which the string is prepared
 * @param   bufferSize is the size of the buffer
 * @return  true if completed successfully, else false
 */
bool DOwiParse::CalculateAndAppendCheckSum( uint8_t *cmdDataBuffer, 
                                            uint32_t cmdDataBufferSize,
                                            uint32_t *CommandLen)
{
    bool successFlag = true;
    
    if (getChecksumEnabled() == true)
    {
      uint8_t checkSum = 0u;
      uint32_t index;
      
      *CommandLen = cmdDataBufferSize + CHEECK_SUM_SIZE;
      
      for(index = 0u; index < cmdDataBufferSize; index++)
      {
        checkSum = checkSum + cmdDataBuffer[index];
      }
      cmdDataBuffer [cmdDataBufferSize] = checkSum;
    }
    else
    {
      *CommandLen = cmdDataBufferSize;
    }
    
    return successFlag;
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 10.3. Ignoring this - explicit conversion from 'signed int' to 'char' is safe
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm136")

/**
 * @brief   Prepare message in specified transmit buffer
 * @param   str - is the character string to transmit
 * @param   buffer - is the buffer in which the string is prepared
 * @param   bufferSize is the size of the buffer
 * @return  true if completed successfully, else false
 */
bool DOwiParse::ValidateCheckSum( 
                                  uint8_t *srcBuffer, 
                                  uint32_t srcBufferSize
                                )
{
    bool successFlag = true;
        
    if (true == getChecksumEnabled())
    {
        uint8_t checkSum = 0u;
        
        for(uint32_t index = 0u; index < srcBufferSize - 1u; index++)
        {
           checkSum = checkSum + srcBuffer[index];
        }
        
        if(checkSum != srcBuffer[srcBufferSize  - 1u])
        {
          successFlag = false;
        }
    }     
    return successFlag;
}



bool DOwiParse::parseAcknowledgement(uint8_t cmd, uint8_t* ptrBuffer)
{
  bool successFlag = false;
  if((cmd == ptrBuffer[0]) && ( E_OWI_RESPONSE_ACC == ptrBuffer[1]))
  {
    successFlag = ValidateCheckSum(ptrBuffer, 3u);
  }
  return successFlag;
}
/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
 bool DOwiParse::getResponseLength(uint8_t cmd, uint32_t *expectedResponseLen)
 {
   bool successFlag = false;
   
   uint32_t index = 0u;
   sOwiCommand_t *element;
   for(index = 0u; index < numCommands; index++)
   {
     element = &commands[index];
     if(cmd == element->command)
     {
          if(( true == getChecksumEnabled() ) && 
             (true == element->checksumAvailableStatusInResponse)
             )
          {
              *expectedResponseLen = element->responseDataLenght + 1u;             
          }
          else
          {
              *expectedResponseLen = element->responseDataLenght;
          }       
          successFlag = true;
          break;
     }
   }
   return successFlag;
 }
 
/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
 bool DOwiParse::asciiHexToData(uint8_t* pBinaryBuffer, uint8_t* pAsciiString, 
                                uint32_t iNumberOfBinaryBytes) 
{ 
	// A helper function for ASCII Hex to Data
    bool retStatus = false;
    uint8_t* pSrc = pAsciiString;
    uint8_t*  pDst = pBinaryBuffer;
    uint8_t hi;
    uint8_t lo;
    if((pBinaryBuffer != NULL) && (pAsciiString != NULL))
    {
      retStatus = true;
      for (uint32_t index = 0u; index < iNumberOfBinaryBytes; ++index)
      {
          hi = *pSrc++; 
          hi -= (hi < 'A' ? '0' : 'A' - 10u);

          lo = *pSrc++; 
          lo -= (lo < 'A' ? '0' : 'A' - 10u);

          *pDst++ = (hi << 4u) | (lo & 0x0Fu); // " & 0x0F" deals with lower-case characters
      }
    }
    else
    {
      retStatus = false;
    }
    return retStatus;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
 bool DOwiParse::dataToAsciiHex(uint8_t* pAsciiString, uint8_t* pBinaryBuffer, 
                                uint32_t iNumberOfBinaryBytes) 
{
	// A helper function for Data to ASCII Hex
    bool retStatus = false;
    uint8_t* pSrc = pBinaryBuffer;
    uint8_t* pDst = pAsciiString;
    uint8_t hi;
    uint8_t lo;
    if((pBinaryBuffer != NULL) && (pAsciiString != NULL))
    {
      retStatus = true;
      for (uint32_t index = 0u; index < iNumberOfBinaryBytes; index++)
      {
              lo = *pSrc++;
              hi = lo >> 4u;
              lo &= 0x0Fu;

              *pDst++ = hi + (hi > 9u ? 'A' - 10u : '0');
              *pDst++ = lo + (lo > 9u ? 'A' - 10u : '0');
      }
    }
    else
    {
      retStatus = false;
    }
    return retStatus;
}


/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
 bool DOwiParse::getRawCountsArg(sRawAdcCounts* prtRawAdcCounts,
                                 uint8_t* pSrcBuffer,
                                 uint32_t srcBufferSize) 
{
	
      bool retStatus = false;
      uint32_t rawCounts = 0u;
      if(srcBufferSize >= 8u)
      {
        if(pSrcBuffer[0] & (0XC0u | E_AMC_SENSOR_BRIDGE_COUNTS_CHANNEL))
        {
          rawCounts =(((pSrcBuffer[0] & (uint32_t)0x0F) << (uint32_t)21) | ((pSrcBuffer[1] & (uint32_t)0x7f) << (uint32_t)14)  | ((pSrcBuffer[2] & (uint32_t)0x7f) << (uint32_t)7)  | pSrcBuffer[3] & (uint32_t)0x7f);

          prtRawAdcCounts->channel1AdcCounts = rawCounts - 0x1000000u;
          
          if(pSrcBuffer[4] & (0XC0u | E_AMC_SENSOR_TEMPERATURE_CHANNEL))
          {
            rawCounts =(((pSrcBuffer[4] & (uint32_t)0x0F) << (uint32_t)21) | ((pSrcBuffer[5] & (uint32_t)0x7f) << (uint32_t)14)  | ((pSrcBuffer[6] & (uint32_t)0x7f) << (uint32_t)7)  | pSrcBuffer[7] & (uint32_t)0x7f);
            
            rawCounts = rawCounts - 0x1000000u;  
            
            prtRawAdcCounts->channel2AdcCounts = rawCounts - 0x1000000u;
            
            retStatus = true;
          }          
        }
      }
      return retStatus;
}


/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
bool DOwiParse::getValueArg(float* pfValue, 
                               uint8_t* pSrcBuffer, 
                               eOwiDataFormat_t srcDataFormat, 
                               uint32_t msgSize
                               )
{
    bool retStatus = false;
    
    uFloat_t uFloatValue;
    
    if(E_OWI_HEX_ASCII == (uint8_t)srcDataFormat)
    {
      if(msgSize >= HEX_ASCII_FORMAT_FLOAT_SIZE)
      {
        retStatus = asciiHexToData(&uFloatValue.byteValue[0], pSrcBuffer, 4u);
        if(true == retStatus)
        {
          *pfValue = uFloatValue.floatValue;
        }
      }
    }
    if(E_OWI_BYTE == (uint8_t)srcDataFormat)
    {
      if(msgSize >= sizeof(float))
      {
        uFloatValue.byteValue[0] = pSrcBuffer[0];
        uFloatValue.byteValue[1] = pSrcBuffer[1];
        uFloatValue.byteValue[2] = pSrcBuffer[2];
        uFloatValue.byteValue[3] = pSrcBuffer[3];
        
        retStatus = true;
      }
    }
    return retStatus;
}
/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
 bool DOwiParse::getCoefficientsArg(uint8_t* pBinaryBuffer,
                                    uint8_t* pAsciiString, 
                                    uint32_t msgSize)
 {
    bool retStatus = false;
    if(msgSize >= HEX_ASCII_FORMAT_COEFFICIENTS_SIZE)
    {      
       asciiHexToData(pBinaryBuffer, pAsciiString,  HEX_ASCII_FORMAT_COEFFICIENTS_SIZE/2u);
       retStatus = true;
    }
    return retStatus;    
 }
/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
 bool DOwiParse::getCalibrationDataArg(uint8_t* pBinaryBuffer,
                                    uint8_t* pAsciiString, 
                                    uint32_t msgSize)
 {
    bool retStatus = false;
    if(msgSize >= HEX_ASCII_FORMAT_CAL_DATA_SIZE)
    {      
       asciiHexToData(pBinaryBuffer, pAsciiString,  HEX_ASCII_FORMAT_CAL_DATA_SIZE/2u);
       retStatus = true;
    }
    return retStatus;    
 }
 
 
 
 
  bool DOwiParse::getHandleToCommandProperties(uint8_t cmd, sOwiCommand_t *ptrToCmd )
  {
    bool retStatus = false;
    for(uint8_t index = 0u; index < numCommands; index++)
    {        
        if(cmd == (commands[index].command))
        {
          ptrToCmd = &commands[index];
          retStatus = true;
          break;
        }
    }
    return retStatus;
  }