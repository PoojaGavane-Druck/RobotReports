/**
* Baker Hughes Confidential
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
* @author   Nageswara & Makarand
* @date     17 April 2020
*
* @brief    AMC sensor base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensorOwiAmc.h"
#include "DOwiParse.h"
#include "string.h"
#include "uart.h"
#include "Utilities.h"
#include "PM620TFW.h"

#include "DPV624.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define REO_1 0x40u
//In version info index details DK0351, V01.0.0
#define DKNUMBER_START_INDEX 2u
#define BUILD_START_INDEX_NT 9u
#define MAJOR_NUMBER_INDEX_NT 11U
#define MINOR_NUMBER_INDEX_NT 14u

#define BUILD_START_INDEX_T 9u
#define MAJOR_NUMBER_INDEX_T 12U
#define MINOR_NUMBER_INDEX_T 15u

#define PM_TERPS_APPLICATION 472
#define PM_TERPS_BRIDGE_RATE E_ADC_SAMPLE_RATE_13_75_HZ
#define PM_TERPS_DIODE_RATE E_ADC_SAMPLE_RATE_13_75_HZ

#define PM_APPLICATION 351
#define PM_BRIDGE_RATE E_ADC_SAMPLE_RATE_55_HZ
#define PM_DIODE_RATE E_ADC_SAMPLE_RATE_55_HZ

#define SENSOR_UPDATE_PASSWORD 0X0C

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
// This time is for the sensor, uncomment for use - Makarand - TODO */
//const uint32_t singleSampleTimeoutPeriod = 400u;
const uint32_t singleSampleTimeoutPeriod = 150u;
const uint32_t firmwareUpgradeRequestTimeoutPeriod = 20000u;
const uint32_t writeLineTimeoutPeriod = 1000u;
const uint8_t lowSupplyVoltageWarning = 0X81u;
const uint32_t bytesInLastLineOfFwFile = 12u;

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

    /* Only for testing sensor errors that are generated after some time */
    myBridgeDiffChannelSampleRate = E_ADC_SAMPLE_RATE_27_5_HZ;
    myTemperatureSampleRate = E_ADC_SAMPLE_RATE_27_5_HZ;

    /*
    myBridgeDiffChannelSampleRate = E_ADC_SAMPLE_RATE_6_875_HZ;
    myTemperatureSampleRate = E_ADC_SAMPLE_RATE_6_875_HZ;
    */
    isSamplingInitiated = (uint8_t)(0);
    mySamplingMode = E_AMC_SENSOR_SAMPLING_TYPE_SINGLE;
    myTemperatureSamplingRatio = E_AMC_SENSOR_SAMPLE_RATIO_1;

    //ToDo Added for testing by Nag
    mySerialNumber = mySensorData.getSerialNumber();
    myFsMaximum = mySensorData.getPositiveFullScale();
    myFsMinimum = mySensorData.getNegativeFullScale();
    myType = static_cast<eSensorType_t>(mySensorData.getTransducerType());

    mySensorData.getManufacturingDate(&myManufactureDate);
    mySensorData.getUserCalDate(&myUserCalDate);

    myIdentity.major = (uint32_t) 0;
    myIdentity.minor = (uint32_t) 0;
    myIdentity.build = (uint32_t) 0;

    myBlIdentity.major = (uint32_t) 0;
    myBlIdentity.minor = (uint32_t) 0;
    myBlIdentity.build = (uint32_t) 0;

    myConnectionStatus = (eSensorConnectionStatus_t)E_SENSOR_DISCONNECTED;
    isSensorSupplyVoltageLow = false;
}



/**
 * @brief   Initialisation function
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensorOwiAmc::initialise()
{
    eSensorError_t sensorError = DSensorOwi::initialise();
    myParser->setChecksumEnabled(true);
    return sensorError;
}

/**
 * @brief   read PM620 sensor application ID
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensorOwiAmc::readAppIdentity(void)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    sensorError = sendQuery(E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER);
    return sensorError;
}

/**
 * @brief   read PM620 sensor bootloader ID
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensorOwiAmc::readBootLoaderIdentity(void)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    sensorError = sendQuery(E_AMC_SENSOR_CMD_QUERY_BOOTLOADER_VER);
    return sensorError;
}

/**
 * @brief   Create Owi command set - the common commands - specific to PM620 AMC sensor
 * @param   void
 * @return  void
 */
