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
* @file     DFunctionMeasurePressureExt.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DFunctionMeasurePressureExt class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//#include <os.h>
//MISRAC_ENABLE
//
//#include "DDPI610E.h"

#include "DFunctionMeasurePressureExt.h"
#include "DSlotMeasurePressureExt.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DFunctionMeasurePressureExt class constructor
 * @param   void
 * @retval  void
 */
DFunctionMeasurePressureExt::DFunctionMeasurePressureExt(uint32_t index)
: DFunctionMeasure(index)
{
    myName = "fExtP";
    myFunction = E_FUNCTION_EXT_PRESSURE;

    //create the slots as appropriate for the instance
    createSlots();

    //events in addition to the default ones in the base class
    //myWaitFlags |= EV_FLAG_TASK_SENSOR_???;
}

/**
 * @brief   Create function slots
 * @param   void
 * @retval  void
 */
void DFunctionMeasurePressureExt::createSlots(void)
{
    //each instance overrides this function
    mySlot = new DSlotMeasurePressureExt(this);
}

/**
 * @brief   Get function settings
 * @param   void
 * @retval  void
 */
void DFunctionMeasurePressureExt::getSettingsData(void)
{
    //get address of data structure
    sPersistentFunctions_t *settings = PV624->persistentStorage->getFunctionSettingsAddr();

    //copy the data
    PV624->persistentStorage->getPersistentData((void *)&settings->data.measureExtP, (void *)&mySettings, sizeof(sFunctionSetting_t));
}
