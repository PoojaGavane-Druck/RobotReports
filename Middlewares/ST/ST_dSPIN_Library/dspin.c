/**
  ******************************************************************************
  * @file    dspin.c 
  * @author  IPC Rennes
  * @version V2.0
  * @date    October 4, 2013
  * @brief   dSPIN (L6470 and L6472) product related routines
  * @note    (C) COPYRIGHT 2013 STMicroelectronics
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "dspin.h"
#include "dspin_config.h"


/** @addtogroup  dSPIN FW library interface
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;
SPI_InitTypeDef SPI_InitStructure;
uint8_t spiTxBursts[dSPIN_CMD_ARG_MAX_NB_BYTES][NUMBER_OF_SLAVES];
uint8_t spiRxBursts[dSPIN_CMD_ARG_MAX_NB_BYTES][NUMBER_OF_SLAVES];
uint8_t arrayTxBytes[NUMBER_OF_SLAVES];
uint32_t arrayValues[NUMBER_OF_SLAVES];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Inserts a delay time.
  * @param  nCount specifies the delay time length.
  * @retval None
  */
void dSPIN_Delay(__IO uint32_t nCount)
{
  for(; nCount!= 0;nCount--);
}

/**
  * @brief  Resets DSPIN and puts it into standby mode
  * @param  None
  * @retval None
  */
void dSPIN_Reset_And_Standby(void)
{
#if (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY))  
  HAL_GPIO_WritePin(dSPIN_STBY_RESET_Port, 3, (GPIO_PinState)(RESET));
  dSPIN_Delay(10000);
  HAL_GPIO_WritePin(dSPIN_STBY_RESET_Port, 3, (GPIO_PinState)(SET));
#endif /* (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY)) */
}

/**
  * @brief  Toggles a GPIO output
  * @param  GPIOx gpio port
  * @param  GPIO_Pin pin number of the Gpio to toggle
  * @retval None
  */
void dSPIN_Gpio_Toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  //if (GPIO_ReadOutputDataBit(GPIOx, GPIO_Pin)!=Bit_RESET)
  if (GPIO_ReadOutputDataBit(GPIOx, GPIO_Pin)!=(uint16_t)(0))
  {
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, (GPIO_PinState)SET);
  }
  else
  {
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, (GPIO_PinState)RESET);
  }
}

#if 0 /* Added by Makarand, this function is not used */

/**
  * @brief  Initializes uC peripherals, GPIOs, clocks, interrupts channels used by dSPIN.
  * @param  None
  * @retval None
  */
void dSPIN_Peripherals_Init(void)
{
  /* Used peripherals clock enable -------------------------------------------*/
  RCC_APB1PeriphClockCmd(dSPIN_PERIPHERAL_CLKs_APB1, ENABLE);
  RCC_APB2PeriphClockCmd(dSPIN_PERIPHERAL_CLKs_APB2, ENABLE);
  
  /* Configure pins used by dSPIN --------------------------------------------*/
#if (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY))
  /* Configure on-board power LED ------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = POWER_LED_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(POWER_LED_Port, &GPIO_InitStructure);
  HAL_GPIO_WritePin(POWER_LED_Port, POWER_LED_Pin);
  /* Configure on-board status LED -----------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = STATUS_LED_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(STATUS_LED_Port, &GPIO_InitStructure);
  /* Configure STBY_RESET GPIO connected to dSPIN STBY_RESET pin*/
  GPIO_InitStructure.GPIO_Pin = dSPIN_STBY_RESET_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(dSPIN_STBY_RESET_Port, &GPIO_InitStructure);
#endif /* (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY)) */ 
#ifdef STEVAL_PCC009V2 /* Only if PCC009V2 evalboard is used ---------------*/
  /* Configure Port C GPIO pin 2 connected to keypad button "*" */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  /* Configure Port C GPIO pin 3 connected to keypad button "7" */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  /* Configure Port C GPIO pin 6 connected to keypad button "4" */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif /* STEVAL_PCC009V2 */    
#ifdef ST_DSPIN_6470H_DISCOVERY /* Only if DISCOVERY board is used -----------*/
  /* Configure on-board busy LED -----------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = LED_BUSY_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LED_BUSY_Port, &GPIO_InitStructure);
  /* Configure on-board spare LED --------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = LED_SPARE_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LED_SPARE_Port, &GPIO_InitStructure);
  /* Configure GPIO connected to S1 button via BUTTON_A wire */
  GPIO_InitStructure.GPIO_Pin = BUTTON_A_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(BUTTON_A_Port, &GPIO_InitStructure);
  /* Configure GPIO connected to S3 button via BUTTON_B wire */
  GPIO_InitStructure.GPIO_Pin = BUTTON_B_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(BUTTON_B_Port, &GPIO_InitStructure);
  /* Configure SW_MOTOR GPIO connected to J8 jumper */
  GPIO_InitStructure.GPIO_Pin = SW_MOTOR_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(SW_MOTOR_Port, &GPIO_InitStructure);
  /* Configure SW GPIO connected to dSPIN SW pin*/
  GPIO_InitStructure.GPIO_Pin = dSPIN_SW_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(dSPIN_SW_Port, &GPIO_InitStructure);
  HAL_GPIO_WritePin(dSPIN_SW_Port, dSPIN_SW_Pin);
#endif /* ST_DSPIN_6470H_DISCOVERY */
      
  /* Configure SPI pin: SCK --------------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = dSPIN_SCK_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(dSPIN_SCK_Port, &GPIO_InitStructure);

  /* Configure SPI pin: MOSI -------------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = dSPIN_MOSI_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(dSPIN_MOSI_Port, &GPIO_InitStructure);

  /* Configure SPI pin: nSS --------------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = dSPIN_nSS_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(dSPIN_nSS_Port, &GPIO_InitStructure);

  /* Configure SPI pin: MISO -------------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = dSPIN_MISO_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(dSPIN_MISO_Port, &GPIO_InitStructure);
  
  /* Configure dSPIN - Busy pin ----------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = dSPIN_BUSY_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(dSPIN_BUSY_Port, &GPIO_InitStructure);	  
  				
  /* Configure dSPIN - Flag pin ----------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = dSPIN_FLAG_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(dSPIN_FLAG_Port, &GPIO_InitStructure);
  
  /* Configure PWM connected to dSPin STCK -----------------------------------*/  
  GPIO_InitStructure.GPIO_Pin = dSPIN_PWM1_Pin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(dSPIN_PWM1_Port, &GPIO_InitStructure);	  
  
  /* SPI configuration ------------------------------------------------------*/
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(dSPIN_SPI, &SPI_InitStructure);
  
  /* Enable SPI */
  SPI_Cmd(dSPIN_SPI, ENABLE);

  /* Interrupt Channel configuration and enable */
  dSPIN_Interrupt_Channel_Config();

}
#endif /* Added by Makarand, this function is not used */

#if 0 /* Added by Makarand, this function is not used */
/**
  * @brief  Interrupt channel configuration and enable
  * @param  None
  * @retval None
  */
