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
* @file     cRTC.h
*
* @author   Elvis Esa
* @date     February 2021
*
* Header file for RTC service functions
*/



#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "misra.h"
MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_rtc.h>
#include <stdbool.h>
MISRAC_ENABLE
#include "Types.h"
/* Private includes ----------------------------------------------------------*/

// None

/* Private defines -----------------------------------------------------------*/

#define DEF_RTC_CONFIGURED_PIN_1  (( uint32_t )0x2D455341u )
#define DEF_RTC_CONFIGURED_PIN_2  (( uint32_t )0xFE45534Fu )

/* Exported types ------------------------------------------------------------*/

// None

/* Exported constants --------------------------------------------------------*/

// None

/* Exported macro ------------------------------------------------------------*/

// None

/* Exported functions prototypes ---------------------------------------------*/

extern bool getTimeRtc(RTC_TimeTypeDef *psRtcTime);
extern bool getDate(RTC_DateTypeDef *psRtcDate);
extern bool getDateAndTime(RTC_DateTypeDef *psRtcDate, RTC_TimeTypeDef *psRtcTime);
extern bool setTimeRtc(RTC_TimeTypeDef *psRtcTime);
extern bool setDate(RTC_DateTypeDef *psRtcDate);
extern bool setDateAndTime(RTC_DateTypeDef *psRtcDate, RTC_TimeTypeDef *psRtcTime);

#ifdef __cplusplus
}
#endif

#endif