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
#include <bsp_os.h>
#include <bsp_int.h>
#include <os_app_hooks.h>
MISRAC_ENABLE

#include "i2c.h"
#include "main.h"



/***************************************** SEMAPHORES MUTEXES AND STATUS FLAG VOLATILES ****************************************************/

static OS_ERR i2c_p_err[I2CnSIZE];;

static OS_MUTEX i2cMutex[I2CnSIZE];
static OS_SEM i2cRxSem[I2CnSIZE];
static OS_SEM i2cTxSem[I2CnSIZE];

#define MAX_I2CTRIALS 2u
static const uint32_t  MAX_ATTEMPTS_GET_READY = 10u;
static const uint32_t cMaxAttempts = 3u;


/************************************* GLOBALS ************************************************************************/

static I2C_HandleTypeDef *I2cHandle[I2CnSIZE];
//const uint32_t I2C1_TIMING = 0x20303E5Du;
//const uint32_t I2C4_TIMING = 0x20303E5Du;

/************************************************* PROTOTYPE **********************************************************/
void I2C2_EV_IRQHandler(void);
void I2C2_ER_IRQHandler(void);
void I2C3_EV_IRQHandler(void);
void I2C3_ER_IRQHandler(void);
void I2C4_EV_IRQHandler(void);
void I2C4_ER_IRQHandler(void);
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);

//static void i2cDeinit(eI2CElement_t elem);
//static void i2cReinitialise(eI2CElement_t elem);
static eI2CElement_t i2c_getElement( I2C_HandleTypeDef *hi2c );





/************************************************* FUNCTIONS **********************************************************/

/**
 * @brief   Initializes the I2C Handler and creates semaphore for transmission and receipt
 * @param   elem The I2C instance
 */

void i2cInit(I2C_HandleTypeDef *hi2c )
{
  
    eI2CElement_t elem = i2c_getElement( hi2c );
    
    if( elem == I2CnNone )
    {
        //setError(E_ERROR_I2C_DRIVER);
    }
    else
    {
        I2cHandle[elem] = hi2c;
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

        
       

    }
}


/**
 * @brief   I2C Memory write Interrupt Callback
 * @param   hi2c I2C Handler
 */

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    eI2CElement_t idx = I2CnNone;

    idx = i2c_getElement(hi2c);
    if (idx != I2CnNone)
    {
        /**** Post Tx Semaphore to finish transaction *****/
        OSSemPost(&i2cTxSem[idx], OS_OPT_POST_1, &i2c_p_err[idx] );
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

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    eI2CElement_t idx = I2CnNone;

    idx = i2c_getElement(hi2c);
    if (idx != I2CnNone)
    {
        /**** Post Rx Semaphore to finish transaction *****/
        OSSemPost(&i2cRxSem[idx], OS_OPT_POST_1, &i2c_p_err[idx]  );
    }
    else
    {
      //setError(E_ERROR_I2C_DRIVER);
    }
}


/**
 * @brief   I2C1 test mode it initialize the I2C1 SCL and SDA pins as GPIO with output open drain,no pull configuration
 */

void i2c1TestMode(void)
{
    
}

#if 0
/**
 * @brief   I2C Deinitialization Function
 * @param   elem
 */

static void i2cDeinit(eI2CElement_t elem)
{
    if ((elem == I2Cn1) || (elem == I2Cn2) || (elem == I2Cn3) || (elem == I2Cn4))
    {
        if (HAL_I2C_GetState(I2cHandle[elem]) !=  HAL_I2C_STATE_RESET )
        {
            /* Deinitialize the I2C */
            HAL_I2C_DeInit(I2cHandle[elem]);
            //I2C1_MspDeInit(&I2cHandle[elem]);
        }
    }
    else
    {
        //setError(E_ERROR_I2C_DRIVER);
    }
}
#endif


/**
 * @brief   I2C Error Handler to re-initialise the I2C Handlers
 * @param   elem I2C Instance
 */

