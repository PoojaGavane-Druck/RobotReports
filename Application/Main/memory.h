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
* @file     memory.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     06 May 2020
*
* @brief    The The memory related functions header file
*/

#ifndef __MEMORY_H
#define __MEMORY_H

#ifdef __cplusplus
extern "C" /* External C language linkage */
{
#endif

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <os.h>
MISRAC_ENABLE

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define MEM_PARTITION_BLK_SIZE    512u    //stack size in terms CPU_STK_SIZE, which is CPU stack size data type

/* Variables --------------------------------------------------------------------------------------------------------*/
extern OS_MEM memPartition;

/* Prototypes -------------------------------------------------------------------------------------------------------*/
void createMemoryPartition(void);

#ifdef __cplusplus
}                                                               /* End of external C language linkage */
#endif

#endif //__MEMORY_H
