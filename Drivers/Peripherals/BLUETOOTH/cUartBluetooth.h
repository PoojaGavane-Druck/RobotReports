/**
* Baker Hughes Confidential
* Copyright 2019.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are protected by trade
* secret or copyright law.  Dissemination of this information or reproduction of this material is strictly forbidden unless prior written
* permission is obtained from Baker Hughes.
*
*
* @file     cUartBluetooth.h
* @author   Elvis Esa
* @date     13 November 2020
* @brief
*/
#ifndef __UART_BLUETOOTH_H
#define __UART_BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "misra.h"

MISRAC_DISABLE
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
MISRAC_ENABLE

/* Exported types ------------------------------------------------------------*/

typedef enum { eUART_Bluetooth_POWER_OFF = 0, eUART_Bluetooth_POWER_ON = 1, eUART_Bluetooth_POWER_MAX = 2, eUART_Bluetooth_POWER_END = 0xFFFFFFFFu } eUartPower_t;

/* Exported constants --------------------------------------------------------*/

/* None */

/* Exported macro ------------------------------------------------------------*/

/* None */

/* Exported functions prototypes ---------------------------------------------*/

void UART_Bluetooth_init(void);
bool UART_Bluetooth_overCurrentOk( void );
bool UART_Bluetooth_power( eUartPower_t ePower );

void UART_Bluetooth_send(char *aTxBuffer, uint32_t size);
bool UART_Bluetooth_ClearRcvBuffer(void);
bool UART_Bluetooth_rcvWait(uint32_t max);
void UART_Bluetooth_IRQHandler(void);
char* UART_Bluetooth_getRcvBuffer(void);
uint16_t UART_Bluetooth_getSize(void);

/* ---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif //__UART_BLUETOOTH_H


