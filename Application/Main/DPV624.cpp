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
* @file     DPV624.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     12 April 2020
*
* @brief    The PV624 instrument class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include "main.h"
#include <os.h>
#include <assert.h>
MISRAC_ENABLE

#include "DPV624.h"
#include "DSlot.h"
#include "i2c.h"
#include "uart.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
//extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
DPV624 *PV624;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
#include "Utilities.h"

DPV624::DPV624(void)
{
    OS_ERR os_error;
    
    //create devices
    /* Commenting these objects for testing commsOwi */
   
   
    
    persistentStorage = new DPersistent();
    
    //initialise I2C interface (must do this before accessing I2C devices)
#ifdef CONTROLLER_BOARD
    i2cInit(&hi2c1);
#endif
    
#ifdef NUCLEO_BOARD
    i2cInit(&hi2c2);
#else
    i2cInit(&hi2c4);
#endif    
    
    
    uartInit(&huart2);
    uartInit(&huart4);  
  
    
    //create application objects
    realTimeClock = new DRtc();
    instrument = new DInstrument(&os_error);
    validateApplicationObject(os_error);

    commsOwi = new DCommsOwi("commsOwi", &os_error);
    validateApplicationObject(os_error);

    commsUSB = new DCommsUSB("commsUSB", &os_error);
    validateApplicationObject(os_error);

    powerManager = new DPowerManager();
   
    errorHandler = new DErrorHandler(&os_error);
    validateApplicationObject(os_error);

    keyHandler = new DKeyHandler(&os_error);
    validateApplicationObject(os_error);



    /*
    errorHandler = new DErrorHandler(&os_error);
    keyHandler = new DKeyHandler(&os_error);
    userInterface = new DUserInterface(&os_error);
    serialComms = new DCommsSerial("commsSerial", &os_error);
    
    */
    
    //Todo Added for Testing by Nag
     mySerialNumber = 10101111u;
}

/**
* @brief    handleError - process messaages from the rest of the system
* @param    errorCode - enumerated error code identifier value
* @param    param -    optional parameter value TODO - Not currently used
* @param    blocking - optional parameter if true to stop here for showstopper faults TODO - blocking error won't cause watchdog to trip
* @return   None
*/
void DPV624::handleError(error_code_t errorCode, int32_t param, bool blocking)
{
    if (errorHandler != NULL)
    {
        errorHandler->handleError(errorCode, param, blocking);
    }
}
/**
 * @brief   validates the application object was created without error
 * @retval  None
 */
void DPV624::validateApplicationObject(OS_ERR os_error)
{
    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE));

    if (!ok)
    {
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE

        if ((PV624 != NULL) && (errorHandler != NULL))
        {
            error_code_t errorCode;
            errorCode.bit.osError = SET;
            PV624->handleError(errorCode, os_error);
        }
        else
        {
            // Resort to global error handler
            Error_Handler();
        }
    }
}

/**
 * @brief   Get serial number
 * @note    Instrument "serial number" if actually a free format ASCII character string
 * @param   void
 * @retval  character string
 */
uint32_t DPV624::getSerialNumber(void)
{
    sConfig_t *configData = persistentStorage->getConfigDataAddr();
    return configData->serialNumber;
}

/**
 * @brief   Set serial number
 * @note    Instrument "serial number" if actually a free format ASCII character string
 * @param   str - string
 * @retval  true = success, false = failed
 */
bool DPV624::setSerialNumber(uint32_t newSerialNumber)
{
    bool flag = false;

    if (newSerialNumber != 0XFFFFFFFFu) //one byte less for null terminator
    {
        //update string value
        sConfig_t *configData = persistentStorage->getConfigDataAddr();
        
        //save in persistent storage
        if (persistentStorage->saveConfigData((void *)&configData->serialNumber, (size_t)INSTRUMENT_ID_SIZE) == true)
        {
            flag = true;
        }
    }

    return flag;
}

/**
 * @brief   Get region of use
 * @param   void
 * @retval  current setting for region of use
 */
eRegionOfUse_t DPV624::getRegion(void)
{
    return (eRegionOfUse_t)0u;
}

/**
 * @brief   Set region of use
 * @param   region - are of use
 * @retval  true = success, false = failed
 */
bool DPV624::setRegion(eRegionOfUse_t region)
{
    return false;
}
