;*******************************************************************************************************
;                                                uC/CPU
;                                    CPU CONFIGURATION & PORT LAYER
;
;                         (c) Copyright 2004-2019; Silicon Laboratories Inc.,
;                                400 W. Cesar Chavez, Austin, TX 78701
;
;                   All rights reserved. Protected by international copyright laws.
;
;                  Your use of this software is subject to your acceptance of the terms
;                  of a Silicon Labs Micrium software license, which can be obtained by
;                  contacting info@micrium.com. If you do not agree to the terms of this
;                  license, you may not use this software.
;
;                  Please help us continue to provide the Embedded community with the finest
;                  software available. Your honesty is greatly appreciated.
;
;                    You can find our product's documentation at: doc.micrium.com
;
;                          For more information visit us at: www.micrium.com
;********************************************************************************************************

;********************************************************************************************************
;
;                                            CPU PORT FILE
;
;                                              Fujitsu FR
;                                           Softune Compiler
;
; Filename : cpu_a.asm
; Version  : V1.31.05
;********************************************************************************************************

        .PROGRAM    CPU_A


;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

        .EXPORT     _CPU_SR_Save
        .EXPORT     _CPU_SR_Restore

        .section    CODE, code, align=4


;********************************************************************************************************
;                                   CRITICAL SECTION FUNCTIONS
;
; Description : Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
;               would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
;               disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
;               into the CPU's status register.
;
; Prototypes  : CPU_SR  CPU_SR_Save   (void);
;               void    CPU_SR_Restore(CPU_SR cpu_sr);
;
; Note(s)     : (1) These functions are used in general like this :
;
;                       void  Task (void  *p_arg)
;                       {
;                           CPU_SR_ALLOC();                     /* Allocate storage for CPU status register */
;                               :
;                               :
;                           CPU_CRITICAL_ENTER();               /* cpu_sr = CPU_SR_Save();                  */
;                               :
;                               :
;                           CPU_CRITICAL_EXIT();                /* CPU_SR_Restore(cpu_sr);                  */
;                               :
;                       }
;********************************************************************************************************


_CPU_SR_Save:
        MOV         PS, R4                   ; Save state of PS
        ANDCCR      #0xEF                    ; Disable interrupts
        RET

_CPU_SR_Restore:
        MOV         R4, PS                   ; Restore state of PS
        RET


;********************************************************************************************************
;                                     CPU ASSEMBLY PORT FILE END
;********************************************************************************************************

        .END

