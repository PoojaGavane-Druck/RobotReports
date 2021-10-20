/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DSlotMeasureBarometer.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlotMeasureBarometer class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DSlotMeasureBarometer.h"
#include "DSensorChipBarometer.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlotMeasureBarometer class constructor
 * @param   void
 * @retval  void
 */
DSlotMeasureBarometer::DSlotMeasureBarometer(DTask *owner)
: DSlot(owner)
{
    myName = "sBaro";
    mySensor = NULL;
    barometerOptionType = E_BAROMETER_NONE;
    
    myWaitFlags |= EV_FLAG_TASK_SHUTDOWN |
              EV_FLAG_TASK_SLOT_CAL_SET_TYPE |
              EV_FLAG_TASK_SLOT_CAL_START_SAMPLING |
              EV_FLAG_TASK_CAL_SAMPLES_COUNT |
              EV_FLAG_TASK_SLOT_CAL_SET_POINT |
              EV_FLAG_TASK_SLOT_CAL_ACCEPT |
              EV_FLAG_TASK_SLOT_CAL_ABORT |
              EV_FLAG_TASK_SLOT_CAL_SET_DATE |
              EV_FLAG_TASK_SLOT_CAL_SET_INTERVAL |
              EV_FLAG_TASK_SLOT_SENSOR_CONTINUE |
              EV_FLAG_TASK_SLOT_SENSOR_RETRY |
              EV_FLAG_TASK_SLOT_CAL_RELOAD;
    
}

/**
 * @brief   Barometer initialise function
 * @param   void
 * @retval  void
 */
void DSlotMeasureBarometer::initialise(void)
{
    /* Hard coded for testing only - MAKARAND - TODO */
    barometerOptionType = E_CHIP_BAROMETER;
    
    if (NULL == mySensor)
    {
        if ((int)E_BAROMETER_NONE == barometerOptionType )
        {
          mySensor = NULL;
        }
        else if ((int)E_CHIP_BAROMETER == barometerOptionType)
        {
            mySensor = new DSensorChipBarometer();
            resume();
        }
        else if((int)E_TERPS_BAROMETER == barometerOptionType)
        {
          
        }
        else
        {
        }
    }
    
}
