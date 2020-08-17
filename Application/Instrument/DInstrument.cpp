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
#include "DChannelAuxiliary.h"

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
    myChannels[E_CHANNEL_3] = new DChannelAuxiliary((uint32_t)E_CHANNEL_3);

    *osErr = (OS_ERR)OS_ERR_NONE;
}

/**
 * @brief   Set Instrument function
 * @param   chan is the channel of the function to run
 * @param   func is the function itself
 * @param   dir is the measure/source specification
 * @retval  true if activated successfully, else false
 */
bool DInstrument::setFunction(eChannel_t chan, eFunction_t func, eFunctionDir_t dir)
{
    bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
    {
        successFlag = myChannels[chan]->setFunction(func, dir);
    }

    return successFlag;
}

/**
 * @brief   Set Instrument source function setpoint of currently running function
 * @param   chan is the channel
 * @param   index is function specific meaning identified a specific output parameter
 * @param   setpoint value
 * @retval  true if all's well, else false
 */
bool DInstrument::setOutput(eChannel_t chan, uint32_t index, float32_t setpoint)
{
    bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
    {
        successFlag = myChannels[chan]->setOutput(index, setpoint);
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
bool DInstrument::getReading(eChannel_t chan, uint32_t index, float32_t *reading)
{
    bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
    {
        DChannel *channel = myChannels[chan];

        if (channel != NULL)
        {
            successFlag = channel->getFunctionValue(index, reading);
        }
    }

    return successFlag;
}

/**
 * @brief   Signal sensor to continue
 * @param   chan is the channel
 * @retval  true if all's well, else false
 */
bool DInstrument::sensorContinue(eChannel_t chan)
{
    bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
    {
        DChannel *channel = myChannels[chan];

        if (channel != NULL)
        {
            successFlag = channel->sensorContinue();
        }
    }

    return successFlag;
}

/**
 * @brief   Signal sensor to retry after failure reported
 * @param   chan is the channel
 * @retval  true if all's well, else false
 */
bool DInstrument::sensorRetry(eChannel_t chan)
{
    bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
    {
        DChannel *channel = myChannels[chan];

        if (channel != NULL)
        {
            successFlag = channel->sensorRetry();
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
bool DInstrument::getPosFullscale(eChannel_t chan, float32_t *fs)
{
    bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
{
        DChannel *channel = myChannels[chan];

        if (channel != NULL)
        {
            successFlag = channel->getPosFullscale(fs);
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
bool DInstrument::getNegFullscale(eChannel_t chan, float32_t *fs)
{
    bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
{
        DChannel *channel = myChannels[chan];

        if (channel != NULL)
        {
            successFlag = channel->getNegFullscale(fs);
        }
    }

    return successFlag;
}

 bool DInstrument::getSensorType(eChannel_t chan, eSensorType_t *pSenType)
 {
  bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
    {
        DChannel *channel = myChannels[chan];

        if (channel != NULL)
        {
            *pSenType = channel->getSensorType();
            successFlag = true;
        }
    }

    return successFlag;
 }

bool DInstrument::getManufactureDate(eChannel_t chan, sDate_t *manfDate)
{
    bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
    {
        DChannel *channel = myChannels[chan];

        if (channel != NULL)
        {
           successFlag = channel->getManufactureDate(manfDate);
        }
    }

    return successFlag;
}
bool DInstrument::getUserCalDate(eChannel_t chan,sDate_t* caldate)
{
    bool successFlag = false;

    if (chan < (eChannel_t)E_CHANNEL_MAX)
    {
        DChannel *channel = myChannels[chan];

        if (channel != NULL)
        {
            successFlag = channel->getUserCalDate(caldate);
            
        }
    }

    return successFlag;
}