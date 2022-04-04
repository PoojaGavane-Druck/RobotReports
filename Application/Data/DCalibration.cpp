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
* @file     DCalibration.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     18 May 2020
*
* @brief    The calibration base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <math.h>
#include <assert.h>
#include <string.h>
MISRAC_ENABLE

#include "DCalibration.h"
#include "crc.h"
#include "DPV624.h"
#include "Utilities.h"
#include "DLock.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
const int32_t ITERATION_LIMIT = 10;         //maximum number of iterations used for inverse quadratic function
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCalibration class constructor
 * @param   pointer to calibration data structure
 * @param   number of cal points
 * @param   convergenceLimit is the value used for inverse calculation by successive approximation
 * @retval  void
 */
DCalibration::DCalibration(sSensorData_t *calData, uint32_t numCalPoints, float32_t convergenceLimit)
{
    OS_ERR os_error = OS_ERR_NONE;

    //create mutex for resource locking
    char *name = "calData";
    memset((void *)&myMutex, 0, sizeof(OS_MUTEX));
    RTOSMutexCreate(&myMutex, (CPU_CHAR *)name, &os_error);
    myCalData = NULL;
    //load cal data
    load(calData, numCalPoints);

    //set up limit for inverse calculations
    myConvergenceLimit = convergenceLimit;
}

/**
 * @brief   Validate & sanity check cal data
 * @note    The status of the cal data is set and can be checked by owning range/sensor/slot/function/channel hierarchy
 * @param   number of cal points
 * @retval  true if valid data, else false
 */
bool DCalibration::validate(uint32_t numCalPoints)
{
    bool valid = true;

    DLock is_on(&myMutex);

    //cal data status set to not valid, not ignoring
    myStatus.value = 0u;

    //set defaults as a starting point
    myCoefficients.a = 0.0f;  //quadratic term = 0
    myCoefficients.b = 1.0f;  //linear term = 1
    myCoefficients.c = 0.0f;  //offset term = 0

    if(myCalData != NULL)
    {
        if((myCalData->calStatus == SENSOR_CALIBRATED))
        {
            if(numCalPoints == 0u)
            {
                //no calibration used for this range - mark as valid as not used doesn't mean it's bad; not using it either!
                myStatus.valid = 1u;
                myStatus.ignore = 1u;
            }

            if(myCalData->data.numPoints != numCalPoints)
            {
                valid = false;
            }

            else if(isDateValid(myCalData->data.date.day, myCalData->data.date.month, myCalData->data.date.year) == false)
            {
                valid = false;
            }

            else
            {
                //check if x values all valid numbers
                for(uint32_t i = 0u; i < numCalPoints; i++)
                {
                    if(isnan(myCalData->data.calPoints[i].x))
                    {
                        valid = false;
                    }
                }

                //continue if still good
                if(valid == true)
                {
                    //check if y values all valid numbers
                    for(uint32_t i = 0u; i < numCalPoints; i++)
                    {
                        if(isnan(myCalData->data.calPoints[i].y))
                        {
                            valid = false;
                            break;
                        }
                    }
                }

                //continue if still good
                if(valid == true)
                {
                    //determine cal coefficients
                    valid = calculateCoefficients();
                }
            }
        }

        else
        {
            myStatus.valid = 0u;
            myStatus.ignore = 1u;
        }
    }

    return valid;
}

/**
 * @brief   Calculates the CalData from the data points
 * @param   void
 * @retval  true if valid data, else false
 */
