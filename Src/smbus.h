/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     smbus.h
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     25 May 2021
*
* @brief    SMBUS Header file
*/

#ifndef __SMBUS_H
#define __SMBUS_H

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

/* Defines and constants  ---------------------------------------------------*/
#define BUFF_LENGTH 10

/* SMBUS FLAGS */
#define SMBUS_FLAG_TX_COMPLETE 0x00000001
#define SMBUS_FLAG_RX_COMPLETE 0x00000002
#define SMBUS_FLAG_ERROR 0x00000004

/* Types --------------------------------------------------------------------*/
typedef enum
{
    esmbusErrorNone = 0,
    esmbusErrorTimeout
}smBusError_t;

/* Variables ----------------------------------------------------------------*/
class SMBUS
{
public:
    SMBUS(SMBUS_HandleTypeDef *smbusInstance);
    ~SMBUS();
  
    smBusError_t smBusWriteWord(uint8_t address, 
                        uint8_t *commandCode, 
                        uint16_t *data);

    smBusError_t smbusWriteByte(uint8_t address, 
                        uint8_t *commandCode, 
                        uint8_t *data);

    smBusError_t smBusReadString(uint8_t address, 
                        uint8_t *commandCode, 
                        uint8_t *str,
                        uint32_t length);

    smBusError_t smBusReadWord(uint8_t address, 
                        uint8_t *commandCode, 
                        uint16_t *data);

    smBusError_t smbusReadByte(uint8_t address, 
                        uint8_t *commandCode, 
                        uint8_t *data);

    smBusError_t smbusRead(uint8_t address, 
                        uint8_t *commandCode, 
                        uint8_t *data,
                        uint32_t length);

    smBusError_t smbusWrite(uint8_t address, 
                        uint8_t *commandCode, 
                        uint8_t *data,
                        uint32_t length);                      

    smBusError_t smbusEnableAlert();

private:
    smBusError_t smbusWaitTransmit(uint32_t timeout);
    smBusError_t smbusWaitReceive(uint32_t timeout);
    
    SMBUS_HandleTypeDef *smbus;
    uint32_t smbusTimeout;
    
    uint8_t txBuffer[BUFF_LENGTH];
    uint8_t rxBuffer[BUFF_LENGTH];
protected:

};
#endif /*__SMBUS_H */