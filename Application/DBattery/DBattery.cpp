/**
* BHGE Confidential
* Copyright 2020. Baker Hughes.
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
#include <os.h>
MISRAC_ENABLE

#include "DBattery.h"
#include "memory.h"
#include "smbus.h"
#include "DLock.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define BATTERY_DEV_ADDRESS 0x0Bu
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

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
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &osError);

    if (osError != (OS_ERR)OS_ERR_NONE)
    {
        //Error handler?
    }
    
   
}





eBatteryError_t DBattery::readParam(uint8_t cmdCode, uint16_t *value)
{
   
    
    eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
    HAL_StatusTypeDef halStatus = HAL_ERROR;
    uint8_t txBuf = cmdCode;
    uint16_t rxBuf = (uint16_t)0;
    halStatus = SMBUS_Transmit((uint8_t)BATTERY_DEV_ADDRESS, 
                               &txBuf,
                               (uint16_t) 1,
                               (uint32_t)SMBUS_FIRST_AND_LAST_FRAME_WITH_PEC);
    if ((uint8_t)halStatus == (uint8_t)HAL_OK)
    {
        halStatus = SMBUS_Receive((uint8_t)BATTERY_DEV_ADDRESS,
                                     (uint8_t *)&rxBuf,
                                     (uint16_t) 2,
                                     (uint32_t)SMBUS_FIRST_AND_LAST_FRAME_WITH_PEC);
        if ((uint8_t)halStatus == (uint8_t)HAL_OK)
        {
            *value = rxBuf;
            batteryErr = E_BATTERY_ERROR_NONE;
        }
    }
    return batteryErr;
}


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
  batteryErr = readParam(E_BATTERY_CMD_VOLTAGE,
                         &paramValue);
  if(E_BATTERY_ERROR_NONE == batteryErr)
  {
    tempValue = (float32_t)(paramValue);
    myVoltage = tempValue/1000.0f;
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
                 remainingCapacity = tempValue/1000.0f;
                 batteryErr = readParam(E_BATTERY_CMD_FULL_CHARGE_CAPACITY,
                                        &paramValue); 
                if(E_BATTERY_ERROR_NONE == batteryErr)
                {
                    tempValue = (float32_t)(paramValue);
                    fullChargeCapacity = tempValue/1000.0f;
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
    
    case EVAL_INDEX_REMAINING_BATTERY_CAPACITY_WHEN_FULLY_CHARGED: //RemainingCapacity()
      *value = fullChargeCapacity;
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
    
    default:
      successFlag = false;
    break;
  }
    
    
  return successFlag;
}