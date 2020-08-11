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
* @file     DCommsState.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The communications class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateOwi.h"
#include "DInstrument.h"
#include "Utilities.h"
#include "uart.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define CMD_HEADER_SIZE  2u
#define OWI_CMD_MASK 0x3Fu
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/


/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsState class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateOwi::DCommsStateOwi(DDeviceSerial *commsMedium)
:DCommsState(commsMedium)
{
    
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateOwi::createCommands(void)
{
    myParser->addCommand(E_DPI620G_CMD_GET_VERSION_INFO, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,  
                         NULL,  
                         fnGetVersionInfo, 
                         E_DPI620G_CMD_LEN_GET_VERSION_INFO , 
                         E_DPI620G_RESP_LEN_GET_VERSION_INFO , 
                         false, 
                         0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_PM620_SENSOR_INFO, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,  
                         fnGetPM620SensorInfo,  
                         E_DPI620G_CMD_LEN_GET_PM620_SENSOR_INFO , 
                         E_DPI620G_RESP_LEN_GET_PM620_SENSOR_INFO , 
                         false, 
                         0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_SET_OPERATING_MODE, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL, 
                         fnSetOperatingMode,   
                         E_DPI620G_CMD_LEN_SET_OPERATING_MODE, 
                         E_DPI620G_RESP_LEN_SET_OPERATING_MODE, 
                         false, 
                         0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_OPERATING_MODE_ACCESS_LEVEL, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL, fnGetOperatingModeAndAccessLevel,    E_DPI620G_CMD_LEN_GET_OPERATING_MODE_ACCESS_LEVEL , E_DPI620G_RESP_LEN_GET_OPERATING_MODE_ACCESS_LEVEL, false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_DEVICE_STATUS, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL, fnGetDeviceStatus,   E_DPI620G_CMD_LEN_GET_DEVICE_STATUS , E_DPI620G_RESP_LEN_GET_DEVICE_STATUS  , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_DEVICE_CONFIGURATION, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE,  NULL,   fnGetDeviceConfiguration, E_DPI620G_CMD_LEN_GET_DEVICE_CONFIGURATION , E_DPI620G_RESP_LEN_GET_DEVICE_CONFIGURATION , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_BATTERY_PARAM_INFO, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL,  fnGetBatteryParamInfo,  E_DPI620G_CMD_LEN_GET_BATTERY_PARAM_INFO ,  E_DPI620G_RESP_LEN_GET_BATTERY_PARAM_INFO , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_MEASUREMENT_AND_STATUS, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL, fnGetMeasurementAndStatus,   E_DPI620G_CMD_LEN_GET_MEASUREMENT_AND_STATUS ,  E_DPI620G_RESP_LEN_GET_MEASUREMENT_AND_STATUS , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_DATE_AND_TIME, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL, fnGetDateAndTime,  E_DPI620G_CMD_LEN_GET_DATE_AND_TIME , E_DPI620G_RESP_LEN_GET_DATE_AND_TIME , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_INPUT_PROCESSING, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL,  fnGetInputProcessing,  E_DPI620G_CMD_LEN_GET_INPUT_PROCESSING , E_DPI620G_RESP_LEN_GET_INPUT_PROCESSING , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_PRESSURE_INFO, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE,  NULL, fnGetPressureInfo,  E_DPI620G_CMD_LEN_GET_PRESSURE_INFO , E_DPI620G_RESP_LEN_GET_PRESSURE_INFO , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_TARE_VALUE, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE,  NULL, fnGetTareValue,  E_DPI620G_CMD_LEN_GET_TARE_VALUE , E_DPI620G_RESP_LEN_GET_TARE_VALUE , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_SERVICE_LOG, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL,    fnGetServiceLog,E_DPI620G_CMD_LEN_GET_SERVICE_LOG , E_DPI620G_RESP_LEN_GET_SERVICE_LOG , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_BAROMETER_INFO, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL,   fnGetBarometerInfo, E_DPI620G_CMD_LEN_GET_BAROMETER_INFO , E_DPI620G_RESP_LEN_GET_BAROMETER_INFO , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_FIRMWARE_UPGRADE_STATUS, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE,NULL, fnGetFirmwareUpgradeStatus,  E_DPI620G_CMD_LEN_GET_FIRMWARE_UPGRADE_STATUS , E_DPI620G_RESP_LEN_GET_FIRMWARE_UPGRADE_STATUS , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_ERROR_NUMBER, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL,  fnGetErrorNumber,  E_DPI620G_CMD_LEN_GET_ERROR_NUMBER , E_DPI620G_RESP_LEN_GET_ERROR_NUMBER , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_SET_FUNCTION, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL,  fnSetFunction,  E_DPI620G_CMD_LEN_SET_FUNCTION , E_DPI620G_RESP_LEN_SET_FUNCTION , true, 0xFFFFu); 
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
eCommOperationMode_t DCommsStateOwi::run(void)
{
  return E_COMMS_READ_OPERATION_MODE;
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
bool DCommsStateOwi::query(uint8_t *cmdBuf,uint8_t cmdLen, uint8_t **pRecvBuf, uint8_t recvLen)
{
    bool successFlag = false;
   
    return successFlag;
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
bool DCommsStateOwi::waitForCommand(uint8_t **pBuf)
{
    bool flag = false;
    uint8_t successFlag = (uint8_t)(0);
    uint32_t numOfBytesRead = 0u;
    uint32_t reponseLength =0u;   
    eOwiCommandType_t cmdType;
    sOwiError_t owiError;
    owiError.value = 1u;
    sOwiCommand_t *element = NULL;
    
    if (myCommsMedium != NULL)
    {        
        successFlag = (uint8_t)myCommsMedium->read(pBuf,  CMD_HEADER_SIZE, &numOfBytesRead, commandTimeoutPeriod);
        if(((uint8_t)(1) == successFlag) && (numOfBytesRead == CMD_HEADER_SIZE))
        {
          successFlag = (uint8_t)myParser->ValidateCheckSum(*pBuf,CMD_HEADER_SIZE);
          if((uint8_t)(1) == successFlag)
          {
              successFlag = (uint8_t)myParser->getHandleToCommandProperties((**pBuf)& OWI_CMD_MASK, &element);
              if((uint8_t)(1) == successFlag)
              {
                  cmdType = getCommandType((**pBuf) & OWI_CMD_MASK);
                  if((eOwiCommandType_t)E_OWI_CMD_READ == cmdType)
                  {
                       owiError = myParser->slaveParse(element->command, (uint8_t*)myTxBuffer, &reponseLength);
                       if(0u == owiError.value )
                       {
                         myParser->CalculateAndAppendCheckSum((uint8_t*)myTxBuffer,reponseLength,&reponseLength);
                         myCommsMedium->write((uint8_t*)myTxBuffer,reponseLength);
                         //successFlag = write((uint8_t*)myTxBuffer,(uint8_t)reponseLength);     
                       }
                       else
                       {
                         //Falied to send response
                         successFlag = (uint8_t)(0);
                       }              
                  }
                  else if((eOwiCommandType_t)E_OWI_CMD_WRITE == cmdType)
                  {
                       sendAck(element->command);
                       numOfBytesRead = 0u;
                       myParser->getResponseLength(element->command, &reponseLength);
                       successFlag = myCommsMedium->read(pBuf,  reponseLength, &numOfBytesRead, commandTimeoutPeriod);
                       if((true == successFlag) && (numOfBytesRead == reponseLength))
                       {
                          successFlag = myParser->ValidateCheckSum(*pBuf,numOfBytesRead);
                          if(true == successFlag)
                          {
                               owiError = myParser->slaveParse(element->command,*pBuf, &reponseLength);
                               if(0u == owiError.value )
                               {
                                 // Command Execution Success
                                 sendAck(element->command);   
                               }
                               else
                               {
                                 // Invalid Data and hence command execution failed
                                 sendNck();
                                 successFlag = false;
                               }              
                          }
                          else
                          {
                            // Invalid Checksum
                            sendNck();
                            successFlag = false;
                          }
                       }
                       else
                       {
                         //  Timeout and command data Read Failed. 
                          successFlag = false;
                       }
                  }
                  else
                  {
                    /* Do Nothing*/
                  }
              }
              else
              {
                // Command Not Found
                sendNck();
                successFlag = false;
              }
          }
          else
          {
            // Check Sum Not Matched
            sendNck();
            successFlag = false;
          }
        }
        else
        {
           //  Timeout and command Read Failed. 
           successFlag = false;
        }
 
    }
    
    if(successFlag == (uint8_t)(0))
    {
      flag = false;
    }
    else
    {
      flag = true;
    }
         
    return flag;
    
}

/**
 * @brief   This function sends not acknowledged response
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
void DCommsStateOwi::sendNck(void)
{
    uint32_t responseLength = (uint32_t)(OWI_NCK_LENGTH);
    
    myTxBuffer[0] = (uint8_t)(E_OWI_RESPONSE_NCK);
    
    myParser->CalculateAndAppendCheckSum((uint8_t*)myTxBuffer,
                                              responseLength,    
                                              &responseLength);
    
    myCommsMedium->write((uint8_t*)myTxBuffer,responseLength);  
}

/**
 * @brief   This function sends not acknowledged response
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
void DCommsStateOwi::sendAck(uint8_t cmd)
{
    uint32_t responseLength = (uint32_t)(OWI_ACK_LENGTH);
    
    myTxBuffer[0] = (uint8_t)(E_OWI_RESPONSE_ACC);
    myTxBuffer[1] = cmd;
    
    myParser->CalculateAndAppendCheckSum((uint8_t*)myTxBuffer,
                                              responseLength,    
                                              &responseLength);
    
    myCommsMedium->write((uint8_t*)myTxBuffer,responseLength);
}

/*********************** STATIC FUNCTIONS **********************************/
/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetVersionInfo(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetVersionInfo(paramBuf, paramBufSize);
    }
    
    return error;  
}

/**
 * @brief   Prepare data to send the current operating mode and access level to DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnSetFunction(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnSetFunction(paramBuf, paramBufSize);
    }
    
    return error;
}
/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetPM620SensorInfo(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetPM620SensorInfo(paramBuf, paramBufSize);
    }
    
    return error;
  
}

/*********************** STATIC FUNCTIONS **********************************/
/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnSetOperatingMode(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnSetOperatingMode(paramBuf, paramBufSize);
    }
    
    return error;
  
}

/*********************** STATIC FUNCTIONS **********************************/
/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetOperatingModeAndAccessLevel(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetOperatingModeAndAccessLevel(paramBuf, paramBufSize);
    }
    
    return error;
  
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetDeviceStatus(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetDeviceStatus(paramBuf, paramBufSize);
    }
    return error;
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetDeviceConfiguration(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetDeviceConfiguration(paramBuf, paramBufSize);
    }
    
    return error;
  
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetBatteryParamInfo(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetBatteryParamInfo(paramBuf, paramBufSize);
    }
    
    return error;
  
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetMeasurementAndStatus(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetMeasurementAndStatus(paramBuf, paramBufSize);
    }
    
    return error;
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetServiceLog(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetServiceLog(paramBuf, paramBufSize);
    }
    
    return error;
  
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetDateAndTime(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetDateAndTime(paramBuf, paramBufSize);
    }
    
    return error;
  
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetBarometerInfo(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetBarometerInfo(paramBuf, paramBufSize);
    }
    return error;
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetPressureInfo(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetPressureInfo(paramBuf, paramBufSize);
    }
    return error;
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetFirmwareUpgradeStatus(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetFirmwareUpgradeStatus(paramBuf, paramBufSize);
    }
    
    return error;
  
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetErrorNumber(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {     
        error = callbackInstance->fnGetErrorNumber(paramBuf, paramBufSize);
    }
  
  return error;
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetTareValue(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetTareValue(paramBuf, paramBufSize);
    }
    
    return error;  
}

/**
 * @brief   Points to get the version information about PV624 to be sent to
 *          DPI620G
 *
 * @param   *instance - function pointer to the callback function  
 *          *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetInputProcessing(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    DCommsStateOwi *callbackInstance = (DCommsStateOwi*)instance;
    
    if(NULL == callbackInstance)
    {
        // Set error
    }
    else
    {
        error = callbackInstance->fnGetInputProcessing(paramBuf, paramBufSize);
    }
    
    return error;
  
}

/*********************** CALLBACK DEFINITIONS ******************************/
/**
 * @brief   Get the version information of the PV624 to send it to DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetVersionInfo(uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    uint32_t index = (uint32_t)(0);
    
    /* 
    This is simulating the version information for the PV624 application
    and the bootloader.
    Byte 0 and byte 1: DK number of bootloader: 0500
    Byte 2: Bootloader version:                 1
    Byte 3: Issue number:                       23
    Byte 4: BUild number:                       45

    Byte 5 and byte 6: DK number of app:        0499
    Byte 7: Application version:                6
    Byte 8: Issue number:                       78
    Byte 9: Build number:                       90
    
    The above values are only for test
    */
    
    /* 
    The following code is only for test purpose 
    Actual implementation of the function shall be different */   
    paramBuf[index] = (uint8_t)(0x01);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(0xF4);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(0x01);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(0x17);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(0x2D);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(0x01);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(0xF3);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(0x06);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(0x4E);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(0x5A);
    index = index + (uint32_t)(1);
    
    *paramBufSize = index;
    
        
    return error;
}

/**
 * @brief   Read PM620 version information to be sent to DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnSetFunction(uint8_t *paramBuf, 
                                                 uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    eFunction_t func = (eFunction_t)(E_FUNCTION_NONE);
    
    switch((eMeasureFunction_t)(*paramBuf))
    {
    case E_FUNC_NONE:
        break;
        
    case E_FUNC_GAUGE_PRESSURE:
        func = E_FUNCTION_EXT_PRESSURE;
        break;
        
    case E_FUNC_ABSOLUTE_PRESSURE:
        break;
        
    case E_FUNC_ATMOSPHERIC_PRESSURE:
        func = E_FUNCTION_BAROMETER;
        break;
        
    default:
        error.value = (uint32_t)(1);
        break;
    }

    if((uint32_t)(0) == error.value)
    {
        PV624->instrument->setFunction(E_CHANNEL_3,
                                        func, 
                                        E_FUNCTION_DIR_MEASURE);
    }
    
    return error;
}

/**
 * @brief   Read PM620 version information to be sent to DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetPM620SensorInfo(uint8_t *paramBuf, 
                                                 uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
          
    return error;
}

/**
 * @brief   Set an operating mode on the PV624 based on command
 *          Command could be any one of:
 *          1. Protected mode
 *          2. Calibration mode
 *          3. Service Mode
 *          4. Factory mode
 *          5. Production test mode*
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnSetOperatingMode(uint8_t *paramBuf, 
                                               uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    if(currentWriteMaster == (eCommMasterInterfaceType_t)E_COMMS_MASTER_NONE)
    {
        nextOperationMode = E_COMMS_WRITE_OPERATION_MODE;
        currentWriteMaster = (eCommMasterInterfaceType_t)E_COMMS_MASTER_OWI;
    }
    else
    {
        error.invalid_args = 1u;
    }
    return error;  
}

/**
 * @brief   Prepare data to send the current operating mode and access level to DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetOperatingModeAndAccessLevel(uint8_t *paramBuf, 
                                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error; 
}

/**
 * @brief   Prepare the data to send the current device status to DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetDeviceStatus(uint8_t *paramBuf, 
                                              uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}


/**
 * @brief   Prepare device configuration data to be sent to the DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetDeviceConfiguration(uint8_t *paramBuf, 
                                                     uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    return error;
}    
    
/**
 * @brief   Prepare device configuration data to be sent to the DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetBatteryParamInfo(uint8_t *paramBuf, 
                                                     uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}

/**
 * @brief   Prepare the measurement and status information of the PV624
 *          pressure base to be sent to the DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetMeasurementAndStatus(uint8_t *paramBuf, 
                                                      uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;
}

/**
 * @brief   Get the current date and time information of the PV6204 to be sent
 *          to the DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetDateAndTime(uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}

/**
 * @brief   Prepare service log data to be sent to DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetServiceLog(uint8_t *paramBuf, 
                                            uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}

/**
 * @brief   Read data from the barometer if connected, and prepare to send to
 *          DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetBarometerInfo(uint8_t *paramBuf, 
                                               uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}

/**
 * @brief   Read information about the reference pressure sensor connected to 
 *          PV624 base station to be sent to DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetPressureInfo(uint8_t *paramBuf, 
                                              uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}

/**
 * @brief   Prepare firmware upgrade status information to be polled by upgrade
 *          utility
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetFirmwareUpgradeStatus(uint8_t *paramBuf, 
                                                       uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}

/**
 * @brief   Prepare to send errors to DPI620G
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetErrorNumber(uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}

/**
 * @brief   Read the Tare value currently set in the PV624
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetTareValue(uint8_t *paramBuf, 
                                           uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}

/**
 * @brief   Get the current operational status of the PV624 base station
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnGetInputProcessing(uint8_t *paramBuf, 
                                                 uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    
    return error;  
}


/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")




/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")

 eOwiCommandType_t  DCommsStateOwi::getCommandType(uint8_t cmd)
 {
    eOwiCommandType_t cmdType = E_OWI_CMD_NONE;
    
    
    switch(cmd)
    {
        /* All read cases */
    case E_DPI620G_CMD_GET_VERSION_INFO:
    case E_DPI620G_CMD_GET_PM620_SENSOR_INFO:
    case E_DPI620G_CMD_GET_OPERATING_MODE_ACCESS_LEVEL:
        cmdType = E_OWI_CMD_READ;
        break;
        
        /* all write cases */
    case E_DPI620G_CMD_SET_OPERATING_MODE:
    case E_DPI620G_CMD_SET_FUNCTION:
        cmdType = E_OWI_CMD_WRITE;
        break;
        
    default:
      break;
    }
    return cmdType;
 }