void dSPIN_Interrupt_Channel_Config(void)
{
 
  NVIC_InitTypeDef NVIC_InitStructure;
#if (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY))
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
#endif /* (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY)) */
#if defined(STEVAL_PCC009V2) 
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif /* defined(STEVAL_PCC009V2) */
#if defined(ST_DSPIN_6470H_DISCOVERY)
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif /* defined(ST_DSPIN_6470H_DISCOVERY) */
  
}
#endif /* Added by Makarand, this function is not used */

#if 0 /* Added by Makarand, this function is not used */
/**
  * @brief  Led On and Off
  * @param  None
  * @retval None
  */
void dSPIN_Led_Check(void)
{
#ifdef STEVAL_PCC009V2 /* Only if PCC009V2 evalboard is used ---------------*/
  dSPIN_Delay(0x00100000);
  HAL_GPIO_WritePin(STATUS_LED_Port, STATUS_LED_Pin);
  dSPIN_Delay(0x00100000);  
  HAL_GPIO_WritePin(STATUS_LED_Port, STATUS_LED_Pin);
  dSPIN_Delay(0x00100000);
#endif /* STEVAL_PCC009V2 */    
#ifdef ST_DSPIN_6470H_DISCOVERY /* Only if DISCOVERY board is used -----------*/
  dSPIN_Delay(0x00100000);
  HAL_GPIO_WritePin(STATUS_LED_Port, STATUS_LED_Pin);
  dSPIN_Delay(0x00100000);  
  HAL_GPIO_WritePin(LED_BUSY_Port, LED_BUSY_Pin);
  dSPIN_Delay(0x00100000);
  HAL_GPIO_WritePin(LED_SPARE_Port, LED_SPARE_Pin);
  dSPIN_Delay(0x00100000);
  HAL_GPIO_WritePin(STATUS_LED_Port, STATUS_LED_Pin);
  HAL_GPIO_WritePin(LED_BUSY_Port, LED_BUSY_Pin);
  HAL_GPIO_WritePin(LED_SPARE_Port, LED_SPARE_Pin);
  dSPIN_Delay(0x00100000);
#endif /* ST_DSPIN_6470H_DISCOVERY */  
}
#endif /* Added by Makarand, this function is not used */


#if 0 /* Added by Makarand, this function is not used */
/**
  * @brief  GPIO config to manage FLAG signal as an interrupt
  * @param  None
  * @retval None
  */
void dSPIN_Flag_Interrupt_GPIO_Config(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;

#if defined(STEVAL_PCC009V2)
  /* Selects the GPIO pin 11 to be used as an EXTI Line */
  /* STM32F10X PB11 pin connected to L6470 FLAG pin */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);

  /* Configure EXTI Line11 to generate an interrupt on rising and falling edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line11;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line11);
#endif /* defined(STEVAL_PCC009V2) */
#if defined(ST_DSPIN_6470H_DISCOVERY)
  /* Selects the GPIO pin 10 to be used as an EXTI Line */
  /* STM32F10X PB10 pin connected to L6470 BUSY\SYNC pin  */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);

  /* Configure EXTI Line10 to generate an interrupt on falling edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line10);
#endif /* defined(ST_DSPIN_6470H_DISCOVERY) */
}
#endif /* Added by Makarand, this function is not used */

#if 0 /* Added by Makarand, this function is not used */
/**
  * @brief  GPIO config to manage BUSY signal as an interrupt
  * @param  None
  * @retval None
  */
void dSPIN_Busy_Interrupt_GPIO_Config(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;

#if defined(STEVAL_PCC009V2)
  /* Selects the GPIO pin 10 to be used as an EXTI Line */
  /* STM32F10X PB10 pin connected to L6470 BUSY\SYNC pin  */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);

  /* Configure EXTI Line10 to generate an interrupt on falling edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line10);
#endif /* defined(STEVAL_PCC009V2) */
#if defined(ST_DSPIN_6470H_DISCOVERY)
  /* Selects the GPIO pin 11 to be used as an EXTI Line */
  /* STM32F10X PB11 pin connected to L6470 BUSY\SYNC pin */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);

  /* Configure EXTI Line11 to generate an interrupt on rising and falling edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line11;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line11);
#endif /* defined(ST_DSPIN_6470H_DISCOVERY) */
}
#endif /* Added by Makarand, this function is not used */

#if 0 /* Added by Makarand, this function is not used */
/**
  * @brief  Disabling of external line interrupt corresponding to BUSY\SYNC GPIO
  * @param  None
  * @retval None
  */
void dSPIN_Busy_Interrupt_GPIO_DeConfig(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;

#if defined(STEVAL_PCC009V2)
  /* Disable EXTI Line10 interrupt */
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);
#endif /* defined(STEVAL_PCC009V2) */
#if defined(ST_DSPIN_6470H_DISCOVERY)
  /* Disable EXTI Line11 interrupt */
  EXTI_InitStructure.EXTI_Line = EXTI_Line11;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);
#endif /* defined(ST_DSPIN_6470H_DISCOVERY) */
}
#endif /* Added by Makarand, this function is not used */

#if 0 /* Added by Makarand, this function is not used */
/**
  * @brief  Board buttons GPIO configuration to be used as EXTI lines 
  * and config of the EXTI lines.
  * @param  None
  * @retval None
  */
void dSPIN_Buttons_Interrupts_GPIO_Config(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;

#if defined(STEVAL_PCC009V2)
  /* Selects the Port C GPIO pins 2, 3 and 6 to be used as EXTI Lines */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource3);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);

  /* Configure EXTI Line2 to generate an interrupt on rising edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line2);
  
  /* Configure EXTI Line3 to generate an interrupt on rising edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line3;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line3);
  
  /* Configure EXTI Line6 to generate an interrupt on rising edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line6;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line6);
#endif /* defined(STEVAL_PCC009V2) */
#if defined(ST_DSPIN_6470H_DISCOVERY)
  /* Select the Port A GPIO pins 1 and 2 to be used as EXTI Lines */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);

  /* Configure EXTI Line1 to generate an interrupt on rising edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line1);
  
  /* Configure EXTI Line2 to generate an interrupt on rising edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line2);  
#endif /* defined(ST_DSPIN_6470H_DISCOVERY) */
}
#endif

#if 0 /* Added by Makarand, this function is not used */
/**
  * @brief  Switch motor GPIO configuration to be used as EXTI line 
  * and config of the EXTI line.
  * @param  None
  * @retval None
  */
void dSPIN_Switch_Motor_Interrupt_Config(void)
{
#if defined(ST_DSPIN_6470H_DISCOVERY)
  EXTI_InitTypeDef EXTI_InitStructure;
  /* Select the Port A GPIO pin 0 as EXTI Line */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
  /* Configure EXTI Line0 to generate an interrupt on falling and rising edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line0);
#endif /* defined(ST_DSPIN_6470H_DISCOVERY) */
}
#endif /* Added by Makarand, this function is not used */

#if 0 /* Added by Makarand, this function is not used */
/**
  * @brief  Enable a PWM on the STCK pin from STM32 
  * @param  Period to be set (PWM Freq = 1MHZ/Period)
  * @retval None
  */
