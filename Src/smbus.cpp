/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     SMBUS.cpp
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     25 May 2021
*
* @brief    SMBUS Source FIle
*/
//*********************************************************************************************************************

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <rtos.h>
#include <stdbool.h>
#include <stm32l4xx_hal.h>
#include <rtos.h>
MISRAC_ENABLE

#include "smbus.h"

/* Typedefs -----------------------------------------------------------------*/

/* Defines ------------------------------------------------------------------*/

/* Macros -------------------------------------------------------------------*/

/* Variables ----------------------------------------------------------------*/
OS_SEM smbusSemTx;
OS_SEM smbusSemRx;
OS_ERR pSmbusError;

OS_FLAG_GRP smbusFlagGroup;
OS_FLAGS smbusFlags;

OS_FLAGS smbusPendFlags;
    
/* Prototypes ---------------------------------------------------------------*/

/* User code ----------------------------------------------------------------*/
/*
 * @brief   SMBUS Contructor
 * @param   smBus reference
 * @retval  void
 */
SMBUS::SMBUS(SMBUS_HandleTypeDef *smbusInstance)
{
    smbus = smbusInstance;
    
    /* Set SM BUS timeout to 200 ms */
    smbusTimeout = 200u;
    smbusFlags = (OS_FLAGS)(0);
    smbusPendFlags = (OS_FLAGS)(0);
    
    /* Create flags for interrupts */
    RTOSFlagCreate(&smbusFlagGroup, 
                   (char *)("smbusFlags"), 
                   smbusFlags, 
                   &pSmbusError);
 
}

/*
 * @brief   SMBUS Destructor
 * @param   smBus reference
 * @retval  void
 */
SMBUS::~SMBUS()
{
    
}

/*
 * @brief   Writes a word to an SMBUS device
 * @param   smBus reference, address, command code, data, timeout
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smBusWriteWord(uint8_t address, 
                                    uint8_t *commandCode, 
                                    uint16_t *data)
{
    smBusError_t error = esmbusErrorTimeout;


    error = smbusWrite(address, commandCode, (uint8_t*)(data), (uint16_t)(3));
    error = smbusWaitTransmit(smbusTimeout);

    return error;
}                     

/*
 * @brief   Writes a byte to an SMBUS device
 * @param   smBus reference, address, command code, data, timeout
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smbusWriteByte(uint8_t address, 
                                    uint8_t *commandCode, 
                                    uint8_t *data)
{
    smBusError_t error = esmbusErrorTimeout;
    uint32_t length = 2u;

    error = smbusWrite(address, commandCode, (uint8_t*)(data), length);
    error = smbusWaitTransmit(smbusTimeout);

    return error;    
}             

/*
 * @brief   Reads a string of given length from an SMBUS device
 * @param   smBus reference, address, command code, data, timeout
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smBusReadString(uint8_t address, 
                                    uint8_t *commandCode, 
                                    uint8_t *str,
                                    uint32_t length)
{
    smBusError_t error = esmbusErrorTimeout;
    length++;

    error = smbusRead(address, commandCode, str, length);

    return error;     
}                     

/*
 * @brief   Reads a word from an SMBUS device
 * @param   smBus reference, address, command code, data, timeout
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smBusReadWord(uint8_t address, 
                                    uint8_t *commandCode, 
                                    uint16_t *data)
{
    smBusError_t error = esmbusErrorTimeout;

    error = smbusRead(address, commandCode, (uint8_t*)(data), (uint16_t)(2));

    return error;      
}                     

/*
 * @brief   Reads a byte from an SMBUS device
 * @param   smBus reference, address, command code, data, timeout
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smbusReadByte(uint8_t address, 
                                    uint8_t *commandCode, 
                                    uint8_t *data)
{
    smBusError_t error = esmbusErrorTimeout;

    error = smbusRead(address, commandCode, (uint8_t*)(data), (uint16_t)(1));

    return error;    
}                     

/*
 * @brief   SMBUS read operation
 * @param   smBus reference, address, command code, data, length, timeout
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smbusRead(uint8_t address, 
                                uint8_t *commandCode, 
                                uint8_t *data,
                                uint32_t length)
{
    smBusError_t error = esmbusErrorTimeout;

    HAL_SMBUS_Master_Transmit_IT(smbus, address, commandCode, (uint16_t)(1), (uint32_t)(SMBUS_FIRST_FRAME));
    error = smbusWaitTransmit(smbusTimeout);
    
    if(error == esmbusErrorNone)
    {
        HAL_SMBUS_Master_Receive_IT(smbus, address, data, length, (uint32_t)(SMBUS_LAST_FRAME_NO_PEC));
        error = smbusWaitReceive(smbusTimeout);
    }
    else
    {

    }
    
    return error;
}                     

/*
 * @brief   SMBUS write operation
 * @param   smBus reference, address, command code, data, length, timeout
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smbusWrite(uint8_t address, 
                                uint8_t *commandCode, 
                                uint8_t *data,
                                uint32_t length)
{
    smBusError_t error = esmbusErrorTimeout;
    uint32_t counter = 0u;

    txBuffer[0] = *commandCode;
    
    for(counter = 1u; counter < (length); counter++)
    {
        txBuffer[counter] = data[counter - 1u];
    }
    
    HAL_SMBUS_Master_Transmit_IT(smbus, 
                                   0x12u, 
                                   (uint8_t*)(txBuffer), 
                                   3u, 
                                   (uint32_t)(SMBUS_FIRST_AND_LAST_FRAME_NO_PEC));

    error = esmbusErrorNone;
    return error;
}                  

/*
 * @brief   SMBUS enable alert pin function
 * @param   smBus reference
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smbusEnableAlert()
{
    smBusError_t error = esmbusErrorTimeout;
    
    HAL_SMBUS_EnableAlert_IT(smbus);
    
    error = esmbusErrorNone;
    return error;    
} 

/*
 * @brief   Wait for an SMBUS transmit interrupt
 * @param   smBus reference
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smbusWaitTransmit(uint32_t timeout)
{
    smBusError_t error = esmbusErrorTimeout;
    
    //OS_FLAGS flags = (OS_FLAGS)(0);

    /* Pend a event here in OS implementation */
   

    RTOSFlagPend(&smbusFlagGroup, 
               SMBUS_FLAG_TX_COMPLETE | SMBUS_FLAG_ERROR, 
               timeout, 
               OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME | OS_OPT_PEND_BLOCKING,
               (CPU_TS *)(0),
               &pSmbusError);
    
    //flags = OSFlagPendGetFlagsRdy(&pSmbusError);
    
    if(pSmbusError == OS_ERR_NONE)
    {
        error = esmbusErrorNone;
    }

    return error;
}

