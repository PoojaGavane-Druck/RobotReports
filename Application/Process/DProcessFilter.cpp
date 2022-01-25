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
* @file     DProcessFilter.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The filter process class source file
*
**********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DProcessFilter.h"

MISRAC_DISABLE
#include <math.h>
MISRAC_ENABLE

/* Constants and Defines --------------------------------------------------------------------------------------------*/

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProcessFilter class constructor
 * @param   void
 * @retval  void
 */
DProcessFilter::DProcessFilter(uint32_t channelIndex)
    : DProcess(channelIndex)
{
    myProcessIndex = E_PROCESS_FILTER;

    myBand = 0.0f;
    myTimeConstant = 0.0f;
}

/**
 * @brief   Run process
 * @param   input value
 * @retval  filtered value
 */
float32_t DProcessFilter::run(float32_t input)
{
    float32_t output = input;

    //only process if enabled, else just pass through input value
    if(myEnabledState == true)
    {
        //save the new input value - in case it is needed anywhere else
        setInput(input);

        //get current filter output value
        output = getOutput();
        float32_t change = input - output;

        //check if change is within the step band
        if(fabsf(change) < myBand)
        {
            //a valid time constant is set then add a proportion of change only
            if(myTimeConstant > 0.0f)
            {
                output += (change / myTimeConstant);
            }

            else
            {
                //if time constant is 0 or less then don't
                output = input;
            }
        }

        else
        {
            //if change is greater than the filter band then don't filter it at all
            output = input;
        }

        //save the processed value too - in case it is needed anywhere else
        setOutput(output);
    }

    return output;
}

/**
 * @brief   Set process parameter (instance specific meaning)
 * @param   value is the tare value
 * @param   index: value 0 = band setting, 1 = time constant setting
 * @retval  void
 */
void DProcessFilter::setParameter(float32_t value, uint32_t index)
{
    DLock is_on(&myMutex);

    switch(index)
    {
    case 0u:
        myBand = value;
        break;

    case 1u:
        myTimeConstant = value;
        break;

    default:
        break;
    }
}

/**
 * @brief   Get process parameter (instance specific meaning)
 * @param   index is an identifier for the parameter
 * @retval  parameter value
 */
float32_t DProcessFilter::getParameter(uint32_t index)
{
    float32_t value;

    DLock is_on(&myMutex);

    switch(index)
    {
    case 0u:
        value = myBand;
        break;

    case 1u:
        value = myTimeConstant;
        break;

    default:
        break;
    }

    return value;
}

/**
 * @brief   Reset filter
 * @param   void
 * @retval  void
 */
void DProcessFilter::reset()
{
    //just set the input and output to be the same and start the filtering again from that point onwards
    DLock is_on(&myMutex);
    float32_t output = getOutput();
    setOutput(output);
}

