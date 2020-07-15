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
* @file     DSensorBarometer.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     05 June 2020
*
* @brief    The barometer sensor base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensorChipBarometer.h"
#include "utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSensorChipBarometer class constructor
 * @param   comms is the reference to the comms instance for this sensor
 * @retval  void
 */
DSensorChipBarometer::DSensorChipBarometer()
: DSensor()
{
    //set up high-level sensor fullscale information
    myFsMinimum = 800.0f;    //stated calibrated -ve FS
    myFsMaximum = 1100.0f;   //stated calibrated +ve FS

    myAbsFsMinimum = myFsMinimum; //absolute minimum FS
    myAbsFsMaximum = myFsMaximum; //absolute maximum FS
}

/**
 * @brief   Initialisation function
 * @param   void
 * @retval  void
 */
eSensorError_t DSensorChipBarometer::initialise(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Close sensor
 * @param   void
 * @retval  void
 */
eSensorError_t DSensorChipBarometer::close(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Perform sensor measurement
 * @param   none
 * @retval  sensor error code
 */
eSensorError_t DSensorChipBarometer::measure()
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;

    float32_t measurement = 1013.25f + (0.25f * myRandomNumber());

    //apply compensation (user cal data)
    measurement = compensate(measurement);

    //update the variable
    setMeasurement(measurement);

    return sensorError;
}