void dSPIN_PWM_Enable(uint16_t Period)
{
#if (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY))
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  TIM_OCInitTypeDef TIM_OCInitStruct;        
  RCC_ClocksTypeDef RCC_Clocks;
  
  /*Get System Clock frequency */
  RCC_GetClocksFreq(&RCC_Clocks);    
  
  /* Time base configuration */
  TIM_TimeBaseStructInit( &TIM_TimeBaseInitStruct );
  /* Set Prescaler to have a timer clock of 1MHZ */
  TIM_TimeBaseInitStruct.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency/1000000)-1;
  /* PWM Frequency will be equal to 1MHZ/Period */
  TIM_TimeBaseInitStruct.TIM_Period = Period-1; 

  TIM_TimeBaseInit(TIM_PWM, &TIM_TimeBaseInitStruct);

  /* PWM1 Mode configuration */
  TIM_OCStructInit( &TIM_OCInitStruct );
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStruct.TIM_Pulse = Period / 2; //range from  0 to TIM_Period, 
                                         //50% duty cycle is equal to Period/2
#if defined(STEVAL_PCC009V2)  
  /* Channel 3 */
  TIM_OC3Init(TIM_PWM, &TIM_OCInitStruct);
#endif
#if defined(ST_DSPIN_6470H_DISCOVERY)   
  /* Channel 4 */
  TIM_OC4Init(TIM_PWM, &TIM_OCInitStruct);
#endif
  
  /* TIM_PWM enable or disable counter */
  TIM_Cmd(TIM_PWM, ENABLE);
#endif /* (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY)) */
}

#endif /* Added by Makarand, this function is not used */

#if 0/* Added by Makarand, this function is not used */

/**
  * @brief  Disable PWM on the STCK pin from STM32 
  * @param  None
  * @retval None
  */
void dSPIN_PWM_DISABLE(void)
{
#if (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY))
  /* TIM_PWM disable counter */
  TIM_Cmd(TIM_PWM, DISABLE);
#endif /* (defined(STEVAL_PCC009V2) || defined(ST_DSPIN_6470H_DISCOVERY)) */
}
#endif /* Added by Makarand, this function is not used */

/**
  * @brief  Fills-in dSPIN configuration structure with default values.
  * @param  dSPIN_RegsStruct structure address (pointer to struct)
  * @retval None
  */
void dSPIN_Regs_Struct_Reset(dSPIN_RegsStruct_TypeDef* dSPIN_RegsStruct)
{
	dSPIN_RegsStruct->ABS_POS = 0;
	dSPIN_RegsStruct->EL_POS = 0;
	dSPIN_RegsStruct->MARK = 0;
	dSPIN_RegsStruct->ACC = 0x08A;
	dSPIN_RegsStruct->DEC = 0x08A;
	dSPIN_RegsStruct->MAX_SPEED = 0x041;
	dSPIN_RegsStruct->MIN_SPEED = 0;
	dSPIN_RegsStruct->FS_SPD = 0x027;
#if defined(L6470)
	dSPIN_RegsStruct->KVAL_HOLD = 0x29;
	dSPIN_RegsStruct->KVAL_RUN = 0x29;
	dSPIN_RegsStruct->KVAL_ACC = 0x29;
	dSPIN_RegsStruct->KVAL_DEC = 0x29;
	dSPIN_RegsStruct->INT_SPD = 0x0408;
	dSPIN_RegsStruct->ST_SLP = 0x19;
	dSPIN_RegsStruct->FN_SLP_ACC = 0x29;
	dSPIN_RegsStruct->FN_SLP_DEC = 0x29;
	dSPIN_RegsStruct->K_THERM = 0;
        dSPIN_RegsStruct->STALL_TH = 0x40;
#endif /* defined(L6470) */
#if defined(L6472)
	dSPIN_RegsStruct->TVAL_HOLD = 0x29;
	dSPIN_RegsStruct->TVAL_RUN = 0x29;
	dSPIN_RegsStruct->TVAL_ACC = 0x29;
	dSPIN_RegsStruct->TVAL_DEC = 0x29;
	dSPIN_RegsStruct->T_FAST = 0x19;
	dSPIN_RegsStruct->TON_MIN = 0x29;
	dSPIN_RegsStruct->TOFF_MIN = 0x29;
#endif /* defined(L6472) */       
	dSPIN_RegsStruct->OCD_TH = 0x8;
	dSPIN_RegsStruct->STEP_MODE = 0x7;
	dSPIN_RegsStruct->ALARM_EN = 0xFF;
	dSPIN_RegsStruct->CONFIG = 0x2E88;
}
 
/**
  * @brief  Configures dSPIN internal registers with values in the config structure.
  * @param  dSPIN_RegsStruct Configuration structure address (pointer to configuration structure)
  * @retval None
  */
void dSPIN_Registers_Set(dSPIN_RegsStruct_TypeDef* dSPIN_RegsStruct)
{
	dSPIN_Set_Param(dSPIN_ABS_POS, dSPIN_RegsStruct->ABS_POS);
	dSPIN_Set_Param(dSPIN_EL_POS, dSPIN_RegsStruct->EL_POS);
	dSPIN_Set_Param(dSPIN_MARK, dSPIN_RegsStruct->MARK);
	dSPIN_Set_Param(dSPIN_ACC, dSPIN_RegsStruct->ACC);
	dSPIN_Set_Param(dSPIN_DEC, dSPIN_RegsStruct->DEC);
	dSPIN_Set_Param(dSPIN_MAX_SPEED, dSPIN_RegsStruct->MAX_SPEED);
	dSPIN_Set_Param(dSPIN_MIN_SPEED, dSPIN_RegsStruct->MIN_SPEED);
	dSPIN_Set_Param(dSPIN_FS_SPD, dSPIN_RegsStruct->FS_SPD);
#if defined(L6470)
	dSPIN_Set_Param(dSPIN_KVAL_HOLD, dSPIN_RegsStruct->KVAL_HOLD);
	dSPIN_Set_Param(dSPIN_KVAL_RUN, dSPIN_RegsStruct->KVAL_RUN);
	dSPIN_Set_Param(dSPIN_KVAL_ACC, dSPIN_RegsStruct->KVAL_ACC);
	dSPIN_Set_Param(dSPIN_KVAL_DEC, dSPIN_RegsStruct->KVAL_DEC);
	dSPIN_Set_Param(dSPIN_INT_SPD, dSPIN_RegsStruct->INT_SPD);
	dSPIN_Set_Param(dSPIN_ST_SLP, dSPIN_RegsStruct->ST_SLP);
	dSPIN_Set_Param(dSPIN_FN_SLP_ACC, dSPIN_RegsStruct->FN_SLP_ACC);
	dSPIN_Set_Param(dSPIN_FN_SLP_DEC, dSPIN_RegsStruct->FN_SLP_DEC);
	dSPIN_Set_Param(dSPIN_K_THERM, dSPIN_RegsStruct->K_THERM);
	dSPIN_Set_Param(dSPIN_STALL_TH, dSPIN_RegsStruct->STALL_TH);
#endif /* defined(L6470) */
#if defined(L6472)
	dSPIN_Set_Param(dSPIN_TVAL_HOLD, dSPIN_RegsStruct->TVAL_HOLD);
	dSPIN_Set_Param(dSPIN_TVAL_RUN, dSPIN_RegsStruct->TVAL_RUN);
	dSPIN_Set_Param(dSPIN_TVAL_ACC, dSPIN_RegsStruct->TVAL_ACC);
	dSPIN_Set_Param(dSPIN_TVAL_DEC, dSPIN_RegsStruct->TVAL_DEC);
	dSPIN_Set_Param(dSPIN_T_FAST, dSPIN_RegsStruct->T_FAST);
	dSPIN_Set_Param(dSPIN_TON_MIN, dSPIN_RegsStruct->TON_MIN);
	dSPIN_Set_Param(dSPIN_TOFF_MIN, dSPIN_RegsStruct->TOFF_MIN);
#endif /* defined(L6472) */        
        dSPIN_Set_Param(dSPIN_OCD_TH, dSPIN_RegsStruct->OCD_TH);
	dSPIN_Set_Param(dSPIN_STEP_MODE, dSPIN_RegsStruct->STEP_MODE);
	dSPIN_Set_Param(dSPIN_ALARM_EN, dSPIN_RegsStruct->ALARM_EN);
	dSPIN_Set_Param(dSPIN_CONFIG, dSPIN_RegsStruct->CONFIG);
}

