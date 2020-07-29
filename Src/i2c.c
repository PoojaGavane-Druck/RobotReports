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
* @file        i2c.c
* @version     1.00.00
* @author      Piyali Sengupta
* @date        3 October 2018
* @brief       I2C initialization and Deinitialization Functions, Interrupt Handlers and I/O Functions:
*/
/************************************************** Includes ************************************************************/

#include "misra.h"
MISRAC_DISABLE
#include <stdio.h>
#include <stm32l4xx_hal.h>
#include <ctype.h>
#include <cpu.h>
#include <lib_mem.h>
#include <os.h>
//#include <bsp_clk.h>
#include <bsp_os.h>
#include <bsp_int.h>
#include <os_app_hooks.h>
MISRAC_ENABLE

#include "i2c.h"
#include "main.h"
//#include "HAL_I2C_Callback.h"


/***************************************** SEMAPHORES MUTEXES AND STATUS FLAG VOLATILES ****************************************************/

static OS_ERR i2c_p_err[I2CnSIZE];;

static OS_MUTEX i2cMutex[I2CnSIZE];
static OS_SEM i2cRxSem[I2CnSIZE];
static OS_SEM i2cTxSem[I2CnSIZE];

#define MAX_I2CTRIALS 2u

static const uint32_t MAX_ATTEMPTS = 3u;

/************************************* GLOBALS ************************************************************************/

static I2C_HandleTypeDef I2cHandle[I2CnSIZE];
extern const uint32_t DPI705E_I2C1_TIMING;
extern const uint32_t DPI705E_I2C2_TIMING;

/************************************************* PROTOTYPE **********************************************************/
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void I2C4_EV_IRQHandler(void);
void I2C4_ER_IRQHandler(void);
static void i2cDeinit(eI2CElement_t elem);
static void i2cReinitialise(eI2CElement_t elem);

/************************************************* FUNCTIONS **********************************************************/

/**
 * @brief   Initializes the I2C Handler and creates semaphore for transmission and receipt
 * @param   elem The I2C instance
 */

void i2cInit(eI2CElement_t elem)
{
    /******* SEMAPHORE INITIALIZATION *********/

    OSSemCreate(&i2cRxSem[elem], "i2cRCVSem", (OS_SEM_CTR)0u, &i2c_p_err[elem] );

    if (i2c_p_err[elem] != OS_ERR_NONE )
    {
        //setError(E_ERROR_I2C_DRIVER);
    }

    OSSemCreate(&i2cTxSem[elem], "i2cSendSem", (OS_SEM_CTR)0u, &i2c_p_err[elem] );

    if (i2c_p_err[elem] != OS_ERR_NONE )
    {
        //setError(E_ERROR_I2C_DRIVER);
    }

    OSMutexCreate(&i2cMutex[elem], "i2cMutex", &i2c_p_err[elem] );

    if (i2c_p_err[elem] != OS_ERR_NONE )
    {
        //setError(E_ERROR_I2C_DRIVER);
    }

    /****** I2C Handler INITIALIZATION *******/
    if (HAL_I2C_GetState( &I2cHandle[elem]) == HAL_I2C_STATE_RESET)
    {
        if (elem == I2Cn4)
        {
            I2cHandle[I2Cn4].Instance = I2C4;
            I2cHandle[I2Cn4].Init.Timing = DPI705E_I2C2_TIMING;
        }
        else if (elem == I2Cn1)
        {
            I2cHandle[I2Cn1].Instance = I2C1;
            I2cHandle[I2Cn1].Init.Timing = DPI705E_I2C1_TIMING;
        }
        else
        {
          //setError(E_ERROR_I2C_DRIVER);
        }
        I2cHandle[elem].Init.OwnAddress1 = 0u;
        I2cHandle[elem].Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        I2cHandle[elem].Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        I2cHandle[elem].Init.OwnAddress2 = 0u;
        I2cHandle[elem].Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        I2cHandle[elem].Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

        /** Initialize the I2C Instance ***/
        if( HAL_I2C_Init(&I2cHandle[elem]) != HAL_OK)
        {
            //setError(E_ERROR_I2C_DRIVER);
        }
    }
}


