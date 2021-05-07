/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DTask.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The task base class header file
*/

#ifndef __DTASK_H
#define __DTASK_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <os.h>
#include <cpu.h>
#include <stdint.h>
//#include <lib_mem.h>
//#include <stm32l4xx_hal.h>
//#include <bsp_os.h>
//#include <bsp_int.h>
MISRAC_ENABLE

//#defines ----------------------------------------------------------------------------------------------------------*/
//Task flags
#define EV_FLAG_TASK_SHUTDOWN               0x00000001u //Shutdown - to end tasks gracefully
#define EV_FLAG_TASK_SENSOR_CONNECT         0x00000002u //sensor connection complete
#define EV_FLAG_TASK_SENSOR_DISCONNECT      0x00000004u //sensor disconnected/comms failed
#define EV_FLAG_TASK_NEW_VALUE              0x00000008u //new sensor measurement available
#define EV_FLAG_TASK_NEW_SETPOINT           0x00000010u //new setpoint for source sensor
#define EV_FLAG_TASK_SENSOR_FAULT           0x00000020u //sensor failure (unspecified)
#define EV_FLAG_TASK_SENSOR_PAUSE           0x00000040u //signal for sensor to pause (stop measurements until told to either continue)
#define EV_FLAG_TASK_SENSOR_CONTINUE        0x00000080u //go-ahead signal for sensor to resume after a pause
#define EV_FLAG_TASK_SENSOR_SYNC            0x00000100u //signal for a synchronised measurement (used after a pause)
#define EV_FLAG_TASK_SENSOR_RETRY           0x00000200u //go-ahead signal for sensor to re-start/retry after, for example after a fault
#define EV_FLAG_TASK_SENSOR_NEW_RANGE       0x00000400u //measure function range (auto-) changed
#define EV_FLAG_TASK_SENSOR_IN_LIMIT        0x00000800u //source function setpoint achieved
#define EV_FLAG_TASK_SENSOR_ZERO_ERROR      0x00001000u //zero attempt failed
#define EV_FLAG_TASK_SENSOR_CAL_REJECTED    0x00002000u //calibration rejected (unsuccessful)
#define EV_FLAG_TASK_SENSOR_CAL_DEFAULT     0x00004000u //default calibration use (ie, measurements may be uncalibrated)
#define EV_FLAG_TASK_SENSOR_CAL_DUE         0x00008000u //calibration due status
#define EV_FLAG_TASK_SENSOR_CAL_DATE        0x00010000u //calibration date status
#define EV_FLAG_TASK_UPDATE_BATTERY_STATUS        0x00020000u //To Update Battery Status LEDS
#define EV_FLAG_TASK_NEW_CONTROLLER_MODE_RECIEVED  0x00040000u //New Controller MOde received
#define EV_FLAG_TASK_NEW_SET_POINT_RECIEVED        0x00080000u //New set point received
#define EV_FLAG_TASK_CAL_SAMPLES_COUNT               0x00100000u //cal samples remaining
/* Types ------------------------------------------------------------------------------------------------------------*/
typedef	enum
{
    E_TASK_STATE_DORMANT = 0,
    E_TASK_STATE_CREATED,
    E_TASK_STATE_INITIALISING,
    E_TASK_STATE_RUNNING,
    E_TASK_STATE_STOPPING,
    E_TASK_STATE_TERMINATED,
    E_TASK_STATE_FAILED

} eTaskState_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DTask
{
private:
    static void taskRunner(void *p_arg);    //task function

protected:
    char *myName;                           //task name
    OS_TCB myTaskTCB;	                    //Task Control Block for this task
    CPU_STK *myTaskStack;                   //allocated by derived class
    eTaskState_t myTaskState;

    OS_FLAG_GRP myEventFlags;               //event flags to pend on

public:
    DTask();
    ~DTask();

    eTaskState_t getState(void);

    void activate(char *taskName, CPU_STK_SIZE stackSize, OS_PRIO priority, OS_MSG_QTY msgQty, OS_ERR *osErr);
    virtual void initialise(void);
    virtual void runFunction(void);
    void shutdown(void);
    virtual void cleanUp(void);

    void postEvent(uint32_t eventFlag);
};

#endif /* __DTASK_H */
