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
#include "main.h"

#include "stm32l4xx_hal_rtc.h"
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
    /* The following commands are added for the parser to identify and process*/
  
    /* Add command GET VERSION INFORMATION from PV624 */
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
    
    /* Add command GET PM620 SENSOR INFORMATION FROM SENSOR > PV624 > DPI620G */
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

    /* Add command to SET OPERATING MODE of the PV624 */
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
    
    /* Add command to AUTHENTICATE USER of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_AUTHENTICATE, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL, 
                         fnSetAuthenticate,   
                         E_DPI620G_CMD_LEN_SET_AUTHENTICATE, 
                         E_DPI620G_RESP_LEN_SET_AUTHENTICATE, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to SET OPERATING MODE AND ACCESS LEVEL of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_OPERATING_MODE_ACCESS_LEVEL, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL, 
                         fnGetOperatingModeAndAccessLevel,    
                         E_DPI620G_CMD_LEN_GET_OPERATING_MODE_ACCESS_LEVEL, 
                         E_DPI620G_RESP_LEN_GET_OPERATING_MODE_ACCESS_LEVEL, 
                         false, 
                         0xFFFFu);     

    /* Add command to ERASE CALIBRATION DATA of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_ERASE_CONFIG_CALIB_DATA, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL, 
                         fnSetEraseConfigAndCalibData,    
                         E_DPI620G_CMD_LEN_SET_ERASE_CONFIG_CALIB_DATA, 
                         E_DPI620G_RESP_LEN_SET_ERASE_CONFIG_CALIB_DATA, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to GET INSTRUMENT ERROR of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_INSTRUMENT_ERROR, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL, 
                         fnGetDeviceStatus,   
                         E_DPI620G_CMD_LEN_GET_INSTRUMENT_ERROR, 
                         E_DPI620G_RESP_LEN_GET_INSTRUMENT_ERROR, 
                         false, 
                         0xFFFFu); 

    /* Add command to SET INSTRUMENT CONFIGURATION of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_INSTRUMENT_CONFIGURATION, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,  
                         NULL,   
                         fnGetDeviceConfiguration, 
                         E_DPI620G_CMD_LEN_SET_INSTRUMENT_CONFIGURATION, 
                         E_DPI620G_RESP_LEN_SET_INSTRUMENT_CONFIGURATION, 
                         false, 
                         0xFFFFu);

    /* Add command to GET INSTRUMENT CONFIGURATION of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_INSTRUMENT_CONFIGURATION, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,  
                         NULL,   
                         fnGetDeviceConfiguration, 
                         E_DPI620G_CMD_LEN_GET_INSTRUMENT_CONFIGURATION, 
                         E_DPI620G_RESP_LEN_GET_INSTRUMENT_CONFIGURATION, 
                         false, 
                         0xFFFFu);
    
    /* Add command to OPTIONAL FEATURES of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_OPTIONAL_FEATURES, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,  
                         NULL,   
                         fnSetOptionalFeatures, 
                         E_DPI620G_CMD_LEN_SET_OPTIONAL_FEATURES, 
                         E_DPI620G_RESP_LEN_SET_OPTIONAL_FEATURES, 
                         false, 
                         0xFFFFu);

    /* Add command to GET BATTERY PARAMETERS of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_BATTERY_PARAM_INFO, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,  
                         fnGetBatteryParamInfo,  
                         E_DPI620G_CMD_LEN_GET_BATTERY_PARAM_INFO,  
                         E_DPI620G_RESP_LEN_GET_BATTERY_PARAM_INFO, 
                         false, 
                         0xFFFFu);     
    
    /* Add command to GET MEASUREMENT VALUE AND STATUS of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_MEASUREMENT_AND_STATUS, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL, 
                         fnGetMeasurementAndStatus,   
                         E_DPI620G_CMD_LEN_GET_MEASUREMENT_AND_STATUS ,  
                         E_DPI620G_RESP_LEN_GET_MEASUREMENT_AND_STATUS , 
                         false, 
                         0xFFFFu);
    
    /* Add command to GET BAROMETER INFORMATION of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_BAROMETER_INFO, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,   
                         fnGetBarometerInfo, 
                         E_DPI620G_CMD_LEN_GET_BAROMETER_INFO, 
                         E_DPI620G_RESP_LEN_GET_BAROMETER_INFO, 
                         false, 
                         0xFFFFu); 

    /* Add command to SET DATE AND TIME of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_DATE_AND_TIME, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,   
                         fnSetDateAndTime, 
                         E_DPI620G_CMD_LEN_SET_DATE_AND_TIME, 
                         E_DPI620G_RESP_LEN_SET_DATE_AND_TIME, 
                         true, 
                         0xFFFFu);
    
    /* Add command to GET DATE AND TIME of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_DATE_AND_TIME, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,   
                         fnGetDateAndTime, 
                         E_DPI620G_CMD_LEN_GET_DATE_AND_TIME, 
                         E_DPI620G_RESP_LEN_GET_DATE_AND_TIME, 
                         false, 
                         0xFFFFu);

    /* Add command to SET BAROMETER CALIBRATION PARAMETERS of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_BARO_CALIB_PARAMS, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,   
                         fnSetBarometerCalibrationParams, 
                         E_DPI620G_CMD_LEN_SET_BARO_CALIB_PARAMS, 
                         E_DPI620G_RESP_LEN_SET_BARO_CALIB_PARAMS, 
                         false, 
                         0xFFFFu);

    /* Add command to SET INPUT PROCESSING of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_INPUT_PROCESSING, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,   
                         fnSetInputProcessing, 
                         E_DPI620G_CMD_LEN_SET_INPUT_PROCESSING, 
                         E_DPI620G_RESP_LEN_SET_INPUT_PROCESSING, 
                         false, 
                         0xFFFFu);
    
    /* Add command to GET INPUT PROCESSING of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_INPUT_PROCESSING, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,  
                         fnGetInputProcessing,  
                         E_DPI620G_CMD_LEN_GET_INPUT_PROCESSING, 
                         E_DPI620G_RESP_LEN_GET_INPUT_PROCESSING, 
                         false, 
                         0xFFFFu);
    
    /* Add command to VENT PRESSURE of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_VENT_PRESSURE, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,  
                         fnSetVentPressure,  
                         E_DPI620G_CMD_LEN_SET_VENT_PRESSURE, 
                         E_DPI620G_RESP_LEN_SET_VENT_PRESSURE, 
                         false, 
                         0xFFFFu);
    
    /* Add command to SET TARE VALUE of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_TARE_VALUE, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,  
                         fnSetTareValue,  
                         E_DPI620G_CMD_LEN_SET_TARE_VALUE, 
                         E_DPI620G_RESP_LEN_SET_TARE_VALUE, 
                         false, 
                         0xFFFFu);

    /* Add command to GET TARE VALUE of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_TARE_VALUE, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,  
                         NULL, 
                         fnGetTareValue,  
                         E_DPI620G_CMD_LEN_GET_TARE_VALUE, 
                         E_DPI620G_RESP_LEN_GET_TARE_VALUE, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to GET CALIBRATION PARAMETERS of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_CALIBRATION_PARAMS, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,  
                         NULL, 
                         fnGetCalibrationParams,  
                         E_DPI620G_CMD_LEN_GET_CALIBRATION_PARAMS, 
                         E_DPI620G_RESP_LEN_GET_CALIBRATION_PARAMS, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to SET CALIBRATION STATE of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_CALIBRATION_STATE, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,  
                         NULL, 
                         fnSetCalibrationState,  
                         E_DPI620G_CMD_LEN_SET_CALIBRATION_STATE, 
                         E_DPI620G_RESP_LEN_SET_CALIBRATION_STATE, 
                         false, 
                         0xFFFFu);    
    
    /* Add command to SET CALIBRATION PARAMETERS of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_CALIBRATION_PARAMS, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,  
                         NULL, 
                         fnSetCalibrationParams,  
                         E_DPI620G_CMD_LEN_SET_CALIBRATION_PARAMS, 
                         E_DPI620G_RESP_LEN_SET_CALIBRATION_PARAMS, 
                         false, 
                         0xFFFFu);     
   
    
    /* Add command to SET CALIBRATION POINT of the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_CALIBRATION_POINT, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,  
                         NULL, 
                         fnSetCalibrationPoints,  
                         E_DPI620G_CMD_LEN_SET_CALIBRATION_POINT, 
                         E_DPI620G_RESP_LEN_SET_CALIBRATION_POINT, 
                         false, 
                         0xFFFFu);  
    
    /* Add command to GET THE SERVICE log from the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_SERVICE_LOG, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,
                         fnGetServiceLog,
                         E_DPI620G_CMD_LEN_GET_SERVICE_LOG, 
                         E_DPI620G_RESP_LEN_GET_SERVICE_LOG, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to CLEAR THE SERVICE log in the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_CLEAR_LOG, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,
                         fnSetClearServiceLog,
                         E_DPI620G_CMD_LEN_SET_CLEAR_LOG, 
                         E_DPI620G_RESP_LEN_SET_CLEAR_LOG, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to GET BAROMETER INFORMATION log in the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_BAROMETER_INFO, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,
                         fnGetBarometerInfo,
                         E_DPI620G_CMD_LEN_GET_BAROMETER_INFO, 
                         E_DPI620G_RESP_LEN_GET_BAROMETER_INFO, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to SET CLAIBRATION DEVICE log in the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_SELECT_DEVICE_FOR_CAL, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,
                         fnSetSelectDeviceForCalibration,
                         E_DPI620G_CMD_LEN_SET_SELECT_DEVICE_FOR_CAL, 
                         E_DPI620G_RESP_LEN_SET_SELECT_DEVICE_FOR_CAL, 
                         false, 
                         0xFFFFu); 
    
        
    /* Add command to GET LATEST FIRMWARE VERSION log in the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_LATEST_FIRMWARE_VERSION, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,
                         fnGetLatestFirmwareVersion,
                         E_DPI620G_CMD_LEN_GET_LATEST_FIRMWARE_VERSION, 
                         E_DPI620G_RESP_LEN_GET_LATEST_FIRMWARE_VERSION, 
                         false, 
                         0xFFFFu); 

        
    /* Add command to START FIRMWARE UPGRADE in the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_START_FIRMWARE_UPGRADE, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,
                         fnSetStartFirmwareUpgrade,
                         E_DPI620G_CMD_LEN_SET_START_FIRMWARE_UPGRADE, 
                         E_DPI620G_RESP_LEN_SET_START_FIRMWARE_UPGRADE, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to GET FIRMWARE UPGRADE STATUS in the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_FIRMWARE_UPGRADE_STATUS, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE,
                         NULL, 
                         fnGetFirmwareUpgradeStatus,  
                         E_DPI620G_CMD_LEN_GET_FIRMWARE_UPGRADE_STATUS, 
                         E_DPI620G_RESP_LEN_GET_FIRMWARE_UPGRADE_STATUS, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to GET ERROR NUMBER in the PV624 */
    myParser->addCommand(E_DPI620G_CMD_GET_ERROR_NUMBER, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,  
                         fnGetErrorNumber,  
                         E_DPI620G_CMD_LEN_GET_ERROR_NUMBER, 
                         E_DPI620G_RESP_LEN_GET_ERROR_NUMBER, 
                         false, 
                         0xFFFFu); 
    
    /* Add command to SET A FUNCTION in the PV624 */
    myParser->addCommand(E_DPI620G_CMD_SET_FUNCTION, 
                         owiArgByteArray,  
                         E_OWI_BYTE, 
                         E_OWI_BYTE, 
                         NULL,  
                         fnSetFunction,  
                         E_DPI620G_CMD_LEN_SET_FUNCTION , 
                         E_DPI620G_RESP_LEN_SET_FUNCTION , 
                         true, 
                         0xFFFFu);             
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

