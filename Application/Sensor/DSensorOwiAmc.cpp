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
#include "DParseMaster.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

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
 * @brief   Create Owi command set - the common commands - specific to PM620 AMC sensor
 * @param   void
 * @return  void
 */
void DSensorOwiAmc::createOwiCommands(void)
{
    DSensorOwi::createDuciCommands();

    myParser->addCommand(E_AMC_SENSOR_CMD_READ_COEFFICIENTS, argAmcCoefficientsInfo,  E_OWI_BYTE, E_OWI_HEX_ASCII, getCoefficientsData,   0u, 8192u,   0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_READ_CAL_DATA, argAmcCalibrationInfo,   E_OWI_BYTE, E_OWI_HEX_ASCII, getCalibrationData,   0u, 2048u,   0xFFFFu);  
    
    myParser->addCommand(E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING, argRawAdcCounts, E_OWI_BYTE, E_OWI_HEX_ASCII, initiateContinuosSamplingRate,   7u, 16u,   0xFFFFu);   
    
    myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_BOOTLOADER_VER, argString, E_OWI_BYTE, E_OWI_ASCII, getBootloaderVersion,   0u, 16u,   0xFFFFu);  
    
    myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER,argString, E_OWI_BYTE, E_OWI_ASCII, getApplicatonVersion,  0u. 16u,   0xFFFFu);  
    
    myParser->addCommand(E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE, argRawAdcCounts, E_OWI_BYTE, E_OWI_HEX_ASCII, getSingleSample,   7u,16u,   0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW, argByteValue, E_OWI_BYTE, E_OWI_HEX_ASCII, checkSupplyVoltage, 0u  2,   0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_CHECKSUM,  E_OWI_BYTE, E_OWI_HEX_ASCII, setCheckSum,  1u, 2u,   0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_SET_ZER0,  E_OWI_BYTE, E_OWI_HEX_ASCII, setZeroOffsetValue,   4u, 2u,   0xFFFFu); 
    
    myParser->addCommand(E_AMC_SENSOR_CMD_GET_ZER0,  E_OWI_BYTE, E_OWI_HEX_ASCII, getZeroOffsetValue,   0u, 4u   0xFFFFu);   //read sensor error status
   
}




/*
 * @brief	Read sensor measured value
 * @param	void
 * @return	sensor error status
 */
eSensorError_t DSensorOwiAmc::measure(void)
{
    //need to check the sensor status first before asking for measurement
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    sOwiError_t owiError;
    owiError.value = 0u;
    char *buffer;      
    uint32_t NumOfBytesRead = 0;
    uint32_t cmdLength;
   
   
    myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE;
    myTxBuffer[1] = (E_AMC_SENSOR_BRIDGE_COUNTS_CHANNEL << 4) |myBridgeDiffChannelSampleRate;
    myTxBuffer[2] = (E_AMC_SENSOR_TEMPERATURE_CHANNEL << 4) | myTemperatureSampleRate;
    myTxBuffer[3] = (E_AMC_SENSOR_RESERVERD1_CHANNEL << 4) | E_ADC_SAMPLE_RATE_CH_OFF;
    myTxBuffer[4] = (E_AMC_SENSOR_RESERVERD2_CHANNEL << 4) | E_ADC_SAMPLE_RATE_CH_OFF;
   
   
   
    //prepare the message for transmission
    myParser->CalculateAndAppendCheckSum( myTxBuffer, 5, cmdLength);  
    
    getResponseLength(E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE, responseLength)
    //read serial number
    if (myComms->query(myTxBuffer, cmdLength, &buffer, responseLength, commandTimeoutPeriod) == true)
    {
        owiError = myParser->parse(buffer);

        //if this transaction is ok, then we can use the received value
        if (owiError.value != 0u)
        {
            sensorError = E_SENSOR_ERROR_COMMAND;
        }
    }
    else
    {
        sensorError = E_SENSOR_ERROR_COMMS;
    }

    return sensorError;

    

    return sensorError;
}



eSensorError_t DSensorOwiAmc::getResponseLength(uint8_t cmd, uint32_t &responseLength)
{
    uint32_t responseDataLength = 0;
    
    bool retStatus = false;
    
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    
    retStatus = myParser->getResponseDataLength(cmd, responseDataLength);
    if(true == retStatus)
    {
      if( true == myParser->getChecksumEnabled() )
      {
        switch(cmd)
        {
          case E_AMC_SENSOR_CMD_READ_COEFFICIENTS:
          case E_AMC_SENSOR_CMD_READ_CAL_DATA:
            responseLength = responseDataLength ;
            break;
          default:
            responseLength = responseDataLength + 1;
          break;
          
        }
      }
      else
      {
        responseLength = responseDataLength;
      }
    }
    else
    {
      sensorError = E_SENSOR_ERROR_PARAMETER;
    }
    return sensorError;
}

