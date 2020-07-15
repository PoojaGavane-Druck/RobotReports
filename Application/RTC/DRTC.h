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
MISRAC_ENABLE
#include "DTask.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/


/* Variables --------------------------------------------------------------------------------------------------------*/

class DRtc : public DTask
{
protected:
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond
    OS_MUTEX myMutex;
    DTask *myOwner;
   

public:
    DRtc(DTask *owner);
    
    virtual void initialize(void);
   
    virtual void runFunction(void);
    virtual void cleanUp(void);
    void initRtc(void);
    void rtcAlarmIRQHandler(void);
    bool checkRTC(RTC_DateTypeDef* pDate, RTC_TimeTypeDef *pTime);
    bool isClockSet(void);
    void getTime( RTC_TimeTypeDef *stime );
    void getDate( RTC_DateTypeDef *sdate );
    bool dateConfig(uint32_t dd, uint32_t mm,uint32_t yy);

};

#endif // _DSLOT_H
