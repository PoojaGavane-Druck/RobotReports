/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     cDateTime.c
* @author   Elvis Esa
* @date     February 2021
*
* RTC Date/Time Utiltiy functions for various computations
*/

/* Includes -------------------------------------------------------------------------------*/

#include "misra.h"
MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stdbool.h>
#include <time.h>
MISRAC_ENABLE
#include "cDateTime.h"

/* Private defines ------------------------------------------------------------------------*/

// None

/* Exported Function Prototypes -----------------------------------------------------------*/

bool dateTime_timeValid( const RTC_TimeTypeDef pTime );
bool dateTime_dateValid( const RTC_DateTypeDef pDate );
uint32_t dateTime_noDaysBtwDates( const RTC_DateTypeDef pDate1, const RTC_DateTypeDef pDate2, uint32_t* error );
uint32_t dateTime_noDaysExpRemInYrToDate( const RTC_DateTypeDef pDate, eDaysRemExp_t pDaysExpRem );

/* Private Function Prototypes ------------------------------------------------------------*/

static bool dateTime_isLeapYear( const uint32_t pYear );

/* Private Variables ----------------------------------------------------------------------*/

typedef struct
{
    eCalendarMonths_t month;
    uint32_t days;
    uint32_t cummulativeDays;
} sDateTime_Month;

const sDateTime_Month sMonthDay[eCalendarMonth_Max] = {{eCalendarMonth_Nul, 0u, 0u},
    {eCalendarMonth_Jan, 31u, 31u},  {eCalendarMonth_Feb, 28u, 59u},  {eCalendarMonth_Mar, 31u, 90u},  {eCalendarMonth_Apr, 30u, 120u},
    {eCalendarMonth_May, 31u, 151u}, {eCalendarMonth_Jun, 30u, 181u}, {eCalendarMonth_Jul, 31u, 212u}, {eCalendarMonth_Aug, 31u, 243u},
    {eCalendarMonth_Sep, 30u, 273u}, {eCalendarMonth_Oct, 31u, 304u}, {eCalendarMonth_Nov, 30u, 334u}, {eCalendarMonth_Dec, 31u, 365u}
};

/* Global Variables -----------------------------------------------------------------------*/

// None

/* External Variables ---------------------------------------------------------------------*/

// None

/*-----------------------------------------------------------------------------------------*/

/*!
* @brief : STATIC - Validates if the year is a leap year
*
* @param[in]     : const uint32_t pYear - year to validate (full year format i.e. 20XX
* @param[out]    : None
* @param[in,out] : None
* @return        : bool - true = yes leap year, false = not leap year
* @note          : None
* @warning       : None
*/
static bool dateTime_isLeapYear( const uint32_t pYear )
{
    bool lLeapYear = false;

    if(( pYear % 4u ) == 0u )
    {
        if(( pYear % 100u ) == 0u )
        {
            if(( pYear % 400u ) == 0u )
            {
                lLeapYear = true;
            }
            else
            {
                lLeapYear = false;
            }
        }
        else
        {
            lLeapYear = true;
        }
    }

    return ( lLeapYear );
}

