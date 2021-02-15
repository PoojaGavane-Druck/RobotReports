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
#include "main.h"
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
    myWaitFlags = EV_FLAG_TASK_SHUTDOWN | EV_FLAG_TASK_UPDATE_BATTERY_STATUS;
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
        bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT));

        if(!ok)
        {
    MISRAC_DISABLE
#ifdef ASSERT_ENABLED
            assert(false);
#endif
    MISRAC_ENABLE
            error_code_t errorCode;
            errorCode.bit.osError = SET;
            PV624->errorHandler->handleError(errorCode, os_error);
        }
         //check for events
        if (ok)
        {       
          if (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT))
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
          else
          {
              if ((actualEvents & EV_FLAG_TASK_UPDATE_BATTERY_STATUS) == EV_FLAG_TASK_UPDATE_BATTERY_STATUS)
              {
                UpdateBatteryStatusOnLEDs();
              }
          }
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

void DPowerManager:: UpdateBatteryStatusOnLEDs()
{
  eBatteryLevel_t batteryLevel;
  batteryLevel =  CheckBatteryLevel();
#ifndef BATTERY_AVILABLE
   static eBatteryLevel_t Level = BATTERY_LEVEL_0_TO_10;
   if(Level >= BATTERY_LEVEL_70_TO_100)
   {
     Level = BATTERY_LEVEL_0_TO_10;
   }
   else
   {
     Level = (eBatteryLevel_t)(Level +( eBatteryLevel_t)1);
   }
   batteryLevel = Level;
  
#endif
  switch (batteryLevel)
  {
    case BATTERY_LEVEL_0_TO_10:
            //1st led Red, remaining off
            HAL_GPIO_WritePin(BAT_LEVEL1_PF2_GPIO_Port, BAT_LEVEL1_PF2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BAT_LEVEL2_PF4_GPIO_Port, BAT_LEVEL2_PF4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL3_PF5_GPIO_Port, BAT_LEVEL3_PF5_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL4_PD10_GPIO_Port, BAT_LEVEL4_PD10_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL5_PD8_GPIO_Port, BAT_LEVEL5_PD8_Pin, GPIO_PIN_RESET);
            break;
    case BATTERY_LEVEL_10_TO_20:
            //2nd LED yellow, rest off	              
            HAL_GPIO_WritePin(BAT_LEVEL1_PF2_GPIO_Port, BAT_LEVEL1_PF2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL2_PF4_GPIO_Port, BAT_LEVEL2_PF4_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BAT_LEVEL3_PF5_GPIO_Port, BAT_LEVEL3_PF5_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL4_PD10_GPIO_Port, BAT_LEVEL4_PD10_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL5_PD8_GPIO_Port, BAT_LEVEL5_PD8_Pin, GPIO_PIN_RESET);
            break;
    case BATTERY_LEVEL_20_TO_45:
            //3rd led Green, remaining off                
            HAL_GPIO_WritePin(BAT_LEVEL1_PF2_GPIO_Port, BAT_LEVEL1_PF2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL2_PF4_GPIO_Port, BAT_LEVEL2_PF4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL3_PF5_GPIO_Port, BAT_LEVEL3_PF5_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BAT_LEVEL4_PD10_GPIO_Port, BAT_LEVEL4_PD10_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL5_PD8_GPIO_Port, BAT_LEVEL5_PD8_Pin, GPIO_PIN_RESET);
            break;
    case BATTERY_LEVEL_45_TO_70:
            // 3rd and 4th leds green, remaining off                
           HAL_GPIO_WritePin(BAT_LEVEL1_PF2_GPIO_Port, BAT_LEVEL1_PF2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL2_PF4_GPIO_Port, BAT_LEVEL2_PF4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL3_PF5_GPIO_Port, BAT_LEVEL3_PF5_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BAT_LEVEL4_PD10_GPIO_Port, BAT_LEVEL4_PD10_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BAT_LEVEL5_PD8_GPIO_Port, BAT_LEVEL5_PD8_Pin, GPIO_PIN_RESET);
            break;
    case BATTERY_LEVEL_70_TO_100:
            // 3rd, 4th and 5th leds green, remaining off           
            HAL_GPIO_WritePin(BAT_LEVEL1_PF2_GPIO_Port, BAT_LEVEL1_PF2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL2_PF4_GPIO_Port, BAT_LEVEL2_PF4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BAT_LEVEL3_PF5_GPIO_Port, BAT_LEVEL3_PF5_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BAT_LEVEL4_PD10_GPIO_Port, BAT_LEVEL4_PD10_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BAT_LEVEL5_PD8_GPIO_Port, BAT_LEVEL5_PD8_Pin, GPIO_PIN_SET);
            break;
    default:
            break;
  }
}

eBatteryLevel_t DPowerManager ::CheckBatteryLevel()
{
  eBatteryLevel_t val;
  float32_t remainingBatCapacity = 0.0f;
  bool status = getValue(EVAL_INDEX_REMAINING_BATTERY_CAPACITY, &remainingBatCapacity);

  if (remainingBatCapacity <= (float32_t)(10))
  {
      val = BATTERY_LEVEL_0_TO_10;
  }
  else if (remainingBatCapacity  <= (float32_t)(20))
  {
    val = BATTERY_LEVEL_10_TO_20;
  }
  else if ((float32_t)(20) < remainingBatCapacity <= (float32_t)(40))
  {
      val = BATTERY_LEVEL_20_TO_45;
  }
  else if ((float32_t)(40) < remainingBatCapacity <= (float32_t)(60))
  {
      val = BATTERY_LEVEL_45_TO_70;
  }
  else if ((float32_t)(60) < remainingBatCapacity <= (float32_t)(80))
  {
    val = BATTERY_LEVEL_70_TO_100;
  }
  else
  {
  //NOP
  }
  return val;
}

void DPowerManager ::LEDsTest(eLED_Num_t LED_Number, GPIO_PinState onOffState)
{
  switch(LED_Number)
    {
  case LED_1:
        HAL_GPIO_WritePin(BAT_LEVEL1_PF2_GPIO_Port, BAT_LEVEL1_PF2_Pin, onOffState);
      break;
   case LED_2:
        HAL_GPIO_WritePin(BAT_LEVEL2_PF4_GPIO_Port, BAT_LEVEL2_PF4_Pin, onOffState);
      break;
   case LED_3:
       HAL_GPIO_WritePin(BAT_LEVEL3_PF5_GPIO_Port, BAT_LEVEL3_PF5_Pin, onOffState);
      break;
   case LED_4:
        HAL_GPIO_WritePin(BAT_LEVEL4_PD10_GPIO_Port, BAT_LEVEL4_PD10_Pin, onOffState);
      break;
   case LED_5:
      HAL_GPIO_WritePin(BAT_LEVEL5_PD8_GPIO_Port, BAT_LEVEL5_PD8_Pin, onOffState);
      break;
   default:
      break;
    }
}


void DPowerManager ::updateBatteryStatus(void)
{
  postEvent(EV_FLAG_TASK_UPDATE_BATTERY_STATUS);
}
