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
* @file     DComms.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The communications class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"
#include "DComms.h"
#include "memory.h"
#include "DPV624.h"
MISRAC_DISABLE
#include <rtos.h>
MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DComms class constructor
 * @param   void
 * @retval  void
 */
DComms::DComms()
    : DTask()
{
    OS_ERR os_error = OS_ERR_NONE;

    //get stack area from the memory partition memory block for function tasks
    myTaskStack = (CPU_STK *)RTOSMemGet((OS_MEM *)&memPartition, (OS_ERR *)&os_error);

#ifdef STACK_MONITOR
    stackArray.commsStack.addr = (void *)myTaskStack;
    stackArray.commsStack.size = (uint32_t)(MEM_PARTITION_BLK_SIZE * 4u);
    fillStack((char *)myTaskStack, 0xEE, (size_t)(MEM_PARTITION_BLK_SIZE * 4u));

#endif

    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE));

    if(!ok)
    {
#ifdef ASSERT_ENABLED
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
#endif
        PV624->handleError(E_ERROR_OS,
                           eSetError,
                           (uint32_t)os_error,
                           50u);
    }
}

/**
 * @brief   DComms class constructor
 * @param   mediumName
 * @param   osErr
 * @retval  void
 */
void DComms::start(char *mediumName, OS_ERR *os_error)
{
    //start off with no comms state machine specified
    myCommsFsm = NULL;

    activate(mediumName, (CPU_STK_SIZE)MEM_PARTITION_BLK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)0u, os_error);
}

/**
 * @brief   DComms initialisation function
 * @param   void
 * @retval  void
 */
void DComms::initialise(void)
{
}

/**
 * @brief   DCommsSerial run function
 * @param   void
 * @retval  void
 */
void DComms::runFunction(void)
{
    if(myCommsFsm != NULL)
    {
        myCommsFsm->createStates(commsMedium, this);
        myCommsFsm->run();
    }
}

/**
 * @brief   Get comms medium
 * @param   void
 * @retval  commsMedium - returns the pointer to its comms medium
 */
DDeviceSerial *DComms::getMedium(void)
{
    return commsMedium;
}

/**
 * @brief   Get external device connection status
 * @param   device is a pointer to for returned connected device status
 * @retval  void
 */
void DComms::getConnectedDeviceInfo(sExternalDevice_t *device)
{
    sExternalDevice_t *connectionStatus = myCommsFsm->getConnectedDeviceInfo();

    device->status = connectionStatus->status;
    device->dk = connectionStatus->dk;
    device->version = connectionStatus->version;
    device->serialNumber = connectionStatus->serialNumber;
}

/**
 * @brief   Grab (lock) comms medium resource
 * @param   sensor is pointer to a sensor instance
 * @retval  flag to indicate success (true) or failure (false) of the request
 */
bool DComms::grab(DSensor *sensor)
{
    bool flag = false;

    if(myCommsFsm != NULL)
    {
        myCommsFsm->suspend();
        flag = true;
    }

    return flag;
}

/**
 * @brief   Release comms medium resource
 * @param   sensor is pointer to a sensor instance
 * @retval  flag to indicate success (true) or failure (false) of the request
 */
bool DComms::release(DSensor *sensor)
{
    bool flag = false;

    if(myCommsFsm != NULL)
    {
        myCommsFsm->resume();
        flag = true;
    }

    return flag;
}

/**
 * @brief   Set (enter/exit) test mode
 * @param   state: value true = enter test mode, false = exit test mode
 * @retval  void
 */
void DComms::setTestMode(bool state)
{
    //by default, do nothing
}

/**
* @brief    handleError - process messaages from the rest of the system
* @param    waitFlags - event flags to which the function will wait
* @param    waitTime -   time out for wait
* @return   return status true : for Sucess false : for fail.
*/
bool DComms::waitForEvent(OS_FLAGS waitFlags, uint32_t waitTime)
{
    bool statusFlag = false;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;


    actualEvents = RTOSFlagPend(&myEventFlags,
                                waitFlags, (OS_TICK)waitTime, //runs, nominally, at 20Hz by default
                                OS_OPT_PEND_BLOCKING |
                                OS_OPT_PEND_FLAG_SET_ANY |
                                OS_OPT_PEND_FLAG_CONSUME,
                                &cpu_ts,
                                &os_error);

    if(os_error == (OS_ERR)OS_ERR_NONE)
    {
        if(actualEvents & waitFlags)
        {
            statusFlag = true;
        }
    }

    return statusFlag;

}