/**9
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
    uint32_t commandDataLength = (uint32_t)(0);
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
                       //myParser->getResponseLength(element->command, &reponseLength);
                       myParser->getCommandDataLength(element->command, 
                                                      &commandDataLength);
                       successFlag = myCommsMedium->read(pBuf,  
                                                         commandDataLength, 
                                                         &numOfBytesRead, 
                                                         commandTimeoutPeriod);
                       if((true == successFlag) && (numOfBytesRead == commandDataLength))
                       {
                          successFlag = myParser->ValidateCheckSum(*pBuf,
                                                                   numOfBytesRead);
                          if(true == successFlag)
                          {
                               owiError = myParser->slaveParse(element->command,
                                                               *pBuf, 
                                                               &commandDataLength);
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
sOwiError_t DCommsStateOwi::fnSetStartFirmwareUpgrade(void *instance, 
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
        error = callbackInstance->fnSetStartFirmwareUpgrade(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnGetLatestFirmwareVersion(void *instance, 
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
        error = callbackInstance->fnGetLatestFirmwareVersion(paramBuf, 
                                                             paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetSelectDeviceForCalibration(void *instance, 
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
        error = callbackInstance->fnSetSelectDeviceForCalibration(paramBuf, 
                                                                  paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetClearServiceLog(void *instance, 
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
        error = callbackInstance->fnSetClearServiceLog(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetCalibrationPoints(void *instance, 
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
        error = callbackInstance->fnSetCalibrationPoints(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetCalibrationParams(void *instance, 
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
        error = callbackInstance->fnSetCalibrationParams(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetCalibrationState(void *instance, 
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
        error = callbackInstance->fnSetCalibrationState(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetTareValue(void *instance, 
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
        error = callbackInstance->fnSetTareValue(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetVentPressure(void *instance, 
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
        error = callbackInstance->fnSetVentPressure(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetInputProcessing(void *instance, 
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
        error = callbackInstance->fnSetInputProcessing(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetBarometerCalibrationParams(void *instance, 
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
        error = callbackInstance->fnSetBarometerCalibrationParams(paramBuf, 
                                                                  paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetDateAndTime(void *instance, 
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
        error = callbackInstance->fnSetDateAndTime(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetOptionalFeatures(void *instance, 
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
        error = callbackInstance->fnSetOptionalFeatures(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetEraseConfigAndCalibData(void *instance, 
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
        error = callbackInstance->fnSetEraseConfigAndCalibData(paramBuf, 
                                                               paramBufSize);
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
sOwiError_t DCommsStateOwi::fnSetAuthenticate(void *instance, 
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
        error = callbackInstance->fnSetAuthenticate(paramBuf, paramBufSize);
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
sOwiError_t DCommsStateOwi::fnGetCalibrationParams(void *instance, 
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
        error = callbackInstance->fnGetCalibrationParams(paramBuf, paramBufSize);
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
    paramBuf[index] = (uint8_t)(BUILD_NUMBER);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(MAJOR_VERSION_NUMBER);
    index = index + (uint32_t)(1);
    paramBuf[index] = (uint8_t)(MINOR_VERSION_NUMBER);
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
        
    case E_FUNC_PRESSURE:
        func = E_FUNCTION_EXT_PRESSURE;
        break;
        
    case E_FUNC_PSEUDO_GAUGE_PRESSURE:
        func = E_FUNCTION_PSEUDO_GAUGE;
        break;
        
    case E_FUNC_PSEUDO_ABSOLUTE_PRESSURE:
        func = E_FUNCTION_PSEUDO_ABS;
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
        PV624->instrument->setFunction( func );
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
    eSensorType_t senType;
    uFloat_t uFvalue;
    uUint32_t uValue;
    sDate_t sDate;
    uint8_t index = 0u;
    uFvalue.floatValue = 0.0f;    
    
    uValue.uint32Value = PV624->mySerialNumber;
    paramBuf[index++] = (uint8_t)uValue.byteValue[3];
    paramBuf[index++] = (uint8_t)uValue.byteValue[2];
    paramBuf[index++] = (uint8_t)uValue.byteValue[1];
    paramBuf[index++] = (uint8_t)uValue.byteValue[0];
    
    PV624->instrument->getManufactureDate((sDate_t*) &sDate); 
    paramBuf[index++] = (uint8_t)sDate.day;
    paramBuf[index++] = (uint8_t)sDate.month;
    paramBuf[index++] = (uint8_t)(sDate.year >> 8u);
    paramBuf[index++] = (uint8_t)(sDate.year & 0XFFu);
    
    PV624->instrument->getUserCalDate( (sDate_t*) &sDate); 
    paramBuf[index++] = (uint8_t)sDate.day;
    paramBuf[index++] = (uint8_t)sDate.month;
    paramBuf[index++] = (uint8_t)(sDate.year >> 8u);
    paramBuf[index++] = (uint8_t)(sDate.year & 0XFFu);
    
    paramBuf[index++] = (uint8_t)0;
    paramBuf[index++] = (uint8_t)0;
    paramBuf[index++] = (uint8_t)0;
    
    PV624->instrument->getSensorType( (eSensorType_t*) &senType); 
    paramBuf[index++] = (uint8_t)0;
    paramBuf[index++] = (uint8_t)0;
    paramBuf[index++] = (uint8_t)0;
    paramBuf[index++] = (uint8_t)senType;
    
    PV624->instrument->getPosFullscale( (float*) &uFvalue.floatValue);
    paramBuf[index++] = (uint8_t)uFvalue.byteValue[3];
    paramBuf[index++] = (uint8_t)uFvalue.byteValue[2];
    paramBuf[index++] = (uint8_t)uFvalue.byteValue[1];
    paramBuf[index++] = (uint8_t)uFvalue.byteValue[0];
    
    uFvalue.floatValue = 0.0f;
    PV624->instrument->getNegFullscale( (float*) &uFvalue.floatValue);
    paramBuf[index++] = (uint8_t)uFvalue.byteValue[3];
    paramBuf[index++] = (uint8_t)uFvalue.byteValue[2];
    paramBuf[index++] = (uint8_t)uFvalue.byteValue[1];
    paramBuf[index++] = (uint8_t)uFvalue.byteValue[0];
    
    *paramBufSize = index;
    
   
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
    uFloat_t ufValue;
    uint8_t index = 0u;
    
    PV624->instrument->getReading((eValueIndex_t) E_VAL_INDEX_VALUE,(float*) &ufValue.floatValue);
    
    paramBuf[index++] = ufValue.byteValue[0];
    paramBuf[index++] = ufValue.byteValue[1];
    paramBuf[index++] = ufValue.byteValue[2];
    paramBuf[index++] = ufValue.byteValue[3];
    
    paramBuf[index++] = 0x00u;
    paramBuf[index++] = 0x00u;
    paramBuf[index++] = 0x00u;
    paramBuf[index++] = 0x00u;
    
    *paramBufSize = index;
    
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
    sDate_t sDate;
    sTime_t sTime;
    uint32_t index = (uint32_t)(0);
    error.value = (uint32_t)(0);
    bool rtcStatus = false;
    
    RTC_TimeTypeDef sTestTime;
    RTC_DateTypeDef sTestDate;
    
    rtcStatus = PV624->realTimeClock->isClockSet();
    if(true == rtcStatus)
    {        
        PV624->realTimeClock->getDateAndTime(&sDate, &sTime);
        
        paramBuf[index++] = (uint8_t)(sDate.year >> 8);
        paramBuf[index++] = (uint8_t)(sDate.year);
        paramBuf[index++] = (uint8_t)(sDate.month);
        paramBuf[index++] = (uint8_t)(sDate.day);
        
        paramBuf[index++] = (uint8_t)(sTime.hours);
        paramBuf[index++] = (uint8_t)(sTime.minutes);
        paramBuf[index++] = (uint8_t)(sTime.seconds);
        
        *paramBufSize = index; 
    }
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
    uint8_t baroIdentity = (uint8_t)(0);
    uint32_t index = (uint32_t)(0);
    
    /* 
    Barometer sensor information shall return the unique number for
    installed barometer sensor.
    If sensor is not installed, error shall be returned.
    The actual length of the response is not fixed yet - TODO
    For an ST LPS22HH barometer the function shall return its unique 
    identification number read from WHO_AM_I register.
    Rest of the bytes here shall be blank.
    This is only for test */
    baroIdentity = (uint8_t)(0xB3);
    /* read who am I register to baroIdentity*/
    paramBuf[index++] = baroIdentity;
    paramBuf[index++] = (uint8_t)(0);
    paramBuf[index++] = (uint8_t)(0);
    paramBuf[index++] = (uint8_t)(0);
    paramBuf[index++] = (uint8_t)(0);
    paramBuf[index++] = (uint8_t)(0);
    paramBuf[index++] = (uint8_t)(0);
    paramBuf[index++] = (uint8_t)(0);
    
    *paramBufSize = index;
      
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
    uFloat_t ufValue;
    uint32_t index = (uint32_t)(0);
    
    /* need to get  this from tare process class */
    ufValue.floatValue = PV624->myTareValue;
    
    paramBuf[index++] = ufValue.byteValue[0];
    paramBuf[index++] = ufValue.byteValue[1];
    paramBuf[index++] = ufValue.byteValue[2];
    paramBuf[index++] = ufValue.byteValue[3];
    
    *paramBufSize = index;
    
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

