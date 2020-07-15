;********************************************************************************************************
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
;                                           Freescale MC9S12
;                                             Codewarrior
;
; Filename : cpu_a.s
; Version  : V1.31.05
;********************************************************************************************************


;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

        xdef  CPU_SR_Save
        xdef  CPU_SR_Restore


;********************************************************************************************************
;                                               EQUATES
;********************************************************************************************************


;********************************************************************************************************
;                                  SAVE THE CCR AND DISABLE INTERRUPTS
;                                                  &
;                                              RESTORE CCR
;
; Description : These function implements OS_CRITICAL_METHOD #3
;
; Arguments   : The function prototypes for the two functions are:
;               1) CPU_SR  CPU_SR_Save(void);
;                             where CPU_SR is the contents of the CCR register prior to disabling
;                             interrupts.
;               2) void    CPU_SR_Restore(CPU_SR cpu_sr);
;                             'cpu_sr' the the value of the CCR to restore.
;
; Note(s)     : 1) It's assumed that the compiler uses the D register to pass a single 16-bit argument
;                  to and from an assembly language function.
;********************************************************************************************************

CPU_SR_Save:
    tfr  ccr,b                         ; It's assumed that 8-bit return value is in register B
    sei                                ; Disable interrupts
    rtc                                ; Return to caller with D containing the previous CCR

CPU_SR_Restore:
    tfr  b,ccr                         ; B contains the CCR value to restore, move to CCR
    rtc


;********************************************************************************************************
;                                     CPU ASSEMBLY PORT FILE END
;********************************************************************************************************

