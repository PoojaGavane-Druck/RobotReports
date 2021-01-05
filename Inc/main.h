/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define BUILD_NUMBER 1u
#define MAJOR_VERSION_NUMBER 2u
#define MINOR_VERSION_NUMBER 0u 
/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define STATUS_RED_Pin GPIO_PIN_2
#define STATUS_RED_GPIO_Port GPIOE
#define STCK_COUNT_Pin GPIO_PIN_3
#define STCK_COUNT_GPIO_Port GPIOE
#define STATUS_BLUE_Pin GPIO_PIN_4
#define STATUS_BLUE_GPIO_Port GPIOE
#define BT_INDICATION_Pin GPIO_PIN_5
#define BT_INDICATION_GPIO_Port GPIOE
#define BAT_LEVEL1_Pin GPIO_PIN_2
#define BAT_LEVEL1_GPIO_Port GPIOF
#define EXT_NRST_Pin GPIO_PIN_3
#define EXT_NRST_GPIO_Port GPIOF
#define BAT_LEVEL2_Pin GPIO_PIN_4
#define BAT_LEVEL2_GPIO_Port GPIOF
#define BAT_LEVEL3_Pin GPIO_PIN_5
#define BAT_LEVEL3_GPIO_Port GPIOF
#define POWER_KEY_Pin GPIO_PIN_8
#define POWER_KEY_GPIO_Port GPIOF
#define POWER_KEY_EXTI_IRQn EXTI9_5_IRQn
#define BT_KEY_Pin GPIO_PIN_9
#define BT_KEY_GPIO_Port GPIOF
#define BT_KEY_EXTI_IRQn EXTI9_5_IRQn
#define STATUS_GREEN_Pin GPIO_PIN_10
#define STATUS_GREEN_GPIO_Port GPIOF
#define IR_SENS_DRIVE_Pin GPIO_PIN_0
#define IR_SENS_DRIVE_GPIO_Port GPIOC
#define IR_SENS_ADC_Pin GPIO_PIN_1
#define IR_SENS_ADC_GPIO_Port GPIOC
#define STEPPER_STCK_Pin GPIO_PIN_0
#define STEPPER_STCK_GPIO_Port GPIOA
#define P24V_MON_ADC1_Pin GPIO_PIN_1
#define P24V_MON_ADC1_GPIO_Port GPIOA
#define PM620_IF_UART2_TX_Pin GPIO_PIN_2
#define PM620_IF_UART2_TX_GPIO_Port GPIOA
#define PM620_IF_UART2_RX_Pin GPIO_PIN_3
#define PM620_IF_UART2_RX_GPIO_Port GPIOA
#define PM620_IF_UART2_RXEN_Pin GPIO_PIN_4
#define PM620_IF_UART2_RXEN_GPIO_Port GPIOA
#define P6V_MON_ADC1_Pin GPIO_PIN_5
#define P6V_MON_ADC1_GPIO_Port GPIOA
#define P5V_MON_ADC1_Pin GPIO_PIN_6
#define P5V_MON_ADC1_GPIO_Port GPIOA
#define P24V_EN_Pin GPIO_PIN_7
#define P24V_EN_GPIO_Port GPIOA
#define VALVE1_DIR_Pin GPIO_PIN_5
#define VALVE1_DIR_GPIO_Port GPIOC
#define VALVE2_DIR_Pin GPIO_PIN_1
#define VALVE2_DIR_GPIO_Port GPIOB
#define VALVE3_DIR_Pin GPIO_PIN_11
#define VALVE3_DIR_GPIO_Port GPIOF
#define VALVE3_nFAULT_Pin GPIO_PIN_13
#define VALVE3_nFAULT_GPIO_Port GPIOF
#define VALVE1_nFAULT_Pin GPIO_PIN_14
#define VALVE1_nFAULT_GPIO_Port GPIOF
#define VALVE2_nFAULT_Pin GPIO_PIN_15
#define VALVE2_nFAULT_GPIO_Port GPIOF
#define STEPPER_FLAG_Pin GPIO_PIN_0
#define STEPPER_FLAG_GPIO_Port GPIOG
#define STEPPER_STBY_RST_Pin GPIO_PIN_1
#define STEPPER_STBY_RST_GPIO_Port GPIOG
#define VALVE1_PWM_Pin GPIO_PIN_9
#define VALVE1_PWM_GPIO_Port GPIOE
#define FLASH_OCTSPI_CLK_Pin GPIO_PIN_10
#define FLASH_OCTSPI_CLK_GPIO_Port GPIOE
#define FLASH_OCTSPI_NCS_Pin GPIO_PIN_11
#define FLASH_OCTSPI_NCS_GPIO_Port GPIOE
#define FLASH_OCTSPI_IO0_Pin GPIO_PIN_12
#define FLASH_OCTSPI_IO0_GPIO_Port GPIOE
#define FLASH_OCTSPI_IO1_Pin GPIO_PIN_13
#define FLASH_OCTSPI_IO1_GPIO_Port GPIOE
#define FLASH_OCTSPI_IO2_Pin GPIO_PIN_14
#define FLASH_OCTSPI_IO2_GPIO_Port GPIOE
#define FLASH_OCTSPI_IO3_Pin GPIO_PIN_15
#define FLASH_OCTSPI_IO3_GPIO_Port GPIOE
#define TERPS_IF_UART3_I2C2_TX_Pin GPIO_PIN_10
#define TERPS_IF_UART3_I2C2_TX_GPIO_Port GPIOB
#define TERPS_IF_UART3_I2C2_RX_Pin GPIO_PIN_11
#define TERPS_IF_UART3_I2C2_RX_GPIO_Port GPIOB
#define P6V_EN_Pin GPIO_PIN_15
#define P6V_EN_GPIO_Port GPIOB
#define BAT_LEVEL5_Pin GPIO_PIN_8
#define BAT_LEVEL5_GPIO_Port GPIOD
#define STEPPER_SW_Pin GPIO_PIN_9
#define STEPPER_SW_GPIO_Port GPIOD
#define BAT_LEVEL4_Pin GPIO_PIN_10
#define BAT_LEVEL4_GPIO_Port GPIOD
#define BAROM_INT_DRDY_Pin GPIO_PIN_11
#define BAROM_INT_DRDY_GPIO_Port GPIOD
#define BARO_EEPROM_I2C4_SCL_Pin GPIO_PIN_12
#define BARO_EEPROM_I2C4_SCL_GPIO_Port GPIOD
#define BARO_EEPROM_I2C4_SDA_Pin GPIO_PIN_13
#define BARO_EEPROM_I2C4_SDA_GPIO_Port GPIOD
#define VALVE2_PWM_Pin GPIO_PIN_14
#define VALVE2_PWM_GPIO_Port GPIOD
#define VALVE3_PWM_Pin GPIO_PIN_15
#define VALVE3_PWM_GPIO_Port GPIOD
#define STEPPER_SPI1_SCK_Pin GPIO_PIN_2
#define STEPPER_SPI1_SCK_GPIO_Port GPIOG
#define STEPPER_SPI1_MISO_Pin GPIO_PIN_3
#define STEPPER_SPI1_MISO_GPIO_Port GPIOG
#define STEPPER_SPI1_MOSI_Pin GPIO_PIN_4
#define STEPPER_SPI1_MOSI_GPIO_Port GPIOG
#define STEPPER_SPI1_NSS_Pin GPIO_PIN_5
#define STEPPER_SPI1_NSS_GPIO_Port GPIOG
#define TEMP_ALERT_Pin GPIO_PIN_6
#define TEMP_ALERT_GPIO_Port GPIOG
#define TEMP_I2C3_SCL_Pin GPIO_PIN_7
#define TEMP_I2C3_SCL_GPIO_Port GPIOG
#define TEMP_I2C3_SDA_Pin GPIO_PIN_8
#define TEMP_I2C3_SDA_GPIO_Port GPIOG
#define USB_ENUM_Pin GPIO_PIN_8
#define USB_ENUM_GPIO_Port GPIOA
#define VBUS_DET_Pin GPIO_PIN_9
#define VBUS_DET_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_uC_DATN_Pin GPIO_PIN_11
#define USB_uC_DATN_GPIO_Port GPIOA
#define USB_uC_DATP_Pin GPIO_PIN_12
#define USB_uC_DATP_GPIO_Port GPIOA
#define JTMS_SWDIO_Pin GPIO_PIN_13
#define JTMS_SWDIO_GPIO_Port GPIOA
#define JTCK_SWCLK_Pin GPIO_PIN_14
#define JTCK_SWCLK_GPIO_Port GPIOA
#define DPI620G_IF_UART4_TX_Pin GPIO_PIN_10
#define DPI620G_IF_UART4_TX_GPIO_Port GPIOC
#define DPI620G_IF_UART4_RX_Pin GPIO_PIN_11
#define DPI620G_IF_UART4_RX_GPIO_Port GPIOC
#define EXT_UART5_TX_Pin GPIO_PIN_12
#define EXT_UART5_TX_GPIO_Port GPIOC
#define MAIN_SPI2_NSS_Pin GPIO_PIN_0
#define MAIN_SPI2_NSS_GPIO_Port GPIOD
#define MAIN_SPI2_SCK_Pin GPIO_PIN_1
#define MAIN_SPI2_SCK_GPIO_Port GPIOD
#define EXT_UART5_RX_Pin GPIO_PIN_2
#define EXT_UART5_RX_GPIO_Port GPIOD
#define MAIN_SPI2_MISO_Pin GPIO_PIN_3
#define MAIN_SPI2_MISO_GPIO_Port GPIOD
#define MAIN_SPI2_MOSI_Pin GPIO_PIN_4
#define MAIN_SPI2_MOSI_GPIO_Port GPIOD
#define DPI620G_IF_RXEN_PD5_Pin GPIO_PIN_5
#define DPI620G_IF_RXEN_PD5_GPIO_Port GPIOD
#define CHGEN_Pin GPIO_PIN_10
#define CHGEN_GPIO_Port GPIOG
#define AC_PRESENT_Pin GPIO_PIN_12
#define AC_PRESENT_GPIO_Port GPIOG
#define BAT_I2C1_SDA_Pin GPIO_PIN_13
#define BAT_I2C1_SDA_GPIO_Port GPIOG
#define BAT_I2C1_SCL_Pin GPIO_PIN_14
#define BAT_I2C1_SCL_GPIO_Port GPIOG
#define STEPPER_BUSY_SYNC_Pin GPIO_PIN_15
#define STEPPER_BUSY_SYNC_GPIO_Port GPIOG
#define JTDO_TSWO_Pin GPIO_PIN_3
#define JTDO_TSWO_GPIO_Port GPIOB
#define SMBALERT_Pin GPIO_PIN_5
#define SMBALERT_GPIO_Port GPIOB
#define BT_USART1_TX_Pin GPIO_PIN_6
#define BT_USART1_TX_GPIO_Port GPIOB
#define BT_USART1_RX_Pin GPIO_PIN_7
#define BT_USART1_RX_GPIO_Port GPIOB
#define BT_PROGRAM_Pin GPIO_PIN_8
#define BT_PROGRAM_GPIO_Port GPIOB
#define BT_ENABLE_Pin GPIO_PIN_9
#define BT_ENABLE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
