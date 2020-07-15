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
* @file     DProcessAlarmSensorLo.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The sensor low alarm process data class header file
*
**********************************************************************************************************************/

#ifndef __DPROCESS_ALARM_SENSOR_LO_H
#define __DPROCESS_ALARM_SENSOR_LO_H

#include "DProcessAlarm.h"

class DProcessAlarmSensorLo : public DProcessAlarm
{
public:
    DProcessAlarmSensorLo(uint32_t channelIndex);

    virtual float32_t run(float32_t input);
};

#endif // __DPROCESS_ALARM_SENSOR_LO_H
