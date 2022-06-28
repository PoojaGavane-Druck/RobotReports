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
* @file     DProductionTest.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     18 September 2020
*
* @brief    The production test functions source file
*/

#ifndef _DAMC_SENSOR_DATA_H
#define _DAMC_SENSOR_DATA_H


#include "DPressureCompensation.h"
#include "DCalibration.h"

#include "string.h"

//
#define AMC_COEFFICIENTS_SIZE       4096u
#define NUMBER_OF_CAL_DATES         10u
#define AMC_CAL_DATA_SIZE           1024u
#define MAXTC                       10u
#define MAXLIN                      24u

#define MINPOINTS 2u
#define DATANUM (3u * MAXLIN + 1u)      /* maximum number of data items per temperature */
#define DATABLOCK (MAXTC*DATANUM)   /* max number of cal data items */

#define BIPOLAR_ADC_CONV_FACTOR_AMC  0.000048828125f //4096/(5 * 2^^24)



#pragma pack(1)

typedef struct  //this is the data structure of the header info in the sensor memory
{
    uint16_t dataStart;                /* 0 */
    uint32_t serialNumber;                       /* 2 */
    uint8_t manufacturingDate[4];                /* 6 */
    uint8_t calibrationDates[4];                          /* A */
    uint8_t calTime[4];                          /* E */
    uint8_t reserved1[6];                        /* 12 */
    int16_t dataFormat;                  /* 18 0006 hex */
    int8_t brandName[16];                    /* 1A */
    int8_t brandGrade[16];                   /* 2A */
    int8_t brandUnits[10];                   /* 3A */
    int8_t brandType[8];                 /* 44 */
    int8_t brandPositiveFullScale[8];            /* 4C */
    int8_t brandNegitiveFullScale[8];         /* 54 */
    uint8_t reserved2[20];                       /* 5C */
    uint8_t manufacturingGrade[16];              /* 70 */
    uint8_t manufacturingUunits[10];              /* 80 */
    uint16_t transducerType;                      /* 8A manufacturers type: None=0, Abs=1, Gauge=2, Diff=3, SG=4, Baro=5 */
    float upperPressure;                      /* 8C manufacturers +ve FS */
    float lowerPressure;                      /* 90 manufacturers -ve FS */
    uint8_t manuf_measurand[8];                  /* 94 */
    float upperTemperature;                      /* 9C */
    float lowerTemperature;                      /* A0 */
    int16_t numbOfRanges;               /* A4 */
    float derivedRange[10];                     /* A6 */
    uint16_t numOfTcCalPoints;                  /* CE */
    uint16_t numOfLinearityCalPoints;                /* D0 */
    //------------------------------------------------------------------------------
    uint8_t reserved3[46];             /* D2 - FF 46 bytes of space */
    uint32_t headerValue;                  /* 100 - 103 02468ACE set when written */
    int16_t numberOfHeaderBytes;           /* 104 - 105 number of header bytes */
    uint8_t reserved4[2];              /* 106 - 107 */
    uint32_t headerChecksum;               /* 108 - 10C */
    uint8_t reserved5[244];            /* blank in AMC */

    //characterisation data starts here - use same format as for PACE sensor
    float characterisationData[MAXTC + 3u * MAXTC * MAXLIN]; /* 10 + 3*10*24 = 730 */
    uint32_t calDataWrite;
    uint16_t numberOfCalibrationBytes;
    uint32_t calDataChecksum;
} sAmcSensorCoefficientsData_t;

typedef union
{
    sAmcSensorCoefficientsData_t amcSensorCoefficientsData ;
    uint8_t sensorCoefficientsDataMemory[AMC_COEFFICIENTS_SIZE];
} uAmcSensorCoefficientsData_t;

typedef struct  //this is the data structure of the cal data as stored in the sensor
{
    uint32_t DAC_write;                  /* 0 (set value after DAC write) */ //
    uint16_t dacSpanValue;      /* 4  */ //
    uint16_t dacZeroValue;      /* 6   */ //
    uint32_t calWrite;          /* 8 (set value after data write)          */ //
    uint16_t numOfPressureCalPoints;    /* C (number of calibration pressures)     */ //
    float pressureCalSetPointValue[3];  /* 10 (calibration pressures used)         */ //
    float spanRatio[2];         /* 1C               */ //
    float offset[2];                /* 24                */ //
    uint32_t zeroWrite;         /* 2C (set value after zero write)         */ //
    float zeroOffset;           /* 30 (calculated zero offset in mbar)     */ //
    int8_t reverse_sign_write;      /* 34 (set to zero for reverse polarity)   */ //
    int8_t calibrationDates [NUMBER_OF_CAL_DATES][4];   /* 36 - ten sets of cal dates (DD/MM/YYYY) */ //
    int8_t neg_temp_coeff;              /* 5E   */ //
} sAmcSensorCalibrationData_t ;

typedef union
{
    sAmcSensorCalibrationData_t amcSensorCalibrationData;
    uint8_t sensorCalibrationDataMemory[AMC_CAL_DATA_SIZE];
} uAmcSensorCalibrationData_t;

