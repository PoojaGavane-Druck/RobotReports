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
* @file     DSlot.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlot base class header file
*/

#ifndef _DPOWER_MANAGER_H
#define _DPOWER_MANAGER_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
MISRAC_ENABLE
#include "Types.h"
#include "DBattery.h"
#include "DTask.h"


/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DPowerManager : public DTask
{
    DBattery *battery;
    uint32_t timeElapsedFromLastBatteryRead;
    void monitorBatteryParams(void);
protected:
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond
    OS_MUTEX myMutex;

    

public:
    DPowerManager(void);
    virtual void initialise(void);
    virtual void runFunction(void);
    virtual void cleanUp(void);
    bool getValue(eValueIndex_t index, float32_t *value);    //get specified floating point function value    
    bool getValue(eValueIndex_t index, uint32_t *value);    //get specified integer function value

};

#endif // _DSLOT_H
