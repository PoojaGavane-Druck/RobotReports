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
* @file     rtos.h
* @version  1.00.00
* @author   Simon Smith
* @date     07 October 2021
*
* @brief    The RTOS wrapper for uC/OS-III with consistent OS error handling, memory initialisation and parameter validation.
*/
//*********************************************************************************************************************

#ifndef __RTOS_H
#define __RTOS_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
#include "main.h"
MISRAC_ENABLE

#ifdef __cplusplus
extern "C"
{
/* External C language linkage */
#endif

void handleOSError(OS_ERR *p_err);

// Refer to uC/OS-III os.h function prototypes for function header comments

void RTOSFlagCreate              (OS_FLAG_GRP           *p_grp,
                                  CPU_CHAR              *p_name,
                                  OS_FLAGS               flags,
                                  OS_ERR                *p_err);

OS_FLAGS      RTOSFlagPend                (OS_FLAG_GRP           *p_grp,
        OS_FLAGS               flags,
        OS_TICK                timeout,
        OS_OPT                 opt,
        CPU_TS                *p_ts,
        OS_ERR                *p_err);

OS_FLAGS      RTOSFlagPost                (OS_FLAG_GRP           *p_grp,
        OS_FLAGS               flags,
        OS_OPT                 opt,
        OS_ERR                *p_err);

void          RTOSMemCreate               (OS_MEM                *p_mem,
        CPU_CHAR              *p_name,
        void                  *p_addr,
        OS_MEM_QTY             n_blks,
        OS_MEM_SIZE            blk_size,
        OS_ERR                *p_err);

void         *RTOSMemGet                  (OS_MEM                *p_mem,
        OS_ERR                *p_err);

void          RTOSMemPut                  (OS_MEM                *p_mem,
        void                  *p_blk,
        OS_ERR                *p_err);

void          RTOSMutexCreate             (OS_MUTEX              *p_mutex,
        CPU_CHAR              *p_name,
        OS_ERR                *p_err);

void          RTOSMutexPend               (OS_MUTEX              *p_mutex,
        OS_TICK                timeout,
        OS_OPT                 opt,
        CPU_TS                *p_ts,
        OS_ERR                *p_err);

void          RTOSMutexPost               (OS_MUTEX              *p_mutex,
        OS_OPT                 opt,
        OS_ERR                *p_err);


void          RTOSSemCreate               (OS_SEM                *p_sem,
        CPU_CHAR              *p_name,
        OS_SEM_CTR             cnt,
        OS_ERR                *p_err);

OS_SEM_CTR    RTOSSemPend                 (OS_SEM                *p_sem,
        OS_TICK                timeout,
        OS_OPT                 opt,
        CPU_TS                *p_ts,
        OS_ERR                *p_err);

OS_SEM_CTR    RTOSSemPost                 (OS_SEM                *p_sem,
        OS_OPT                 opt,
        OS_ERR                *p_err);

void          RTOSSemSet                  (OS_SEM                *p_sem,
        OS_SEM_CTR             cnt,
        OS_ERR                *p_err);


void          RTOSTaskCreate              (OS_TCB                *p_tcb,
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
        OS_ERR                *p_err);

void          RTOSTaskSuspend             (OS_TCB                *p_tcb,
        OS_ERR                *p_err);


void*         RTOSTaskQPend               (OS_TICK                timeout,
        OS_OPT                 opt,
        OS_MSG_SIZE           *p_msg_size,
        CPU_TS                *p_ts,
        OS_ERR                *p_err);

void          RTOSTaskQPost (OS_TCB       *p_tcb,
                             void         *p_void,
                             OS_MSG_SIZE   msg_size,
                             OS_OPT        opt,
                             OS_ERR       *p_err);


void          RTOSTimeDlyHMSM             (CPU_INT16U             hours,
        CPU_INT16U             minutes,
        CPU_INT16U             seconds,
        CPU_INT32U             milli,
        OS_OPT                 opt,
        OS_ERR                *p_err);

OS_TICK       RTOSTimeGet                 (OS_ERR                *p_err);

void          RTOSTmrCreate               (OS_TMR                *p_tmr,
        CPU_CHAR              *p_name,
        OS_TICK                dly,
        OS_TICK                period,
        OS_OPT                 opt,
        OS_TMR_CALLBACK_PTR    p_callback,
        void                  *p_callback_arg,
        OS_ERR                *p_err);

CPU_BOOLEAN   RTOSTmrDel                  (OS_TMR                *p_tmr,
        OS_ERR                *p_err);

void          RTOSTmrSet                  (OS_TMR                *p_tmr,
        OS_TICK                dly,
        OS_TICK                period,
        OS_TMR_CALLBACK_PTR    p_callback,
        void                  *p_callback_arg,
        OS_ERR                *p_err);

OS_TICK       RTOSTmrRemainGet            (OS_TMR                *p_tmr,
        OS_ERR                *p_err);

CPU_BOOLEAN   RTOSTmrStart                (OS_TMR                *p_tmr,
        OS_ERR                *p_err);

OS_STATE      RTOSTmrStateGet             (OS_TMR                *p_tmr,
        OS_ERR                *p_err);

CPU_BOOLEAN   RTOSTmrStop                 (OS_TMR                *p_tmr,
        OS_OPT                 opt,
        void                  *p_callback_arg,
        OS_ERR                *p_err);

#ifdef __cplusplus
}                                                               /* End of external C language linkage */
#endif

#endif /* __RTOS_H */