void DSensorOwiAmc::createOwiCommands(void)
{

    myParser->addCommand(E_AMC_SENSOR_CMD_READ_COEFFICIENTS,
                         owiArgAmcSensorCoefficientsInfo,
                         E_OWI_BYTE, E_OWI_HEX_ASCII,
                         NULL,
                         fnGetCoefficientsData,
                         0u,
                         8192u,
                         false,
                         0xFFFFu);

    myParser->addCommand(E_AMC_SENSOR_CMD_READ_CAL_DATA,
                         owiArgAmcSensorCalibrationInfo,
                         E_OWI_BYTE, E_OWI_HEX_ASCII,
                         NULL,
                         fnGetCalibrationData,
                         0u,
                         2048u,
                         false,
                         0xFFFFu);

    myParser->addCommand(E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING,
                         owiArgRawAdcCounts,
                         E_OWI_BYTE, E_OWI_BYTE,
                         fnGetSample, NULL,
                         7u,
                         8u,
                         true,
                         0xFFFFu);

    myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_BOOTLOADER_VER,
                         owiArgString,
                         E_OWI_BYTE,
                         E_OWI_ASCII,
                         fnGetBootloaderVersion,
                         NULL,
                         0u,
                         16u,
                         true,
                         0xFFFFu);

    myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER,
                         owiArgString,
                         E_OWI_BYTE,
                         E_OWI_ASCII,
                         fnGetApplicationVersion,
                         NULL,
                         0u,
                         16u,
                         true,
                         0xFFFFu);

    myParser->addCommand(E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE,
                         owiArgRawAdcCounts,
                         E_OWI_BYTE, E_OWI_BYTE,
                         fnGetSample,
                         NULL,
                         5u,
                         4u,
                         true,
                         0xFFFFu);

    myParser->addCommand(E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW,
                         owiArgByteValue,
                         E_OWI_BYTE,
                         E_OWI_BYTE,
                         NULL,
                         NULL,
                         0u,
                         2u,
                         true,
                         0xFFFFu);

    myParser->addCommand(E_AMC_SENSOR_CMD_CHECKSUM,
                         owiArgByteValue,
                         E_OWI_BYTE,
                         E_OWI_BYTE,
                         NULL,
                         NULL,
                         1u,
                         2u,
                         true,
                         0xFFFFu);

    myParser->addCommand(E_AMC_SENSOR_CMD_SET_ZER0,
                         owiArgString,
                         E_OWI_HEX_ASCII,
                         E_OWI_BYTE,
                         NULL,
                         NULL,
                         8u,
                         2u,
                         true,
                         0xFFFFu);

    myParser->addCommand(E_AMC_SENSOR_CMD_GET_ZER0,
                         owiArgValue,
                         E_OWI_BYTE,
                         E_OWI_HEX_ASCII,
                         fnGetZeroOffsetValue,
                         NULL,
                         0u,
                         8u,
                         true,
                         0xFFFFu);   //read sensor error status

}