/**
 * @brief   I2C Memory write Interrupt Callback
 * @param   hi2c I2C Handler
 */

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    eI2CElement_t idx = I2CnNone;

    if (hi2c->Instance == I2C1)
    {
        idx = I2Cn1;
    }
    else if (hi2c->Instance == I2C4)
    {
        idx = I2Cn4;
    }
    else
    {
        //setError(E_ERROR_I2C_DRIVER);
    }

    if (idx != I2CnNone)
    {
        /**** Post Tx Semaphore to finish transaction *****/
        OSSemPost(&i2cTxSem[idx], OS_OPT_POST_1, &i2c_p_err[idx] );
    }
}


/**
 * @brief	 I2C Memory Read Interrupt Callback
 * @param	 hi2c I2C Handler
 */

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    eI2CElement_t idx = I2CnNone;

    if (hi2c->Instance == I2C1)
    {
        idx = I2Cn1;
    }
    else if (hi2c->Instance == I2C4)
    {
        idx = I2Cn4;
    }
    else
    {
        //setError(E_ERROR_I2C_DRIVER);
    }

    if (idx != I2CnNone)
    {
        /**** Post Rx Semaphore to finish transaction *****/
        OSSemPost(&i2cRxSem[idx], OS_OPT_POST_1, &i2c_p_err[idx]  );
    }
}


/**
 * @brief   I2C1 test mode it initialize the I2C1 SCL and SDA pins as GPIO with output open drain,no pull configuration
 */

void i2c1TestMode(void)
{
    
}


/**
 * @brief   I2C Deinitialization Function
 * @param   elem
 */

static void i2cDeinit(eI2CElement_t elem)
{
    if ((elem == I2Cn1) || (elem == I2Cn4))
    {
        if (HAL_I2C_GetState(&I2cHandle[elem]) !=  HAL_I2C_STATE_RESET )
        {
            /* Deinitialize the I2C */
            HAL_I2C_DeInit(&I2cHandle[elem]);
            //I2C1_MspDeInit(&I2cHandle[elem]);
        }
    }
    else
    {
        //setError(E_ERROR_I2C_DRIVER);
    }
}


/**
 * @brief   I2C Error Handler to re-initialise the I2C Handlers
 * @param   elem I2C Instance
 */

static void i2cReinitialise(eI2CElement_t elem)
{
    OS_ERR os_error;
    //De-initialize and re-initialise the I2C bus
    HAL_I2C_DeInit(&I2cHandle[elem]);
    HAL_I2C_Init(&I2cHandle[elem]);
    
     OSTimeDlyHMSM(0u, 0u, 0u, 10, OS_OPT_TIME_HMSM_STRICT, &os_error);
}


/**
 * @brief   Function to Write to I2C device memory buffer
 * @param   elem The I2Cnx bus instance where x=1,2..
 * @param   Addr Address of I2C device
 * @param   devMemLocation address of destination buffer in device to write to
 * @param   devMemSize I2C device memeory size
 * @param   pBuffer address of source buffer in microcontroller to write from
 * @param   Length length of data to be transferred
 * @return  HAL Status of transaction
 */

