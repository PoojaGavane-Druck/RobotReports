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
#include "DPV624.h"
#include "utilities.h"
#include "cLPS22HH.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define LPS22HH_DEVICE_ID       0XB3
//static OS_SEM LPS22HH_DRDY;
//static OS_ERR  err_Sen;
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSensorChipBarometer class constructor
 * @param   comms is the reference to the comms instance for this sensor
 * @retval  void
 */

DSensorChipBarometer::DSensorChipBarometer(): DSensor()
{
    // set sensor Type
    myType = E_SENSOR_TYPE_PRESS_BARO;
    //set up high-level sensor fullscale information
    myFsMinimum = 800.0f;    //stated calibrated -ve FS
    myFsMaximum = 1100.0f;   //stated calibrated +ve FS

    myAbsFsMinimum = myFsMinimum; //absolute minimum FS
    myAbsFsMaximum = myFsMaximum; //absolute maximum FS

    myLatency = 640u;               //TODO Nag: need to verify
    myCalSamplesRequired = 5u;      //number of cal samples at each cal point for averaging (arbitrary)

    //get calibration data from persistent storage - validated in the range instance
    //get address of data structure
    sCalData_t *calDataBlock = PV624->persistentStorage->getCalDataAddr();
    myCalData = new DCalibration(&calDataBlock->measureBarometer, myNumCalPoints, myResolution);



}

/**
 * @brief   Initialisation function
 * @param   void
 * @retval  sensor error codesensor error code
 */
eSensorError_t DSensorChipBarometer::initialise(void)
{
    eSensorError_t  sensorError = E_SENSOR_ERROR_NONE;
    sSensorStatus_t status;
    status.value = 0u;

    resetStatus();

    setMode(E_SENSOR_MODE_NORMAL);

    //load the calibration on initialisation
    //loadCalibrationData();

    bool flag = LPS22HH_initialise();

    //trigger first reading
    flag &= LPS22HH_trigger();

    if(flag == false)
    {
        status.fault = 1u;
        setStatus(status);

        sensorError = E_SENSOR_ERROR_FAULT;
    }

    else
    {
        setManfIdentity((uint32_t)(LPS22HH_DEVICE_ID));
    }

    return sensorError;
}
/**
 * @brief   Close sensor
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensorChipBarometer::close(void)
{
    eSensorError_t  sensorError = E_SENSOR_ERROR_NONE;

    if(LPS22HH_close() == false)
    {
        sensorError  = E_SENSOR_ERROR_FAULT;
    }

    return sensorError;
}


/**
 * @brief   Perform sensor measurement
 * @param   none
 * @retval  sensor error code
 */
eSensorError_t DSensorChipBarometer::measure(void)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;

    float32_t measurement = 0.0f;

    bool flag = LPS22HH_read(&measurement);

    if(flag == true)
    {
        //store measurement
        setValue(E_VAL_INDEX_RAW_VALUE, measurement);

        //In cal mode we do not apply calibration and do not allow autoranging. This is 'belt and braces' as even if
        //these functions are called they will not do anything in cal mode.
        eSensorMode_t sensorMode = getMode();

        if(sensorMode == (eSensorMode_t)E_SENSOR_MODE_CALIBRATION)
        {
            //add sample to accumulator
            addCalSample(measurement);
        }

        else
        {
            //apply compensation (user cal data)
            //measurement = compensate(measurement);
        }

        //update the variable
        setValue(E_VAL_INDEX_VALUE, measurement);
    }

    //trigger next reading
    flag &= LPS22HH_trigger();

    if(flag == false)
    {
        //signal error in sensor status
        sSensorStatus_t status;
        status.value = 0u;
        status.fault = 1u;
        setStatus(status);
        sensorError = E_SENSOR_ERROR_MEASUREMENT;
    }

    return sensorError;
}

