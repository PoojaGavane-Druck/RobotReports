
#include "DAmcSensorData.h" 
#include "uart.h"
#include "Utilities.h"
#include <string>
using namespace std;

#define T 0                // temperature
#define P 1                // pressure
#define VB 2               // bridge voltage
#define VC 3               // common mode voltage

#define DATAFORMAT 3
#define DATAFORMAT_MULTIRANGE 5


DAmcSensorData::DAmcSensorData()
{
    isMyCoefficientsDataValid = false;
    isMyCalibrationDataValid = false;
  
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
}

float DAmcSensorData::getZeroOffset()
{
    return (float)compensationData.zeroOffset; 
}

void DAmcSensorData::setZeroOffset(float dNewZeroOffset)
{
    compensationData.zeroOffset = (float)dNewZeroOffset;

    //save reversed-bytes version too, as that is from where the 
    //zero function will get the bytes to write to sensor
    convertValueFromAppToSensorFormat((float)dNewZeroOffset,
                   (float*)&myCalibrationData.amcSensorCalibrationData.zeroOffset);
}


void DAmcSensorData::getManufacturingDate(sDate_t *pManfDate)
{
  pManfDate->day = myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[0];
  pManfDate->month = myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[1];
  pManfDate->year = ((uint32_t)(myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[2]) * 100u) + myCoefficientsData.amcSensorCoefficientsData.manufacturingDate[3];
}

void DAmcSensorData::getUserCalDate(sDate_t *pUserCalDate)
{
  pUserCalDate->day = myCoefficientsData.amcSensorCoefficientsData.calibrationDates[0];
  pUserCalDate->month = myCoefficientsData.amcSensorCoefficientsData.calibrationDates[1];
  pUserCalDate->year = ((uint32_t)myCoefficientsData.amcSensorCoefficientsData.calibrationDates[2] * 100u) + myCoefficientsData.amcSensorCoefficientsData.calibrationDates[3];
}


bool DAmcSensorData::isPMTERPS()
{
    return isItPMTERPS;
}


void DAmcSensorData::setPMTERPS(bool bIsTerps)
{
    isItPMTERPS = bIsTerps;
}


void DAmcSensorData::trashCoefficientData()
{
    //write over local copy of data so next validation fails unless data is 
    //re-read correctly from the sensor
    myCoefficientsData.amcSensorCoefficientsData.headerValue = 0u;
    myCoefficientsData.amcSensorCoefficientsData.calDataWrite = 0u;
    myCoefficientsData.amcSensorCoefficientsData.headerChecksum = 0u;
    myCoefficientsData.amcSensorCoefficientsData.calDataChecksum = 0u;
}

void DAmcSensorData::trashSensorInformation()
{
	//clear info etc
    myBridgeCounts = 0u;
    myTemperatureCounts = (uint32_t)0;
    diodeCounts = 0u;
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

bool DAmcSensorData::validateCoefficientData()
{
    isMyCoefficientsDataValid = false;

    if (convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.headerValue) == 0x02468ACEu)    /* this is set once written */
    { 
        uint16_t usSize = (convertValueFromSensorToAppFormat((uint16_t)myCoefficientsData.amcSensorCoefficientsData.numberOfHeaderBytes)) / 2u;

        if(usSize <= AMC_COEFFICIENTS_SIZE)
        {
            uint32_t ulChecksum = 0u;
            uint8_t ucDataByteHi; 
            uint8_t ucDataByteLo; 

            //calculate header checksum
            for (uint16_t i = 0u; i < usSize; i++)
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
            if (ulChecksum == ulStoredChecksum)
            {
                    isMyCoefficientsDataValid = true;
            }
        }
    }

    isMyCalibrationDataValid = false;

    if (convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.calDataWrite) == 0x13579BDFu)
    {
        uint32_t ulChecksum = calculateSpamfits();

        uint32_t ulStoredChecksum = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.calDataChecksum); 

        if (ulChecksum == ulStoredChecksum)
        {
                isMyCalibrationDataValid = true;
        }
    }

    return isMyCoefficientsDataValid;
}

