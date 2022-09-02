/**
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     Utilities.c
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     05 February 2020
*
* @brief    The utilities and helper functions source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <assert.h>
#include <os.h>
#include <math.h>
#include <time.h>
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_rtc.h>
#include "usbd_def.h" // for MIN MAX macros
MISRAC_ENABLE

#include "Utilities.h"
#include "cRtc.h"
#include "CDateTime.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
const uint32_t MIN_ALLOWED_YEAR = 2018u;
const uint32_t MAX_ALLOWED_YEAR = 2099u;
static const uint32_t VALID_WEEKDAY = 1u;
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
static const float32_t EPSILON = 1E-10f;  //arbitrary 'epsilon' value

/* Prototypes -------------------------------------------------------------------------------------------------------*/
uint32_t (*__bootLoader_API)(uint32_t command, const uint8_t *dataPtr, const uint32_t size, const uint32_t reset, CRC_HandleTypeDef *crcHandle) = (uint32_t (*)(const uint32_t command, const uint8_t *dataPtr, const uint32_t size, const uint32_t reset, CRC_HandleTypeDef *crcHandle))0x0801F001u;

/* User code --------------------------------------------------------------------------------------------------------*/
/**
* @brief  Call bootloader API
* @param
* @return
*/
uint32_t bootloaderApi(uint32_t command, const uint8_t *dataPtr, const uint32_t size, const uint32_t reset, CRC_HandleTypeDef *crcHandle)
{
    return (*__bootLoader_API)(command, dataPtr, size, reset, crcHandle);
}

/* User code --------------------------------------------------------------------------------------------------------*/

//*********************************************************************************************************************
/**
* @brief  Wait for specified time
* @param  ms is the time in milliseconds
* @return void
*/
void sleep(uint32_t ms)
{
    uint32_t secs = 0u;

    if(ms > 0u)
    {
        OS_ERR os_error = OS_ERR_NONE;

        if(ms > 999u)
        {
            secs = ms / 1000u;

            if(secs > 59u)
            {
                secs = 59u;
            }

            ms %= 1000u;
        }

        //delay ms delay before retrying
        RTOSTimeDlyHMSM(0u, 0u, (CPU_INT16U)secs, ms, OS_OPT_TIME_HMSM_STRICT, &os_error);
    }
}

/*
 * @brief Compare two floating point numbers for equality
 * @param a - first floating point value
 * @param b - second floating point value
 * @return true if the difference between the two values is less than EPSILON, else false
 */
bool floatEqual(float32_t a, float32_t b)
{
    return (fabsf(a - b) < EPSILON) ? (bool)true : (bool)false;
}


static int32_t mySeed = 0;

// Generate pseudo-random number (0.0 - 1.0)
float32_t myRandomNumber(void)
{
    mySeed = (214013 * mySeed + 2531011);
    float32_t fRandom = ((float32_t)mySeed / 4294967296.0f);
    return fRandom;
}


/*
 * @brief   Check date is valid
 * @param   day
 * @param   month
 * @param   year
 * @return true if date is valid, else false
 */
bool isDateValid(uint32_t day, uint32_t month, uint32_t year)
{
    bool sucessFlag = true; //assume true unless find something wrong

    //Check all ranges first for day, month and year
    if((year >= MIN_ALLOWED_YEAR) && (year <= MAX_ALLOWED_YEAR))
    {
        RTC_DateTypeDef pDate = {VALID_WEEKDAY, (uint8_t)month, (uint8_t)day, (uint8_t)(year - MIN_ALLOWED_YEAR)};
        sucessFlag = dateTime_dateValid(pDate);
    }

    return sucessFlag;
}

/*
 * @brief   Return no of days since specified date
 * @param   date - pointer to date structure
 * @param   days - pointer for return value (cannot be negative, that would mean last cal is in the future!)
 * @return  true if check is valid, false if either specified or RTC date are not valid
 */
bool daysSinceDate(sDate_t *date, uint32_t *days)
{
    bool flag = false;

    *days = 0u;

    if(isDateValid(date->day, date->month, date->year) == true)
    {
        RTC_DateTypeDef pDate1 = {VALID_WEEKDAY, (uint8_t)date->month, (uint8_t)date->day, (uint8_t)(date->year - MIN_ALLOWED_YEAR)};
        RTC_DateTypeDef pDate2;

        getDate(&pDate2);
        uint32_t error;
        *days = dateTime_noDaysBtwDates(pDate1, pDate2, &error);

        if(error == 0u)
        {
            flag = true;
        }
    }

    return flag;
}

/*
 * @brief   Get date from RTC
 * @param   date - pointer to date structure
 * @return  true if check is valid, false if either specified or RTC date are not valid
 */
bool getSystemDate(sDate_t *date)
{
    RTC_DateTypeDef psDate;
    bool status = getDate(&psDate);

    date->day = psDate.Date;
    date->month = psDate.Month;
    date->year = (uint32_t)psDate.Year + MIN_ALLOWED_YEAR;

    return status;
}

/*
 * @brief   Set date in RTC
 * @param   date - pointer to date structure
 * @return  true if check is valid, false if either specified or RTC date are not valid
 */
bool setSystemDate(sDate_t *date)
{
    bool status = false;

    if((date->year >= MIN_ALLOWED_YEAR) && (date->year <= MAX_ALLOWED_YEAR))
    {
        RTC_DateTypeDef psDate = {VALID_WEEKDAY, (uint8_t)date->month, (uint8_t)date->day, (uint8_t)(date->year - MIN_ALLOWED_YEAR)};
        status = setDate(&psDate);
    }

    return status;
}