bool DCalibration::calculateCoefficients(void)
{
    DLock is_on(&myMutex);

    if(myCalData != NULL)
    {
        sCalPoint_t *calPoints = &myCalData->data.calPoints[0];

        //make sure the cal point are in order (only for 2 or more cal points)
        if((myCalData->data.numPoints > 1u) && (myCalData->data.numPoints <= 3u))
        {
            sortCalPoints(calPoints, myCalData->data.numPoints);
        }

        //start off with default settings
        myCoefficients.a = 0.0f;  //quadratic term = 0
        myCoefficients.b = 1.0f;  //linear term = 1
        myCoefficients.c = 0.0f;  //offset term = 0

        //update coefficients depending on no of cal points
        switch(myCalData->data.numPoints)
        {
        case 1:     //offset only
            myCoefficients.c = calPoints[0].y - calPoints[0].x;
            myStatus.valid = 1u;      //mark as valid
            break;

        case 2:     //"y = bx + c" relationship
        {
            float32_t dx = (calPoints[1].x  - calPoints[0].x);

            //guard against divide-by-zero
            if(floatEqual(dx, 0.0f) == true)
            {
                myStatus.valid = 0u;      //mark as invalid
            }

            else
            {
                myCoefficients.b = (calPoints[1].y  - calPoints[0].y) / dx;
                myCoefficients.c = calPoints[0].y - (myCoefficients.b * calPoints[0].x);
                myStatus.valid = 1u;      //mark as valid
            }
        }
        break;

        case 3:     //quadratic (y = ax^^2 + bx + c) relationship
            if(determineQuadraticCoefficients() == true)
            {
                myStatus.valid = 1u;      //mark as valid
            }

            else
            {
                myStatus.valid = 0u;      //mark as invalid
            }

            break;

        default: //invalid or unsupported number of cal points
            myStatus.valid = 0u;      //mark as invalid
            break;
        }
    }

    //return true is cal data valid and coefficients calculated
    return (myStatus.valid == 1u);
}

/**
 * @brief   Determine quadratic coefficients from cal data points
 * @note    calibration input points are assumed to be in order
 *
 *          For a quadratic equation: y =  (a * x^^2) + (b * x) + c
 *
 *          (x1,y1), (x2,y2) & (x3,y3) are coordinates of reference values (xi) and corresponding measured values (y1)
 *
 *          Coefficients (a, b & c) are given by:
 *
 *              a = (y3-y2) / ((x3-x2) * (x3-x1)) – (y2-y1) / ((x2-x1) * (x3-x1))
 *
 *              b = (y2-y1) / (x2-x1) – A * (x2+x1)
 *
 *              c = y1 – A * x1 * x1 – B * x1
 *
 * @param   void
 * @retval  true if calculated successfully, false if failed
 */
bool DCalibration::determineQuadraticCoefficients(void)
{
    float32_t x1 = myCalData->data.calPoints[0].x;
    float32_t x2 = myCalData->data.calPoints[1].x;
    float32_t x3 = myCalData->data.calPoints[2].x;

    float32_t y1 = myCalData->data.calPoints[0].y;
    float32_t y2 = myCalData->data.calPoints[1].y;
    float32_t y3 = myCalData->data.calPoints[2].y;

    //guard against divide-by-zero
    bool success = !floatEqual((x3 - x2), 0.0f);

    if(success == true)
    {
        success = !floatEqual((x3 - x1), 0.0f);

        if(success == true)
        {
            success = !floatEqual((x2 - x1), 0.0f);

            if(success == true)
            {
                myCoefficients.a = (y3 - y2) / ((x3 - x2) * (x3 - x1)) - (y2 - y1) / ((x2 - x1) * (x3 - x1));
                myCoefficients.b = (y2 - y1) / (x2 - x1) - myCoefficients.a * (x2 + x1);
                myCoefficients.c = y1 - (myCoefficients.a * x1 * x1)  - (myCoefficients.b * x1);

                //TODO HSB: could sanity check coefficients - 'a' should be close to 0.0, 'b' should be close to 1.0, and 'c'
                //should be close to 0.0 as it should be an almost straight line relationship between x and y. Dean thinks it is
                //good idea to have checks but my statement above is not quite right. He says: There's a problem with this as
                //different measurands are expressed in different units. So, for instance, you would expect the "c" to be about
                //1000x bigger in mV than it is in Volts.
                //even though I say that I think that should be covered by scaling factors, which are empirically determined,
                //to give default values to be close to a sensible measurement for each of: ADC counts to mA; ADC counts to Volts
                //and ADC counts to mV - such that, even with no cal data applied the readings are about right. So, on top of
                //these the adjustment using parabolic coefficient a, b and c should be small.
                //But Dean says: yes, but c is in the same units of the measurand, b is in units of measurand/measurand and 'a'
                //is in units of 1/measurand. So the only one we can say anything about is b and it should be 1 +/- 5% say
                //You could say that magnitude of c should be < 500ppmFS say and you could say something similar for 'a' if I
                //thought about it (maybe). I think it would be a definite enhancement but needs a bit of thought to do something
                //that's worthwhile and is going to catch bad/wrong stuff
            }
        }
    }

    return success;
}

