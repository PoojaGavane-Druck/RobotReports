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

/* Typedefs ------------------------------------------------------------------*/

/* Defines -------------------------------------------------------------------*/
#define BATTERY_POLLING_INTERVAL 20

/* Macros --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
OS_ERR pSmbusErr;
OS_FLAG_GRP smbusErrFlagGroup;
OS_FLAGS smbusErrFlag;
OS_FLAGS smbusErrPendFlags;

/* Prototypes ----------------------------------------------------------------*/

/* User code -----------------------------------------------------------------*/
/**
 * @brief   DSlot class constructor
 * @note    An owner is needed if sensor events are to be sent back. This is expected to be a DFunction instance.
 * @param   owner: the task that created this slot
 * @retval  void
 */
DPowerManager::DPowerManager(SMBUS_HandleTypeDef *smbus)
: DTask()
{
    OS_ERR osError;

    // Create mutex for resource locking
    char *name = "PowerManager";
    
    /* Create objects required by task */    
    ltc4100 = new LTC4100(smbus);
    battery = new smartBattery(smbus);
    voltageMonitor = new DVoltageMonitor();
    chargingStatus = (uint32_t)(0);

    /* Read the full capacity of the battery */
    battery->getValue(eFullChargeCapacity, &fullCapacity);
    handleChargerAlert();

    /* Init class veriables */
    timeElapsed = (uint32_t)(0);
    
    // Specify the flags that this function must respond to
    myWaitFlags = EV_FLAG_TASK_SHUTDOWN | 
                  EV_FLAG_TASK_UPDATE_BATTERY_STATUS | 
                  EV_FLAG_TASK_BATT_CHARGER_ALERT;
    
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &osError);

    if (osError != (OS_ERR)OS_ERR_NONE)
    {
        //Error handler?        
    }
    
    // Get stack area from the memory partition memory block for function tasks
    myTaskStack = (CPU_STK*)OSMemGet((OS_MEM*)&memPartition, (OS_ERR*)&osError);   
    
    if (osError == (OS_ERR)OS_ERR_NONE)
    {
        
        // Memory block from the partition obtained, so can go ahead and run
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
    errorCode.bit.smBusBatteryComFailed = SET;
    //PV624->errorHandler->clearError(errorCode);
    
    if(E_BATTERY_ERROR_NONE == batteryErr)
    {   
        if(E_BATTERY_ERROR_NONE != batteryErr)
        {
            //ToDO: Set Error Flag
        }
        else
        {      
            //PV624->errorHandler->handleError(errorCode);
        }
    }
    else
    {      
      	PV624->errorHandler->handleError(errorCode);
    }
}