void DAmcSensorData::trashCalData()
{
    //write over local copy of data so next validation fails unless data is 
    //re-read correctly from the sensor
    myCalibrationData.amcSensorCalibrationData.calWrite = 0u;
}

void DAmcSensorData::validateCalData()
{
    int32_t index = 0;
    // check whether cal data from pressure standard has been saved
    compensationData.calWrite = convertValueFromSensorToAppFormat(myCalibrationData.amcSensorCalibrationData.calWrite);

    if (compensationData.calWrite == (uint32_t)0x89ABCDEFu)
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
        convertCalDateFromSensorToAppFormat(compensationData.calibrationDates[index],myCalibrationData.amcSensorCalibrationData.calibrationDates[index],index);
    }

    // will be ff when empty/new
    // could have also checked for 17, the check value
    if(myCalibrationData.amcSensorCalibrationData.calibrationDates[0][0] == -1)	
    {
        // get date from header
        uint8_t cal_date[4];  
        sDate_t hdrDate;

        convertHeaderDateFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.calibrationDates,&hdrDate);			

        int8_t* pCalDate = compensationData.calibrationDates[0];
        pCalDate[0] = static_cast<int8_t>(hdrDate.day);
        pCalDate[1] = static_cast<int8_t>(hdrDate.month);
        pCalDate[2] = static_cast<int8_t>(hdrDate.year /100u);
        pCalDate[3] = static_cast<int8_t>(hdrDate.year % 100u);

    }
}

void DAmcSensorData::loadUserCal()
{
    //copy the data read from the sensor into the local calibration data structure
    userCalibrationData.numPoints = (uint8_t)compensationData.numOfPressureCalPoints;
    
    if (userCalibrationData.numPoints < 3u)
    {
            userCalibrationData.numPoints = 1u;
    }
    else
    {
        userCalibrationData.numPoints = static_cast<uint32_t>(compensationData.numOfPressureCalPoints - 1u);
    }

    userCalibrationData.breakpoint[0] = (float)compensationData.pressureCalSetPointValue[1];

    for (uint8_t i = 0u; i < 2u; i++)
    {
        float spanRatio = (float)compensationData.spanRatio[i];	

        userCalibrationData.segments[i].m = spanRatio;

        userCalibrationData.segments[i].c = (float)compensationData.offset[i];
    }

    for (uint8_t i = 0u; i < 3u; i++)
    {
        userCalibrationData.calPoints[i].x = (float)compensationData.pressureCalSetPointValue[i];
        userCalibrationData.calPoints[i].y = 0.0f;
    }

    int8_t* pCalDate = compensationData.calibrationDates[0];
    userCalibrationData.date.day = static_cast<uint8_t>(pCalDate[0]);
    userCalibrationData.date.month = static_cast<uint8_t>(pCalDate[1]);
    userCalibrationData.date.year = (uint32_t)(static_cast<uint32_t>(pCalDate[2]) * 100u) + static_cast<uint32_t>(pCalDate[3]);
}

void DAmcSensorData::saveUserCal()
{
    //copy the local calibration data structure to compensationData 
    compensationData.numOfPressureCalPoints = (uint16_t)userCalibrationData.numPoints;

    for (uint8_t i = 0u; i < 2u; i++)
    {

        float fGain = (float)userCalibrationData.segments[i].m;

        compensationData.spanRatio[i] = fGain;

        compensationData.offset[i] = (float)userCalibrationData.segments[i].c; 
    }

    compensationData.pressureCalSetPointValue[1] = (float)userCalibrationData.breakpoint[0];

    for (uint8_t i = 0u; i < 3u; i++)
    {
            compensationData.pressureCalSetPointValue[i] = (float)userCalibrationData.calPoints[i].x;
    }

    //userCalibrationData.ucNoCalData = (unsigned char)(myCalibrationData.amcSensorCalibrationData.cal_number - 1);

    int8_t* pCalDate;
    // shuffle down the previous cal dates
    // alternatively have 9 and make the start pos all ff's
    for(uint8_t i = NUMBER_OF_CAL_DATES; i > 0u; i--)	
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
    pCalDate[0] = static_cast<int8_t>(userCalibrationData.date.day);
    pCalDate[1] = static_cast<int8_t>(userCalibrationData.date.month);
    pCalDate[2] = static_cast<int8_t>(userCalibrationData.date.year / 100u);
    pCalDate[3] = static_cast<int8_t>(userCalibrationData.date.year % 100u);

    //mark cal data as good
    compensationData.calWrite = 0x89ABCDEFu;

    //clear zero offset
    compensationData.zeroOffset = 0.0f;        

    //Write data in raw format for sensor's storage area
    formatCalData();
}

