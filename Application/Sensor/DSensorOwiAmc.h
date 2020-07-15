/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DSensorOwi.h
* @version  1.00.00
* @author   P. Nageswara Rao
* @date     17 April 2020
*
* @brief    The OWI sensor base class header file
*/

#ifndef __DSENSOR_OWI_AMC_H
#define __DSENSOR_OWI_AMC_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensorOwi.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define AMC_COEFFICIENTS_SIZE       4096
#define NUMBER_OF_CAL_DATES         10
#define AMC_CAL_DATA_SIZE           1024
#define MAXTC                       10
#define MAXLIN                      24
/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum : uint32_t
{
    E_AMC_SENSOR_TYPE_NONE       = 0x00000000u,		//no error
    E_AMC_SENSOR_PM620           = 0x00000001u,		//PM620 Sensor
    E_AMC_SENSOR_PMTERPS         = 0x00000002u,		//PM620 TERPS   
} eAmcSensorType_t;

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
    E_AMC_SENSOR_CMD_GET_GLITCH_DETECTION     = 0X0F
    
} eAmcSensorCommand_t;


typedef enum : uint8_t
{
    E_AMC_SENSOR_RESPONSE_ACC              = 0X86,		
    E_AMC_SENSOR_RESPONSE_NCK              = 0X95,
    E_AMC_SENSOR_LOW_SUPPLY_VOLTAGE_WAR    = 0X81,
} eAmcSensorResponse_t;

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

typedef enum : uint8_t
{
    E_AMC_SENSOR_BRIDGE_COUNTS_CHANNEL     = 0X00,		
    E_AMC_SENSOR_TEMPERATURE_CHANNEL       = 0X01,
    E_AMC_SENSOR_RESERVERD1_CHANNEL        = 0X02,
    E_AMC_SENSOR_RESERVERD2_CHANNEL        = 0X03,
} eAmcSensorChannel_t;

typedef enum : uint8_t
{
    E_CHECKSUM_DISABLED       = 0x0u,		//Checksum Disabled 
    E_CHECKSUM_ENABLED         = 0x1u,		//Checksum Enabled
} eCheckSumStatus_t;

typedef enum : uint32_t
{
    E_SENSOR_DISCONNECTED       = 0x00000000u,		//Sensor disconnected
    E_SENSOR_CONNECTED           = 0x00000001u,		//Sensor Connected
} eSensorConnectionStatus_t;

typedef union
{
    
    struct AmcSensorCoefficientsData //this is the data structure of the header info in the sensor memory
    {
	uint16_t dataStart;		           /* 0 */
	uint32_t serialSumber;                       /* 2 */
	uint8_t manufacturingDate[4];                /* 6 */
	uint8_t calDate[4];                          /* A */
	uint8_t calTime[4];                          /* E */
	uint8_t reserved1[6];                        /* 12 */
	int16_t dataFormat;		             /* 18 0006 hex */
	int8_t brandName[16];		             /* 1A */
	int8_t brandGrade[16];		             /* 2A */
	int8_t brandUnits[10];		             /* 3A */
	int8_t brandType[8];			     /* 44 */
	int8_t brandPositiveFullScale[8];            /* 4C */
	int8_t brandNegitiveFullScale[8];	      /* 54 */
	uint8_t reserved2[20];                       /* 5C */
	uint8_t manufacturingGrade[16];              /* 70 */
	uint8_t manufacturingUunits[10];              /* 80 */
	int16_t transducerType;		              /* 8A manufacturers type: None=0, Abs=1, Gauge=2, Diff=3, SG=4, Baro=5 */
	float upperPressure;		              /* 8C manufacturers +ve FS */
	float lowerPressure;		              /* 90 manufacturers -ve FS */
	uint8_t manuf_measurand[8];                  /* 94 */
	float upperTemperature;	                     /* 9C */
	float lowerTemperature;	                     /* A0 */
	int16_t numbOfRanges;			    /* A4 */
	float derivedRange[10];	                    /* A6 */
	int16_t tc_pts;				    /* CE */
	int16_t lin_pts;			     /* D0 */
	//------------------------------------------------------------------------------
	uint8_t reserved3[46];			   /* D2 - FF 46 bytes of space */
	uint32_t headerValue;		           /* 100 - 103 02468ACE set when written */    
	int16_t numberOfHeaderBytes;		   /* 104 - 105 number of header bytes */    
	uint8_t reserved4[2];			   /* 106 - 107 */    
	uint32_t headerChecksum;			   /* 108 - 10C */
	uint8_t reserved5[244];			   /* blank in AMC */    

	//characterisation data starts here - use same format as for PACE sensor 
	float characterisationData[MAXTC + 3*MAXTC*MAXLIN];   /* 10 + 3*10*24 = 730 */ 
        uint32_t caldataWrite;	
	uint16_t numberOfCalibrationBytes;
	uint32_t calibrationDataChecksum;
      };
      uint8_t sensorCoefficientsDataMemory[AMC_COEFFICIENTS_SIZE];
} sAmcSensorCoefficientsData_t;

