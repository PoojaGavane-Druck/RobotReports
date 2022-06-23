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
* @file     DAmcSensorData.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     18 September 2020
*
* @brief    The AMC Sensor data functions source file
*/

#include "DAmcSensorData.h"
#include "uart.h"
#include "Utilities.h"
#include <string>


#define T 0                // temperature
#define P 1                // pressure
#define VB 2               // bridge voltage
#define VC 3               // common mode voltage

#define DATAFORMAT 3
#define DATAFORMAT_MULTIRANGE 5
#define ENABLE_CONVERSION_FROM_SENSOR_TO_APP_FORMAT

/**
 * @brief   DAmcSensorData class constructor
 * @param   void
 * @retval  void
 */
DAmcSensorData::DAmcSensorData()
{
    memset((uint8_t *)&myCoefficientsData.sensorCoefficientsDataMemory[0],
           0,
           AMC_COEFFICIENTS_SIZE);
    memset((uint8_t *)&myCalibrationData.sensorCalibrationDataMemory[0],
           0,
           AMC_CAL_DATA_SIZE);
    memset((uint8_t *)&userCalibrationData,
           0,
           sizeof(sSensorCal_t));
    isMyCoefficientsDataValid = false;
    isMyCalibrationDataValid = false;
    myBridgeCounts = 0;
    diodeCounts = 0;
    myTemperatureCounts = 0;
    bridgeVoltageInmv = 0.0f;
    diodeVoltageInmv = 0.0f;
    positiveFullScale = 0.0f;
    negativeFullScale = 0.0f;
    transducerType = 0u;
    numOfLinearityCalPoints = 0u;
    numOfTcCalPoints = 0u;
    reverseSetpointGain = 0.0f;
    reverseSetpointOffset = 0.0f;
    pdcrSupplyRatio = 0.0f;
    isItPMTERPS = false;
    //ToDo: Added by Nag for testing
    myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[0] = 15u;
    myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[1] = 8u;
    myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[2] = 20u;
    myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[3] = 20u;

    myCoefficientsData.amcSensorCoefficientsData.calibrationDates[0] = 16u;
    myCoefficientsData.amcSensorCoefficientsData.calibrationDates[1] = 8u;
    myCoefficientsData.amcSensorCoefficientsData.calibrationDates[2] = 20u;
    myCoefficientsData.amcSensorCoefficientsData.calibrationDates[3] = 20u;

    myCoefficientsData.amcSensorCoefficientsData.serialNumber = 10101010u;
    myCoefficientsData.amcSensorCoefficientsData.transducerType = E_SENSOR_TYPE_PRESS_GAUGE;
    myCoefficientsData.amcSensorCoefficientsData.upperPressure = -1000.0f;
    myCoefficientsData.amcSensorCoefficientsData.lowerPressure = 25000.0f;

    compensationData.upperPressure = myCoefficientsData.amcSensorCoefficientsData.upperPressure ;
    compensationData.lowerPressure = myCoefficientsData.amcSensorCoefficientsData.lowerPressure ;

    compensationData.zeroOffset = 0.0f;

    muxInput = 0u;
}

/**
* @brief    To get sensor zero offset value
* @param    void
* @return   float returns sensors zero offset value
*/
float32_t DAmcSensorData::getZeroOffset(void)
{
    return myCalibrationData.amcSensorCalibrationData.zeroOffset;
    //return (float32_t)compensationData.zeroOffset;
}

/**
* @brief    To set sensor zero offset value
* @param    dNewZeroOffset new zero offset value
* @return   void
*/
void DAmcSensorData::setZeroOffset(float32_t dNewZeroOffset)
{
    compensationData.zeroOffset = (float32_t)dNewZeroOffset;

    //save reversed-bytes version too, as that is from where the
    //zero function will get the bytes to write to sensor
    convertValueFromAppToSensorFormat((float32_t)dNewZeroOffset,
                                      (float32_t *)&myCalibrationData.amcSensorCalibrationData.zeroOffset);
}

/**
* @brief    To get sensor manufacturing date
* @param    pointer to date structure to return manufacturing date
* @return   void
*/
void DAmcSensorData::getManufacturingDate(sDate_t *pManfDate)
{
    pManfDate->day = myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[0];
    pManfDate->month = myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[1];
    pManfDate->year = ((uint32_t)(myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[2]) * 100u) + myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[3];
}

/**
* @brief    To get sensor calibration date
* @param    pointer to date structure to return calibration date
* @return   void
*/
void DAmcSensorData::getUserCalDate(sDate_t *pUserCalDate)
{
    pUserCalDate->day = (uint32_t)compensationData.calibrationDates[0][0];
    pUserCalDate->month = (uint32_t)compensationData.calibrationDates[0][1];
    pUserCalDate->year = ((uint32_t)compensationData.calibrationDates[0][2] * 100u) + (uint32_t)compensationData.calibrationDates[0][3];
}

/**
* @brief    To check PM620 is TERPs or Not
* @param    void
* @return   returns true if PM620 is TERPs, false if PM620 is non TERPs
*/
bool DAmcSensorData::isPMTERPS()
{
    return isItPMTERPS;
}