void DAmcSensorData::formatCalData()
{
    uint32_t index = 0u;
    convertValueFromAppToSensorFormat(compensationData.calWrite, (uint32_t*)&myCalibrationData.amcSensorCalibrationData.calWrite);
    convertValueFromAppToSensorFormat(compensationData.numOfPressureCalPoints, (uint16_t*)&myCalibrationData.amcSensorCalibrationData.numOfPressureCalPoints);

    // calibration pressures
    for(index = 0u; index < 3u; index++)
    {
        convertValueFromAppToSensorFormat((float)compensationData.pressureCalSetPointValue[index],
                                          (float*)&myCalibrationData.amcSensorCalibrationData.pressureCalSetPointValue[index]);        									
    }

    // span ratios
    for(index = 0u; index < 2u; index++)
    {
        convertValueFromAppToSensorFormat((float)compensationData.spanRatio[index],
                                          (float*)&myCalibrationData.amcSensorCalibrationData.spanRatio[index]);        						
    }

    // offset values
    for(index = 0u; index < 2u; index++)
    {
        convertValueFromAppToSensorFormat((float)compensationData.offset[index], 
                                          (float*)&myCalibrationData.amcSensorCalibrationData.offset[index]);        			
    }

    // zero offset - not used so just clear both to zero
    convertValueFromAppToSensorFormat((float)0.0f,
                                      (float*)&myCalibrationData.amcSensorCalibrationData.zeroOffset);
    convertValueFromAppToSensorFormat((uint32_t)0u,
                                      (float*)&myCalibrationData.amcSensorCalibrationData.zeroWrite);

    //CALDATE
    // write in here (all of them)
    for(index = 0u; index < NUMBER_OF_CAL_DATES; index++)	
    {
        convertCalDateFromAppToSensorFormat(myCalibrationData.amcSensorCalibrationData.calibrationDates[index],
                                compensationData.calibrationDates[index],
                               static_cast<int32_t>(index));
    }
}



void DAmcSensorData::trashZeroData()
{
}
/*********************************************************************************************************************/
 //SUPPRESS: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma ("diag_suppress=Pm046")
/*********************************************************************************************************************/
void DAmcSensorData::validateZeroData(float fZeroValueFromSensor)
{
    myCalibrationData.amcSensorCalibrationData.zeroOffset = fZeroValueFromSensor; //save it, but this is not really necessary

    //value in sensor has reversed-bytes, so undo this for application data structure
    float fZeroValue = convertValueFromSensorToAppFormat(myCalibrationData.amcSensorCalibrationData.zeroOffset);        									

    //guard against illegal values
    //if ((fabs(fZeroValue) < FLT_MIN)  (fabs(fZeroValue) > FLT_MAX) || (ISNAN(fZeroValue)))
    if ((fabs(fZeroValue) < FLT_MIN) || (ISNAN(fZeroValue) ))
    {
        fZeroValue = 0.0f;
    }
    else if (fabs(fZeroValue) > FLT_MAX)
    {
       fZeroValue = 0.0f;
    }
    else
    {
      /* Do Nothing*/
    }

    compensationData.zeroOffset = (float)fZeroValue;
}

/*********************************************************************************************************************/
 //RESTORE: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma ("diag_default=Pm046")
/*********************************************************************************************************************/

