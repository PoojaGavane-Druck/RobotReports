/**
* Baker Hughes Confidential
* Copyright 2021. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     rtos.cpp
* @version  1.00.00
* @author   Simon Smith
* @date     07 October 2021
*
* @brief    The RTOS wrapper for uC/OS-III with consistent OS error handling, memory initialisation and parameter validation.
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "rtos.h"

MISRAC_DISABLE
#include <assert.h>
#include <stdbool.h>
#include <string.h>
MISRAC_ENABLE

#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
static uint32_t dbgQueueOverrunCount = 0u;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   Handle OS error codes from each call to uC/OS-III API
 * @param   pointer to error code
 * @return  void
 */
void handleOSError(OS_ERR *p_err)
{
    switch(*p_err)
    {
    // These errors are expected and can be safely ignored
    case OS_ERR_NONE:
    case OS_ERR_TIMEOUT:
    case OS_ERR_OS_NOT_RUNNING:
    case OS_ERR_PEND_WOULD_BLOCK:
        break;

    // This error is expected and can be safely ignored
    // Caused by RTOSSemSet(&sUartSem[lUartIdx].uartSemRx, (OS_SEM_CTR)0, &p_err);
    // as the semaphore count is already zero (could possibly occur from other causes).
    case OS_ERR_TASK_WAITING:
        break;

    // Remaining errors indicate a bug

    // TODO: Fix DE99708 unexpected OS errors - these are currently ignored
    case OS_ERR_OBJ_TYPE:
    case OS_ERR_TMR_INVALID:
    case OS_ERR_MUTEX_OWNER:
    case OS_ERR_MUTEX_NESTING:
    case OS_ERR_TIME_DLY_ISR:
    case OS_ERR_SET_ISR:
    case OS_ERR_TMR_ISR:
    case OS_ERR_TMR_STOPPED:
        break;

    default:

        // Tip: examine state of *p_err and *OSTCBCurPtr at this point in time in debugger
        MISRAC_DISABLE
        assert((uint16_t)*p_err == 0u);
        MISRAC_ENABLE

        if((PV624 != NULL) && (PV624->errorHandler != NULL))
        {
            PV624->handleError(E_ERROR_OS,
                               eSetError,
                               (uint32_t)(*p_err),
                               (uint16_t)60);
        }

        else
        {
            // Resort to global error handler
            Error_Handler();
        }

        break;
    }
}

// Refer to uC/OS-III os.h function prototypes for function header comments

void RTOSFlagCreate(OS_FLAG_GRP           *p_grp,
                    CPU_CHAR              *p_name,
                    OS_FLAGS               flags,
                    OS_ERR                *p_err)
{
    // Initialise memory for when restarting debugger
    memset((void *)p_grp, 0, sizeof(OS_FLAG_GRP));

    OSFlagCreate(p_grp, p_name, flags, p_err);
    handleOSError(p_err);
}

OS_FLAGS      RTOSFlagPend(OS_FLAG_GRP           *p_grp,
                           OS_FLAGS               flags,
                           OS_TICK                timeout,
                           OS_OPT                 opt,
                           CPU_TS                *p_ts,
                           OS_ERR                *p_err)
{
    OS_FLAGS retval = OSFlagPend(p_grp, flags, timeout, opt, p_ts, p_err);
    handleOSError(p_err);

    return retval;
}

OS_FLAGS      RTOSFlagPost(OS_FLAG_GRP           *p_grp,
                           OS_FLAGS               flags,
                           OS_OPT                 opt,
                           OS_ERR                *p_err)
{
    OS_FLAGS  retval = OSFlagPost(p_grp, flags, opt, p_err);
    handleOSError(p_err);

    return retval;
}

void          RTOSMemCreate(OS_MEM                *p_mem,
                            CPU_CHAR              *p_name,
                            void                  *p_addr,
                            OS_MEM_QTY             n_blks,
                            OS_MEM_SIZE            blk_size,
                            OS_ERR                *p_err)
{
    // Initialise memory for when restarting debugger
    memset((void *)p_mem, 0, sizeof(OS_MEM));

    OSMemCreate(p_mem, p_name, p_addr, n_blks, blk_size, p_err);
    handleOSError(p_err);
}

void         *RTOSMemGet(OS_MEM                *p_mem,
                         OS_ERR                *p_err)
{
    void *retval = OSMemGet(p_mem, p_err);
    handleOSError(p_err);
    return retval;
}

