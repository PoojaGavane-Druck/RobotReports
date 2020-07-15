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

#ifndef _DSLOT_H
#define _DSLOT_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

#include "DTask.h"
#include "DSensor.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum : uint32_t
{
    E_SENSOR_STATUS_DISCOVERING = 0u,
    E_SENSOR_STATUS_IDENTIFYING,
    E_SENSOR_STATUS_READY,
    E_SENSOR_STATUS_RUNNING,
    E_SENSOR_STATUS_DISCONNECTED

} eSensorStatus_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DSlot : public DTask
{
protected:
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond
    OS_MUTEX myMutex;

    DSensor *mySensor;
    DTask *myOwner;

    eSensorStatus_t myState;

    void updateSensorStatus(eSensorError_t sensorError);

public:
    DSlot(DTask *owner);

    virtual void start(void);
    virtual void runFunction(void);
    virtual void cleanUp(void);

    DSensor *getSensor(void);

    void pause(void);
    void resume(void);
    void synchronise(void);
    void retry(void);

    //functions that must go through the slot and not directly to sensor
    virtual bool setOutput(uint32_t index, float32_t value);

};

#endif // _DSLOT_H
