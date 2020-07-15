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
* @file     DInstrument.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The instrument base class header file
*/

#ifndef _DINSTRUMENT_H
#define _DINSTRUMENT_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DChannel.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_CHANNEL_1 = 0,
    E_CHANNEL_2,
    E_CHANNEL_3,

    E_CHANNEL_MAX

} eChannel_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DInstrument
{
private:
    DChannel *myChannels[E_CHANNEL_MAX];

public:
    DInstrument(OS_ERR *osErr);

    bool setFunction(eChannel_t chan, eFunction_t func, eFunctionDir_t dir);
    bool setOutput(eChannel_t chan, uint32_t index, float32_t setpoint);
    bool getReading(eChannel_t chan, uint32_t index, float32_t *reading);
    bool sensorContinue(eChannel_t chan);
    bool sensorRetry(eChannel_t chan);
};

#endif // _DINSTRUMENT_H
