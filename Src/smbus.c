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
* @file        smbus.c
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

#include "smbus.h"
#include "main.h"


/***************************************** SEMAPHORES MUTEXES AND STATUS FLAG VOLATILES ****************************************************/

static OS_ERR smbus_p_err;

static OS_MUTEX smbusMutex;
static OS_SEM smbusRxSem;
static OS_SEM smbusTxSem;



static const uint32_t MAX_ATTEMPTS = 3u;

/************************************* GLOBALS ************************************************************************/



/************************************************* PROTOTYPE **********************************************************/

static void smbusReinitialise(void);

/************************************************* FUNCTIONS **********************************************************/

/**
 * @brief   Initializes the smbus Handler and creates semaphore for transmission and receipt
 * 
 */

void smbusInit(void)
{
    /******* SEMAPHORE INITIALIZATION *********/

    OSSemCreate(&smbusRxSem, "smBusRCVSem", (OS_SEM_CTR)0u, &smbus_p_err );

    if (smbus_p_err != OS_ERR_NONE )
    {
        //setError(E_ERROR_SMBUS_DRIVER);
    }

    OSSemCreate(&smbusTxSem, "smBusSendSem", (OS_SEM_CTR)0u, &smbus_p_err);

    if (smbus_p_err != OS_ERR_NONE )
    {
        //setError(E_ERROR_SMBUS_DRIVER);
    }

    OSMutexCreate(&smbusMutex, "smBusMutex", &smbus_p_err );

    if (smbus_p_err != OS_ERR_NONE )
    {
        //setError(E_ERROR_I2C_DRIVER);
    }
    if (HAL_SMBUS_GetState(&hsmbus1) == HAL_SMBUS_STATE_RESET)
    {
        HAL_SMBUS_Init(&hsmbus1);
    }
    
}


/**
 * @brief   I2C Memory write Interrupt Callback
 * @param   hi2c I2C Handler
 */

void HAL_SMBUS_MasterTxCpltCallback(SMBUS_HandleTypeDef  *psmbus)
{
    

    if (psmbus->Instance == I2C1)
    {
        /**** Post Tx Semaphore to finish transaction *****/
        OSSemPost(&smbusTxSem, OS_OPT_POST_1, &smbus_p_err);
    }    
    else
    {
        //setError(E_ERROR_I2C_DRIVER);
    }

    
}


/**
 * @brief	 I2C Memory Read Interrupt Callback
 * @param	 hi2c I2C Handler
 */

void HAL_SMBUS_MasterRxCpltCallback(SMBUS_HandleTypeDef * psmbus)
{
   

    if (psmbus->Instance == I2C1)
    {
        /**** Post Rx Semaphore to finish transaction *****/
        OSSemPost(&smbusRxSem, OS_OPT_POST_1, &smbus_p_err);
    }
    else
    {
        //setError(E_ERROR_I2C_DRIVER);
    }

    
}


/**
 * @brief   I2C1 test mode it initialize the I2C1 SCL and SDA pins as GPIO with output open drain,no pull configuration
 */

void smbusTestMode(void)
{
    
}


/**
 * @brief   I2C Deinitialization Function
 * @param   elem
 */

static void smbusDeinit(void)
{
    if (HAL_SMBUS_GetState(&hsmbus1) != HAL_SMBUS_STATE_RESET)
    {
        /* Deinitialize the SMBUS */
        HAL_SMBUS_DeInit(&hsmbus1);
    }    
    else
    {
        /* Do Nothing*/
    }
}


/**
 * @brief   I2C Error Handler to re-initialise the I2C Handlers
 * @param   elem I2C Instance
 */

static void smbusReinitialise(void)
{
    OS_ERR os_error;
    //De-initialize and re-initialise the I2C bus
    HAL_SMBUS_DeInit(&hsmbus1);
    HAL_SMBUS_Init(&hsmbus1);
    
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

HAL_StatusTypeDef SMBUS_Transmit( uint16_t deviceAddr, uint8_t *pBuffer, uint16_t Length, uint32_t xferOptions)
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t attempts = 0u;

    OSMutexPend(&smbusMutex,0u, OS_OPT_PEND_BLOCKING,(CPU_TS *)0, &smbus_p_err);

    while ((status != HAL_OK) && (attempts < MAX_ATTEMPTS))
    {
        //if this is not the first attempt then re-initialise the bus
        if (attempts > 0u)
        {
            smbusReinitialise();
        }

        //check device ready
        status = HAL_SMBUS_IsDeviceReady(&hsmbus1, deviceAddr, 5u, 5u); 

        if (status == HAL_OK)
        {
            // Write to smbus device via Interrupt
            status = HAL_SMBUS_Master_Transmit_IT(&hsmbus1, deviceAddr, pBuffer, Length, xferOptions);

            if (status == HAL_OK)
            {
                //Pending on smbusTxSem for 500ms to complete transaction
                OSSemPend(&smbusTxSem, 500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &smbus_p_err);
            }
        }

        attempts++;
    }

    OSMutexPost(&smbusMutex, OS_OPT_POST_NO_SCHED, &smbus_p_err);

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

HAL_StatusTypeDef SMBUS_Receive( uint16_t deviceAddr, uint8_t *pBuffer, uint16_t Length, uint32_t xferOptions)
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t attempts = 0u;

    OSMutexPend(&smbusMutex, 0u, OS_OPT_PEND_BLOCKING,(CPU_TS *)0, &smbus_p_err);

    while ((status != HAL_OK) && (attempts < MAX_ATTEMPTS))
    {
        //if this is not the first attempt then re-initialise the bus
        if (attempts > 0u)
        {
            smbusReinitialise();
        }

        //check device ready
        status = HAL_SMBUS_IsDeviceReady(&hsmbus1, deviceAddr, 5u, 5u); 

        if (status == HAL_OK)
        {
            //smbus Interrupt READ
            status = HAL_SMBUS_Master_Receive_IT(&hsmbus1, deviceAddr, pBuffer, Length, xferOptions);

            if (status == HAL_OK)
            {
                // Pending on smbusRxSem for 500ms to complete transaction
                OSSemPend(&smbusRxSem, 500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &smbus_p_err);
            }
        }

        attempts++;
    }

    OSMutexPost(&smbusMutex, OS_OPT_POST_NO_SCHED, &smbus_p_err);

    return status;
}


/**
 * @brief I2C1 Event Interrupt Request Handler
 */

void I2C1_EV_IRQHandler(void)
{
    OSIntEnter();
    HAL_SMBUS_EV_IRQHandler( &hsmbus1);
    OSIntExit();
}


/**
 * @brief   I2C1 Error Request Handler
 */

void I2C1_ER_IRQHandler(void)
{
    OSIntEnter();
    HAL_SMBUS_ER_IRQHandler( &hsmbus1 );
    OSIntExit();
}






/************************************************** END OF FUNCTIONS ***************************************************/



