/*!
* @brief : STATIC - Computes the number of days expired/remaining in the presented year
*
* @param[in]     : const RTC_DateTypeDef pDate - date structure
*                : uint32_t daysExpRem - 1 = remaining days from date (including pDate), 0 = expired days to date (excluding pDate)
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lTotalDays - days expired/remaining (9999999 = error)
* @note          : None
* @warning       : None
*/
uint32_t dateTime_noDaysExpRemInYrToDate( const RTC_DateTypeDef pDate, eDaysRemExp_t pDaysExpRem )
{
    uint32_t lTotalDays = 0u;
    uint32_t leapDay = 0u;

    if( dateTime_dateValid( pDate ))
    {
        // Validate if leap year, if so add 1
        if( dateTime_isLeapYear(( uint32_t )pDate.Year + eCalendarYear_Min ))
        {
            leapDay = 1u;
        }

        // Check if we need to accumulate days in month
        if( pDate.Month == eCalendarMonth_Jan )
        {
            lTotalDays = pDate.Date;
        }
        else
        {
            lTotalDays = sMonthDay[pDate.Month - 1u].cummulativeDays + pDate.Date;

            if( pDate.Month > eCalendarMonth_Feb )
            {
                // Add leap Date as cumulative days is not based on leap Year
                lTotalDays = lTotalDays + leapDay;
            }
        }

        // Check If we want remaining days in the Year or expired
        if( pDaysExpRem == eDaysRemaining )
        {
            // We want remaining days from the date being tested (inclusive)
            lTotalDays = (( 365u + leapDay ) - lTotalDays ) + 1u;
        }
        else
        {
            // We want expired days excluding the date being tested (exclusive)
            lTotalDays -= 1u;
        }
    }
    else
    {
        lTotalDays = 9999999u;
    }

    return( lTotalDays );
}

/*!
* @brief : Validates the date presented
*
* @param[in]     : const RTC_DateTypeDef pDate - date structure to validate
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - 1 = ok, 0 = error
* @note          : None
* @warning       : None
*/
bool dateTime_dateValid( const RTC_DateTypeDef pDate )
{
    uint32_t lError = 0u;
    bool lok = false;

    // Validate month range
    if(( pDate.Month < eCalendarMonth_Jan ) || ( pDate.Month > eCalendarMonth_Dec ))
    {
        lError |= 1u;
    }
    else
    {
        uint32_t days = sMonthDay[pDate.Month].days;

        // Validate month
        if( pDate.Month == eCalendarMonth_Feb )
        {
            if( dateTime_isLeapYear(( uint32_t )pDate.Year + eCalendarYear_Min ))
            {
                days = days + 1u;
            }
        }

        // Validate date for the month
        if(( pDate.Date < 1u ) || ( pDate.Date > days ))
        {
            lError |= 1u;
        }
        else
        {
            // Validate weekday
            if(( pDate.WeekDay < 1u ) || ( pDate.WeekDay > 7u ))
            {
                lError |= 1u;
            }
        }
    }

    if( lError == 0u )
    {
        lok = true;
    }

    return( lok );
}