/*
 * @brief   Wait for smbusReceive interrupt
 * @param   smBus reference
 * @retval  smbusError_t
 */
smBusError_t SMBUS::smbusWaitReceive(uint32_t timeout)
{
    smBusError_t error = esmbusErrorTimeout;

    //OS_FLAGS flags = (OS_FLAGS)(0);
    /* Pend for an event here in OS implementation */
   
    RTOSFlagPend(&smbusFlagGroup, 
               SMBUS_FLAG_RX_COMPLETE | SMBUS_FLAG_ERROR, 
               timeout, 
               OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME | OS_OPT_PEND_BLOCKING,
               (CPU_TS *)(0),
               &pSmbusError);
        
    //flags = OSFlagPendGetFlagsRdy(&pSmbusError);
    
    if(pSmbusError == OS_ERR_NONE)
    {
        error = esmbusErrorNone;
    }

    return error;
}

/**
  * @brief  Master Tx Transfer completed callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
void HAL_SMBUS_MasterTxCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
    if(hsmbus->Instance == I2C1)
    {
       smbusPendFlags = RTOSFlagPost(&smbusFlagGroup,
                                          (OS_FLAGS)(SMBUS_FLAG_TX_COMPLETE),
                                          OS_OPT_POST_FLAG_SET,
                                          &pSmbusError);
                    
    }
}

/**
  * @brief  Master Rx Transfer completed callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
void HAL_SMBUS_MasterRxCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
    if(hsmbus->Instance == I2C1)
    {
        smbusPendFlags = RTOSFlagPost(&smbusFlagGroup,
                                          (OS_FLAGS)(SMBUS_FLAG_RX_COMPLETE),
                                          OS_OPT_POST_FLAG_SET,
                                          &pSmbusError);
    }
}