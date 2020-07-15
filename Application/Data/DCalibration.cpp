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
* @file     DCalibration.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     18 May 2020
*
* @brief    The calibration base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCalibration.h"
#include "Utilities.h"
#include "PersistentCal.h"

//MISRAC_DISABLE
//#include <math.h>
//MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCalibration class constructor
 * @param   pointer to calibration data structure
 * @retval  void
 */
DCalibration::DCalibration(sCalRange_t* calData)
{
	myData = calData;
}

/**
 * @brief   Calculates the CalData from the data points
 * @param   void
 * @retval  void
 */
void DCalibration::calculateCalDataFromCalPoints(void)
{
    if (myData != NULL)
    {
        sCalSegment_t* calData = &myData->segments[0];
        sCalPoint_t* calPoint = &myData->calPoints[0];

        switch(myData->numPoints)
        {
            case 0:		// No calibration
                myData->numSegments = 0u;
                calData->m = 1.0f;
                calData->c = 0.0f;
                break;

            case 1:		// Offset calibration
                myData->numSegments = 1u;
                calData->m = 1.0f;
                calData->c = calPoint->y - calPoint->x;
                break;

            default:
                calculateCalDataFromMultiCalPoints();
                break;
        }
    }
}

/**
 * @brief   Calculate cal data coefficients where there is more than one calibration point
 * @param   void
 * @retval  void
 */
void DCalibration::calculateCalDataFromMultiCalPoints(void)
{
	if (myData != NULL)
    {
        uint32_t i;

        myData->numSegments = myData->numPoints - 1u;

        //sort in ascending order of input values
        sortCalPoints(&myData->calPoints[0]);

        //set breakpoint between segments
        for (i = 0u; i < (myData->numPoints - 2u); i++)
        {
            myData->breakpoint[i] = myData->calPoints[i + 1u].x;
        }

        //do the y = m x + c calculation for each segment
        for (i = 0u; i < myData->numSegments; i++)
        {
            twoPointDataCalculate(&myData->calPoints[i], &myData->calPoints[i + 1u], &myData->segments[i]);
        }
    }
}

/**
 * @brief   Sort all of the calibration points into accending order according to input value
 * @param   pointer to cal points
 * @retval  void
 */
void DCalibration::sortCalPoints(sCalPoint_t *points)
{
	if (myData != NULL)
    {
        float32_t fTemp;

        //sort the cal points in ascending input value order
        for (uint32_t i = 0u; i < (myData->numPoints - 1u); i++)
        {
            for (uint32_t j = 1u; j < (myData->numPoints - i); j++)
            {
                if (points[i].x > points[i+j].x)
                {
                    fTemp = points[i].x;
                    points[i].x = points[i+j].x;
                    points[i+j].x = fTemp;

                    fTemp = points[i].y;
                    points[i].y = points[i+j].y;
                    points[i+j].y = fTemp;
                }
            }
        }
    }
}

/**
 * @brief   Calculate compensated value using appropriate cal for multiple points
 * @param   uncompensated value
 * @retval  compensated value
 */
float32_t DCalibration::calculateMultipleCalPoint(float32_t x)
{
    uint32_t seg = 0u;

	while ((seg < (myData->numSegments - 1u)) && (x > myData->breakpoint[seg]))
    {
        seg++;
    }

	return calPointCalculate(&myData->segments[seg], x);
}

/**
 * @brief   Calculate the gain and offset relative to two calibration points
 * @param   p1 is first calibration point
 * @param   p2 is second calibration point
 * @param   calData is pointer to calibration data structure to update
 * @retval  void
 */
void DCalibration::twoPointDataCalculate(sCalPoint_t* p1, sCalPoint_t* p2, sCalSegment_t* calData)
{
   float32_t dx = (p2->x  - p1->x);

   //guard against divide-by-zero
   if (floatEqual(dx, 0.0f) == false)
   {
       calData->m = 1.0f;
       calData->c = 0.0f;
   }
   else
   {
       calData->m = (p2->y  - p1->y)/dx;
       calData->c = p1->y - (calData->m * p1->x);
   }
}

/**
 * @brief   Perform reverse calculation for calibration point
 * @param   calibration gain & offest data to use
 * @param   uncompensated value
 * @retval  compensated value
 */
float32_t DCalibration::calPointCalculate(sCalSegment_t* calData, float32_t x)
{
    float32_t y = x;

	if (myData != NULL)
    {
        y = (calData->m * x) + calData->c;
    }

	return y;
}

