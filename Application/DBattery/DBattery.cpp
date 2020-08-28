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
: DTask()
{
    OS_ERR osError;


    //create mutex for resource locking
    char *name = "Battery";
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &osError);

    if (osError != (OS_ERR)OS_ERR_NONE)
    {
        //Error handler?
    }
    
     //get stack area from the memory partition memory block for function tasks
    myTaskStack = (CPU_STK*)OSMemGet((OS_MEM*)&memPartition, (OS_ERR*)&osError);   
    
    if (osError == (OS_ERR)OS_ERR_NONE)
    {
        
        //memory block from the partition obtained, so can go ahead and run
        activate(myName, (CPU_STK_SIZE)MEM_PARTITION_BLK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)10u, &osError);
        
    }
    else
    {
        //report error
    }
    //specify the flags that this function must respond to (add more as necessary in derived class)
    myWaitFlags = EV_FLAG_TASK_SHUTDOWN;
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
  eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
  batteryErr = readParam(E_BATTERY_CMD_VOLTAGE,
                         &paramValue);
  if(E_BATTERY_ERROR_NONE == batteryErr)
  {
    myVoltage = paramValue;   
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
                 remainingCapacity = paramValue; 
                 batteryErr = readParam(E_BATTERY_CMD_FULL_CHARGE_CAPACITY,
                                        &paramValue); 
                if(E_BATTERY_ERROR_NONE == batteryErr)
                {
                    fullChargeCapacity = paramValue;
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
void DBattery::runFunction(void)
{
    //this is a while loop that pends on event flags
    bool runFlag = true;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
    
   eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
    while (runFlag == true)
    {
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)10000u, //runs, nominally, at 2Hz by default
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
        //check for events
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
            batteryErr = readBatteryParams();
            if(E_BATTERY_ERROR_NONE == batteryErr)
            {
              
            }
            
        }
        else if (os_error != (OS_ERR)OS_ERR_NONE)
        {
            //os error
            batteryErr = E_BATTERY_ERROR_OS;
        }
        else
        {
            
        }

 
    }

    
}


/**
 * @brief   Clean up after termination of task
 * @param   void
 * @return  void
 */
void DBattery::cleanUp(void)
{
    OS_ERR err;

    //if a stack was allocated then free that memory to the partition
    if (myTaskStack != NULL)
    {
        //Return the stack memory block back to the partition
        OSMemPut((OS_MEM*)&memPartition, (void*)myTaskStack, (OS_ERR*)&err);

        if (err == (OS_ERR)OS_ERR_NONE)
        {
            //memory block from the partition obtained
        }

        myTaskStack = NULL;
    }
}

