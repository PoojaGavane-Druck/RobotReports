/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DCalibration.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     18 May 2020
*
* @brief    The calibration data class header file
*/

#ifndef __DCALIBRATION_H
#define __DCALIBRATION_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
MISRAC_ENABLE

#include "Types.h"
#include "PersistentCal.h"
/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
//Calibration status
typedef union
{
    uint32_t value;

    struct
    {
        uint32_t valid       : 1;      //1 = valid cal data, 0 = invalid cal data (using defaults)
        uint32_t ignore      : 1;      //1 = do not apply calibration data, 0 = apply calibration data
        uint32_t calPoints   : 4;      //4-bit field used to keep track of cal points entered when performing new calibration

        uint32_t reserved    : 26;     //unused - available for future use
    };

} sCalStatus_t;

typedef struct
{
    float32_t a;    //quadratic calculation coefficient of the squared term
    float32_t b;    //quadratic calculation coefficient of the linear term
    float32_t c;    //quadratic calculation coefficient of the constant/offset term

} sQuadraticCoeffs_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DCalibration
{
protected:
    OS_MUTEX myMutex;                   //resource lock
    sCalRange_t* myData;                //data from persistent storage
    sCalStatus_t myStatus;              //cal data status
    sQuadraticCoeffs_t myCoefficients;  //quadratic coefficients
    float32_t myConvergenceLimit;       //used for inverse calculation by successive approximation

    void sortCalPoints(sCalPoint_t *points, uint32_t numPoints);
    bool determineQuadraticCoefficients(void);

public:
    DCalibration(sCalRange_t* calData, uint32_t numCalPoints, float32_t convergenceLimit);

    bool validate(uint32_t numCalPoints);
    void clear(void);
    bool calculateCoefficients(void);
    sQuadraticCoeffs_t *getCoefficients(void);

    bool load(sCalRange_t* calData, uint32_t numCalPoints);
    void apply(void);
    void revert(void);

    void setCalIgnore(bool state);
    bool isValidated(void);
    bool hasCalData(void);

    float32_t calculate(float32_t x);
    float32_t reverse(float32_t y);
    float32_t inverseCalculate(float32_t y);

    void calInitialise(void);
    bool getCalComplete(uint32_t numCalPoints);
    bool setNumCalPoints(uint32_t numCalPoints);
    bool setCalPoint(uint32_t index, float32_t x, float32_t y);

    void getDate(sDate_t* date);
    void setDate(sDate_t* date);
};

#endif //__DCALIBRATION_H