/**
 * @brief   Perform reverse calculation for calibration point
 * @param   calibration gain & offest data to use
 * @param   compensated value
 * @retval  uncompensated value
 */
float32_t DCalibration::calPointReverse(sCalSegment_t* calData, float32_t y)
{
    float32_t x = y;

	if (myData != NULL)
    {
        x = (y - calData->c) / calData->m;
    }

	return x;
}

/**
 * @brief   Determine which cal segment to use and translate supplied uncalibrated value given to calibrated value
 * @param   uncalibrated value
 * @retval  calibrated value
 */
float32_t DCalibration::calculate(float32_t x)
{
    float32_t y = x;

	if (myData != NULL)
    {
        switch (myData->numSegments)
        {
            case 0: //no segments
                //leave as y = x;
                break;

            case 1: //1 segment, so use first index (0)
                y = calPointCalculate(&myData->segments[0], x);
                break;

            default: //more than 1 segment
                y = calculateMultipleCalPoint(x);
                break;
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

	if (myData != NULL)
    {
        //check how many straight line segments there are in this cal data
        switch(myData->numSegments)
        {
            case 0: //no segments
                //leave x = y;
                break;

            case 1: //1 segment, so use first index (0)
                x = calPointReverse(&myData->segments[0], y);
                break;

            default: //more than 1 segment
                {
                    //work out which segment to use
                    uint32_t seg = 0u;

                    float bp = calPointCalculate(&myData->segments[seg], myData->breakpoint[seg]);

                    while ((y > bp) && (seg < (myData->numSegments - 1u)) )
                    {
                        seg++;
                        bp = calPointCalculate(&myData->segments[seg], myData->breakpoint[seg]);
                    }

                    //use the right segment
                    x = calPointReverse(&myData->segments[seg], y);
                }
                break;
        }
    }

    return x;
}

/**
 * @brief   Clear all cal data
 * @param   void
 * @retval  void
 */
void DCalibration::clearAllCal()
{
	if (myData != NULL)
    {
        myData->date.day = 0u;
        myData->date.month = 0u;
        myData->date.year = 0u;

        clearCalPoints();
        clearCalData();
    }
}

/**
 * @brief   Set cal point and perform cal data calculation as a result of this
 * @param   point is the cal point number (starting at 1 ...)
 * @param   x is input value
 * @param   y is output value
 * @retval  void
 */
bool DCalibration::setCalPoint(uint32_t point, float32_t x, float32_t y)
{
    bool flag = true;

    //cal point must start at 1, so '0' is illegal
	if ((myData != NULL) && (point > 0u))
    {
        //check index to calPoints array
        uint32_t index = point - 1u;

        if (index < (uint32_t)MAX_CAL_POINTS)
        {
            myData->numPoints = point;

            myData->calPoints[index].x = x;
            myData->calPoints[index].y = y;

            calculateCalDataFromCalPoints();
        }
        else
        {
            flag = false;
        }
    }

    return flag;
}

/**
 * @brief   Clear all cal points
 * @param   void
 * @retval  void
 */
void DCalibration::clearCalPoints(void)
{
	if (myData != NULL)
    {
        myData->numPoints = 0u;
        myData->numSegments = 0u;

        for (int i = 0; i < MAX_CAL_POINTS; i++)
        {
            myData->calPoints[i].x = 0.0f;
            myData->calPoints[i].y = 0.0f;
        }
    }
}

/**
 * @brief   Clear cal data
 * @param   void
 * @retval  void
 */
void DCalibration::clearCalData()
{
	if (myData != NULL)
    {
        for (int i = 0; i < (MAX_CAL_POINTS - 1); i++)
        {
            myData->segments[i].m = 1.0f;
            myData->segments[i].c = 0.0f;

            if (i < (MAX_CAL_POINTS - 1))
            {
                myData->breakpoint[i] = 0.0f;
            }
        }
    }
}

/**
 * @brief   Get cal gain value (the 'm' in 'y = mx + c')
 * @param   index to calData array
 * @retval  gain value
 */
float32_t DCalibration::getGain(uint32_t index)
{
    float32_t m = 0.0f;

	if ((myData != NULL) && (index < (uint32_t)(MAX_CAL_POINTS - 1)))
	{
        m = myData->segments[index].m;
    }

    return m;
}

/**
 * @brief   Set cal gain value (the 'm' in 'y = mx + c')
 * @param   index to calData array
 * @param   gain value to set
 * @retval  void
 */
void DCalibration::setGain(uint32_t index, float32_t m)
{
	if ((myData != NULL) && (index < (uint32_t)(MAX_CAL_POINTS - 1)))
	{
        myData->segments[index].m = m;
    }
}

/**
 * @brief   Get cal offset value (the 'c' in 'y = mx + c')
 * @param   index to calData array
 * @retval  offset value
 */
float32_t DCalibration::getOffset(uint32_t index)
{
    float32_t c = 0.0f;

	if ((myData != NULL) && (index < (uint32_t)(MAX_CAL_POINTS - 1)))
	{
        c = myData->segments[index].c;
    }

    return c;
}

/**
 * @brief   Set cal offset value (the 'c' in 'y = mx + c')
 * @param   index to calData array
 * @param   offset value to set
 * @retval  void
 */
void DCalibration::setOffset(uint32_t  index, float32_t c)
{
	if ((myData != NULL) && (index < (uint32_t)(MAX_CAL_POINTS - 1)))
	{
        myData->segments[index].c = c;
    }
}

/**
 * @brief   Set calibration segment breakpoint
 * @param   index to breakpoint array
 * @param   value to set
 * @retval  void
 */
void DCalibration::setBreakpoint(uint32_t index, float32_t value)
{
	if ((myData != NULL) && (index < (uint32_t)(MAX_CAL_POINTS - 2)))
	{
        myData->breakpoint[index] = value;
    }
}

/**
 * @brief   Get date
 * @param   pointer to date structure for return value
 * @retval  void
 */
void DCalibration::getDate(sDate_t* date)
{
	if (myData != NULL)
    {
        date->day = myData->date.day;
        date->month = myData->date.month;
        date->year = myData->date.year;
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
void DCalibration::setDate(sDate_t* date)
{
	if (myData != NULL)
    {
        myData->date.day = date->day;
        myData->date.month = date->month;
        myData->date.year = date->year;
    }
}

/**
 * @brief   Get number of cal segments
 * @param   void
 * @retval  number of segments
 */
uint32_t DCalibration::getNumSegments(void)
{
	uint32_t numSegments = 0u;

	if (myData != NULL)
	{
		numSegments = myData->numSegments;
	}

	return numSegments;
}

/*********************************************************************************************************************/
 //SUPPRESS: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma ("diag_suppress=Pm046")
/*********************************************************************************************************************/
/**
 * @brief   Check cal data does not contain invalid values
 * @param   void
 * @retval  true if ok, else false
 */
bool DCalibration::checkSanity(void)
{
    bool flag = true;
	uint32_t i;

	uint32_t numPoints = (uint32_t) MAX_CAL_POINTS;
	uint32_t numSegments = (uint32_t)(MAX_CAL_POINTS - 1);

    //bad data if num of cal points or segments is invalid
	if ((myData->numPoints > numPoints) || (myData->numSegments > numSegments))
	{
		flag = false;
	}

    //bad data if any cal point values are invalid numbers
	for (i = 0u; (flag == true) && (i < numPoints); i++)
	{
		if ((ISNAN(myData->calPoints[i].x) == true) || (ISNAN(myData->calPoints[i].y) == true))
		{
			flag = false;
		}
	}

    //bad data if any cal point values are invalid numbers
	for (i = 0u; (flag == true) && (i < numSegments); i++)
	{
		if ((ISNAN(myData->segments[i].m) == true) || (ISNAN(myData->segments[i].c) == true))
		{
			flag = false;
		}
	}

    //bad data if breakpoint value(s) invalid
	for (i = 0u; (flag == true) && (i < (numSegments - 1u)); i++)
	{
		if (ISNAN(myData->breakpoint[i]) == true)
		{
			flag = false;
		}
	}

	return flag;
}
/*********************************************************************************************************************/
 //RESTORE: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma ("diag_default=Pm046")
/*********************************************************************************************************************/

/**
 * @brief   Validate & sanity check cal data
 * @param   minimum number of cal points required
 * @param   maximum number of cal points required
 * @retval  true if valid data, else false
 */
bool DCalibration::validate(uint32_t minPoints, uint32_t maxPoints)
{
	bool valid = true;

	if (checkSanity() == true)
	{
		uint32_t minSegments = 1u;

        if (minPoints > 1u)
        {
            minSegments = minPoints - 1u;
        }

		uint32_t maxSegments = 1u;

        if (maxPoints > 1u)
        {
            minSegments = maxPoints - 1u;
        }

		uint32_t actualSegments = getNumSegments();

		if ((actualSegments < minSegments) || (actualSegments > maxSegments))
		{
			valid = false;
		}
	}
	else
	{
		valid = false;
	}

	return valid;
}
