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
* @file     DSlotExternal.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The DSlotExternal class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <os.h>
#include <memory.h>
MISRAC_ENABLE

#include "DSlotExternal.h"
#include "DSensorOwiAmc.h"
#include "Utilities.h"
#include "DSensorExternal.h"
#include "uart.h"
#include "leds.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define TEMPERATURE_POLLING_INTERVAL 20000

#define PM620_TIME_ADJUSTMENT 79 // ms
#define IS_PMTERPS 0x01

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;

uint32_t runOnce = 0u;
volatile uint32_t pmTime = (uint32_t)(0);

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlotExternal class constructor
 * @param   owner: the task that created this slot
 * @retval  void
 */
DSlotExternal::DSlotExternal(DTask *owner)
: DSlot(owner)
{
    myWaitFlags |= EV_FLAG_TASK_SENSOR_CONTINUE | EV_FLAG_TASK_SENSOR_RETRY | EV_FLAG_TASK_SLOT_SENSOR_CONTINUE | EV_FLAG_TASK_SENSOR_TAKE_NEW_READING;
}

/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */
void DSlotExternal::start(void)
{
    OS_ERR err;

    //get stack area from the memory partition memory block for function tasks
    myTaskStack = (CPU_STK*)OSMemGet((OS_MEM*)&memPartition, (OS_ERR*)&err);   
    
    if (err == (OS_ERR)OS_ERR_NONE)
    {
        
        //memory block from the partition obtained, so can go ahead and run
        activate(myName, (CPU_STK_SIZE)MEM_PARTITION_BLK_SIZE, (OS_PRIO)3u, (OS_MSG_QTY)10u, &err);
        
    }
    else
    {
        //report error
    }
}

/**
 * @brief   Run DSlotExternal task function
 * @param   void
 * @retval  void
 */
