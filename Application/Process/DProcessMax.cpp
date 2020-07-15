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
* @file     DProcessMax.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The maximum value capture process class source file
*
**********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DProcessMax.h"

/* Constants and Defines --------------------------------------------------------------------------------------------*/

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProcessMax class constructor
 * @param   void
 * @retval  void
 */
DProcessMax::DProcessMax(uint32_t channelIndex)
: DProcess(channelIndex)
{
    myProcessIndex = E_PROCESS_MAXIMUM;

    //threshold is set to a very big negative number to force update from first reading onwards
    myMaximum = BIG_NEGATIVE_NUMBER;
}

/**
 * @brief   Reset process
 * @param   void
 * @retval  void
 */
void DProcessMax::reset()
{
    DLock is_on(&myMutex);
    myMaximum = BIG_NEGATIVE_NUMBER;
}

/**
 * @brief   Run process
 * @param   input value
 * @retval  input value (unchanged)
 */
float32_t DProcessMax::run(float32_t input)
{
    //only need to process if enabled
    if (myEnabledState == true)
    {
        //if greater than current maximum then set input as new maximum
        if (input > myMaximum)
        {
            myMaximum = input;
            notify(E_UI_MSG_NEW_MAX);
        }
    }

	return input;
}
