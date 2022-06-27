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
* @file     crc.h
* @version  1.00.00
* @author   Harvinder Bhuhi (based on Julio Andrade's DPI705E code)
* @date     18 June 2020
*
* @brief    The persistent (non-volatile) calibration data header file
*/
//*********************************************************************************************************************
#ifndef __CRC_H
#define __CRC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
MISRAC_ENABLE

uint32_t crc32(uint8_t buf[], uint32_t len);
uint32_t crc32ExternalStorage(uint8_t buf[], uint32_t len, uint32_t crc);
uint8_t crc8(uint8_t *data, uint8_t length, uint8_t *crc);
void generateTableCrc8ExternalStorage(uint8_t polynomial);
uint32_t crc32Offset(uint8_t buf[], uint32_t len, uint8_t offset);

#ifdef __cplusplus
}
#endif

#endif //__CRC_H
