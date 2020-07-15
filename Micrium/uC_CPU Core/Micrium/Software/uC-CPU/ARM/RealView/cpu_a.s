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
;                                                ARM
;                                      RealView Development Suite
;                            RealView Microcontroller Development Kit (MDK)
;                                       ARM Developer Suite (ADS)
;                                            Keil uVision
;
; Filename : cpu_a.s
; Version  : V1.31.05
;********************************************************************************************************


;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

    EXPORT  CPU_SR_Save
    EXPORT  CPU_SR_Restore

    EXPORT  CPU_IntDis
    EXPORT  CPU_IntEn

    EXPORT  CPU_IRQ_Dis
    EXPORT  CPU_IRQ_En

    EXPORT  CPU_FIQ_Dis
    EXPORT  CPU_FIQ_En


;********************************************************************************************************
;                                                EQUATES
;********************************************************************************************************

CPU_ARM_CTRL_INT_DIS  EQU       0xC0                            ; Disable both FIQ & IRQ
CPU_ARM_CTRL_FIQ_DIS  EQU       0x40                            ; Disable FIQ.
CPU_ARM_CTRL_IRQ_DIS  EQU       0x80                            ; Disable IRQ.


;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

    AREA _CPU_A_CODE_, CODE, READONLY
    ARM


;********************************************************************************************************
;                                      CRITICAL SECTION FUNCTIONS
;
; Description : Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking, the
;               state of the interrupt disable flag is stored in the local variable 'cpu_sr' & interrupts
;               are then disabled ('cpu_sr' is allocated in all functions that need to disable interrupts).
;               The previous interrupt state is restored by copying 'cpu_sr' into the CPU's status register.
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
;
;               (2) CPU_SR_Restore() is implemented as recommended by Atmel's application note :
;
;                       "Disabling Interrupts at Processor Level"
;********************************************************************************************************

CPU_SR_Save
        MRS     R0, CPSR

CPU_SR_Save_Loop
                                                                ; Set IRQ & FIQ bits in CPSR to DISABLE all interrupts
        ORR     R1, R0, #CPU_ARM_CTRL_INT_DIS
        MSR     CPSR_c, R1
        MRS     R1, CPSR                                        ; Confirm that CPSR contains the proper interrupt disable flags
        AND     R1, R1, #CPU_ARM_CTRL_INT_DIS
        CMP     R1,     #CPU_ARM_CTRL_INT_DIS
        BNE     CPU_SR_Save_Loop                                ; NOT properly DISABLED (try again)
        BX      LR                                              ; DISABLED, return the original CPSR contents in R0


CPU_SR_Restore                                                  ; See Note #2
        MSR     CPSR_c, R0
        BX      LR


;********************************************************************************************************
;                                     ENABLE & DISABLE INTERRUPTS
;
; Description : Disable/Enable IRQs & FIQs.
;
; Prototypes  : void  CPU_IntEn (void);
;               void  CPU_IntDis(void);
;********************************************************************************************************

CPU_IntDis
    	MRS     R0, CPSR
    	ORR     R0, R0, #CPU_ARM_CTRL_INT_DIS                   ; Set IRQ and FIQ bits in CPSR to disable all interrupts.
    	MSR     CPSR_c, R0
    	BX      LR

CPU_IntEn
    	MRS     R0, CPSR
    	BIC     R0, R0, #CPU_ARM_CTRL_INT_DIS                   ; Clear IRQ and FIQ bits in CPSR to enable all interrupts.
    	MSR     CPSR_c, R0
    	BX      LR


;********************************************************************************************************
;                                        ENABLE & DISABLE IRQs
;
; Description : Disable/Enable IRQs.
;
; Prototypes  : void  CPU_IRQ_En (void);
;               void  CPU_IRQ_Dis(void);
;********************************************************************************************************

CPU_IRQ_Dis
    	MRS     R0, CPSR
    	ORR     R0, R0, #CPU_ARM_CTRL_IRQ_DIS                   ; Set IRQ bit in CPSR to disable IRQs.
    	MSR     CPSR_c, R0
    	BX      LR


CPU_IRQ_En
    	MRS     R0, CPSR
    	BIC     R0, R0, #CPU_ARM_CTRL_IRQ_DIS                   ; Clear IRQ bit in CPSR to enable IRQs.
    	MSR     CPSR_c, R0
    	BX      LR


;********************************************************************************************************
;                                        ENABLE & DISABLE FIQs
;
; Description : Disable/Enable FIQs.
;
; Prototypes  : void  CPU_FIQ_En (void);
;               void  CPU_FIQ_Dis(void);
;********************************************************************************************************

CPU_FIQ_Dis
    	MRS     R0, CPSR
    	ORR     R0, R0, #CPU_ARM_CTRL_FIQ_DIS                   ; Set FIQ bit in CPSR to disable FIQs.
    	MSR     CPSR_c, R0
    	BX      LR

CPU_FIQ_En
    	MRS     R0, CPSR
    	BIC     R0, R0, #CPU_ARM_CTRL_FIQ_DIS                   ; Clear FIQ bit in CPSR to enable FIQs.
    	MSR     CPSR_c, R0
    	BX      LR


;********************************************************************************************************
;                                     CPU ASSEMBLY PORT FILE END
;********************************************************************************************************

        END

