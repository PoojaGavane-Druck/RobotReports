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
* @file     DSlot.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlot class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <rtos.h>
MISRAC_ENABLE

#include "DBattery.h"
#include "memory.h"
#include "smbus.h"
#include "DLock.h"
#include "i2c.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define BATTERY_DEV_ADDRESS 0x02u
/* Macros -----------------------------------------------------------------------------------------------------------*/
//extern I2C_HandleTypeDef hi2c2;
/* Variables --------------------------------------------------------------------------------------------------------*/
//I2C_HandleTypeDef hi2c2 = I2cHandle[I2Cn4];
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlot class constructor
 * @note    An owner is needed if sensor events are to be sent back. This is expected to be a DFunction instance.
 * @param   owner: the task that created this slot
 * @retval  void
 */
DBattery::DBattery()

{
    OS_ERR osError;

    //create mutex for resource locking
    char *name = "Battery";
    RTOSMutexCreate(&myMutex, (CPU_CHAR *)name, &osError);
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 Error[Pm100]: dynamic heap memory allocation shall not be used (MISRA C 2004 rule 20.4)
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm148")

eBatteryError_t DBattery::readParam(uint8_t cmdCode, uint16_t *value)
{
    eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
    HAL_StatusTypeDef halStatus = HAL_ERROR;
    uint8_t rxBuf[3];
    uint16_t data = (uint16_t)0;
    uint8_t crc8 = 0u;
    uint8_t length = 2u;

    halStatus = SMBUS_I2C_ReadBuffer((eI2CElement_t) I2Cn1, BATTERY_DEV_ADDRESS, cmdCode, (uint8_t *)&rxBuf[0], length);

    if(halStatus == (HAL_StatusTypeDef)HAL_OK)
    {
        batteryErr = calculateCRC(&rxBuf[0], length, &crc8);

        if(crc8 == rxBuf[length])
        {
            data = (uint16_t)rxBuf[0];
            data <<= 8u;
            data |= (uint16_t)rxBuf[1];
            *value = data;
            batteryErr = E_BATTERY_ERROR_NONE;
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

    return batteryErr;
}
/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 Error[Pm100]: dynamic heap memory allocation shall not be used (MISRA C 2004 rule 20.4)
 **********************************************************************************************************************/
_Pragma("diag_default=Pm148")

eBatteryError_t DBattery::writeParam(uint8_t cmdCode, uint16_t value)
{
    eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
    return batteryErr;
}


eBatteryError_t DBattery::readBatteryInfo(void)
{
    uint16_t paramValue = (uint16_t)0;
    eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
    batteryErr = readBatteryParams();

    if(E_BATTERY_ERROR_NONE == batteryErr)
    {
        batteryErr = readParam(E_BATTERY_CMD_REMAINING_CAPACITY_ALARM,
                               &paramValue);

        if(E_BATTERY_ERROR_NONE == batteryErr)
        {
            remainingCapacityAlarm = paramValue;
            batteryErr = readParam(E_BATTERY_CMD_REMAINING_TIME_ALARM,
                                   &paramValue);

            if(E_BATTERY_ERROR_NONE == batteryErr)
            {
                remainingTimeAlarm = paramValue;
                batteryErr = readParam(E_BATTERY_CMD_SPECIFICATION_INFO,
                                       &paramValue);

                if(E_BATTERY_ERROR_NONE == batteryErr)
                {
                    specificationInfo = paramValue;
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
        else
        {
            /* Do Nothing. Added for Misra*/
        }
    }
    else
    {
        /* Do Nothing. Added for Misra*/
    }

    return batteryErr;
}


eBatteryError_t DBattery::readBatteryParams(void)
{
    uint16_t paramValue = (uint16_t)0;
    float32_t tempValue = 0.0f;
    eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;

    batteryErr = readParam(EVAL_INDEX_BATTERY_SERIAL_NUMBER,
                           &paramValue);

    if(E_BATTERY_ERROR_NONE == batteryErr)
    {
        serialNumber = paramValue;

        batteryErr = readParam(E_BATTERY_CMD_VOLTAGE,
                               &paramValue);

        if(E_BATTERY_ERROR_NONE == batteryErr)
        {
            tempValue = (float32_t)(paramValue);
            myVoltage = tempValue / 1000.0f;
            batteryErr = readParam(E_BATTERY_CMD_RELATIVE_STATE_OF_CHARGE,
                                   &paramValue);

            if(E_BATTERY_ERROR_NONE == batteryErr)
            {
                relativeStateOfCharge = paramValue;
                batteryErr = readParam(E_BATTERY_CMD_ABSOLUTE_STATE_OF_CHARGE,
                                       &paramValue);

                if(E_BATTERY_ERROR_NONE == batteryErr)
                {
                    absoluteStateOfCharge = paramValue;
                    batteryErr = readParam(E_BATTERY_CMD_REMAINING_CAPACITY,
                                           &paramValue);

                    if(E_BATTERY_ERROR_NONE == batteryErr)
                    {
                        tempValue = (float32_t)(paramValue);
                        remainingCapacity = tempValue / 1000.0f;
                        batteryErr = readParam(E_BATTERY_CMD_FULL_CHARGE_CAPACITY,
                                               &paramValue);

                        if(E_BATTERY_ERROR_NONE == batteryErr)
                        {
                            tempValue = (float32_t)(paramValue);
                            fullChargeCapacity = tempValue / 1000.0f;
                            batteryErr = readParam(E_BATTERY_CMD_RUN_TIME_TO_EMPTY,
                                                   &paramValue);

                            if(E_BATTERY_ERROR_NONE == batteryErr)
                            {
                                remainingBatteryLife = paramValue;
                                batteryErr = readParam(E_BATTERY_CMD_AVERAGE_TIME_TO_EMPTY,
                                                       &paramValue);

                                if(E_BATTERY_ERROR_NONE == batteryErr)
                                {
                                    averageTimeToEmpty = paramValue;
                                    batteryErr = readParam(E_BATTERY_CMD_AVERAGE_TIME_TO_FULL,
                                                           &paramValue);

                                    if(E_BATTERY_ERROR_NONE == batteryErr)
                                    {
                                        averageTimeToFull = paramValue;
                                        batteryErr = readParam(E_BATTERY_CMD_BATTERY_STATUS,
                                                               &paramValue);

                                        if(E_BATTERY_ERROR_NONE == batteryErr)
                                        {
                                            batteryStatus = paramValue;
                                            batteryErr = readParam(E_BATTERY_CMD_BATTERY_MODE,
                                                                   &paramValue);

                                            if(E_BATTERY_ERROR_NONE == batteryErr)
                                            {
                                                batteryMode = paramValue;
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

    return batteryErr;
}
/**
 * @brief   Run DBattery task funtion
 * @param   void
 * @retval  void
 */

bool DBattery::getValue(eValueIndex_t index, float32_t *value)   //get specified floating point function value
{
    bool successFlag = false;

    switch(index)
    {
    case EVAL_INDEX_BATTERY_TEMPERATURE:
        *value = internalTemperature;
        break;

    case EVAL_INDEX_BATTERY_VOLTAGE:
        *value = myVoltage;
        break;

    case EVAL_INDEX_BATTERY_CURRENT:
        *value = myCurrent;
        break;

    case EVAL_INDEX_DESIRED_CHARGING_CURRENT:
        *value = desiredChargingCurrent;
        break;

    case EVAL_INDEX_DESIRED_CHARGING_VOLTAGE:
        *value = desiredChargingVoltage;
        break;

    case EVAL_INDEX_REMAINING_BATTERY_CAPACITY: //RemainingCapacity
        *value = remainingCapacity;
        break;

    case EVAL_INDEX_REMAINING_BATTERY_CAPACITY_WHEN_FULLY_CHARGED: //RemainingCapacity when fully charged
        *value = fullChargeCapacity;
        break;

    case E_VAL_INDEX_BATTERY_RELATIVE_STATE_OF_CHARGE:
        *value = (float32_t) relativeStateOfCharge;
        break;



    default:
        successFlag = false;
        break;
    }

    return successFlag;
}

bool DBattery::getValue(eValueIndex_t index, uint32_t *value)    //get specified integer function value
{
    bool successFlag = false;
    DLock is_on(&myMutex);
    successFlag = true;

    switch(index)
    {
    case EVAL_INDEX_REMAINING_BATTERY_PERCENTAGE: //RelativeStateOfCharge
        *value = relativeStateOfCharge;
        break;

    case EVAL_INDEX_REMAINING_BATTERY_LIFE://RunTimeToEmpty
        *value = remainingBatteryLife;
        break;

    case EVAL_INDEX_TIME_REQUIRED_FOR_FULL_CHARGE://AverageTimeToFull
        *value = averageTimeToFull;
        break;


    case EVAL_INDEX_BATTERY_STATUS_INFO:
        *value = batteryStatus;
        break;

    case EVAL_INDEX_CHARGE_DISCHARGE_CYCLE_COUNT:
        *value = cycleCount;
        break;

    case EVAL_INDEX_BATTERY_SERIAL_NUMBER:
        *value = serialNumber;
        break;

    case E_VAL_INDEX_BATTERY_TIME_TO_EMPTY:
        *value =  remainingBatteryLife;
        break;

    default:
        successFlag = false;
        break;
    }


    return successFlag;
}
/**
 * @brief   CRC calculation function
 * @param   void
 * @retval  void
 */
//uint8_t calculateCRC(uint8_t* data, uint8_t len)
eBatteryError_t DBattery::calculateCRC(uint8_t *data, uint8_t len, uint8_t *crc)
{
    eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
    uint8_t crctable[] = {0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
                          0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
                          0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
                          0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
                          0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
                          0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
                          0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
                          0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
                          0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
                          0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
                          0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
                          0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
                          0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
                          0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
                          0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
                          0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
                         };
    *crc = 0u;
    uint8_t *pData  = (uint8_t *)NULL;

    if((data != NULL) && (len > 0u))
    {
        *pData = *data;

        while(0u != len)
        {
            /* XOR-in next input byte */
            uint8_t index = (*data ^ *crc);
            /* get current CRC value = remainder */
            *crc = crctable[index];
            data++;
            len--;
            batteryErr = E_BATTERY_ERROR_NONE;
        }
    }

    else
    {
        /* Do Nothing. Added for Misra*/
    }

    return batteryErr;
}