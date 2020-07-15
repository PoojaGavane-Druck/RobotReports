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
* @file     DFunctionMeasureBarometer.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DFunctionMeasureBarometer class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DFunctionMeasureBarometer.h"
#include "DSlotMeasureBarometer.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DFunctionMeasureBarometer class constructor
 * @param   void
 * @retval  void
 */
DFunctionMeasureBarometer::DFunctionMeasureBarometer(uint32_t index)
: DFunctionMeasure(index)
{
    myName = "fBaro";
    myFunction = E_FUNCTION_BAROMETER;

    //create the slots as appropriate for the instance
    createSlots();
}

/**
 * @brief   Create function slots
 * @param   void
 * @retval  void
 */
void DFunctionMeasureBarometer::createSlots(void)
{
    mySlot = new DSlotMeasureBarometer(this);
}


/**
 * @brief   Get function settings
 * @param   void
 * @retval  void
 */
void DFunctionMeasureBarometer::getSettingsData(void)
{
    //get address of data structure
    sPersistentFunctions_t *settings = PV624->persistentStorage->getFunctionSettingsAddr();

    //copy the data
    PV624->persistentStorage->getPersistentData((void *)&settings->data.measureBarometer, (void *)&mySettings, sizeof(sFunctionSetting_t));
}
