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
* @file     DProcessMin.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The minimum value capture process class source file
*
**********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DProcessMin.h"

/* Constants and Defines --------------------------------------------------------------------------------------------*/

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProcessMin class constructor
 * @param   void
 * @retval  void
 */
DProcessMin::DProcessMin(uint32_t channelIndex)
    : DProcess(channelIndex)
{
    myProcessIndex = E_PROCESS_MINIMUM;

    //threshold is set to a very big positive number to force update from first reading onwards
    myMinimum = BIG_POSITIVE_NUMBER;
}

/**
 * @brief   Reset process
 * @param   void
 * @retval  void
 */
void DProcessMin::reset()
{
    DLock is_on(&myMutex);
    myMinimum = BIG_POSITIVE_NUMBER;
}

/**
 * @brief   Run process
 * @param   input value
 * @retval  input value (unchanged)
 */
float32_t DProcessMin::run(float32_t input)
{
    //only need to process if enabled
    if(myEnabledState == true)
    {
        //if greater than current minimum then set input as new minimum
        if(input < myMinimum)
        {
            myMinimum = input;
            notify(E_UI_MSG_NEW_MIN);
        }
    }

    return input;
}
