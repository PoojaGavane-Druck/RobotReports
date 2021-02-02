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
* @file     SPI.c
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     02 November 2020
*
* @brief    SPI source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "main.h"
#include "SPI.h"


/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

extern SPI_HandleTypeDef hspi1;
static volatile uint32_t spiComplete;
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Issues dSPIN Reset Device command.
  * @param  None
  * @retval None
  */
uint8_t SPI_TransmitReceive(uint8_t byte)
{
    /* Sends and receives data from SPI */
    uint8_t rcvByte;
    spiComplete = (uint32_t)(0);

    HAL_SPI_TransmitReceive_IT(&hspi1, &byte, &rcvByte, (uint16_t)(1));

    while((uint32_t)(1) != spiComplete)
    {

    }
    spiComplete = (uint32_t)(0);
    
    return rcvByte;
}

/**
  * @brief  Issues dSPIN Reset Device command.
  * @param  None
  * @retval None
  */
void SPI_SetNss(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);
}

/**
  * @brief  Issues dSPIN Reset Device command.
  * @param  None
  * @retval None
  */
void SPI_ResetNss(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET);
}

/**
  * @brief  Issues dSPIN Reset Device command.
  * @param  None
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    spiComplete = (uint32_t)(1);
}

/**
  * @brief  Issues dSPIN Reset Device command.
  * @param  None
  * @retval None
  */
uint16_t GPIO_ReadOutputDataBit(GPIO_TypeDef *port, uint16_t pin)
{
    return 0; // Todo
}