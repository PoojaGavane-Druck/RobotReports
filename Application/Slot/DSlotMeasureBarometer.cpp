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
#include "app_cfg.h"
MISRAC_ENABLE

#include "DPV624.h"
#include "DSlotMeasureBarometer.h"
#include "DSensorChipBarometer.h"

/* Error handler instance parameter starts from 5001 to 5100 */

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
CPU_STK barometerTaskStack[APP_CFG_BAROMETER_TASK_STACK_SIZE];
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
    myTaskId = eBarometerTask;
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

    myDefaultSampleInterval = 5000u;
    myMinSampleInterval = myDefaultSampleInterval;
    myCalSampleInterval = 250u;

}

/**
 * @brief   DSlotMeasureBarometer class destructor
 * @param   void
 * @retval  void
 */
DSlotMeasureBarometer::~DSlotMeasureBarometer(void)
{
    if(NULL != mySensor)
    {
        delete mySensor;
    }
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

    if(NULL == mySensor)
    {
        if((int)E_BAROMETER_NONE == barometerOptionType)
        {
            mySensor = NULL;
        }

        else if((int)E_CHIP_BAROMETER == barometerOptionType)
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

/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */
void DSlotMeasureBarometer::start(void)
{

    OS_ERR err;

    myTaskStack = (CPU_STK *)&barometerTaskStack[0];

#ifdef ENABLE_STACK_MONITORING
    stackArray.uiStack.addr = (void *)myTaskStack;
    stackArray.uiStack.size = (uint32_t)(APP_CFG_BAROMETER_TASK_STACK_SIZE * 4u);
    fillStack((char *)myTaskStack, 0x33, (size_t)(APP_CFG_BAROMETER_TASK_STACK_SIZE * 4u));
#endif

    activate(myName, (CPU_STK_SIZE)APP_CFG_BAROMETER_TASK_STACK_SIZE, (OS_PRIO)APP_CFG_BAROMETER_TASK_PRIO, (OS_MSG_QTY)APP_CFG_BAROMETER_TASK_MSG_QTY, &err);


}