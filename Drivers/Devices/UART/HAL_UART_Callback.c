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
* @file     HAL_UART_Callback.c
* @version  1.0
* @author   Julio Andrade
* @date     Jul 2, 2018
* @brief    your file decription here
*/

#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

//#include "gpio.h"
#include "uart.h"


/**
 * @brief UART MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's clock enable
 *           - Peripheral's GPIO Configuration
 *           - NVIC configuration for UART interrupt request enable
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef periphClkInit;
    
   
    if(USART1 == huart->Instance)
    {
       periphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
       periphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
       if (HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
      {

      }
      /*##-1- Enable peripherals and GPIO Clocks #################################*/
      /* Enable GPIO TX/RX clock */
      USART1_TX_GPIO_CLK_ENABLE();
      USART1_RX_GPIO_CLK_ENABLE();
      /* Enable USARTx clock */
      USART1_CLK_ENABLE();

      disableSerialPortTxLine(UART_PORT1);
      /*##-2- Configure peripheral GPIO ##########################################*/
      /* UART TX GPIO pin configuration  */
      GPIO_InitStruct.Pin = USART1_TX_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = USART1_TX_PULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = USART1_TX_AF;
      HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);
      /* UART RX GPIO pin configuration  */
      GPIO_InitStruct.Pin = USART1_RX_PIN;
      GPIO_InitStruct.Alternate = USART1_RX_AF;
      GPIO_InitStruct.Pull = USART1_RX_PULL;
      HAL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);

      /*##-3- Configure the NVIC for UART ########################################*/
      /* NVIC for USART1 */
      HAL_NVIC_SetPriority(USART1_IRQn, 0u, 0u); //interrupt priority
      HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
    else if(USART2 == huart->Instance)
    {
       periphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
       periphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
       if (HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
      {

      }
      /*##-1- Enable peripherals and GPIO Clocks #################################*/
      /* Enable GPIO TX/RX clock */
      USART2_TX_GPIO_CLK_ENABLE();
      USART2_RX_GPIO_CLK_ENABLE();
      /* Enable USARTx clock */
      USART2_CLK_ENABLE();

      disableSerialPortTxLine(UART_PORT2);
      /*##-2- Configure peripheral GPIO ##########################################*/
      /* UART TX GPIO pin configuration  */
      GPIO_InitStruct.Pin = USART2_TX_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = USART2_TX_PULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = USART2_TX_AF;
      HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);
      /* UART RX GPIO pin configuration  */
      GPIO_InitStruct.Pin = USART2_RX_PIN;
      GPIO_InitStruct.Alternate = USART2_RX_AF;
      GPIO_InitStruct.Pull = USART2_RX_PULL;
      HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);

      /*##-3- Configure the NVIC for UART ########################################*/
      /* NVIC for USART1 */
      HAL_NVIC_SetPriority(USART2_IRQn, 0u, 0u); //interrupt priority
      HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else if(USART3 == huart->Instance)
    {
       periphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
       periphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
       if (HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
      {

      }
      /*##-1- Enable peripherals and GPIO Clocks #################################*/
      /* Enable GPIO TX/RX clock */
      USART3_TX_GPIO_CLK_ENABLE();
      USART3_RX_GPIO_CLK_ENABLE();
      /* Enable USARTx clock */
      USART3_CLK_ENABLE();

      disableSerialPortTxLine(UART_PORT3);
      /*##-2- Configure peripheral GPIO ##########################################*/
      /* UART TX GPIO pin configuration  */
      GPIO_InitStruct.Pin = USART3_TX_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = USART3_TX_PULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = USART3_TX_AF;
      HAL_GPIO_Init(USART3_TX_GPIO_PORT, &GPIO_InitStruct);
      /* UART RX GPIO pin configuration  */
      GPIO_InitStruct.Pin = USART3_RX_PIN;
      GPIO_InitStruct.Alternate = USART3_RX_AF;
      GPIO_InitStruct.Pull = USART3_RX_PULL;
      HAL_GPIO_Init(USART3_RX_GPIO_PORT, &GPIO_InitStruct);

      /*##-3- Configure the NVIC for UART ########################################*/
      /* NVIC for USART1 */
      HAL_NVIC_SetPriority(USART3_IRQn, 0u, 0u); //interrupt priority
      HAL_NVIC_EnableIRQ(USART3_IRQn);
    }
    else if(UART4 == huart->Instance)
    {
       periphClkInit.PeriphClockSelection = RCC_PERIPHCLK_UART4;
       periphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
       if (HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
      {

      }
      /*##-1- Enable peripherals and GPIO Clocks #################################*/
      /* Enable GPIO TX/RX clock */
      UART4_TX_GPIO_CLK_ENABLE();
      UART4_RX_GPIO_CLK_ENABLE();
      /* Enable USARTx clock */
      UART4_CLK_ENABLE();

      disableSerialPortTxLine(UART_PORT4);
      /*##-2- Configure peripheral GPIO ##########################################*/
      /* UART TX GPIO pin configuration  */
      GPIO_InitStruct.Pin = UART4_TX_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = UART4_TX_PULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = UART4_TX_AF;
      HAL_GPIO_Init(UART4_TX_GPIO_PORT, &GPIO_InitStruct);
      /* UART RX GPIO pin configuration  */
      GPIO_InitStruct.Pin = UART4_RX_PIN;
      GPIO_InitStruct.Alternate = UART4_RX_AF;
      GPIO_InitStruct.Pull = UART4_RX_PULL;
      HAL_GPIO_Init(UART4_RX_GPIO_PORT, &GPIO_InitStruct);

      /*##-3- Configure the NVIC for UART ########################################*/
      /* NVIC for USART1 */
      HAL_NVIC_SetPriority(UART4_IRQn, 0u, 0u); //interrupt priority
      HAL_NVIC_EnableIRQ(UART4_IRQn);
    }
    else if(UART5 == huart->Instance)
    {
       periphClkInit.PeriphClockSelection = RCC_PERIPHCLK_UART4;
       periphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
       if (HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
      {

      }
      /*##-1- Enable peripherals and GPIO Clocks #################################*/
      /* Enable GPIO TX/RX clock */
      UART5_TX_GPIO_CLK_ENABLE();
      UART5_RX_GPIO_CLK_ENABLE();
      /* Enable USARTx clock */
      UART5_CLK_ENABLE();

      disableSerialPortTxLine(UART_PORT5);
      /*##-2- Configure peripheral GPIO ##########################################*/
      /* UART TX GPIO pin configuration  */
      GPIO_InitStruct.Pin = UART5_TX_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = UART5_TX_PULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = UART5_TX_AF;
      HAL_GPIO_Init(UART5_TX_GPIO_PORT, &GPIO_InitStruct);
      /* UART RX GPIO pin configuration  */
      GPIO_InitStruct.Pin = UART5_RX_PIN;
      GPIO_InitStruct.Alternate = UART5_RX_AF;
      GPIO_InitStruct.Pull = UART5_RX_PULL;
      HAL_GPIO_Init(UART5_RX_GPIO_PORT, &GPIO_InitStruct);

      /*##-3- Configure the NVIC for UART ########################################*/
      /* NVIC for USART1 */
      HAL_NVIC_SetPriority(UART5_IRQn, 0u, 0u); //interrupt priority
      HAL_NVIC_EnableIRQ(UART5_IRQn);
    }
    else
    {
      /* Do Nothing */
    }
}

/**
 * @brief UART MSP De-Initialization
 *        This function frees the hardware resources used in this example:
 *          - Disable the Peripheral's clock
 *          - Revert GPIO and NVIC configuration to their default state
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if(USART1 == huart->Instance)
    {
      /*##-1- Reset peripherals ##################################################*/
      USART1_FORCE_RESET();
      USART1_RELEASE_RESET();
      /*##-2- Disable peripherals and GPIO Clocks #################################*/
      /* De-Initialize USART1 Tx */
      HAL_GPIO_DeInit(USART1_TX_GPIO_PORT, USART1_TX_PIN);
      /* De-Initialize USART1 Rx */
      HAL_GPIO_DeInit(USART1_RX_GPIO_PORT, USART1_RX_PIN);
      /*##-3- Disable the NVIC for UART ###########################################*/
      HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
    else if(USART2 == huart->Instance)
    {
      /*##-1- Reset peripherals ##################################################*/
      USART2_FORCE_RESET();
      USART2_RELEASE_RESET();
      /*##-2- Disable peripherals and GPIO Clocks #################################*/
      /* De-Initialize USART1 Tx */
      HAL_GPIO_DeInit(USART2_TX_GPIO_PORT, USART2_TX_PIN);
      /* De-Initialize USART1 Rx */
      HAL_GPIO_DeInit(USART2_RX_GPIO_PORT, USART2_RX_PIN);
      /*##-3- Disable the NVIC for UART ###########################################*/
      HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
    else if(USART3 == huart->Instance)
    {
      /*##-1- Reset peripherals ##################################################*/
      USART3_FORCE_RESET();
      USART3_RELEASE_RESET();
      /*##-2- Disable peripherals and GPIO Clocks #################################*/
      /* De-Initialize USART1 Tx */
      HAL_GPIO_DeInit(USART3_TX_GPIO_PORT, USART3_TX_PIN);
      /* De-Initialize USART1 Rx */
      HAL_GPIO_DeInit(USART3_RX_GPIO_PORT, USART3_RX_PIN);
      /*##-3- Disable the NVIC for UART ###########################################*/
      HAL_NVIC_DisableIRQ(USART3_IRQn);
    }
    else if(UART4 == huart->Instance)
    {
      /*##-1- Reset peripherals ##################################################*/
      UART4_FORCE_RESET();
      UART4_RELEASE_RESET();
      /*##-2- Disable peripherals and GPIO Clocks #################################*/
      /* De-Initialize USART1 Tx */
      HAL_GPIO_DeInit(UART4_TX_GPIO_PORT, UART4_TX_PIN);
      /* De-Initialize USART1 Rx */
      HAL_GPIO_DeInit(UART4_RX_GPIO_PORT, UART4_RX_PIN);
      /*##-3- Disable the NVIC for UART ###########################################*/
      HAL_NVIC_DisableIRQ(UART4_IRQn);
    }
    else if(UART5 == huart->Instance)
    {
      /*##-1- Reset peripherals ##################################################*/
      UART5_FORCE_RESET();
      UART5_RELEASE_RESET();
      /*##-2- Disable peripherals and GPIO Clocks #################################*/
      /* De-Initialize USART1 Tx */
      HAL_GPIO_DeInit(UART5_TX_GPIO_PORT, UART5_TX_PIN);
      /* De-Initialize USART1 Rx */
      HAL_GPIO_DeInit(UART5_RX_GPIO_PORT, UART5_RX_PIN);
      /*##-3- Disable the NVIC for UART ###########################################*/
      HAL_NVIC_DisableIRQ(UART5_IRQn);
    }
    else
    {
      /* Do Nothing */
    }
}