/**
 * @brief   Clear all cal data
 * @param   void
 * @retval  void
 */
void DCalibration::clear(void)
{
    DLock is_on(&myMutex);

    myStatus.value = 0u;    //cal data status set to not valid, not calibrating, not ignoring

    myCoefficients.a = 0.0f;  //quadratic term = 0
    myCoefficients.b = 1.0f;  //linear term = 1
    myCoefficients.c = 0.0f;  //offset term = 0
}

/**
 * @brief   Load calibration data
 * @param   pointer to calibration data structure
 * @param   number of cal points
 * @retval  true if valid data, else false
 */
bool DCalibration::load(sSensorData_t *calData, uint32_t numCalPoints)
{
    DLock is_on(&myMutex);

    myCalData = calData;

    //validate the cal data before using
    return validate(numCalPoints);
}

/**
 * @brief   Apply cal data
 * @param   void
 * @retval  void
 */
void DCalibration::apply(void)
{
    DLock is_on(&myMutex);

}

/**
 * @brief   Revert to cal data
 * @param   void
 * @retval  void
 */
void DCalibration::revert(void)
{
    DLock is_on(&myMutex);

}

/**
 * @brief   Set ignore calibration mode
 * @param   void
 * @retval  void
 */
void DCalibration::setCalIgnore(bool state)
{
    DLock is_on(&myMutex);
    myStatus.ignore = (state == false) ? 0u : 1u;
}

/**
 * @brief   Query if cal is valid
 * @param   void
 * @retval  true if cal is validated and active, else false
 */
bool DCalibration::isValidated(void)
{
    bool validated = false;

    DLock is_on(&myMutex);

    if(myStatus.valid == 1u)
    {
        validated = true;
    }

    return validated;
}

/**
 * @brief   Query if this instance has cal data
 * @param   void
 * @retval  true if cal data is not NULL, else false
 */
bool DCalibration::hasCalData(void)
{
    DLock is_on(&myMutex);
    return (myCalData != NULL);
}


/**
 * @brief   Sort all of the calibration points into ascending order according to input value
 * @param   pointer to cal points
 * @param   numPoints is the number of cal points
 * @retval  void
 */
void DCalibration::sortCalPoints(sCalPoint_t *points, uint32_t numPoints)
{
    if(myCalData != NULL)
    {
        float32_t fTemp;

        //sort the cal points in ascending input value order
        for(uint32_t i = 0u; i < (numPoints - 1u); i++)
        {
            for(uint32_t j = 1u; j < (numPoints - i); j++)
            {
                if(points[i].x > points[i + j].x)
                {
                    fTemp = points[i].x;
                    points[i].x = points[i + j].x;
                    points[i + j].x = fTemp;

                    fTemp = points[i].y;
                    points[i].y = points[i + j].y;
                    points[i + j].y = fTemp;
                }
            }
        }
    }
}

/**
 * @brief   Determine which cal segment to use and translate supplied uncalibrated value given to calibrated value
 * @param   uncalibrated value
 * @retval  calibrated value
 */
