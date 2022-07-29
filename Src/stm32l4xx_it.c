/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_it.h"
/* Private includes ----------------------------------------------------------*/
#include <os.h>
/* USER CODE BEGIN Includes */
#include "stm32fxx_STLparam.h"
#include "stm32fxx_STLlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile static uint16_t tmpCC1_last;   /* keep last TIM16/Chan1 captured value */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;
extern LPTIM_HandleTypeDef hlptim1;
extern OSPI_HandleTypeDef hospi1;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SMBUS_HandleTypeDef hsmbus1;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern WWDG_HandleTypeDef hwwdg;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/
/**
  * @brief This function handles I2C1 event interrupt.
  */
void I2C1_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_EV_IRQn 0 */
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  /* USER CODE END I2C1_EV_IRQn 0 */
  HAL_SMBUS_EV_IRQHandler(&hsmbus1);
  /* USER CODE BEGIN I2C1_EV_IRQn 1 */
  OSIntExit();
  /* USER CODE END I2C1_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C1 error interrupt.
  */
void I2C1_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_ER_IRQn 0 */
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  /* USER CODE END I2C1_ER_IRQn 0 */
  HAL_SMBUS_ER_IRQHandler(&hsmbus1);
  /* USER CODE BEGIN I2C1_ER_IRQn 1 */
  OSIntExit();
  /* USER CODE END I2C1_ER_IRQn 1 */
}


/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  /* USER CODE END EXTI0_IRQn 0 */
  //pinZeroInterruptHandler(GPIO_PIN_0);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */
  OSIntExit();
  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line2 interrupt.
  */
void EXTI2_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI2_IRQn 0 */
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  /* USER CODE END EXTI2_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  /* USER CODE BEGIN EXTI2_IRQn 1 */
  OSIntExit();
  /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */
  OSIntExit();
  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */
  OSIntExit();
  /* USER CODE END EXTI15_10_IRQn 1 */
}

void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */
 // MX_USB_DEVICE_GetConnected();
  
#if 0
  // Turn USB_PEN GPIO on immediately when client USB is connected but use HW timer to delay turning off by 1s when disconnnected
  HAL_TIM_Base_Stop_IT(&htim3);
  if (MX_USB_DEVICE_GetConnected())
  {
    HAL_GPIO_WritePin(USB_PEN_GPIO_GPIO_Port, USB_PEN_GPIO_Pin, GPIO_PIN_SET);
  }
  else
  {
    HAL_TIM_Base_Start_IT(&htim3);
  }
#endif
    OSIntExit();
  /* USER CODE END OTG_FS_IRQn 1 */
}
/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}


/**
  * @brief This function handles SPI1 global interrupt.
  */
void SPI1_IRQHandler(void)
{
  /* USER CODE BEGIN SPI1_IRQn 0 */

  /* USER CODE END SPI1_IRQn 0 */
  HAL_SPI_IRQHandler(&hspi1);
  /* USER CODE BEGIN SPI1_IRQn 1 */

  /* USER CODE END SPI1_IRQn 1 */
}

/**
  * @brief This function handles SPI2 global interrupt.
  */