/*
 * @brief   Send query command Owi sensor
 * @param   command string
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::sendQuery(int8_t cmd)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;

    sOwiError_t owiError;
    owiError.value = 0u;
    char *buffer;
    
    uint32_t responseLength;
    uint32_t cmdLength;
    myTxBuffer[0] = cmd  | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE;
   
    //prepare the message for transmission
    myParser->CalculateAndAppendCheckSum( myTxBuffer, 1, cmdLength);  
    
    getResponseLength(cmd, responseLength)
    //read serial number
    if (myComms->query(myTxBuffer, cmdLength, &buffer, responseLength, commandTimeoutPeriod) == true)
    {
        owiError = myParser->parse(buffer);

        //if this transaction is ok, then we can use the received value
        if (owiError.value != 0u)
        {
            sensorError = E_SENSOR_ERROR_COMMAND;
        }
    }
    else
    {
        sensorError = E_SENSOR_ERROR_COMMS;
    }

    return sensorError;
}

/*
 * @brief   Send query command Owi sensor
 * @param   command string
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::set(int8_t cmd, uint8_t *cmdData, uint32_t cmdDataLen)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    sOwiError_t owiError;
    owiError.value = 0u;
    char *buffer;   
    uint32_t ackLength;
    uint32_t NumOfBytesRead = 0;
    uint32_t cmdLength;
    uint32_t index = 0;
    myTxBuffer[0] = cmd | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE;
   
     //prepare the message for transmission
     myParser->CalculateAndAppendCheckSum( myTxBuffer, 1, cmdLength);  
     
     getResponseLength(cmd, responseLength) 
     //prepare the message for transmission
     myComms->clearRxBuffer();
    
    retStatus =  myComms->write(myTxBuffer, cmdLength);
    
    if (true == retStatus)
    {
       
      retStatus =  myComms->read(&buffer, 
                                 ackLength, 
                                 NumOfBytesRead, 
                                 commandTimeoutPeriod);
    
        if(ackLength == NumOfBytesRead)
        {
          owiError = myParser->parseAcknowledgement(cmd,buffer);
          if (owiError.value != 0u)
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
        for(index = 0; index < cmdDataLength; index++)
        {
           myTxBuffer[index] = cmdData[index]
        }
        myParser->CalculateAndAppendCheckSum( myTxBuffer, cmdDataLength, cmdLength);
        
        retStatus =  myComms->write(myTxBuffer, cmdLength);
        if(true == retStatus)
        {
          retStatus =  myComms->read(&buffer, 
                                     ackLength, 
                                     NumOfBytesRead, 
                                     commandTimeoutPeriod);
          if(ackLength == NumOfBytesRead)
          {
            owiError = myParser->parseAcknowledgement(cmd,buffer);
            if (owiError.value != 0u)
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
  eSensorError_t sensorError;
  sensorError = sendQuery(E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING);
  return sensorError;
}

eSensorError_t DSensorOwiAmc::getSingleSample(void)
{
  eSensorError_t sensorError;
  sensorError = sendQuery(E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE);
  return sensorError;
}

eSensorError_t DSensorOwiAmc::checkSupplyVoltage(void)
{
  eSensorError_t sensorError;
  sensorError = sendQuery(E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW);
  return sensorError;
}

eSensorError_t DSensorOwiAmc::setCheckSum(eCheckSumStatus_t checksumStatus)
{
  eSensorError_t sensorError;
  uint8_t cmdData[2];
  cmdData[0] = checksumStatus;
  sensorError = set(E_AMC_SENSOR_CMD_CHECKSUM, cmdData, 1);
  return sensorError;
}

eSensorError_t DSensorOwiAmc::getZeroOffsetValue(void)
{
  eSensorError_t sensorError;
  sensorError = sendQuery(E_AMC_SENSOR_CMD_GET_ZER0);
  return sensorError;
}

eSensorError_t DSensorOwiAmc::setZeroOffsetValue(float newZeroOffsetValue)
{
  eSensorError_t sensorError;
  uint8_t *ptrByte;
  uint8_t cmdData[4];
  uFloat_t fValue; 
  
  fValue.floatValue = newZeroOffsetValue;
  
  cmdData[0] = fValue.byteValue[0];
  cmdData[1] = fValue.byteValue[1];
  cmdData[2] = fValue.byteValue[2];
  cmdData[3] = fValue.byteValue[3];
  
  sensorError = set(E_AMC_SENSOR_CMD_SET_ZER0, cmdData, 4);
  
  return sensorError;
}
   