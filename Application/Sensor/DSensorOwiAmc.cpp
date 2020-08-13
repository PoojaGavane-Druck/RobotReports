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
* @file     DSensorDuciRTD.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     17 April 2020
*
* @brief    The RTD-Interface (RS485) sensor base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensorOwiAmc.h"
#include "DOwiParse.h"
#include "string.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define REO_1 0x40u     
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
// This time is for the sensor, uncomment for use - Makarand - TODO */
//const uint32_t singleSampleTimeoutPeriod = 400u;
const uint32_t singleSampleTimeoutPeriod = 10000u;
const uint8_t lowSupplyVoltageWarning = 0X81u;
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSensorOwiAmc class constructor
 * @param   comms is the reference to the comms instance for this sensor
 * @retval  void
 */
DSensorOwiAmc::DSensorOwiAmc(OwiInterfaceNo_t interfaceNumber)
: DSensorOwi(interfaceNumber)
{
	myCalSamplesRequired = 5u;        //number of cal samples at each cal point for averaging
        myBridgeDiffChannelSampleRate = E_ADC_SAMPLE_RATE_6_875_HZ;
        myTemperatureSampleRate = E_ADC_SAMPLE_RATE_6_875_HZ;
        mySamplingMode = E_AMC_SENSOR_SAMPLING_TYPE_SINGLE;
        myTemperatureSamplingRatio = E_AMC_SENSOR_SAMPLE_RATIO_1;
}



/**
 * @brief   Initialisation function
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensorOwiAmc::initialise()
{
    eSensorError_t sensorError = DSensorOwi::initialise();
    
    return sensorError;
}

/**
 * @brief   Initialisation function
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensorOwiAmc::readIdentity(void)
{
    eSensorError_t sensorError;
    sensorError = sendQuery(E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER);
    return sensorError;
}

/**
 * @brief   Create Owi command set - the common commands - specific to PM620 AMC sensor
 * @param   void
 * @return  void
 */
void DSensorOwiAmc::createOwiCommands(void)
{
    
    myParser->addCommand(E_AMC_SENSOR_CMD_READ_COEFFICIENTS, owiArgAmcSensorCoefficientsInfo,  E_OWI_BYTE, E_OWI_HEX_ASCII, NULL, fnGetCoefficientsData,   0u, 8192u, false, 0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_READ_CAL_DATA, owiArgAmcSensorCalibrationInfo,   E_OWI_BYTE, E_OWI_HEX_ASCII, NULL, fnGetCalibrationData,   0u, 2048u,  false, 0xFFFFu);  
    
    myParser->addCommand(E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING, owiArgRawAdcCounts, E_OWI_BYTE, E_OWI_BYTE, fnGetSample, NULL,   7u, 8u,  true, 0xFFFFu);   
    
    myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_BOOTLOADER_VER, owiArgString, E_OWI_BYTE, E_OWI_ASCII, fnGetBootloaderVersion, NULL,  0u, 0u, true,  0xFFFFu);  
    
    myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER,owiArgString, E_OWI_BYTE, E_OWI_ASCII, fnGetApplicationVersion, NULL, 0u, 17u, true,  0xFFFFu);  
    
    //myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER,owiArgString, E_OWI_BYTE, E_OWI_ASCII, fnGetApplicationVersion, NULL, 0u, 0u, true,  0xFFFFu);  
    
    myParser->addCommand(E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE, owiArgRawAdcCounts, E_OWI_BYTE, E_OWI_BYTE, fnGetSample, NULL,  5u, 8u, true,  0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW, owiArgByteValue, E_OWI_BYTE, E_OWI_BYTE, NULL, NULL, 0u,  2u,true,   0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_CHECKSUM, owiArgByteValue, E_OWI_BYTE, E_OWI_BYTE, NULL, NULL, 1u, 2u, true,  0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_SET_ZER0, owiArgString, E_OWI_HEX_ASCII, E_OWI_BYTE, NULL, NULL,  4u, 2u,true,   0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_GET_ZER0,  owiArgValue, E_OWI_BYTE, E_OWI_HEX_ASCII, fnGetZeroOffsetValue,  NULL, 0u, 4u, true,  0xFFFFu);   //read sensor error status
   
}