#if defined(DEBUG)  
/**
  * @brief Reads dSPIN internal registers and print them to terminal I/O
  * @param dSPIN_RegsStruct Configuration structure address (pointer to configuration structure)
  * @retval None
  */
void dSPIN_Registers_Get(dSPIN_RegsStruct_TypeDef* dSPIN_RegsStruct)
{
  uint32_t read_reg;
  char diff[1];
  char str[10];
  
  PRINT_REG(ABS_POS, read_reg, diff, str);
  PRINT_REG(EL_POS, read_reg, diff, str);
  PRINT_REG(MARK, read_reg, diff, str);
  PRINT_REG(SPEED, read_reg, diff, str);
  PRINT_REG(ACC, read_reg, diff, str);
  PRINT_REG(DEC, read_reg, diff, str);
  PRINT_REG(MAX_SPEED, read_reg, diff, str);
  PRINT_REG(MIN_SPEED, read_reg, diff, str);
  PRINT_REG(KVAL_HOLD, read_reg, diff, str);
  PRINT_REG(KVAL_RUN, read_reg, diff, str);
  PRINT_REG(KVAL_ACC, read_reg, diff, str);
  PRINT_REG(KVAL_DEC, read_reg, diff, str);
  PRINT_REG(INT_SPD, read_reg, diff, str);
  PRINT_REG(ST_SLP, read_reg, diff, str);
  PRINT_REG(FN_SLP_ACC, read_reg, diff, str);
  PRINT_REG(FN_SLP_DEC, read_reg, diff, str);
  PRINT_REG(K_THERM, read_reg, diff, str);
  PRINT_REG(ADC_OUT, read_reg, diff, str);
  PRINT_REG(OCD_TH, read_reg, diff, str);
  PRINT_REG(STALL_TH, read_reg, diff, str);
  PRINT_REG(FS_SPD, read_reg, diff, str);
  PRINT_REG(STEP_MODE, read_reg, diff, str);
  PRINT_REG(ALARM_EN, read_reg, diff, str);
  PRINT_REG(CONFIG, read_reg, diff, str);
  PRINT_REG(STATUS, read_reg, diff, str);
}
#endif /* defined(DEBUG) */

#if defined(DEBUG)  
/**
  * @brief  Reads dSPIN internal writable registers and compares with the values in the code
  * @param  dSPIN_RegsStruct Configuration structure address (pointer to configuration structure)
  * @retval Bitmap with the bits, corresponding to the unmatched registers, set
  */
uint32_t dSPIN_Registers_Check(dSPIN_RegsStruct_TypeDef* dSPIN_RegsStruct)
{
  uint32_t result = 0;
  
  CHECK_REG(ABS_POS, result);
  CHECK_REG(EL_POS, result);
  CHECK_REG(MARK, result);
  CHECK_REG(ACC, result);
  CHECK_REG(DEC, result);
  CHECK_REG(MAX_SPEED, result);
  CHECK_REG(MIN_SPEED, result);
  CHECK_REG(KVAL_HOLD, result);
  CHECK_REG(KVAL_RUN, result);
  CHECK_REG(KVAL_ACC, result);
  CHECK_REG(KVAL_DEC, result);
  CHECK_REG(INT_SPD, result);
  CHECK_REG(ST_SLP, result);
  CHECK_REG(FN_SLP_ACC, result);
  CHECK_REG(FN_SLP_DEC, result);
  CHECK_REG(K_THERM, result);
  CHECK_REG(OCD_TH, result);
  CHECK_REG(STALL_TH, result);
  CHECK_REG(FS_SPD, result);
  CHECK_REG(STEP_MODE, result);
  CHECK_REG(ALARM_EN, result);
  CHECK_REG(CONFIG, result);
  
  return result;
}
#endif /* defined(DEBUG) */

/**
  * @brief Issues dSPIN NOP command.
  * @param None
  * @retval None
  */
void dSPIN_Nop(void)
{
	/* Send NOP operation code to dSPIN */
	dSPIN_Write_Byte(dSPIN_NOP);
}

/**
  * @brief  Issues dSPIN Set Param command.
  * @param  param dSPIN register address
  * @param  value to be set
  * @retval None
  */
void dSPIN_Set_Param(dSPIN_Registers_TypeDef param, uint32_t value)
{
	/* Send SetParam operation code to dSPIN */
	dSPIN_Write_Byte((uint8_t)dSPIN_SET_PARAM | (uint8_t)param);
	switch (param)
	{
		case dSPIN_ABS_POS: ;
		case dSPIN_MARK: ;
			/* Send parameter - byte 2 to dSPIN */
			dSPIN_Write_Byte((uint8_t)(value >> 16));
		case dSPIN_EL_POS: ;
		case dSPIN_ACC: ;
		case dSPIN_DEC: ;
		case dSPIN_MAX_SPEED: ;
		case dSPIN_MIN_SPEED: ;
		case dSPIN_FS_SPD: ;
#if defined(L6470)
		case dSPIN_INT_SPD: ;
#endif /* defined(L6470) */
		case dSPIN_CONFIG: ;
		case dSPIN_STATUS:
			/* Send parameter - byte 1 to dSPIN */
		   	dSPIN_Write_Byte((uint8_t)(value >> 8));
		default:
			/* Send parameter - byte 0 to dSPIN */
		   	dSPIN_Write_Byte((uint8_t)(value));
	}
}

/**
  * @brief  Issues dSPIN Get Param command.
  * @param  param dSPIN register address
  * @retval Register value - 1 to 3 bytes (depends on register)
  */