/**
 * @brief   Get system time from RTC
 * @param   time - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool getSystemTime(sTime_t *_time)
{
    RTC_TimeTypeDef psTime;
    bool status = getTimeRtc(&psTime);

    if(status)
    {
        _time->hours = psTime.Hours;
        _time->minutes = psTime.Minutes;
        _time->seconds = psTime.Seconds;
        _time->milliseconds = (psTime.SecondFraction - psTime.SubSeconds) * 1000u / (psTime.SecondFraction + 1u);
    }

    else
    {
        _time->hours = 0u;
        _time->minutes = 0u;
        _time->seconds = 0u;
        _time->milliseconds = 0u;
    }

    return status;
}

/**
 * @brief   Set system time in RTC
 * @param   time - pointer to system time structure containing time to set the RTC
 * @retval  true = success, false = failed
 */
bool setSystemTime(sTime_t *_time)
{
    RTC_TimeTypeDef psTime;
    psTime.Hours = (uint8_t)_time->hours;
    psTime.Minutes = (uint8_t)_time->minutes;
    psTime.Seconds = (uint8_t)_time->seconds;
    psTime.TimeFormat = 0u;
    return setTimeRtc(&psTime);
}
/**
 * @brief   Convert date and time to time in seconds since the epoch (maximum date requirement fits into 32 bits)
 * @param   date - pointer to system date structure containing date
 * @param   time - pointer to system time structure containing time
 * @param   sec - pointer to time in seconds since the epoch
 * @retval  void
 */
void convertLocalDateTimeToTimeSinceEpoch(const sDate_t *date,
        const sTime_t *_time,
        uint32_t *sec)
{
    struct tm t = {0};
    t.tm_year = (int)date->year - 1900;
    t.tm_mon = (int)date->month;
    t.tm_mday = (int)date->day;
    t.tm_hour = (int)_time->hours;
    t.tm_min = (int)_time->minutes;
    t.tm_sec = (int)_time->seconds;
    time_t timeSinceEpoch = mktime(&t);
    *sec = (uint32_t)timeSinceEpoch;
}


bool getMilliSeconds(uint32_t *ms)
{
    bool status = false;
    sDate_t sDate;
    sTime_t sTime;
    status = getSystemTime(&sTime);

    if(true == status)
    {
        status = getSystemDate(&sDate);
    }

    if(true == status)
    {
        *ms = sTime.milliseconds;
    }

    return status;
}

bool getEpochTime(uint32_t *epochTime)
{
    bool status = false;
    sDate_t sDate;
    sTime_t sTime;
    status = getSystemTime(&sTime);

    if(true == status)
    {
        status = getSystemDate(&sDate);
    }

    if(true == status)
    {
        convertLocalDateTimeToTimeSinceEpoch(&sDate, &sTime, epochTime);
    }

    return status;
}

/**
 * @brief   convert the month number to English string representation
 * @param   uint32_t month
 * @retval  const char*
 */
const char *convertMonthToString(uint32_t month)
{
    assert(((int)month >= 1) && ((int)month <= 12));
    month = MAX(month, 1u);
    month = MIN(month, 12u);
    const char *monthString[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    return monthString[(int)month - 1];
}

/**
 * @brief   convert the month number to 3 character English string representation
 * @param   uint32_t month
 * @retval  const char*
 */
const char *convertMonthToAbbreviatedString(uint32_t month)
{
    assert(((int)month >= 1) && ((int)month <= 12));
    month = MAX(month, 1u);
    month = MIN(month, 12u);
    const char *monthString[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    return monthString[(int)month - 1];
}

/**
 * @brief   fetches a string from multiline string
 * @param   uint32_t srcBuf  pointer to multiline string
 * @param   uint32_t respBuf  pointer to buffer to store string
 * @retval  number of bytes if the response string
 */
uint32_t fetchString(const uint8_t *srcBuf, uint8_t *respBuf)
{

    uint32_t index = 0u;

    if((srcBuf != NULL) && (respBuf != NULL))
    {

        while(srcBuf[index] != 0X0Au)
        {
            respBuf[index] = srcBuf[index];
            index++;
        }

        respBuf[index] = srcBuf[index];
        index++;
    }

    return index;
}
/**
 * @brief   find number of days between from date to Todate
 * @param   date - pointer to date structure containing from date
 * @param   date - pointer to date structure containing to date
 * @retval  returns date difference
 */
int32_t getDateDiff(const sDate_t *fromDate, const sDate_t *toDate)
{

    sTime_t timeValue;

    uint32_t fromDateInEpoch = 0u;
    uint32_t toDateInEpoch = 0u;
    int32_t dateDiff = 0;
    timeValue.hours = 0u;
    timeValue.minutes = 0u;
    timeValue.seconds = 0u;
    timeValue.milliseconds = 0u;

    convertLocalDateTimeToTimeSinceEpoch(fromDate, &timeValue, &fromDateInEpoch);
    convertLocalDateTimeToTimeSinceEpoch(toDate, &timeValue, &toDateInEpoch);
    dateDiff = ((int32_t)toDateInEpoch - (int32_t)fromDateInEpoch);
    return dateDiff;
}

/**
 * @brief   Compare two arrays for similarity for the given length
 * @param   uint8_t * array1 -
            uint8_t * array2
            uint32_t length
 * @retval  uint32_t isEqual - 1 if equal, 0 if not
 */
uint32_t compareArrays(const uint8_t *array1, const uint8_t *array2, uint32_t length)
{
    uint32_t isSame = 0u;
    uint32_t counter = 0u;

    for(counter = 0u; counter < length; counter++)
    {
        if(array1[counter] == array2[counter])
        {
            isSame = 1u;
        }

        else
        {
            isSame = 0u;
            /* Any point in time the arrays dont compare to be equal, make the counter equal to length to exit loop */
            counter = length;
        }
    }

    return isSame;
}


