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
* @file     i2c.h
* @version  1.00.00
* @author   Piyali Sengupta
* @date     3 October 2018
* @brief    Header File for I2C Instantiation
*/

#ifndef __I2C_H
#define __I2C_H

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
    I2Cn1 = 0,
    I2Cn4,
    I2CnSIZE,

    I2CnNone //invalid value

} eI2CElement_t;

/****************** PROTOTYPES ******************/

void i2cInit(eI2CElement_t elem );
HAL_StatusTypeDef I2C_WriteBuffer(eI2CElement_t elem, uint16_t Addr, uint16_t devMemLocation, uint16_t devMemSize, uint8_t *pBuffer, uint16_t Length);
HAL_StatusTypeDef I2C_ReadBuffer(eI2CElement_t elem, uint16_t Addr, uint16_t devMemLocation, uint16_t devMemSize, uint8_t *pBuffer, uint16_t Length);
HAL_StatusTypeDef I2CDeviceReady(eI2CElement_t elem, uint16_t DevAddress);
void i2c1TestMode(void);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H_ */
