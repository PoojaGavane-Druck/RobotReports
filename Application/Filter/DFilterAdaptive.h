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
* @file     DFilterAdaptive.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     17 July 2020
*
* @brief    The sensor sample adaptive filter class header file
*/

#ifndef _DFILTER_ADAPTIVE_H
#define _DFILTER_ADAPTIVE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DFilter.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define ADAPTIVE_FILTER_OFF_VALUE 1E-6f //very small number; effectively no filtering, non-zero to prevent div/zero error

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DFilterAdaptive : public DFilter
{
protected:
    // Process values
    float32_t	myPrediction;	// predicted output
    float32_t	myError;		// error
    float32_t	myErrorInt;		// error with integrated term
    float32_t	myPreGain;		// pre-gain
    float32_t	myKalmanGain;   // Kalman gain
    float32_t	myOutput;		// output

public:
    float32_t myMeasurementNoise;

    DFilterAdaptive(void);

    virtual float32_t run(float32_t input);
    void setMeasurementNoise(float32_t noise);
};

#endif /* _DFILTER_ADAPTIVE_H */


