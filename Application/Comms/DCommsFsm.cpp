/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DCommsFsm.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The communications finite state machine base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <assert.h>
#include <os.h>
#include <lib_def.h>
#include <stdint.h>
#include <stdlib.h>
MISRAC_ENABLE

#include "DCommsFsm.h"
#include "DLock.h"
#include "Utilities.h"
#include "DPV624.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsFsm class constructor
 * @param   void
 * @retval  void
 */
DCommsFsm::DCommsFsm(void)
{
    OS_ERR os_error = OS_ERR_NONE;

    //create mutex for resource locking
    char *name = "commsFsm";
    memset((void*)&myMutex, 0, sizeof(OS_MUTEX));
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &os_error);

    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_OBJ_CREATED));

    if (!ok)
    {
#ifdef ASSERT_ENABLED
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE        
 #endif
       PV624->handleError(E_ERROR_OS, 
                           eSetError,
                           (uint32_t)os_error,
                           (uint16_t)51);
    }
}

/**
 * @brief   Create required states of the state machine
 * @param   commsMedium is pointer to serial comms medium
 * @param   task is  pointer to owner task
 * @retval  void
 */
void DCommsFsm::createStates(DDeviceSerial *commsMedium, DTask *task)
{
}

/**
 * @brief   DCommsFsm run function - once started, it runs forever
 * @param   void
 * @retval  void
 */
void DCommsFsm::run(void)
{
    setState(myInitialState);
    eStateDuci_t state;

    while (DEF_TRUE)
    {
        state = getState();

        if (myStateArray[state] != NULL)
        {
            state = myStateArray[myCurrentState]->run();

            //update state
            setState(state);
        }
        else
        {
            sleep(500u);
        }
    }
}

/**
 * @brief   Set state machine state
 * @param   void
 * @retval  current FSM state
 */
eStateDuci_t DCommsFsm::getState(void)
{
    DLock is_on(&myMutex);
    return myCurrentState;
}

/**
 * @brief   Set state machine state
 * @param   state is the new FSM state
 * @retval  void
 */
void DCommsFsm::setState(eStateDuci_t state)
{
    DLock is_on(&myMutex);
    myCurrentState = state;
}

/**
 * @brief   Suspend state machine
 * @param   void
 * @retval  void
 */
void DCommsFsm::suspend(void)
{
    eStateDuci_t state = getState();
    myStateArray[state]->suspend();
}

/**
 * @brief   Resume state machine
 * @param   void
 * @retval  void
 */
void DCommsFsm::resume(void)
{
    eStateDuci_t state = getState();
    myStateArray[state]->resume();
}

/**
 * @brief   Get external device connection status
 * @param   void
 * @retval  pointer to for returning connected device status
 */
sExternalDevice_t *DCommsFsm::getConnectedDeviceInfo(void)
{
    DLock is_on(&myMutex);
    return &DCommsState::externalDevice;
}
