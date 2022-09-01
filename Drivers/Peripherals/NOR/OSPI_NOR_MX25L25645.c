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
* @file     OSPI_NOR_MX25L25645.c
*
* @version  0.1
*
* @author   Sean Parkinson & Simon Smith
* @date     19th August 2020
*
* OctoSPI/Serial Multi-IO NOR Flash access routines - targeted at
* compatibility with Macronix MX25L25645 devices.
*
*/

/* Project header files */
#include "misra.h"
#include "ospi_nor_mx25l25645.h"
#include "Utilities.h"
MISRAC_DISABLE
#include <string.h>
#include <assert.h>
#include "user_diskio.h"
MISRAC_ENABLE

#define DEF_WAIT_STATUS_WIP                     (( uint8_t )0x01u )
#define DEF_WAIT_STATUS_WEL                     (( uint8_t )0x02u )
#define DEF_WAIT_STATUS_WIP_WEL                 (( uint8_t )0x03u )
#define DEF_WAIT_STATUS_WR_STATUS_TIMEOUT       (( uint32_t )50u  )

uint8_t bufReadback[STORAGE_BLK_SIZ];
__IO uint8_t CmdCplt;
extern volatile efsOwnership_t fsOwnership;

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 14.7 as functions have multiple points of exit violates the rule.
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm073")
_Pragma("diag_suppress=Pm143")

