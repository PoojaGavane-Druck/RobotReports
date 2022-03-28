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
* @file     DSlotMeasurePressureExt.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlotMeasurePressureExt class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include "app_cfg.h"
MISRAC_ENABLE

#include "DPV624.h"
#include "DSlotMeasurePressureExt.h"
#include "DSensorOwiAmc.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
CPU_STK refSensorTaskStack[APP_CFG_REF_SENSOR_TASK_STACK_SIZE];
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlotMeasurePressureExt class constructor
 * @param   void
 * @retval  void
 */
DSlotMeasurePressureExt::DSlotMeasurePressureExt(DTask *owner)
    : DSlotExternal(owner)
{
    mySensor = new DSensorOwiAmc(OWI_INTERFACE_1);
    myName = "sExtP";
    myTaskId = ePM620Task;
}

/**
 * @brief   PM620 initialise function
 * @param   void
 * @retval  void
 */
void DSlotMeasurePressureExt::initialise(void)
{

}
/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */
void DSlotMeasurePressureExt::start(void)
{

    OS_ERR err;

    myTaskStack = (CPU_STK *)&refSensorTaskStack[0];

#ifdef ENABLE_STACK_MONITORING
    stackArray.uiStack.addr = (void *)myTaskStack;
    stackArray.uiStack.size = (uint32_t)(APP_CFG_REF_SENSOR_TASK_STACK_SIZE * 4u);
    fillStack((char *)myTaskStack, 0x22, (size_t)(APP_CFG_REF_SENSOR_TASK_STACK_SIZE * 4u));
#endif
    activate(myName, (CPU_STK_SIZE)APP_CFG_REF_SENSOR_TASK_STACK_SIZE, (OS_PRIO)3u, (OS_MSG_QTY)10u, &err);


}
