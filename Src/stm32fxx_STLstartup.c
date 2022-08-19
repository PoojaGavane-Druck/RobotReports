/**
  ******************************************************************************
  * @file    stm32fxx_STLstartup.c 
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    19-Jun-2017
  * @brief   This file contains sequence of self-test routines to be executed just
  *          after initialization of the application before main execution starts
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
#include "stm32fxx_STLparam.h"
#define ALLOC_GLOBALS
#include "stm32fxx_STLlib.h"
#include "validation.h"
#include "misra.h"

/** @addtogroup STM32FxxSelfTestLib_src
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  eSILTestTypeInvalid        = -1,
  eSILTestTypeStartup        =  0,
  eSILTestTypeBackground     =  1,
  eSILTestTypeOnDemand       =  2

}eSILTestTypes_t;


/* Private define ------------------------------------------------------------*/
#define MEM_TEST_PATTERN_1                (0x55555555UL)
#define MEM_TEST_PATTERN_2                (0xAAAAAAAAUL)
#define VAR_MEM_TYPES                     (2U) 
#define VAR_MEM_START_INDEX               (0U)
#define VAR_MEM_END_INDEX                 (1U)
#define VAR_MEM_START_END_ADDR            (2U)
#define VAR_MEM_TEST_SLICE                (8U) /* Test only 8 Words per Variable Memory Test Call*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RCC_ClkInitTypeDef RCC_ClkInitStruct;
RCC_OscInitTypeDef RCC_OscInitStruct;
IWDG_HandleTypeDef IwdgHandle = {0};
WWDG_HandleTypeDef WwdgHandle = {0};
CRC_HandleTypeDef CrcHandle = {0};
GPIO_InitTypeDef GPIO_InitStruct = {0};

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
//ErrorStatus SILSUPER_RunVarMemTest(void);
ErrorStatus SILSUPER_PerformWalkPat(uint32_t* pStartAddr, uint32_t NumOfCells, uint32_t Pattern);
ErrorStatus SILSUPER_IdentifyVarMemSlice(uint32_t* pStartAddr, uint32_t* pSliceSize);
void passTestResult(uint32_t resultType);

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/**
  * @brief  This routine is executed in case of failure is detected by one of
  *    self-test routines. The routine is empty and it has to be filled by end
  *    user to keep application safe while define proper recovery operation
  * @param  : None
  * @retval : None
  */
void FailSafePOR(void)
{
  /* disable any checking services at SystTick interrupt */
  TickCounter = TickCounterInv = 0u;

  // Enable clock for GPIO_Init for turning ON all LED's   
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, BAT_LEVEL5_PD8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, BT_RED_PE5_Pin|STATUS_RED_PE2_Pin|BT_BLUE_PE4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, BAT_LEVEL1_PF2_Pin|BAT_LEVEL2_PF4_Pin|BAT_LEVEL3_PF5_Pin|STATUS_GREEN_PF10_Pin, GPIO_PIN_RESET);
  
  /*Configure GPIO pins : PD9 BAT_LEVEL5_PD8_Pin */
  GPIO_InitStruct.Pin = BAT_LEVEL5_PD8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /*Configure GPIO pins : BT_BLUE_PE4_Pin */
  GPIO_InitStruct.Pin = BT_BLUE_PE4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
 
  /*Configure GPIO pins : STATUS_GREEN_PF10_Pin */
  GPIO_InitStruct.Pin = STATUS_GREEN_PF10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOD, BAT_LEVEL5_PD8_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOE, BT_BLUE_PE4_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOF, STATUS_GREEN_PF10_Pin, GPIO_PIN_SET);
  
#ifdef FAIL_SAFE_POR_ENABLED
  while(1)
  {
    #ifndef NO_RESET_AT_FAIL_MODE
      /* Generate system reset */
      HAL_NVIC_SystemReset();
    #else
      while(1)
      {
	#ifdef USE_INDEPENDENT_WDOG
    	  IwdgHandle.Instance = IWDG;
    	  HAL_IWDG_Refresh(&IwdgHandle);
	#endif /* USE_INDEPENDENT_WDOG */
        #ifdef USE_WINDOW_WDOG
          WwdgHandle.Instance = WWDG;
          HAL_WWDG_Refresh(&WwdgHandle);
        #endif /* USE_WINDOW_WDOG */
      }
    #endif /* NO_RESET_AT_FAIL_MODE */
  }
