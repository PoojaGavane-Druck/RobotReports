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
MISRAC_ENABLE

#include "DCommsFsm.h"
#include "DLock.h"

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
//        error_code_t errorCode;
//        errorCode.bit.osError = SET;
//        DPI610E->handleError(errorCode, os_error);
 #endif
    }
}

/**
 * @brief   Create required states of the state machine
 * @param   commsMedium is pointer to serial comms medium
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
   eCommOperationMode_t mode;
   setMode(myInitialMode);
    while (DEF_TRUE)
    {
        mode = getMode();

        if (myStateArray[mode] != NULL)
        {
        	mode = myStateArray[myCurrentMode]->run();
		//update state
            setMode(mode);
        }
    }
}

/**
 * @brief   Set state machine state
 * @param   void
 * @retval  current FSM state
 */
eCommOperationMode_t DCommsFsm::getMode(void)
{
    DLock is_on(&myMutex);
    return myCurrentMode;
}

/**
 * @brief   Set state machine state
 * @param   state is the new FSM state
 * @retval  void
 */
void DCommsFsm::setMode(eCommOperationMode_t newMode)
{
    DLock is_on(&myMutex);
    myCurrentMode = newMode;
}

/**
 * @brief   Suspend state machine
 * @param   void
 * @retval  void
 */
void DCommsFsm::suspend(void)
{
    eCommOperationMode_t mode=getMode() ;
    myStateArray[mode]->suspend();
}

/**
 * @brief   Resume state machine
 * @param   void
 * @retval  void
 */
void DCommsFsm::resume(void)
{
    eCommOperationMode_t mode=getMode() ;
    myStateArray[mode]->resume();
}

/**
 * @brief   Get external device connection status
 * @param   void
 * @retval  pointer to for returning connected device status
 */
sExternalDevice_t *DCommsFsm::getConnectedDeviceInfo(void)
{
    return &DCommsState::externalDevice;
}
