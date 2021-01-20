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

__IO uint8_t CmdCplt;

/* TODO - MISRA checks temporarily disabled */
MISRAC_DISABLE
#include <string.h>
#include <assert.h>
#define STORAGE_BLK_SIZ 4096 // sector size must match in usbd_storage_if.c
#define STORAGE_PAGE_SIZ 256

/*!
* @brief    Read Electronic Manufacturer and Device ID (RDID)
*
* @param    manufID             Pointer to returned manufacturer ID data.
* @param    memTypeID           Pointer to returned memory type data.
* @param    memDensityID        Pointer to returned memory density data.
*
* @return   tOSPINORStatus Status of the read operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_ReadManufDeviceID(uint8_t *manufID, uint8_t *memTypeID, uint8_t *memDensityID)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    uint8_t pData[3];                                             // Buffer to hold data to send

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 3;
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
    sCommand.DummyCycles        = 0;
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
    sCommand.DummyCycles           = 0;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 1;
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

/*!
* @brief    Write Status Register (WRSR)
*
* @param    statData    Status register bits to be set on the NOR flash.
* @param    cfgData     Config register bits to be set on the NOR flash.
*
* @return   tOSPINORStatus Status of the write operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_WriteStatusCfgReg(uint8_t *statData, uint8_t *cfgData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    uint8_t pData[2];                                             // Buffer to hold data to send

    // Form data to be sent
    pData[0] = statData[0];
    pData[1] = cfgData[0];

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 2;
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
tOSPINORStatus OSPI_NOR_WriteStatusCfgRegQuad(uint8_t *statData, uint8_t *cfgData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure
    uint8_t pData[2];                                             // Buffer to hold data to send

    // Form data to be sent
    pData[0] = statData[0];
    pData[1] = cfgData[0];

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 2;
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
* @brief    Read Status Register (RDSR)
*
* @param    pData     Buffer to hold read status.
*
* @return   tOSPINORStatus Status of the read operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_ReadStatusReg(uint8_t *pData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0; //DUMMY_CYCLES_READ_OCTAL;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 1;
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
tOSPINORStatus OSPI_NOR_ReadSecurityReg(uint8_t *pData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0; //DUMMY_CYCLES_READ_OCTAL;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 1;
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
tOSPINORStatus OSPI_NOR_ReadCfgReg(uint8_t *pData)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0; //DUMMY_CYCLES_READ_OCTAL;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData                = 1;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = READ_CONFIG_REG_CMD;
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
* @brief  This function read the SR of the memory and wait the EOP.
*
* @param    hospi       OSPI handle
* @return   tOSPINORStatus Status of the read operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_AutoPollingMemReady(OSPI_HandleTypeDef *hospi)
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
    sCommand.Address            = 0x0;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_4_LINES;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_4_LINES;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData             = 1;
    sCommand.DummyCycles        = DUMMY_CLOCK_CYCLES_READ_REG;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

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
    sCommand.DummyCycles        = 0;
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
    sCommand.DummyCycles        = 0;
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
        return OSPI_NOR_FAIL;

    // Reset the NOR device (sets back to SPI mode)
    if (OSPI_NOR_Reset() != OSPI_NOR_SUCCESS)
        return OSPI_NOR_FAIL;

    return OSPI_NOR_SUCCESS;
}

/*!
* @brief  This function send a Write Enable and wait it is effective.
*
* @param  hospi     OSPI handle
* @return tOSPINORStatus Status of the write enable commands.
*/
tOSPINORStatus OSPI_NOR_WriteEnable(OSPI_HandleTypeDef *hospi)
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
    sCommand.DummyCycles        = 0;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for write enabling ---- */
    sCommand.Instruction    = READ_STATUS_REG_CMD;
    sCommand.Address        = 0x0;
    sCommand.AddressMode    = HAL_OSPI_ADDRESS_NONE;
    sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataMode       = HAL_OSPI_DATA_1_LINE;
    sCommand.DataDtrMode    = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData         = 1;
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
tOSPINORStatus OSPI_NOR_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the read command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.DummyCycles           = 0; //DUMMY_CYCLES_READ_OCTAL;
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
    sCommand.DummyCycles        = 0;
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
    sCommand.NbData      = 1;

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

