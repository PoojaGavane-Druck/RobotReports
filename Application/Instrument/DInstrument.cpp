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
* @file     DInstrument.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The instrument class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DInstrument.h"
#include "DFunctionMeasureAndControl.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DInstrument class constructor
 * @param   void
 * @retval  void
 */
DInstrument::DInstrument(OS_ERR *osErr)
{  
   myCurrentFunction = new DFunctionMeasureAndControl();

    *osErr = (OS_ERR)OS_ERR_NONE;
}

/**
 * @brief   Set Instrument function
 * @param   chan is the channel of the function to run
 * @param   func is the function itself
 * @param   dir is the measure/source specification
 * @retval  true if activated successfully, else false
 */
bool DInstrument::setFunction( eFunction_t func)
{
    bool successFlag = false;

    if (func < (eFunction_t)E_FUNCTION_MAX)
    {
        successFlag = myCurrentFunction->setFunction(func);
    }

    return successFlag;
}


/**
 * @brief   Get specified value of currently running function
 * @param   chan is the channel
 * @param   index is function specific meaning identified a specific output parameter
 * @param   pointer to variable for return of value
 * @retval  true if all's well, else false
 */
bool DInstrument::getReading( eValueIndex_t index, float32_t *reading)
{
    bool successFlag = false;

    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(index, reading);
    }

    return successFlag;
}

/**
 * @brief   Signal sensor to continue
 * @param   chan is the channel
 * @retval  true if all's well, else false
 */
bool DInstrument::sensorContinue(void)
{
    bool successFlag = false;

    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->sensorContinue();
    }

    return successFlag;
}

/**
 * @brief   Signal sensor to retry after failure reported
 * @param   chan is the channel
 * @retval  true if all's well, else false
 */
bool DInstrument::sensorRetry(void)
{
    bool successFlag = false;

    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->sensorRetry();
    }

    return successFlag;
}
/**
 * @brief   Get positive fullscale of channel function
 * @param   channel - instrument channel
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getPosFullscale( float32_t *fs)
{
    bool successFlag = false;

    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_SENSOR_POS_FS, fs);
    }

    return successFlag;   
}

/**
 * @brief   Get positive fullscale of channel function
 * @param   channel - instrument channel
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getNegFullscale( float32_t *fs)
{
    bool successFlag = false;

    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_SENSOR_NEG_FS, fs);
    }

    return successFlag;     
   
}

/**
 * @brief   Get positive fullscale of channel function
 * @param   channel - instrument channel
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
 bool DInstrument::getSensorType(eSensorType_t *pSenType)
 {
    bool successFlag = false;

    if (myCurrentFunction != NULL)
    {
        if(NULL != pSenType)
        {
          myCurrentFunction->getValue(E_VAL_INDEX_SENSOR_TYPE, (uint32_t*)pSenType);
          successFlag = true;
        }
    }

    return successFlag;
 }

/**
 * @brief   Get positive fullscale of channel function
 * @param   channel - instrument channel
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getManufactureDate(sDate_t *manfDate)
{
    bool successFlag = false;

    if (myCurrentFunction != NULL)
    {
        myCurrentFunction->getManufactureDate(manfDate);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Get positive fullscale of channel function
 * @param   channel - instrument channel
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DInstrument::getUserCalDate( sDate_t* caldate)
{
    bool successFlag = false;

    if (myCurrentFunction != NULL)
    {
        myCurrentFunction->getCalDate((eSensorCalType_t)E_SENSOR_CAL_TYPE_USER, 
                                      caldate);
        successFlag = true;
    }

    return successFlag;

    
}

/**
 * @brief   Get Barometer Manufacture ID
 * @param   identity - Barometer Identity
 * @retval  true = success, false = failed
 */
bool DInstrument::getBarometerIdentity( uint32_t *identity)
{
    bool successFlag = false;

    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(EVAL_INDEX_BAROMETER_ID,
                                                  identity);
    }

    return successFlag;
}

/**
 * @brief   Get PM620 sendor App Identity ID
 * @param   identity - 
 * @retval  true = success, false = failed
 */
bool DInstrument::getExternalSensorAppIdentity(uSensorIdentity_t *identity)
{
    bool successFlag = false;
    uint32_t val = 0u;
    
    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_PM620_APP_IDENTITY,
                                                  &val);
        if(true == successFlag)
        {
          identity->value = val;
        }
        else
        {
          identity->value = 0u;
        }
    }

    return successFlag;  
}

/**
 * @brief   Get PM620 sendor Bootloader Identity ID
 * @param   identity - 
 * @retval  true = success, false = failed
 */
bool DInstrument::getExternalSensorBootLoaderIdentity(uSensorIdentity_t *identity)
{
    bool successFlag = false;
    uint32_t val = 0u;
    
    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_PM620_BL_IDENTITY,
                                                  &val);
        if(true == successFlag)
        {
          identity->value = val;
        }
        else
        {
          identity->value = 0u;
        }
    }

    return successFlag;    
}

bool DInstrument::getControllerMode(eControllerMode_t *controllerMode)
{
    bool successFlag = false;
    uint32_t val = 0u;
    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->getValue(E_VAL_INDEX_CONTROLLER_MODE,
                                                  &val);
        if(true == successFlag)
        {
          *controllerMode = (eControllerMode_t)val;
        }
        else
        {
          *controllerMode = (eControllerMode_t)E_CONTROLLER_MODE_NONE;
        }
    }
    return successFlag; 
}
bool DInstrument::setControllerMode(eControllerMode_t newCcontrollerMode)
{
    bool successFlag = false;
    uint32_t val = newCcontrollerMode;
    if (myCurrentFunction != NULL)
    {
        successFlag = myCurrentFunction->setValue(E_VAL_INDEX_CONTROLLER_MODE,
                                                  val);
      
    }
   return successFlag; 
}