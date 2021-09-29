/**
* BHGE Confidential
* Copyright 2021.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are protected by trade
* secret or copyright law.  Dissemination of this information or reproduction of this material is strictly forbidden unless prior written
* permission is obtained from Baker Hughes.
*
* @file     cRtc.c
* @author   Elvis Esa
* @date     February 2021
*
* The RTC date/time service functions to set and get the paramter structures.
*
*/

/* Includes -------------------------------------------------------------------------------*/

#include "cRtc.h"
#include "cDateTime.h"

/* Private defines ------------------------------------------------------------------------*/
#define YEAR_OFFSET 2000
// None

/* Exported Function Prototypes -----------------------------------------------------------*/

bool getTimeRtc(RTC_TimeTypeDef *psRtcTime);
bool getDate(RTC_DateTypeDef *psRtcDate);
bool getDateAndTime(RTC_DateTypeDef *psRtcDate, RTC_TimeTypeDef *psRtcTime);
bool setTimeRtc(RTC_TimeTypeDef *psRtcTime);
bool setDate(RTC_DateTypeDef *psRtcDate);
bool setDateAndTime(RTC_DateTypeDef *psRtcDate, RTC_TimeTypeDef *psRtcTime);
/* Private Function Prototypes ------------------------------------------------------------*/

// None

/* Private Variables ----------------------------------------------------------------------*/

// None

/* Global Variables -----------------------------------------------------------------------*/

// None

/* External Variables ---------------------------------------------------------------------*/

extern RTC_HandleTypeDef hrtc;

/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */

bool getTimeRtc(RTC_TimeTypeDef *psRtcTime)
{
    bool status = false;
    if( NULL != psRtcTime)
    {
      RTC_DateTypeDef sRtcDate;
      
      if(HAL_OK == HAL_RTC_GetTime(&hrtc, psRtcTime, (uint32_t)(RTC_FORMAT_BIN)))
      {
        if(HAL_OK == HAL_RTC_GetDate(&hrtc, &sRtcDate, (uint32_t)(RTC_FORMAT_BIN)))
        {
          status = true;
        }
      }
    }
    return status;
}

bool getDate(RTC_DateTypeDef *psRtcDate)
{
    bool status = false;
    if(NULL != psRtcDate)
    {
      RTC_TimeTypeDef sRtcTime;
      
      if(HAL_OK == HAL_RTC_GetTime(&hrtc, &sRtcTime, (uint32_t)(RTC_FORMAT_BIN)))
      {
        if(HAL_OK == HAL_RTC_GetDate(&hrtc, psRtcDate, (uint32_t)(RTC_FORMAT_BIN)))
        {
          status = true;
        }
      }
    }
    return status; 
}

bool getDateAndTime(RTC_DateTypeDef *psRtcDate, RTC_TimeTypeDef *psRtcTime)
{
    bool status = false;
   
    status = getTimeRtc(psRtcTime);
    if(true == status)
    {
      status = getDate(psRtcDate);
    }
    return status;         

}

       
bool setTimeRtc(RTC_TimeTypeDef *psRtcTime)
{
    bool status = false;
   
    HAL_StatusTypeDef halStatus = (HAL_StatusTypeDef)(HAL_ERROR);
    if( NULL != psRtcTime)
    { 

        RTC_TimeTypeDef sRtcTime = ( *psRtcTime );
        // validate Time
        if( dateTime_timeValid( sRtcTime ))
        {
            // Default unused parameters
            status = true;
            psRtcTime->SubSeconds = 0x00u;
            psRtcTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
            psRtcTime->StoreOperation = RTC_STOREOPERATION_RESET;
            
            HAL_PWR_EnableBkUpAccess();
            uint32_t dummy = READ_BIT(PWR->CR1, PWR_CR1_DBP); // Dummy read
             // Set Time
            halStatus = HAL_RTC_SetTime(&hrtc, psRtcTime, (uint32_t)(RTC_FORMAT_BIN));
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
    }
    return status;
}       

bool setDate( RTC_DateTypeDef *psRtcDate)                              
{
    bool status = false;
  
    HAL_StatusTypeDef halStatus = (HAL_StatusTypeDef)(HAL_ERROR);
    if( NULL != psRtcDate)
    {       
        
        if( dateTime_dateValid( *psRtcDate ))
        {
          status = true;
          
          HAL_PWR_EnableBkUpAccess();
          uint32_t dummy = READ_BIT(PWR->CR1, PWR_CR1_DBP); // Dummy read
                
          halStatus = HAL_RTC_SetDate(&hrtc, psRtcDate, (uint32_t)(RTC_FORMAT_BIN));
          if((HAL_StatusTypeDef)(HAL_OK) != halStatus)
          {
              status = false;
          }
          
          HAL_PWR_DisableBkUpAccess();
          dummy = READ_BIT(PWR->CR1, PWR_CR1_DBP); // Dummy read
          
           // Wait for update of the date
          if ( HAL_RTC_WaitForSynchro(&hrtc) != HAL_OK)
          {
              status = false;
              //Error_Handler();
          }
        }
        else
        {
          status = false;
        }

    }    
    
    return status;
}
       
bool setDateAndTime(RTC_DateTypeDef *psRtcDate, RTC_TimeTypeDef *psRtcTime)
{
    bool status = false;
    status = setTimeRtc(psRtcTime);
    
    if(true == status)
    {
      setDate(psRtcDate);
    }
    return status;
}