/*!
* @brief    Read Electronic Manufacturer and Device ID (RDID)
*
* @param    manufID             Pointer to returned manufacturer ID data.
* @param    memTypeID           Pointer to returned memory type data.
* @param    memDensityID        Pointer to returned memory density data.
*
* @return   tOSPINORStatus Status of the read operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_ReadManufDeviceID(uint8_t* const manufID, uint8_t* const memTypeID, uint8_t* const memDensityID)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    uint8_t pData[3];                                             // Buffer to hold data to send

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 3u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = READ_MEM_TYPE_ID_CMD;
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Reception of the data */
    if (HAL_OSPI_Receive(&hospi1, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    *manufID      = pData[0];
    *memTypeID    = pData[1];
    *memDensityID = pData[2];

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Enable 4 Byte Mode (EN4B)
*
* @return   tOSPINORStatus Status of the enable operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_Enable4ByteMode(void)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the reset command */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = ENABLE_4BYTE_MODE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

#if 0
/*!
* @brief    Write Extended Address Register (WREAR)
*
* @return   tOSPINORStatus Status of the write operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_WriteExtendedAddressReg(uint8_t data)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 1u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = WRITE_EXT_ADDR_REG_CMD;
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    // Send the associated payload data
    if (HAL_OSPI_Transmit(&hospi1, &data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}
#endif

/*!
* @brief    Write Status Register (WRSR)
*
* @param    statData    Status register bits to be set on the NOR flash.
* @param    cfgData     Config register bits to be set on the NOR flash.
*
* @return   tOSPINORStatus Status of the write operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_WriteStatusCfgReg(const uint8_t* const statData, const uint8_t* const cfgData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    uint8_t pData[2];                                             // Buffer to hold data to send

    // Form data to be sent
    pData[0] = statData[0];
    pData[1] = cfgData[0];

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0u;//WRITE_STATUS_REG_DUMMY_CYCLES;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 2u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = WRITE_STATUS_REG_CMD;
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    // Send the associated payload data
    if (HAL_OSPI_Transmit(&hospi1, &pData[0x00], HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Write Status Register (WRSR)
*
* @param    statData    Status register bits to be set on the NOR flash.
* @param    cfgData     Config register bits to be set on the NOR flash.
*
* @return   tOSPINORStatus Status of the write operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
#if 0
tOSPINORStatus OSPI_NOR_WriteStatusCfgRegQuad(const uint8_t* const statData, const uint8_t* const cfgData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    uint8_t pData[2];                                             // Buffer to hold data to send

    // Form data to be sent
    pData[0] = statData[0];
    pData[1] = cfgData[0];

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 2u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = WRITE_STATUS_REG_CMD;
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    // Send the associated payload data
    if (HAL_OSPI_Transmit(&hospi1, &pData[0x00], HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}
#endif

/*!
* @brief    Read Status Register (RDSR)
*
* @param    pData     Buffer to hold read status.
*
* @return   tOSPINORStatus Status of the read operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_ReadStatusReg(uint8_t* const pData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0u; //DUMMY_CYCLES_READ_OCTAL;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 1u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = READ_STATUS_REG_CMD;
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Reception of the data */
    if (HAL_OSPI_Receive(&hospi1, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Read Security Register (RDSCUR)
*
* @param    pData     Buffer to hold read status.
*
* @return   tOSPINORStatus Status of the read operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_ReadSecurityReg(uint8_t* const pData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0u; //DUMMY_CYCLES_READ_OCTAL;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 1u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = READ_SECURITY_REG_CMD;
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Reception of the data */
    if (HAL_OSPI_Receive(&hospi1, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Read Config Register (RDCR)
*
* @param    pData     Buffer to hold read configuration.
*
* @return   tOSPINORStatus Status of the read operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_ReadCfgReg(uint8_t* const pData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0u; //DUMMY_CYCLES_READ_OCTAL;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 1u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = READ_CONFIG_REG_CMD;
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    /* Configure the command */
    if(HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Reception of the data */
    if (HAL_OSPI_Receive(&hospi1, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief  This function read the SR of the memory and wait the EOP.
*
* @param    hospi       OSPI handle
* @return   tOSPINORStatus Status of the read operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_AutoPollingMemReady(OSPI_HandleTypeDef* const hospi)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    uint8_t reg[2];                                               // Register for read status

    /* Configure automatic polling mode to wait for memory ready ------ */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = READ_STATUS_REG_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_4_LINES;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.Address            = 0x0u;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_4_LINES;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_4_LINES;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData             = 1u;
    sCommand.DummyCycles        = DUMMY_CLOCK_CYCLES_READ_REG;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    do
    {
        HAL_StatusTypeDef status;
        status = HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
        if (status != HAL_OK)
        {
            return OSPI_NOR_FAIL;
        }

        if (HAL_OSPI_Receive(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return OSPI_NOR_FAIL;
        }
    }
    while((reg[0] & MEMORY_READY_MASK_VALUE) != MEMORY_READY_MATCH_VALUE);

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Reset NOR Flash memory device (RST)
*
*           Flash device requires the reset enable command, before the reset command can
*           be accepted.
*
* @return   tOSPINORStatus Status of the reset operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_Reset(void)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the reset command */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = RESET_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_4_LINES;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Enable Reset NOR Flash memory device (RSTEN)
*
* @return   tOSPINORStatus Status of the reset enable operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_Reset_Enable(void)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the reset command */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = RESET_ENABLE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_4_LINES;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

//
// SINGLE MODE ONLY COMMANDS
//

/*!
* @brief  Set NOR device to SPI single mode.
*
* @return tOSPINORStatus Status of the set to SPI mode commands.
*/
tOSPINORStatus OSPI_NOR_SPIMode(void)
{
    // Enable reset
    if (OSPI_NOR_Reset_Enable() != OSPI_NOR_SUCCESS)
    {
        return OSPI_NOR_FAIL;
    }

    // Reset the NOR device (sets back to SPI mode)
    if (OSPI_NOR_Reset() != OSPI_NOR_SUCCESS)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief  This function send a Write Enable and wait it is effective.
*
* @param  hospi     OSPI handle
* @return tOSPINORStatus Status of the write enable commands.
*/
tOSPINORStatus OSPI_NOR_WriteEnable(OSPI_HandleTypeDef* const hospi)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    uint8_t reg[2];                                               // Read status information

    /* Enable write operations ------------------------------------------ */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = WRITE_ENABLE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for write enabling ---- */
    sCommand.Instruction    = READ_STATUS_REG_CMD;
    sCommand.Address        = 0x0u;
    sCommand.AddressMode    = HAL_OSPI_ADDRESS_NONE;
    sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataMode       = HAL_OSPI_DATA_1_LINE;
    sCommand.DataDtrMode    = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData         = 1u;
    sCommand.DummyCycles    = DUMMY_CLOCK_CYCLES_READ_REG;

    do
    {
        if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return OSPI_NOR_FAIL;
        }

        if (HAL_OSPI_Receive(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return OSPI_NOR_FAIL;
        }
    }
    while((reg[0] & WRITE_ENABLE_MASK_VALUE) != WRITE_ENABLE_MATCH_VALUE);

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief  Read (Normal Read) (READ).
*
* @param    pData       Pointer to data to be read
* @param    ReadAddr    Read start address
* @param    Size        Size of data to read
* @return   tOSPINORStatus Status of the read quad operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
#if 0
tOSPINORStatus OSPI_NOR_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0u; //DUMMY_CYCLES_READ_OCTAL;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.Address               = ReadAddr;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.AddressSize           = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = Size;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = READ_CMD;
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Reception of the data */
    if (HAL_OSPI_Receive(&hospi1, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}
#endif

//
// QUAD MODE ONLY COMMANDS
//

/*!
* @brief    Enable-QPI (EQIO)
*
* @return   tOSPINORStatus Status of the enter quad mode operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_QSPIMode(void)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Enable QPI mode ---------------------------------------- */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = ENTER_QUAD_MODE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = ENTER_QUAD_DUMMY_CYCLES;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    /* Send the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Enable-QPI (EQIO)
*
* @return   tOSPINORStatus Status of the enter quad mode operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
#if 0
tOSPINORStatus OSPI_NOR_QSPIMode_AutoPolling(void)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    OSPI_AutoPollingTypeDef sConfig;                              // Polled config register

    /* Enable QPI mode ---------------------------------------- */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = ENTER_QUAD_MODE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for write enabling ---- */
    sCommand.Instruction = READ_STATUS_REG_CMD;
    sCommand.DataMode    = HAL_OSPI_DATA_4_LINES;
    sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData      = 1u;

    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    sConfig.Match         = QUAD_ENABLE_MATCH_VALUE;
    sConfig.Mask          = QUAD_ENABLE_MASK_VALUE;
    sConfig.MatchMode     = HAL_OSPI_MATCH_MODE_AND;
    sConfig.Interval      = AUTO_POLLING_INTERVAL;
    sConfig.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_OSPI_AutoPolling(&hospi1, &sConfig, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}
#endif

/*!
* @brief    Fast Read Quad (4READ)
*
* @param    pData       Data buffer to hold the data read.
* @param    ReadAddr    Address to be read from.
* @param    Size        Size of data block to be read.
* @return   tOSPINORStatus Status of the read quad operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_ReadQPIQuad4B(uint32_t ReadAddr, uint8_t* const pData, uint32_t Size)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = FAST_READ_QUAD_DUMMY_CYCLES;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.Address               = ReadAddr;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_4_LINES;
    sCommand.AddressSize           = HAL_OSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_4_LINES;
    sCommand.NbData                = Size;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = IO_READ_CMD; // must match in OSPI_NOR_EnableMemMappedQuadMode()
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Reception of the data */
    if (HAL_OSPI_Receive(&hospi1, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief  This function send a Write Enable (for QPI mode) and wait it is effective.
*
* @param  hospi     OSPI handle
* @return tOSPINORStatus Status of the write enable commands.
*/
#if 0
tOSPINORStatus OSPI_NOR_WriteEnableQuad(OSPI_HandleTypeDef* const hospi)
{
    OSPI_RegularCmdTypeDef  sCommand;
    uint8_t reg[2];

    /* Enable write operations ------------------------------------------ */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = WRITE_ENABLE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_4_LINES;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for write enabling ---- */
    sCommand.Instruction    = READ_STATUS_REG_CMD;
    sCommand.Address        = 0x0u;
    sCommand.AddressMode    = HAL_OSPI_ADDRESS_NONE;
    sCommand.AddressSize    = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataMode       = HAL_OSPI_DATA_4_LINES;
    sCommand.DataDtrMode    = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData         = 1u;
    sCommand.DummyCycles    = DUMMY_CLOCK_CYCLES_READ_REG;

    do
    {
        if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return OSPI_NOR_FAIL;
        }

        if (HAL_OSPI_Receive(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return OSPI_NOR_FAIL;
        }
    }
    while((reg[0] & WRITE_ENABLE_MASK_VALUE) != WRITE_ENABLE_MATCH_VALUE);

    return OSPI_NOR_SUCCESS;
}
#endif

/*!
* @brief    Quad Page Program (4PP4B)
*
* @param    address     Address of page to be programmed.
* @param    buffer      Buffer of data to be programmed (MAXBUFFER size).
* @return   tOSPINORStatus Status of the quad page program operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_WriteSPIPageQuad4B(uint32_t address, uint8_t* const buffer, uint32_t buffersize)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the program command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction           = QUAD_PAGE_WRITE_4B_CMD; // must match in OSPI_NOR_EnableMemMappedQuadMode()
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_4_LINES;
    sCommand.AddressSize           = HAL_OSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_4_LINES;
    sCommand.DummyCycles           = 0u;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;

    sCommand.Address = address;
    sCommand.NbData  = buffersize;

    /* Configure the command */
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Transmission of the data */
    if (HAL_OSPI_Transmit(&hospi1, buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for end of program */
    OSPI_NOR_AutoPollingMemReady(&hospi1);

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Sector Erase [4K] (SE4B)
*
* @param    address     Address of the 4K sector to be erased.
* @return   tOSPINORStatus Status of the erase operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_EraseSector4KB(uint32_t address)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Erasing Sequence ------------------------------------------------- */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = SECTOR_4K_B_ERASE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.Address            = address;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;

    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for end of erase ------ */
    if (OSPI_NOR_AutoPollingMemReady(&hospi1) != OSPI_NOR_SUCCESS)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Block Erase [32K] (BE32K)
*
* @param    address     Address of the 32K block to be erased.
* @return   tOSPINORStatus Status of the erase operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
#if 0
tOSPINORStatus OSPI_NOR_BlockErase32K(uint32_t address)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Erasing Sequence ------------------------------------------------- */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = BLOCK_32K_ERASE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.Address            = address;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;

    if (HAL_OSPI_Command_IT(&hospi1, &sCommand) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for end of erase ------ */
    if (OSPI_NOR_AutoPollingMemReady(&hospi1) != OSPI_NOR_SUCCESS)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Block Erase [64K] (BE)
*
* @param    address     Address of the 64K block to be erased.
* @return   tOSPINORStatus Status of the erase operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_BlockErase64K(uint32_t address)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Erasing Sequence ------------------------------------------------- */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = BLOCK_64K_ERASE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.Address            = address;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;

    if (HAL_OSPI_Command_IT(&hospi1, &sCommand) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for end of erase ------ */
    if (OSPI_NOR_AutoPollingMemReady(&hospi1) != OSPI_NOR_SUCCESS)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Chip Erase (CE)
*
* @return   tOSPINORStatus Status of the erase operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_ChipErase(void)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Erasing Sequence ------------------------------------------------- */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = CHIP_ERASE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0u;

    if (HAL_OSPI_Command_IT(&hospi1, &sCommand) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for end of erase ------ */
    if (OSPI_NOR_AutoPollingMemReady(&hospi1) != OSPI_NOR_SUCCESS)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Function to Enable Memory mapped mode in Quad mode 4-4-4
*
* @return   tOSPINORStatus Status of the read quad operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_EnableMemMappedQuadMode(void)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    OSPI_MemoryMappedTypeDef sMemMappedCfg;

    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
    sCommand.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = HAL_OSPI_DATA_4_LINES;
    sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.Address = 0u;
    sCommand.NbData = 1u;

    /* Memory-mapped mode configuration for Quad Read mode 4-4-4*/
    sCommand.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
    sCommand.Instruction = IO_READ_CMD;
    sCommand.DummyCycles = FAST_READ_QUAD_DUMMY_CYCLES;
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Memory-mapped mode configuration for Quad Write mode 4-4-4*/
    sCommand.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
    sCommand.Instruction = QUAD_PAGE_WRITE_4B_CMD;
    sCommand.DummyCycles = WRITE_QUAD_DUMMY_CYCLES;
    sCommand.DQSMode = HAL_OSPI_DQS_ENABLE;
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /*Disable timeout counter for memory mapped mode*/
    sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;

    /*Enable memory mapped mode*/
    if (HAL_OSPI_MemoryMapped(&hospi1, &sMemMappedCfg) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief    Function to stop the OSPI memory mapped 4-4-4 mode
*
* @return   tOSPINORStatus Status of the abort operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_DisableMemMappedQuadMode(void)
{
    /* Abort OctoSPI driver to stop the memory-mapped mode ------------ */
    if (HAL_OSPI_Abort(&hospi1) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    return OSPI_NOR_SUCCESS;
}

//static uint8_t aTxBuffer[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF };
static uint8_t aTxBuffer[] = { 0xDE, 0xAD, 0xBE, 0xEF };
//static uint8_t aTxBuffer[65536];
static uint8_t aRxBuffer[sizeof(aTxBuffer)] = { 0 };
#define BUFFERSIZE 4u
tOSPINORStatus OSPI_NOR_EraseWriteQPIExample(uint32_t address, __IO uint8_t* const step)
{
    uint8_t statData[10] = {0}, cfgData[10] = {0}, scurData = 0u;
    tOSPINORStatus status = OSPI_NOR_SUCCESS;

    switch(*step)
    {
        case 0:
            CmdCplt = 0u;
            /*for (uint32_t i = 0; i < 65536; i++)
            {
            aTxBuffer[i] = i % 256u;
            }*/
            /* Enable write operations ------------------------------------------ */
            OSPI_NOR_WriteEnable(&hospi1);
            OSPI_NOR_ReadStatusReg(&statData[0x00]);
            if ((statData[0x00] & 0x02u) != 0u)
            {
                // Erase a sector
                OSPI_NOR_BlockErase64K(address);

                *step = (*step)+1u;
            }
            else
            {
                status = OSPI_NOR_FAIL;
            }
            break;

        case 1:
        {
            if(CmdCplt != 0u)
            {
                CmdCplt = 0u;

                //
                // Ensure erase status is OK
                //
                OSPI_NOR_ReadSecurityReg(&scurData);
                if (scurData == 0x00u)
                {
                    // Read status and config registers using SPI mode, to OR
                    // the new bit settings in, as we don't want to clobber the Block Protect bits.
                    OSPI_NOR_SPIMode();                                     // Reset flash device back to SPI mode

                    OSPI_NOR_ReadStatusReg(statData);                       // Read status register
                    OSPI_NOR_ReadCfgReg(cfgData);                           // Read config register
                    cfgData[0] = cfgData[0] | 0x20u;                         // Set 4-byte addressing
                    statData[0] = statData[0] | 0x40u;                       // Set QE bit enabled

                    OSPI_NOR_WriteEnable(&hospi1);                          // Enable flash register writes
                    OSPI_NOR_WriteStatusCfgReg(statData, cfgData);          // Set status & config registers

                    OSPI_NOR_WriteEnable(&hospi1);
                    OSPI_NOR_WriteSPIPageQuad(address, aTxBuffer, BUFFERSIZE);

                    HAL_Delay(MEMORY_PAGE_PROG_DELAY);

                    //
                    // Ensure programming status is OK
                    //
                    OSPI_NOR_ReadSecurityReg(&scurData);
                    if (scurData == 0x00u)
                    {
                        OSPI_NOR_QSPIMode();                                    // Enable QE mode
                        // Read array data (start of verify sequence?)
                        OSPI_NOR_ReadQPIQuad(address, aRxBuffer, BUFFERSIZE);

                        OSPI_NOR_SPIMode();

                        if (memcmp(aRxBuffer, aTxBuffer, BUFFERSIZE) != 0)
                        {
                            status = OSPI_NOR_FAIL;
                        }
                    }
                    else
                    {
                        status = OSPI_NOR_FAIL;
                    }
                }
                else
                {
                    status = OSPI_NOR_FAIL;
                }
            }
            HAL_Delay(50u);

            *step = 0u;
        }
        break;

        default :
            status = OSPI_NOR_FAIL;
            break;
    }

    return status;
}

/**
  * @brief  Command completed callbacks.
  * @param  hospi: OSPI handle
  * @retval None
  */
void HAL_OSPI_CmdCpltCallback(OSPI_HandleTypeDef *hospi)
{
    CmdCplt++;
}
#endif

/*!
* @brief    Initialise NOR flash
*
* @return   tOSPINORStatus Status of the initialise operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_Init(void)
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    mx25Status &= OSPI_NOR_SPIMode();

    uint8_t id[3];
    mx25Status &= OSPI_NOR_ReadManufDeviceID(&id[0], &id[1], &id[2]);
    mx25Status &= (tOSPINORStatus)(int)(id[0] == 0xc2u);
    mx25Status &= (tOSPINORStatus)(int)(id[1] == 0x20u);
    mx25Status &= (tOSPINORStatus)(int)(id[2] == 0x19u);

    // enable 32-bit addressing
    mx25Status &= OSPI_NOR_Enable4ByteMode();

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

/*!
* @brief    Read a number of bytes
*
* @param    blk_addr            Block address to read.
* @param    buf                 Pointer to buffer to read.
* @param    size                Number of bytes to read.
*
* @return   tOSPINORStatus Status of the read operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_Read(uint32_t blk_addr, uint8_t* const buf, uint32_t size)
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    // size is limited to sector size in this implementation
    assert(size <= STORAGE_ERASE_SIZ);

    // Read array data
    mx25Status &= OSPI_NOR_QSPIMode();                                    // Enable QE mode
    mx25Status &= OSPI_NOR_ReadQPIQuad4B(blk_addr, buf, size);
    mx25Status &= OSPI_NOR_SPIMode();

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

/*!
* @brief    Equivalent of floor() but when memory is split into regions of discrete increments (e.g. blocks, sectors, pages)
*
* @param    addr                Address.
* @param    inc                 Increment in bytes.
*
* @return   First address.
*/
uint32_t OSPI_NOR_GetFirstAddr(uint32_t addr, uint32_t inc)
{
    // truncate with integer division
    return (addr / inc) * inc;
}

/*!
* @brief    Erase and write a number of bytes
*
* @param    blk_addr            Block address to erase.
* @param    buf                 Pointer to buffer to write.
* @param    size                Number of bytes to write.
*
* @return   tOSPINORStatus Status of the erase and write operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_EraseWrite(uint32_t blk_addr, uint8_t* const buf, uint32_t size)
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    // size is limited to sector size in this implementation
    assert(size <= STORAGE_ERASE_SIZ);

    uint32_t erase_addr_first = OSPI_NOR_GetFirstAddr(blk_addr, STORAGE_ERASE_SIZ);
    uint32_t erase_addr_last  = OSPI_NOR_GetFirstAddr(blk_addr + size - 1u, STORAGE_ERASE_SIZ) + STORAGE_ERASE_SIZ;
    uint32_t segment_first = blk_addr;
    uint32_t segment_last = segment_first + size;
    if (segment_last > (erase_addr_first + STORAGE_ERASE_SIZ))
    {
        segment_last = erase_addr_first + STORAGE_ERASE_SIZ;
    }
    uint32_t segment_size = segment_last - segment_first;
    uint32_t buf_addr = 0u;
    uint32_t offset = segment_first - erase_addr_first;

    // Read each affected sector into readback buffer and update the bytes in it that need to change
    for (uint32_t i = erase_addr_first; i < erase_addr_last; i += STORAGE_ERASE_SIZ)
    {
        mx25Status &= OSPI_NOR_Read(i, bufReadback, STORAGE_ERASE_SIZ);

        bool dirty = false;
        for (uint32_t j = segment_first; j < segment_last; j++)
        {
            // sector doesn't need erasing if affected bytes are already erased or already match affected bytes
            // could reduce number of erasures by checking if the affected bytes need to flip any bit from 0 to 1
            if ((bufReadback[j - segment_first + offset] != buf[buf_addr]) && (bufReadback[j - segment_first + offset] != 0xffu))
            {
                dirty = true;
            }

            bufReadback[j - segment_first + offset] = buf[buf_addr];
            buf_addr++;
        }

        if (dirty)
        {
            // erase sector
            mx25Status &= OSPI_NOR_Erase(i, STORAGE_BLK_SIZ);
            assert(mx25Status == OSPI_NOR_SUCCESS);

            // write all pages in sector
            for (uint32_t j = 0u; j < STORAGE_ERASE_SIZ; j += STORAGE_PAGE_SIZ)
            {
                mx25Status &= OSPI_NOR_Write(i + j, bufReadback + j, STORAGE_PAGE_SIZ);
                assert(mx25Status == OSPI_NOR_SUCCESS);
            }
        }
        else
        {
            // round down first address and round up last address within sector to sector boundary to get changed pages
            uint32_t write_addr_first = OSPI_NOR_GetFirstAddr(segment_first - i, STORAGE_PAGE_SIZ);
            uint32_t write_addr_last = OSPI_NOR_GetFirstAddr(segment_last - i + STORAGE_PAGE_SIZ, STORAGE_PAGE_SIZ);
            if (write_addr_last > STORAGE_ERASE_SIZ)
            {
                write_addr_last = STORAGE_ERASE_SIZ;
            }

            // write changed pages in sector
            for (uint32_t j = write_addr_first; j < write_addr_last; j += STORAGE_PAGE_SIZ)
            {
                mx25Status &= OSPI_NOR_Write(i + j, bufReadback + j, STORAGE_PAGE_SIZ);
                assert(mx25Status == OSPI_NOR_SUCCESS);
            }
        }

        // align next segment with next sector
        segment_first = segment_last;
        segment_last = segment_first + size - segment_size;
        segment_size = segment_last - segment_first;
        offset = 0u;
    }

    // verify successful write
    mx25Status &= OSPI_NOR_Read(blk_addr, bufReadback, size);
    assert(mx25Status == OSPI_NOR_SUCCESS);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(bufReadback, buf, size) == 0);

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

/*!
* @brief    Erase a number of bytes
*
* @param    blk_addr            Block address to erase.
* @param    size                Number of bytes to write.
*
* @return   tOSPINORStatus Status of the erase operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_Erase(uint32_t blk_addr, uint32_t size)
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    // address must be on sector boundary
    assert ((blk_addr % STORAGE_BLK_SIZ) == 0u);

    // note WPSEL bit is 0 (block protection mode) remaining at factory default and is only one time programmable to 1 (individual sector protection mode),
    // so block erase is not possible.

    uint8_t statData[10] = {0};
    int timeoutCount = 0;
    const int timeoutCountLimit = 400;

    for (uint32_t i = 0u; i < size; i+= STORAGE_BLK_SIZ)
    {
        /* Enable write operations ------------------------------------------ */
        mx25Status &= OSPI_NOR_SPIMode();
        assert(mx25Status == OSPI_NOR_SUCCESS);
        mx25Status &= OSPI_NOR_WriteEnable(&hospi1);
        assert(mx25Status == OSPI_NOR_SUCCESS);
        mx25Status &= OSPI_NOR_ReadStatusReg(&statData[0x00]);
        assert(mx25Status == OSPI_NOR_SUCCESS);
        if ((statData[0x00] & 0x02u) != 0u)
        {
            // Erase sector
            mx25Status &= OSPI_NOR_EraseSector4KB(blk_addr + i);
            assert(mx25Status == OSPI_NOR_SUCCESS);

            // Wait for WEL and WIP bits in SR to clear
            timeoutCount = 0;
            do
            {
                mx25Status &= OSPI_NOR_ReadStatusReg(&statData[0x00]);
                assert(mx25Status == OSPI_NOR_SUCCESS);
                HAL_Delay(1u);

                if (++timeoutCount > timeoutCountLimit)
                {
                    mx25Status = OSPI_NOR_FAIL;
                    break;
                }
            }
            while ((statData[0x00] & 0x03u) != 0u);
        }
        else
        {
            mx25Status = OSPI_NOR_FAIL;
        }

        assert(mx25Status == OSPI_NOR_SUCCESS);
        if (mx25Status == OSPI_NOR_FAIL)
        {
            break;
        }
    }

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

/*!
* @brief    Write number of bytes to NOR flash
*
* @param    blk_addr            Block address to write to.
* @param    buf                 Pointer to buffer to write.
* @param    size                Number of bytes to write.
*
* @return   tOSPINORStatus Status of the write operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_Write(uint32_t blk_addr, uint8_t* const buf, uint32_t size)
{
    // Address must be on page boundary and size a multiple of page boundary
    assert ((blk_addr % STORAGE_PAGE_SIZ) == 0u);
    assert ((size % STORAGE_PAGE_SIZ) == 0u);

    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    for (uint32_t i = 0u; i < size; i += STORAGE_PAGE_SIZ)
    {
        mx25Status &= OSPI_NOR_PageWrite(blk_addr + i, buf + i);
    }

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

/*!
* @brief    Write a page of size STORAGE_PAGE_SIZ to NOR flash
*
* @param    blk_addr            Block address to write to.
* @param    buf                 Pointer to buffer to write.
*
* @return   tOSPINORStatus Status of the write operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_PageWrite(uint32_t blk_addr, uint8_t* const buf)
{
    // Address must be on page boundary
    assert ((blk_addr % STORAGE_PAGE_SIZ) == 0u);

    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;
    uint8_t statData[10] = {0}, cfgData[10] = {0}, scurData = 0u;

    // Ensure erase status is OK
    mx25Status &= OSPI_NOR_SPIMode();
    mx25Status &= OSPI_NOR_ReadSecurityReg(&scurData);
    mx25Status &= (tOSPINORStatus)(int)(scurData == 0x00u);

    if (scurData == 0x00u)
    {
        // Read status and config registers using SPI mode, to OR
        // the new bit settings in, as we don't want to clobber the Block Protect bits.
        mx25Status &= OSPI_NOR_SPIMode();                                     // Reset flash device back to SPI mode

        mx25Status &= OSPI_NOR_ReadStatusReg(statData);                       // Read status register
        statData[0] = statData[0] | 0x40u;                       // Set QE bit enabled

        mx25Status &= OSPI_NOR_ReadCfgReg(cfgData);                           // Read config register
        cfgData[0] = cfgData[0] | 0x20u;                         // Set 4-byte addressing

        mx25Status &= OSPI_NOR_WriteEnable(&hospi1);                          // Enable flash register writes
        mx25Status &= OSPI_NOR_WriteStatusCfgReg(statData, cfgData);          // Set status & config registers
        mx25Status &= OSPI_NOR_WaitStatus( DEF_WAIT_STATUS_WIP, DEF_WAIT_STATUS_WR_STATUS_TIMEOUT );

        mx25Status &= OSPI_NOR_WriteEnable(&hospi1);
        mx25Status &= OSPI_NOR_WriteSPIPageQuad4B(blk_addr, buf, STORAGE_PAGE_SIZ);

        // Wait for WEL and WIP bits in SR to clear
        int timeoutCount = 0;
        const int timeoutCountLimit = 150;
        do
        {
            mx25Status &= OSPI_NOR_ReadStatusReg(&statData[0x00]);
            HAL_Delay(1u);

            if (++timeoutCount > timeoutCountLimit)
            {
                assert(false);
                return OSPI_NOR_FAIL;
            }
        }
        while ((statData[0x00] & 0x03u) != 0u);
    }

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

/*!
* @brief    Wait for status byte bits to be cleared for timeout value provided
* @return   tOSPINORStatus Status of the unit tests, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_WaitStatus( uint8_t pStatusBits, uint32_t pTimeout )
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;
    uint8_t lStatusByte;
    uint32_t lTimeoutCount = 0u;

    do
    {
        mx25Status &= OSPI_NOR_ReadStatusReg(&lStatusByte);
        HAL_Delay(1u);

        if (( ++lTimeoutCount ) > pTimeout )
        {
            assert(false);
            return OSPI_NOR_FAIL;
        }
    }
    while (( lStatusByte & pStatusBits ) > 0u );

    return mx25Status;
}

/*!
* @brief    Self test (destructive to memory contents and causes wear)

* @return   tOSPINORStatus Status of the unit tests, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_SelfTest(void)
{
    // need exclusive access for test
    fsOwnership = SHUTDOWN;

    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    static uint8_t buf[STORAGE_BLK_SIZ];

    // test pattern ensuring that some pages are different
    for (uint32_t i = 0u; i < STORAGE_BLK_SIZ; i++)
    {
        if (i / STORAGE_PAGE_SIZ == 0u)
        {
            buf[STORAGE_PAGE_SIZ - i] = (uint8_t)i;
        }
        else
        {
            buf[i] = (uint8_t)i;
        }
    }

    // another test pattern
    static uint8_t bufSmall[10] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

    mx25Status &= OSPI_NOR_Init();

    // Test sector write aligned on boundary - pass
    mx25Status &= OSPI_NOR_Read(4096u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_Erase(4096u, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_Read(4096u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_EraseWrite(4096u, buf, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_Read(4096u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf, bufReadback, STORAGE_BLK_SIZ) == 0);

    // Test multiple contiguous writes, first aligned on boundary - pass
    mx25Status &= OSPI_NOR_Erase(8192u, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_Read(8192u, bufReadback, 20u);
    mx25Status &= OSPI_NOR_EraseWrite(8192u, buf, 10u);
    mx25Status &= OSPI_NOR_EraseWrite(8192u+10u, buf+10u, 10u);
    mx25Status &= OSPI_NOR_Read(8192u, bufReadback, 20u);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf, bufReadback, 20u) == 0);

    // Test multiple contiguous writes with no alignment - pass
    mx25Status &= OSPI_NOR_Erase(12288u, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_Read(12288u+1u, bufReadback, 30u);
    mx25Status &= OSPI_NOR_EraseWrite(12288u+1u, buf, 15u);
    mx25Status &= OSPI_NOR_EraseWrite(12288u+1u+15u, buf+15u, 15u);
    mx25Status &= OSPI_NOR_Read(12288u+1u, bufReadback, 30u);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf, bufReadback, 30u) == 0);

    // Test accesses not aligning on block boundaries - pass
    mx25Status &= OSPI_NOR_Erase(12288u, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_Read(12288u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_EraseWrite(12288u+1u, bufSmall, 3u);
    mx25Status &= OSPI_NOR_Read(12288u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(bufSmall, bufReadback+1u, 3u) == 0);
    mx25Status &= OSPI_NOR_EraseWrite(12288u+254u, bufSmall, 5u);
    mx25Status &= OSPI_NOR_Read(12288u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(bufSmall, bufReadback+254u, 5u) == 0);
    mx25Status &= OSPI_NOR_EraseWrite(12288u+510u, bufSmall, 1u);
    mx25Status &= OSPI_NOR_Read(12288u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(bufSmall, bufReadback+510u, 1u) == 0);
    mx25Status &= OSPI_NOR_EraseWrite(12288u+511u, bufSmall, 3u);
    mx25Status &= OSPI_NOR_Read(12288u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(bufSmall, bufReadback+511u, 3u) == 0);

    // Test accesses spanning multiple sectors - pass
    memset(buf, 0xff, 416u);
    memset(buf+416u, 0x0, 95u);
    memset(buf+512u, 0xff, 3583u);
    mx25Status &= OSPI_NOR_Erase(0x0u, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_Erase(0x1000u, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_EraseWrite(0x0u, buf, 4096u);
    mx25Status &= OSPI_NOR_Read(0xe60u, bufReadback, 4096u);
    memset(buf, 42, 512u);
    memset(buf+512u, 0xaa, 4096u-512u);
    mx25Status &= OSPI_NOR_EraseWrite(0xe60u, buf, 512u);
    mx25Status &= OSPI_NOR_Read(0xe60u, bufReadback, 416u);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf, bufReadback, 416u) == 0);
    mx25Status &= OSPI_NOR_Read(4096u, bufReadback, 96u);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf+416u, bufReadback, 96u) == 0);
    mx25Status &= OSPI_NOR_Read(0xe60u, bufReadback, 512u);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf, bufReadback, 512u) == 0);

    // Test remaining data in next sector is unaffected and still in erased state - pass
    mx25Status &= OSPI_NOR_Read(0xe60u + 512u, bufReadback, 1u);
    assert(bufReadback[0] == 0xffu);

    // Test read spanning multiple sectors of multiple writes aligned to sectors - pass
    mx25Status &= OSPI_NOR_EraseWrite(12288u, buf, 4096u);
    mx25Status &= OSPI_NOR_EraseWrite(16384u, bufSmall, 10u);
    mx25Status &= OSPI_NOR_Read(12288u+5u, bufReadback, 4096u);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf+5u, bufReadback, 4096u-5u) == 0);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(bufSmall, bufReadback+4096u-5u, 5u) == 0);

    // Test read spanning multiple sectors of write spanning multiple sectors - pass
    mx25Status &= OSPI_NOR_EraseWrite(20480u+5u, buf, 4096u);
    mx25Status &= OSPI_NOR_EraseWrite(24576u+5u, bufSmall, 10u);
    mx25Status &= OSPI_NOR_Read(20480u+5u, bufReadback, 4096u);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf, bufReadback, 4096u) == 0);

    // Test multiple sector erase fully erases expected sectors only (the middle 32Kb block) - pass
    mx25Status &= OSPI_NOR_EraseWrite(0u, bufSmall, 10u);
    for (uint32_t i = 0u; i < 32768u; i += STORAGE_BLK_SIZ)
    {
        mx25Status &= OSPI_NOR_EraseWrite(32768u + i, buf, 4096u);
    }
    mx25Status &= OSPI_NOR_EraseWrite(65536u, bufSmall, 10u);
    mx25Status &= OSPI_NOR_Erase(32768u, 32768u);
    mx25Status &= OSPI_NOR_Read(0u, bufReadback, 10u);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(bufSmall, bufReadback, 10u) == 0);
    for (uint32_t i = 0u; i < 32768u; i += STORAGE_BLK_SIZ)
    {
        mx25Status &= OSPI_NOR_Read(32768u + i, bufReadback, STORAGE_BLK_SIZ);
        for (uint32_t j = 0u; j < STORAGE_BLK_SIZ; j++)
        {
            if (bufReadback[j] != 0xffu)
            {
                mx25Status = OSPI_NOR_FAIL;
            }
        }
    }
    mx25Status &= OSPI_NOR_Read(65536u, bufReadback, 10u);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(bufSmall, bufReadback, 10u) == 0);

    // Test 16MB boundary of 24 to 32 bit addresses
    mx25Status &= OSPI_NOR_EraseWrite(16u*1024u*1024u, buf, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_Read(16u*1024u*1024u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf, bufReadback, STORAGE_BLK_SIZ) == 0);

    // Test 30MB boundary of start of pseudobank region
    mx25Status &= OSPI_NOR_EraseWrite(30u*1024u*1024u, buf, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_Read(30u*1024u*1024u, bufReadback, STORAGE_BLK_SIZ);
    mx25Status &= (tOSPINORStatus)(int)(memcmp(buf, bufReadback, STORAGE_BLK_SIZ) == 0);

    // Relinquish exclusive access
    fsOwnership = NONE;

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

/*!
* @brief    Get the number of blocks for storage for file system (internal) and USB mass storage (external)

* @return   uint32_t number of blocks for storage
*/
uint32_t OSPI_NOR_GetStorageBlocksNumber(void)
{
    // Check bootloader version for compatibility with application
    return STORAGE_BLK_NBR;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 14.7 as functions have multiple points of exit violates the rule.
 **********************************************************************************************************************/
_Pragma("diag_default=Pm073")
_Pragma("diag_default=Pm143")