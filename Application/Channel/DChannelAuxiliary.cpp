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
* @file     DChannelAuxiliary.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DChannelAuxiliary class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DChannelAuxiliary.h"

#include "DFunctionMeasurePressureExt.h"
#include "DFunctionMeasureBarometer.h"
#include "DFunctionMeasureAddExtIntP.h"
#include "DFunctionMeasureDiffExtIntP.h"
#include "DFunctionMeasureAddIntBaro.h"
#include "DFunctionMeasureDiffIntBaro.h"
#include "DFunctionMeasureAddExtBaro.h"
#include "DFunctionMeasureDiffExtBaro.h"


/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DChannelAuxiliary class constructor
 * @param   void
 * @retval  void
 */
DChannelAuxiliary::DChannelAuxiliary(uint32_t index)
: DChannel()
{
   
    myMeasureFunctions[E_FUNCTION_EXT_PRESSURE] = new DFunctionMeasurePressureExt(index);
   
    //myMeasureFunctions[E_FUNCTION_BAROMETER] = new DFunctionMeasureBarometer(index);
    //myMeasureFunctions[E_FUNCTION_HART] = new DFunctionMeasureHART(index);
    //myMeasureFunctions[E_FUNCTION_ADD_EXT_INT_P] = new DFunctionMeasureAddExtIntP(index);
    //myMeasureFunctions[E_FUNCTION_DIFF_EXT_INT_P] = new DFunctionMeasureDiffExtIntP(index);
//    myMeasureFunctions[E_FUNCTION_ADD_INT_BARO] = new DFunctionMeasureAddIntBaro(index);
//    myMeasureFunctions[E_FUNCTION_DIFF_INT_BARO] = new DFunctionMeasureDiffIntBaro(index);
//    myMeasureFunctions[E_FUNCTION_ADD_EXT_BARO] = new DFunctionMeasureAddExtBaro(index);
//    myMeasureFunctions[E_FUNCTION_DIFF_EXT_BARO] = new DFunctionMeasureDiffExtBaro(index);
}

