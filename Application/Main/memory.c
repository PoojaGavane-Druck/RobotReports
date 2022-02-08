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
* @file     memory.c
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     06 May 2020
*
* @brief    The memory related source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "memory.h"

MISRAC_DISABLE
#include <assert.h>
MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define STACK_NUM_BLOCKS    6u                              //number of blocks in memory partition
#define STACK_BLOCK_SIZE    (MEM_PARTITION_BLK_SIZE * 4u)   //sizeof CPU_INT32U is 4 bytes (multiply by 4 to get bytes)

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
OS_MEM memPartition;

CPU_INT08U memBlocks[STACK_NUM_BLOCKS][STACK_BLOCK_SIZE];

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

//create partitions for function and slot task stack dynamic allocation
void createMemoryPartition(void)
{
    OS_ERR err = OS_ERR_NONE;

    RTOSMemCreate((OS_MEM *)&memPartition,
                  (CPU_CHAR *)"Memory610E",
                  (void *)&memBlocks[0][0],
                  (OS_MEM_QTY)STACK_NUM_BLOCKS,   //number of memory blocks
                  (OS_MEM_SIZE)STACK_BLOCK_SIZE,  //sizeof each memory block in bytes
                  &err);

    assert(err == (OS_ERR)OS_ERR_NONE);
}