uint32_t dSPIN_Get_Param(dSPIN_Registers_TypeDef param)
{
	uint32_t temp = 0;
	uint32_t rx = 0;

	/* Send GetParam operation code to dSPIN */
	temp = dSPIN_Write_Byte((uint8_t)dSPIN_GET_PARAM | (uint8_t)param);
	/* MSB which should be 0 */
	temp = temp << 24;
	rx |= temp;
	switch (param)
	{
		case dSPIN_ABS_POS: ;
		case dSPIN_MARK: ;
		case dSPIN_SPEED:
		   	temp = dSPIN_Write_Byte((uint8_t)(0x00));
			temp = temp << 16;
			rx |= temp;
		case dSPIN_EL_POS: ;
		case dSPIN_ACC: ;
		case dSPIN_DEC: ;
		case dSPIN_MAX_SPEED: ;
		case dSPIN_MIN_SPEED: ;
		case dSPIN_FS_SPD: ;
#if defined(L6470)
		case dSPIN_INT_SPD: ;
#endif /* defined(L6470) */
		case dSPIN_CONFIG: ;
		case dSPIN_STATUS:
		   	temp = dSPIN_Write_Byte((uint8_t)(0x00));
			temp = temp << 8;
			rx |= temp;
		default:
		   	temp = dSPIN_Write_Byte((uint8_t)(0x00));
			rx |= temp;
	}
	return rx;
}

/**
  * @brief  Issues dSPIN Run command.
  * @param  direction Movement direction (FWD, REV)
  * @param  speed over 3 bytes
  * @retval None
  */
void dSPIN_Run(dSPIN_Direction_TypeDef direction, uint32_t speed)
{
	/* Send RUN operation code to dSPIN */
	dSPIN_Write_Byte((uint8_t)dSPIN_RUN | (uint8_t)direction);
	/* Send speed - byte 2 data dSPIN */
	dSPIN_Write_Byte((uint8_t)(speed >> 16));
	/* Send speed - byte 1 data dSPIN */
	dSPIN_Write_Byte((uint8_t)(speed >> 8));
	/* Send speed - byte 0 data dSPIN */
	dSPIN_Write_Byte((uint8_t)(speed));
}

/**
  * @brief  Issues dSPIN Step Clock command.
  * @param  direction Movement direction (FWD, REV)
  * @retval None
  */
void dSPIN_Step_Clock(dSPIN_Direction_TypeDef direction)
{
	/* Send StepClock operation code to dSPIN */
	dSPIN_Write_Byte((uint8_t)dSPIN_STEP_CLOCK | (uint8_t)direction);
}

/**
  * @brief  Issues dSPIN Move command.
  * @param  direction mMovement direction
  * @param  n_step number of steps
  * @retval None
  */
void dSPIN_Move(dSPIN_Direction_TypeDef direction, uint32_t n_step)
{
	/* Send Move operation code to dSPIN */
	dSPIN_Write_Byte((uint8_t)dSPIN_MOVE | (uint8_t)direction);
	/* Send n_step - byte 2 data dSPIN */
	dSPIN_Write_Byte((uint8_t)(n_step >> 16));
	/* Send n_step - byte 1 data dSPIN */
	dSPIN_Write_Byte((uint8_t)(n_step >> 8));
	/* Send n_step - byte 0 data dSPIN */
	dSPIN_Write_Byte((uint8_t)(n_step));
}

/**
  * @brief  Issues dSPIN Go To command.
  * @param  abs_pos absolute position where requested to move
  * @retval None
  */
void dSPIN_Go_To(uint32_t abs_pos)
{
	/* Send GoTo operation code to dSPIN */
	dSPIN_Write_Byte(dSPIN_GO_TO);
	/* Send absolute position parameter - byte 2 data to dSPIN */
	dSPIN_Write_Byte((uint8_t)(abs_pos >> 16));
	/* Send absolute position parameter - byte 1 data to dSPIN */
	dSPIN_Write_Byte((uint8_t)(abs_pos >> 8));
	/* Send absolute position parameter - byte 0 data to dSPIN */
	dSPIN_Write_Byte((uint8_t)(abs_pos));
}

/**
  * @brief  Issues dSPIN Go To Dir command.
  * @param  direction movement direction
  * @param  abs_pos absolute position where requested to move
  * @retval None
  */
void dSPIN_Go_To_Dir(dSPIN_Direction_TypeDef direction, uint32_t abs_pos)
{
	/* Send GoTo_DIR operation code to dSPIN */
	dSPIN_Write_Byte((uint8_t)dSPIN_GO_TO_DIR | (uint8_t)direction);
	/* Send absolute position parameter - byte 2 data to dSPIN */
	dSPIN_Write_Byte((uint8_t)(abs_pos >> 16));
	/* Send absolute position parameter - byte 1 data to dSPIN */
	dSPIN_Write_Byte((uint8_t)(abs_pos >> 8));
	/* Send absolute position parameter - byte 0 data to dSPIN */
	dSPIN_Write_Byte((uint8_t)(abs_pos));
}

/**
  * @brief  Issues dSPIN Go Until command.
  * @param  action
  * @param  direction movement direction
  * @param  speed
  * @retval None
  */
void dSPIN_Go_Until(dSPIN_Action_TypeDef action, dSPIN_Direction_TypeDef direction, uint32_t speed)
{
	/* Send GoUntil operation code to dSPIN */
	dSPIN_Write_Byte((uint8_t)dSPIN_GO_UNTIL | (uint8_t)action | (uint8_t)direction);
	/* Send speed parameter - byte 2 data to dSPIN */
	dSPIN_Write_Byte((uint8_t)(speed >> 16));
	/* Send speed parameter - byte 1 data to dSPIN */
	dSPIN_Write_Byte((uint8_t)(speed >> 8));
	/* Send speed parameter - byte 0 data to dSPIN */
	dSPIN_Write_Byte((uint8_t)(speed));
}

/**
  * @brief  Issues dSPIN Release SW command.
  * @param  action
  * @param  direction movement direction
  * @retval None
  */
void dSPIN_Release_SW(dSPIN_Action_TypeDef action, dSPIN_Direction_TypeDef direction)
{
	/* Send ReleaseSW operation code to dSPIN */
	dSPIN_Write_Byte((uint8_t)dSPIN_RELEASE_SW | (uint8_t)action | (uint8_t)direction);
}

/**
  * @brief  Issues dSPIN Go Home command. (Shorted path to zero position)
  * @param  None
  * @retval None
  */
void dSPIN_Go_Home(void)
{
	/* Send GoHome operation code to dSPIN */
	dSPIN_Write_Byte(dSPIN_GO_HOME);
}

/**
  * @brief  Issues dSPIN Go Mark command.
  * @param  None
  * @retval None
  */
void dSPIN_Go_Mark(void)
{
    /* Send GoMark operation code to dSPIN */
    dSPIN_Write_Byte(dSPIN_GO_MARK);
}

/**
  * @brief  Issues dSPIN Reset Pos command.
  * @param  None
  * @retval None
  */
void dSPIN_Reset_Pos(void)
{
	  /* Send ResetPos operation code to dSPIN */
	  dSPIN_Write_Byte(dSPIN_RESET_POS);
}

/**
  * @brief  Issues dSPIN Reset Device command.
  * @param  None
  * @retval None
  */
void dSPIN_Reset_Device(void)
{
	/* Send ResetDevice operation code to dSPIN */
	  dSPIN_Write_Byte(dSPIN_RESET_DEVICE);
    dSPIN_Write_Byte((uint8_t)(0));
    dSPIN_Write_Byte((uint8_t)(0));
    dSPIN_Write_Byte((uint8_t)(0));
}

