/**
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DFilterAdaptive.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     17 July 2020
*
* @brief    The sensor sample adaptive filter class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DFilterAdaptive.h"
#include "Utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DFilterAdaptive class constructor
 * @param   void
 * @retval  void
 */
DFilterAdaptive::DFilterAdaptive(void)
    : DFilter()
{
    myOutput = 0.0f;
    myMeasurementNoise = ADAPTIVE_FILTER_OFF_VALUE;
    myPreGain = 0.0f;
    myError = 0.0f;
    myErrorInt = 0.0f;

    reset();
}

/**
 * @brief   Set measurement noise factor for adaptive filter
 * @param   noise is the value of the measurement noise factor
 * @retval  void
 */
void DFilterAdaptive::setMeasurementNoise(float32_t noise)
{
    myMeasurementNoise = noise;
}

/*********************************************************************************************************************/
//SUPPRESS: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma("diag_suppress=Pm046")
/*********************************************************************************************************************/
/**
 * @brief   Run filter
 * @param   input
 * @retval  output - filtered version of input
 */
float32_t DFilterAdaptive::run(float32_t input)
{
    if(enabled)
    {
        if(myReset)
        {
            myReset = false;

            myPreGain = 0.0f;
            myErrorInt = 0.0f;

            myOutput = input;
        }

        else
        {
            float32_t lastGain = myPreGain;
            float32_t cov = 0.0f;
            float32_t cov2 = 0.0f;

            myPrediction = myOutput;

            myError = myPrediction - input;

            cov = myError * myError;

            myPreGain = cov / (cov + myMeasurementNoise);

            myErrorInt = (myErrorInt * (1.0f - lastGain) * 0.8f) + (3.0f * myError);

            cov2 = myErrorInt * myErrorInt;

            myKalmanGain = cov2 / (cov2 + myMeasurementNoise);

            myOutput = (myKalmanGain * input) + (myPrediction * (1.0f - myKalmanGain));

            //TODO: sanity-check the result
            if(/*(isfinite(myOutput) == true)
                || (isfinite(myPreGain) == true)
                || (isfinite(myErrorInt) == true)
                || */ (ISNAN(myOutput) == true)
                || (ISNAN(myPreGain) == true)
                || (ISNAN(myErrorInt) == true))
            {
                myOutput = input;
                myPreGain = 0.0f;
                myErrorInt = 0.0f;
            }
        }
    }

    else
    {
        myOutput = input;
    }

    return myOutput;
}
/*********************************************************************************************************************/
//RESTORE: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma("diag_default=Pm046")
/*********************************************************************************************************************/
