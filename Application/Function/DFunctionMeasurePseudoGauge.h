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
* @file     DFunctionMeasurePseudoGauge.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DFunctionMeasurePseudoGauge base class header file
*/

#ifndef _DFUNCTION_MEASURE_PSEUDO_GAUGE_H
#define _DFUNCTION_MEASURE_PSEUDO_GAUGE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

#include "DFunctionMeasure.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DFunctionMeasurePseudoGauge : public DFunctionMeasure
{
protected:

public:
    DFunctionMeasurePseudoGauge(uint32_t index);
};

#endif // _DFUNCTION_MEASURE_PSEUDO_GAUGE_H