/*
 * @brief   Send query command Owi sensor
 * @param   command string
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::set(uint8_t cmd, 
                                  uint8_t *cmdData, 
                                  uint32_t cmdDataLength)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;    
    uint8_t *buffer;   
    uint32_t responseLength;
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength;
    uint32_t index = 0u;
    myTxBuffer[0] =  OWI_SYNC_BIT | OWI_TYPE_BIT | cmd;
   
     //prepare the message for transmission
     myParser->CalculateAndAppendCheckSum( myTxBuffer, 1u, &cmdLength);  
     
     myParser->getResponseLength(cmd, &responseLength); 
     //prepare the message for transmission
     myComms->clearRxBuffer();
    
    retStatus =  myComms->write(myTxBuffer, cmdLength);
    
    if (true == retStatus)
    {
       
        retStatus =  myComms->read(&buffer, 
                                    responseLength, 
                                    &numOfBytesRead, 
                                    commandTimeoutPeriod);
    
        if(responseLength == numOfBytesRead)
        {
          retStatus = myParser->parseAcknowledgement(cmd,buffer);
          if (false == retStatus)
          {
              sensorError = E_SENSOR_ERROR_COMMAND;
          }
        }
        else
        {
          sensorError = E_SENSOR_ERROR_COMMS;
        }
      
     
    }
    else
    {
      sensorError = E_SENSOR_ERROR_COMMS;
    }
    if(E_SENSOR_ERROR_NONE == sensorError)
    {
        for(index = 0u; index < cmdDataLength; index++)
        {
           myTxBuffer[index] = cmdData[index];
        }
        //prepare the message for transmission
        myComms->clearRxBuffer();
        
        myParser->getResponseLength(cmd, &responseLength); 
        
        myParser->CalculateAndAppendCheckSum( myTxBuffer, cmdDataLength, &cmdLength);
        
        retStatus =  myComms->write(myTxBuffer, cmdLength);
        if(true == retStatus)
        {
          retStatus =  myComms->read(&buffer, 
                                     responseLength, 
                                     &numOfBytesRead, 
                                     commandTimeoutPeriod);
          if(responseLength == numOfBytesRead) 
          {
            retStatus = myParser->parseAcknowledgement(cmd,buffer);
            if (false == retStatus)
            {
                sensorError = E_SENSOR_ERROR_COMMAND;
            }
          }
          else
          {
            sensorError = E_SENSOR_ERROR_COMMS;
          }
        }
        else
        {
           sensorError = E_SENSOR_ERROR_COMMS;
        }
    }
    return sensorError;
}


/*
 * @brief   Send query command Owi sensor
 * @param   command string
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::get(uint8_t cmd)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t *buffer;   
    uint32_t responseLength;
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength;    
    myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | cmd;
   
     //prepare the message for transmission
     myParser->CalculateAndAppendCheckSum( myTxBuffer, 1u, &cmdLength);  
     
     myParser->getResponseLength(cmd, &responseLength); 
     
     myComms->clearRxBuffer();
    
    retStatus =  myComms->write(myTxBuffer, cmdLength);
    
    if (true == retStatus)
    {
       
        retStatus =  myComms->read(&buffer, 
                                   responseLength, 
                                   &numOfBytesRead, 
                                   commandTimeoutPeriod);
    
        if(responseLength == numOfBytesRead)
        {
          owiError = myParser->parse(cmd, buffer, numOfBytesRead);
          if (owiError.value != 0u)
          {
              sensorError = E_SENSOR_ERROR_COMMAND;
          }
        }
        else if(E_OWI_RESPONSE_NCK ==buffer[0] )
        {
          sensorError = E_SENSOR_ERROR_NCK;
        }
        else
        {
          sensorError = E_SENSOR_ERROR_COMMS;
        }
      
     
    }
    else
    {
      sensorError = E_SENSOR_ERROR_COMMS;
    }

    return sensorError;
}

eSensorError_t DSensorOwiAmc::getCoefficientsData(void)
{
  eSensorError_t sensorError;
  sensorError = sendQuery(E_AMC_SENSOR_CMD_READ_COEFFICIENTS);
  return sensorError;
}

eSensorError_t DSensorOwiAmc::getCalibrationData(void)
{
  eSensorError_t sensorError;
  sensorError = sendQuery(E_AMC_SENSOR_CMD_READ_CAL_DATA);
  return sensorError;
}   

eSensorError_t DSensorOwiAmc::getApplicatonVersion(void)
{
  eSensorError_t sensorError;
  sensorError = sendQuery(E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER);
  return sensorError;
}

eSensorError_t DSensorOwiAmc::getBootloaderVersion(void)
{
  eSensorError_t sensorError;
  sensorError = sendQuery(E_AMC_SENSOR_CMD_QUERY_BOOTLOADER_VER);
  return sensorError;
}


eSensorError_t DSensorOwiAmc::InitiateSampling(void)
{
   eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t *buffer;      
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength;
    uint32_t responseLength;
   
    myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING;
    myTxBuffer[1] = (E_AMC_SENSOR_BRIDGE_COUNTS_CHANNEL << 4) | (uint8_t)myBridgeDiffChannelSampleRate;
    myTxBuffer[2] = (E_AMC_SENSOR_TEMPERATURE_CHANNEL << 4) | (uint8_t)myTemperatureSampleRate;
    myTxBuffer[3] = (REO_1)|(E_AMC_SENSOR_TEMPERATURE_CHANNEL << 4) | (uint8_t)myTemperatureSamplingRatio;    
    myTxBuffer[4] = (E_AMC_SENSOR_RESERVERD1_CHANNEL << 4) | E_ADC_SAMPLE_RATE_CH_OFF;
    myTxBuffer[5] = (REO_1)|(E_AMC_SENSOR_RESERVERD1_CHANNEL << 4) | (uint8_t)myTemperatureSamplingRatio; 
    myTxBuffer[6] = (E_AMC_SENSOR_RESERVERD2_CHANNEL << 4) | E_ADC_SAMPLE_RATE_CH_OFF;
    myTxBuffer[7] = (REO_1)|(E_AMC_SENSOR_RESERVERD2_CHANNEL << 4) | (uint8_t)myTemperatureSamplingRatio; 
   
   
    //prepare the message for transmission
    myParser->CalculateAndAppendCheckSum( myTxBuffer, 8u, &cmdLength);  
    
    myParser->getResponseLength(E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING, &responseLength);
    
    myComms->clearRxBuffer();
    
    retStatus =  myComms->write(myTxBuffer, cmdLength);
    
    if(true == retStatus)
    {
        retStatus =  myComms->read(&buffer, 
                                       responseLength, 
                                       &numOfBytesRead, 
                                       singleSampleTimeoutPeriod);
        if(true == retStatus)
        {
            if(responseLength == numOfBytesRead)
            {
              owiError = myParser->parse(E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING,
                                         buffer, 
                                         numOfBytesRead);
              if (owiError.value != 0u)
              {
                  sensorError = E_SENSOR_ERROR_COMMAND;
              }
              else
              {
                isSensorSupplyVoltageLow = false;
              }
            }
            else if(lowSupplyVoltageWarning == buffer[0] )
            {
              sensorError = E_SENSOR_SUPPLY_VOLAGE_LOW;
              isSensorSupplyVoltageLow = true;
            }
            else
            {
              sensorError = E_SENSOR_ERROR_COMMS;
            }
        }
        else
        {
            sensorError = E_SENSOR_ERROR_COMMS;
        }
    }
    else
    {
        sensorError = E_SENSOR_ERROR_COMMS;
    }
    return sensorError; 
}

eSensorError_t DSensorOwiAmc::getContinousSample(void)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t *buffer;      
    uint32_t responseLength =0u;
    uint32_t numOfBytesRead = 0u;
       
    myParser->getResponseLength(E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE, &responseLength);
    
    retStatus =  myComms->read(&buffer, 
                               responseLength, 
                               &numOfBytesRead, 
                               singleSampleTimeoutPeriod);
    if(true == retStatus)
    {
        if(responseLength == numOfBytesRead)
        {
          owiError = myParser->parse(E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE, buffer, numOfBytesRead);
          if (owiError.value != 0u)
          {
              sensorError = E_SENSOR_ERROR_COMMAND;
          }
          else
          {
            isSensorSupplyVoltageLow = false;
          }
        }
        else if(lowSupplyVoltageWarning == buffer[0] )
        {
          sensorError = E_SENSOR_SUPPLY_VOLAGE_LOW;
          isSensorSupplyVoltageLow = true;
        }
        else
        {
          sensorError = E_SENSOR_ERROR_COMMS;
        }
    }
    else
    {
        sensorError = E_SENSOR_ERROR_COMMS;
    }

    return sensorError;
}


eSensorError_t DSensorOwiAmc::getSingleSample(void)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t *buffer;      
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength;
    uint32_t responseLength = 0u;
   
    myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE;
    myTxBuffer[1] = (E_AMC_SENSOR_BRIDGE_COUNTS_CHANNEL << 4) | (uint8_t)myBridgeDiffChannelSampleRate;
    myTxBuffer[2] = ((E_AMC_SENSOR_TEMPERATURE_CHANNEL << 4u) | (uint8_t)myTemperatureSampleRate);
    myTxBuffer[3] = (E_AMC_SENSOR_RESERVERD1_CHANNEL << 4u) | E_ADC_SAMPLE_RATE_CH_OFF;
    myTxBuffer[4] = (E_AMC_SENSOR_RESERVERD2_CHANNEL << 4u) | E_ADC_SAMPLE_RATE_CH_OFF;
   
    //prepare the message for transmission
        
    myParser->CalculateAndAppendCheckSum( myTxBuffer, 5u, &cmdLength);  
    
    myParser->getResponseLength(E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE, &responseLength);
    
    myComms->clearRxBuffer();
    
    retStatus =  myComms->write(myTxBuffer, cmdLength);
    
    if(true == retStatus)
    {
        retStatus =  myComms->read(&buffer, 
                                   responseLength, 
                                   &numOfBytesRead, 
                                   singleSampleTimeoutPeriod);
        if(true == retStatus)
        {
            if(responseLength == numOfBytesRead)
            {
              owiError = myParser->parse(E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE, buffer, numOfBytesRead);
              if (owiError.value != 0u)
              {
                  sensorError = E_SENSOR_ERROR_COMMAND;
              }
              else
              {
                isSensorSupplyVoltageLow = false;
              }
            }
            else if(lowSupplyVoltageWarning == buffer[0] )
            {
              sensorError = E_SENSOR_SUPPLY_VOLAGE_LOW;
              isSensorSupplyVoltageLow = true;
            }
            else
            {
              sensorError = E_SENSOR_ERROR_COMMS;
            }
        }
        else
        {
            sensorError = E_SENSOR_ERROR_COMMS;
        }
    }
    else
    {
        sensorError = E_SENSOR_ERROR_COMMS;
    }
    return sensorError; 
}


eSensorError_t DSensorOwiAmc::setCheckSum(eCheckSumStatus_t checksumStatus)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;    
    uint8_t* buffer;   
    uint32_t responseLength;
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength;
    
    myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_CHECKSUM;
   
     //prepare the message for transmission
    if(E_CHECKSUM_ENABLED == (uint8_t)checksumStatus)
    {
       responseLength = 3u;
    }
    else
    {
       responseLength = 2u;
    }
     
    myParser->CalculateAndAppendCheckSum( myTxBuffer, 1u, &cmdLength);  
         
    myComms->clearRxBuffer();
    
    retStatus =  myComms->write(myTxBuffer, cmdLength);
    
    if (true == retStatus)
    {
       
        retStatus =  myComms->read(&buffer, 
                                   responseLength, 
                                   &numOfBytesRead, 
                                   commandTimeoutPeriod);
    
        if((responseLength == numOfBytesRead) &&
           ((uint8_t)E_AMC_SENSOR_CMD_CHECKSUM == buffer[0]) && 
           ( (uint8_t)E_OWI_RESPONSE_ACC == buffer[1]))
        {
            if((uint8_t)E_CHECKSUM_ENABLED == (uint8_t)checksumStatus)
            {
              if(((uint8_t)E_AMC_SENSOR_CMD_CHECKSUM + (uint8_t)E_OWI_RESPONSE_ACC) == buffer[2])                
              {
                  myParser->setChecksumEnabled(true);
              }
              else
              {
                  sensorError = E_SENSOR_ERROR_COMMS;
              }
            }
            else if((uint8_t)E_CHECKSUM_DISABLED == (uint8_t)checksumStatus)
            {
                  myParser->setChecksumEnabled(false);
            }
            else
            {
              sensorError = E_SENSOR_ERROR_COMMS;
            } 
        }
        else if(buffer[0] == E_OWI_RESPONSE_NCK)
        {
          sensorError = E_SENSOR_ERROR_NCK;
        }
        else
        {
          sensorError = E_SENSOR_ERROR_COMMS;
        } 
    }
    else
    {
      sensorError = E_SENSOR_ERROR_COMMS;
    }
 
  return sensorError;
}

eSensorError_t DSensorOwiAmc::checkSupplyVoltage(bool &isLowSupplyVoltage)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    uint8_t *buffer;   
    uint32_t responseLength;
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength;
    
    myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW;
         
    myParser->CalculateAndAppendCheckSum( myTxBuffer, 1u, &cmdLength);  
    
    myParser->getResponseLength(E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW, &responseLength); 
       
    myComms->clearRxBuffer();
    
    retStatus =  myComms->write(myTxBuffer, cmdLength);
    
    if (true == retStatus)
    {
       
        retStatus =  myComms->read(&buffer, 
                                    responseLength, 
                                    &numOfBytesRead, 
                                    commandTimeoutPeriod);
    
        if((responseLength == numOfBytesRead) &&
           (E_AMC_SENSOR_CMD_CHECKSUM == buffer[0]) && 
           ( E_OWI_RESPONSE_ACC == buffer[1]))
        {
            retStatus = myParser->parseAcknowledgement(E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW,buffer);
            if (true == retStatus)
            {
                isSensorSupplyVoltageLow = false;
                isLowSupplyVoltage = false;
            }
            else
            {
                sensorError = E_SENSOR_ERROR_COMMS;
            }
        }
        else if(buffer[0] == lowSupplyVoltageWarning)
        {
          isSensorSupplyVoltageLow = true;
          isLowSupplyVoltage = true;
        }
        else
        {
          sensorError = E_SENSOR_ERROR_COMMS;
        } 
    }
    else
    {
      sensorError = E_SENSOR_ERROR_COMMS;
    }
 
  return sensorError;
}

eSensorError_t DSensorOwiAmc::getZeroOffsetValue(void)
{
  eSensorError_t sensorError;
  sensorError = get(E_AMC_SENSOR_CMD_GET_ZER0);
  if(E_SENSOR_ERROR_NCK == sensorError)
  {   
    float** ptrZeroOffset = NULL;
    *ptrZeroOffset = mySensorData.getHandleToZeroOffset();
    **ptrZeroOffset =  0.0f;
  }
  return sensorError;
}

eSensorError_t DSensorOwiAmc::setZeroOffsetValue(float newZeroOffsetValue)
{
  eSensorError_t sensorError;  
  uint8_t cmdData[4];
  uFloat_t fValue; 
  
  fValue.floatValue = newZeroOffsetValue;
  
  cmdData[0] = fValue.byteValue[0];
  cmdData[1] = fValue.byteValue[1];
  cmdData[2] = fValue.byteValue[2];
  cmdData[3] = fValue.byteValue[3];
  
  sensorError = set(E_AMC_SENSOR_CMD_SET_ZER0, cmdData, 4u);
  if(E_SENSOR_ERROR_NONE == sensorError)
  {    
    float** ptrZeroOffset = NULL;
    *ptrZeroOffset = mySensorData.getHandleToZeroOffset();
    **ptrZeroOffset =  0.0f;
  }
  return sensorError;
}


sOwiError_t DSensorOwiAmc::fnGetCoefficientsData(uint8_t *ptrCoeffBuff, uint32_t* paramBufSize)
{
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t** ptrSensorDataMemory = NULL; 
    *ptrSensorDataMemory= mySensorData.getHandleToSensorDataMemory();
    memcpy(*ptrSensorDataMemory, 
           ptrCoeffBuff,              
           AMC_COEFFICIENTS_SIZE);

    return owiError; 
}


sOwiError_t DSensorOwiAmc::fnGetCalibrationData(uint8_t *ptrCalBuff ,uint32_t* paramBufSize) 
{
   sOwiError_t owiError;
   owiError.value = 0u;
   uint8_t** ptrSensorCalDataMemory = NULL; 
   *ptrSensorCalDataMemory= mySensorData.getHandleToSensorCalDataMemory();
   memcpy(*ptrSensorCalDataMemory, 
          ptrCalBuff,              
          AMC_CAL_DATA_SIZE);
  

    return owiError;
  
}
sOwiError_t DSensorOwiAmc::fnGetBootloaderVersion(sOwiParameter_t *ptrOwiParam)
{
  sOwiError_t owiError;
  owiError.value = 0u;
   
  uint8_t** ptrSensorBootLoaderVersion = NULL;
  *ptrSensorBootLoaderVersion = mySensorData.getBooterVersionString();
  if (NULL == (strcpy((char*)*ptrSensorBootLoaderVersion, 
               (char const*)&ptrOwiParam->byteArray[0])))
  {
      owiError.invalid_response = 1u;
  }
  else
  {
      owiError.value = 0u;
  }
  return owiError;
}

sOwiError_t DSensorOwiAmc::fnGetApplicatonVersion(sOwiParameter_t * ptrOwiParam)
{
  sOwiError_t owiError;
  owiError.value = 0u;
   uint8_t** ptrSensorApplicationVersion = NULL;
  *ptrSensorApplicationVersion = mySensorData.getApplicationVersionString();
  if (NULL == (strcpy((char*)*ptrSensorApplicationVersion, 
               (char const*)&ptrOwiParam->byteArray[0])))
  {
      owiError.invalid_response = 1u;
  }
  else
  {
    owiError.value = 0u;
  }
  return owiError;  
}

#define TEST_CODE_FOR_ADC_SAMPLE 1
#define ADC_PRESSURE_SENSITIVITY 1.549720856530720E-06f
#define ADC_PRESS_OFFSET 1.0f                                          

sOwiError_t DSensorOwiAmc::fnGetSample(sOwiParameter_t *ptrOwiParam)
{
  sRawAdcCounts rawAdcCounts;
  eSensorError_t sensorError = E_SENSOR_ERROR_COMMS;
  sOwiError_t owiError;
  owiError.value = 0u;
  float32_t measValue = 0.0f; 
  rawAdcCounts = ptrOwiParam->rawAdcCounts;
  /*
  measValue = mySensorData.getPressureMeasurement(rawAdcCounts.channel1AdcCounts,
                                      rawAdcCounts.channel2AdcCounts);
  */
  