/**
  * @brief  Issues dSPIN Soft Stop command.
  * @param  None
  * @retval None
  */
void dSPIN_Soft_Stop(void)
{
	/* Send SoftStop operation code to dSPIN */
	dSPIN_Write_Byte(dSPIN_SOFT_STOP);
}

/**
  * @brief  Issues dSPIN Hard Stop command.
  * @param  None
  * @retval None
  */
void dSPIN_Hard_Stop(void)
{
	/* Send HardStop operation code to dSPIN */
	dSPIN_Write_Byte(dSPIN_HARD_STOP);
}

/**
  * @brief  Issues dSPIN Soft HiZ command.
  * @param  None
  * @retval None
  */
void dSPIN_Soft_HiZ(void)
{
	/* Send SoftHiZ operation code to dSPIN */
	dSPIN_Write_Byte(dSPIN_SOFT_HIZ);
}

/**
  * @brief  Issues dSPIN Hard HiZ command.
  * @param  None
  * @retval None
  */
void dSPIN_Hard_HiZ(void)
{
	/* Send HardHiZ operation code to dSPIN */
	dSPIN_Write_Byte(dSPIN_HARD_HIZ);
}

/**
  * @brief  Issues dSPIN Get Status command.
  * @param  None
  * @retval Status Register content
  */
uint16_t dSPIN_Get_Status(void)
{
	uint16_t temp = 0;
	uint16_t rx = 0;

	/* Send GetStatus operation code to dSPIN */
	dSPIN_Write_Byte(dSPIN_GET_STATUS);
	/* Send zero byte / receive MSByte from dSPIN */
	temp = dSPIN_Write_Byte((uint8_t)(0x00));
	temp = temp << 8;
	rx |= temp;
	/* Send zero byte / receive LSByte from dSPIN */
	temp = dSPIN_Write_Byte((uint8_t)(0x00));
	rx |= temp;
	return rx;
}

#if 0
/**
  * @brief  Checks if the dSPIN is Busy by hardware - active Busy signal.
  * @param  None
  * @retval one if chip is busy, otherwise zero
  */
uint8_t dSPIN_Busy_HW(void)
{
	if(!(GPIO_ReadInputDataBit(dSPIN_BUSY_Port, 11))) return 0x01;
	else return 0x00;
}
#endif
/**
  * @brief  Checks if the dSPIN is Busy by SPI - Busy flag bit in Status Register.
  * @param  None
  * @retval one if chip is busy, otherwise zero
  */
uint8_t dSPIN_Busy_SW(void)
{
	if(!(dSPIN_Get_Status() & dSPIN_STATUS_BUSY)) return 0x01;
	else return 0x00;
}

#if 0
/**
  * @brief  Checks dSPIN Flag signal.
  * @param  None
  * @retval one if Flag signal is active, otherwise zero
  */
uint8_t dSPIN_Flag(void)
{
	if(!(GPIO_ReadInputDataBit(dSPIN_FLAG_Port, 10))) return 0x01;
	else return 0x00;
}
#endif
/**
  * @brief  Transmits/Receives one byte to/from dSPIN over SPI.
  * @param  byte Transmited byte
  * @retval Received byte
  */
uint8_t dSPIN_Write_Byte(uint8_t byte)
{
        uint8_t rcvdByte = (uint8_t)(0);
	/* nSS signal activation - low */
	SPI_ResetNss();
	/* SPI byte send */
        rcvdByte = SPI_TransmitReceive(byte);
	/* Wait for SPIx Busy flag */
	//while (SPI_I2S_GetFlagStatus(dSPIN_SPI, SPI_I2S_FLAG_BSY) != RESET);
	/* nSS signal deactivation - high */
	SPI_SetNss();
	//return (uint8_t)(SPI_I2S_ReceiveData(dSPIN_SPI));
        return (rcvdByte);
}

/**
  * @brief  Transmits/Receives several bytes to dSPIN over SPI
  * @param  pTxByte pTxBytePointer to TX bytes
  * @param  pRxByte Pointer to RX bytes
  * @param  nBytes Number of TX = RX bytes
  * @retval None
  */
void dSPIN_Write_Daisy_Chain_Bytes(uint8_t *pTxByte, uint8_t *pRxByte, uint8_t nBytes)
{
        uint32_t index;
	/* nSS signal activation - low */
	SPI_ResetNss();
	/* SPI byte send */
        for (index = 0; index < nBytes; index++)
        {
          *pRxByte = SPI_TransmitReceive(*pTxByte);
          /* Wait for SPIx Busy flag */
	  //while (SPI_I2S_GetFlagStatus(dSPIN_SPI, SPI_I2S_FLAG_BSY) != RESET);
          //*pRxByte = SPI_I2S_ReceiveData(dSPIN_SPI);
          pTxByte++;
          pRxByte++;
        }
	/* nSS signal deactivation - high */
	SPI_SetNss();
}

/**
  * @brief  Issues dSPIN Set Param command to each device (slave).
  * @param  slaves_number number of slaves
  * @param  pParam Pointer to an array of dSPIN register address
  * @param  pValue Pointer to an array of dSPIN parameter value
  * @retval None
  */
void dSPIN_All_Slaves_Set_Param(uint8_t slaves_number, uint8_t *pParam, uint32_t *pValue)
{
  uint32_t i;
  uint8_t maxArgumentNbBytes = 0;

  for (i = 0; i < slaves_number; i++)
  {
     switch (*pParam)
     {
	case dSPIN_ABS_POS: ;
	case dSPIN_MARK: ;
	case dSPIN_SPEED:
                 spiTxBursts[0][i] = *pParam;
                 spiTxBursts[1][i] = (uint8_t)(*pValue >> 16);
                 spiTxBursts[2][i] = (uint8_t)(*pValue >> 8);
	         maxArgumentNbBytes = 3;
                 break;
        case dSPIN_EL_POS: ;
	case dSPIN_ACC: ;
	case dSPIN_DEC: ;
	case dSPIN_MAX_SPEED: ;
	case dSPIN_MIN_SPEED: ;
	case dSPIN_FS_SPD: ;
#if defined(L6470)
	case dSPIN_INT_SPD: ;
#endif /* defined(L6470) */
	case dSPIN_CONFIG: ;
	case dSPIN_STATUS:
                 spiTxBursts[0][i] = dSPIN_NOP;
                 spiTxBursts[1][i] = *pParam;
                 spiTxBursts[2][i] = (uint8_t)(*pValue >> 8);           
                 if (maxArgumentNbBytes < 2)
                 {
                    maxArgumentNbBytes = 2;
                 }
                 break;
	default:
                 spiTxBursts[0][i] = dSPIN_NOP;
                 spiTxBursts[1][i] = dSPIN_NOP;
                 spiTxBursts[2][i] = *pParam;
                 if (maxArgumentNbBytes < 1)
                 {                 
                    maxArgumentNbBytes = 1;
                 }
     }
     spiTxBursts[3][i] = (uint8_t)(*pValue);
     pParam++;
     pValue++;
  }
  for (i = dSPIN_CMD_ARG_MAX_NB_BYTES-1-maxArgumentNbBytes; i < dSPIN_CMD_ARG_MAX_NB_BYTES; i++)
  {
     dSPIN_Write_Daisy_Chain_Bytes(&spiTxBursts[i][0], &spiRxBursts[i][0], slaves_number);
  }
}