#endif

}

#ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
#pragma optimize = none
#endif

/******************************************************************************/
/**
  * @brief  Contains the very first test routines executed right after
  *   the reset
  * @param  : None
  *   Flash interface initialized, Systick timer ON (2ms timebase)
  * @retval : None
  */
void STL_StartUp(void)
{
  ClockStatus clk_sts;
  /* Reset of all peripherals, Initializes the Flash interface and the Systick */
  HAL_Init();
  CLEAR_ALL_TEST_RESULTS();
  /*--------------------------------------------------------------------------*/
  /*------------------- CPU registers and flags self test --------------------*/
  /*--------------------------------------------------------------------------*/
//  CLEAR_TEST();
  init_control_flow();
  /* WARNING: all registers destroyed when exiting this function (including
  preserved registers R4 to R11) while excluding stack pointer R13) */
  if (STL_StartUpCPUTest() != CPUTEST_SUCCESS)
  {
    FailSafePOR();
  }
  else  /* CPU Test OK */
  {
    passTestResult((uint32_t)CPU_TEST);
  }
  /*--------------------------------------------------------------------------*/
  /*--------------------- WDOGs functionality self test ----------------------*/
  /*--------------------------------------------------------------------------*/

//  /* two phases IWDG & WWDG test, system reset is performed here */
  //STL_WDGSelfTest();
  
  passTestResult((uint32_t)WATCHDOG_TEST);
  
  /*--------------------------------------------------------------------------*/
  /*--------------------- Switch ON PLL for maximum speed --------------------*/
  /*--------------------------------------------------------------------------*/  
  /* No Control flow check here (not safety critical) */
  /* Switch on the PLL to speed-up Flash and RAM tests */
  StartUpClock_Config();
  
  /*--------------------------------------------------------------------------*/
  /*--------------------- Invariable memory CRC check ------------------------*/
  /*--------------------------------------------------------------------------*/  
  /* Compute the 32-bit crc of the whole Flash by CRC unit except the checksum
     pattern stored at top of FLASH */

  __CRC_CLK_ENABLE();
  
  CrcHandle.Instance = CRC;
  #ifdef  CRC_UNIT_CONFIGURABLE
    CrcHandle.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_ENABLE;
    CrcHandle.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
    CrcHandle.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
    CrcHandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLED;
    CrcHandle.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;
  #endif
  HAL_CRC_Init(&CrcHandle);
  
  /* ==============================================================================*/
  /* MISRA violation of rule 11.4, 17.4 - pointer arithmetic is used for
     CRC calculation */
  #pragma diag_suppress=Pm088,Pm141

  if(1u == validation_bankCRC( FLASH_BANK_1, ( *(( uint32_t* )cFlashBank1CrcAddr ))))
  {
     FailSafePOR();
  }
 
  passTestResult((uint32_t)CRC32_TEST);

  HAL_CRC_DeInit(&CrcHandle);

  /*--------------------------------------------------------------------------*/
  /* --------------------- Variable memory functional test -------------------*/
  /*--------------------------------------------------------------------------*/  
  /* no stack operation can be performed during the test */  
  __disable_irq();
  
  /* WARNING: Stack is zero-initialized when exiting from this routine */

  if(ERROR == SILSUPER_RunVarMemTest())
  {
     FailSafePOR();
  }
  // Full RAM Test OK 
  passTestResult((uint32_t)RAM_TEST);

  /* restore interrupt capability */
  __enable_irq();

  /*--------------------------------------------------------------------------*/
  /*----------------------- Clock Frequency Self Test ------------------------*/
  /*--------------------------------------------------------------------------*/  
  /* test LSI & HSE clock systems */
  clk_sts = STL_ClockStartUpTest();
  
  if(clk_sts != FREQ_OK)
  {
      FailSafePOR();
  }
    
  passTestResult((uint32_t)CLOCK_SWITCH_TEST);  
  /*--------------------------------------------------------------------------*/
  /* --------------- Initialize stack overflow pattern ---------------------- */
  /*--------------------------------------------------------------------------*/  
  aStackOverFlowPtrn[0] = 0xEEEEEEEEuL;
  aStackOverFlowPtrn[1] = 0xCCCCCCCCuL;
  aStackOverFlowPtrn[2] = 0xBBBBBBBBuL;
  aStackOverFlowPtrn[3] = 0xDDDDDDDDuL;
  
  passTestResult((uint32_t)STACK_OVERFLOW_TEST_FLAG);

  /*--------------------------------------------------------------------------*/
  /* -----  Verify Control flow before Starting main program execution ------ */
  /*--------------------------------------------------------------------------*/
   
   /* startup test completed successfully - restart the application */
   GotoCompilerStartUp();
}

