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
* @file     DRange.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     17 April 2020
*
* @brief    The sensor range base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DRange.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DRange class constructor
 * @param   min - lower fullscale of this range
 * @param   max - upper fullscale of this range
 * @param   resolution - resolution
 * @param   calPoints - number of cal points this range requires (0 means doesn't require calibration)
 * @param   caldata - pointer to calibration data structure for this range (default paramter value is NULL)
 * @retval  void
 */
DRange::DRange(float32_t min, float32_t max, float32_t resolution, uint32_t calPoints, sCalRange_t *calData)
{
    myMin = min;                //set lower fullscale of this range
    myMax = max;                //set upper fullscale of this range

    myMinAutoRange = min;       //set threshold same as fullscale (can be changed later)
    myMaxAutoRange = max;       //set threshold same as fullscale (can be changed later)

	myResolution = resolution;
	myScalingFactor = 1.0f;

    myCal = new DCalibration(calData);

	myCalPoints = calPoints;    //set no of cal points required

    usingDefaultCal = true;     //assume default cal until a valid cal is loaded, if available
}

/**
 * @brief   Get resolution
 * @param   void
 * @retval  resolution for this range
 */
float32_t DRange::getResolution(void)
{
    return myResolution;
}

/**
 * @brief   Get value of the lower threshold for autoranging
 * @note    This is the value below which the owning sensor should switch to a lower range if available
 * @param   void
 * @retval  threshold value
 */
float32_t DRange::getMinAutoRange(void)
{
    return myMinAutoRange;
}

/**
 * @brief   Set value of the lower threshold for autoranging.
 * @note    This is the value below which the owning sensor should switch to a lower range if available
 * @param   threshold value
 * @retval  void
 */
void DRange::setMinAutoRange(float32_t min)
{
    myMinAutoRange = min;
}

/**
 * @brief   Get value of the upper threshold for autoranging.
 * @note    This is the value above which the owning sensor should switch to a higher range if available
 * @param   void
 * @retval  threshold value
 */
float32_t DRange::getMaxAutoRange(void)
{
    return myMaxAutoRange;
}

/**
 * @brief   Set value of the upper threshold for autoranging.
 * @note    This is the value above which the owning sensor should switch to a higher range if available
 * @param   threshold value
 * @retval  void
 */
void DRange::setMaxAutoRange(float32_t max)
{
    myMaxAutoRange = max;
}

/**
 * @brief   Set value of the upper threshold for autoranging.
 * @note    This is the value above which the owning sensor should switch to a higher range if available
 * @param   threshold value
 * @retval  void
 */
DCalibration* DRange::getCalibration(void)
{
    //TODO: Resource locks
    return myCal;
}




