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
#include "DPowerManager.h"
#include "DErrorhandler.h"
#include "DBattery.h"
#include "memory.h"
#include "smbus.h"
#include "DLock.h"
#include "DPV624.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define BATTERY_DEV_ADDRESS 0x0Bu
#define BATTERY_POLLING_INTERVAL (uint32_t)20
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
DPowerManager::DPowerManager()
: DTask()
{
    OS_ERR osError;


    //create mutex for resource locking
    char *name = "PowerManager";
    battery = new DBattery();
    //specify the flags that this function must respond to (add more as necessary in derived class)
    timeElapsedFromLastBatteryRead = (uint32_t)0;
    myWaitFlags = EV_FLAG_TASK_SHUTDOWN;
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
    
 
}


void DPowerManager::initialise(void)
{
 
  eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
  error_code_t errorCode;
  errorCode.bit.smBusbatteryComFailed = SET;
  PV624->errorHandler->clearError(errorCode);
  batteryErr = battery->readBatteryInfo();
  
  if(E_BATTERY_ERROR_NONE == batteryErr)
  {   
    batteryErr = battery->readBatteryParams();
    if(E_BATTERY_ERROR_NONE != batteryErr)
    {
      //ToDO: Set Error Flag
    }
    else
    {      
      PV624->errorHandler->handleError(errorCode);
    }
  }
  else
  {
      //ToDo: Set Error Flag
  }
}
void DPowerManager::monitorBatteryParams(void)
{
  
}
/**
 * @brief   Run DBattery task funtion
 * @param   void
 * @retval  void
 */
void DPowerManager::runFunction(void)
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
                                    myWaitFlags, (OS_TICK)500u, //runs, nominally, at 2Hz by default
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
        //check for events
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
            timeElapsedFromLastBatteryRead++;
            
            if(BATTERY_POLLING_INTERVAL <= timeElapsedFromLastBatteryRead)
            {
              batteryErr = battery->readBatteryParams();
              if(E_BATTERY_ERROR_NONE == batteryErr)
              {
                  monitorBatteryParams();
              }
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
void DPowerManager::cleanUp(void)
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

bool DPowerManager::getValue(eValueIndex_t index, float32_t *value)   //get specified floating point function value    
{
  bool successFlag = false;
   switch(index)
  {
    case EVAL_INDEX_BATTERY_TEMPERATURE: 
    case EVAL_INDEX_BATTERY_VOLTAGE:
    case EVAL_INDEX_BATTERY_CURRENT:    
    case EVAL_INDEX_DESIRED_CHARGING_CURRENT:    
    case EVAL_INDEX_DESIRED_CHARGING_VOLTAGE:    
    case EVAL_INDEX_REMAINING_BATTERY_CAPACITY: //RemainingCapacity    
    case EVAL_INDEX_REMAINING_BATTERY_CAPACITY_WHEN_FULLY_CHARGED: //RemainingCapacity()
      successFlag = battery->getValue(index,value);
      break;
    default:
      successFlag = false;
    break;
  }
  return successFlag;
}

bool DPowerManager::getValue(eValueIndex_t index, uint32_t *value)    //get specified integer function value
{
  bool successFlag = false;
  DLock is_on(&myMutex);
  successFlag = true;
  switch(index)
  {
    case EVAL_INDEX_REMAINING_BATTERY_PERCENTAGE: //RelativeStateOfCharge   
    case EVAL_INDEX_REMAINING_BATTERY_LIFE://RunTimeToEmpty    
    case EVAL_INDEX_TIME_REQUIRED_FOR_FULL_CHARGE://AverageTimeToFull 
    case EVAL_INDEX_BATTERY_STATUS_INFO:  
    case EVAL_INDEX_CHARGE_DISCHARGE_CYCLE_COUNT:    
    case EVAL_INDEX_BATTERY_SERIAL_NUMBER:
      successFlag = battery->getValue(index,value);
      break;
    default:
      successFlag = false;
    break;
  }
    
    
  return successFlag;
}