/**
* @brief    sets isItPMTERPS variable with new value
* @param    bIsTerps
* @return   void
*/
void DAmcSensorData::setPMTERPS(bool bIsTerps)
{
    isItPMTERPS = bIsTerps;
}

/**
* @brief    initialize the coefficient data with Zero
* @param    void
* @return   void
*/
void DAmcSensorData::trashCoefficientData()
{
    //write over local copy of data so next validation fails unless data is
    //re-read correctly from the sensor
    myCoefficientsData.amcSensorCoefficientsData.headerValue = 0u;
    myCoefficientsData.amcSensorCoefficientsData.calDataWrite = 0u;
    myCoefficientsData.amcSensorCoefficientsData.headerChecksum = 0u;
    myCoefficientsData.amcSensorCoefficientsData.calDataChecksum = 0u;
}

/**
* @brief    initialize sensor information with Zero
* @param    void
* @return   void
*/
void DAmcSensorData::trashSensorInformation()
{
    //clear info etc
    myBridgeCounts = 0;
    myTemperatureCounts = 0;
    diodeCounts = 0;
    bridgeVoltageInmv = 0.0f;
    diodeVoltageInmv = 0.0f;
    positiveFullScale = 0.0f;
    negativeFullScale = 0.0f;
    transducerType = 0u;
    numOfLinearityCalPoints = 0u;
    numOfTcCalPoints = 0u;
    reverseSetpointGain = 0.0f;
    reverseSetpointOffset = 0.0f;
    pdcrSupplyRatio = 0.0f;


    isMyCoefficientsDataValid = false;
    isMyCalibrationDataValid = false;

    compensationData.upperPressure = 0.0f;
    compensationData.lowerPressure = 0.0f;
}

/**
* @brief    Validates coefficients data
* @param    void
* @return   returns true if data is valid, false if data is invalid
*/
bool DAmcSensorData::validateCoefficientData()
{
    isMyCoefficientsDataValid = false;

    if(convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.headerValue) == 0x02468ACEu)     /* this is set once written */
    {
        uint16_t usSize = (convertValueFromSensorToAppFormat((uint16_t)myCoefficientsData.amcSensorCoefficientsData.numberOfHeaderBytes)) / 2u;

        if(usSize <= AMC_COEFFICIENTS_SIZE)
        {
            uint32_t ulChecksum = 0u;
            uint8_t ucDataByteHi;
            uint8_t ucDataByteLo;

            //calculate header checksum
            for(uint16_t i = 0u; i < usSize; i++)
            {
                //convert data to 2 ascii hex characters
                ucDataByteHi = myCoefficientsData.sensorCoefficientsDataMemory[i] >> 4u;
                ucDataByteLo = myCoefficientsData.sensorCoefficientsDataMemory[i] & 0x0Fu;

                ucDataByteHi += (ucDataByteHi > 9u ? 'A' - 10u : '0');
                ucDataByteLo += (ucDataByteLo > 9u ? 'A' - 10u : '0');

                ulChecksum += (uint32_t)ucDataByteHi;
                ulChecksum += (uint32_t)ucDataByteLo;
            }

            ulChecksum %= 10000u;

            //now verify that data is good
            uint32_t ulStoredChecksum = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.headerChecksum);

            if(ulChecksum == ulStoredChecksum)
            {
                isMyCoefficientsDataValid = true;
            }
        }
    }

    isMyCalibrationDataValid = false;

    if(convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.calDataWrite) == 0x13579BDFu)
    {
        uint32_t ulChecksum = calculateSpamfits();

        uint32_t ulStoredChecksum = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.calDataChecksum);

        if(ulChecksum == ulStoredChecksum)
        {
            isMyCalibrationDataValid = true;
        }
    }

    return isMyCoefficientsDataValid;
}

/**
* @brief    initialize the calibration data with Zero
* @param    void
* @return   void
*/
void DAmcSensorData::trashCalData()
{
    //write over local copy of data so next validation fails unless data is
    //re-read correctly from the sensor
    myCalibrationData.amcSensorCalibrationData.calWrite = 0u;
}

