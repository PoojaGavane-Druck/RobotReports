/**
* BHGE Confidential
* Copyright 2019.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are protected by trade
* secret or copyright law.  Dissemination of this information or reproduction of this material is strictly forbidden unless prior written
* permission is obtained from Baker Hughes.
*
* @file    smbus.h
* @version  1.00.00
* @author   Piyali Sengupta
* @date     3 October 2018
* @brief    Header File for I2C Instantiation
*/

#ifndef __SMBUS_H
#define __SMBUS_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "misra.h"


MISRAC_DISABLE
#include <stm32l4xx_hal.h>
MISRAC_ENABLE



/***************** TYPEDEFS *********************/

typedef enum
{
    SMBUSn1 = 0,
    SMBUSn4,
    SMBUSnSIZE,

    SMBUSnNone //invalid value

} eSmbusElement_t;

/****************** PROTOTYPES ******************/

void smbusInit(void );
HAL_StatusTypeDef SMBUS_Transmit( uint16_t deviceAddr, 
                                 uint8_t *pBuffer, 
                                 uint16_t Length, 
                                 uint32_t xferOptions);
HAL_StatusTypeDef SMBUS_Receive( uint16_t deviceAddr,
                                 uint8_t *pBuffer, 
                                 uint16_t Length, 
                                 uint32_t xferOptions);


#ifdef __cplusplus
}
#endif

#endif /* __I2C_H_ */
