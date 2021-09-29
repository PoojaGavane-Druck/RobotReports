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
* @file     DErrorHandler.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 April 2020
*
* @brief    The error handler source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DErrorHandler.h"

MISRAC_DISABLE
#include "main.h"
MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
//#define SIMON_SAYS
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DErrorHandler class constructor
 * @param   comms is the reference to the comms instance for this sensor
 * @retval  void
 */
DErrorHandler::DErrorHandler(OS_ERR *os_error)
    : DTask()
{
    clearAllErrors();
    currentError.bytes = 0u;
}

/**
* @brief    handleError - process messaages from the rest of the system
* @param    errorCode - enumerated erro code identifier value
* @param    param -    optional parameter value TODO - Not currently used
* @param    blocking - optional parameter if true to stop here for showstopper faults TODO - blocking error won't cause watchdog to trip
* @return   None
*/
void DErrorHandler::handleError(error_code_t errorCode, int32_t param, bool blocking)
{
    currentError.bytes |= errorCode.bytes;

#ifdef SIMON_SAYS

#ifdef DEBUG
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); /* Red LED */
#endif

    if(blocking == true)
    {
#ifdef DEBUG
        HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin); /* Red LED */
        HAL_Delay(500u);
#endif
    }

#endif //SIMON_SAYS
}

/**
 * @brief Clears the specified error
 *
 * @param error_code_t errorCode
 * @return None
 */
void DErrorHandler::clearError(error_code_t errorCode)
{
    currentError.bytes &= ~(errorCode.bytes);

#ifdef SIMON_SAYS

#ifdef DEBUG

    if(currentError.bytes == RESET)
    {
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); /* Red LED */
    }

#endif

#endif //SIMON_SAYS
}

/**
 * @brief Clears all current errors
 *
 * @param None
 * @return error_code_t
 */
void DErrorHandler::clearAllErrors(void)
{


    currentError.bytes = RESET;
#ifdef TOGGLE_LED_ONERROR
    
#ifdef DEBUG
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); /* Red LED */
#endif
    
#endif   


}

/**
 * @brief: clear error log
 *
 *  @param None
 *  @return None
 */
void DErrorHandler::clearErrorLog(void)
{
//TODO:
}

/**
 * @brief: Get all current errors
 *
 * @param None
 * @return error_code_t
 */
error_code_t DErrorHandler::getErrors(void)
{
    return currentError;
}