/******************************************************************************/
/**
  * @brief  Verifies the watchdog by forcing watchdog resets
  * @param  : None
  * @retval : None
  */
void STL_WDGSelfTest(void)
{
  /* ==============================================================================*/
  /* MISRA violation of rule 12.4 - side effect of && and || operators ignored */
  #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
    #pragma diag_suppress=Pm026
  #endif /* __IAR_SYSTEMS_ICC__ */

  /* start watchdogs test if one of the 4 conditions below is valid */
  if ( (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)\
   ||  (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET)\
   ||  (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST) != RESET)\
   || ((__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET) && (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) == RESET) && (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) == RESET)))
  {
    // Power-on or software reset, testing IWDG
    /* Clear all flags before resuming test */
    //__HAL_RCC_CLEAR_FLAG();

    /* Setup IWDG to minimum period */
    IwdgHandle.Instance = IWDG;
    IwdgHandle.Init.Prescaler = IWDG_PRESCALER_4;
    IwdgHandle.Init.Reload = 1U;
    #ifdef IWDG_FEATURES_BY_WINDOW_OPTION
      IwdgHandle.Init.Window = IWDG_WINDOW_DISABLE;
    #endif /* IWDG_FEATURES_BY_WINDOW_OPTION */
    /* Initialization */
    HAL_IWDG_Init(&IwdgHandle);

    /* Wait for an independent watchdog reset */
    while(1)
    { }
  }
  else  /* Watchdog test or software reset triggered by application failure */
  {
    /* If WWDG only was set, re-start the complete test (indicates a reset triggered by safety routines */
    if ((__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)  != RESET) && (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) == RESET) && (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET))
    {
      //__HAL_RCC_CLEAR_FLAG();
      // WWDG reset, re-start WDG test
      NVIC_SystemReset();
    }
    else  /* If IWDG only was set, continue the test with WWDG test*/
    {
      if ((__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)  != RESET) && (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) && (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) == RESET))
      { /* If IWDG only was set, test WWDG*/
        // IWDG reset from test or application, testing WWDG

         /* Setup WWDG to minimum period */
        __WWDG_CLK_ENABLE();
        WwdgHandle.Instance = WWDG;
        WwdgHandle.Init.Prescaler = WWDG_PRESCALER_1;
        WwdgHandle.Init.Counter = 64U;
        WwdgHandle.Init.Window = 63U;
        WwdgHandle.Init.EWIMode = WWDG_EWI_DISABLE;
        HAL_WWDG_Init(&WwdgHandle);

        while(1)
        { }
      }
      else  /* If both flags IWDG & WWDG flags are set, means that watchdog test is completed */
      {                
        if ((__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)  != RESET) && (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) && (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET))
        {
          // Not resetting the flags here as they will be used by the main code
          //__HAL_RCC_CLEAR_FLAG();
        // WWDG reset, WDG test completed
        }
        else  /* Unexpected Flag configuration, re-start WDG test */
        {
          //__HAL_RCC_CLEAR_FLAG();
          // Unexpected Flag configuration, re-start WDG test
          NVIC_SystemReset();
        } /* End of Unexpected Flag configuration */
      } /* End of normal test sequence */
    } /* End of partial WDG test (IWDG test done) */
  } /* End of part where 1 or 2 Watchdog flags are set */

  #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
    #pragma diag_default=Pm026
  #endif /* __IAR_SYSTEMS_ICC__ */
  /* ==============================================================================*/
}

/**
  * @}
  */
  