typedef union
{
    struct AmcSensorCalibrationData //this is the data structure of the cal data as stored in the sensor
    {
	uint32_t DAC_write;	                 /* 0 (set value after DAC write) */ //
	uint16_t dacSpanValue;			/* 4  */ //
	uint16_t dacZeroValue;			/* 6   */ //             
	uint32_t cal_write;					/* 8 (set value after data write)          */ //
	uint16_t cal_number;					/* C (number of calibration pressures)     */ //
	float cal_pressure[3];				/* 10 (calibration pressures used)         */ //
	float span_ratio[2];				/* 1C                                      */ //
	float offset[2];					/* 24                                      */ //
	uint32_t zero_write;					/* 2C (set value after zero write)         */ //
	float zero_offset;					/* 30 (calculated zero offset in mbar)     */ //
	int8_t reverse_sign_write;			/* 34 (set to zero for reverse polarity)   */ //
	int8_t cal_dates [NUMBER_OF_CAL_DATES][4];	/* 36 - ten sets of cal dates (DD/MM/YYYY) */ //
	int8_t neg_temp_coeff;				/* 5E   */ //
    };

}sAmcSensorCalibrationData_t;



/* Variables --------------------------------------------------------------------------------------------------------*/

class DSensorOwiAmc : public DSensorOwi
{
private:
    sAmcSensorCoefficientsData_t myCoefficientsData;
    sAmcSensorCalibrationData_t  myCalibrationData;    
    eSensorConnectionStatus_t    myConnectionStatus;
    eAmcSensorAdcSampleRate_t    myBridgeDiffChannelSampleRate;
    eAmcSensorAdcSampleRate_t    myTemperatureSampleRate;
    float   zeroOffsetValue;
    eSensorError_t validateZeroOffsetValue(float zeroValue);
    eSensorError_t getSingleSample();

    static sOwiError_t fnGetCoefficientsData(void *instance, sOwiParameter_t * parameterArray);
    static sOwiError_t fgetCalibrationData(void *instance, sOwiParameter_t * parameterArray);    
    static sOwiError_t fnApplicatonVersion(void *instance, sOwiParameter_t * parameterArray);
    static sOwiError_t fnBootloaderVersion(void *instance, sOwiParameter_t * parameterArray);
    static sOwiError_t fnInitiateSampling(void *instance, sOwiParameter_t * parameterArray);
    static sOwiError_t fnGetSingleSample(void *instance, sOwiParameter_t * parameterArray);    
    static sOwiError_t fnCheckSupplyVoltage(void *instance, sOwiParameter_t * parameterArray);
    static sOwiError_t fnSetCheckSum(void *instance, sOwiParameter_t * parameterArray);
    static sOwiError_t fnSetZeroOffsetValue(void *instance, sOwiParameter_t * parameterArray);
    static sOwiError_t fnGetZeroOffsetValue(void *instance, sOwiParameter_t * parameterArray);
   
protected:
    virtual void createOwiCommands(void);

public:
    DSensorOwiAmc(OwiInterfaceNo_t interfaceNumber);

    virtual eSensorError_t initialise(void);
    virtual eSensorError_t measure(void);

    virtual eSensorError_t calStartSampling(void);
    virtual uint32_t getSampleCount(void);
//    virtual eSensorError_t setCalPoint(float32_t value);
//    virtual eSensorError_t calAbort(void);
//    virtual eSensorError_t calAccept(void);

    void setOperatingMode(uint32_t mode);
    uint32_t getOperatingMode(void);

    eSensorError_t readSampleCount(void);

    virtual eSensorError_t readFullscaleAndType(void);
    virtual eSensorError_t readNegativeFullscale(void);

    eSensorError_t readOperatingMode(void);
    eSensorError_t writeOperatingMode(uint32_t mode);

    static sOwiError_t fnGetCoefficientsData(sOwiParameter_t * parameterArray);
    static sOwiError_t fgetCalibrationData(sOwiParameter_t * parameterArray);    
    static sOwiError_t fnApplicatonVersion(sOwiParameter_t * parameterArray);
    static sOwiError_t fnBootloaderVersion(sOwiParameter_t * parameterArray);
    static sOwiError_t fnInitiateSampling(sOwiParameter_t * parameterArray);
    static sOwiError_t fnGetSingleSample(sOwiParameter_t * parameterArray);    
    static sOwiError_t fnCheckSupplyVoltage(sOwiParameter_t * parameterArray);
    static sOwiError_t fnSetCheckSum(sOwiParameter_t * parameterArray);
    static sOwiError_t fnSetZeroOffsetValue(sOwiParameter_t * parameterArray);
    static sOwiError_t fnGetZeroOffsetValue(sOwiParameter_t * parameterArray);
    
    eSensorError_t getCoefficientsData(void);
    eSensorError_t getCalibrationData(void);   
    eSensorError_t getApplicatonVersion(void);
    eSensorError_t getBootloaderVersion(void);
    eSensorError_t InitiateSampling(void);
    eSensorError_t getSingleSample(void);    
    eSensorError_t checkSupplyVoltage(void);
    eSensorError_t setCheckSum(eCheckSumStatus_t checksumStatus);
    eSensorError_t getZeroOffsetValue(void);
    eSensorError_t setZeroOffsetValue(float newZeroOffsetValue);
   
    
    
    eSensorError_t getResponseLength(uint32_t &responseLength);
    eAmcSensorType_t getAmcSensorType(void);
    eSensorError_t initiateContinuosSamplingRate(void);

};

#endif /* __DSENSOR_DUCI_RTD_H */