#if TEST_CODE_FOR_ADC_SAMPLE
  measValue = (float)(rawAdcCounts.channel1AdcCounts) 
                  * (float)(ADC_PRESSURE_SENSITIVITY);
  measValue = measValue - (float)(ADC_PRESS_OFFSET);
  
#endif
  // for test only - makarand - todo
  setMeasurement(measValue);
  
  return owiError; 
}

sOwiError_t DSensorOwiAmc::fnGetZeroOffsetValue(sOwiParameter_t *ptrOwiParam)
{
  sOwiError_t owiError;
  owiError.value = 0u;
  float** ptrZeroOffset = NULL;
  *ptrZeroOffset = mySensorData.getHandleToZeroOffset();
  **ptrZeroOffset = ptrOwiParam->floatValue;
  return owiError;
}

  /*
    
    static sOwiError_t fnCheckSupplyVoltage(void *instance, sOwiParameter_t * parameterArray);
    static sOwiError_t fnSetCheckSum(void *instance, sOwiParameter_t * parameterArray);
    static sOwiError_t fnSetZeroOffsetValue(void *instance, sOwiParameter_t * parameterArray);
*/

/*
 * @brief   Handle operating mode reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetCoefficientsData(void *instance, 
                                                 uint8_t *paramBuf, 
                                                 uint32_t* paramBufSize)
                                                 
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc*)instance;

    if (myInstance != NULL)
    {
        owiError = myInstance->fnGetCoefficientsData(paramBuf, paramBufSize);
    }
    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
}   

/*
 * @brief   Handle operating mode reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetCalibrationData(void *instance, 
                                                  uint8_t *paramBuf,
                                                  uint32_t* paramBufSize)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc*)instance;

    if (myInstance != NULL)
    {
        owiError = myInstance->fnGetCalibrationData(paramBuf, paramBufSize);
    }
    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
} 

/*
 * @brief   Handle operating mode reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetApplicationVersion(void *instance, 
                                                 sOwiParameter_t * owiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc*)instance;

    if (myInstance != NULL)
    {
        owiError = myInstance->fnGetApplicatonVersion(owiParam);
    }
    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
} 

/*
 * @brief   Handle operating mode reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetBootloaderVersion(void *instance, 
                                                 sOwiParameter_t * owiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc*)instance;

    if (myInstance != NULL)
    {
        owiError = myInstance->fnGetBootloaderVersion(owiParam);
    }
    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
} 
/*
 * @brief   Handle operating mode reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetSample(void *instance, 
                                       sOwiParameter_t *ptrOwiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc*)instance;

    if (myInstance != NULL)
    {
        owiError = myInstance->fnGetSample(ptrOwiParam);
    }
    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
} 

/*
 * @brief   Handle operating mode reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetZeroOffsetValue(void *instance, sOwiParameter_t *ptrOwiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc*)instance;

    if (myInstance != NULL)
    {
        owiError = myInstance->fnGetZeroOffsetValue(ptrOwiParam);
    }
    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
}



/*
 * @brief	Read sensor measured value
 * @param	void
 * @return	sensor error status
 */
eSensorError_t DSensorOwiAmc::measure(void)
{
   eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
   
   if(E_AMC_SENSOR_SAMPLING_TYPE_SINGLE == (uint8_t)mySamplingMode)
   {
     sensorError = getSingleSample();
   }
   else if(E_AMC_SENSOR_SAMPLINGTYPE_CONTINOUS == (uint8_t)mySamplingMode)
   {
     sensorError = getContinousSample();
   }
   else
   {
     // Sampling mode not set
   }
   
   return sensorError;

}


 eSensorError_t DSensorOwiAmc::calculatePressure(uint32_t bridgeDiffCounts,
                                                 uint32_t temperatureCounts)
 {
   return E_SENSOR_ERROR_NONE;
 }