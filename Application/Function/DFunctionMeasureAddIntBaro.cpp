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
* @file     DFunctionMeasureAddIntBaro.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     02 June 2020
*
* @brief    The DFunctionMeasureAddIntBaro class source file
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

#include "DFunctionMeasureAddIntBaro.h"
#include "DDPI610E.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DFunctionMeasureAddIntBaro class constructor
 * @param   void
 * @retval  void
 */
DFunctionMeasureAddIntBaro::DFunctionMeasureAddIntBaro(uint32_t index)
: DFunctionMeasure(index)
{
    myName = "fInt+Baro";
    myFunction = E_FUNCTION_ADD_INT_BARO;

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
void DFunctionMeasureAddIntBaro::createSlots(void)
{
//    //create a pseudo slot for deriving reading values
//    mySlot = new DSlotPseudo(this);
//
//    //create a primary slot, with it's owner being the pseudo slot
//    DSlot *primSlot = new DSlotMeasurePressureExt(mySlot);
//
//    //create a secondary slot, with it's owner being the pseudo slot
//    DSlot *secondarySlot = new DSlotMeasurePressureInt(mySlot);
//
//    //set up the pseudo slot with these two, specifying the association between them
//    ((DSlotPseudo *)mySlot)->addSlots(primSlot, secondarySlot, E_SLOT_ASSOC_DIFF);
}

/**
 * @brief   Get function settings
 * @param   void
 * @retval  void
 */
void DFunctionMeasureAddIntBaro::getSettingsData(void)
{
    //get address of data structure
    sPersistentFunctions_t *settings = DPI610E->persistentStorage->getFunctionSettingsAddr();

    //copy the data
    DPI610E->persistentStorage->getPersistentData((void *)&settings->data.measureSumIntBaro, (void *)&mySettings, sizeof(sFunctionSetting_t));
}