/*!
* @brief    Fast Read Quad (4READ)
*
* @param    pData       Data buffer to hold the data read.
* @param    ReadAddr    Address to be read from.
* @param    Size        Size of data block to be read.
* @return   tOSPINORStatus Status of the read quad operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_ReadQPIQuad(uint32_t ReadAddr, uint8_t* pData, uint32_t Size)
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
    sCommand.AddressSize           = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_4_LINES;
    sCommand.NbData                = Size;
    sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Instruction           = FAST_READ_QUAD_CMD;
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
tOSPINORStatus OSPI_NOR_WriteEnableQuad(OSPI_HandleTypeDef *hospi)
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
    sCommand.DummyCycles        = 0;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Configure automatic polling mode to wait for write enabling ---- */
    sCommand.Instruction    = READ_STATUS_REG_CMD;
    sCommand.Address        = 0x0;
    sCommand.AddressMode    = HAL_OSPI_ADDRESS_NONE;
    sCommand.AddressSize    = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataMode       = HAL_OSPI_DATA_4_LINES;
    sCommand.DataDtrMode    = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData         = 1;
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
* @brief    Quad Page Program (4PP)
*
* @param    address     Address of page to be programmed.
* @param    buffer      Buffer of data to be programmed (MAXBUFFER size).
* @return   tOSPINORStatus Status of the quad page program operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_WriteSPIPageQuad(uint32_t address, uint8_t *buffer, uint32_t buffersize)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Initialize the program command */
    sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction           = QUAD_PAGE_WRITE_CMD;
    sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode           = HAL_OSPI_ADDRESS_4_LINES;
    sCommand.AddressSize           = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode              = HAL_OSPI_DATA_4_LINES;
    sCommand.DummyCycles           = 0;
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
* @brief    Sector Erase [4K] (SE)
*
* @param    address     Address of the 4K sector to be erased.
* @return   tOSPINORStatus Status of the erase operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
tOSPINORStatus OSPI_NOR_EraseSector4K(uint32_t address)
{
    OSPI_RegularCmdTypeDef sCommand;                              // OCTOSPI command structure

    /* Erasing Sequence ------------------------------------------------- */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = SECTOR_4K_ERASE_CMD;
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
    sCommand.DummyCycles        = 0;

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
* @brief    Block Erase [32K] (BE32K)
*
* @param    address     Address of the 32K block to be erased.
* @return   tOSPINORStatus Status of the erase operation, OSPI_NOR_SUCCESS or OSPI_NOR_FAIL.
*/
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
    sCommand.DummyCycles        = 0;

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
    sCommand.DummyCycles        = 0;

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
    sCommand.DummyCycles        = 0;

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
    sCommand.Address = 0;
    sCommand.NbData = 1;

    /* Memory-mapped mode configuration for Quad Read mode 4-4-4*/
    sCommand.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
    sCommand.Instruction = FAST_READ_QUAD_CMD;
    sCommand.DummyCycles = FAST_READ_QUAD_DUMMY_CYCLES;
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return OSPI_NOR_FAIL;
    }

    /* Memory-mapped mode configuration for Quad Write mode 4-4-4*/
    sCommand.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
    sCommand.Instruction = QUAD_PAGE_WRITE_CMD;
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

extern __IO uint8_t CmdCplt;

