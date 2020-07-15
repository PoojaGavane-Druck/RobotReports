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

/********************************************************************************************************
*
*                                            CPU PORT FILE
*
*                                      Renesas RL78 Specific code
*                                          GNU RL78 C Compiler
*
* Filename : cpu_c.c
* Version  : V1.31.05
*********************************************************************************************************
*/

#include  <cpu.h>


/********************************************************************************************************
*                                    set_interrupt_state & get_interrupt_state
*
* Description: Set or retrieve interrupt priority level.
* KPIT GNU Work around for set and get interrupt states
*********************************************************************************************************
*/

void __set_interrupt_state(CPU_INT08U cpu_sr){
    if (cpu_sr)
        asm("EI");
    else
        asm("DI");
}
	
CPU_INT08U __get_interrupt_state(void){

    CPU_INT08U  cpu_sr;


    asm(" MOV  A, PSW");                                        /* Get Process Status Word Register PSW value           */
    asm(" SHR  A, 7");                                          /* Save only the Interrupt Enabled (IE) Bit             */
    asm(" MOV  %0, A" : "=r"(cpu_sr));                          /* Save IE bit value into cpu_sr                        */       //__asm

    return(cpu_sr);
}

