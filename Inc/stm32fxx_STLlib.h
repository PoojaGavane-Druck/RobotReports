/**
  ******************************************************************************
  * @file    stm32fxx_STLlib.h 
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    19-June-2017
  * @brief   This file references all header files of the Self Test Library
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32Fxx_STL_LIB_H
#define __STM32Fxx_STL_LIB_H

/* Includes ------------------------------------------------------------------*/
#include "stm32fxx_STLparam.h"
/* Include Class B variables */
  /* ==============================================================================*/
  /* MISRA violation of rule 8.5 - alocation of variables done via header file jus at this place */
	#ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
		#pragma diag_suppress=Pm123
	#endif /* __IAR_SYSTEMS_ICC__ */
#include "stm32fxx_STLclassBvar.h"
	#ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
		#pragma diag_default=Pm123
	#endif /* __IAR_SYSTEMS_ICC__ */
  /* ==============================================================================*/

/* Self Test library routines main flow after initialization and at run */
#include "stm32fxx_STLstartup.h"
//#include "stm32fxx_STLmain.h"

/* Cortex-M4 CPU test */
#include "stm32fxx_STLcpu.h"

/* Clock frequency test */
#include "stm32fxx_STLclock.h"

/* Invariable memory test */
//#include "stm32fxx_STLcrc32.h"

/* Variable memory test */
//#include "stm32fxx_STLRam.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern UART_HandleTypeDef UartHandle;
extern IWDG_HandleTypeDef IwdgHandle;
//#ifdef WATCHDOG_ENABLE
extern WWDG_HandleTypeDef WwdgHandle;
//#endif
extern CRC_HandleTypeDef CrcHandle;
extern RCC_ClkInitTypeDef RCC_ClkInitStruct;
extern RCC_OscInitTypeDef RCC_OscInitStruct;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __stm32fxx_STL_LIB_H */

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