static void i2c_reInit(eI2CElement_t elem)
{
    OS_ERR os_error;
    //De-initialize and re-initialise the I2C bus
    HAL_I2C_DeInit(I2cHandle[elem]);
    HAL_I2C_Init(I2cHandle[elem]);
    
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

HAL_StatusTypeDef I2C_WriteBuffer(eI2CElement_t elem, uint16_t Addr, uint16_t devMemLocation, uint16_t devMemSize, uint8_t *pBuffer, uint16_t Length, uint32_t pBlocking)
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t attempts = 0u;

    OSMutexPend(&i2cMutex[elem],0u, OS_OPT_PEND_BLOCKING,(CPU_TS *)0, &i2c_p_err[elem]);

    while ((status != HAL_OK) && (attempts < cMaxAttempts))
    {
        //if this is not the first attempt then re-initialise the bus
        if (attempts > 0u)
        {
            i2c_reInit(elem);
        }

        //check device ready
        status = HAL_I2C_IsDeviceReady(I2cHandle[elem], Addr, 5u, 5u); //isDeviceReady(elem, Addr);

        if (status == HAL_OK)
        {
            // Write to I2C memory via Interrupt
            if( pBlocking )
            {
                //I2C READ
                status = HAL_I2C_Mem_Write(I2cHandle[elem], Addr, (uint16_t)devMemLocation, devMemSize, pBuffer, Length, 100u );
            }
            else
            {
                status = HAL_I2C_Mem_Write_IT(I2cHandle[elem], Addr, (uint16_t)devMemLocation, devMemSize, pBuffer, Length);
            }
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

HAL_StatusTypeDef I2C_ReadBuffer(eI2CElement_t elem, uint16_t Addr, uint16_t devMemLocation, uint16_t devMemSize, uint8_t *pBuffer, uint16_t Length, uint32_t pBlocking)
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t attempts = 0u;

    OSMutexPend(&i2cMutex[elem], 0u, OS_OPT_PEND_BLOCKING,(CPU_TS *)0, &i2c_p_err[elem]);

    while ((status != HAL_OK) && (attempts < cMaxAttempts))
    {
        //if this is not the first attempt then re-initialise the bus
        if (attempts > 0u)
        {
            i2c_reInit(elem);
        }

        //check device ready
        status = HAL_I2C_IsDeviceReady(I2cHandle[elem], Addr, 5u, 5u); //isDeviceReady(elem, Addr);

        if (status == HAL_OK)
        {
            if( pBlocking )
            {
                //I2C READ
                status = HAL_I2C_Mem_Read(I2cHandle[elem], Addr, (uint16_t)devMemLocation, devMemSize, pBuffer, Length, 100u );
            }
            else
            {
                //I2C Interrupt READ
                status = HAL_I2C_Mem_Read_IT(I2cHandle[elem], Addr, (uint16_t)devMemLocation, devMemSize, pBuffer, Length);
            }

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

static eI2CElement_t i2c_getElement( I2C_HandleTypeDef *hi2c )
{
    eI2CElement_t elem = I2CnNone;

    // This was ex 705 code so disabled misra - MISRA 2004 Rule 11.3
    MISRAC_DISABLE
    if ( hi2c->Instance == I2C1 )
    {
        elem = I2Cn1;
    }
    else if ( hi2c->Instance == I2C2 )
    {
        elem = I2Cn2;
    }
    else if ( hi2c->Instance == I2C3 )
    {
        elem = I2Cn3;
    }
    else if ( hi2c->Instance == I2C4 )
    {
        elem = I2Cn4;
    }
    else
    {
        elem = I2CnNone;
    }
    MISRAC_ENABLE

    return( elem );
}

/**
 * @brief I2C1 Event Interrupt Request Handler
 */

void I2C2_EV_IRQHandler(void)
{

  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  
  HAL_I2C_EV_IRQHandler( I2cHandle[I2Cn2] );
    
  OSIntExit();
}


/**
 * @brief   I2C1 Error Request Handler
 */

void I2C2_ER_IRQHandler(void)
{

  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  
  HAL_I2C_ER_IRQHandler( I2cHandle[I2Cn2] );
  
  OSIntExit();
   
}


/**
 * @brief I2C1 Event Interrupt Request Handler
 */

void I2C3_EV_IRQHandler(void)
{
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  
  HAL_I2C_EV_IRQHandler( I2cHandle[I2Cn3] );
  
  OSIntExit();
    
}


/**
 * @brief   I2C1 Error Request Handler
 */

void I2C3_ER_IRQHandler(void)
{
 
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
    
  HAL_I2C_ER_IRQHandler( I2cHandle[I2Cn3] );
   
  OSIntExit();
}
/**
 * @brief   I2C2 Event Interrupt Request Handler
 */

void I2C4_EV_IRQHandler(void)
{
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
    
  HAL_I2C_EV_IRQHandler(I2cHandle[I2Cn4]);
    
  OSIntExit(); 
}


/**
 * @brief   I2C2 Error Request Handler
 */

void I2C4_ER_IRQHandler(void)
{
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  
  HAL_I2C_ER_IRQHandler(I2cHandle[I2Cn4]);
    
  OSIntExit(); 
    
}

/**
 * @brief   SMBUS read data with CRC
 */
HAL_StatusTypeDef SMBUS_I2C_ReadBuffer(eI2CElement_t elem, uint8_t addr, uint8_t cmdCode, uint8_t *value, uint8_t len)
{
    //OS_ERR os_error = OS_ERR_NONE;
    //eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
    HAL_StatusTypeDef halStatus = HAL_ERROR;
    uint8_t txBuf = cmdCode;
    uint8_t rxBuf[3];
    uint32_t attempts = (uint32_t)0;

    
    rxBuf[0] = 0u;
    rxBuf[1] = 0u;
    rxBuf[2] = 0u;
    OSMutexPend(&i2cMutex[elem], 0u, OS_OPT_PEND_BLOCKING,(CPU_TS *)0, &i2c_p_err[elem]);
    halStatus = (HAL_StatusTypeDef)(HAL_I2C_GetState(I2cHandle[elem]));
     while ((halStatus != (HAL_StatusTypeDef)(HAL_I2C_STATE_READY)) && (attempts < MAX_ATTEMPTS_GET_READY))
     {
       attempts++;
       halStatus = (HAL_StatusTypeDef)(HAL_I2C_GetState(I2cHandle[elem]));

     }
    //!= (HAL_I2C_StateTypeDef)HAL_I2C_STATE_READY)

    if (halStatus ==  (HAL_StatusTypeDef)(HAL_I2C_STATE_READY))
    {
    halStatus = HAL_I2C_Master_Transmit_IT(I2cHandle[elem], (uint16_t)addr, &txBuf, (uint16_t)1);
    if (halStatus == HAL_OK)
    {
    
      OSSemPend(&i2cTxSem[elem], 500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &i2c_p_err[elem]);
      if (i2c_p_err[elem] == OS_ERR_NONE)
      {

        halStatus = HAL_I2C_Master_Receive_IT(I2cHandle[elem], (uint16_t)addr, (uint8_t*)&rxBuf[0], (uint16_t)3u);
        if(halStatus == HAL_OK)
        {
          //while(RxFlag == (uint32_t)(0));  
          OSSemPend(&i2cRxSem[elem], 500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &i2c_p_err[elem]);
          if (i2c_p_err[elem] == OS_ERR_NONE)
          {
            *value = rxBuf[0];
            *(value+1u) = rxBuf[1];
            *(value+2u) = rxBuf[2]; 
          }
          else
          {
            /* Do Nothing. Added for Misra*/
          }

                    
        }
        else
        {
          /* Do Nothing. Added for Misra*/
        } 
      }
    }
    
    else
    {
      /* Do Nothing. Added for Misra*/
    }
    }
    OSMutexPost(&i2cMutex[elem], OS_OPT_POST_NO_SCHED, &i2c_p_err[elem]);
    return halStatus;
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    eI2CElement_t idx = I2CnNone;

    idx = i2c_getElement(hi2c);
    if (idx != I2CnNone)
    {
        /**** Post Rx Semaphore to finish transaction *****/
        OSSemPost(&i2cTxSem[idx], OS_OPT_POST_1, &i2c_p_err[idx]  );
    }
    else
    {
      //setError(E_ERROR_I2C_DRIVER);
    }
    
}
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    eI2CElement_t idx = I2CnNone;
    idx = i2c_getElement(hi2c);
    if (idx != I2CnNone)
    {
        /**** Post Rx Semaphore to finish transaction *****/
        OSSemPost(&i2cRxSem[idx], OS_OPT_POST_1, &i2c_p_err[idx]  );
    }
    else
    {
      //setError(E_ERROR_I2C_DRIVER);
    }

}
/************************************************** END OF FUNCTIONS ***************************************************/



