/**
  * @brief  Issues dSPIN Get Param command to each device (slave).
  * @param  slaves_number number of slaves
  * @param  pParam Pointer to an array of dSPIN register address
  * @param  pValue Pointer to an array of dSPIN parameter value
  * @retval None
  */
void dSPIN_All_Slaves_Get_Param(uint8_t slaves_number, uint8_t *pParam, uint32_t *pValue)
{
  uint32_t i;
  uint8_t maxArgumentNbBytes = 0;
  
  for (i = 0; i < slaves_number; i++)
  {
     switch (*pParam)
     {
	case dSPIN_ABS_POS: ;
	case dSPIN_MARK: ;
	case dSPIN_SPEED:
                 spiTxBursts[0][i] = ((uint8_t)dSPIN_GET_PARAM )| (*pParam);
                 spiTxBursts[1][i] = dSPIN_NOP;
                 spiTxBursts[2][i] = dSPIN_NOP;
	         maxArgumentNbBytes = 3;
                 break;
        case dSPIN_EL_POS: ;
	case dSPIN_ACC: ;
	case dSPIN_DEC: ;
	case dSPIN_MAX_SPEED: ;
	case dSPIN_MIN_SPEED: ;
	case dSPIN_FS_SPD: ;
#if defined(L6470)
	case dSPIN_INT_SPD: ;
#endif /* defined(L6470) */
	case dSPIN_CONFIG: ;
	case dSPIN_STATUS:
                 spiTxBursts[0][i] = dSPIN_NOP;
                 spiTxBursts[1][i] = ((uint8_t)dSPIN_GET_PARAM )| (*pParam);
                 spiTxBursts[2][i] = dSPIN_NOP;
                 if (maxArgumentNbBytes < 2)
                 {
                    maxArgumentNbBytes = 2;
                 }
                 break;
	default:
                 spiTxBursts[0][i] = dSPIN_NOP;
                 spiTxBursts[1][i] = dSPIN_NOP;
                 spiTxBursts[2][i] = ((uint8_t)dSPIN_GET_PARAM )| (*pParam);
                 if (maxArgumentNbBytes < 1)
                 {                 
                    maxArgumentNbBytes = 1;
                 }
     }
     spiTxBursts[3][i] = dSPIN_NOP;
     spiRxBursts[1][i] = 0;
     spiRxBursts[2][i] = 0;
     spiRxBursts[3][i] = 0;
     pParam++;
  }
  for (i = dSPIN_CMD_ARG_MAX_NB_BYTES-1-maxArgumentNbBytes; i < dSPIN_CMD_ARG_MAX_NB_BYTES; i++)
  {
     dSPIN_Write_Daisy_Chain_Bytes(&spiTxBursts[i][0], &spiRxBursts[i][0], slaves_number);
  }
  for (i = 0; i < slaves_number; i++)
  {
    *pValue = (spiRxBursts[1][i] << 16) | (spiRxBursts[2][i] << 8) | (spiRxBursts[3][i]);
    pValue++;
  }
}

/**
  * @brief  Configures dSPIN slaves internal registers with values in the config structure.
  * @param  slaves_number number of slaves 
  * @param  dSPIN_RegsStructArray Configuration structure array address (pointer to configuration structure array)
  * @retval None
  */
void dSPIN_All_Slaves_Registers_Set(uint8_t slaves_number, dSPIN_RegsStruct_TypeDef *dSPIN_RegsStructArray)
{
    uint32_t i;
    
    /* ABS_POS */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_ABS_POS;
      arrayValues[i] = dSPIN_RegsStructArray[i].ABS_POS;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* EL_POS */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_EL_POS;
      arrayValues[i] = dSPIN_RegsStructArray[i].EL_POS;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* MARK */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_MARK;
      arrayValues[i] = dSPIN_RegsStructArray[i].MARK;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* ACC */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_ACC;
      arrayValues[i] = dSPIN_RegsStructArray[i].ACC;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* DEC*/
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_DEC;
      arrayValues[i] = dSPIN_RegsStructArray[i].DEC;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);  
    /* MAX_SPEED */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_MAX_SPEED;
      arrayValues[i] = dSPIN_RegsStructArray[i].MAX_SPEED;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* MIN_SPEED */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_MIN_SPEED;
      arrayValues[i] = dSPIN_RegsStructArray[i].MIN_SPEED;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* FS_SPD */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_FS_SPD;
      arrayValues[i] = dSPIN_RegsStructArray[i].FS_SPD;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
#if defined(L6470)
    /* KVAL_HOLD */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_KVAL_HOLD;
      arrayValues[i] = dSPIN_RegsStructArray[i].KVAL_HOLD;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* KVAL_RUN */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_KVAL_RUN;
      arrayValues[i] = dSPIN_RegsStructArray[i].KVAL_RUN;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* KVAL_ACC */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_KVAL_ACC;
      arrayValues[i] = dSPIN_RegsStructArray[i].KVAL_ACC;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues); 
    /* KVAL_DEC */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_KVAL_DEC;
      arrayValues[i] = dSPIN_RegsStructArray[i].KVAL_DEC;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* INT_SPD */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_INT_SPD;
      arrayValues[i] = dSPIN_RegsStructArray[i].INT_SPD;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* ST_SLP */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_ST_SLP;
      arrayValues[i] = dSPIN_RegsStructArray[i].ST_SLP;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* FN_SLP_ACC */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_FN_SLP_ACC;
      arrayValues[i] = dSPIN_RegsStructArray[i].FN_SLP_ACC;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* FN_SLP_DEC */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_FN_SLP_DEC;
      arrayValues[i] = dSPIN_RegsStructArray[i].FN_SLP_DEC;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* K_THERM */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_K_THERM;
      arrayValues[i] = dSPIN_RegsStructArray[i].K_THERM;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* STALL_TH */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_STALL_TH;
      arrayValues[i] = dSPIN_RegsStructArray[i].STALL_TH;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
#endif /* defined(L6470) */
#if defined(L6472)
    /* TVAL_HOLD */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_TVAL_HOLD;
      arrayValues[i] = dSPIN_RegsStructArray[i].TVAL_HOLD;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* TVAL_RUN */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_TVAL_RUN;
      arrayValues[i] = dSPIN_RegsStructArray[i].TVAL_RUN;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* TVAL_ACC */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_TVAL_ACC;
      arrayValues[i] = dSPIN_RegsStructArray[i].TVAL_ACC;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues); 
    /* TVAL_DEC */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_TVAL_DEC;
      arrayValues[i] = dSPIN_RegsStructArray[i].TVAL_DEC;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* T_FAST */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_T_FAST;
      arrayValues[i] = dSPIN_RegsStructArray[i].T_FAST;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* TON_MIN */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_TON_MIN;
      arrayValues[i] = dSPIN_RegsStructArray[i].TON_MIN;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* TOFF_MIN */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_TOFF_MIN;
      arrayValues[i] = dSPIN_RegsStructArray[i].TOFF_MIN;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
