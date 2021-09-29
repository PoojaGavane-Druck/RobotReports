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
* @file     DFunction.h
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     04 November 2020
*
* @brief    SPI header file
*/

#ifndef _SPI_H_
#define _SPI_H_

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "main.h"


/* Defines ----------------------------------------------------------------------------------------------------------*/
#define GPIO_NSS_PORT GPIOE
#define GPIO_NSS_PIN 2
/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

void SPI_SetNss(void);
void SPI_ResetNss(void);
uint8_t SPI_TransmitReceive(uint8_t byte);
uint16_t GPIO_ReadOutputDataBit(GPIO_TypeDef *port, uint16_t pin);


#ifdef __cplusplus
}
#endif
#endif