/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : stm32l4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "misra.h"
#include "main.h"
#include "uart.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
    /* USER CODE BEGIN MspInit 0 */

    /* USER CODE END MspInit 0 */

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* System interrupt init*/

    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
}

/**
* @brief CRC MSP Initialization
* This function configures the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
void HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc)
{
    if(hcrc->Instance == CRC)
    {
        /* USER CODE BEGIN CRC_MspInit 0 */

        /* USER CODE END CRC_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_CRC_CLK_ENABLE();
        /* USER CODE BEGIN CRC_MspInit 1 */

        /* USER CODE END CRC_MspInit 1 */
    }

}

/**
* @brief CRC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
void HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc)
{
    if(hcrc->Instance == CRC)
    {
        /* USER CODE BEGIN CRC_MspDeInit 0 */

        /* USER CODE END CRC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CRC_CLK_DISABLE();
        /* USER CODE BEGIN CRC_MspDeInit 1 */

        /* USER CODE END CRC_MspDeInit 1 */
    }

}

/**
* @brief I2C MSP Initialization
* This function configures the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if(hi2c->Instance == BAT_INTERFACE)
    {
        /* USER CODE BEGIN I2C1_MspInit 0 */

        /* USER CODE END I2C1_MspInit 0 */

        __HAL_RCC_GPIOG_CLK_ENABLE();
        HAL_PWREx_EnableVddIO2();
        /**I2C1 GPIO Configuration
        PG13     ------> I2C1_SDA
        PG14     ------> I2C1_SCL
        */
        GPIO_InitStruct.Pin = BAT_INTERFACE_I2C1_SDA_Pin | BAT_INTERFACE_I2C1_SCL_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = BAT_INTERFACE_I2C1_AF_NUM;
        HAL_GPIO_Init(BAT_INTERFACE_I2C1_SDA_GPIO_Port, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_I2C1_CLK_ENABLE();
        /* USER CODE BEGIN I2C1_MspInit 1 */

        /* USER CODE END I2C1_MspInit 1 */
    }

    else if(hi2c->Instance == BAROMETER_EEPROM_INTERFACE)
    {
        /* USER CODE BEGIN I2C1_MspInit 0 */

        /* USER CODE END I2C1_MspInit 0 */

        __HAL_RCC_GPIOG_CLK_ENABLE();
        HAL_PWREx_EnableVddIO2();
        /**I2C1 GPIO Configuration
        PG13     ------> I2C1_SDA
        PG14     ------> I2C1_SCL
        */
        GPIO_InitStruct.Pin = BAROMETER_EEPROM_INTERFACE_I2C4_SDA_Pin | BAROMETER_EEPROM_INTERFACE_I2C4_SCL_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C4;
        HAL_GPIO_Init(BAROMETER_EEPROM_INTERFACE_I2C4_SDA_GPIO_Port, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_I2C4_CLK_ENABLE();
        /* USER CODE BEGIN I2C1_MspInit 1 */

        /* USER CODE END I2C1_MspInit 1 */
    }

}

/**
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == BAT_INTERFACE)
    {
        /* USER CODE BEGIN I2C1_MspDeInit 0 */

        /* USER CODE END I2C1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C1_CLK_DISABLE();

        /**I2C1 GPIO Configuration
        PG13     ------> I2C1_SDA
        PG14     ------> I2C1_SCL
        */
        HAL_GPIO_DeInit(BAT_INTERFACE_I2C1_SDA_GPIO_Port,
                        BAT_INTERFACE_I2C1_SDA_Pin | BAT_INTERFACE_I2C1_SCL_Pin);

        /* USER CODE BEGIN I2C1_MspDeInit 1 */

        /* USER CODE END I2C1_MspDeInit 1 */
    }

    if(hi2c->Instance == BAROMETER_EEPROM_INTERFACE)
    {
        /* USER CODE BEGIN I2C4_MspDeInit 0 */

        /* USER CODE END I2C4_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C4_CLK_DISABLE();

        /**I2C1 GPIO Configuration
        PG13     ------> I2C4_SDA
        PG14     ------> I2C4_SCL
        */
        HAL_GPIO_DeInit(BAROMETER_EEPROM_INTERFACE_I2C4_SDA_GPIO_Port,
                        BAROMETER_EEPROM_INTERFACE_I2C4_SDA_Pin | BAROMETER_EEPROM_INTERFACE_I2C4_SCL_Pin);

        /* USER CODE BEGIN I2C4_MspDeInit 1 */

        /* USER CODE END I2C4_MspDeInit 1 */
    }
}

/**
* @brief LPTIM MSP Initialization
* This function configures the hardware resources used in this example
* @param hlptim: LPTIM handle pointer
* @retval None
*/
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef *hlptim)
{
    if(hlptim->Instance == LPTIM1)
    {
        /* USER CODE BEGIN LPTIM1_MspInit 0 */

        /* USER CODE END LPTIM1_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_LPTIM1_CLK_ENABLE();
        /* USER CODE BEGIN LPTIM1_MspInit 1 */

        /* USER CODE END LPTIM1_MspInit 1 */
    }

}

/**
* @brief LPTIM MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hlptim: LPTIM handle pointer
* @retval None
*/
void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef *hlptim)
{
    if(hlptim->Instance == LPTIM1)
    {
        /* USER CODE BEGIN LPTIM1_MspDeInit 0 */

        /* USER CODE END LPTIM1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_LPTIM1_CLK_DISABLE();

        /* LPTIM1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
        /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

        /* USER CODE END LPTIM1_MspDeInit 1 */
    }

}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    RCC_PeriphCLKInitTypeDef periphClkInit;


    if(LPUART1 == huart->Instance)
    {
        periphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
        periphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;

        if(HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
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

        if(HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
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

        if(HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
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

        if(HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
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

        if(HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
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
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{

    ////////////////////////////////////////////////////////////////////
    if(LPUART1 == huart->Instance)
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

    ////////////////////////////////////////////////////////////////////


}



/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim_base)
{
    if(htim_base->Instance == TIM3)
    {
        /* USER CODE BEGIN TIM3_MspInit 0 */

        /* USER CODE END TIM3_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
        /* TIM3 interrupt Init */
        HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
        /* USER CODE BEGIN TIM3_MspInit 1 */

        /* USER CODE END TIM3_MspInit 1 */
    }

}

/**
* @brief TIM_Base MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim_base)
{
    if(htim_base->Instance == TIM3)
    {
        /* USER CODE BEGIN TIM3_MspDeInit 0 */

        /* USER CODE END TIM3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();

        /* TIM3 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
        /* USER CODE BEGIN TIM3_MspDeInit 1 */

        /* USER CODE END TIM3_MspDeInit 1 */
    }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