float32_t DCalibration::calculate(float32_t x)
{
    float32_t y = x;

    if(myCalData != NULL)
    {
        //validated cal that we are not ignoring
        if((myStatus.valid == 1u) && (myStatus.ignore == 0u))
        {
            //use coefficients depending on no of cal points
            switch(myCalData->data.numPoints)
            {
            case 1:     //offset only
                y = x + myCoefficients.c;
                break;

            case 2:     //"y = bx + c" relationship
                y = myCoefficients.b * x + myCoefficients.c;
                break;

            case 3:     //quadratic (y = ax^^2 + bx + c) relationship
                y = (myCoefficients.a * x * x) + (myCoefficients.b * x) + myCoefficients.c;
                break;

            default: //invalid or unsupported number of cal points, so just pass over
                break;
            }
        }
    }

    return y;
}

/**
 * @brief   Determine which cal segment to use and translate supplied calibrated value given to uncalibrated value
 * @param   calibrated value
 * @retval  uncalibrated value
 */
float32_t DCalibration::reverse(float32_t y)
{
    float32_t x = y;

    if(myCalData != NULL)
    {
        if((myStatus.valid == 1u) && (myStatus.ignore == 0u))
        {
            switch(myCalData->data.numPoints)
            {
            case 1:     //offset only
                x = y - myCoefficients.c;
                break;

            case 2:     //"x = (y - c)/b
                x = (y - myCoefficients.c) / myCoefficients.b;
                break;

            case 3:     //quadratic (y = ax^^2 + bx + c) relationship
                x = inverseCalculate(y);
                break;

            default: //invalid or unsupported number of cal points, so just pass over
                break;
            }
        }
    }

    return x;
}

/**
 * @brief   Calculate inverse function by successive approximation
 * @note    It is expected that within 2 interations we will be within acceptance limit.
 *          Method is to iterate towards the answer using the equations from
 *
 *          the quadratic equation: y =  (a * x^^2) + (b * x) + c
 *
 *          (x1,y1), (x2,y2) & (x3,y3) are coordinates of reference values (xi) and corresponding measured values (y1)
 *
 *          As stated in the determineQuadraticCoefficients() function, the coefficient 'c' is given by:
 *
 *              c = y1 – A * x1 * x1 – B * x1
 *
 *          Therefore, for any 'y' value, 'x' is given by:
 *
 *              x = (y - c)/((a * x) + b)
 *
 *          For iterations, xj is the current value derived from the previous iteration value xi:
 *
 *              xj = (y - c)/((a * xi) + b)
 *
 *          The initial 'guess' uses xi = 0.
 *
 * @param   y is input value
 * @return  x is output value
 */
float32_t DCalibration::inverseCalculate(float32_t y)
{
    float32_t x = y;
    float32_t xi = 0.0f;
    float32_t lastX;

    //Probably where xi changes by less than 10ppmFS feels about right but it might need some experiment - 100ppmFS might be good enough. It's a bit difficult to say as we don't know what it will be used for!

    for(int32_t i = 0; i < ITERATION_LIMIT; i++)
    {
        //save current iteration value
        lastX = xi;

        //calculate new iteration value
        xi = (y - myCoefficients.c) / ((myCoefficients.a * xi) + myCoefficients.b);

        //it is expected that there would have to be something wrong with the cal points for it not to converge
        //very quickly; ie, after 2 or 3 iterations. So, we can check if iterations have converged enough from
        //second iteration onwards
        if(i > 0)
        {
            //if the difference between iterations is close enough we can consider it done
            if(fabsf(xi - lastX) < myConvergenceLimit)
            {
                x = xi;
                break;
            }
        }
    }

    //sanity check the value by forward calculating and comparing
    float32_t yi = calculate(x);

    //if the error is too big then just return y
    if(fabsf(yi - y) > myConvergenceLimit)
    {
        x = y;

        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
    }

    return x;
}

/**
 * @brief   Set expected number of calibration points
 * @param   numCalPoints is the number of cal points
 * @retval  true if accepted, else false
 */
bool DCalibration::setNumCalPoints(uint32_t numCalPoints)
{
    bool flag = false;

    DLock is_on(&myMutex);

    if((myCalData != NULL) && (numCalPoints <= (uint32_t)MAX_CAL_POINTS))
    {
        myCalData->data.numPoints = numCalPoints;
        flag = true;
    }

    return flag;
}

