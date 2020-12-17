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
*
* @file     uart.h
* @version  1.0
* @author   Julio Andrade
* @date     Jul 2, 2018
* @brief
*/
#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "misra.h"
MISRAC_DISABLE
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE


typedef enum {
    BAUDRATE_300 =0,
    BAUDRATE_1200,
    BAUDRATE_2400,
    BAUDRATE_4800,
    BAUDRATE_9600,
    BAUDRATE_19200,
    BAUDRATE_38400,
    BAUDRATE_57600,
    BAUDRATE_115200
}Baudrate_t;

typedef enum  UsartDirection_tag
{
  DIRECTION_NONE = 0,
  DIRECTION_RX,
  DIRECTION_TX,
  DIRECTION_TX_RX
}Direction_t;


typedef enum DataLength_tag
{
  DATA_LENGTH_7BITS = 0,
  DATA_LENGTH_8BITS,
  DATA_LENGTH_9BITS
}DataLength_t;

typedef enum StopBits_tag
{
  STOPBITS_0_5 = 0,
  STOPBITS_1,
  STOPBITS_1_5,
  STOPBITS_2
  
}StopBits_t;


typedef enum ParityBits_tag
{
  PARITY_NONE = 0,
  PARITY_EVEN,
  PARITY_ODD
}ParityBits_t;

typedef enum  FlowControl_tag
{
  FLOW_CONTROL_NONE = 0,
  FLOW_CONTROL_RTS,
  FLOW_CONTROL_CTS,
  FLOW_CONTROL_RTS_CTS
}FlowControl_t;

/* Following USB Device Speed */
typedef enum
{
  UART_PORT1  = 0U,
  UART_PORT2,  
  UART_PORT3,
  UART_PORT4,
  UART_PORT5,
  MAX_NUM_OF_UART_PORTS,
  UART_INVALID_PORTNUMBER
} PortNumber_t;

typedef enum
{
  OVER_SAMPLE_BY_16 = 0U,
  OVER_SAMPLE_BY_8
}OverSamplingType_t;

typedef struct
{
  PortNumber_t       portNumber;
  Baudrate_t         baudRate;
  DataLength_t       dataLength;
  StopBits_t         numOfStopBits;
  ParityBits_t       parityType;
  FlowControl_t      flowControlMode;
  Direction_t        direction;
  OverSamplingType_t overSamplingType;
  
}USART_ConfigParams;

typedef enum
{
  BY_ASCII_CHARACTER,
  BY_LENGTH
}EndOfFrameDelimitor_t;

#define USART1_RX_BUFF_SIZE 			  256u
#define USART2_RX_BUFF_SIZE 			  8256u
#define USART3_RX_BUFF_SIZE 			  256u
#define UART4_RX_BUFF_SIZE 			  64u
#define UART5_RX_BUFF_SIZE 			  256u



//bool uartInit(USART_ConfigParams configParams);
bool uartInit(UART_HandleTypeDef *huart);
bool uartDeInit(PortNumber_t portNumber);

void sendOverUSART1(uint8_t *aTxBuffer, uint32_t size);
void sendOverUSART2(uint8_t *aTxBuffer, uint32_t size);
void sendOverUSART3(uint8_t *aTxBuffer, uint32_t size);
void sendOverUART4(uint8_t *aTxBuffer, uint32_t size); 
void sendOverUART5(uint8_t *aTxBuffer, uint32_t size);

bool ClearUARTxRcvBuffer(PortNumber_t portNumber);
bool getAvailableUARTxReceivedByteCount(PortNumber_t portNumber,
                                            uint16_t* avlBytes);
bool getHandleToUARTxRcvBuffer(PortNumber_t portNumber, uint8_t** bufHdl);

bool waitToReceiveOverUsart1(uint32_t numberOfToRead, uint32_t timeout);
bool waitToReceiveOverUsart2(uint32_t numberOfToRead, uint32_t timeout);
bool waitToReceiveOverUsart3(uint32_t numberOfToRead, uint32_t timeout);
bool waitToReceiveOverUart4(uint32_t numberOfToRead, uint32_t timeout);
bool waitToReceiveOverUart5(uint32_t numberOfToRead, uint32_t timeout);

void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
bool enableSerialPortTxLine(PortNumber_t portNumber);
bool disableSerialPortTxLine(PortNumber_t portNumber);
static PortNumber_t getUartPortNumber( UART_HandleTypeDef *huart );
#ifdef __cplusplus
}
#endif

#endif //__UART_H