uint8_t* DAmcSensorData::getSensorDataArea()
{
    return myCoefficientsData.sensorCoefficientsDataMemory;
}

uint8_t* DAmcSensorData::getCalDataArea()
{
    return myCalibrationData.sensorCalibrationDataMemory;
}

float* DAmcSensorData::getHandleToZeroOffset()
{
    return (float*)&myCalibrationData.amcSensorCalibrationData.zeroOffset;
}

float DAmcSensorData::getPressureMeasurement(uint32_t bridgeCounts, 
                                             uint32_t temperatureCounts)
{    
    
    if(0XFFFFFFFFu != bridgeCounts)
    {
      myBridgeCounts = bridgeCounts;
    }
    
    if(0XFFFFFFFFu != temperatureCounts)
    {
      myTemperatureCounts = temperatureCounts;
    }
    float norm_Vb = (float)myBridgeCounts * BIPOLAR_ADC_CONV_FACTOR_AMC;
    float norm_Vd = (float)myTemperatureCounts * BIPOLAR_ADC_CONV_FACTOR_AMC;

    // calculate pressure 
    return getCompensatedPressureMeasurement(norm_Vb,norm_Vd);
}




/*******************************************************/
/* get_index - calculates index of data item           */
/*                                                     */
/* input parameters -                                  */
/*                    short t - temperature index      */
/*                    short lin linearity index        */
/*                    data 0 - temperature             */
/*                    data 1 - pressure                */
/*                    data 2 - bridge                  */
/*                    data 3 - cmode                   */
/* output parameter - index to float number            */
/*******************************************************/

int16_t DAmcSensorData::get_index(int16_t t, int16_t lin, int16_t data)
{
    int16_t index;
    index = t * (static_cast<int16_t>(compensationData.numOfLinearityCalPoints) * 3 + 1);// start of data

    if ( 0 == data)             // temperature
    {
        index = index;   // only 1 temperature per block
    }
    else
    {
        index = (index + (lin * 3) + data);
    }
    return index;
}

/*******************************************************/
/* compensate - calculates pressure                    */
/*                                                     */
/* input parameters -                                  */
/*                    float bridge voltage            */
/*                    float temperature voltage       */
/* output parameter - compensated pressure reading     */
/*                    in mbar (float)                 */
/*******************************************************/
float DAmcSensorData::getCompensatedPressureMeasurement(float bridgeVoltage, float diodeVoltage)
{
    // cubic fit
    float pressureReading;
    float t1,t2,t3,p1,p2,p3;
    int16_t i,n;

    float tcBuffer[MAXTC];

    if (numOfTcCalPoints >= 3u)     // at least 3 temperature points available
    {
        for (i = static_cast<int16_t>(0); i < static_cast<int16_t>(numOfTcCalPoints); i++)
        {
                // calculate expected diode voltages at each calibration
                // temperature, given the measured bridge voltage 
                tcBuffer[i] = calculateTemperatureFromMilliVolt((uint8_t)i,bridgeVoltage);
        }
        i = static_cast<int16_t>(0);
        n = static_cast<int16_t>(numOfTcCalPoints) - static_cast<int16_t>(1);

        // find which tcBuffer values the measured diode voltage
        // lies between i.e. which calibration temperatures the 
        // sensor is between. i represents the value immediately below
        // diode voltage decreases with increasing temperature
        while ((i < n )&& (diodeVoltage < tcBuffer[i]))
        {
            ++i;
        }

        // gone past the point
        i--;

        if (i >= (n - static_cast<int16_t>(1)))     // make sure data is in range - check for outside limits
        {
            i = n- static_cast<int16_t>(2);
        }
        else if (i < (static_cast<int16_t>(0)))
        {
            i = static_cast<int16_t>(0);
        }
        else
        {
          /* Do Nothing*/
        }


        // linearise reading - Volts to pressure

        t1 = tcBuffer[i];
        t2 = tcBuffer[i+1];
        t3 = tcBuffer[i+2];

        // calculate expected pressures at each calibration
        // temperature, given the measured bridge voltage
        p1 = CalculatePressureFromMilliVolt ((uint8_t)i,bridgeVoltage);
        p2 = CalculatePressureFromMilliVolt (static_cast<uint8_t>(i + static_cast<int16_t>(1)),bridgeVoltage);
        p3 = CalculatePressureFromMilliVolt (static_cast<uint8_t>(i +static_cast<int16_t> (2) ),bridgeVoltage);

        // prepare for quadratic solving

        point_t p[3];
        p[0].x = (float)t1;
        p[0].y = (float)p1;
        p[1].x = (float)t2;
        p[1].y = (float)p2;
        p[2].x = (float)t3;
        p[2].y = (float)p3;
        sQuadratic_t quad;	  
        quadsolve (p, quad);
        pressureReading = (quad.x2 * diodeVoltage * diodeVoltage) + (quad.x * diodeVoltage) + quad.k;
    }
    else  // only 1 set of tc data available
    {
        pressureReading = CalculatePressureFromMilliVolt (0u,bridgeVoltage);
    }

    // end of cubic fit
    return (pressureReading);
}


