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
* @file     DLock.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The mutex class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DLock.h"

MISRAC_DISABLE
#include <stdio.h>
#include <assert.h>
MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 12.10 comma separator shall not be used (otherwise, doesn't like assert function)
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm035")

/**
 * @brief   DLock class constructor
 * @note    The mutex is acquired when the instance is created
 * @param   mutex is pointer to the locking mutex
 * @retval  void
 */
DLock::DLock(OS_MUTEX *mutex)
{
    OS_ERR err = OS_ERR_NONE;
    CPU_TS  ts;

    myMutex = mutex;

    if(myMutex != NULL)
    {
        RTOSMutexPend(myMutex, (OS_TICK)0, (OS_OPT)OS_OPT_PEND_BLOCKING, (CPU_TS *)&ts, &err);

        //catch problem during development - it is left to the user of the DLock instance
        //to make sure the mutex is successully created before use

        assert((err == (OS_ERR)OS_ERR_NONE) || (err == (OS_ERR)OS_ERR_MUTEX_OWNER) || (err == (OS_ERR)OS_ERR_MUTEX_NESTING));
    }
}

/**
 * @brief   DLock class destructor
 * @note    The mutex is released when the instance goes out of scope
 * @param   void
 * @retval  void
 */
DLock::~DLock()
{
    if(myMutex != NULL)
    {
        OS_ERR err = OS_ERR_NONE;
        RTOSMutexPost(myMutex, (OS_OPT)OS_OPT_POST_NONE, &err);
        myMutex = NULL;

        //catch problem during development - it is left to the user of the DLock instance
        //to make sure the mutex is successully created before use
        assert((err == (OS_ERR)OS_ERR_NONE) || (err == (OS_ERR)OS_ERR_MUTEX_OWNER) || (err == (OS_ERR)OS_ERR_MUTEX_NESTING));
    }
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 12.10 comma separator shall not be used.
 **********************************************************************************************************************/
_Pragma("diag_default=Pm035")

