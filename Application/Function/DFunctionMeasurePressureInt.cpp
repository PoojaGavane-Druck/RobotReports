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
* @file     DFunctionMeasurePressureInt.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DFunctionMeasurePressureInt class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <os.h>
MISRAC_ENABLE

#include "DFunctionMeasurePressureInt.h"
#include "DSlotMeasurePressureInt.h"
#include "DDPI610E.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DFunctionMeasurePressureInt class constructor
 * @param   void
 * @retval  void
 */
DFunctionMeasurePressureInt::DFunctionMeasurePressureInt(uint32_t index)
: DFunctionMeasure(index)
{
    myName = "fIntP";
    myFunction = E_FUNCTION_INT_PRESSURE;

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
void DFunctionMeasurePressureInt::createSlots(void)
{
    //each instance overrides this function
    mySlot = new DSlotMeasurePressureInt(this);
}

/**
 * @brief   Get function settings
 * @param   void
 * @retval  void
 */
void DFunctionMeasurePressureInt::getSettingsData(void)
{
    //get address of data structure
    sPersistentFunctions_t *settings = DPI610E->persistentStorage->getFunctionSettingsAddr();

    //copy the data
    DPI610E->persistentStorage->getPersistentData((void *)&settings->data.measureIntP, (void *)&mySettings, sizeof(sFunctionSetting_t));
}