/*******************************************************/
/* calibration_calculation - calculates spam fits      */
/*                                                     */
/* input parameters -                                  */
/* output parameter - none                             */
/*******************************************************/

uint32_t DAmcSensorData::calculateSpamfits(void)
{
    int16_t i,t;
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

    if (compensationData.numOfLinearityCalPoints > MAXLIN)
    {
        compensationData.numOfLinearityCalPoints = MAXLIN;
    }

    if (compensationData.numOfTcCalPoints > MAXTC)
    {
        compensationData.numOfTcCalPoints = MAXTC;
    }

    numOfLinearityCalPoints = compensationData.numOfLinearityCalPoints;
    numOfTcCalPoints = compensationData.numOfTcCalPoints;

    
    for (t = 0; t < static_cast<int16_t>(numOfTcCalPoints); t++)
    {
        // find address offset for pointer to data
        compensationData.temperature[t] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t,0,0)]); 			

        Checksum += float_checksum(compensationData.temperature[t]);                

        for (i = 0; i < static_cast<int16_t>(numOfLinearityCalPoints); i++)
        {
            xy [i][0] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t,i,VB)]); 						
            Pxy[i].x = (float)xy [i][0];
            Checksum += float_checksum((float)xy [i][0]);

            xy [i][1] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t,i,P)]); 									

            Pxy[i].y = (float)xy [i][1];

            Checksum += float_checksum((float)xy [i][1]);
        }

        spamfit (Pxy, static_cast<int16_t>(numOfLinearityCalPoints), newlinspam[t]);

        for (i = 0; i < static_cast<int16_t>(numOfLinearityCalPoints); i++)
        {
            xy [i][0] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t,i,VB)]); 									

            Pxy[i].x = (float)xy [i][0];

            xy [i][1] = convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.characterisationData[get_index(t,i,VC)]); 												

            Pxy[i].y = (float)xy [i][1];
            Checksum += float_checksum((float)xy [i][1]);
        }

        spamfit (Pxy, static_cast<int16_t>(numOfLinearityCalPoints), newtcspam[t]);
    }

    Checksum %= 10000u;

    return Checksum;
}

/*******************************************************/
/* float_checksum - converts float to ascii hex and then*/
/*		    then calculates checksum of digits */
/*                                                     */
/* input parameters - float read from eeprom           */
/* input parameters -				       */
/* output parameter - checksum                 	       */
/*******************************************************/
uint32_t DAmcSensorData::float_checksum(float value)
{
    uint32_t checksum = 0u;
    uint16_t i,j;
    uint8_t digit[2];
    uFloat_t uFdata;
    

    uFdata.floatValue = value;

    for(i=0u ;i < 4u ; i++)
    {
        digit[0] = uFdata.byteValue[i] / 16u;    // MSB
        digit[1] = uFdata.byteValue[i] % 16u;    // LSB

        // now convert to hex ascii characters
        for(j = 0u; j < 2u ; j++)
        {
          /*
            if(uFdata.byteValue[j] < 10u)
            {
                uFdata.byteValue[j] += 48u;
            }
            else
            {
                uFdata.byteValue[j] += 55u;
            }
*/

            checksum += (uint32_t)digit[j];
        }

    }

    return checksum;
}