/**
 * @brief   Send query command Owi sensor
 * @param   cmd command number
 * @param   cmdData  - command string
 * @param   cmdDataLength - number of bytes in command data
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
    myParser->CalculateAndAppendCheckSum(myTxBuffer, 1u, &cmdLength);

    myParser->getResponseLength(cmd, &responseLength);
    //prepare the message for transmission
    myComms->clearRxBuffer();

    retStatus =  myComms->write(myTxBuffer, cmdLength);

    if(true == retStatus)
    {

        myComms->read(&buffer,
                      responseLength,
                      &numOfBytesRead,
                      commandTimeoutPeriod);

        if(responseLength == numOfBytesRead)
        {
            retStatus = myParser->parseAcknowledgement(cmd, buffer);

            if(false == retStatus)
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

        myParser->CalculateAndAppendCheckSum(myTxBuffer, cmdDataLength, &cmdLength);

        retStatus =  myComms->write(myTxBuffer, cmdLength);

        if(true == retStatus)
        {
            myComms->read(&buffer,
                          responseLength,
                          &numOfBytesRead,
                          commandTimeoutPeriod);

            if(responseLength == numOfBytesRead)
            {
                retStatus = myParser->parseAcknowledgement(cmd, buffer);

                if(false == retStatus)
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


/**
 * @brief   Send query command Owi sensor
 * @param   command number
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
    myParser->CalculateAndAppendCheckSum(myTxBuffer, 1u, &cmdLength);

    myParser->getResponseLength(cmd, &responseLength);

    myComms->clearRxBuffer();

    retStatus =  myComms->write(myTxBuffer, cmdLength);

    if(true == retStatus)
    {

        myComms->read(&buffer,
                      responseLength,
                      &numOfBytesRead,
                      commandTimeoutPeriod);

        if(responseLength == numOfBytesRead)
        {
            owiError = myParser->parse(cmd, buffer, numOfBytesRead);

            if(owiError.value != 0u)
            {
                sensorError = E_SENSOR_ERROR_COMMAND;
            }
        }

        else if(E_OWI_RESPONSE_NCK == buffer[0])
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

/**
 * @brief   Send  command to Owi sensor to read coefficients
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::getCoefficientsData(void)
{
    eSensorError_t sensorError;
    sensorError = sendQuery(E_AMC_SENSOR_CMD_READ_COEFFICIENTS);
    return sensorError;
}

/**
 * @brief   Send  command to Owi sensor to read coefficients
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::getZeroData(void)
{
    eSensorError_t sensorError;
    sensorError = sendQuery(E_AMC_SENSOR_CMD_GET_ZER0);
    return sensorError;
}


/**
 * @brief   Send  command to Owi sensor to read coefficients
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::setZeroData(void)
{
    eSensorError_t sensorError;
    uint8_t buffer[8];
    uint8_t index = (uint8_t)(0);

    // current zero value of the sensor - 0xc213eb85

    buffer[index] = (uint8_t)(0x0C);
    index++;
    buffer[index] = (uint8_t)(0x02);
    index++;
    buffer[index] = (uint8_t)(0x01);
    index++;
    buffer[index] = (uint8_t)(0x03);
    index++;
    buffer[index] = (uint8_t)(0x0E);
    index++;
    buffer[index] = (uint8_t)(0x0B);
    index++;
    buffer[index] = (uint8_t)(0x08);
    index++;
    buffer[index] = (uint8_t)(0x05);
    index++;

    sensorError = set(E_AMC_SENSOR_CMD_SET_ZER0, &buffer[0], 8u);
    return sensorError;
}


/**
 * @brief   Send  command to Owi sensor to read calibration data
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::getCalibrationData(void)
{
    eSensorError_t sensorError;
    sensorError = sendQuery(E_AMC_SENSOR_CMD_READ_CAL_DATA);
    return sensorError;
}

/**
 * @brief   Send  command to Owi sensor to read application version
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::getApplicatonVersion(void)
{
    eSensorError_t sensorError;
    sensorError = sendQuery(E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER);
    return sensorError;
}

/**
 * @brief   Send  command to Owi sensor to read bootloader version
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::getBootloaderVersion(void)
{
    eSensorError_t sensorError;
    sensorError = sendQuery(E_AMC_SENSOR_CMD_QUERY_BOOTLOADER_VER);
    return sensorError;
}

/**
 * @brief   Send  command to Owi sensor to initiate measure in single reading mode
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::InitiateSampling(void)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
#if 0
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t *buffer;
    uint32_t numOfBytesRead = 0u;
#endif
    uint32_t cmdLength;
    uint32_t responseLength;

    myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING;
    myTxBuffer[1] = (E_AMC_SENSOR_BRIDGE_COUNTS_CHANNEL << 4) | (uint8_t)myBridgeDiffChannelSampleRate;
    myTxBuffer[2] = (E_AMC_SENSOR_TEMPERATURE_CHANNEL << 4) | (uint8_t)myTemperatureSampleRate;
    myTxBuffer[3] = (REO_1) | (E_AMC_SENSOR_TEMPERATURE_CHANNEL << 4) | (uint8_t)myTemperatureSamplingRatio;
    myTxBuffer[4] = (E_AMC_SENSOR_RESERVERD1_CHANNEL << 4) | E_ADC_SAMPLE_RATE_CH_OFF;
    myTxBuffer[5] = (REO_1) | (E_AMC_SENSOR_RESERVERD1_CHANNEL << 4) | (uint8_t)E_ADC_SAMPLE_RATE_CH_OFF;
    myTxBuffer[6] = (E_AMC_SENSOR_RESERVERD2_CHANNEL << 4) | E_ADC_SAMPLE_RATE_CH_OFF;
    myTxBuffer[7] = (REO_1) | (E_AMC_SENSOR_RESERVERD2_CHANNEL << 4) | (uint8_t)E_ADC_SAMPLE_RATE_CH_OFF;

    //prepare the message for transmission
    myParser->CalculateAndAppendCheckSum(myTxBuffer, 8u, &cmdLength);

    myParser->getResponseLength(E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING, &responseLength);

    myComms->clearRxBuffer();

    retStatus =  myComms->write(myTxBuffer, cmdLength);

    if(true == retStatus)
    {
        isSamplingInitiated = (uint8_t)(1);
    }

#if 0

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

                if(owiError.value != 0u)
                {
                    sensorError = E_SENSOR_ERROR_COMMAND;
                }

                else
                {
                    isSensorSupplyVoltageLow = false;
                }
            }

            else if(lowSupplyVoltageWarning == buffer[0])
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

#endif
    return sensorError;
}

/**
 * @brief   Send  command to Owi sensor to initiate measure in continous mode
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::getContinousSample(void)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t *buffer;
    uint32_t responseLength = 0u;
    uint32_t numOfBytesRead = 0u;


    if((uint8_t)(0) == isSamplingInitiated)
    {
        InitiateSampling();
        isSamplingInitiated = (uint8_t)(1);
    }

    myParser->getResponseLength(E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING, &responseLength);

    retStatus =  myComms->read(&buffer,
                               responseLength,
                               &numOfBytesRead,
                               singleSampleTimeoutPeriod);

    if(true == retStatus)
    {
        if(responseLength == numOfBytesRead)
        {
            owiError = myParser->parse(E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING, buffer, numOfBytesRead);

            if(owiError.value != 0u)
            {
                sensorError = E_SENSOR_ERROR_COMMAND;
            }

            else
            {
                isSensorSupplyVoltageLow = false;
            }
        }

        else if(lowSupplyVoltageWarning == buffer[0])
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

/**
 * @brief   Send  command to Owi sensor to initiate measure in single  reading mode
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::getSingleSample(uint32_t channelSelection)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t *buffer;
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength;
    uint32_t responseLength = 0u;
    uint32_t sampleRate = (uint32_t)(0);

    uint8_t bridgeDiffChannelSampleRate = E_ADC_SAMPLE_RATE_CH_OFF;
    uint8_t temperatureSampleRate = E_ADC_SAMPLE_RATE_CH_OFF;
    uint8_t ch2SampleRate = E_ADC_SAMPLE_RATE_CH_OFF;
    uint8_t ch3SampleRate = E_ADC_SAMPLE_RATE_CH_OFF;

    /* Change sampling speed based on terps or non terps PM */

    /* Read the sampling rate requested for the new reading */
    getValue(E_VAL_INDEX_SAMPLE_RATE, &sampleRate);

    if((uint32_t)(PM_TERPS_APPLICATION) == myIdentity.dk)
    {
        myBridgeDiffChannelSampleRate = PM_TERPS_BRIDGE_RATE;
        myTemperatureSampleRate = PM_TERPS_DIODE_RATE;
        channelSelection = E_CHANNEL_1 | E_CHANNEL_2;

        if(channelSelection & (uint32_t)E_CHANNEL_0)
        {
            bridgeDiffChannelSampleRate = E_ADC_SAMPLE_RATE_CH_OFF;
            responseLength = responseLength + (uint32_t)4;
        }

        if(channelSelection & (uint32_t)E_CHANNEL_1)
        {
            temperatureSampleRate = myTemperatureSampleRate;
            responseLength = responseLength + (uint32_t)4;
        }

        if(channelSelection & (uint32_t)E_CHANNEL_2)
        {
            ch2SampleRate = myBridgeDiffChannelSampleRate;
            responseLength = responseLength + (uint32_t)6;
        }

        if(channelSelection & (uint32_t)E_CHANNEL_3)
        {
            ch3SampleRate = E_ADC_SAMPLE_RATE_CH_OFF;
            responseLength = responseLength + (uint32_t)4;
        }

        myTxBuffer[0] = (uint8_t)(0xC9);
        myTxBuffer[1] = (uint8_t)(0x20) | (uint8_t)(sampleRate);
        myTxBuffer[2] = (uint8_t)(0x1F);
        myTxBuffer[3] = (uint8_t)(0x00);
        myTxBuffer[4] = (uint8_t)(0x00);
    }

    else if((uint32_t)(PM_APPLICATION) == myIdentity.dk)
    {
        myBridgeDiffChannelSampleRate = (eAmcSensorAdcSampleRate_t)sampleRate;
        myTemperatureSampleRate = (eAmcSensorAdcSampleRate_t)sampleRate;

        if(channelSelection & (uint32_t)E_CHANNEL_0)
        {
            bridgeDiffChannelSampleRate = myBridgeDiffChannelSampleRate;
            responseLength = responseLength + (uint32_t)4;
        }

        if(channelSelection & (uint32_t)E_CHANNEL_1)
        {
            temperatureSampleRate = myTemperatureSampleRate;
            responseLength = responseLength + (uint32_t)4;
        }

        if(channelSelection & (uint32_t)E_CHANNEL_2)
        {
            ch2SampleRate = myBridgeDiffChannelSampleRate;
            responseLength = responseLength + (uint32_t)4;
        }

        if(channelSelection & (uint32_t)E_CHANNEL_3)
        {
            ch3SampleRate = myBridgeDiffChannelSampleRate;
            responseLength = responseLength + (uint32_t)4;
        }

        myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE;
        myTxBuffer[1] = (E_AMC_SENSOR_BRIDGE_COUNTS_CHANNEL << 4) | (uint8_t)bridgeDiffChannelSampleRate;
        myTxBuffer[2] = ((E_AMC_SENSOR_TEMPERATURE_CHANNEL << 4u) | (uint8_t)temperatureSampleRate);
        myTxBuffer[3] = (E_AMC_SENSOR_RESERVERD1_CHANNEL << 4u) | ch2SampleRate;
        myTxBuffer[4] = (E_AMC_SENSOR_RESERVERD2_CHANNEL << 4u) | ch3SampleRate;
    }

    else
    {
    }

    //prepare the message for transmission

    myParser->CalculateAndAppendCheckSum(myTxBuffer, 5u, &cmdLength);

    if(true == myParser->getChecksumEnabled())
    {
        responseLength = responseLength + (uint32_t)1;
    }

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

                if(owiError.value != 0u)
                {
                    sensorError = E_SENSOR_ERROR_COMMAND;
                }

                else
                {
                    isSensorSupplyVoltageLow = false;
                }
            }

            else if(lowSupplyVoltageWarning == buffer[0])
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

