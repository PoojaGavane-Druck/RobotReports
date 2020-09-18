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
#include "Utilities.h"
#include "DSensorExternal.h"
#include "uart.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define TEMPERATURE_POLLING_INTERVAL 20000

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

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
    myWaitFlags |= EV_FLAG_TASK_SENSOR_CONTINUE | EV_FLAG_TASK_SENSOR_RETRY;
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
    eSensorError_t sensorError = mySensor->initialise();
 
    myState = E_SENSOR_STATUS_DISCOVERING;

    while (runFlag == true)
    {
        /* Sensor data acquisition is stopping after a certain amount of time
        The following code is changed to test it quickly */
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)50u, //runs, nominally, at 20Hz by default
                                    OS_OPT_PEND_BLOCKING | 
                                    OS_OPT_PEND_FLAG_SET_ANY | 
                                    OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);

#if 0
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)500u, //runs, nominally, at 2Hz by default
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
#endif
        //check actions to execute routinely (ie, timed)
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
            switch(myState)
            {
                case E_SENSOR_STATUS_DISCOVERING:
                    //any sensor error will be mopped up below
                    //sensorError = mySensorDiscover();
                    break;

                case E_SENSOR_STATUS_IDENTIFYING:
                    sensorError = mySensorIdentify();

                    //if no sensor error than proceed as normal (errors will be mopped up below)
                    if (sensorError == E_SENSOR_ERROR_NONE)
                    {
                        //notify parent that we have connected, awaiting next action - this is to allow
                        //the higher level to decide what other initialisation/registration may be required
                        myOwner->postEvent(EV_FLAG_TASK_SENSOR_CONNECT);
                        resume();
                    }
                    break;

                case E_SENSOR_STATUS_RUNNING:
                    //take measurement and post event
                    if((uint32_t)(0) == timeElapsed)
                    {
                      channelSel = E_CHANNEL_0 | E_CHANNEL_1;
                    }
                    else if((uint32_t)(TEMPERATURE_POLLING_INTERVAL) <= timeElapsed)
                    {
                       channelSel = (uint32_t) E_CHANNEL_1;
                    }
                    else
                    {
                      channelSel =(uint32_t) E_CHANNEL_0; 
                    }
                    disableSerialPortTxLine(UART_PORT3);        
                    enableSerialPortTxLine(UART_PORT3);
                    sensorError = mySensor->measure(channelSel); 
                    //if no sensor error than proceed as normal (errors will be mopped up below)
                    if (sensorError == E_SENSOR_ERROR_NONE)
                    {
                        myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
                        if((uint32_t)E_CHANNEL_1 == channelSel)
                        {
                          timeElapsed = (uint32_t)0;
                        }
                        timeElapsed = timeElapsed + (uint32_t)50;
                        
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
                    if ((actualEvents & EV_FLAG_TASK_SENSOR_CONTINUE) == EV_FLAG_TASK_SENSOR_CONTINUE)
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

                default:
                    break;
            }
        }

        //set/clear any errors in executing sensor functions
        updateSensorStatus(sensorError);
    }

    sensorError = mySensor->close();
}

/**
 * @brief   Discover sensor on external comms
 * @param   void
 * @retval  sensor error status
 */
eSensorError_t DSlotExternal::mySensorDiscover(void)
{
    DSensorExternal *sensor = (DSensorExternal *)mySensor;

    eSensorError_t sensorError = sensor->readIdentity();

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