/*******************************************************/
/* comp_linearity - mV to pressure linearity calc      */
/*                                                     */
/* input parameters -                                  */
/* input parameters - mV pressure input (float)       */
/* output parameter - pressure reading                 */
/*******************************************************/

float DAmcSensorData::CalculatePressureFromMilliVolt(uint8_t index,float milliVolt)
{
    float pressureValue = 0.0f;
    if (numOfLinearityCalPoints < MINPOINTS)      // no linearity points
    {
        pressureValue = milliVolt;                     // return input mV
    }	
    else
    {
        pressureValue = spam(newlinspam[index], 
                             static_cast<int16_t>(numOfLinearityCalPoints - 1u), (float)milliVolt);
    }
		
    return pressureValue;
}


/*********************************************************/
/* comp_temperature - mV common mode to pressure mV calc */
/*                                                       */
/* input parameters -                                    */
/* input parameters - mV temperature input (float)      */
/* output parameter - pressure reading                   */
/*********************************************************/

float DAmcSensorData::calculateTemperatureFromMilliVolt(uint8_t index, float x)
{
    float temperatureValue = 0.0f;
    if (numOfLinearityCalPoints < MINPOINTS)   // no linearity therefore no tc
    {
        temperatureValue = x;                 // return input mV
    }
    else
    {
	temperatureValue = spam(newtcspam[index], static_cast<int16_t>(numOfLinearityCalPoints - 1u), (float)x);
    }
    return temperatureValue;
}


void DAmcSensorData::reverseBytes(uint8_t* ptrByteBuffer,uint16_t byteBufferSize)
{
    uint8_t temp;

    for(uint16_t index = 0u; index < byteBufferSize / 2u; index++)
    {
        temp = ptrByteBuffer[index];
        ptrByteBuffer[index] = ptrByteBuffer[byteBufferSize - 1u - index];
        ptrByteBuffer[byteBufferSize - 1u - index] = temp;
    }
}

/* writes data to structure members AND reverses bytes to match header upload/download commands */
void DAmcSensorData::convertValueFromAppToSensorFormat(uint16_t usValue, uint16_t *ptrUsValue)	// short version
{
    uUint16_t uUint16Value;
    
    uUint16Value.uint16Value = usValue;
    
    reverseBytes(&uUint16Value.byteValue[0],2u);

    *ptrUsValue = uUint16Value.uint16Value;


}

void DAmcSensorData::convertValueFromAppToSensorFormat(uint32_t uiValue, uint32_t *ptrUiValue)	// long version
{
    uUint32_t uUint32Value;
    
    uUint32Value.uint32Value = uiValue;
    
    reverseBytes(&uUint32Value.byteValue[0],4u);

    *ptrUiValue = uUint32Value.uint32Value;

}

void DAmcSensorData::convertValueFromAppToSensorFormat(float fValue, float *ptrFloatValue)	// float version
{
    uFloat_t uFloatValue;
    
    uFloatValue.floatValue = fValue;
    
    reverseBytes(&uFloatValue.byteValue[0],4u);

    *ptrFloatValue = uFloatValue.floatValue;

}
//like the write command above reads data to structure members AND reverses bytes 
//to match header upload/download commands 
int16_t DAmcSensorData::convertValueFromSensorToAppFormat(int16_t sValue)	// short version
{
    uSint16_t int16Value;
    
    int16Value.int16Value = sValue;
    
    reverseBytes(&int16Value.byteValue[0],2u);

    return (int16Value.int16Value);
}

//like the write command above reads data to structure members AND reverses bytes 
//to match header upload/download commands 
uint16_t DAmcSensorData::convertValueFromSensorToAppFormat(uint16_t usValue)	// short version
{
    uUint16_t uUint16Value;
    
    uUint16Value.uint16Value = usValue;
    
    reverseBytes(&uUint16Value.byteValue[0],2u);

    return (uUint16Value.uint16Value);
}