/**
 * @brief   Send  command to Owi sensor to enable checksum for following transactions
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::setCheckSum(eCheckSumStatus_t checksumStatus)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    uint8_t *buffer = NULL;
    uint32_t responseLength;
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength;

    myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_CHECKSUM;
    myTxBuffer[1] = checksumStatus;

    //prepare the message for transmission
    if(E_CHECKSUM_ENABLED == (uint8_t)checksumStatus)
    {
        responseLength = 3u;
    }

    else
    {
        responseLength = 2u;
    }

    myParser->CalculateAndAppendCheckSum(myTxBuffer, 2u, &cmdLength);

    myComms->clearRxBuffer();

    retStatus =  myComms->write(myTxBuffer, cmdLength);

    if(true == retStatus)
    {

        myComms->read(&buffer,
                      responseLength,
                      &numOfBytesRead,
                      commandTimeoutPeriod);

        if((responseLength == numOfBytesRead) &&
                ((uint8_t)E_OWI_RESPONSE_ACC == buffer[0]) &&
                ((uint8_t)E_AMC_SENSOR_CMD_CHECKSUM == buffer[1]))
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

/**
 * @brief   Send  command to Owi sensor to check supply voltage status
 * @param   isLowSupplyVoltage refernce variable to return supply voltage status
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::checkSupplyVoltage(bool &isLowSupplyVoltage)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    uint8_t *buffer;
    uint32_t responseLength;
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength;

    myTxBuffer[0] = OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW;

    myParser->CalculateAndAppendCheckSum(myTxBuffer, 1u, &cmdLength);

    myParser->getResponseLength(E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW, &responseLength);

    myComms->clearRxBuffer();

    retStatus =  myComms->write(myTxBuffer, cmdLength);

    if(true == retStatus)
    {

        myComms->read(&buffer,
                      responseLength,
                      &numOfBytesRead,
                      commandTimeoutPeriod);

        if((responseLength == numOfBytesRead) &&
                (E_AMC_SENSOR_CMD_CHECKSUM == buffer[0]) &&
                (E_OWI_RESPONSE_ACC == buffer[1]))
        {
            retStatus = myParser->parseAcknowledgement(E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW, buffer);

            if(true == retStatus)
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

/**
 * @brief   Send  command to Owi sensor to read zero offset value
 * @param   void
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::getZeroOffsetValue(void)
{
    eSensorError_t sensorError;
    sensorError = get(E_AMC_SENSOR_CMD_GET_ZER0);
#if 0

    if(E_SENSOR_ERROR_NCK == sensorError)
    {
        float **ptrZeroOffset = NULL;
        *ptrZeroOffset = mySensorData.getHandleToZeroOffset();
        **ptrZeroOffset =  0.0f;
    }

#endif
    return sensorError;
}

/**
 * @brief   Send  command to Owi sensor to set zero offset value
 * @param   newZeroOffsetValue
 * @return  sensor error code
 */
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
#if 0

    if(E_SENSOR_ERROR_NONE == sensorError)
    {
        float **ptrZeroOffset = NULL;
        *ptrZeroOffset = mySensorData.getHandleToZeroOffset();
        **ptrZeroOffset =  0.0f;
    }

