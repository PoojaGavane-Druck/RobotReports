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
  MAX_NUM_OF_UART_PORTS
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
#define USART2_RX_BUFF_SIZE 			  256u
#define USART3_RX_BUFF_SIZE 			  256u
#define UART4_RX_BUFF_SIZE 			  256u
#define UART5_RX_BUFF_SIZE 			  256u

#define USART1_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()

#define USART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define USART1_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USART1_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USART1_TX_PIN                    GPIO_PIN_6
#define USART1_TX_GPIO_PORT              GPIOB
#define USART1_TX_AF                     GPIO_AF7_USART1
#define USART1_RX_PIN                    GPIO_PIN_7
#define USART1_RX_GPIO_PORT              GPIOB
#define USART1_RX_AF                     GPIO_AF7_USART1

#define USART1_RX_PULL                   GPIO_PULLUP
#define USART1_TX_PULL                   GPIO_NOPULL

/* Definition for USARTx's NVIC */


/* Definition for USARTx's DMA */
#define USART1_TX_DMA_CHANNEL            DMA1_Channel4
#define USART1_RX_DMA_CHANNEL            DMA1_Channel5

/* Definition for USARTx's DMA Request */
#define USARTx_TX_DMA_REQUEST             DMA_REQUEST_2
#define USARTx_RX_DMA_REQUEST             DMA_REQUEST_2
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn               DMA1_Channel4_IRQn
#define USARTx_DMA_RX_IRQn               DMA1_Channel5_IRQn
#define USARTx_DMA_TX_IRQHandler         DMA1_Channel4_IRQHandler
#define USARTx_DMA_RX_IRQHandler         DMA1_Channel5_IRQHandler



#define USART2_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()

#define USART2_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USART2_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USART2_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USART2_TX_PIN                    GPIO_PIN_2
#define USART2_TX_GPIO_PORT              GPIOA
#define USART2_TX_AF                     GPIO_AF7_USART2
#define USART2_RX_PIN                    GPIO_PIN_3
#define USART2_RX_GPIO_PORT              GPIOA
#define USART2_RX_AF                     GPIO_AF7_USART2

#define USART2_RX_PULL                   GPIO_PULLUP
#define USART2_TX_PULL                   GPIO_NOPULL

/* Definition for USARTx's NVIC */

#define USART3_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE()

#define USART3_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USART3_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()

#define USART3_FORCE_RESET()             __HAL_RCC_USART3_FORCE_RESET()
#define USART3_RELEASE_RESET()           __HAL_RCC_USART3_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USART3_TX_PIN              GPIO_PIN_8
#define USART3_TX_GPIO_PORT              GPIOD
#define USART3_TX_AF                     GPIO_AF7_USART3
#define USART3_RX_PIN                    GPIO_PIN_9
#define USART3_RX_GPIO_PORT              GPIOD
#define USART3_RX_AF                     GPIO_AF7_USART3

#define USART3_RX_PULL                   GPIO_PULLUP
#define USART3_TX_PULL                   GPIO_NOPULL


#define UART4_CLK_ENABLE()             __HAL_RCC_UART4_CLK_ENABLE()
#define UART4_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define UART4_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()

#define UART4_FORCE_RESET()           __HAL_RCC_UART4_FORCE_RESET()
#define UART4_RELEASE_RESET()         __HAL_RCC_UART4_RELEASE_RESET()

/* Definition for USARTx Pins */
#define UART4_TX_PIN                    GPIO_PIN_9
#define UART4_TX_GPIO_PORT              GPIOA
#define UART4_TX_AF                     GPIO_AF8_UART4
#define UART4_RX_PIN                    GPIO_PIN_10
#define UART4_RX_GPIO_PORT              GPIOA
#define UART4_RX_AF                     GPIO_AF8_UART4

#define UART4_RX_PULL                   GPIO_PULLUP
#define UART4_TX_PULL                   GPIO_NOPULL

#define UART5_CLK_ENABLE()             __HAL_RCC_UART4_CLK_ENABLE()

#define UART5_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define UART5_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOD_CLK_ENABLE()

#define UART5_FORCE_RESET()           __HAL_RCC_UART5_FORCE_RESET()
#define UART5_RELEASE_RESET()         __HAL_RCC_UART5_RELEASE_RESET()

/* Definition for USARTx Pins */
#define UART5_TX_PIN                    GPIO_PIN_12
#define UART5_TX_GPIO_PORT              GPIOC
#define UART5_TX_AF                     GPIO_AF8_UART5
#define UART5_RX_PIN                    GPIO_PIN_2
#define UART5_RX_GPIO_PORT              GPIOD
#define UART5_RX_AF                     GPIO_AF8_UART5

#define UART5_RX_PULL                   GPIO_PULLUP
#define UART5_TX_PULL                   GPIO_NOPULL

bool uartInit(USART_ConfigParams configParams);
bool uartDeInit(PortNumber_t portNumber);

void sendOverUSART1(uint8_t *aTxBuffer, uint32_t size);
void sendOverUSART2(uint8_t *aTxBuffer, uint32_t size);
void sendOverUSART3(uint8_t *aTxBuffer, uint32_t size);
void sendOverUART4(uint8_t *aTxBuffer, uint32_t size); 
void sendOverUART5(uint8_t *aTxBuffer, uint32_t size);

bool ClearUARTxRcvBuffer(PortNumber_t portNumber);
bool getAvailableUARTxReceivedByteCount(PortNumber_t portNumber,
                                            uint32_t* avlBytes);
bool getHandleToUARTxRcvBuffer(PortNumber_t portNumber, uint8_t* bufHdl);

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

#ifdef __cplusplus
}
#endif

#endif //__UART_H


