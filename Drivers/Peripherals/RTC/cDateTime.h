/*!
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the
* property of Baker Hughes and its suppliers, and affiliates if any.
* The intellectual and technical concepts contained herein are
* proprietary to Baker Hughes and its suppliers and affiliates
* and may be covered by U.S. and Foreign Patents, patents in
* process, and are protected by trade secret or copyright law.
* Dissemination of this information or reproduction of this
* material is strictly forbidden unless prior written permission
* is obtained from Baker Hughes.
*
* @file     cDateTime.h
*
* @author   Elvis Esa
* @date     February 2021
*
* Header file for DateTime service functions
*/

#ifndef __DATETIME_H
#define __DATETIME_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "main.h"
//#include <stdbool.h>

/* Private includes ----------------------------------------------------------*/

// None

/* Private defines -----------------------------------------------------------*/

// None

/* Exported types ------------------------------------------------------------*/

typedef enum { eCalendarMonth_Nul = 0,  eCalendarMonth_Jan = 1,  eCalendarMonth_Feb = 2,  eCalendarMonth_Mar = 3,  eCalendarMonth_Apr = 4,
               eCalendarMonth_May = 5,  eCalendarMonth_Jun = 6,  eCalendarMonth_Jul = 7,  eCalendarMonth_Aug = 8,  eCalendarMonth_Sep = 9,
               eCalendarMonth_Oct = 10, eCalendarMonth_Nov = 11, eCalendarMonth_Dec = 12, eCalendarMonth_Max = 13, eCalendarMonth_End = 0xFFFFFFFFu
             } eCalendarMonths_t;

typedef enum { eDaysExpired = 0, eDaysRemaining = 1, eDaysEnd = 0xFFFFFFFFu } eDaysRemExp_t;

// WARNING : eCalendarYear_Min Year should be multiple of 100 (as STM32 has year from 00 to 99)
// WARNING : DO NOT MODIFY (as STM32 has year from 00 to 99 - only valid for 2000 to 2099 )
// WARNING : NO validation checks are in place for years outside 2000 to 2099
typedef enum { eCalendarYear_Min = 2000,  eCalendarYear_Max = 2099,  eCalendarYear_End = 0xFFFFFFFFu } eCalendarYear_t;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Exported constants --------------------------------------------------------*/

// None

/* Exported macro ------------------------------------------------------------*/

// None

/* Exported functions prototypes ---------------------------------------------*/

extern bool dateTime_timeValid( const RTC_TimeTypeDef pTime );
extern bool dateTime_dateValid( const RTC_DateTypeDef pDate );
extern uint32_t dateTime_noDaysBtwDates( const RTC_DateTypeDef pDate1, const RTC_DateTypeDef pDate2, uint32_t* error );
extern uint32_t dateTime_noDaysExpRemInYrToDate( const RTC_DateTypeDef pDate, eDaysRemExp_t pDaysExpRem );

//extern void test ( void );

#ifdef __cplusplus
}
#endif

#endif