void DSlotExternal::runFunction(void)
{
    //this is a while loop that pends on event flags
    bool runFlag = true;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
    uint32_t failCount = (uint32_t)0; //used for retrying in the event of failure
    uint32_t timeElapsed = (uint32_t)0; 
    uint32_t channelSel = (uint32_t)0;
    uint32_t value = (uint32_t)(0);
    uint32_t sampleRate = (uint32_t)(0);
    eSensorError_t sensorError = mySensor->initialise();
    
 
    myState = E_SENSOR_STATUS_DISCOVERING;

    while (runFlag == true)
    {
        /* Sensor data acquisition is stopping after a certain amount of time
        The following code is changed to test it quickly */
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)200u, //runs, nominally, at 20Hz by default
                                    OS_OPT_PEND_BLOCKING | 
                                    OS_OPT_PEND_FLAG_SET_ANY | 
                                    OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);

        //check actions to execute routinely (ie, timed)
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
            switch(myState)
            {
                case E_SENSOR_STATUS_DISCOVERING:
                    //any sensor error will be mopped up below
                    // Add delay to allow sensor to be powered up and running    
                  
                    if(0u == runOnce)
                    {
                        ledBlink((uint32_t)(7));               
                        runOnce = 1u;
                    }
                      
                    sensorError = mySensorChecksumDisable();                  
                    
                    //HAL_Delay((uint16_t)(1000));
                    if(E_SENSOR_ERROR_NONE == sensorError)
                    {
                        sensorError = mySensorDiscover();
                    }                    
                    
                    if(E_SENSOR_ERROR_NONE == sensorError)
                    {
                        myState = E_SENSOR_STATUS_IDENTIFYING;
                        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
                        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
                    }
                    break;

                case E_SENSOR_STATUS_IDENTIFYING:                                        
                    sensorError = mySensorIdentify();

                    //if no sensor error than proceed as normal (errors will be mopped up below)
                    if (sensorError == E_SENSOR_ERROR_NONE)
                    {                                  
                        /* have one reading available from the sensor */
                        setValue(E_VAL_INDEX_SAMPLE_RATE, (uint32_t)E_ADC_SAMPLE_RATE_27_5_HZ);
                        channelSel = E_CHANNEL_0 | E_CHANNEL_1;
                        sensorError = mySensor->measure(channelSel); 
                        
                        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
                        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
                        //notify parent that we have connected, awaiting next action - this is to allow
                        //the higher level to decide what other initialisation/registration may be required
                        myOwner->postEvent(EV_FLAG_TASK_SENSOR_CONNECT);
                        resume();
                    }
                    break;

                case E_SENSOR_STATUS_RUNNING:     
                    // Always read both channels
                    channelSel = E_CHANNEL_0 | E_CHANNEL_1;
                    sensorError = mySensor->measure(channelSel); 
                    //if no sensor error than proceed as normal (errors will be mopped up below)
                    if (sensorError == E_SENSOR_ERROR_NONE)
                    {
                        myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);                        
                    }
                    break;

                default:
                    break;
            }

            //check if there is an issue
            if (sensorError != E_SENSOR_ERROR_NONE)
            {
                failCount++;

                if ((failCount > 3u) && (myState != E_SENSOR_STATUS_DISCONNECTED))
                {
                    //TODO: what error stattus to set????

                    myState = E_SENSOR_STATUS_DISCONNECTED;
                    sensorError = (eSensorError_t)(E_SENSOR_ERROR_NONE);
                    //notify parent that we have hit a problem and are awaiting next action from higher level functions
                    myOwner->postEvent(EV_FLAG_TASK_SENSOR_DISCONNECT);
                    timeElapsed = (uint32_t)0;
                }
            }
            else 
            {
                failCount = 0u;
            }
        }
        else if (os_error != (OS_ERR)OS_ERR_NONE)
        {
            //os error
            sensorError = E_SENSOR_ERROR_OS;
        }
        else if ((actualEvents & EV_FLAG_TASK_SHUTDOWN) == EV_FLAG_TASK_SHUTDOWN)
        {
            runFlag = false;
        }
        else if ((actualEvents & EV_FLAG_TASK_SENSOR_CONNECT) == EV_FLAG_TASK_SENSOR_CONNECT)
        {
            resume();
        }
        else
        {
            //check events that can occur at any time first
            switch (myState)
            {
                case E_SENSOR_STATUS_READY:
                    //waiting to be told to start running
                    if ((actualEvents & EV_FLAG_TASK_SLOT_SENSOR_CONTINUE) == EV_FLAG_TASK_SLOT_SENSOR_CONTINUE)
                    {
                        myState = E_SENSOR_STATUS_RUNNING;
                        timeElapsed = (uint32_t)0;
                    }
                    break;

                case E_SENSOR_STATUS_DISCONNECTED:
                    //disconnected or not responding
                    //waiting to be told to re-start (go again from the beginning)
                    if ((actualEvents & EV_FLAG_TASK_SENSOR_RETRY) == EV_FLAG_TASK_SENSOR_RETRY)
                    {
                        myState = E_SENSOR_STATUS_DISCOVERING;
                        //TODO: allow some delay before restarting, or rely on higher level to do that?
                        sleep(250u);
                    }
                    break;
                case E_SENSOR_STATUS_RUNNING:
                    if ((actualEvents & EV_FLAG_TASK_SENSOR_TAKE_NEW_READING) == EV_FLAG_TASK_SENSOR_TAKE_NEW_READING)
                    {
                        
                        mySensor->getValue(E_VAL_INDEX_SENSOR_TYPE, &value);
                        getValue(E_VAL_INDEX_SAMPLE_RATE, &sampleRate);
                        
                        if(value & (uint32_t)(IS_PMTERPS) == (uint32_t)(IS_PMTERPS))
                        {
                            pmTime = (uint32_t)(0);
                            //HAL_TIM_Base_Stop(&htim2);                        
                        }
                        else
                        {
                            if(sampleRate == (uint32_t)(E_ADC_SAMPLE_RATE_55_HZ))
                            {
                                pmTime = (uint32_t)(0);
                                //HAL_TIM_Base_Start_IT(&htim2);
                            }
                        }
                        /* Always check both bridge and diode */
                        channelSel = E_CHANNEL_0 | E_CHANNEL_1;
                        
                        sensorError = mySensor->measure(channelSel); 
                        //if no sensor error than proceed as normal (errors will be mopped up below)
                        if (sensorError == E_SENSOR_ERROR_NONE)
                        {          
                            if(value & (uint32_t)(IS_PMTERPS) == (uint32_t)(IS_PMTERPS))
                            {                             
                                myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
                            }
                            else
                            {
                                if(sampleRate == (uint32_t)(E_ADC_SAMPLE_RATE_55_HZ))
                                {
                                    while(pmTime < (uint32_t)(PM620_TIME_ADJUSTMENT))
                                    {
                                    
                                    }
                                }
                                pmTime = (uint32_t)(0);
                                //HAL_TIM_Base_Stop(&htim2);
                                myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
                            }
                        }
                        else
                        {
                            myState = E_SENSOR_STATUS_DISCOVERING;
                        }
                        
                    }
                    break;

                default:
                    break;
            }
        }

        //set/clear any errors in executing sensor functions
        updateSensorStatus(sensorError);
    }

    sensorError = mySensor->close();
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  
  if(htim->Instance == TIM2)
  {
      pmTime++;
  }
  if(htim->Instance == TIM3)
  {
      HAL_TIM_Base_Stop(htim);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_RESET);         
  }
  if(htim->Instance == TIM4)
  {
      HAL_TIM_Base_Stop(htim);
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);        
  }
  if(htim->Instance == TIM6)
  {
      HAL_TIM_Base_Stop(htim);
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);        
  }
  else
  {
  }
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
}


