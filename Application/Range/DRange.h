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
* @file     DRange.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     17 April 2020
*
* @brief    The sensor range base class header file
*/

#ifndef __DRANGE_H
#define __DRANGE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "Types.h"
#include "DCalibration.h"
//#include "cAnalogCH1.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DRange
{
protected:
    float32_t myMin;         //lower fullscale of this range
    float32_t myMax;         //upper fullscale of this range

    float32_t myMinAutoRange;     //threshold at which sensor may switch to a lower range, if available
    float32_t myMaxAutoRange;     //threshold at which sensor may switch to a higher range, if available

    float32_t myResolution;

 
    float32_t myScalingFactor;

    DCalibration* myCal;

    uint32_t myCalPoints;

    bool usingDefaultCal;

public:
    DRange(float32_t min, float32_t max, float32_t resolution, uint32_t calPoints, sCalRange_t *calData = NULL);



    float32_t getResolution(void);

    float32_t getMinAutoRange(void);
    void setMinAutoRange(float32_t min);

    float32_t getMaxAutoRange(void);
    void setMaxAutoRange(float32_t max);

    DCalibration* getCalibration(void);
};

#endif /* __DRANGE_H */