float DAmcSensorData::convertValueFromSensorToAppFormat(float fValue)	// short version
{
    uFloat_t uFloatValue;
    
    uFloatValue.floatValue = fValue;
    
    reverseBytes(&uFloatValue.byteValue[0],4u);

    return (uFloatValue.floatValue);
}

uint32_t DAmcSensorData::convertValueFromSensorToAppFormat(uint32_t uiValue)	// long version
{
    uUint32_t uUint32Value;
    
    uUint32Value.uint32Value = uiValue;
    
    reverseBytes(&uUint32Value.byteValue[0],4u);

    return (uUint32Value.uint32Value);

}

void DAmcSensorData::convertHeaderDateFromSensorToAppFormat(uint8_t* buff, sDate_t* date)	// long version
{
    date->day = convertHeaderDateByte(buff[1]);
    date->month = convertHeaderDateByte(buff[2]);
    date->year = convertHeaderDateByte(buff[3]) + 2000u;
}


/// 
/// Converts from sensor format (chk, d,m,yy)
/// to app format (d,m,yy,yy)
/// 
/// returns <0 if not a valid date
int DAmcSensorData::convertCalDateFromSensorToAppFormat(int8_t* dest, int8_t* src,int32_t index)	// long version
{
    int retValue = static_cast<int32_t> (-1);
#if 0
    int chkVal = 17u * (index+1);	// from pers board/idos	
    
    if(src[0] != chkVal)	{
            memset(dest,-1,4u);
            return -1;
    }
    dest[0] = src[1];	// day
    dest[1] = src[2];	// month
    dest[2] = 20u;	// year upper
    dest[3] = src[3];	// year lower
#endif
    return retValue;
}

///
///	Converts back to sensor format
///
///
int32_t DAmcSensorData::convertCalDateFromAppToSensorFormat(int8_t* dest, int8_t* src,int32_t index)	// long version
{
    int retValue = static_cast <int32_t> (-1);
#if 0    
    int32_t chkVal = static_cast <int32_t> (17 * (index+1u));	// from pers board/idos	
    
    // is it valid?
    if(src[0] > static_cast <int32_t> 0 && src[0] <= static_cast <int32_t> 31)	
    { 
        dest[0] = chkVal;	// day
        dest[1] = src[0];	// day
        dest[2] = src[1];	// month
        dest[3] = src[3];	// year lower, year is 2 digits
        retValue =static_cast <int32_t> (0);
    }
    else
    {
        memset(dest,-1,4u);
    }
#endif
    return retValue;
}



///
///	header dates are stored in ascii encoded hex
/// ie 28/11/2008 = 0x28, 0x11, 0x08
/// All other dates will be stored in the internal format
///
uint32_t DAmcSensorData::convertHeaderDateByte(uint8_t b)
{
    uint32_t resultValue;
    resultValue =static_cast<uint32_t> ((static_cast<uint32_t> (b) & 0xfu));
    b >>=4u;
    resultValue += (static_cast<uint32_t>(b) & 0xfu)*10u;
    return resultValue;
}


uint16_t DAmcSensorData::getTransducerType ()
{
    uint16_t sType = myCoefficientsData.amcSensorCoefficientsData.transducerType;
    return convertValueFromSensorToAppFormat(sType);
}

uint32_t DAmcSensorData::getSerialNumber()
{
   uint32_t serailNumber = 0u;
   if(true == isMyCoefficientsDataValid)
   {
     serailNumber =convertValueFromSensorToAppFormat(myCoefficientsData.amcSensorCoefficientsData.serialNumber);
   }
   else
   {
     serailNumber = 11111111u;
   }
    return serailNumber;
}

float DAmcSensorData::getPositiveFullScale()
{
    return compensationData.upperPressure;
}

float DAmcSensorData::getNegativeFullScale()
{
    return compensationData.lowerPressure;
}

void DAmcSensorData::getBrandUnits(char* brandUnits)
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