HAL_StatusTypeDef I2C_WriteBuffer(eI2CElement_t elem, uint16_t Addr, uint16_t devMemLocation, uint16_t devMemSize, uint8_t *pBuffer, uint16_t Length)
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t attempts = 0u;

    OSMutexPend(&i2cMutex[elem],0u, OS_OPT_PEND_BLOCKING,(CPU_TS *)0, &i2c_p_err[elem]);

    while ((status != HAL_OK) && (attempts < MAX_ATTEMPTS))
    {
        //if this is not the first attempt then re-initialise the bus
        if (attempts > 0u)
        {
            i2cReinitialise(elem);
        }

        //check device ready
        status = HAL_I2C_IsDeviceReady(&I2cHandle[elem], Addr, 5u, 5u); //isDeviceReady(elem, Addr);

        if (status == HAL_OK)
        {
            // Write to I2C memory via Interrupt
            status = HAL_I2C_Mem_Write_IT(&I2cHandle[elem], Addr, (uint16_t)devMemLocation, devMemSize, pBuffer, Length);

            if (status == HAL_OK)
            {
                //Pending on i2cTxSem for 500ms to complete transaction
                OSSemPend(&i2cTxSem[elem], 500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &i2c_p_err[elem]);
            }
        }

        attempts++;
    }

    OSMutexPost(&i2cMutex[elem], OS_OPT_POST_NO_SCHED, &i2c_p_err[elem]);

    return status;
}


/**
 * @brief   Function to Read from I2C device memory buffer
 * @param   elem The I2Cnx bus instance where x=1,2..
 * @param   Addr Address of I2C device
 * @param   devMemLocation address of source buffer in device to read from
 * @param   devMemSize device memeory size
 * @param   pBuffer address of destination buffer in microcontroller to read to
 * @param   Length length of data to be transferred
 * @return  HAL Status of transaction
 */

HAL_StatusTypeDef I2C_ReadBuffer(eI2CElement_t elem, uint16_t Addr, uint16_t devMemLocation, uint16_t devMemSize, uint8_t *pBuffer, uint16_t Length)
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t attempts = 0u;

    OSMutexPend(&i2cMutex[elem], 0u, OS_OPT_PEND_BLOCKING,(CPU_TS *)0, &i2c_p_err[elem]);

    while ((status != HAL_OK) && (attempts < MAX_ATTEMPTS))
    {
        //if this is not the first attempt then re-initialise the bus
        if (attempts > 0u)
        {
            i2cReinitialise(elem);
        }

        //check device ready
        status = HAL_I2C_IsDeviceReady(&I2cHandle[elem], Addr, 5u, 5u); //isDeviceReady(elem, Addr);

        if (status == HAL_OK)
        {
            //I2C Interrupt READ
            status = HAL_I2C_Mem_Read_IT(&I2cHandle[elem], Addr, (uint16_t)devMemLocation, devMemSize, pBuffer, Length);

            if (status == HAL_OK)
            {
                // Pending on i2cRxSem for 500ms to complete transaction
                OSSemPend(&i2cRxSem[elem], 500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &i2c_p_err[elem]);
            }
        }

        attempts++;
    }

    OSMutexPost(&i2cMutex[elem], OS_OPT_POST_NO_SCHED, &i2c_p_err[elem]);

    return status;
}


/**
 * @brief I2C1 Event Interrupt Request Handler
 */

void I2C1_EV_IRQHandler(void)
{
    OSIntEnter();
    HAL_I2C_EV_IRQHandler( &I2cHandle[I2Cn1] );
    OSIntExit();
}


/**
 * @brief   I2C1 Error Request Handler
 */

void I2C1_ER_IRQHandler(void)
{
    OSIntEnter();
    HAL_I2C_ER_IRQHandler( &I2cHandle[I2Cn1] );
    OSIntExit();
}


/**
 * @brief   I2C2 Event Interrupt Request Handler
 */

void I2C4_EV_IRQHandler(void)
{
    OSIntEnter();
    HAL_I2C_EV_IRQHandler(&I2cHandle[I2Cn4]);
    OSIntExit();
}


/**
 * @brief   I2C2 Error Request Handler
 */

void I2C4_ER_IRQHandler(void)
{
    OSIntEnter();
    HAL_I2C_ER_IRQHandler(&I2cHandle[I2Cn4]);
    OSIntExit();
}


/************************************************** END OF FUNCTIONS ***************************************************/



