#endif
    return sensorError;
}

/**
 * @brief   call back function for get coefficient data
 * @param   ptrCoeffBuff uint8_t buffer to return coefficient data
 * @param   paramBufSize pointer to return coefficient data length
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetCoefficientsData(uint8_t *ptrCoeffBuff, uint32_t *paramBufSize)
{
    sOwiError_t owiError;
    owiError.value = 0u;
    bool statusFlag = false;
    uint8_t *ptrSensorDataMemory = NULL;
    ptrSensorDataMemory = mySensorData.getHandleToSensorDataMemory();
    memcpy(ptrSensorDataMemory,
           ptrCoeffBuff,
           AMC_COEFFICIENTS_SIZE);
    statusFlag = mySensorData.validateCoefficientData();

    if(true == statusFlag)
    {
        mySerialNumber = mySensorData.getSerialNumber();
        mySensorData.getBrandUnits(myBrandUnits);
        myFsMaximum = mySensorData.getPositiveFullScale();
        myFsMinimum = mySensorData.getNegativeFullScale();
        myType = static_cast<eSensorType_t>(mySensorData.getTransducerType());
        mySensorData.getManufacturingDate(&myManufactureDate);
        mySensorData.getUserCalDate(&myUserCalDate);
        owiError.value = 0u;
    }

    else
    {
        owiError.value = 1u;
    }

    return owiError;
}

/**
 * @brief   call back function for get calibration data
 * @param   ptrCalBuff uint8_t buffer to return calibration data
 * @param   paramBufSize pointer to return calibration data length
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetCalibrationData(uint8_t *ptrCalBuff, uint32_t *paramBufSize)
{
    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t **ptrSensorCalDataMemory = NULL;
    *ptrSensorCalDataMemory = mySensorData.getHandleToSensorCalDataMemory();
    memcpy(*ptrSensorCalDataMemory,
           ptrCalBuff,
           AMC_CAL_DATA_SIZE);

    mySensorData.validateCalData();
    return owiError;

}

/**
 * @brief   call back function for read sensor bootloader version.
 * @param   ptrOwiParam pointer to variable to return bootloader version
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetBootloaderVersion(sOwiParameter_t *ptrOwiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;
    int8_t *endPtr;

    myBlIdentity.dk = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[DKNUMBER_START_INDEX], (char **) &endPtr, (int)10);

    if((uint32_t)(PM_TERPS_APPLICATION) == myIdentity.dk)
    {
        setManfIdentity((uint32_t)(1));
        myBlIdentity.major = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[MAJOR_NUMBER_INDEX_T], (char **)  &endPtr, (int)10);
        myBlIdentity.minor = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[MINOR_NUMBER_INDEX_T], (char **) &endPtr, (int)10);
        myBlIdentity.build = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[BUILD_START_INDEX_T], (char **) &endPtr, (int)10);
    }

    else
    {
        setManfIdentity((uint32_t)(0));
        myBlIdentity.major = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[MAJOR_NUMBER_INDEX_NT], (char **)  &endPtr, (int)10);
        myBlIdentity.minor = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[MINOR_NUMBER_INDEX_NT], (char **) &endPtr, (int)10);
        myBlIdentity.build = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[BUILD_START_INDEX_NT], (char **) &endPtr, (int)10);
    }

    return owiError;
}

/**
 * @brief   call back function for read sensor Application version.
 * @param   ptrOwiParam pointer to variable to return Application version
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetApplicatonVersion(sOwiParameter_t *ptrOwiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    int8_t *endPtr;

    myIdentity.dk = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[DKNUMBER_START_INDEX], (char **) &endPtr, (int)10);

    if((uint32_t)(PM_TERPS_APPLICATION) == myIdentity.dk)
    {
        setManfIdentity((uint32_t)(1));
        myIdentity.major = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[MAJOR_NUMBER_INDEX_T], (char **)  &endPtr, (int)10);
        myIdentity.minor = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[MINOR_NUMBER_INDEX_T], (char **) &endPtr, (int)10);
        myIdentity.build = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[BUILD_START_INDEX_T], (char **) &endPtr, (int)10);
    }

    else
    {
        setManfIdentity((uint32_t)(0));
        myIdentity.major = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[MAJOR_NUMBER_INDEX_NT], (char **)  &endPtr, (int)10);
        myIdentity.minor = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[MINOR_NUMBER_INDEX_NT], (char **) &endPtr, (int)10);
        myIdentity.build = (uint32_t) strtol((char const *)&ptrOwiParam->byteArray[BUILD_START_INDEX_NT], (char **) &endPtr, (int)10);
    }


    return owiError;
}

/**
 * @brief   call back function for get single masurement value.
 * @param   ptrOwiParam pointer to variable to return bridge counts
 * @return  sensor error code
 */
