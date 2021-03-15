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
#include "cDateTime.h"
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



bool DRtc::getTime(sTime_t *sTime)
{
    bool status = false;
    RTC_DateTypeDef sRtcDate;
    RTC_TimeTypeDef sRtcTime;
    if((HAL_StatusTypeDef)HAL_OK == HAL_RTC_GetTime(&hrtc, &sRtcTime, (uint32_t)(RTC_FORMAT_BIN)))
    {
      if((HAL_StatusTypeDef)HAL_OK == HAL_RTC_GetDate(&hrtc, &sRtcDate, (uint32_t)(RTC_FORMAT_BIN)))
      {
        status = true;
        sTime->hours = (uint32_t)(sRtcTime.Hours);
        sTime->minutes = (uint32_t)(sRtcTime.Minutes);
        sTime->seconds = (uint32_t)(sRtcTime.Seconds);
      }
    }
    return status;
}

bool DRtc::getDate(sDate_t *sDate)
{
    bool status = false;
    RTC_DateTypeDef sRtcDate;
    RTC_TimeTypeDef sRtcTime;
    if((HAL_StatusTypeDef)HAL_OK == HAL_RTC_GetTime(&hrtc, &sRtcTime, (uint32_t)(RTC_FORMAT_BIN)))
    {
      if((HAL_StatusTypeDef)HAL_OK == HAL_RTC_GetDate(&hrtc, &sRtcDate, (uint32_t)(RTC_FORMAT_BIN)))
      {
        status = true;
        sDate->day = (uint32_t)(sRtcDate.Date);
        sDate->month = (uint32_t)(sRtcDate.Month);
        sDate->year = (uint32_t)(sRtcDate.Year) + (uint32_t)(YEAR_OFFSET);
      }
    }
    return status; 
}

bool DRtc::getDateAndTime(sDate_t *sDate, sTime_t *sTime)
{
    bool status = false;

    
    status = getTime(sTime);
    if(true == status)
    {
      status = getDate(sDate);
    }
    return status;         

}

       
bool DRtc::setTime(uint32_t hour,  uint32_t minute, uint32_t second)
{
    bool status = false;
    RTC_TimeTypeDef sTime;
    
      
    HAL_StatusTypeDef halStatus = (HAL_StatusTypeDef)(HAL_ERROR);
      
    sTime.Hours = (uint8_t)(hour);
    sTime.Minutes = (uint8_t)(minute);
    sTime.Seconds = (uint8_t)(second);
    
    // validate Time
    if( dateTime_timeValid( sTime ))
    {
        // Default unused parameters
        status = true;
        sTime.SubSeconds = 0x00u;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;
        
        HAL_PWR_EnableBkUpAccess();
        uint32_t dummy = READ_BIT(PWR->CR1, PWR_CR1_DBP); // Dummy read
         // Set Time
        halStatus = HAL_RTC_SetTime(&hrtc, &sTime, (uint32_t)(RTC_FORMAT_BIN));
        if((HAL_StatusTypeDef)(HAL_OK) != halStatus)
        {
          status = false;
        }
        
        HAL_PWR_DisableBkUpAccess();
        dummy = READ_BIT(PWR->CR1, PWR_CR1_DBP); // Dummy read
    }
    else
    {
      status = false;
    }
    return status;
}       

bool DRtc::setDate(uint32_t day, uint32_t month, uint32_t year)                              
{
    bool status = false;
    
    RTC_DateTypeDef sDate;
      
    HAL_StatusTypeDef halStatus = (HAL_StatusTypeDef)(HAL_ERROR);
        
    sDate.Date = (uint8_t)(day);
    sDate.Month = (uint8_t)(month);
    sDate.Year = (uint8_t)(year - (uint32_t)(YEAR_OFFSET));
    sDate.WeekDay = 0u;
    
    if( dateTime_dateValid( sDate ))
    {
      status = true;
      
      HAL_PWR_EnableBkUpAccess();
      uint32_t dummy = READ_BIT(PWR->CR1, PWR_CR1_DBP); // Dummy read
            
      halStatus = HAL_RTC_SetDate(&hrtc, &sDate, (uint32_t)(RTC_FORMAT_BIN));
      if((HAL_StatusTypeDef)(HAL_OK) != halStatus)
      {
          status = false;
      }
      
      HAL_PWR_DisableBkUpAccess();
      dummy = READ_BIT(PWR->CR1, PWR_CR1_DBP); // Dummy read
      
       // Wait for update of the date
      if ( (HAL_StatusTypeDef)HAL_OK != HAL_RTC_WaitForSynchro(&hrtc) )
      {
          status = false;
          //Error_Handler();
      }
    }
    else
    {
      status = false;
    }

        
    
    return status;
}
       
bool DRtc::setDateAndTime(uint32_t day, 
                              uint32_t month,
                              uint32_t year,
                              uint32_t hour,
                              uint32_t minute,
                              uint32_t second)
{
    bool status = false;
    status = setTime(hour, minute, second);
    
    if(true == status)
    {
      setDate(day, month, year);
    }
    return status;
}