/**
* @brief    Validates calibration data
* @param    void
* @return   returns true if data is valid, false if data is invalid
*/
void DAmcSensorData::validateCalData()
{
    int32_t index = 0;
    // check whether cal data from pressure standard has been saved
    compensationData.calWrite = convertValueFromSensorToAppFormat(myCalibrationData.amcSensorCalibrationData.calWrite);

    if(compensationData.calWrite == 0x89ABCDEFu)
    {
        // read in data from pressure standard calibration

        // read value
        compensationData.numOfPressureCalPoints = convertValueFromSensorToAppFormat(myCalibrationData.amcSensorCalibrationData.numOfPressureCalPoints);

        // read in calibration pressures
        for(index = 0; index < 3; index++)
        {
            compensationData.pressureCalSetPointValue[index] = convertValueFromSensorToAppFormat(myCalibrationData.amcSensorCalibrationData.pressureCalSetPointValue[index]);
        }

        // read in span ratios
        for(index = 0; index < 2; index++)
        {
            compensationData.spanRatio[index] = convertValueFromSensorToAppFormat(myCalibrationData.amcSensorCalibrationData.spanRatio[index]);
        }

        // read in offset values
        for(index = 0; index < 2; index++)
        {
            compensationData.offset[index] = convertValueFromSensorToAppFormat(myCalibrationData.amcSensorCalibrationData.offset[index]);
        }
    }

    else    // no standard calibration data so apply no correction
    {
        compensationData.numOfPressureCalPoints = 0u;
        compensationData.spanRatio[0] = 1.0f;
        compensationData.offset[0] = 0.0f;
    }

    for(index = 0; index < static_cast<int16_t>(NUMBER_OF_CAL_DATES); index++)
    {
        convertCalDateFromSensorToAppFormat(compensationData.calibrationDates[index], myCalibrationData.amcSensorCalibrationData.calibrationDates[index], index);
    }

    // will be ff when empty/new
    // could have also checked for 17, the check value
    if(myCalibrationData.amcSensorCalibrationData.calibrationDates[0][0] == -1)
    {
        // get date from header
        sDate_t hdrDate;

        convertHeaderDateFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.calibrationDates, &hdrDate);

        int8_t *pCalDate = compensationData.calibrationDates[0];
        pCalDate[0] = static_cast<int8_t>(hdrDate.day);
        pCalDate[1] = static_cast<int8_t>(hdrDate.month);
        pCalDate[2] = static_cast<int8_t>(hdrDate.year / 100u);
        pCalDate[3] = static_cast<int8_t>(hdrDate.year % 100u);

    }
}

/**
* @brief    loads calibration data
* @param    void
* @return   void
*/
void DAmcSensorData::loadUserCal()
{
    //copy the data read from the sensor into the local calibration data structure
    userCalibrationData.numPoints = (uint8_t)compensationData.numOfPressureCalPoints;

    if(userCalibrationData.numPoints < 3u)
    {
        userCalibrationData.numPoints = 1u;
    }

    else
    {
        userCalibrationData.numPoints = static_cast<uint32_t>(compensationData.numOfPressureCalPoints - 1u);
    }

    userCalibrationData.breakpoint[0] = (float32_t)compensationData.pressureCalSetPointValue[1];

    for(uint8_t i = 0u; i < 2u; i++)
    {
        float32_t spanRatio = (float32_t)compensationData.spanRatio[i];

        userCalibrationData.segments[i].m = spanRatio;

        userCalibrationData.segments[i].c = (float32_t)compensationData.offset[i];
    }

    for(uint8_t i = 0u; i < 3u; i++)
    {
        userCalibrationData.calPoints[i].x = (float32_t)compensationData.pressureCalSetPointValue[i];
        userCalibrationData.calPoints[i].y = 0.0f;
    }

    int8_t *pCalDate = compensationData.calibrationDates[0];
    userCalibrationData.calDate.day = static_cast<uint8_t>(pCalDate[0]);
    userCalibrationData.calDate.month = static_cast<uint8_t>(pCalDate[1]);
    userCalibrationData.calDate.year = (uint32_t)(static_cast<uint32_t>(pCalDate[2]) * 100u) + static_cast<uint32_t>(pCalDate[3]);
}

/**
* @brief    save calibration data
* @param    void
* @return   void
*/
void DAmcSensorData::saveUserCal()
{
    //copy the local calibration data structure to compensationData
    compensationData.numOfPressureCalPoints = (uint16_t)userCalibrationData.numPoints;

    for(uint8_t i = 0u; i < 2u; i++)
    {

        float32_t fGain = (float32_t)userCalibrationData.segments[i].m;

        compensationData.spanRatio[i] = fGain;

        compensationData.offset[i] = (float32_t)userCalibrationData.segments[i].c;
    }

    compensationData.pressureCalSetPointValue[1] = (float32_t)userCalibrationData.breakpoint[0];

    for(uint8_t i = 0u; i < 3u; i++)
    {
        compensationData.pressureCalSetPointValue[i] = (float32_t)userCalibrationData.calPoints[i].x;
    }

    //userCalibrationData.ucNoCalData = (unsigned char)(myCalibrationData.amcSensorCalibrationData.cal_number - 1);

    int8_t *pCalDate;

    // shuffle down the previous cal dates
    // alternatively have 9 and make the start pos all ff's
    for(uint8_t i = NUMBER_OF_CAL_DATES - 1u; i > 0u; i--)
    {
        memcpy(compensationData.calibrationDates[i],
               compensationData.calibrationDates[i - 1u ],
               sizeof(compensationData.calibrationDates[i]));
    }

//ToDo: Need to call RTC API
#if 0
    SYSTEMTIME sSystemTime;
    GetClock(sSystemTime);

    userCalibrationData.Date.Day = sSystemTime.wDay;
    userCalibrationData.Date.Month = sSystemTime.wMonth;
    userCalibrationData.Date.Year = sSystemTime.wYear;
#endif
    pCalDate = compensationData.calibrationDates[0];
    pCalDate[0] = static_cast<int8_t>(userCalibrationData.calDate.day);
    pCalDate[1] = static_cast<int8_t>(userCalibrationData.calDate.month);
    pCalDate[2] = static_cast<int8_t>(userCalibrationData.calDate.year / 100u);
    pCalDate[3] = static_cast<int8_t>(userCalibrationData.calDate.year % 100u);

    //mark cal data as good
    compensationData.calWrite = 0x89ABCDEFu;

    //clear zero offset
    compensationData.zeroOffset = 0.0f;

    //Write data in raw format for sensor's storage area
    formatCalData();
}