void          RTOSMemPut(OS_MEM                *p_mem,
                         void                  *p_blk,
                         OS_ERR                *p_err)
{
    OSMemPut(p_mem, p_blk, p_err);
    handleOSError(p_err);
}

void          RTOSMutexCreate(OS_MUTEX              *p_mutex,
                              CPU_CHAR              *p_name,
                              OS_ERR                *p_err)
{
    // Initialise memory for when restarting debugger
    memset((void *)p_mutex, 0, sizeof(OS_MUTEX));

    OSMutexCreate(p_mutex, p_name, p_err);
    handleOSError(p_err);
}

void          RTOSMutexPend(OS_MUTEX              *p_mutex,
                            OS_TICK                timeout,
                            OS_OPT                 opt,
                            CPU_TS                *p_ts,
                            OS_ERR                *p_err)
{
    OSMutexPend(p_mutex, timeout, opt, p_ts, p_err);
    handleOSError(p_err);
}

void          RTOSMutexPost(OS_MUTEX              *p_mutex,
                            OS_OPT                 opt,
                            OS_ERR                *p_err)
{
    OSMutexPost(p_mutex, opt, p_err);
    handleOSError(p_err);
}

void          RTOSSemCreate(OS_SEM                *p_sem,
                            CPU_CHAR              *p_name,
                            OS_SEM_CTR             cnt,
                            OS_ERR                *p_err)
{
    // Initialise memory for when restarting debugger
    memset((void *)p_sem, 0, sizeof(OS_SEM));

    OSSemCreate(p_sem, p_name, cnt, p_err);
    handleOSError(p_err);
}

OS_SEM_CTR    RTOSSemPend(OS_SEM                *p_sem,
                          OS_TICK                timeout,
                          OS_OPT                 opt,
                          CPU_TS                *p_ts,
                          OS_ERR                *p_err)
{
    OS_SEM_CTR retval = OSSemPend(p_sem, timeout, opt, p_ts, p_err);
    handleOSError(p_err);
    return retval;
}

OS_SEM_CTR    RTOSSemPost(OS_SEM                *p_sem,
                          OS_OPT                 opt,
                          OS_ERR                *p_err)
{
    OS_SEM_CTR retval = OSSemPost(p_sem, opt, p_err);
    handleOSError(p_err);
    return retval;
}

void          RTOSSemSet(OS_SEM                *p_sem,
                         OS_SEM_CTR             cnt,
                         OS_ERR                *p_err)
{
    OSSemSet(p_sem, cnt, p_err);
    handleOSError(p_err);
}

void          RTOSTaskCreate(OS_TCB                *p_tcb,
                             CPU_CHAR              *p_name,
                             OS_TASK_PTR            p_task,
                             void                  *p_arg,
                             OS_PRIO                prio,
                             CPU_STK               *p_stk_base,
                             CPU_STK_SIZE           stk_limit,
                             CPU_STK_SIZE           stk_size,
                             OS_MSG_QTY             q_size,
                             OS_TICK                time_quanta,
                             void                  *p_ext,
                             OS_OPT                 opt,
                             OS_ERR                *p_err)
{
    MISRAC_DISABLE
    assert(p_name != NULL);
    assert(p_stk_base != NULL);
    MISRAC_ENABLE

    // Initialise memory for when restarting debugger
    memset((void *)p_tcb, 0, sizeof(OS_TCB));

    OSTaskCreate(p_tcb, p_name, p_task, p_arg, prio, p_stk_base, stk_limit, stk_size, q_size, time_quanta, p_ext, opt, p_err);
    handleOSError(p_err);
}

void          RTOSTaskSuspend(OS_TCB                *p_tcb,
                              OS_ERR                *p_err)
{
    OSTaskSuspend(p_tcb, p_err);
    handleOSError(p_err);
}

void         *RTOSTaskQPend(OS_TICK                timeout,
                            OS_OPT                 opt,
                            OS_MSG_SIZE           *p_msg_size,
                            CPU_TS                *p_ts,
                            OS_ERR                *p_err)
{
    MISRAC_DISABLE
    assert(opt == OS_OPT_PEND_BLOCKING);
    MISRAC_ENABLE

    void *retval = OSTaskQPend(timeout, opt, p_msg_size, p_ts, p_err);
    handleOSError(p_err);

    return retval;
}

