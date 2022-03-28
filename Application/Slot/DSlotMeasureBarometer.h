/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DSlotMeasureBarometer.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlotMeasureBarometer base class header file
*/

#ifndef _DSLOT_MEASURE_BAROMETER_H
#define _DSLOT_MEASURE_BAROMETER_H

/* Includes ---------------------------------------------------------------------------------------------------------*/


#include "DSlot.h"
#include "DTask.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_BAROMETER_NONE = 0,
    E_CHIP_BAROMETER,
    E_TERPS_BAROMETER
} eBarometerType_t;
/* Variables --------------------------------------------------------------------------------------------------------*/

class DSlotMeasureBarometer : public DSlot
{
    eBarometerType_t barometerOptionType;
public:
    DSlotMeasureBarometer(DTask *owner);
    virtual void initialise(void);
    virtual void start(void);
};

#endif // _DSLOT_MEASURE_BAROMETER_H