/**
* @brief    format sensor data from application to sensor
* @param    void
* @return   void
*/
void DAmcSensorData::formatCalData()
{
    uint32_t index = 0u;
    convertValueFromAppToSensorFormat(compensationData.calWrite, (uint32_t *)&myCalibrationData.amcSensorCalibrationData.calWrite);
    convertValueFromAppToSensorFormat(compensationData.numOfPressureCalPoints, (uint16_t *)&myCalibrationData.amcSensorCalibrationData.numOfPressureCalPoints);

    // calibration pressures
    for(index = 0u; index < 3u; index++)
    {
        convertValueFromAppToSensorFormat((float32_t)compensationData.pressureCalSetPointValue[index],
                                          (float32_t *)&myCalibrationData.amcSensorCalibrationData.pressureCalSetPointValue[index]);
    }

    // span ratios
    for(index = 0u; index < 2u; index++)
    {
        convertValueFromAppToSensorFormat((float32_t)compensationData.spanRatio[index],
                                          (float32_t *)&myCalibrationData.amcSensorCalibrationData.spanRatio[index]);
    }

    // offset values
    for(index = 0u; index < 2u; index++)
    {
        convertValueFromAppToSensorFormat((float32_t)compensationData.offset[index],
                                          (float32_t *)&myCalibrationData.amcSensorCalibrationData.offset[index]);
    }

#if 1
    // zero offset - not used so just clear both to zero
    convertValueFromAppToSensorFormat(0.0f,
                                      (float *)&myCalibrationData.amcSensorCalibrationData.zeroOffset);
    convertValueFromAppToSensorFormat(0u,
                                      (float *)&myCalibrationData.amcSensorCalibrationData.zeroWrite);
#else

    convertValueFromAppToSensorFormat(0.0f,
                                      ((float *)&myCalibrationData +
                                       offsetof(uAmcSensorCalibrationData_t, amcSensorCalibrationData.zeroOffset));
                                      convertValueFromAppToSensorFormat(0u,
                                              (float *)&myCalibrationData +
                                              offsetof(uAmcSensorCalibrationData_t, amcSensorCalibrationData.zeroWrite));


#endif

    //CALDATE
    // write in here (all of them)
    for(index = 0u; index < NUMBER_OF_CAL_DATES; index++)
    {
        convertCalDateFromAppToSensorFormat(myCalibrationData.amcSensorCalibrationData.calibrationDates[index],
                                            compensationData.calibrationDates[index],
                                            static_cast<int32_t>(index));
    }
}


/**
* @brief    initiate sensor offset with zero
* @param    void
* @return   void
*/
void DAmcSensorData::trashZeroData()
{
}
/*********************************************************************************************************************/
//SUPPRESS: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma("diag_suppress=Pm046")
/*********************************************************************************************************************/

/**
* @brief    validate sensor's zero offset value
* @param    fZeroValueFromSensor
* @return   void
*/
void DAmcSensorData::validateZeroData(float fZeroValueFromSensor)
{
    myCalibrationData.amcSensorCalibrationData.zeroOffset = fZeroValueFromSensor; //save it, but this is not really necessary

    //value in sensor has reversed-bytes, so undo this for application data structure
    float fZeroValue = convertValueFromSensorToAppFormat(myCalibrationData.amcSensorCalibrationData.zeroOffset);

    //guard against illegal values
    //if ((fabs(fZeroValue) < FLT_MIN)  (fabs(fZeroValue) > FLT_MAX) || (ISNAN(fZeroValue)))
    if((fabs(fZeroValue) < FLT_MIN) || (ISNAN(fZeroValue)))
    {
        fZeroValue = 0.0f;
    }

    else if(fabs(fZeroValue) > FLT_MAX)
    {
        fZeroValue = 0.0f;
    }

    else
    {
        /* Do Nothing*/
    }

    compensationData.zeroOffset = (float32_t)fZeroValue;
}

/*********************************************************************************************************************/
//RESTORE: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma("diag_default=Pm046")
/*********************************************************************************************************************/


/**
* @brief    gets the Sensor Data Area
* @param    void
* @return   returns sensor data area address
*/
uint8_t *DAmcSensorData::getSensorDataArea()
{
    return myCoefficientsData.sensorCoefficientsDataMemory;
}

/**
* @brief    gets the Sensor calibration Data Area
* @param    void
* @return   returns sensor calibration data area address
*/
uint8_t *DAmcSensorData::getCalDataArea()
{
    return myCalibrationData.sensorCalibrationDataMemory;
}

/**
* @brief    gets the Sensor's Zero offset variable address
* @param    void
* @return   returns Sensor's Zero offset variable address
*/
float *DAmcSensorData::getHandleToZeroOffset()
{
    return (float *)&myCalibrationData.amcSensorCalibrationData.zeroOffset;
}

/**
* @brief    calculate pressure measurement from bridge counts and teperature counts
* @param    bridgeCounts
* @param    temperatureCounts
* @return   returns Sensor's Zero offset variable address
*/
float DAmcSensorData::getPressureMeasurement(int32_t bridgeCounts,
        int32_t temperatureCounts)
{

    if((int32_t)(0XFFFFFFFFu) != bridgeCounts)
    {
        myBridgeCounts = bridgeCounts;
    }

    if((int32_t)(0XFFFFFFFFu) != temperatureCounts)
    {
        myTemperatureCounts = temperatureCounts;
    }

    float32_t norm_Vb = (float32_t)myBridgeCounts * BIPOLAR_ADC_CONV_FACTOR_AMC;
    float32_t norm_Vd = (float32_t)myTemperatureCounts * BIPOLAR_ADC_CONV_FACTOR_AMC;

    // calculate pressure
    return getCompensatedPressureMeasurement(norm_Vb, norm_Vd);
}




/**
* @brief get_index - calculates index of data item
* @param               short t - temperature index
* @param               short lin linearity index
* @param               short data
*                      data 0 - temperature
*                      data 1 - pressure
*                      data 2 - bridge
*                      data 3 - cmode
* @return  - returnd index to float number
*/

int16_t DAmcSensorData::get_index(int16_t t, int16_t lin, int16_t data)
{
    int16_t index;
    index = t * (static_cast<int16_t>(compensationData.numOfLinearityCalPoints) * 3 + 1);// start of data

    if(0 == data)               // temperature
    {
        index = index;   // only 1 temperature per block
    }

    else
    {
        index = (index + (lin * 3) + data);
    }

    return index;
}

/**
* @brief compensate - calculates pressure
*
* @param  float bridge voltage
* @param  float diode voltage
* @param  - compensated pressure reading in mbar (float)
*/
float DAmcSensorData::getCompensatedPressureMeasurement(float bridgeVoltage, float diodeVoltage)
{
    // cubic fit
    float pressureReading;
    float t1, t2, t3, p1, p2, p3;
    int16_t i, n;

    float tcBuffer[MAXTC];

    if(numOfTcCalPoints >= 3u)      // at least 3 temperature points available
    {
        for(i = static_cast<int16_t>(0); i < static_cast<int16_t>(numOfTcCalPoints); i++)
        {
            // calculate expected diode voltages at each calibration
            // temperature, given the measured bridge voltage
            tcBuffer[i] = calculateTemperatureFromMilliVolt((uint8_t)i, bridgeVoltage);
        }

        i = static_cast<int16_t>(0);
        n = static_cast<int16_t>(numOfTcCalPoints) - static_cast<int16_t>(1);

        // find which tcBuffer values the measured diode voltage
        // lies between i.e. which calibration temperatures the
        // sensor is between. i represents the value immediately below
        // diode voltage decreases with increasing temperature
        while((i < n) && (diodeVoltage < tcBuffer[i]))
        {
            ++i;
        }

        // gone past the point
        i--;

        if(i >= (n - static_cast<int16_t>(1)))      // make sure data is in range - check for outside limits
        {
            i = n - static_cast<int16_t>(2);
        }

        else if(i < (static_cast<int16_t>(0)))
        {
            i = static_cast<int16_t>(0);
        }

        else
        {
            /* Do Nothing*/
        }


        // linearise reading - Volts to pressure

        t1 = tcBuffer[i];
        t2 = tcBuffer[i + 1];
        t3 = tcBuffer[i + 2];

        // calculate expected pressures at each calibration
        // temperature, given the measured bridge voltage
        p1 = CalculatePressureFromMilliVolt((uint8_t)i, bridgeVoltage);
        p2 = CalculatePressureFromMilliVolt(static_cast<uint8_t>(i + static_cast<int16_t>(1)), bridgeVoltage);
        p3 = CalculatePressureFromMilliVolt(static_cast<uint8_t>(i + static_cast<int16_t>(2)), bridgeVoltage);

        // prepare for quadratic solving

        point_t p[3];
        p[0].x = (float32_t)t1;
        p[0].y = (float32_t)p1;
        p[1].x = (float32_t)t2;
        p[1].y = (float32_t)p2;
        p[2].x = (float32_t)t3;
        p[2].y = (float32_t)p3;
        sQuadratic_t quad;
        quadsolve(p, quad);
        pressureReading = (quad.x2 * diodeVoltage * diodeVoltage) + (quad.x * diodeVoltage) + quad.k;
    }

    else  // only 1 set of tc data available
    {
        pressureReading = CalculatePressureFromMilliVolt(0u, bridgeVoltage);
    }

    // end of cubic fit
    return (pressureReading);
}


/**
* @brief calibration_calculation - calculates spam fits
* @param void
* @return - returns checksum of the coefficient data
*/
uint32_t DAmcSensorData::calculateSpamfits(void)
{
    int16_t i, t;
    float xy[MAXLIN][2];
    uint32_t Checksum = 0u;
    point_t Pxy[MAXLIN];

    // store pressure range
    compensationData.upperPressure = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.upperPressure);
    compensationData.lowerPressure = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.lowerPressure);

    // store temperature range
    compensationData.upperTemperature = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.upperTemperature);
    compensationData.lowerTemperature = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.lowerTemperature);

    // store number of pressure and temperature points
    compensationData.numOfTcCalPoints = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.numOfTcCalPoints);
    compensationData.numOfLinearityCalPoints = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.numOfLinearityCalPoints);

    if(compensationData.numOfLinearityCalPoints > MAXLIN)
    {
        compensationData.numOfLinearityCalPoints = MAXLIN;
    }

    if(compensationData.numOfTcCalPoints > MAXTC)
    {
        compensationData.numOfTcCalPoints = MAXTC;
    }

    numOfLinearityCalPoints = compensationData.numOfLinearityCalPoints;
    numOfTcCalPoints = compensationData.numOfTcCalPoints;


    for(t = 0; t < static_cast<int16_t>(numOfTcCalPoints); t++)
    {
        // find address offset for pointer to data
        compensationData.temperature[t] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t, 0, 0)]);

        Checksum += float_checksum(compensationData.temperature[t]);

        for(i = 0; i < static_cast<int16_t>(numOfLinearityCalPoints); i++)
        {
            xy [i][0] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t, i, VB)]);
            Pxy[i].x = (float32_t)xy [i][0];
            Checksum += float_checksum((float32_t)xy [i][0]);

            xy [i][1] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t, i, P)]);

            Pxy[i].y = (float32_t)xy [i][1];

            Checksum += float_checksum((float32_t)xy [i][1]);
        }

        spamfit(Pxy, static_cast<int16_t>(numOfLinearityCalPoints), newlinspam[t]);

        for(i = 0; i < static_cast<int16_t>(numOfLinearityCalPoints); i++)
        {
            xy [i][0] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t, i, VB)]);

            Pxy[i].x = (float32_t)xy [i][0];

            xy [i][1] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t, i, VC)]);

            Pxy[i].y = (float32_t)xy [i][1];
            Checksum += float_checksum((float32_t)xy [i][1]);
        }

        spamfit(Pxy, static_cast<int16_t>(numOfLinearityCalPoints), newtcspam[t]);
    }

    Checksum %= 10000u;

    return Checksum;
}

