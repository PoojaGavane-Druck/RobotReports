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
#include "Utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define CMD_HEADER_SIZE  2u
#define OWI_CMD_MASK 0x3Fu
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
#if 0
sExternalDevice_t DCommsStateOwi::externalDevice = { 0 };
#endif
eStateOwiComms_t DCommsStateOwi::commsOwnership = E_STATE_OWI_COMMS_OWNED;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsState class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateOwi::DCommsStateOwi(DDeviceSerial *commsMedium)
{
    myCommsMedium = commsMedium;

    if (commsMedium != NULL)
    {
        myTxBuffer = myCommsMedium->getTxBuffer();
        myTxBufferSize = myCommsMedium->getTxBufferSize();
    }

    commandTimeoutPeriod = 500u; //default time in (ms) to wait for a response to a DUCI command

    commsOwnership = E_STATE_OWI_COMMS_OWNED;
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateOwi::createOwiCommands(void)
{
    myParser->addCommand(E_DPI620G_CMD_GET_VERSION_INFO, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE,  NULL,  fnGetVersionInfo, E_DPI620G_CMD_LEN_GET_VERSION_INFO , E_DPI620G_RESP_LEN_GET_VERSION_INFO , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_GET_PM620_SENSOR_INFO, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL,  fnGetPM620SensorInfo,  E_DPI620G_CMD_LEN_GET_PM620_SENSOR_INFO , E_DPI620G_RESP_LEN_GET_PM620_SENSOR_INFO , false, 0xFFFFu); 
    
    myParser->addCommand(E_DPI620G_CMD_SET_OPERATING_MODE, owiArgByteArray,  E_OWI_BYTE, E_OWI_BYTE, NULL, fnSetOperatingMode,   E_DPI620G_CMD_LEN_SET_OPERATING_MODE, E_DPI620G_RESP_LEN_SET_OPERATING_MODE , false, 0xFFFFu); 
    
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

}

void DCommsStateOwi::initialise(void)
{
}

/**
 * @brief   Suspend state machine
 * @param   void
 * @retval  void
 */
void DCommsStateOwi::suspend(void)
{
    commsOwnership = E_STATE_OWI_COMMS_REQUESTED;

    while (commsOwnership == (eStateOwiComms_t)E_STATE_OWI_COMMS_REQUESTED)
    {
        //wait until request has been processed
        sleep(100u);
    }
}

/**
 * @brief   Resume state machine
 * @param   void
 * @retval  void
 */
void DCommsStateOwi::resume(void)
{
    commsOwnership = E_STATE_OWI_COMMS_OWNED;
}

eStateOwi_t DCommsStateOwi::run(void)
{
    return E_STATE_OWI_READ;
}

void DCommsStateOwi::cleanup(void)
{
}

void DCommsStateOwi::clearRxBuffer(void) //Temporarily overriden - all comms has own buffer which base class could clear
{
    if (myCommsMedium != NULL)
    {
        myCommsMedium->clearRxBuffer();
    }
}


bool DCommsStateOwi::write(uint8_t *buf,uint8_t bufLen)  //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;
    uint8_t cmdLength = 0u;

    if (myCommsMedium != NULL)
    {
        memcpy(myTxBuffer,buf,static_cast<uint32_t>(bufLen));
        successFlag = myParser->CalculateAndAppendCheckSum((uint8_t*)myTxBuffer, (uint32_t) bufLen,(uint32_t*) &cmdLength);

        if (successFlag == true)
        {
            successFlag = myCommsMedium->write((uint8_t*)myTxBuffer,(uint32_t)cmdLength);
        }
    }

    return successFlag;
}

bool DCommsStateOwi::query(uint8_t *cmdBuf,uint8_t cmdLen, uint8_t **pRecvBuf, uint8_t recvLen)
{
    bool successFlag = false;
   
    return successFlag;
}

bool DCommsStateOwi::waitForCommand(uint8_t **pBuf)
{
    bool successFlag = false;
    uint32_t numOfBytesRead = 0u;
    uint32_t reponseLength =0u;   
    eOwiCommandType_t cmdType;
    sOwiError_t owiError;
    owiError.value = 1u;
    sOwiCommand_t **element = NULL;
    if (myCommsMedium != NULL)
    {
        successFlag = myCommsMedium->read(pBuf,  CMD_HEADER_SIZE, &numOfBytesRead, commandTimeoutPeriod);
        if((true == successFlag) && (numOfBytesRead == CMD_HEADER_SIZE))
        {
          successFlag = myParser->ValidateCheckSum(*pBuf,CMD_HEADER_SIZE);
          if(true == successFlag)
          {
              successFlag = myParser->getHandleToCommandProperties((**pBuf)& OWI_CMD_MASK,*element);
              if(true == successFlag)
              {
                  cmdType = getCommandType(**pBuf);
                  if((eOwiCommandType_t)E_OWI_CMD_READ == cmdType)
                  {
                       owiError = myParser->slaveParse((*element)->command, (uint8_t*)myTxBuffer, &reponseLength);
                       if(0u == owiError.value )
                       {
                         myParser->CalculateAndAppendCheckSum((uint8_t*)myTxBuffer,reponseLength,&reponseLength);
                         successFlag = write((uint8_t*)myTxBuffer,(uint8_t)reponseLength);     
                       }
                       else
                       {
                         //Falied to send response
                         successFlag = false;
                       }              
                  }
                  else if((eOwiCommandType_t)E_OWI_CMD_WRITE == cmdType)
                  {
                       sendAck((*element)->command);
                       numOfBytesRead = 0u;
                       myParser->getResponseLength((*element)->command, &reponseLength);
                       successFlag = myCommsMedium->read(pBuf,  reponseLength, &numOfBytesRead, commandTimeoutPeriod);
                       if((true == successFlag) && (numOfBytesRead == reponseLength))
                       {
                          successFlag = myParser->ValidateCheckSum(*pBuf,numOfBytesRead);
                          if(true == successFlag)
                          {
                               owiError = myParser->slaveParse((*element)->command,*pBuf, &reponseLength);
                               if(0u == owiError.value )
                               {
                                 // Command Execution Success
                                 sendAck((*element)->command);   
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

    return successFlag;
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
    return cmdType;
    
 }
