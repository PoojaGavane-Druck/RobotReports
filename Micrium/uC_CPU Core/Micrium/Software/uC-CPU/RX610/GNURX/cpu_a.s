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
;                                      Renesas RX610 Specific code
;                                          GNU RX C Compiler
;
; Filename : cpu_a.s
; Version  : V1.31.05
;********************************************************************************************************


;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

	.global     _set_ipl
	.global     _get_ipl
	
	.text


;********************************************************************************************************
;                                        set_ipl() & get_ipl()
;
; Description: Set or retrieve interrupt priority level.
;********************************************************************************************************

_set_ipl:
	PUSH R2
    MVFC PSW, R2
	AND  #-0F000001H, R2
	SHLL #24, R1
	OR   R1, R2 	
    MVTC R2, PSW
	POP  R2
	RTS
	
_get_ipl:
    MVFC PSW, R1
	SHLR #24, R1
	AND  #15, R1
	RTS


    .END

