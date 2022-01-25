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

#ifndef _DRTC_H
#define _DRTC_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_def.h>
#include <os.h>
#include <Types.h>
MISRAC_ENABLE


/* Defines ----------------------------------------------------------------------------------------------------------*/
#define RTC_FORMAT_BIN                      0x00000000u
#define RTC_FORMAT_BCD                      0x00000001u

/* Types ------------------------------------------------------------------------------------------------------------*/


/* Variables --------------------------------------------------------------------------------------------------------*/

class DRtc
{
private:

public:
    DRtc(void);
    bool getTime(sTime_t *sTime);
    bool getDate(sDate_t *sDate);
    bool getDateAndTime(sDate_t *sDate, sTime_t *sTime);
    bool setTime(uint32_t hour,  uint32_t minute, uint32_t second);
    bool setDate(uint32_t day, uint32_t month, uint32_t year);
    bool setDateAndTime(uint32_t day,
                        uint32_t month,
                        uint32_t year,
                        uint32_t hour,
                        uint32_t minute,
                        uint32_t second);
};

#endif // _DSLOT_H