#endif /* defined(L6472) */
    /* OCD_TH */
     for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_OCD_TH;
      arrayValues[i] = dSPIN_RegsStructArray[i].OCD_TH;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* STEP_MODE */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_STEP_MODE;
      arrayValues[i] = dSPIN_RegsStructArray[i].STEP_MODE;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* ALARM_EN */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_ALARM_EN;
      arrayValues[i] = dSPIN_RegsStructArray[i].ALARM_EN;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues);
    /* CONFIG */
    for (i=0;i<slaves_number;i++)
    {
      arrayTxBytes[i] = dSPIN_CONFIG;
      arrayValues[i] = dSPIN_RegsStructArray[i].CONFIG;
    }
    dSPIN_All_Slaves_Set_Param(slaves_number, arrayTxBytes, arrayValues); 
}

/**
  * @brief  Issues dSPIN Move command to one slave device
  * @param  slaves_number number of slaves 
  * @param  slaveNumber slave number
  * @param  direction movement direction
  * @param  n_step number of steps
  * @retval None
  */
void dSPIN_One_Slave_Move(uint8_t slaves_number, uint8_t slaveNumber, dSPIN_Direction_TypeDef direction, uint32_t n_step)
{
  dSPIN_One_Slave_Send_Command(slaveNumber, slaves_number, (uint8_t)dSPIN_MOVE | (uint8_t)direction, n_step);
}

/**
  * @brief  Issues dSPIN Run command to one slave device
  * @param  slaveNumber slave number
  * @param  slaves_number number of slaves 
  * @param  direction movement direction
  * @param  speed
  * @retval None
  */
void dSPIN_One_Slave_Run(uint8_t slaveNumber, uint8_t slaves_number, dSPIN_Direction_TypeDef direction, uint32_t speed)
{
  dSPIN_One_Slave_Send_Command(slaveNumber, slaves_number, (uint8_t)dSPIN_RUN | (uint8_t)direction, speed);
}

/**
  * @brief  Issues a command to one slave device
  * @param  slaveNumber slave number
  * @param  slaves_number number of slaves 
  * @param  param command to issue
  * @param  value command argument 
  * @retval None
  */
void dSPIN_One_Slave_Send_Command(uint8_t slaveNumber, uint8_t slaves_number, uint8_t param, uint32_t value)
{
  uint32_t i;
  
  for (i = 0; i < slaves_number; i++)
  {
    if (i == slaveNumber)
    {
      spiTxBursts[0][i] = (param);
      spiTxBursts[1][i] = (uint8_t)(value >> 16);
      spiTxBursts[2][i] = (uint8_t)(value >> 8);
      spiTxBursts[3][i] = (uint8_t)(value);
    }
    else
    {
      spiTxBursts[0][i] = dSPIN_NOP;
      spiTxBursts[1][i] = dSPIN_NOP;
      spiTxBursts[2][i] = dSPIN_NOP;
      spiTxBursts[3][i] = dSPIN_NOP;     
    }
  }
  for (i = dSPIN_CMD_ARG_MAX_NB_BYTES-dSPIN_CMD_ARG_NB_BYTES_MOVE; i < dSPIN_CMD_ARG_MAX_NB_BYTES; i++)
  {
     dSPIN_Write_Daisy_Chain_Bytes(&spiTxBursts[i][0], &spiRxBursts[i][0], slaves_number);
  }
}

/**
  * @brief Issues commands to the slave devices for synchronous execution
  * @param slaves_number number of slaves
  * @param pParam Pointer to an array of dSPIN commands
  * @param pValue Pointer to an array of dSPIN arguments

  * @retval None
  */
void dSPIN_All_Slaves_Send_Command(uint8_t slaves_number, uint8_t *pParam, uint32_t *pValue)
{
  uint32_t i;
  uint8_t maxArgumentNbBytes = 0;

  for (i = 0; i < slaves_number; i++)
  {
     switch ((*pParam) & DAISY_CHAIN_COMMAND_MASK)
     {
        case dSPIN_RUN: ;
        case dSPIN_MOVE: ;
        case dSPIN_GO_TO: ;
        case dSPIN_GO_TO_DIR: ;
        case dSPIN_GO_UNTIL: ;
        case dSPIN_GO_UNTIL_ACT_CPY:
                 spiTxBursts[0][i] = *pParam;
                 spiTxBursts[1][i] = (uint8_t)(*pValue >> 16);
                 spiTxBursts[2][i] = (uint8_t)(*pValue >> 8);
                 spiTxBursts[3][i] = (uint8_t)(*pValue);
	         maxArgumentNbBytes = 3;
                 break;
	default:
                 spiTxBursts[0][i] = dSPIN_NOP;
                 spiTxBursts[1][i] = dSPIN_NOP;
                 spiTxBursts[2][i] = dSPIN_NOP;
                 spiTxBursts[3][i] = *pParam;
     }
     pParam++;
     pValue++;
  }
  for (i = dSPIN_CMD_ARG_MAX_NB_BYTES-1-maxArgumentNbBytes; i < dSPIN_CMD_ARG_MAX_NB_BYTES; i++)
  {
     dSPIN_Write_Daisy_Chain_Bytes(&spiTxBursts[i][0], &spiRxBursts[i][0], slaves_number);
  }
}

/**
  * @brief Issues dSPIN Get Status command to each device (slave)
  * @param slaves_number number of slaves
  * @param pValue pointer to an array of Status Register content
  * @retval None
  */
void dSPIN_All_Slaves_Get_Status(uint8_t slaves_number, uint32_t *pValue)
{
  uint32_t i;

  for (i = 0; i < slaves_number; i++)
  {
     spiTxBursts[0][i] = dSPIN_GET_STATUS;
     spiTxBursts[1][i] = dSPIN_NOP;
     spiTxBursts[2][i] = dSPIN_NOP;
     spiRxBursts[1][i] = 0;
     spiRxBursts[2][i] = 0;
  }
  for (i = 0; i < dSPIN_CMD_ARG_NB_BYTES_GET_STATUS+dSPIN_RSP_NB_BYTES_GET_STATUS; i++)
  {
     dSPIN_Write_Daisy_Chain_Bytes(&spiTxBursts[i][0], &spiRxBursts[i][0], slaves_number);
  }
  for (i = 0; i < slaves_number; i++)
  {
    *pValue = (spiRxBursts[1][i] << 8) | (spiRxBursts[2][i]);
    pValue++;
  }
}

/**
  * @brief  Checks if one of the dSPIN device (slave) is Busy by SPI - Busy flag bit in Status Register.
  * @param  slaves_number number of slaves
  * @retval one if there is a busy chip, otherwise zero
  */
uint8_t dSPIN_One_Or_More_Slaves_Busy_SW(uint8_t slaves_number)
{
  uint32_t i;
  uint16_t status;
  dSPIN_All_Slaves_Get_Status(slaves_number, arrayValues);
  for (i = 0; i < slaves_number; i++)
  {
    status |= arrayValues[i];
  }
  if(!(status & dSPIN_STATUS_BUSY)) return 0x01;
  else return 0x00;
}

/** @} */  
/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