void SPI2_IRQHandler(void)
{
  /* USER CODE BEGIN SPI2_IRQn 0 */
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  /* USER CODE END SPI2_IRQn 0 */
  HAL_SPI_IRQHandler(&hspi2);
  /* USER CODE BEGIN SPI2_IRQn 1 */
  OSIntExit();
  /* USER CODE END SPI2_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */

/**
  * @brief This function handles OCTOSPI1 global interrupt.
  */
void OCTOSPI1_IRQHandler(void)
{
  /* USER CODE BEGIN OCTOSPI1_IRQn 0 */
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
  
  /* USER CODE END OCTOSPI1_IRQn 0 */
  HAL_OSPI_IRQHandler(&hospi1);
  /* USER CODE BEGIN OCTOSPI1_IRQn 1 */
  OSIntExit();
  /* USER CODE END OCTOSPI1_IRQn 1 */
}

/* USER CODE BEGIN 1 */
/**
  * @brief  This function handles TIM16 global interrupt request.
  * @param  : None
  * @retval : None
  */
void TIM1_UP_TIM16_IRQHandler(void)
{
  uint16_t tmpCC1_last_cpy;
   
  if ((TIM16->SR & TIM_SR_CC1IF) != 0u )
  {
    /* store previous captured value */
    tmpCC1_last_cpy = tmpCC1_last; 
    /* get currently captured value */
    tmpCC1_last = (uint16_t)(TIM16->CCR1);
    /* The CC1IF flag is already cleared here be reading CCR1 register */

    /* overight results only in case the data is required */
    if (LSIPeriodFlag == 0u)
    {
      /* take correct measurement only */
      if ((TIM16->SR & TIM_SR_CC1OF) == 0u)
      {
        /* Compute period length */
        PeriodValue = ((uint32_t)(tmpCC1_last) - (uint32_t)(tmpCC1_last_cpy)) & 0xFFFFuL;
        PeriodValueInv = ~PeriodValue;
      
        /* Set Flag tested at main loop */
        LSIPeriodFlag = 0xAAAAAAAAuL;
        LSIPeriodFlagInv = 0x55555555uL;
      }
      else
      {
        /* ignore computation in case of IC overflow */
        TIM16->SR &= (uint16_t)(~TIM_SR_CC1OF);
      }
    }
    /* ignore computation in case data is not required */
  }
}


/******************************************************************************/
/**
  * @brief Configure TIM16 to measure LSI period
  * @param  : None
  * @retval : ErrorStatus = (ERROR, SUCCESS)
  */
ErrorStatus STL_InitClock_Xcross_Measurement(void)
{
  ErrorStatus result = SUCCESS;
  TIM_HandleTypeDef  tim_capture_handle;
  TIM_IC_InitTypeDef tim_input_config;
  
  /*## Enable peripherals and GPIO Clocks ####################################*/
  /* TIMx Peripheral clock enable */
  __TIM16_CLK_ENABLE();
  
  /*## Configure the NVIC for TIMx ###########################################*/
  HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 4u, 0u);
  
  /* Enable the TIM16 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
  
  /* TIM16 configuration: Input Capture mode ---------------------
  The LSI oscillator is connected to TIM16 CH1.
  The Rising edge is used as active edge, ICC input divided by 8
  The TIM16 CCR1 is used to compute the frequency value. 
  ------------------------------------------------------------ */
  tim_capture_handle.Instance = TIM16;
  tim_capture_handle.Init.Prescaler         = 0u; 
  tim_capture_handle.Init.CounterMode       = TIM_COUNTERMODE_UP;  
  tim_capture_handle.Init.Period            = 0xFFFFFFFFul; 
  tim_capture_handle.Init.ClockDivision     = 0u;     
  tim_capture_handle.Init.RepetitionCounter = 0u;
  /* define internal HAL driver status here as handle structure is defined locally */
  __HAL_RESET_HANDLE_STATE(&tim_capture_handle);
  if(HAL_TIM_IC_Init(&tim_capture_handle) != HAL_OK)
  {
    /* Initialization Error */
    result = ERROR;
  }
  /* Connect internally the TIM16_CH1 Input Capture to the LSI clock output */
  HAL_TIMEx_RemapConfig(&tim_capture_handle, TIM_TIM16_TI1_LSI);
  
  /* Configure the TIM16 Input Capture of channel 1 */
  tim_input_config.ICPolarity  = TIM_ICPOLARITY_RISING;
  tim_input_config.ICSelection = TIM_ICSELECTION_DIRECTTI;
  tim_input_config.ICPrescaler = TIM_ICPSC_DIV8;
  tim_input_config.ICFilter    = 0u;
  if(HAL_TIM_IC_ConfigChannel(&tim_capture_handle, &tim_input_config, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Initialization Error */
    result = ERROR;
  }
  
  /* Reset the flags */
  tim_capture_handle.Instance->SR = 0u;
  LSIPeriodFlag = 0u;
  
  /* Start the TIM Input Capture measurement in interrupt mode */
  if(HAL_TIM_IC_Start_IT(&tim_capture_handle, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Initialization Error */
    result = ERROR;
  }
  return(result);
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