sOwiError_t DSensorOwiAmc::fnGetSample(sOwiParameter_t *ptrOwiParam)
{
    sRawAdcCounts rawAdcCounts;
    sOwiError_t owiError;

    owiError.value = 0u;
    float32_t measValue = 0.0f;
    float32_t zeroValue = 0.0f;

    rawAdcCounts = ptrOwiParam->rawAdcCounts;

    measValue = mySensorData.getPressureMeasurement((int32_t)(rawAdcCounts.channel1AdcCounts),
                (int32_t)(rawAdcCounts.channel2AdcCounts));

    //measValue = measValue + zeroValue;
    setValue(E_VAL_INDEX_VALUE, measValue);

    return owiError;
}

/**
 * @brief   call back function to get sensor zero offset value.
 * @param   ptrOwiParam pointer to variable to return sensor zero offset value
 * @return  sensor error code
 */

sOwiError_t DSensorOwiAmc::fnGetZeroOffsetValue(sOwiParameter_t *ptrOwiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

#if 0
    float **ptrZeroOffset = NULL;
    *ptrZeroOffset = mySensorData.getHandleToZeroOffset();

    **ptrZeroOffset = mySensorData.getZeroOffset();
#endif
    mySensorData.setZeroOffset(ptrOwiParam->floatValue);
    return owiError;
}