/**
 * @brief   Discover sensor on external comms
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorDiscover(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;
    OS_ERR os_error;
    eSensorError_t sensorError = sensor->readAppIdentity();

    if (sensorError == E_SENSOR_ERROR_NONE)
    {            
        OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_error);
        sensorError = sensor->readBootLoaderIdentity();
        
        if (sensorError == E_SENSOR_ERROR_NONE)
        {
          sensorError = sensor->readSerialNumber();

          if (sensorError == E_SENSOR_ERROR_NONE)
          {
              myState = E_SENSOR_STATUS_IDENTIFYING;
          }
          else
          {
              myState = E_SENSOR_STATUS_DISCONNECTED;
          }
        }
    }

    return sensorError;
}

/**
 * @brief   Get sensor details
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorIdentify(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;
    
    eSensorError_t sensorError = E_SENSOR_ERROR_TIMEOUT;
    
    sensorError = sensor->getCoefficientsData();
      
    if(E_SENSOR_ERROR_NONE == sensorError)
    {
        //sensorError = sensor->getCalibrationData();
        sensorError = E_SENSOR_ERROR_NONE;
        myState = E_SENSOR_STATUS_READY;
    }
      
#if 0
    //read fullscale and type
    eSensorError_t sensorError = sensor->readFullscaleAndType();

    if (sensorError == E_SENSOR_ERROR_NONE)
    {
        //read negative fullscale
        sensorError = sensor->readNegativeFullscale();

        if (sensorError == E_SENSOR_ERROR_NONE)
        {
            //read cal interval
            sensorError = sensor->readCalInterval();

            if (sensorError == E_SENSOR_ERROR_NONE)
            {
                //read last cal date
                sensorError = sensor->readCalDate();

                if (sensorError == E_SENSOR_ERROR_NONE)
                {
                    //read manufacture date
                    sensorError = sensor->readManufactureDate();

                    if (sensorError == E_SENSOR_ERROR_NONE)
                    {
                        myState = E_SENSOR_STATUS_READY;
                        //myState = E_SENSOR_STATUS_RUNNING;
                    }
                }
            }
        }
    }
#endif
    return sensorError;
}

eSensorError_t DSlotExternal::mySensorChecksumEnable(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;
    
    eSensorError_t sensorError = E_SENSOR_ERROR_TIMEOUT;
    
    sensorError = sensor->setCheckSum(E_CHECKSUM_ENABLED);  
    
    return sensorError;
}

eSensorError_t DSlotExternal::mySensorChecksumDisable(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;
    
    eSensorError_t sensorError = E_SENSOR_ERROR_TIMEOUT;
    
    sensorError = sensor->setCheckSum(E_CHECKSUM_DISABLED);  
    
    return sensorError;
}

eSensorError_t DSlotExternal::ledBlink(uint32_t seconds)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    OS_ERR os_error;
    
    uint32_t ms = seconds * (uint32_t)(1000);
    uint32_t blinks = (uint32_t)(10);
    uint32_t msPerBlink = (ms) / (blinks * (uint32_t)(2));
    uint32_t index = (uint32_t)(0);
    
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
    
    for(index = (uint32_t)(0); index < blinks; index++)
    {
        OSTimeDlyHMSM(0u, 0u, 0u, msPerBlink, OS_OPT_TIME_HMSM_STRICT, &os_error);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
        OSTimeDlyHMSM(0u, 0u, 0u, msPerBlink, OS_OPT_TIME_HMSM_STRICT, &os_error);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
    }
    
    return sensorError;
}