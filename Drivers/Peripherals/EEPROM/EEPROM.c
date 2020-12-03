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
* @file     EEPROM.c
* @version  1.00.00
* @author   Harvinder Bhuhi (based on Piyali Sengupta's DPI705E code)
* @date     15 June 2020
*
* @brief    The EEPROM base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "EEPROM.h"
#include "i2c.h"


/* Constants and Defines --------------------------------------------------------------------------------------------*/
#define MAX_EEPROM_PG_WR    0x20u
#define MIN_EEPROM_PG_WR    0x01u
#define EEPROM_MEM_RD       0xA1u
#define EEPROM_MEM_WR       0xA0u
#define EEPROM_ID_RD        0xB1u
#define EEPROM_ID_WR        0xB0u
#define EEPROM_REG_SIZE     0x02u
#define EEPROM_SIZE_MAX     0x1F40u

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
* @brief   Function reads data starting from offsetLocation in EEPROM to buffer address destAddr where
*          length of data transfer is no_of_bytes. The offset location is calculated from 0.
* @param   destAddr pointer to location to store the data read from EEPROM
* @param   offsetLocation Offset from memory start location in EEPROM
* @param   no_of_bytes Length of data to be read
* @return  EEPROM read status
*/
bool eepromRead(uint8_t *destAddr, uint16_t offsetLocation, uint16_t no_of_bytes)
{
    HAL_StatusTypeDef status = HAL_OK;

    //Reads no_of_bytes of data when the read offset and page start aligns
#ifdef NO_HARDWARE_AVAILABLE
    status = I2C_ReadBuffer(I2Cn2, EEPROM_MEM_RD,offsetLocation, EEPROM_REG_SIZE, destAddr, no_of_bytes );
#else
    status = I2C_ReadBuffer(I2Cn4, EEPROM_MEM_RD,offsetLocation, EEPROM_REG_SIZE, destAddr, no_of_bytes, DEF_NON_BLOCKING );
#endif

    if (status == HAL_OK)
    {
        no_of_bytes = 0u;
    }

    return (status == HAL_OK) ? (bool)true : (bool)false;
}

/**
* @brief   Function writes data from memory buffer of srcAddr to offsetLocation in EEPROM where
*          length of data transfer is no_of_bytes.The offset location is calculated from 0.
* @param   srcAddr pointer to location to data that needs to be written to the EEPROM.
* @param   offsetLocation Offset from memory start location in EEPROM.
* @param   no_of_bytes Length of data to be written.
* @return  EEPROM write status
*/
bool eepromWrite(uint8_t *srcAddr, uint16_t offsetLocation, uint32_t no_of_bytes)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t length;

    while ((no_of_bytes > 0u) && (status == HAL_OK))
    {
        //work out length of data for page writing to EEPROM
        length = MAX_EEPROM_PG_WR - ((uint32_t)offsetLocation & (MAX_EEPROM_PG_WR - 1u));

        if (length > no_of_bytes)
        {
            length = no_of_bytes;
        }

#ifdef NO_HARDWARE_AVAILABLE
        status = I2C_WriteBuffer(I2Cn2, EEPROM_MEM_WR, offsetLocation, EEPROM_REG_SIZE, srcAddr, (uint16_t)length );
#else
        status = I2C_WriteBuffer(I2Cn4, EEPROM_MEM_WR, offsetLocation, EEPROM_REG_SIZE, srcAddr, (uint16_t)length, DEF_NON_BLOCKING );
#endif
        if (status == HAL_OK)
        {
            //adjust the offset and number of bytes remaining
            srcAddr += length;
            no_of_bytes -= length;
            offsetLocation += (uint16_t)length;
        }

        //allow 5 ms for write to complete (specified in data sheet for the M24C64 device)
        HAL_Delay(5u);
    }

    return (status == HAL_OK) ? (bool)true : (bool)false;
}

/**
* @brief    EEPROM test function to write a sequence of values and read them.
* @param    void
* @return   test status, true = success, false = failed
*/
bool eepromTest(void)
{
    bool status = true;
    bool testStatus = false;
    uint8_t reg[32] = {0};
    uint8_t reg_eeprom[32] = {0};
    uint16_t checksumW = 0u;
    uint16_t checksumR = 0u;

    /* Generate pattern to write to EEPROM and calculate checksum*/
    for(uint8_t itr = 0u; itr < 32u; itr++)
    {
        reg[itr]  = (itr % 2u);
        checksumW += reg[itr];
    }

    /* Read existing data from EEPROM */
    status = eepromRead(reg_eeprom, 64u, MAX_EEPROM_PG_WR);

    if (status == true)
    {
        /* write the generated pattern to EEPROM */
        status = eepromWrite(reg, 64u, MAX_EEPROM_PG_WR);

        if (status == true)
        {
            for (uint8_t itr = 0u; itr < 32u; itr++)
            {
                reg[itr] = 0u;
            }

            status = eepromRead(reg, 64u, MAX_EEPROM_PG_WR);

            if (status == true)
            {
                for (uint8_t itr = 0u; itr < 32u; itr++)
                {
                    checksumR += reg[itr];
                }

                if (checksumR == checksumW)
                {
                    testStatus = true;
                }
            }
        }
    }

    /* Restore EEPROM Data */
    eepromWrite(reg_eeprom, 64u, MAX_EEPROM_PG_WR);

    return testStatus;
}

/*********************************************** End Of Functions****************************************************/
