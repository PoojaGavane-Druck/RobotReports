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
#include <os.h>
#include <math.h>
MISRAC_ENABLE

#include "Utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
static const uint32_t MIN_ALLOWED_YEAR = 2018u;
static const uint32_t MAX_ALLOWED_YEAR = 2099u;

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
static const float32_t EPSILON = 1E-10f;  //arbitrary 'epsilon' value

// Max days in a month                    J   F    M    A    M    J    J    A    S    O    N    D
static const uint32_t monthDays[12] = { 31u, 29u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u };

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

//*********************************************************************************************************************
/**
* @brief  Wait for specified time
* @param  ms is the time in milliseconds
* @return void
*/
void sleep(uint32_t ms)
{
    if (ms > 0u)
    {
        OS_ERR os_error;

        //delay ms delay before retrying
        OSTimeDlyHMSM(0u, 0u, 0u, ms, OS_OPT_TIME_HMSM_STRICT, &os_error);
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
    bool flag = true; //assume true unless find something wrong

    //Check all ranges first for day, month and year
    if ((year < MIN_ALLOWED_YEAR) || (year > MAX_ALLOWED_YEAR) || (month < 1u) || (month > 12u) || (day < 1u))
    {
        flag = false;
    }
    else if (day > monthDays[month - 1u]) //check upper limit of day value
    {
        flag = false;
    }
    else if (month == 2u)
    {
        //additional February check for leap year (which is true when year is a multiple of 4 and not multiple of 100
        // or when year is multiple of 400
        if (((((year % 4u == 0u) && (year % 100u != 0u)) || (year % 400u == 0u)) == false) && (day > 28u))
        {
            //not a leap year and day is > 28
            flag = false;
        }
    }
    else
    {
        //added for MISRA check compliance
    }

    return flag;
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

    if (isDateValid(date->day, date->month, date->year) == true)
    {
        //TODO: check against RTC date to determine value
        *days = 365u;
        flag = true;
    }

    return flag;
}