/**
 * @brief   initialise for calibration/adjustment
 * @param   void
 * @retval  void
 */
void DCalibration::calInitialise(void)
{
    DLock is_on(&myMutex);
    myStatus.calPoints = 0u;    //clear 'entered calibration point' field
    myStatus.ignore = 1u;       //do not use (ignore) calibration data
}

/**
 * @brief   Get 'entered calibration points' status
 * @note    This function assumes that number of cal points is between 1 and 3.
 * @param   numCalPoint sis the number of expected cal points
 * @retval  true if all cals point have been supplied, else false
 */
bool DCalibration::getCalComplete(uint32_t numCalPoints)
{
    bool complete = false;

    DLock is_on(&myMutex);

    switch(numCalPoints)
    {
    case 1: //check that one and only cal point has been entered
        complete = ((myStatus.calPoints & 0x1u) == 0x1u);
        break;

    case 2: //check that both cal points have been entered
        complete = ((myStatus.calPoints & 0x3u) == 0x3u);
        break;

    case 3: //check that all three cal points have been entered
        complete = ((myStatus.calPoints & 0x7u) == 0x7u);
        break;

    default:    //can't happen as we don't support more than 3 cal points (MAX_CAL_POINTS)
        break;
    }

    return complete;
}

/**
 * @brief   Set cal point and perform cal data calculation as a result of this
 * @param   point is the cal point number (starting at 1 ...)
 * @param   x is input value
 * @param   y is output value
 * @retval  true if accepted, else false
 */
bool DCalibration::setCalPoint(uint32_t point, float32_t x, float32_t y)
{
    bool flag = false;
    DLock is_on(&myMutex);

    //cal point must start at 1, so '0' is illegal
    if((myCalData != NULL) && (point > 0u))
    {
        //check index to calPoints array
        uint32_t index = point - 1u;

        if(index < (uint32_t)MAX_CAL_POINTS)
        {
            //update status to indicate that this cal point has been supplied
            myStatus.calPoints |= ((uint32_t)0x1u << index);

            myCalData->data.calPoints[index].x = x;
            myCalData->data.calPoints[index].y = y;

            flag = true;
        }
    }

    return flag;
}

/**
 * @brief   Get date
 * @param   pointer to date structure for return value
 * @retval  void
 */
void DCalibration::getDate(sDate_t *date)
{
    DLock is_on(&myMutex);

    if(myCalData != NULL)
    {
        date->day = myCalData->data.date.day;
        date->month = myCalData->data.date.month;
        date->year = myCalData->data.date.year;
    }

    else
    {
        date->day = 0u;
        date->month = 0u;
        date->year = 0u;
    }
}

/**
 * @brief   Set date
 * @param   pointer to date structure
 * @retval  void
 */
void DCalibration::setDate(sDate_t *date)
{
    DLock is_on(&myMutex);

    if(myCalData != NULL)
    {
        myCalData->data.date.day = date->day;
        myCalData->data.date.month = date->month;
        myCalData->data.date.year = date->year;
    }
}

/**
 * @brief   get pointer to coefficient structure
 * @param   void
 * @retval  pointer to date structure
 */
sQuadraticCoeffs_t *DCalibration::getCoefficients(void)
{
    DLock is_on(&myMutex);
    return (sQuadraticCoeffs_t *)&myCoefficients;
}

/**
 * @brief   set pointer to coefficient structure
 * @param   pointer to date structure
 * @retval  void
 */
void DCalibration::setCalDataAddr(sSensorData_t *ptrCalData)
{
    DLock is_on(&myMutex);
    myCalData = ptrCalData;
}

/**
 * @brief   set pointer to coefficient structure
 * @param   pointer to date structure
 * @retval  void
 */
sSensorData_t *DCalibration::getCalDataAddr(void)
{
    DLock is_on(&myMutex);
    return myCalData;
}
/**
 * @brief   Set calibration date
 * @param   date of calibration
 * @retval  void
 */