/**
* @brief float_checksum - converts float to ascii hex and then then calculates checksum of digits
* @param - float read from eeprom
* @return returns checksum
*/
uint32_t DAmcSensorData::float_checksum(float value)
{
    uint32_t checksum = 0u;
    uint16_t i, j;
    uint8_t digit[2];
    uFloat_t uFdata;


    uFdata.floatValue = value;

    for(i = 0u ; i < 4u ; i++)
    {
        digit[0] = uFdata.byteValue[i] / 16u;    // MSB
        digit[1] = uFdata.byteValue[i] % 16u;    // LSB

        // now convert to hex ascii characters
        for(j = 0u; j < 2u ; j++)
        {
            checksum += (uint32_t)digit[j];
        }

    }

    return checksum;
}

/**
* @brief comp_linearity - mV to pressure linearity calc
* @param index
* @param index   mV pressure input (float)
* @return  pressure reading
*/
float32_t DAmcSensorData::CalculatePressureFromMilliVolt(uint8_t index, float32_t milliVolt)
{
    float32_t pressureValue = 0.0f;

    if(numOfLinearityCalPoints < MINPOINTS)       // no linearity points
    {
        pressureValue = milliVolt;                     // return input mV
    }

    else
    {
        pressureValue = spam(newlinspam[index],
                             static_cast<int16_t>(numOfLinearityCalPoints - 1u), (float32_t)milliVolt);
    }

    return pressureValue;
}