//static uint8_t aTxBuffer[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF };
static uint8_t aTxBuffer[] = { 0xDE, 0xAD, 0xBE, 0xEF };
//static uint8_t aTxBuffer[65536];
static uint8_t aRxBuffer[sizeof(aTxBuffer)] = { 0 };
#define BUFFERSIZE 4u
tOSPINORStatus OSPI_NOR_EraseWriteQPIExample(uint32_t address, __IO uint8_t *step)
{
    uint8_t statData[10] = {0}, cfgData[10] = {0}, scurData = 0;
    tOSPINORStatus status = OSPI_NOR_SUCCESS;

    switch(*step)
    {
        case 0:
            CmdCplt = 0;
            /*for (uint32_t i = 0; i < 65536; i++)
            {
            aTxBuffer[i] = i % 256u;
            }*/
            /* Enable write operations ------------------------------------------ */
            OSPI_NOR_WriteEnable(&hospi1);
            OSPI_NOR_ReadStatusReg(&statData[0x00]);
            if ((statData[0x00] & 0x02) != 0)
            {
                // Erase a sector
                OSPI_NOR_BlockErase64K(address);

                *step = (*step)+1;
            }
            else
            {
                status = OSPI_NOR_FAIL;
            }
            break;

        case 1:
        {
            if(CmdCplt != 0)
            {
                CmdCplt = 0;

                //
                // Ensure erase status is OK
                //
                OSPI_NOR_ReadSecurityReg(&scurData);
                if (scurData == 0x00)
                {
                    // Read status and config registers using SPI mode, to OR
                    // the new bit settings in, as we don't want to clobber the Block Protect bits.
                    OSPI_NOR_SPIMode();                                     // Reset flash device back to SPI mode

                    OSPI_NOR_ReadStatusReg(statData);                       // Read status register
                    OSPI_NOR_ReadCfgReg(cfgData);                           // Read config register
                    cfgData[0] = cfgData[0] | 0x20;                         // Set 4-byte addressing
                    statData[0] = statData[0] | 0x40;                       // Set QE bit enabled

                    OSPI_NOR_WriteEnable(&hospi1);                          // Enable flash register writes
                    OSPI_NOR_WriteStatusCfgReg(statData, cfgData);          // Set status & config registers

                    OSPI_NOR_WriteEnable(&hospi1);
                    OSPI_NOR_WriteSPIPageQuad(address, aTxBuffer, BUFFERSIZE);

                    HAL_Delay(MEMORY_PAGE_PROG_DELAY);

                    //
                    // Ensure programming status is OK
                    //
                    OSPI_NOR_ReadSecurityReg(&scurData);
                    if (scurData == 0x00)
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
            HAL_Delay(50);

            *step = 0;
        }
        break;

        default :
            status = OSPI_NOR_FAIL;
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

tOSPINORStatus OSPI_NOR_Init(void)
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    mx25Status &= OSPI_NOR_SPIMode();

    uint8_t id[3];
    mx25Status &= OSPI_NOR_ReadManufDeviceID(&id[0], &id[1], &id[2]);
    mx25Status &= (id[0] == 0xc2);
    mx25Status &= (id[1] == 0x20);
    mx25Status &= (id[2] == 0x19);

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

tOSPINORStatus OSPI_NOR_ReadSector(uint32_t blk_addr, uint8_t *buf)
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    // address must be on sector boundary
    assert ((blk_addr % STORAGE_BLK_SIZ) == 0);

    mx25Status &= OSPI_NOR_QSPIMode();                                    // Enable QE mode

    // Read array data
    mx25Status &= OSPI_NOR_ReadQPIQuad(blk_addr, buf, STORAGE_BLK_SIZ);
    mx25Status &= OSPI_NOR_SPIMode();

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

uint8_t bufReadback[STORAGE_BLK_SIZ];
tOSPINORStatus OSPI_NOR_EraseWriteSector(uint32_t blk_addr, uint8_t *buf)
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    // address must be on sector boundary
    assert ((blk_addr % STORAGE_BLK_SIZ) == 0);

    mx25Status &= OSPI_NOR_ReadSector(blk_addr, &bufReadback[0]);

    bool dirty = false;
    for (int i = 0; i < STORAGE_BLK_SIZ; i++)
    {
        if (bufReadback[i] != 0xff)
        {
            dirty = true;
            break;
        }
    }

    if (dirty)
    {
        mx25Status &= OSPI_NOR_EraseSector(blk_addr);
    }

    for (int i = 0; i < STORAGE_BLK_SIZ; i += STORAGE_PAGE_SIZ)
    {
        mx25Status &= OSPI_NOR_WritePage(blk_addr + i, buf + i);
        mx25Status &= OSPI_NOR_ReadSector(blk_addr, &bufReadback[0]);
    }

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

tOSPINORStatus OSPI_NOR_EraseSector(uint32_t blk_addr)
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    // address must be on sector boundary
    assert ((blk_addr % STORAGE_BLK_SIZ) == 0);

    CmdCplt = 0;
    uint8_t statData[10] = {0};

    mx25Status &= OSPI_NOR_SPIMode();

    /* Enable write operations ------------------------------------------ */
    mx25Status &= OSPI_NOR_WriteEnable(&hospi1);
    mx25Status &= OSPI_NOR_ReadStatusReg(&statData[0x00]);
    if ((statData[0x00] & 0x02) != 0)
    {
        mx25Status &= OSPI_NOR_EraseSector4K(blk_addr);
    }
    else
    {
        assert(false);
        return OSPI_NOR_FAIL;
    }

    // Wait for WEL and WIP bits in SR to clear
    int timeoutCount = 0;
    const int timeoutCountLimit = 150;
    do
    {
        mx25Status &= OSPI_NOR_ReadStatusReg(&statData[0x00]);
        HAL_Delay(1);

        if (++timeoutCount > timeoutCountLimit)
        {
            assert(false);
            return OSPI_NOR_FAIL;
        }
    }
    while ((statData[0x00] & 0x03) != 0);

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

tOSPINORStatus OSPI_NOR_WritePage(uint32_t blk_addr, uint8_t *buf)
{
    // Address must be on page boundary
    assert ((blk_addr % STORAGE_PAGE_SIZ) == 0);

    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;
    uint8_t statData[10] = {0}, cfgData[10] = {0}, scurData = 0;

    mx25Status &= OSPI_NOR_SPIMode();

    CmdCplt = 0;

    //
    // Ensure erase status is OK
    //
    mx25Status &= OSPI_NOR_ReadSecurityReg(&scurData);
    mx25Status &= (scurData == 0x00);

    if (scurData == 0x00)
    {
        // Read status and config registers using SPI mode, to OR
        // the new bit settings in, as we don't want to clobber the Block Protect bits.
        mx25Status &= OSPI_NOR_SPIMode();                                     // Reset flash device back to SPI mode

        mx25Status &= OSPI_NOR_ReadStatusReg(statData);                       // Read status register
        statData[0] = statData[0] | 0x40;                       // Set QE bit enabled

        mx25Status &= OSPI_NOR_ReadCfgReg(cfgData);                           // Read config register
        cfgData[0] = cfgData[0] | 0x20;                         // Set 4-byte addressing

        mx25Status &= OSPI_NOR_WriteEnable(&hospi1);                          // Enable flash register writes
        mx25Status &= OSPI_NOR_WriteStatusCfgReg(statData, cfgData);          // Set status & config registers

        mx25Status &= OSPI_NOR_WriteEnable(&hospi1);
        mx25Status &= OSPI_NOR_WriteSPIPageQuad(blk_addr, buf, STORAGE_PAGE_SIZ);

        // Wait for WEL and WIP bits in SR to clear
        int timeoutCount = 0;
        const int timeoutCountLimit = 150;
        do
        {
            mx25Status &= OSPI_NOR_ReadStatusReg(&statData[0x00]);
            HAL_Delay(1);

            if (++timeoutCount > timeoutCountLimit)
            {
                assert(false);
                return OSPI_NOR_FAIL;
            }
        }
        while ((statData[0x00] & 0x03) != 0);
    }

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

tOSPINORStatus OSPI_NOR_Tests()
{
    tOSPINORStatus mx25Status = OSPI_NOR_SUCCESS;

    uint8_t buf[STORAGE_BLK_SIZ];
    uint8_t bufReadback[STORAGE_BLK_SIZ];

    // test pattern ensuring that some pages are different
    for (int i = 0; i < STORAGE_BLK_SIZ; i++)
    {
        if (i / STORAGE_PAGE_SIZ == 0)
        {
            buf[STORAGE_PAGE_SIZ-i] = i % 256;
        }
        else
        {
            buf[i] = i % 256;
        }
    }

    mx25Status &= OSPI_NOR_Init();
    mx25Status &= OSPI_NOR_ReadSector(8192, &bufReadback[0]);
    mx25Status &= OSPI_NOR_EraseSector(8192);
    mx25Status &= OSPI_NOR_ReadSector(8192, &bufReadback[0]);
    mx25Status &= OSPI_NOR_EraseWriteSector(8192, buf);
    mx25Status &= OSPI_NOR_ReadSector(8192, &bufReadback[0]);

    mx25Status &= memcmp(buf, bufReadback, STORAGE_BLK_SIZ) == 0;

    assert(mx25Status == OSPI_NOR_SUCCESS);
    return mx25Status;
}

/* TODO - MISRA checks temporarily disabled */
MISRAC_ENABLE