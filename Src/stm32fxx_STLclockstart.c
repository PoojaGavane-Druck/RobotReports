/**
  ******************************************************************************
  * @file    stm32fxx_STLclockstart.c 
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    19-Jun-2017
  * @brief   This file contains the test function for clock circuitry
  *          and wrong frequency detection at start-up.
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


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32fxx_STLlib.h"


/** @addtogroup STM32FxxSelfTestLib_src
  * @{
  */ 


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/ 
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Start up the internal and external oscillators and verifies
  *   that clock source is within the expected range
  * @param  : None
  * @retval : ClockStatus = (LSI_START_FAIL, HSE_START_FAIL,
  *   HSI_HSE_SWITCH_FAIL, XCROSS_CONFIG_FAIL, EXT_SOURCE_FAIL, FREQ_OK)
  */
ClockStatus STL_ClockStartUpTest(void)
{
  ClockStatus clck_sts = TEST_ONGOING;
  CtrlFlowCnt += CLOCK_TEST_CALLEE;

  /* Start low speed internal (LSI) oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* LSI clock fails */
    clck_sts = LSI_START_FAIL;
  }
  
#ifdef HSE_CLOCK_APPLIED  
  /* if LSI started OK - Start High-speed external oscillator (HSE) */
  if(clck_sts == TEST_ONGOING)
  {
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    /* Use HSEState parameter RCC_HSE_BYPASS when external generator is applied */
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      /* HSE clock fails */
      clck_sts = HSE_START_FAIL;
    }
  }
  
  /* if HSE started OK - enable CSS */
  if(clck_sts == TEST_ONGOING)
  {
    HAL_RCC_EnableCSS();
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
       clocks dividers */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
      /* switch to HSE clock fails */
      clck_sts = HSI_HSE_SWITCH_FAIL;
    }
  }
  
  /*-------------- Start Reference Measurement -------------------------------*/
  if(clck_sts == TEST_ONGOING)
  {  
    /* Configure dedicated timer to measure LSI period */
    if(STL_InitClock_Xcross_Measurement() == ERROR)
    {
      clck_sts = XCROSS_CONFIG_FAIL;
    }
    else
    {   
      /* Wait for two subsequent LSI periods measurements */
      LSIPeriodFlag = 0u;
      while (LSIPeriodFlag == 0u)
      { }
      LSIPeriodFlag = 0u;
      while (LSIPeriodFlag == 0u)
      { }

      /*-------------------- HSE measurement check -------------------------*/
      if (PeriodValue < HSE_LimitLow(HSE_VALUE))
      {
        /* Sub-harmonics: HSE -25% below expected */
        clck_sts = EXT_SOURCE_FAIL;
      }
      else if (PeriodValue > HSE_LimitHigh(HSE_VALUE))
      {
        /* Harmonics: HSE +25% above expected */
        clck_sts = EXT_SOURCE_FAIL;
      }
    }
  }
#else
  if(clck_sts == TEST_ONGOING)
  {
    /* Configure dedicated timer to measure LSI period */
    if(STL_InitClock_Xcross_Measurement() == ERROR)
    {
      clck_sts = XCROSS_CONFIG_FAIL;
    }
    else
    {   
      /* Wait for two subsequent LSI periods measurements */
      LSIPeriodFlag = 0u;
      while (LSIPeriodFlag == 0u)
      { }
      LSIPeriodFlag = 0u;
      while (LSIPeriodFlag == 0u)
      { }

      /*-------------------- HSI measurement check -------------------------*/
      if (PeriodValue < HSI_LimitLow(SYSTCLK_AT_STARTUP))
      {
        /* HSI -20% below expected */
        clck_sts = EXT_SOURCE_FAIL;
      }
      else if (PeriodValue > HSI_LimitHigh(SYSTCLK_AT_STARTUP))
      {
        /* HSI +20% above expected */
        clck_sts = EXT_SOURCE_FAIL;
      }
      else
      { }
    }
  }
#endif
  
  /* Switch back HSI internal clock at any case */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, MAX_FLASH_LATENCY) != HAL_OK)
  {
    /* switch to HSE clock fails */
    clck_sts = HSE_HSI_SWITCH_FAIL;
  }
  else
  {
    /* Switch off PLL */
    RCC_OscInitStruct.OscillatorType = 0u;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      /* PLL off fails */
      clck_sts = PLL_OFF_FAIL;
    }
    else
    {
      if(clck_sts == TEST_ONGOING)
      {
        /* the test was finished correctly */
        clck_sts = FREQ_OK;
      }
    }
  }
  
  CtrlFlowCntInv -= CLOCK_TEST_CALLEE;
  
  return(clck_sts);
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