/**
* @brief   comp_temperature - mV common mode to temperature mV calc
* @param   index
* @param   mV temperature input (float)
* @return  temperature reading
*/
float32_t DAmcSensorData::calculateTemperatureFromMilliVolt(uint8_t index, float32_t x)
{
    float32_t temperatureValue = 0.0f;

    if(numOfLinearityCalPoints < MINPOINTS)    // no linearity therefore no tc
    {
        temperatureValue = x;                 // return input mV
    }

    else
    {
        temperatureValue = spam(newtcspam[index], static_cast<int16_t>(numOfLinearityCalPoints - 1u), (float32_t)x);
    }

    return temperatureValue;
}

/**
* @brief   reverse bytes in the buffer
* @param   ptrByteBuffer byte array buffer
* @param   byte array buffer size
* @return  void
*/
void DAmcSensorData::reverseBytes(uint8_t *ptrByteBuffer, uint16_t byteBufferSize)
{
    uint8_t temp;

    for(uint16_t index = 0u; index < byteBufferSize / 2u; index++)
    {
        temp = ptrByteBuffer[index];
        ptrByteBuffer[index] = ptrByteBuffer[byteBufferSize - 1u - index];
        ptrByteBuffer[byteBufferSize - 1u - index] = temp;
    }
}

/**
* @brief   converts unsigned short value from Application format to sensor format
* @param   usValue unsigned short value
* @param   ptrUsValue pointer to variable to return reersed bytes it matches sensor format
* @return  void
*/
void DAmcSensorData::convertValueFromAppToSensorFormat(uint16_t usValue, uint16_t *ptrUsValue)  // short version
{
    uUint16_t uUint16Value;

    uUint16Value.uint16Value = usValue;

    reverseBytes(&uUint16Value.byteValue[0], 2u);

    *ptrUsValue = uUint16Value.uint16Value;


}

