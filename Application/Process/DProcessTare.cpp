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
* @file     DProcessTare.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The tare process class source file
*
**********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DProcessTare.h"

/* Constants and Defines --------------------------------------------------------------------------------------------*/

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProcessTare class constructor
 * @param   void
 * @retval  void
 */
DProcessTare::DProcessTare(uint32_t channelIndex)
    : DProcess(channelIndex)
{
    myProcessIndex = E_PROCESS_TARE;

    myTareValue = 0.0f;
}

/**
 * @brief   Run process
 * @param   input value
 * @retval  processed value
 */
float32_t DProcessTare::run(float32_t input)
{
    float32_t output = input;

    //only process if enabled, else just pass through input value
    if(myEnabledState == true)
    {
        //save the new input value - in case it is needed anywhere else
        setInput(input);

        //subtract tare value
        output -= myTareValue;

        //save the processed value too - in case it is needed anywhere else
        setOutput(output);
    }

    return output;
}

/**
 * @brief   Set process parameter (instance specific meaning)
 * @param   value is the tare value
 * @param   index is ignored (recommended to be the dafault paramter value = 0)
 * @retval  void
 */
void DProcessTare::setParameter(float32_t value, uint32_t index)
{
    DLock is_on(&myMutex);
    myTareValue = value;
}

/**
 * @brief   Get process parameter (instance specific meaning)
 * @param   index is an identifier for the parameter
 * @retval  parameter value
 */
float32_t DProcessTare::getParameter(uint32_t index)
{
    DLock is_on(&myMutex);
    return myTareValue;
}

