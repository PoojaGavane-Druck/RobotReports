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

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlot class constructor
 * @note    An owner is needed if sensor events are to be sent back. This is expected to be a DFunction instance.
 * @param   owner: the task that created this slot
 * @retval  void
 */
DRtc::DRtc(OS_ERR *os_error)
: DTask()
{
    //create mutex for resource locking
    char *name = "RTC";
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, os_error);

    if (*os_error != (OS_ERR)OS_ERR_NONE)
    {
        //Error handler?
    }

    //specify the flags that this function must respond to (add more as necessary in derived class)
    myWaitFlags =  EV_FLAG_TASK_SENSOR_CONTINUE;
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

void DRtc::getDateAndTime(sDate_t *sDate, sTime_t *sTime)
{
    RTC_DateTypeDef sRtcDate;
    RTC_TimeTypeDef sRtcTime;
    
    getDate(&sRtcDate);
    getTime(&sRtcTime);
    
    sDate->day = (uint32_t)(sRtcDate.Date);
    sDate->month = (uint32_t)(sRtcDate.Month);
    sDate->year = (uint32_t)(sRtcDate.Year);
    
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
    sDate.Year = (uint8_t)(year);
    
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

/**
 * @brief   Run DSlot task funtion
 * @param   void
 * @retval  void
 */
void DRtc::runFunction(void)
{
    //this is a while loop that pends on event flags
    bool runFlag = true;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
  
    //notify parent that we have connected, awaiting next action - this is to allow
    //the higher level to decide what other initialisation/registration may be required

    while (runFlag == true)
    {
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)500u, //runs, nominally, at 2Hz by default
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
        //check for events
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
            
        }
        else if (os_error != (OS_ERR)OS_ERR_NONE)
        {
            //os error
          
        }
        else
        {
            if ((actualEvents & EV_FLAG_TASK_SHUTDOWN) == EV_FLAG_TASK_SHUTDOWN)
            {
                runFlag = false;
            }
          
        }

        //handle reported sensor status changes and set/clear any errors from sensor functions
       
    }

   
}

/**
 * @brief   Clean up after termination of task
 * @param   void
 * @return  void
 */
void DRtc::cleanUp(void)
{
    OS_ERR err;

    //if a stack was allocated then free that memory to the partition
    if (myTaskStack != NULL)
    {
        //Return the stack memory block back to the partition
        OSMemPut((OS_MEM*)&memPartition, (void*)myTaskStack, (OS_ERR*)&err);

        if (err == (OS_ERR)OS_ERR_NONE)
        {
            //memory block from the partition obtained
        }

        myTaskStack = NULL;
    }
}