/**
* @brief   converts unsigned int value from Application format to sensor format
* @param   uiValue unsigned int value
* @param   ptrUiValue pointer to variable to return reersed bytes it matches sensor format
* @return  void
*/
void DAmcSensorData::convertValueFromAppToSensorFormat(uint32_t uiValue, uint32_t *ptrUiValue)  // long version
{
    uUint32_t uUint32Value;

    uUint32Value.uint32Value = uiValue;

    reverseBytes(&uUint32Value.byteValue[0], 4u);

    *ptrUiValue = uUint32Value.uint32Value;

}

/**
* @brief   converts float value from Application format to sensor format
* @param   fValue float value
* @param   ptrFloatValue pointer to variable to return reersed bytes it matches sensor format
* @return  void
*/
void DAmcSensorData::convertValueFromAppToSensorFormat(float fValue, float *ptrFloatValue)  // float version
{
    uFloat_t uFloatValue;

    uFloatValue.floatValue = fValue;

    reverseBytes(&uFloatValue.byteValue[0], 4u);

    *ptrFloatValue = uFloatValue.floatValue;

}

/**
* @brief   converts signed short value from sensor format to Application format
* @param   sValue
* @return  int16_t returns signed short value matching with application format
*/
int16_t DAmcSensorData::convertValueFromSensorToAppFormat(int16_t sValue)   // short version
{
    uSint16_t int16Value;

    int16Value.int16Value = sValue;

    reverseBytes(&int16Value.byteValue[0], 2u);

    return (int16Value.int16Value);
}

/**
* @brief   converts unsigned short value from sensor format to Application format
* @param   usValue
* @return  uint16_t returns usigned short value matching with application format
*/
uint16_t DAmcSensorData::convertValueFromSensorToAppFormat(uint16_t usValue)    // short version
{
    uUint16_t uUint16Value;

    uUint16Value.uint16Value = usValue;

    reverseBytes(&uUint16Value.byteValue[0], 2u);

    return (uUint16Value.uint16Value);
}

/**
* @brief   converts float value from sensor format to Application format
* @param   fValue float value
* @return  float returns float value matching with application format
*/
float DAmcSensorData::convertValueFromSensorToAppFormat(float fValue)   // short version
{
    uFloat_t uFloatValue;

    uFloatValue.floatValue = fValue;

    reverseBytes(&uFloatValue.byteValue[0], 4u);

    return (uFloatValue.floatValue);
}

/**
* @brief   converts unsigned int value from sensor format to Application format
* @param   uiValue unsigned int value
* @return  uint32_t returns unsigned int value matching with application format
*/
uint32_t DAmcSensorData::convertValueFromSensorToAppFormat(uint32_t uiValue)    // long version
{
    uUint32_t uUint32Value;

    uUint32Value.uint32Value = uiValue;

    reverseBytes(&uUint32Value.byteValue[0], 4u);

    return (uUint32Value.uint32Value);

}

/**
* @brief   converts date value from sensor format to Application format
* @param   buff byte buffer containing date in sensor format
* @param   date pointer to date structure to return date in application format
* @return  void
*/
void DAmcSensorData::convertHeaderDateFromSensorToAppFormat(uint8_t *buff, sDate_t *date)   // long version
{
    date->day = convertHeaderDateByte(buff[1]);
    date->month = convertHeaderDateByte(buff[2]);
    date->year = convertHeaderDateByte(buff[3]) + 2000u;
}


/**
* @brief   converts calibration date value from sensor format to Application format
* @param   dest --- pointer to return calibration date in application format
* @param   src ---- byte buffer containing calibration date in sensor format
* @param   index --- gives the position of calibration date bytes in src buffer
* @return  returns 0 if sucess otherwise -1
*/
int DAmcSensorData::convertCalDateFromSensorToAppFormat(int8_t *dest, int8_t *src, int32_t index)   // long version
{
    int retValue = static_cast<int32_t>(-1);
#ifdef ENABLE_CONVERSION_FROM_SENSOR_TO_APP_FORMAT
    int32_t chkVal = 17 * (index + 1); // from pers board/idos

    if(src[0] != chkVal)
    {
        memset(dest, -1, 4u);
        retValue = -1;
    }

    dest[0] = src[1];   // day
    dest[1] = src[2];   // month
    dest[2] = 20;  // year upper
    dest[3] = src[3];   // year lower
#endif
    return retValue;
}

