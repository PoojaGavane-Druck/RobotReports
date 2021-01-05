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
* @file     DSlot.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlot class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <main.h>
#include <Types.h>

MISRAC_ENABLE

#include "Drtc.h"
#include "memory.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define YEAR_OFFSET 2000
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;
/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlot class constructor
 * @note    An owner is needed if sensor events are to be sent back. This is expected to be a DFunction instance.
 * @param   owner: the task that created this slot
 * @retval  void
 */
DRtc::DRtc(void)
{
    
}

/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */

void DRtc::getTime( RTC_TimeTypeDef *stime )
{    
    HAL_RTC_GetTime(&hrtc, stime, (uint32_t)(RTC_FORMAT_BIN));    
}


void DRtc::getDate( RTC_DateTypeDef *sdate )
{            
    HAL_RTC_GetDate(&hrtc, sdate, (uint32_t)(RTC_FORMAT_BIN));
}

void DRtc::getTime(sTime_t *sTime)
{
    RTC_DateTypeDef sRtcDate;
    RTC_TimeTypeDef sRtcTime;
    getTime(&sRtcTime);
    getDate(&sRtcDate);
    
    sTime->hours = (uint32_t)(sRtcTime.Hours);
    sTime->minutes = (uint32_t)(sRtcTime.Minutes);
    sTime->seconds = (uint32_t)(sRtcTime.Seconds);
}

void DRtc::getDate(sDate_t *sDate)
{
    RTC_DateTypeDef sRtcDate;
    RTC_TimeTypeDef sRtcTime;
    getTime(&sRtcTime);
    getDate(&sRtcDate);
    
    sDate->day = (uint32_t)(sRtcDate.Date);
    sDate->month = (uint32_t)(sRtcDate.Month);
    sDate->year = (uint32_t)(sRtcDate.Year) + (uint32_t)(YEAR_OFFSET);
}

void DRtc::getDateAndTime(sDate_t *sDate, sTime_t *sTime)
{
    RTC_DateTypeDef sRtcDate;
    RTC_TimeTypeDef sRtcTime;
    
    getTime(&sRtcTime);
    getDate(&sRtcDate);
    
    sDate->day = (uint32_t)(sRtcDate.Date);
    sDate->month = (uint32_t)(sRtcDate.Month);
    sDate->year = (uint32_t)(sRtcDate.Year) + (uint32_t)(YEAR_OFFSET);
    
    sTime->hours = (uint32_t)(sRtcTime.Hours);
    sTime->minutes = (uint32_t)(sRtcTime.Minutes);
    sTime->seconds = (uint32_t)(sRtcTime.Seconds);
}


bool DRtc::setDateAndTime(uint32_t day, 
                              uint32_t month,
                              uint32_t year,
                              uint32_t hour,
                              uint32_t minute,
                              uint32_t second)
{
    bool status = false;
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
      
    HAL_StatusTypeDef halStatus = (HAL_StatusTypeDef)(HAL_ERROR);
      
    sTime.Hours = (uint8_t)(hour);
    sTime.Minutes = (uint8_t)(minute);
    sTime.Seconds = (uint8_t)(second);
    
    sDate.Date = (uint8_t)(day);
    sDate.Month = (uint8_t)(month);
    sDate.Year = (uint8_t)(year - (uint32_t)(YEAR_OFFSET));
    sDate.WeekDay = 0u;
    halStatus = HAL_RTC_SetTime(&hrtc, &sTime, (uint32_t)(RTC_FORMAT_BIN));
    if((HAL_StatusTypeDef)(HAL_OK) == halStatus)
    {
          halStatus = HAL_RTC_SetDate(&hrtc, &sDate, (uint32_t)(RTC_FORMAT_BIN));
        if((HAL_StatusTypeDef)(HAL_OK) == halStatus)
        {
            status = true;
        }
    }
    return status;
}

/**
 * @brief   RTC Alarm Interrrupt Request Handler .
 */

void DRtc::rtcAlarmIRQHandler(void)
{
    OSIntEnter( );
    //HAL_RTC_AlarmIRQHandler(&RtcHandle);
    OSIntExit( );
}

/**
 * @brief   validate the RTC date and time against unit's valid date and time
 * @param   pDate rtc date handler
 * @param   pTime rtc time handler
 * @return  Valid date status
 */

bool DRtc::checkRTC(RTC_DateTypeDef* pDate, RTC_TimeTypeDef *pTime)
{
    //get the current date and time
    getDate(pDate);
    getTime(pTime);
    return true;
    //check for valid date
    //return dateTimeCalcValidate((uint32_t)pDate->Date, (uint32_t)pDate->Month, (uint32_t)(2000u + (uint32_t)pDate->Year));
}

/**
 * @brief   Checks if clock is set and the RTC date and time is valid
 * @param   void
 * @return  flag - true if RTC set and date and time valid, else false
 */

bool DRtc::isClockSet(void)
{
    //check that clock is set
    RTC_DateTypeDef d;
    RTC_TimeTypeDef t;

    return checkRTC(&d, &t);
}









