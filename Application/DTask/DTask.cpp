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
* @file     DTask.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     26 March 2020
*
* @brief    The task base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DTask.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

DTask::DTask()
{
    OS_ERR os_error;

    myTaskStack = NULL; //to indicate that stack space has not been allocated yet
    myTaskState = (eTaskState_t)E_TASK_STATE_DORMANT;

    //TODO: Should every DTask have a name?
    RTOSFlagCreate(&myEventFlags, "TaskEvents", (OS_FLAGS)0, &os_error);
}

DTask::~DTask()
{
}

void DTask::activate(char *taskName, CPU_STK_SIZE stackSize, OS_PRIO priority, OS_MSG_QTY msgQty, OS_ERR *osErr)
{
//    //create stack area for this instance
//    CPU_STK_SIZE stackBytes = stackSize * (CPU_STK_SIZE)sizeof(CPU_STK_SIZE);
//    myTaskStack = (CPU_STK *)new char[stackBytes];

    //check if stack has been created successfully before going ahead
    if(myTaskStack == NULL)
    {
        *osErr = (OS_ERR)OS_ERR_STK_INVALID;
    }

    else
    {
        //Calls OS function to create the Key Task.
        RTOSTaskCreate(&myTaskTCB,
                       (CPU_CHAR *)taskName,
                       DTask::taskRunner,
                       (void *)this,
                       (OS_PRIO)priority,
                       (CPU_STK *)myTaskStack,
                       (CPU_STK_SIZE)(stackSize / 10u),
                       (CPU_STK_SIZE)stackSize,
                       (OS_MSG_QTY)msgQty,
                       (OS_TICK)0u,
                       (void *)0u,
                       (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                       osErr);
    }

    //report error if task did not create successfully
    if(*osErr != (OS_ERR)OS_ERR_NONE)
    {
        myTaskState = (eTaskState_t)E_TASK_STATE_FAILED;
        //TODO: PV624->errorHandler->handleError(E_ERROR_OS_TASK_CREATE);
    }

    else
    {
        myTaskState = (eTaskState_t)E_TASK_STATE_CREATED;
    }
}

void DTask::initialise(void)
{
}

void DTask::runFunction(void)
{
    //this needs to be a while loop that pends on event flags
    //OSFlagPend(&myEventFlags, tasksToWaitFor, (OS_TICK)5000u, OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ALL, &cpu_ts, &os_error);
    //where. eg,     OS_FLAGS tasksToWaitFor = EV_FLAG_TASK_SHUTDOWN |
    //                                EV_FLAG_TASK_???? |
    //                                EV_FLAG_TASK_????;
}

/**
 * @brief   Clean up after termination of task
 * @param   void
 * @return  void
 */
void DTask::cleanUp(void)
{
}

/**
 * @brief   Task main loop function
 * @param p_arg pointer to this instance
 * @return void
 */
void DTask::taskRunner(void *p_arg)
{
    //OS_ERR osErr;

    DTask *thisTask = (DTask *)p_arg;

    //instance will override initialise function if required
    thisTask->myTaskState = (eTaskState_t)E_TASK_STATE_INITIALISING;
    thisTask->initialise();

    //instance must override runFunction function if it is to do anything
    thisTask->myTaskState = (eTaskState_t)E_TASK_STATE_RUNNING;
    thisTask->runFunction();

    //instance will override cleanUp function if required and then call the base class 'cleanUp' function to tidy up
    thisTask->myTaskState = (eTaskState_t)E_TASK_STATE_STOPPING;
    //thisTask->cleanUp();

    //TODO: Task delete doesn't do the intended thing here. so letting the OS's accidental task return catch delete it instead, as that works
//    //when a task terminates (or runs to completion) it must be deleted itself before exit.
//    OSTaskDel(&thisTask->myTaskTCB, &osErr);
//
//    //report error if task did not delete
//    if (osErr != (OS_ERR)OS_ERR_NONE)
//    {
//        thisTask->myTaskState = (eTaskState_t)E_TASK_STATE_FAILED;
//        //TODO: DPI610E->errorHandler->handleError(E_ERROR_OS_TASK_DELETE);
//    }
//    else
//    {
    thisTask->myTaskState = (eTaskState_t)E_TASK_STATE_DORMANT;
//    }
}

/**
 *  @brief get task state
 *  @param  void
 *  @return task state
 */
eTaskState_t DTask::getState(void)
{
    return myTaskState;
}

/**
 *  @brief shutdown - shuts down and cleans up the memory area used by the task.
 *
 *  @param  void
 *  @return void
 */
void DTask::shutdown(void)
{
    OS_ERR os_error;

    postEvent(EV_FLAG_TASK_SHUTDOWN);

    //wait until shutdown has completed - timeout if it doesn't happen within 5 seconds //TODO: have sensible time here
    bool successful = false;
    int count = 0;

    for(int i = 0; i < 100; i++)
    {
        if(myTaskState == (eTaskState_t)E_TASK_STATE_DORMANT)
        {
            successful = true;
            break;
        }

        //Polling again every 50 ms
        RTOSTimeDlyHMSM(0u, 0u, 0u, 50u, OS_OPT_TIME_HMSM_STRICT, &os_error);
        count++;
    }

    if(successful != true)
    {
        //report error
        RTOSTimeDlyHMSM(0u, 0u, 0u, 50u, OS_OPT_TIME_HMSM_STRICT, &os_error);
    }
}

/**
 *  @brief Post specified event flag to this task instance
 *
 *  @param  event flag - one or more bits as appropriate
 *  @return void
 */
void DTask::postEvent(uint32_t eventFlag)
{
    OS_ERR os_error;

    //signal shutdown request to task
    RTOSFlagPost(&myEventFlags, eventFlag, OS_OPT_POST_FLAG_SET, &os_error);

    if(os_error != (OS_ERR)OS_ERR_NONE)
    {
        //report error
    }
}

