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
//#include "DCalibration.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/
/*********************************************************************************************************************/
//SUPPRESS: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma ("diag_suppress=Pm046")
/*********************************************************************************************************************/
#define  ISNAN(x) ((x) != (x))
/*********************************************************************************************************************/
//RESTORE: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma ("diag_default=Pm046")
/*********************************************************************************************************************/

/* Types ------------------------------------------------------------------------------------------------------------*/
//Instrument mode - all bit s0 means local mode, else remote or test as indicated by individual bits
//typedef union
//{
//    uint32_t value;
//
//    struct
//    {
//        uint32_t valid        : 1;  //cal is validated, ie crc checked
//        uint32_t none         : 1;  //no cal data specified
//        uint32_t usingDefault : 1;  //default cal being used (m = 1.0, c = 0.0)
//
//        uint32_t reserved     : 29; //available for more status bits as required
//    };
//
//} sCalStatus_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DCalibration
{
protected:
    void calculateCalDataFromCalPoints(void);
    void calculateCalDataFromMultiCalPoints(void);
    void sortCalPoints(sCalPoint_t *points);

    float32_t calculateMultipleCalPoint(float32_t x);

    void twoPointDataCalculate(sCalPoint_t* p1, sCalPoint_t* p2, sCalSegment_t* calData);
    float32_t calPointCalculate(sCalSegment_t* calData, float32_t x);
    float32_t calPointReverse(sCalSegment_t* calData, float32_t y);

public:
    //sCalStatus_t myStatus;
    sCalRange_t* myData;

    DCalibration(sCalRange_t* calData);

    float32_t calculate(float32_t x);
    float32_t reverse(float32_t y);

    void clearCalPoints(void);

    void clearAllCal();
    void clearCalData(void);

    bool setCalPoint(uint32_t index, float32_t x, float32_t y);

    float32_t getGain(uint32_t index);
    void setGain(uint32_t index, float32_t m);

    float32_t getOffset(uint32_t index);
    void setOffset(uint32_t  index, float32_t c);

    void setBreakpoint(uint32_t index, float32_t value);
    void getDate(sDate_t* date);
    void setDate(sDate_t* date);

    uint32_t getNumSegments(void);

    bool checkSanity(void);

    bool validate(uint32_t minPoints, uint32_t maxPoints);
};

#endif //__DCALIBRATION_H