bool DCalibration::saveCalDate(sDate_t *date)
{
    bool flag = false;

    DLock is_on(&myMutex);

    //date is already validated - update it in sensor and/or persistent storage as well
    if(myCalData != NULL)
    {
        myCalData->data.date.day = date->day;
        myCalData->data.date.month = date->month;
        myCalData->data.date.year = date->year;

        //calculate new CRC value for sensor cal data as the cal range values will have changed
        myCalData->crc = crc32((uint8_t *)&myCalData->data, sizeof(sSensorCal_t));

        //save cal data for this sensor (which includes all ranges)
        flag = PV624->persistentStorage->saveCalibrationData((void *)myCalData, sizeof(sSensorData_t), E_PERSIST_CAL_DATA);
        //reload calibration from persistent storage
        loadCalibrationData();
    }

    return flag;
}

/**
 * @brief   Set validated calibration interval (in number of days)
 * @param   interval value
 * @retval  void
 */
bool DCalibration::saveCalInterval(uint32_t interval)
{
    bool flag = false;

    DLock is_on(&myMutex);

    //date is already validated - update it in sensor and/or persistent storage as well
    if(myCalData != NULL)
    {
        myCalData->data.calInterval = interval;

        //calculate new CRC value for sensor cal data as the cal range values will have changed
        myCalData->crc = crc32((uint8_t *)&myCalData->data, sizeof(sSensorCal_t));

        //save cal data for this sensor (which includes all ranges)
        flag = PV624->persistentStorage->saveCalibrationData((void *)myCalData, sizeof(sSensorData_t), E_PERSIST_CAL_DATA);

        //reload calibration from persistent storage
        loadCalibrationData();
    }

    return flag;
}

/**
 * @brief   Load calibration data from persistent storage
 * @param   void
 * @retval  true = success, false = failed
 */

bool DCalibration::loadCalibrationData(void)
{
    bool flag = true; //if a sensor has no cal data then just return true

    //myCaldata is pointer to cal data for this sensor
    if(myCalData != NULL)
    {
        //read from persistent storage the cal data for this sensor (which includes all ranges)
        flag = PV624->persistentStorage->loadCalibrationData((void *)myCalData, sizeof(sSensorData_t));

        //make sure ranges have up-to-date cal data
        flag &= validate(myCalData->data.numPoints);
    }

    return flag;
}


/**
 * @brief   Save calibration to persistent storage
 * @param   void
 * @retval  true = success, false = failed
 */
bool DCalibration::saveCalibrationData(void)
{
    bool flag = false;

    if(myCalData != NULL)
    {
        //TODO HSB: set cal interval to 0u
        myCalData->data.calInterval = 0u;

        //calculate new CRC value for sensor cal data as the cal range values will have changed
        myCalData->crc = crc32((uint8_t *)&myCalData->data, sizeof(sSensorCal_t));

        //save cal data for this sensor (which includes all ranges)
        flag = PV624->persistentStorage->saveCalibrationData((void *)myCalData, sizeof(sSensorData_t), E_PERSIST_CAL_DATA);
    }

    return flag;
}

/**
 * @brief   Get cal point
 * @param   point is the cal point number (starting at 1 ...)
 * @param   x is input value
 * @param   y is output value
 * @retval  true if accepted, else false
 */
bool DCalibration::getCalPoint(uint32_t point, float32_t *x, float32_t *y)
{
    bool flag = false;
    DLock is_on(&myMutex);

    //cal point must start at 1, so '0' is illegal
    if((myCalData != NULL) && (point > 0u))
    {
        //check index to calPoints array
        uint32_t index = point - 1u;

        if(index < (uint32_t)MAX_CAL_POINTS)
        {
            //update status to indicate that this cal point has been supplied
            myStatus.calPoints |= ((uint32_t)0x1u << index);

            *x = myCalData->data.calPoints[index].x;
            *y = myCalData->data.calPoints[index].y;

            flag = true;
        }
    }

    MISRAC_DISABLE
    assert(flag);
    MISRAC_ENABLE

    return flag;
}
