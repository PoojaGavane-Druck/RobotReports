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
* @file     DSensorOwiAmc.h
* @version  1.00.00
* @author   Nageswara & Makarand
* @date     17 April 2020
*
* @brief    The OWI sensor base class header file
*/

#ifndef __DSENSOR_OWI_AMC_H
#define __DSENSOR_OWI_AMC_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensorOwi.h"
#include "DAmcSensorData.h"
/* Defines ----------------------------------------------------------------------------------------------------------*/
#define MAX_ADC_BUFFER 8u
#define MAX_ADC_BUFFER_SHIFT 2u
/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum : uint32_t
{
    E_AMC_SENSOR_TYPE_NONE       = 0x00000000u,     //no error
    E_AMC_SENSOR_PM620           = 0x00000001u,     //PM620 Sensor
    E_AMC_SENSOR_PMTERPS         = 0x00000002u,     //PM620 TERPS
} eAmcSensorType_t;

typedef enum : uint8_t
{
    E_AMC_SENSOR_SAMPLING_TYPE_NONE       = 0u,
    E_AMC_SENSOR_SAMPLING_TYPE_SINGLE,
    E_AMC_SENSOR_SAMPLINGTYPE_CONTINOUS,
} eAmcSensorSamplingMode_t;

typedef enum : uint8_t
{
    E_AMC_SENSOR_CMD_BOOTLOADER_UPDATE       = 0X00,
    E_AMC_SENSOR_CMD_APPLICATION_UPDATE      = 0X01,
    E_AMC_SENSOR_CMD_WRITE_COEFFICIENTS      = 0X02,
    E_AMC_SENSOR_CMD_READ_COEFFICIENTS       = 0X03,
    E_AMC_SENSOR_CMD_WRITE_CAL_DATA          = 0X04,
    E_AMC_SENSOR_CMD_READ_CAL_DATA           = 0X05,
    E_AMC_SENSOR_CMD_INITIATE_CONT_SAMPLING  = 0X06,
    E_AMC_SENSOR_CMD_QUERY_BOOTLOADER_VER    = 0X07,
    E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER   = 0X08,
    E_AMC_SENSOR_CMD_REQUEST_SINGLE_SAMPLE   = 0X09,
    E_AMC_SENSOR_CMD_QUERY_SUPPLY_VOLTAGE_LOW = 0X0A,
    E_AMC_SENSOR_CMD_CHECKSUM                 = 0X0B,
    E_AMC_SENSOR_CMD_SET_ZER0                 = 0X0C,
    E_AMC_SENSOR_CMD_GET_ZER0                 = 0X0D,
    E_AMC_SENSOR_CMD_SET_GLITCH_DETECTION     = 0X0E,
    E_AMC_SENSOR_CMD_GET_GLITCH_DETECTION     = 0X0F,
    E_AMC_SENSOR_CMD_QUERY_BOOTLOADER_VER_T   = 0x17,
    E_AMC_SENSOR_CMD_QUERY_APPLICATION_VER_T  = 0x18

} eAmcSensorCommand_t;

typedef enum : uint8_t
{
    E_ADC_SAMPLE_RATE_CH_OFF               = 0X00,
    E_ADC_SAMPLE_RATE_3520_HZ              = 0X01,
    E_ADC_SAMPLE_RATE_1760_HZ              = 0X02,
    E_ADC_SAMPLE_RATE_880_HZ               = 0X03,
    E_ADC_SAMPLE_RATE_440_HZ               = 0X04,
    E_ADC_SAMPLE_RATE_220_HZ               = 0X05,
    E_ADC_SAMPLE_RATE_110_HZ               = 0X06,
    E_ADC_SAMPLE_RATE_55_HZ                = 0X07,
    E_ADC_SAMPLE_RATE_27_5_HZ              = 0X08,
    E_ADC_SAMPLE_RATE_13_75_HZ             = 0X09,
    E_ADC_SAMPLE_RATE_6_875_HZ             = 0X0F

} eAmcSensorAdcSampleRate_t;

typedef enum : uint16_t
{
    E_AMC_SENSOR_SAMPLE_RATIO_1            = 1,
    E_AMC_SENSOR_SAMPLE_RATIO_2            = 2,
    E_AMC_SENSOR_SAMPLE_RATIO_3            = 3,
    E_AMC_SENSOR_SAMPLE_RATIO_4            = 4,
    E_AMC_SENSOR_SAMPLE_RATIO_8            = 8,
    E_AMC_SENSOR_SAMPLE_RATIO_10           = 10,
    E_AMC_SENSOR_SAMPLE_RATIO_25           = 25,
    E_AMC_SENSOR_SAMPLE_RATIO_50           = 50,
    E_AMC_SENSOR_SAMPLE_RATIO_100          = 100,
    E_AMC_SENSOR_SAMPLE_RATIO_200          = 200,
    E_AMC_SENSOR_SAMPLE_RATIO_400          = 400,
    E_AMC_SENSOR_SAMPLE_RATIO_600          = 600,
    E_AMC_SENSOR_SAMPLE_RATIO_800          = 800,
    E_AMC_SENSOR_SAMPLE_RATIO_1000         = 1000,
    E_AMC_SENSOR_SAMPLE_RATIO_1500         = 1500,
    E_AMC_SENSOR_SAMPLE_RATIO_2000         = 2000

} eAmcSensorSamplingRatio_t;



