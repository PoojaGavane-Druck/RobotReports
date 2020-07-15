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
* @file     DProcessAlarm.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The alarm process base class header file
*
**********************************************************************************************************************/

#ifndef __DPROCESS_ALARM_H
#define __DPROCESS_ALARM_H

#include "DProcess.h"

class DProcessAlarm : public DProcess
{
protected:
    float32_t myThreshold;
    bool myAlarmState;

public:
    DProcessAlarm(uint32_t channelIndex);

    virtual void reset();

    virtual float32_t getParameter(uint32_t index = 0u);
    virtual void setParameter(float32_t value, uint32_t index = 0u);
};

#endif // __DPROCESS_ALARM_H
