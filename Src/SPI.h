/**
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
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
* @brief	SPI module header file
*/

#ifndef __SPI_H__
#define __SPI_H__

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

/* Defines and constants ----------------------------------------------------*/
/* SMBUS FLAGS */
#define SPI_FLAG_TX_COMPLETE 0x00000001
#define SPI_FLAG_RX_COMPLETE 0x00000002
#define SPI_FLAG_ERROR 0x00000004
#define SPI_FLAG_DATA_READY 0x00000008

#define BUFFER_SIZE 10

/* Types --------------------------------------------------------------------*/

/* Variables ----------------------------------------------------------------*/

class spi
{

private:
	uint8_t dummyTx[BUFFER_SIZE];
	uint8_t dummyRx[BUFFER_SIZE];
        uint32_t spiTimeout;
	uint32_t resetDummyBuffers(void);

protected:

public:
	spi(SPI_HandleTypeDef *spiInstance);
        SPI_HandleTypeDef *spiHandle;
	~spi();
	
	uint32_t transmit(uint8_t *data, uint8_t length);
	uint32_t receive(uint8_t *data, uint8_t length);
	uint32_t getDataReady(void);
};

#endif /* spi.h*/
