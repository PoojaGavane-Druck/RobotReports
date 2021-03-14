/**
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DFilter.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     17 July 2020
*
* @brief    The sensor sample filter base class source file
*///*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DFilter.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DFilter class constructor
 * @param   void
 * @retval  void
 */
DFilter::DFilter(void)
{
    reset();
    enabled = true;
}

/**
 * @brief   Run filter
 * @param   input
 * @retval  output - filtered version of input
 */
float32_t DFilter::run(float32_t input)
{
    return input;
}

/**
 * @brief   Reset filter
 * @param   void
 * @retval  void
 */
void DFilter::reset(void)
{
    myReset = true;
}

/**
 * @brief   Get enabled state
 * @param   void
 * @retval  true if enabled, else false
 */
bool DFilter::getEnabled(void)
{
    return enabled;
}

/**
 * @brief   Set enabled state
 * @param   void
 * @retval  void
 */
void DFilter::setEnabled(bool state)
{
    enabled = state;
}

