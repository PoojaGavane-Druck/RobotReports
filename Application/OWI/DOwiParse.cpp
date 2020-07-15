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

/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define OWI_HEADER_SIZE     1u
#define CHEECK_SUM_SIZE     1u
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
                           eOwiDataFormat_t cmdDataFormat,
                           eOwiDataFormat_t responseDataFormat,
                           fnPtrOwi processCmdFunction,
                           uint32_t commandDataLength,
                           uint32_t responseDataLength,
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

    element->processCmdFunction = processCmdFunction;   
    element->commandDataLength = commandDataLength;
    element->responseDataLenght = responseDataLength;
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
sOwiError_t DOwiParse::parse(uint8_t *str, uint32_t msgSize)
{
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t *pData = str;

    // Calculate Check Sum
    // Process Command

    return owiError;
}

sOwiError_t DOwiParse::processCommand(uint32_t cmdIndex, uint8_t *str)
{
    sOwiError_t owiError;
    owiError.value = 0u;

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
                                            uint32_t &CommandLen)
{
    bool successFlag = true;
    
    if (getChecksumEnabled() == true)
    {
      uint8_t checkSum = 0u;
      uint32_t index;
      
      CommandLen = cmdDataBufferSize + CHEECK_SUM_SIZE;
      
      for(index = 0u; index < cmdDataBufferSize; index++)
      {
        checkSum = checkSum + cmdDataBuffer[index];
      }
      cmdDataBuffer [cmdDataBufferSize] = checkSum;
    }
    else
    {
      CommandLen = cmdDataBufferSize;
    }
    
    return successFlag;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
 bool DOwiParse::getResponseDataLength(uint8_t cmd, uint32_t &expectedResponseLen)
 {
   bool successFlag = false;
   
   uint32_t index = 0u;
   sOwiCommand_t *element;
   for(index = 0u; index < numCommands; index++)
   {
     element = &commands[index];
     if(cmd == element->command)
     {
       expectedResponseLen = element->responseDataLenght;
       successFlag = true;
     }
   }
   return successFlag;
 }
 
/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
 bool DOwiParse::asciiHexToData(uint8_t* pAsciiString, uint8_t* pBinaryBuffer, 
                                uint_t32 iNumberOfBinaryBytes) 
{ 
	// A helper function for ASCII Hex to Data
	uint8_t* pSrc = pAsciiString;
	uint8_t*  pDst = pBinaryBuffer;
	uint8_t hi;
	uint8_t lo;

	for (uint32_t index = 0; index < iNumberOfBinaryBytes; ++index)
	{
		hi = *pSrc++; 
		hi -= (hi < 'A' ? '0' : 'A' - 10);

		lo = *pSrc++; 
		lo -= (lo < 'A' ? '0' : 'A' - 10);

		*pDst++ = (hi << 4) | (lo & 0x0F); // " & 0x0F" deals with lower-case characters
	}
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm136")
 bool DOwiParse::dataToAsciiHex(uint8_t* pAsciiString, uint8_t* pBinaryBuffer, 
                                uint32_t iNumberOfBinaryBytes) 
{
	// A helper function for Data to ASCII Hex
	uint8_t* pSrc = pBinaryBuffer;
	uint8_t* pDst = pAsciiString;
	uint8_t hi;
	uint8_t lo;

	for (uint32_t index = 0; index < iNumberOfBinaryBytes; index++)
	{
		lo = *pSrc++;
		hi = lo >> 4;
		lo &= 0x0F;

		*pDst++ = hi + (hi > 9 ? 'A' - 10 : '0');
		*pDst++ = lo + (lo > 9 ? 'A' - 10 : '0');
	}
}