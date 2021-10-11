/**
* Baker Hughes Confidential\n* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file		spi.c
* @version	1.00.00
* @author	Makarand Deshmukh
* @date		06-09-2021
*
* @brief	SPI module source file
*/

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include <stdbool.h>
#include <stm32l4xx_hal.h>
#include <os.h>
MISRAC_ENABLE

#include "spi.h"

/* Defines and constants ----------------------------------------------------*/
#define MAX_SPI_SEM_NAME_SIZE 25u
//#define POLL_GPIO_PIN
/* Types --------------------------------------------------------------------*/

/* Global Variables ---------------------------------------------------------*/
OS_SEM spiSemRx;
OS_SEM spiSemTx;
OS_SEM spiDataReady;
OS_ERR spiError;

static char spiTxSemName[MAX_SPI_SEM_NAME_SIZE] = {"SpiTx"};
static char spiRxSemName[MAX_SPI_SEM_NAME_SIZE] = {"SpiRx"};
static char spiDrdySemName[MAX_SPI_SEM_NAME_SIZE] = {"SpiDrdy"};

/* File Statics -------------------------------------------------------------*/

/* User Code ----------------------------------------------------------------*/

/**
* @brief	spi class constructor
* @param	void
* @retval	void
*/
spi::spi(SPI_HandleTypeDef *spiInstance)
{
    uint32_t error = 0u;
    
    spiHandle = spiInstance;
    
    OSSemCreate(&spiSemRx,
                  spiRxSemName,  
                  (OS_SEM_CTR)0, 
                  &spiError);
    
    if(OS_ERR_NONE != spiError)
    {
        error = 1u;
    }

    if(0u == error)
    {
        OSSemCreate(&spiSemTx,
                      spiTxSemName,  
                      (OS_SEM_CTR)0, 
                      &spiError);      
    }
    
    if(OS_ERR_NONE != spiError)
    {
        error = 1u;
    }
    
    if(0u == error)
    {
        
        OSSemCreate(&spiDataReady,
                      spiDrdySemName,  
                      (OS_SEM_CTR)0, 
                      &spiError);                       
    }
    
    if(OS_ERR_NONE != spiError)
    {
        error = 1u;
    }    
    
    if(0u == error)
    {    
        /* Set SPI timeout to 10 ms */
        spiTimeout = (uint32_t)(100);
        resetDummyBuffers();
    }
}

/**
* @brief	spi class destructor
* @param	void
* @retval	void
*/
spi::~spi()
{

}

/**
* @brief	transmit some bytes 
* @param	void
* @retval	void
*/
uint32_t spi::resetDummyBuffers(void)
{
    uint32_t status = 0u;
    uint32_t counter = 0u;

    for(counter = 0u; counter < (uint32_t)(BUFFER_SIZE); counter++)
    {
        dummyTx[counter] = (uint8_t)(0);
        dummyRx[counter] = (uint8_t)(0);
    }
    
    return status;
}

/**
* @brief	transmit some bytes 
* @param	void
* @retval	void
*/
uint32_t spi::transmit(uint8_t *data, uint8_t length)
{
    uint32_t status = 0u;

    OSSemSet(&spiSemTx, (OS_SEM_CTR)0, &spiError);
    HAL_SPI_TransmitReceive_IT(spiHandle, data, dummyRx, length);
    OSSemPend(&spiSemTx, (OS_TICK)(spiTimeout), OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &spiError);

    if(spiError == OS_ERR_NONE)
    {
        status = 1u;
    }
    
    return status;
}

/**
* @brief	transmit some bytes and receive data
* @param	void
* @retval	void
*/
uint32_t spi::receive(uint8_t *data, uint8_t length)
{
    uint32_t status = 0u;
    
    OSSemSet(&spiSemTx, (OS_SEM_CTR)0, &spiError);
    HAL_SPI_TransmitReceive_IT(spiHandle, dummyTx, data, length);
    OSSemPend(&spiSemTx, (OS_TICK)(spiTimeout), OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &spiError);

    if(spiError == OS_ERR_NONE)
    {
        status = 1u;
    }
    
    return status;
}

/**
* @brief	transmit some bytes and receive data
* @param	void
* @retval	void
*/
uint32_t spi::getDataReady()
{
    uint32_t status = 0u;
    uint32_t drdyPin = 0u;
#ifdef POLL_GPIO_PIN        
    
    while(GPIO_PIN_RESET == drdyPin)
    {
        drdyPin = HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0);
    }
    
    if(drdyPin == GPIO_PIN_SET)
    {
        status = 1u;
    }
#else
    //OSSemSet(&spiDataReady, (OS_SEM_CTR)0, &spiError);
    drdyPin = HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0);
    OSSemPend(&spiDataReady, (OS_TICK)(spiTimeout), OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &spiError);

    if(spiError == OS_ERR_NONE)
    {
        status = 1u;
    }    
    else
    {
        status = 2u;
        drdyPin = HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0);
    }
#endif
    
    
    return status;
}

/* Interrupt handler callbacks */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPI2)
    {
        OSSemPost(&spiSemTx, OS_OPT_POST_1, &spiError);               
    }
}

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void PIN_ZERO_EXTI_Callback(void)
{
    OSSemPost(&spiDataReady, OS_OPT_POST_1, &spiError);     
}