/**
 * @brief   OWI call back function read coefficient data coand
 * @param   instance is a pointer to the FSM state instance
 * @param   paramBuf  pointer to uint8_t buffer to return coefficient data
 * @param   paramBufSize  buffer size
 * @retval  error status
 */
sOwiError_t DSensorOwiAmc::fnGetCoefficientsData(void *instance,
        uint8_t *paramBuf,
        uint32_t *paramBufSize)

{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc *)instance;

    if(myInstance != NULL)
    {
        owiError = myInstance->fnGetCoefficientsData(paramBuf, paramBufSize);
    }

    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
}

/**
 * @brief   OWI call back function read calibration data coand
 * @param   instance is a pointer to the FSM state instance
 * @param   paramBuf  pointer to uint8_t buffer to return calibration data
 * @param   paramBufSize  buffer size
 * @retval  error status
 */
sOwiError_t DSensorOwiAmc::fnGetCalibrationData(void *instance,
        uint8_t *paramBuf,
        uint32_t *paramBufSize)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc *)instance;

    if(myInstance != NULL)
    {
        owiError = myInstance->fnGetCalibrationData(paramBuf, paramBufSize);
    }

    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
}

/**
 * @brief   OWI call back function to read sensor firmware application version
 * @param   instance is a pointer to the FSM state instance
 * @param   owiParam  pointer to variable to return application version
 * @retval  error status
 */
sOwiError_t DSensorOwiAmc::fnGetApplicationVersion(void *instance,
        sOwiParameter_t *owiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc *)instance;

    if(myInstance != NULL)
    {
        owiError = myInstance->fnGetApplicatonVersion(owiParam);
    }

    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
}

/**
 * @brief   OWI call back function to read sensor firmware bootloader version
 * @param   instance is a pointer to the FSM state instance
 * @param   owiParam  pointer to variable to return bootloader version
 * @retval  error status
 */
sOwiError_t DSensorOwiAmc::fnGetBootloaderVersion(void *instance,
        sOwiParameter_t *owiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc *)instance;

    if(myInstance != NULL)
    {
        owiError = myInstance->fnGetBootloaderVersion(owiParam);
    }

    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
}

/**
 * @brief   OWI call back function to read measurement value from sensor
 * @param   instance is a pointer to the FSM state instance
 * @param   owiParam  pointer to variable to return measured bridge counts
 * @retval  error status
 */
sOwiError_t DSensorOwiAmc::fnGetSample(void *instance,
                                       sOwiParameter_t *ptrOwiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc *)instance;

    if(myInstance != NULL)
    {
        owiError = myInstance->fnGetSample(ptrOwiParam);
    }

    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
}

/**
 * @brief   OWI call back function to read sensor zero offset value
 * @param   instance is a pointer to the FSM state instance
 * @param   owiParam  pointer to variable to return sensor zero offset value
 * @retval  error status
 */
sOwiError_t DSensorOwiAmc::fnGetZeroOffsetValue(void *instance, sOwiParameter_t *ptrOwiParam)
{
    sOwiError_t owiError;
    owiError.value = 0u;

    DSensorOwiAmc *myInstance = (DSensorOwiAmc *)instance;

    if(myInstance != NULL)
    {
        owiError = myInstance->fnGetZeroOffsetValue(ptrOwiParam);
    }

    else
    {
        owiError.unhandledMessage = 1u;
    }

    return owiError;
}

/**
 * @brief   high level function for meausre
 * @param   channelSelection channel number to measure
 * @retval  error status
 */