/**
 * @brief   Get the current operational status of the PV624 base station
 *
 * @param   *paramBuf - buffer that is used to store data to be sent
 *          *paramBufSize - Length of the data bytes
 *
 * @return  sOwiError_t error - any errors that are generated
 */
sOwiError_t DCommsStateOwi::fnSetStartFirmwareUpgrade(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetSelectDeviceForCalibration(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetClearServiceLog(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetCalibrationPoints(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetCalibrationParams(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetCalibrationState(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetTareValue(uint8_t *paramBuf, 
                                           uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    uFloat_t ufValue;
    
    /* Sets the currently measured pressure as the tare value */
    
    /* This function has only ACK as response */
    
    /* This will get barometer value if atmospheric pressure is selected 
        What do we need to call here to get the measured pressure value ? - TODO
        Also need to include from TARE process class
    */
    
    PV624->instrument->getReading((eValueIndex_t) E_VAL_INDEX_VALUE,(float*) &ufValue.floatValue);
    
    PV624->myTareValue = ufValue.floatValue;
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
sOwiError_t DCommsStateOwi::fnSetVentPressure(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetInputProcessing(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetBarometerCalibrationParams(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetDateAndTime(uint8_t *paramBuf, 
                                             uint32_t* paramBufSize)
{
    sOwiError_t error;
    error.value = (uint32_t)(0);
    sDate_t sDate;
    sTime_t sTime;
    uint32_t index = (uint32_t)(0);
    
    sDate.year = (uint32_t)(((uint32_t)(paramBuf[index++])) << 8);
    sDate.year = sDate.year | (uint32_t)(paramBuf[index++]);
    sDate.month = (uint32_t)(paramBuf[index++]);
    sDate.day = (uint32_t)(paramBuf[index++]);
    
    sTime.hours = (uint32_t)(paramBuf[index++]);
    sTime.minutes = (uint32_t)(paramBuf[index++]);
    sTime.seconds = (uint32_t)(paramBuf[index++]);
    
    PV624->realTimeClock->setDateAndTime(sDate.day,
                                         sDate.month,
                                         sDate.year,
                                         sTime.hours,
                                         sTime.minutes,
                                         sTime.seconds);
    
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
sOwiError_t DCommsStateOwi::fnSetOptionalFeatures(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetEraseConfigAndCalibData(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnSetAuthenticate(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnGetLatestFirmwareVersion(uint8_t *paramBuf, 
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
sOwiError_t DCommsStateOwi::fnGetCalibrationParams(uint8_t *paramBuf, 
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

/**
 * @brief   Find out the type of the command that is received by the PV624
 *
 * @param   uint8_t cmd - value of the command received
 *
 * @return  eOwiCommandType_t cmdType - Type of command, READ, WRITE or NONE
 */
 eOwiCommandType_t  DCommsStateOwi::getCommandType(uint8_t cmd)
 {
    eOwiCommandType_t cmdType = E_OWI_CMD_NONE;
    
    
    switch(cmd)
    {
    case E_DPI620G_CMD_GET_VERSION_INFO:
    case E_DPI620G_CMD_GET_PM620_SENSOR_INFO:    
    case E_DPI620G_CMD_GET_OPERATING_MODE_ACCESS_LEVEL:    
    case E_DPI620G_CMD_GET_INSTRUMENT_ERROR:
    case E_DPI620G_CMD_SET_INSTRUMENT_CONFIGURATION:
    case E_DPI620G_CMD_GET_INSTRUMENT_CONFIGURATION:    
    case E_DPI620G_CMD_GET_BATTERY_PARAM_INFO:
    case E_DPI620G_CMD_GET_MEASUREMENT_AND_STATUS:
    case E_DPI620G_CMD_GET_BAROMETER_READING:    
    case E_DPI620G_CMD_GET_DATE_AND_TIME:    
    case E_DPI620G_CMD_GET_INPUT_PROCESSING:    
    case E_DPI620G_CMD_GET_TARE_VALUE:
    case E_DPI620G_CMD_GET_CALIBRATION_PARAMS:    
    case E_DPI620G_CMD_GET_SERVICE_LOG:    
    case E_DPI620G_CMD_GET_BAROMETER_INFO:    
    case E_DPI620G_CMD_GET_LATEST_FIRMWARE_VERSION:    
    case E_DPI620G_CMD_GET_FIRMWARE_UPGRADE_STATUS:
    case E_DPI620G_CMD_GET_ERROR_NUMBER:
        cmdType = E_OWI_CMD_READ;
        break;
    
    case E_DPI620G_CMD_SET_OPERATING_MODE:
    case E_DPI620G_CMD_SET_AUTHENTICATE:
    case E_DPI620G_CMD_SET_ERASE_CONFIG_CALIB_DATA:
    case E_DPI620G_CMD_SET_OPTIONAL_FEATURES:
    case E_DPI620G_CMD_SET_DATE_AND_TIME:
    case E_DPI620G_CMD_SET_BARO_CALIB_PARAMS:
    case E_DPI620G_CMD_SET_INPUT_PROCESSING:
    case E_DPI620G_CMD_SET_VENT_PRESSURE:
    case E_DPI620G_CMD_SET_TARE_VALUE:
    case E_DPI620G_CMD_SET_CALIBRATION_STATE:
    case E_DPI620G_CMD_SET_CALIBRATION_PARAMS:
    case E_DPI620G_CMD_SET_CALIBRATION_POINT:
    case E_DPI620G_CMD_SET_CLEAR_LOG:
    case E_DPI620G_CMD_SET_SELECT_DEVICE_FOR_CAL:
    case E_DPI620G_CMD_SET_START_FIRMWARE_UPGRADE:
    case E_DPI620G_CMD_SET_FUNCTION:
        cmdType = E_OWI_CMD_WRITE;
        break;
      
    default:
        cmdType = E_OWI_CMD_NONE;
        break;


    }
#if 0    
    switch(cmd)
    {
        /* All read cases */
    case E_DPI620G_CMD_GET_VERSION_INFO:
    case E_DPI620G_CMD_GET_PM620_SENSOR_INFO:
    case E_DPI620G_CMD_GET_OPERATING_MODE_ACCESS_LEVEL:
    case E_DPI620G_CMD_GET_MEASUREMENT_AND_STATUS:
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
#endif
    
    return cmdType;
 }