struct sCalibration
{
    sDate_t Date;
    uint8_t ucNoCalPoints;                  //no of cal points
    uint8_t ucNoCalData;                    //no of straight line segments
    sCalPoint_t CalPoints[MAX_CAL_POINTS];      //cal points used
    sCalPoint_t  CalData[MAX_CAL_POINTS - 1];   //array of straight line segments
    float   dBreakPoints[MAX_CAL_POINTS - 2]; //segment breakpoints
};

typedef struct  //this is the data structure used by the application
{
    float upperPressure;
    float lowerPressure;
    float upperTemperature;
    float lowerTemperature;
    uint16_t numOfTcCalPoints;
    uint16_t numOfLinearityCalPoints;
    float temperature[MAXTC];
    uint32_t calWrite;
    uint16_t numOfPressureCalPoints;
    float pressureCalSetPointValue[3];
    float spanRatio[2];
    float offset[2];
    float zeroOffset;
    int8_t calibrationDates [NUMBER_OF_CAL_DATES][4];       /* 36 - ten sets of cal dates (DD/MM/YYYY) */ //
} sCompensationData_t;




#pragma pack()

class DAmcSensorData
{
    int32_t myBridgeCounts;
    int32_t diodeCounts;
    int32_t myTemperatureCounts;
    float bridgeVoltageInmv;
    float diodeVoltageInmv;
    float positiveFullScale;          // positive fullscale
    float negativeFullScale;          // negaive fullscale
    uint16_t transducerType;                 // transducer type
    uint16_t numOfLinearityCalPoints;     /* no of linearity cal points */
    uint16_t numOfTcCalPoints;      /* and tc cal points */
    float reverseSetpointGain;
    float reverseSetpointOffset;
    float pdcrSupplyRatio;
    float reverseSetpoint(float pressure, float temperature);



    bool isMyCoefficientsDataValid;
    bool isMyCalibrationDataValid;
    bool isItPMTERPS;

    uAmcSensorCoefficientsData_t myCoefficientsData;
    uAmcSensorCalibrationData_t  myCalibrationData;


    sSensorCal_t userCalibrationData;



    void reverseBytes(uint8_t *ptrByteBuffer, uint16_t byteBufferSize);
    void convertValueFromAppToSensorFormat(uint16_t usValue, uint16_t *ptrUsValue);
    void convertValueFromAppToSensorFormat(uint32_t data, uint32_t *ptrUiValue);
    void convertValueFromAppToSensorFormat(float data, float *ptrFloatValue);
    uint16_t convertValueFromSensorToAppFormat(uint16_t usValue);
    int16_t convertValueFromSensorToAppFormat(int16_t sValue);
    float convertValueFromSensorToAppFormat(float fValue);
    uint32_t convertValueFromSensorToAppFormat(uint32_t data);
    void convertHeaderDateFromSensorToAppFormat(uint8_t *buff, sDate_t *date);
    uint32_t convertHeaderDateByte(uint8_t b);
    int convertCalDateFromSensorToAppFormat(int8_t *dest, int8_t *src, int32_t index);
    int convertCalDateFromAppToSensorFormat(int8_t *dest, int8_t *src, int32_t index);

    void formatCalData();

public:
    sCompensationData_t compensationData;
    sSpamCubic_t newlinspam[MAXTC][MAXLIN];
    sSpamCubic_t newtcspam[MAXTC][MAXLIN];
    DAmcSensorData();

    uint8_t *getSensorDataArea();
    sAmcSensorCoefficientsData_t *GetSensorData()
    {
        return (sAmcSensorCoefficientsData_t *)&myCoefficientsData.amcSensorCoefficientsData;
    }
    uint8_t *getHandleToSensorDataMemory()
    {
        return myCoefficientsData.sensorCoefficientsDataMemory;
    }
    uint8_t *getHandleToSensorCalDataMemory()
    {
        return myCalibrationData.sensorCalibrationDataMemory;
    }
    uint8_t *getCalDataArea();
    float *getHandleToZeroOffset();

    float getZeroOffset(void);
    void setZeroOffset(float dNewZeroOffset);



    int8_t *getModelString();
    bool isPMTERPS();
    void setPMTERPS(bool IsPMTerps);

    bool validateCoefficientData();
    void validateCalData();
    void validateZeroData(float fZeroValue);

    void trashSensorInformation();
    void trashCoefficientData();
    void trashCalData();
    void trashZeroData();



    float getPositiveFullScale();
    float getNegativeFullScale();
    void getBrandMin(int8_t *brandMin);
    void getBrandMax(int8_t *brandMax);
    void getBrandType(int8_t *brandType);
    void getBrandUnits(int8_t *brandUnits);
    uint16_t muxInput;

    int16_t get_index(int16_t t, int16_t lin, int16_t data);
    float getCompensatedPressureMeasurement(float bridgeVoltage, float temperatureVoltage);
    float getPressureMeasurement(int32_t bridgeCount,
                                 int32_t temperatureCount);

    uint32_t calculateSpamfits(void);
    uint32_t float_checksum(float value);
    float CalculatePressureFromMilliVolt(uint8_t index, float x);
    float calculateTemperatureFromMilliVolt(uint8_t index, float x);

    uint16_t getTransducerType();
    uint32_t getSerialNumber();
    void getManufacturingDate(sDate_t *pManfDate);
    void getUserCalDate(sDate_t *pUserCalDate);
    void loadUserCal();
    void saveUserCal();

};

#endif // DAmcSensorData_H