/**
 * @brief   Run DBattery task funtion
 * @param   void
 * @retval  void
 */
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
    
    uint32_t terminateCharging = (uint32_t)(0);
    uint32_t fullyChargedStatus = (uint32_t)(0);

    uint32_t status = (uint32_t)(0);
    eBatteryError_t batteryErr = E_BATTERY_ERROR_HAL;
    
    while (runFlag == true)
    {
        actualEvents = OSFlagPend(&myEventFlags,
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
            //PV624->errorHandler->handleError(errorCode, os_error);
        }
        //check for events
        if (ok)
        {       
            if (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT))
            {
                timeElapsed++;
                updateBatteryLeds();
                battery->getTerminateChargeAlarm(&terminateCharging);
                battery->getFullyChargedStatus(&fullyChargedStatus);

                if(((uint32_t)(1) == fullyChargedStatus) || 
                                ((uint32_t)(1) == terminateCharging))
                {
                    if(eBatteryCharging == chargingStatus)
                    {
                        if(((uint32_t)(1) == fullyChargedStatus) || 
                                    ((uint32_t)(1) == terminateCharging))
                        {
                            chargingStatus = eBatteryDischarging;
                            ltc4100->stopCharging();
                        }
                    }                    
                }
                else
                {
                    handleChargerAlert();
                }
                        
                if((uint32_t)(BATTERY_POLLING_INTERVAL) <= timeElapsed)
                {
                    timeElapsed = (uint32_t)(0);
                    battery->getAllParameters();
                }
            }         
            else
            {
                if((actualEvents & EV_FLAG_TASK_BATT_CHARGER_ALERT) == EV_FLAG_TASK_BATT_CHARGER_ALERT)
                {
                    handleChargerAlert();
                }
                else
                {
                    /* For misra */
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

/**
 * @brief   Updates the battery percentage on 5 LEDs
 * @param   void
 * @return  void
 */
void DPowerManager::updateBatteryLeds(void)
{
    uint32_t remCapacity = (uint32_t)(0);
    uint32_t fullCapacity = (uint32_t)(0);
    float percentCap = (float)(0);

    battery->getValue(eRemainingCapacity, &remCapacity);
    battery->getValue(eFullChargeCapacity, &fullCapacity);

    percentCap = (float)(remCapacity) * float(100) / (float)(fullCapacity);

    PV624->leds->updateBatteryLeds(percentCap, chargingStatus);     
}

/**
 * @brief   Handles a charger alert condition
 * @param   void
 * @return  void
 */
void DPowerManager::handleChargerAlert(void)
{
    uint32_t status = (uint32_t)(0);
    uint32_t batteryStatus = (uint32_t)(0);
    uint32_t acStatus = (uint32_t)(0);
    uint32_t capacity = (uint32_t)(0);

    /* A charger alert condition has occured 
    1. Read the charger status
    2. Interpret the changes in status
    3. If AC is present
        a. Read the battery charge
        b. Start charging if battery is not fully charged
    4. If AC is absent
        a. Inhibit charging by turning off charger and CHGEN pin
    5. If battery is absent, generate battery error
    */
    ltc4100->getChargerStatus(&status);
    ltc4100->getIsBatteryPresent(&batteryStatus);
    ltc4100->getIsAcPresent(&acStatus);

    if((uint32_t)(BATTERY_PRESENT) == batteryStatus)
    {
        if((uint32_t)(AC_PRESENT) == acStatus)
        {
            /* Both AC and battery are present 
            So, read the battery percentage */
            battery->getRemainingCapacity(&capacity);
            if(capacity < fullCapacity)
            {
                /* Current capacity is less than full so start charging */
                chargingStatus = eBatteryCharging;
                startCharging();
            }
            else
            {
                /* Current capacity is equal to or more than full so start charging */
                chargingStatus = eBatteryDischarging;
                stopCharging();
            }
        }
        else
        {
            /* Current capacity is equal to or more than full so start charging */
            chargingStatus = eBatteryDischarging;
            stopCharging();
        }
    }
}

/**
 * @brief   Starts battery charging
 * @param   void
 * @return  void
 */
void DPowerManager::startCharging(void)
{
    ltc4100->startCharging();
}

/**
 * @brief   Stops battery charging
 * @param   void
 * @return  void
 */
void DPowerManager::stopCharging(void)
{
    ltc4100->stopCharging();
}

/**
 * @brief   Reads different values as a floating point number
 * @param   void
 * @return  void
 */
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
            break;
        case EVAL_INDEX_BATTERY_5VOLT_VALUE:
            successFlag = voltageMonitor->getVoltage(eVoltageLevelFiveVolts, value);
            break;
        case EVAL_INDEX_BATTERY_6VOLT_VALUE:
            successFlag = voltageMonitor->getVoltage(eVoltageLevelSixVolts, value);
            break;
        case EVAL_INDEX_BATTERY_24VOLT_VALUE:
            successFlag = voltageMonitor->getVoltage(eVoltageLevelTwentyFourVolts, value);
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
    VOLTAGE_STATUS_t status;
    switch(index)
    {
        case EVAL_INDEX_REMAINING_BATTERY_PERCENTAGE: //RelativeStateOfCharge   
        case EVAL_INDEX_REMAINING_BATTERY_LIFE://RunTimeToEmpty    
        case EVAL_INDEX_TIME_REQUIRED_FOR_FULL_CHARGE://AverageTimeToFull 
        case EVAL_INDEX_BATTERY_STATUS_INFO:  
        case EVAL_INDEX_CHARGE_DISCHARGE_CYCLE_COUNT:    
        case EVAL_INDEX_BATTERY_SERIAL_NUMBER:
            break;
          
        case EVAL_INDEX_BATTERY_5VOLT_STATUS:
            successFlag = voltageMonitor->getVoltageStatus(eVoltageLevelFiveVolts,(VOLTAGE_STATUS_t*)&status);
            if((VOLTAGE_STATUS_t)eVoltageStatusOK == status)
            {
              *value = 1u;
            }
            else if((VOLTAGE_STATUS_t)eVoltageStatusNotOK == status)
            {
              *value = 0u;
            }
            else
            {
              *value = 0u;
            }
            break;
          
        case EVAL_INDEX_BATTERY_6VOLT_STATUS:
            successFlag = voltageMonitor->getVoltageStatus(eVoltageLevelSixVolts,(VOLTAGE_STATUS_t*)&status);
            if((VOLTAGE_STATUS_t)eVoltageStatusOK == status)
            {
              *value = 1u;
            }
            else if((VOLTAGE_STATUS_t)eVoltageStatusNotOK == status)
            {
              *value = 0u;
            }
            else
            {
              *value = 0u;
            }
            break;
          
        case EVAL_INDEX_BATTERY_24VOLT_STATUS:
            successFlag = voltageMonitor->getVoltageStatus(eVoltageLevelTwentyFourVolts,(VOLTAGE_STATUS_t*)&status);
            if((VOLTAGE_STATUS_t)eVoltageStatusOK == status)
            {
              *value = 1u;
            }
            else if((VOLTAGE_STATUS_t)eVoltageStatusNotOK == status)
            {
              *value = 0u;
            }
            else
            {
              *value = 0u;
            }
            break;
          
        case EVAL_INDEX_IR_SENSOR_ADC_COUNTS:
            successFlag = voltageMonitor->getAdcCounts(eVoltageLevelNone,value);    
            break;
            
        default:
            successFlag = false;
        break;
    }

    return successFlag;
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

/**
  * @brief  Update battery status event generation.
  * @param  None.
  * @retval None
  */
void DPowerManager::updateBatteryStatus(void)
{
    postEvent(EV_FLAG_TASK_UPDATE_BATTERY_STATUS);
}

/**
  * @brief  SMBUS ERROR callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
void HAL_SMBUS_ErrorCallback(SMBUS_HandleTypeDef *hsmbus)
{
    if(hsmbus->Instance == I2C1)
    {
        PV624->powerManager->postEvent(EV_FLAG_TASK_BATT_CHARGER_ALERT);    
    }
}