eSensorError_t DSensorOwiAmc::measure(uint32_t channelSelection)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;

    if(E_AMC_SENSOR_SAMPLING_TYPE_SINGLE == (uint8_t)mySamplingMode)
    {
        sensorError = getSingleSample(channelSelection);
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



/**
 * @brief   upgrades the sensor firmware
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSensorOwiAmc::upgradeFirmware(void)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    upgrade((const uint8_t *)pm620tFirmware);
    return sensorError;
}

/**
 * @brief   writes  one line to sensor
 * @param   pointer to the char buffer which contains data to be written
 * @param   number of bytes to write
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::writeLine(uint8_t *buf, uint32_t bufLen)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    uint8_t *buffer;
    uint32_t numOfBytesRead = 0u;
    uint32_t responseLength = 2u;

    memcpy(myTxBuffer, buf, (uint32_t)bufLen);

    myComms->clearRxBuffer();

    retStatus =  myComms->write(myTxBuffer, bufLen);

    if(true == retStatus)
    {
        retStatus =  myComms->read(&buffer,
                                   responseLength,
                                   &numOfBytesRead,
                                   writeLineTimeoutPeriod);

        if(true == retStatus)
        {
            if(responseLength == numOfBytesRead)
            {
                if(E_OWI_RESPONSE_ACC != buffer[0])
                {
                    sensorError = E_SENSOR_ERROR_COMMAND;
                }
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

/**
 * @brief   reads inter hex file line by line and write to the sensor
 * @param   pointer to the char buffer which points to new image in internal flash
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::uploadFile(const uint8_t *imgAddress)
{

    uint32_t nackCount = (uint32_t)0;
    uint32_t completedByteCount = (uint32_t)0;
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    uint32_t percentComplete = 0u;

    bool bFinished = false;

    while(((eSensorError_t)E_SENSOR_ERROR_NONE == sensorError) && (!bFinished))
    {
        uint32_t bytesToWrite = fetchString((const uint8_t *)&imgAddress[completedByteCount],
                                            (uint8_t *)myBuffer);

        if(bytesToWrite >= (uint32_t)10)
        {
            nackCount = (uint32_t)0;

            do
            {
                sensorError = writeLine(myBuffer, bytesToWrite);

                if(bytesInLastLineOfFwFile == bytesToWrite)
                {
                    if(strncmp((char const *)myBuffer, ":00000001FF\n", 12u) == (int32_t)0)
                    {
                        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_RESET);
                        bFinished = TRUE;
                        break;
                    }
                }

                if(sensorError != E_SENSOR_ERROR_NONE)
                {
                    if(++nackCount > (uint32_t)4)
                    {
                        sensorError = E_SENSOR_UPDATE_NACK_ERROR;
                    }
                }

            }
            while(((eSensorError_t)E_SENSOR_ERROR_NONE != sensorError) && (nackCount < (uint32_t)5));

            completedByteCount = completedByteCount + bytesToWrite; // Need to add 1. to consider \n and \r

            // Count the percentage
            percentComplete = completedByteCount * 100u;
            percentComplete = percentComplete / 32996u;
            PV624->setPmUpgradePercentage(percentComplete);
        }
    }

    return sensorError;
}

/**
 * @brief   upgrades new image into sensor
 * @param   pointer to the char buffer which points to new image in internal flash
 * @return  sensor error code
 */
eSensorError_t DSensorOwiAmc::upgrade(const uint8_t *imageAddress)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    bool retStatus = false;
    uint8_t *buffer;
    uint32_t numOfBytesRead = 0u;
    uint32_t cmdLength = 0u;
    uint32_t responseLength = 2u;// After receiving firmware upgrade command sensor disables the checksum and hence response length is 1

    //sensorError =  uploadFile(imageAddress);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_Delay(100u);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_SET);

    myTxBuffer[0] =  OWI_SYNC_BIT | OWI_TYPE_BIT | E_AMC_SENSOR_CMD_APPLICATION_UPDATE;

    //prepare the message for transmission

    myParser->CalculateAndAppendCheckSum(myTxBuffer, 1u, &cmdLength);

    myComms->clearRxBuffer();

    retStatus = myComms->write(myTxBuffer, cmdLength);

    if(true == retStatus)
    {
        retStatus =  myComms->read(&buffer,
                                   responseLength,
                                   &numOfBytesRead,
                                   firmwareUpgradeRequestTimeoutPeriod);

        if(true == retStatus)
        {
            if(responseLength == numOfBytesRead)
            {
                if(E_OWI_RESPONSE_ACC != buffer[0])
                {
                    sensorError = E_SENSOR_ERROR_COMMAND;
                }
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

    // Send Sensor upgrade password
    if((eSensorError_t)E_SENSOR_ERROR_NONE == sensorError)
    {
        myTxBuffer[0] = (uint8_t) SENSOR_UPDATE_PASSWORD;
        //prepare the message for transmission
        responseLength = 2u;
        myComms->clearRxBuffer();
        retStatus =  myComms->write(myTxBuffer, 1u);

        if(true == retStatus)
        {
            /* Here there will be a requirement to power cycle the PM terps if the version is V1.01.02 or lower
            Versions ahead of this will have the upgrade fix included
            powerCycleSensor();
            */

            retStatus =  myComms->read(&buffer,
                                       responseLength,
                                       &numOfBytesRead,
                                       firmwareUpgradeRequestTimeoutPeriod);

            if(true == retStatus)
            {
                if(responseLength == numOfBytesRead)
                {
                    if(E_OWI_RESPONSE_ACC != buffer[0])
                    {
                        sensorError = E_SENSOR_ERROR_COMMAND;
                    }
                }
            }

            else
            {
                sensorError = E_SENSOR_ERROR_COMMS;
            }
        }

        sleep(100u);

        if((eSensorError_t)E_SENSOR_ERROR_NONE == sensorError)
        {
            sleep(100u);
            sensorError =  uploadFile(imageAddress);
        }
    }

    return sensorError;
}