typedef enum : uint32_t
{
    E_SENSOR_DISCONNECTED       = 0x00000000u,      //Sensor disconnected
    E_SENSOR_CONNECTED           = 0x00000001u,     //Sensor Connected
} eSensorConnectionStatus_t;




/* Variables --------------------------------------------------------------------------------------------------------*/


class DSensorOwiAmc : public DSensorOwi
{
private:

    eSensorConnectionStatus_t    myConnectionStatus;
    eAmcSensorAdcSampleRate_t    myBridgeDiffChannelSampleRate;
    eAmcSensorAdcSampleRate_t    myTemperatureSampleRate;
    eAmcSensorSamplingMode_t     mySamplingMode;
    eAmcSensorSamplingRatio_t    myTemperatureSamplingRatio;
    DAmcSensorData               mySensorData;


    bool isSensorSupplyVoltageLow;


    eSensorError_t set(uint8_t cmd, uint8_t *cmdData,
                       uint32_t cmdDataLen);
    eSensorError_t get(uint8_t cmd);

    eSensorError_t validateZeroOffsetValue(float zeroValue);

    eSensorError_t getSingleSample(uint32_t channelSelection);

    eSensorError_t writeLine(uint8_t *buf, uint32_t bufLen);

    eSensorError_t uploadFile(const uint8_t *imgAddress);

    eSensorError_t upgrade(const uint8_t *imageAddress);

    static sOwiError_t fnGetCoefficientsData(void *instance, uint8_t *paramBuf, uint32_t *paramBufSize);
    static sOwiError_t fnGetCalibrationData(void *instance, uint8_t *paramBuf, uint32_t *paramBufSize);
    static sOwiError_t fnGetApplicationVersion(void *instance, sOwiParameter_t *parameterArray);
    static sOwiError_t fnGetBootloaderVersion(void *instance, sOwiParameter_t *parameterArray);
    static sOwiError_t fnInitiateSampling(void *instance, sOwiParameter_t *parameterArray);
    static sOwiError_t fnGetSample(void *instance, sOwiParameter_t *parameterArray);
    static sOwiError_t fnCheckSupplyVoltage(void *instance, sOwiParameter_t *parameterArray);
    static sOwiError_t fnSetCheckSum(void *instance, sOwiParameter_t *parameterArray);
    static sOwiError_t fnSetZeroOffsetValue(void *instance, sOwiParameter_t *parameterArray);
    static sOwiError_t fnGetZeroOffsetValue(void *instance, sOwiParameter_t *parameterArray);

    uint8_t isSamplingInitiated;
    uint8_t myBuffer[250];
    uint32_t pressureAdcIndex;
    uint32_t pressureBuffer[MAX_ADC_BUFFER];
    uint32_t pressureADC;



protected:
    virtual void createOwiCommands(void);

public:
    DSensorOwiAmc(OwiInterfaceNo_t interfaceNumber);
    ~DSensorOwiAmc(void);

    virtual eSensorError_t readAppIdentity(void);
    virtual eSensorError_t readBootLoaderIdentity(void);
    virtual eSensorError_t readAppIdentityTerps(void);
    virtual eSensorError_t readBootLoaderIdentityTerps(void);
    virtual eSensorError_t initialise(void);

    virtual eSensorError_t measure(uint32_t channelSelection);

    sOwiError_t fnGetCoefficientsData(uint8_t *paramBuf, uint32_t *ptrCoeffBuff);
    sOwiError_t fnGetCalibrationData(uint8_t *paramBuf, uint32_t *ptrCalBuff);
    sOwiError_t fnGetApplicatonVersion(sOwiParameter_t *ptrOwiParam);
    sOwiError_t fnGetBootloaderVersion(sOwiParameter_t *ptrOwiParam);
    sOwiError_t fnInitiateSampling(sOwiParameter_t *ptrOwiParam);
    sOwiError_t fnGetSample(sOwiParameter_t *ptrOwiParam);
    sOwiError_t fnCheckSupplyVoltage(sOwiParameter_t *ptrOwiParam);
    sOwiError_t fnSetCheckSum(sOwiParameter_t *ptrOwiParam);
    sOwiError_t fnSetZeroOffsetValue(sOwiParameter_t *ptrOwiParam);
    sOwiError_t fnGetZeroOffsetValue(sOwiParameter_t *ptrOwiParam);

    eSensorError_t readCoefficientsData(void);
    eSensorError_t readCalibrationData(void);
    eSensorError_t readZeroData(void);
    eSensorError_t checkSupplyVoltage(bool &isLowSupplyVoltage);
    virtual eSensorError_t setCheckSum(eCheckSumStatus_t checksumStatus);

    virtual eSensorError_t upgradeFirmware(void);
    virtual eSensorError_t setZeroData(float32_t zeroVal);
    virtual eSensorError_t getZeroData(float32_t *zeroVal);
    virtual void initializeSensorInfo(void);
    virtual float32_t compensate(float32_t rawReading);

};

#endif /* __DSENSOR_DUCI_RTD_H */