/****************************** Variable Memory Test - Start ******************/
ErrorStatus SILSUPER_RunVarMemTest(void)
{
  ErrorStatus silSysErr = ERROR;
  uint32_t startAddr = 0UL;/* Variable for Start address of Memory under test */
  uint32_t NumberOfCells = 0UL; /* Number of cells in Memory under test */
  uint32_t* pStartAddr = 0UL;
  uint32_t index_i = 0UL;
 __disable_irq();
  silSysErr = SILSUPER_IdentifyVarMemSlice(&startAddr,&NumberOfCells);
  pStartAddr = (uint32_t *)startAddr;

 while ( (0UL != pStartAddr) && (0UL != NumberOfCells) )
  {

    for(index_i = 0UL;index_i < NumberOfCells;index_i++)
    {
      /* Copy the contents of Memory Under test to a Swap Buffer.*/
      pSwapAddr[index_i ]= pStartAddr[index_i ];
    }
    /*Perform Walk-pat with on memory under test with pattern (0x55555555)*/
    silSysErr = SILSUPER_PerformWalkPat(pStartAddr , NumberOfCells, MEM_TEST_PATTERN_1);
    if( SUCCESS == silSysErr )
    {
      /*Perform Walk-pat with on memory under test with pattern (0xAAAAAAAA)*/
      silSysErr = SILSUPER_PerformWalkPat(pStartAddr , NumberOfCells, MEM_TEST_PATTERN_2);
    }
    for(index_i = 0UL;index_i < NumberOfCells;index_i++)
    {
      /* Restore the contents of Tested Memory from Swap Buffer.*/
      pStartAddr[index_i ] = pSwapAddr[index_i];
    }

    if( SUCCESS == silSysErr )
    {
     NumberOfCells = 0UL;
     silSysErr = SILSUPER_IdentifyVarMemSlice(&startAddr,&NumberOfCells);
     if((startAddr == LAST_ADDRESS_TEST) && (0UL == NumberOfCells))
     {
       silSysErr = SUCCESS;
     }
     pStartAddr = (uint32_t *)startAddr;
    }
  }
/* Enable previous interrupts here */
__enable_irq();

  return silSysErr;
}
/****************************** Variable Memory Test - End ********************/
/********************* Identify Variable Memory Slice - Start *****************/
/**
 *
 * @private
 *
 * @brief Identify Variable Memory Slice.
 *
 * This is a helper function that can be used to identify the
 * variable memory slice to be tested.
 *
 * @param[OUT] pStartAddr  -  The Start Address of the memory to test.
 * @param[OUT] pSliceSize  -  The size of the memory slice under test.
 *
 * @return ErrorStatus - System Error
 */
ErrorStatus SILSUPER_IdentifyVarMemSlice(uint32_t* pStartAddr, uint32_t* pSliceSize)
{
  ErrorStatus silSysErr = ERROR;
  static uint32_t previousSize = 0UL;    /* Indicates the size of previous Slice tested(in bytes).*/
  uint32_t TempAddr = 0UL;               /* Temporary variable to hold address, added for readabilty and clarity.*/

  /* Validate Pass parameter */
  if((NULL != pStartAddr) && (NULL != pSliceSize))
  {
    TempAddr = (uint32_t)RAM_START + previousSize;

    /*Is the identified start Address within range of that specific memory?*/
    if (TempAddr < (uint32_t)(RAM_END))
    {
      /* Test Slice identified. Update start address and size of the slice to be tested*/
      *pStartAddr = (uint32_t)RAM_START + previousSize;
      *pSliceSize = VAR_MEM_TEST_SLICE ;
      /* Update previous size in bytes*/
      previousSize = previousSize + (VAR_MEM_TEST_SLICE  * sizeof(uint32_t)); 
       silSysErr = SUCCESS;
    }
  }
  return silSysErr;
}
/********************* Identify Variable Memory Slice - End *******************/
/**
 *
 * @private
 *
 * @brief Perform Walk-Pat Test.
 *
 * This function performs the Walk-Pat test on the specified variable memory range.\n
 * \nWalk-Pat Algorithm\n
 * (1)Initialize the memory under test with a pattern.\n
 * (2)Base cell = First cell.\n
 * (3)Flip the value of base cell.\n
 * (4)Read and verify all other cells.\n
 * (5)Read and verify base cell.\n
 * (6)Flip the value of base cell again.\n
 * (7)Base cell = Next cell.\n
 * (8)Repeat steps (3) to (7) until entire address range complete.\n
 *
 * @note Step 1 in the above algorithm is done by the calling function to reduce complexity.
 *
 * @param [IN,OUT] pStartAddr  -  Starting Address of the memory to test.
 * @param [IN] NumOfCells - Size of memory to test.
 * @param [IN] Pattern - Pattern for test.
 *
 * @attention MISRA Rule 17.4 Disabled - This Rule will remain Disabled for this file.
 * Re: Function returning pointer to array - Rule 17.4
 * by misra-c » Fri Jul 09, 2010 12:27 pm
 * The MISRA C working group agrees that this is a problem with the current wording for Rule 17.4.
 * The problem is not limited to returns from functions.
 * For example:
 * int_32 * select_row (int_32 arr[3][2], uint_32 index)
 * |
 * return arr[index]
 * |
 * int_32 array[3][2] = ||1, 2|, |3, 4|, |5, 6||
 * uint_32 i
 * for ( i = 0U; i < 3U; i++)
 * |
 * int_32 *row = select_row(array, i)
 *   ... row[0] ....   violates 17.4
 * |
 * In many cases, returning a pointer to an array is best avoided as suggested by a previous response to this question.
 * However, it is appreciated that slicing an array may be required and it may be more readable to use a pointer to the slice.
 * For this reason, the next version of the MISRA C guidelines to likely to allow such indexing.
 * Indexing such slices is non-compliant in this version of MISRA C and will need a deviation.
 * 
 * @return ErrorStatus
 */
