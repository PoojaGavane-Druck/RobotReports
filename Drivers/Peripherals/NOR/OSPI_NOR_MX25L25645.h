/*!
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the
* property of Baker Hughes and its suppliers, and affiliates if any.
* The intellectual and technical concepts contained herein are
* proprietary to Baker Hughes and its suppliers and affiliates
* and may be covered by U.S. and Foreign Patents, patents in
* process, and are protected by trade secret or copyright law.
* Dissemination of this information or reproduction of this
* material is strictly forbidden unless prior written permission
* is obtained from Baker Hughes.
*
* @file     OSPI_NOR_MX25L25645.h
*
* @version  0.1
*
* @author   Sean Parkinson & Simon Smith
* @date     19th August 2020
*
* Header for OctoSPI/Serial Multi-IO NOR Flash access routines - targeted at
* compatibility with Macronix MX25L25645 devices.
*
*/

#ifndef __OSPI_NOR_MX25L25645_H
#define __OSPI_NOR_MX25L25645_H

#include "misra.h"
MISRAC_DISABLE
#include <stdbool.h>
#include "stm32l4xx_hal.h"
MISRAC_ENABLE

#ifdef __cplusplus
extern "C"
{
/* External C language linkage */
#endif

// Definitions - NOR flash commands
#if 0
#define PAGE_PROG_CMD                   0x02u
#define READ_CMD                        0x03u
#define WRITE_DISABLE_CMD               0x04u
#define WRITE_4PAGE_CMD                 0x12u
#define BLOCK_32K_ERASE_CMD             0x52u
#define WRITE_CFG_REG_2_CMD             0x72u
#define ENABLE_4BYTE_MODE_CMD           0xB7u
#define WRITE_EXT_ADDR_REG_CMD          0xC5u
#define CHIP_ERASE_CMD                  0xC7u
#define BLOCK_64K_ERASE_CMD             0xD8u
#define IO_READ_CMD                     0xECu
#define EXIT_QUAD_MODE                  0xF5u
#endif
#define WRITE_STATUS_REG_CMD            0x01u
#define READ_STATUS_REG_CMD             0x05u
#define WRITE_ENABLE_CMD                0x06u
#define READ_CONFIG_REG_CMD             0x15u
#define SECTOR_4K_ERASE_CMD             0x20u
#define READ_SECURITY_REG_CMD           0x2Bu
#define ENTER_QUAD_MODE_CMD             0x35u
#define QUAD_PAGE_WRITE_CMD             0x38u
#define RESET_ENABLE_CMD                0x66u
#define RESET_CMD                       0x99u
#define READ_MEM_TYPE_ID_CMD            0x9Fu
#define FAST_READ_QUAD_CMD              0xEBu

#define FAST_READ_QUAD_DUMMY_CYCLES     6u
#if 0
#define WRITE_QUAD_DUMMY_CYCLES         0u
#endif
#define ENTER_QUAD_DUMMY_CYCLES         0u

/* Size of the flash */
#if 0
#define OSPI_FLASH_SIZE                 26
#define OSPI_PAGE_SIZE                  256
#define OSPI_END_ADDR                   (1 << OSPI_FLASH_SIZE)
#endif

/* Memory delay */
#if 0
#define MEMORY_REG_WRITE_DELAY          40
#endif
#define MEMORY_PAGE_PROG_DELAY          2u

/* Dummy clocks cycles */
#if 0
#define DUMMY_CLOCK_CYCLES_READ         6u
#endif
#define DUMMY_CLOCK_CYCLES_READ_REG     0u

/* Auto-polling values */
#define MEMORY_READY_MATCH_VALUE        0x00u
#define MEMORY_READY_MASK_VALUE         0x01u
#define WRITE_ENABLE_MATCH_VALUE        0x02u
#define WRITE_ENABLE_MASK_VALUE         0x02u

#if 0
#define WRITE_DISABLE_MATCH_VALUE       0x00
#define WRITE_DISABLE_MASK_VALUE        0x02
#define QUAD_ENABLE_MATCH_VALUE         0x40u
#define QUAD_ENABLE_MASK_VALUE          0x40u
#define AUTO_POLLING_INTERVAL           0x10u
#endif

// Global type definitions
typedef enum eOSPINORStatus { OSPI_NOR_FAIL, OSPI_NOR_SUCCESS } tOSPINORStatus;
extern OSPI_HandleTypeDef hospi1;

// Global function prototypes
tOSPINORStatus OSPI_NOR_Init(void);
tOSPINORStatus OSPI_NOR_Read(uint32_t blk_addr, uint8_t *buf, uint32_t size);
tOSPINORStatus OSPI_NOR_EraseWrite(uint32_t blk_addr, uint8_t *buf, uint32_t size);
tOSPINORStatus OSPI_NOR_Erase(uint32_t blk_addr, uint32_t size);
tOSPINORStatus OSPI_NOR_Write(uint32_t blk_addr, uint8_t *buf, uint32_t size);
tOSPINORStatus OSPI_NOR_PageWrite(uint32_t blk_addr, uint8_t *buf);
tOSPINORStatus OSPI_NOR_waitStatus( uint8_t pStatusBits, uint32_t pTimeout );
uint32_t OSPI_NOR_GetFirstAddr(uint32_t addr, uint32_t inc);

// Device Manufacturer and Type ID Command
tOSPINORStatus OSPI_NOR_ReadManufDeviceID(uint8_t *manufID, uint8_t *memTypeID, uint8_t *memDensityID);

// Status and Configuration Commands
tOSPINORStatus OSPI_NOR_WriteStatusCfgReg(uint8_t *statData, uint8_t *cfgData);
#if 0
tOSPINORStatus OSPI_NOR_WriteStatusCfgRegQuad(uint8_t *statData, uint8_t *cfgData);
#endif
tOSPINORStatus OSPI_NOR_ReadStatusReg(uint8_t *pData);
tOSPINORStatus OSPI_NOR_ReadCfgReg(uint8_t *pData);
tOSPINORStatus OSPI_NOR_AutoPollingMemReady(OSPI_HandleTypeDef *hospi);
tOSPINORStatus OSPI_NOR_ReadSecurityReg(uint8_t *pData);

// Reset Commands
tOSPINORStatus OSPI_NOR_Reset(void);
tOSPINORStatus OSPI_NOR_Reset_Enable(void);

// NOR Flash Erase Routines
tOSPINORStatus OSPI_NOR_EraseSector4K(uint32_t address);
#if 0
tOSPINORStatus OSPI_NOR_BlockErase32K(uint32_t address);
tOSPINORStatus OSPI_NOR_BlockErase64K(uint32_t address);
tOSPINORStatus OSPI_NOR_ChipErase(void);
#endif

// NOR Flash Array Read / Write Routines
// SINGLE SPI MODE
tOSPINORStatus OSPI_NOR_SPIMode(void);
#if 0
tOSPINORStatus OSPI_NOR_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
#endif
tOSPINORStatus OSPI_NOR_WriteEnable(OSPI_HandleTypeDef *hospi);


// NOR Flash Array Read / Write Routines
// QUAD-SPI MODE
#if 0
tOSPINORStatus OSPI_NOR_Enable4ByteMode(void);
tOSPINORStatus OSPI_NOR_QSPIMode_AutoPolling(void);
tOSPINORStatus OSPI_NOR_WriteEnableQuad(OSPI_HandleTypeDef *hospi);
#endif
tOSPINORStatus OSPI_NOR_QSPIMode(void);
tOSPINORStatus OSPI_NOR_ReadQPIQuad(uint32_t ReadAddr, uint8_t* pData, uint32_t Size);
tOSPINORStatus OSPI_NOR_WriteSPIPageQuad(uint32_t address, uint8_t *buffer, uint32_t buffersize);

// Memory mapped operation Routines
#if 0
tOSPINORStatus OSPI_NOR_EnableMemMappedQuadMode(void);
tOSPINORStatus OSPI_NOR_DisableMemMappedQuadMode(void);
#endif

#if 0
tOSPINORStatus OSPI_NOR_WriteExtendedAddressReg(uint8_t data);
#endif

// Example and test Function Prototypes
#if 0
tOSPINORStatus OSPI_NOR_EraseWriteQPIExample(uint32_t address, __IO uint8_t *step);
#endif
tOSPINORStatus OSPI_NOR_SelfTest(void);

#ifdef __cplusplus
}                                                               /* End of external C language linkage */
#endif

#endif