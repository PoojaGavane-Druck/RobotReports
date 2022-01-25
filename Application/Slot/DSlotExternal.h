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
* @file     DSlotExternal.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The DSlotExternal base class header file
*/

#ifndef _DSLOT_EXTERNAL_H
#define _DSLOT_EXTERNAL_H

/* Includes ---------------------------------------------------------------------------------------------------------*/

#include "DSlot.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DSlotExternal : public DSlot
{
protected:
    virtual eSensorError_t mySensorDiscover(void);
    virtual eSensorError_t mySensorIdentify(void);
    virtual eSensorError_t mySensorReadZero(void);
    virtual eSensorError_t mySensorSetZero(void);
    virtual eSensorError_t mySensorChecksumEnable(void);
    virtual eSensorError_t mySensorChecksumDisable(void);

public:
    DSlotExternal(DTask *owner);

    virtual void runFunction(void);
    virtual void start(void);
};

#endif // _DSLOT_EXTERNAL_H
