/*
*********************************************************************************************************
*                                                uC/CPU
*                                    CPU CONFIGURATION & PORT LAYER
*
*                         (c) Copyright 2004-2019; Silicon Laboratories Inc.,
*                                400 W. Cesar Chavez, Austin, TX 78701
*
*                   All rights reserved. Protected by international copyright laws.
*
*                  Your use of this software is subject to your acceptance of the terms
*                  of a Silicon Labs Micrium software license, which can be obtained by
*                  contacting info@micrium.com. If you do not agree to the terms of this
*                  license, you may not use this software.
*
*                  Please help us continue to provide the Embedded community with the finest
*                  software available. Your honesty is greatly appreciated.
*
*                    You can find our product's documentation at: doc.micrium.com
*
*                          For more information visit us at: www.micrium.com
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            CPU PORT FILE
*
*                                               Nios II
*                                            GNU C Compiler
*
* Filename : cpu_c.c
* Version  : V1.31.05
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    MICRIUM_SOURCE
#include  <cpu.h>
#include  <cpu_core.h>

#ifdef __cplusplus
extern  "C" {
#endif


/*
*********************************************************************************************************
*                                            CPU_SR_Save()
*
* Description : This function disables interrupts for critical sections of code.
*
* Argument(s) : none.
*
* Return(s)   : The CPU's status register, so that interrupts can later be returned to their original
*               state.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_SR  CPU_SR_Save (void)
{
    return (alt_irq_disable_all());
}


/*
*********************************************************************************************************
*                                          CPU_SR_Restore()
*
* Description : Restores interrupts after critical sections of code.
*
* Argument(s) : cpu_sr    The interrupt status that will be restored.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  CPU_SR_Restore (CPU_SR  cpu_sr)
{
    alt_irq_enable_all(cpu_sr);
}

#ifdef __cplusplus
}
#endif
