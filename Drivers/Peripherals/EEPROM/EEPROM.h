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
* @file     EEPROM.h
* @version  1.00.00
* @author   Harvinder Bhuhi (based on Piyali Senshupta's DPI705E code)
* @date     15 June 2020
*
* @brief    The EEPROM base class header file
*/
//*********************************************************************************************************************

#ifndef __EEPROM_H
#define __EEPROM_H

#ifdef __cplusplus
extern "C"
{
#endif

/************************************************* INCLUDES ****************************************************/
#include "misra.h"
MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stdbool.h>
MISRAC_ENABLE


/*********************************************** PROTOTYPES ****************************************************/
bool eepromRead(uint8_t *destAddr, uint16_t offsetLocation, uint16_t no_of_bytes);
bool eepromWrite(uint8_t *srcAddr, uint16_t offsetLocation, uint32_t no_of_bytes );

bool eepromTest(void);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_H */
