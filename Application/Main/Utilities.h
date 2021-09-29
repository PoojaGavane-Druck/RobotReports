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
* @file     Utilities.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     05 February 2020
*
* @brief    The utilities and helper functions header file
*/

#ifndef __UTILITIES_H
#define __UTILITIES_H

#ifdef __cplusplus
extern "C"
{
/* External C language linkage */
#endif

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <stdbool.h>
MISRAC_ENABLE

#include "Types.h"

/* Prototypes -------------------------------------------------------------------------------------------------------*/
/*********************************************************************************************************************/
//SUPPRESS: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma ("diag_suppress=Pm046")
/*********************************************************************************************************************/
#define  ISNAN(x) ((x) != (x))
/*********************************************************************************************************************/
//RESTORE: floating point values shall not be tested for exact equality or inequality (MISRA C 2004 rule 13.3)

_Pragma ("diag_default=Pm046")
/*********************************************************************************************************************/

void sleep(uint32_t ms);
bool floatEqual(float32_t a, float32_t b);


float32_t myRandomNumber(void);


bool isDateValid(uint32_t day, uint32_t month, uint32_t year) ;
bool daysSinceDate(sDate_t *date, uint32_t *days);

bool getSystemDate(sDate_t *date);
bool setSystemDate(sDate_t *date);
bool getSystemTime(sTime_t *_time);
bool setSystemTime(sTime_t *_time);
bool getEpochTime(uint32_t* epochTime);
void convertLocalDateTimeToTimeSinceEpoch(const sDate_t *date,
                                          const sTime_t* _time, 
                                          uint32_t *sec);
#ifdef __cplusplus
}                                                               /* End of external C language linkage */
#endif

#endif //__UTILITIES_H