/**
* @brief   converts calibration date value from Application format to sensor format
* @param   dest --- pointer to return calibration date in sensor format
* @param   src ---- byte buffer containing calibration date in application format
* @param   index --- gives the position of calibration date bytes in src buffer
* @return  returns 0 if sucess otherwise -1
*/
int32_t DAmcSensorData::convertCalDateFromAppToSensorFormat(int8_t *dest, int8_t *src, int32_t index)   // long version
{
    int retValue = static_cast <int32_t>(-1);
#ifdef ENABLE_CONVERSION_FROM_APP_TO_SENSOR_FORMAT
    int32_t chkVal = static_cast <int32_t>(17 * (index + 1u));  // from pers board/idos

    // is it valid?
    if(src[0] > static_cast <int32_t> 0 && src[0] <= static_cast <int32_t> 31)
    {
        dest[0] = chkVal;   // day
        dest[1] = src[0];   // day
        dest[2] = src[1];   // month
        dest[3] = src[3];   // year lower, year is 2 digits
        retValue = static_cast <int32_t>(0);
    }

    else
    {
        memset(dest, -1, 4u);
    }

#endif
    return retValue;
}

/**
* @brief   converts header date byte
* @param   b --- byte to conert
* @return  uint32_t  converted  header date byte
*/
uint32_t DAmcSensorData::convertHeaderDateByte(uint8_t b)
{
    uint32_t resultValue;
    resultValue = static_cast<uint32_t>((static_cast<uint32_t>(b) & 0xfu));
    b >>= 4u;
    resultValue += (static_cast<uint32_t>(b) & 0xfu) * 10u;
    return resultValue;
}

/**
* @brief   get trasducer type
* @param   void
* @return  uint16_t  returns transducer type
*/
uint16_t DAmcSensorData::getTransducerType()
{
    uint16_t sType = myCoefficientsData.amcSensorCoefficientsData.transducerType;
    return convertValueFromSensorToAppFormat(sType);
}

/**
* @brief   get sensor's serial number
* @param   void
* @return  uint32_t  returns sensor's serial number
*/
uint32_t DAmcSensorData::getSerialNumber()
{
    uint32_t serailNumber = 0u;

    if(true == isMyCoefficientsDataValid)
    {
        serailNumber = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.serialNumber);
    }

    else
    {
        serailNumber = 11111111u;
    }

    return serailNumber;
}

/**
* @brief   get sensor's positive full scale value
* @param   void
* @return  float  returns sensor's positive full scale value
*/
float DAmcSensorData::getPositiveFullScale()
{
    return compensationData.upperPressure;
}

/**
* @brief   get sensor's negatie full scale value
* @param   void
* @return  float  returns sensor's negative full scale value
*/
float DAmcSensorData::getNegativeFullScale()
{
    return compensationData.lowerPressure;
}

/**
* @brief   get sensor's brand units name
* @param   brandUnits pointer to char buffer to return sensor's brand units
* @return  float  returns sensor positive full scale value
*/
void DAmcSensorData::getBrandMin(char *brandMin)
{
    brandMin[0] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandNegitiveFullScale[0]);
    brandMin[1] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandNegitiveFullScale[1]);
    brandMin[2] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandNegitiveFullScale[2]);
    brandMin[3] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandNegitiveFullScale[3]);
    brandMin[4] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandNegitiveFullScale[4]);
    brandMin[5] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandNegitiveFullScale[5]);
    brandMin[6] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandNegitiveFullScale[6]);
    brandMin[7] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandNegitiveFullScale[7]);
}

/**
* @brief   get sensor's brand units name
* @param   brandUnits pointer to char buffer to return sensor's brand units
* @return  float  returns sensor positive full scale value
*/
void DAmcSensorData::getBrandMax(char *brandMax)
{
    brandMax[0] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandPositiveFullScale[0]);
    brandMax[1] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandPositiveFullScale[1]);
    brandMax[2] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandPositiveFullScale[2]);
    brandMax[3] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandPositiveFullScale[3]);
    brandMax[4] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandPositiveFullScale[4]);
    brandMax[5] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandPositiveFullScale[5]);
    brandMax[6] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandPositiveFullScale[6]);
    brandMax[7] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandPositiveFullScale[7]);
}

/**
* @brief   get sensor's brand units name
* @param   brandUnits pointer to char buffer to return sensor's brand units
* @return  float  returns sensor positive full scale value
*/
void DAmcSensorData::getBrandType(char *brandType)
{
    brandType[0] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandType[0]);
    brandType[1] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandType[1]);
    brandType[2] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandType[2]);
    brandType[3] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandType[3]);
    brandType[4] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandType[4]);
    brandType[5] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandType[5]);
    brandType[6] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandType[6]);
    brandType[7] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandType[7]);
}
/**
* @brief   get sensor's brand units name
* @param   brandUnits pointer to char buffer to return sensor's brand units
* @return  float  returns sensor positive full scale value
*/
void DAmcSensorData::getBrandUnits(char *brandUnits)
{
    brandUnits[0] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[0]);
    brandUnits[1] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[1]);
    brandUnits[2] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[2]);
    brandUnits[3] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[3]);
    brandUnits[4] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[4]);
    brandUnits[5] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[5]);
    brandUnits[6] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[6]);
    brandUnits[7] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[7]);
    brandUnits[8] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[8]);
    brandUnits[9] = (char)(myCoefficientsData.amcSensorCoefficientsData.brandUnits[9]);
}