ErrorStatus SILSUPER_PerformWalkPat(uint32_t* pStartAddr, uint32_t NumOfCells, uint32_t Pattern)
{
  ErrorStatus silSysErr = SUCCESS;
  /*Perform Step2 of Walk-pat algorithm.
    Base cell = First cell.*/
  uint32_t Index_bc = 0UL; /*Base cell index*/
  uint32_t Index_oc = 0UL; /*Other cell index*/
  uint32_t flip_pattern = (~Pattern);

  /*MISRA Rule 17.4 Disabled*/
  MISRAC_DISABLE

    for(Index_oc = 0UL;Index_oc < NumOfCells;Index_oc++)
    {
      /*Perform Step1 of Walk-pat algorithm. Fill the Memory under test with a pattern*/
      pStartAddr[Index_oc ] = Pattern;
    }
  silSysErr = SUCCESS;
  /* @Note: The loop break on error could be omitted to speed up code further.*/
  while( ( Index_bc < NumOfCells ) && (SUCCESS == silSysErr ) )
  {
    /*Perform Step3 of Walk-pat algorithm. Flip the value of base cell.*/
    pStartAddr[Index_bc] = flip_pattern ;   
    /*Read and verify all other cells before base cell.*/
    for( Index_oc = 0UL;Index_oc < Index_bc; Index_oc++)
    {
      if(Pattern != pStartAddr[Index_oc])
      {
        silSysErr = ERROR;
      }
    }
    /* Note both index are equal here and point to base cell. */

    /*Read and verify all  cells after base cell.*/
    for( Index_oc = Index_bc + 1UL;Index_oc < NumOfCells; Index_oc++)
    {
      if(Pattern != pStartAddr[Index_oc])
      {
        silSysErr = ERROR;
      }
    }
    /* Read and verify base cell.*/
    if(flip_pattern  != pStartAddr[Index_bc])
    {
      silSysErr = ERROR;
    }

    /*Perform Step6 of Walk-pat algorithm. Flip the value of base cell again*/
    pStartAddr[Index_bc] = Pattern;

    /*Perform Step7 of Walk-pat algorithm. Base cell = Next cell.*/
    Index_bc += 1UL;
  }
  /*MISRA Rule 17.4 Enabled*/
  MISRAC_ENABLE
    return silSysErr;
}
/********************* Memory Walk-Pat Test - End *****************************/

void passTestResult(uint32_t resultType)
{
  uint32_t testFlagUpdate = 0UL;        // used to update test result in flag
  testFlagUpdate = selfTestFlag[0];
  
  testFlagUpdate |= (1 << resultType);
  selfTestFlag[0] = testFlagUpdate;

}

/**
  * @brief  This routine is used return captured test results
  * @param  : None
  * @retval : test results: uint32_t
  */
uint32_t getBistTestResult(void)
{
  uint32_t testFlagResult = 0UL;        // used to update test result in flag
  testFlagResult = selfTestFlag[0];

  return(testFlagResult);
}
/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