/*!
* @brief : Checks the number of days between two dates (start date is inclusive, end date is not inclusive)
*
* @param[in]     : const RTC_DateTypeDef pDate1
*                : const RTC_DateTypeDef pDate2
* @param[out]    : uint32_t* error - pointer to error holding variable ( 1 = error, 0 = ok )
* @param[in,out] : None
* @return        : uint32_t - number of days
* @note          : Does not have to be in any particular order
* @warning       : WARNING: start date is inclusive, end date is not inclusive
*/
uint32_t dateTime_noDaysBtwDates( const RTC_DateTypeDef pDate1, const RTC_DateTypeDef pDate2, uint32_t* error )
{
    uint32_t lError = 0u;
    uint32_t lTotalDays = 0u;
    uint32_t lSumYearDays = 0u;
    RTC_DateTypeDef pDateH;
    RTC_DateTypeDef pDateL;

    // Validate the dates
    if( false == dateTime_dateValid( pDate1 ))
    {
        lError |= 1u;
    }
    if( false == dateTime_dateValid( pDate2 ))
    {
        lError |= 1u;
    }

    if( 0u == lError )
    {
        // Check which date is larger
        if( pDate1.Year > pDate2.Year )
        {
            //PDATe 1 Year is bigger
            pDateH = pDate1;
            pDateL = pDate2;
        }
        else if( pDate1.Year == pDate2.Year )
        {
            // PDATEs years are equal
            if( pDate1.Month > pDate2.Month )
            {
                pDateH = pDate1;
                pDateL = pDate2;
            }
            else if( pDate1.Month == pDate2.Month )
            {
                if( pDate1.Date > pDate2.Date )
                {
                    pDateH = pDate1;
                    pDateL = pDate2;
                }
                else if ( pDate1.Date == pDate2.Date )
                {
                    pDateH = pDate1;
                    pDateL = pDate2;
                }
                else
                {
                    pDateH = pDate2;
                    pDateL = pDate1;
                }
            }
            else
            {
                pDateH = pDate2;
                pDateL = pDate1;
            }
        }
        else
        {
            // PDATE2 Year is bigger
            pDateH = pDate2;
            pDateL = pDate1;
        }

        // Calculate the number of whole years between dates
        uint32_t lWholeYears = (( uint32_t )pDateH.Year - ( uint32_t)pDateL.Year );

        // Exclude years we are measuring from and to - only count whole years in between
        if( lWholeYears > 1u )
        {
            lWholeYears -= 1u;

            //Add here days in Year including leap
            for( uint32_t x = 0u; x < ( lWholeYears ); x++ )
            {
                if( dateTime_isLeapYear( pDateL.Year + x + 1u + eCalendarYear_Min ))
                {
                    lSumYearDays += 1u;
                }

                lSumYearDays = lSumYearDays + 365u;
            }
        }

        if( lWholeYears > 0u )
        {
            //Get no of days remaining in the Year
            lTotalDays = dateTime_noDaysExpRemInYrToDate( pDateL, eDaysRemaining );

            // Get number of days elapsed
            lTotalDays += dateTime_noDaysExpRemInYrToDate( pDateH, eDaysExpired );

            lTotalDays += lSumYearDays;
        }
        else
        {
            lTotalDays = dateTime_noDaysExpRemInYrToDate( pDateH, eDaysExpired );
            lTotalDays -= dateTime_noDaysExpRemInYrToDate( pDateL, eDaysExpired );
        }

        // Error if we have error in noDaysInYrToDate (as this returns 999999)
        if( lTotalDays >= 999999u )
        {
            lError |= 1u;
        }
    }

    *error = lError;

    return( lTotalDays );
}

/*!
* @brief : Validates the time presented
*
* @param[in]     : const RTC_TimeTypeDef pTime - The time parameter presented for validation
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - 1 = ok, 0 = error
* @note          : None
* @warning       : WARNING: checks for 24hr time format - Only validates the hh, mm, ss, am/pm (as check)
*/
bool dateTime_timeValid( const RTC_TimeTypeDef pTime )
{
    bool lok;

    if(( pTime.Hours > 23u ) || ( pTime.Minutes > 59u ) || ( pTime.Seconds > 59u ) || ( pTime.TimeFormat > 1u ))
    {
        lok = false;
    }
    else
    {
        lok = true;
    }

    return( lok );
}


//#include "cRtc.h"
//void test ( void )
//{
//    uint32_t year = 2000u;
//    bool lok = false;
//    uint32_t lError = 0u;
//    // Wkdy, month, date, year
//
//    static RTC_DateTypeDef test1 = { 1u, 1u, 1u, 00u };
//    static RTC_DateTypeDef test2 = { 1u, 1u, 1u, 00u };
//    uint32_t days;
//    static RTC_TimeTypeDef a;
//    static RTC_DateTypeDef b;
//
//    a.Hours = 10u;
//    a.Minutes = 10u;
//    a.Seconds = 10u;
//
//    b.Month = 10u;
//    b.Date = 10u;
//    b.Year = 10u;
//    uint32_t xxxx = 0u;
//
//    if( xxxx == 1u )
//    {
//        rtc_setDate( &b );
//        rtc_setTime( &a );
//
//        rtc_getTime( &a );
//        rtc_getDate( &b );
//
//        rtc_getTime( &a );
//        rtc_getDate( &b );
//    }
//
//    lok = dateTime_isLeapYear( year );
//
//    days = dateTime_noDaysExpRemInYrToDate( test1, eDaysRemaining );
//
//    days = dateTime_noDaysExpRemInYrToDate( test1, eDaysExpired );
//
//    lok = dateTime_dateValid( test1 );
//
//    days = dateTime_noDaysBtwDates( test1, test2, &lError );
//}