void RTOSTaskQPost(OS_TCB       *p_tcb,
                   void         *p_void,
                   OS_MSG_SIZE   msg_size,
                   OS_OPT        opt,
                   OS_ERR       *p_err)
{
    MISRAC_DISABLE
    assert(msg_size != 0);
    MISRAC_ENABLE

    OSTaskQPost(p_tcb, p_void, (OS_MSG_SIZE)0, (OS_OPT) opt, p_err);

    // Recover from any overruns from full task message queues
    if(*p_err == (OS_ERR)OS_ERR_Q_MAX)
    {
        dbgQueueOverrunCount++;
        OSTaskQFlush(p_tcb, p_err);
    }

    handleOSError(p_err);
}

void          RTOSTimeDlyHMSM(CPU_INT16U             hours,
                              CPU_INT16U             minutes,
                              CPU_INT16U             seconds,
                              CPU_INT32U             milli,
                              OS_OPT                 opt,
                              OS_ERR                *p_err)
{
    MISRAC_DISABLE
    assert(opt == OS_OPT_TIME_HMSM_STRICT);
    MISRAC_ENABLE

    OSTimeDlyHMSM(hours, minutes, seconds, milli, opt, p_err);
    handleOSError(p_err);
}

OS_TICK       RTOSTimeGet(OS_ERR                *p_err)
{
    OS_TICK retval = OSTimeGet(p_err);
    handleOSError(p_err);
    return retval;
}

void          RTOSTmrCreate(OS_TMR                *p_tmr,
                            CPU_CHAR              *p_name,
                            OS_TICK                dly,
                            OS_TICK                period,
                            OS_OPT                 opt,
                            OS_TMR_CALLBACK_PTR    p_callback,
                            void                  *p_callback_arg,
                            OS_ERR                *p_err)
{
    // Prevent recreating a timer that's already started as its callback will never occur (gets stuck in a tight loop)
    if(RTOSTmrStateGet(p_tmr, p_err) != OS_TMR_STATE_RUNNING)
    {
        // Initialise memory for when restarting debugger
        memset((void *)p_tmr, 0, sizeof(OS_TMR));

        OSTmrCreate(p_tmr, p_name, dly, period, opt, p_callback, p_callback_arg, p_err);
        handleOSError(p_err);
    }

    else
    {
        *p_err = OS_ERR_TMR_INVALID;
        handleOSError(p_err);
    }
}

CPU_BOOLEAN   RTOSTmrDel(OS_TMR                *p_tmr,
                         OS_ERR                *p_err)
{
    CPU_BOOLEAN retval = OSTmrDel(p_tmr, p_err);
    handleOSError(p_err);

    if(retval == DEF_FALSE)
    {
        *p_err = OS_ERR_TMR_INVALID;
        handleOSError(p_err);
    }

    return retval;
}

void          RTOSTmrSet(OS_TMR                *p_tmr,
                         OS_TICK                dly,
                         OS_TICK                period,
                         OS_TMR_CALLBACK_PTR    p_callback,
                         void                  *p_callback_arg,
                         OS_ERR                *p_err)
{
    OSTmrSet(p_tmr, dly, period, p_callback, p_callback_arg, p_err);
    handleOSError(p_err);
}

CPU_BOOLEAN   RTOSTmrStart(OS_TMR                *p_tmr,
                           OS_ERR                *p_err)
{
    CPU_BOOLEAN retval = OSTmrStart(p_tmr, p_err);
    handleOSError(p_err);

    if(retval == DEF_FALSE)
    {
        *p_err = OS_ERR_TMR_INVALID;
        handleOSError(p_err);
    }

    return retval;
}

OS_STATE      RTOSTmrStateGet(OS_TMR                *p_tmr,
                              OS_ERR                *p_err)
{
    OS_STATE retval = OSTmrStateGet(p_tmr, p_err);
    handleOSError(p_err);
    return retval;
}

CPU_BOOLEAN   RTOSTmrStop(OS_TMR                *p_tmr,
                          OS_OPT                 opt,
                          void                  *p_callback_arg,
                          OS_ERR                *p_err)
{
    CPU_BOOLEAN retval = OSTmrStop(p_tmr, opt, p_callback_arg, p_err);
    handleOSError(p_err);

    if(retval == DEF_FALSE)
    {
        *p_err = OS_ERR_TMR_INVALID;
        handleOSError(p_err);
    }

    return retval;
}