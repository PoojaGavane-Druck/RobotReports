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
//#define CONTROLLER_TESTING

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define BUILD_NUMBER 0u
#define MAJOR_VERSION_NUMBER 0u
#define MINOR_VERSION_NUMBER 12u 
#define SUB_VERSION_NUMBER 00u
/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void EnableDeferredIWDG(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define STATUS_RED_PE2_Pin GPIO_PIN_2
#define STATUS_RED_PE2_GPIO_Port GPIOE
#define STCK_COUNT_PE3_Pin GPIO_PIN_3
#define STCK_COUNT_PE3_GPIO_Port GPIOE
#define STATUS_BLUE_PE4_Pin GPIO_PIN_4
#define STATUS_BLUE_PE4_GPIO_Port GPIOE
#define BT_INDICATION_PE5_Pin GPIO_PIN_5
#define BT_INDICATION_PE5_GPIO_Port GPIOE
#define TERPS_IF_RXEN_PF0_Pin GPIO_PIN_0
#define TERPS_IF_RXEN_PF0_GPIO_Port GPIOF
#define TERPS_IF_TXEN_PF1_Pin GPIO_PIN_1
#define TERPS_IF_TXEN_PF1_GPIO_Port GPIOF
#define BAT_LEVEL1_PF2_Pin GPIO_PIN_2
#define BAT_LEVEL1_PF2_GPIO_Port GPIOF
#define EXT_NRST_PF3_Pin GPIO_PIN_3
#define EXT_NRST_PF3_GPIO_Port GPIOF
#define BAT_LEVEL2_PF4_Pin GPIO_PIN_4
#define BAT_LEVEL2_PF4_GPIO_Port GPIOF
#define BAT_LEVEL3_PF5_Pin GPIO_PIN_5
#define BAT_LEVEL3_PF5_GPIO_Port GPIOF
#define POWER_KEY_PF8_Pin GPIO_PIN_8
#define POWER_KEY_PF8_GPIO_Port GPIOF
#define BT_KEY_PF9_Pin GPIO_PIN_9
#define BT_KEY_PF9_GPIO_Port GPIOF
#define STATUS_GREEN_PF10_Pin GPIO_PIN_10
#define STATUS_GREEN_PF10_GPIO_Port GPIOF
#define IR_SENS_DRIVE_PC0_Pin GPIO_PIN_0
#define IR_SENS_DRIVE_PC0_GPIO_Port GPIOC
#define IR_SENS_ADC_PC1_Pin GPIO_PIN_1
#define IR_SENS_ADC_PC1_GPIO_Port GPIOC
#define STEPPER_STCK_PA0_Pin GPIO_PIN_0
#define STEPPER_STCK_PA0_GPIO_Port GPIOA
#define P24V_MON_ADC1_PA1_Pin GPIO_PIN_1
#define P24V_MON_ADC1_PA1_GPIO_Port GPIOA
#define PM620_IF_UART2_TX_PA2_Pin GPIO_PIN_2
#define PM620_IF_UART2_TX_PA2_GPIO_Port GPIOA
#define PM620_IF_UART2_RX_PA3_Pin GPIO_PIN_3
#define PM620_IF_UART2_RX_PA3_GPIO_Port GPIOA
#define PM620_IF_UART2_RXEN_PA4_Pin GPIO_PIN_4
#define PM620_IF_UART2_RXEN_PA4_GPIO_Port GPIOA
#define P6V_MON_ADC1_PA5_Pin GPIO_PIN_5
#define P6V_MON_ADC1_PA5_GPIO_Port GPIOA
#define P5V_MON_ADC1_PA6_Pin GPIO_PIN_6
#define P5V_MON_ADC1_PA6_GPIO_Port GPIOA
#define P24V_EN_PA7_Pin GPIO_PIN_7
#define P24V_EN_PA7_GPIO_Port GPIOA
#define VALVE1_DIR_PC5_Pin GPIO_PIN_5
#define VALVE1_DIR_PC5_GPIO_Port GPIOC
#define VALVE2_DIR_PB1_Pin GPIO_PIN_1
#define VALVE2_DIR_PB1_GPIO_Port GPIOB
#define VALVE3_DIR_PF11_Pin GPIO_PIN_11
#define VALVE3_DIR_PF11_GPIO_Port GPIOF
#define VALVE3_nFAULT_PF13_Pin GPIO_PIN_13
#define VALVE3_nFAULT_PF13_GPIO_Port GPIOF
#define VALVE1_nFAULT_PF14_Pin GPIO_PIN_14
#define VALVE1_nFAULT_PF14_GPIO_Port GPIOF
#define VALVE2_nFAULT_PF15_Pin GPIO_PIN_15
#define VALVE2_nFAULT_PF15_GPIO_Port GPIOF
#define STEPPER_FLAG_PG0_Pin GPIO_PIN_0
#define STEPPER_FLAG_PG0_GPIO_Port GPIOG
#define STEPPER_STBY_RST_PG1_Pin GPIO_PIN_1
#define STEPPER_STBY_RST_PG1_GPIO_Port GPIOG
#define VALVE1_PWM_PE9_Pin GPIO_PIN_9
#define VALVE1_PWM_PE9_GPIO_Port GPIOE
#define FLASH_OCTSPI_CLK_PE10_Pin GPIO_PIN_10
#define FLASH_OCTSPI_CLK_PE10_GPIO_Port GPIOE
#define FLASH_OCTSPI_NCS_PE11_Pin GPIO_PIN_11
#define FLASH_OCTSPI_NCS_PE11_GPIO_Port GPIOE
#define FLASH_OCTSPI_IO0_PE12_Pin GPIO_PIN_12
#define FLASH_OCTSPI_IO0_PE12_GPIO_Port GPIOE
#define FLASH_OCTSPI_IO1_PE13_Pin GPIO_PIN_13
#define FLASH_OCTSPI_IO1_PE13_GPIO_Port GPIOE
#define FLASH_OCTSPI_IO2_PE14_Pin GPIO_PIN_14
#define FLASH_OCTSPI_IO2_PE14_GPIO_Port GPIOE
#define FLASH_OCTSPI_IO3_PE15_Pin GPIO_PIN_15
#define FLASH_OCTSPI_IO3_PE15_GPIO_Port GPIOE
#define TERPS_IF_UART3_I2C2_TX_PB10_Pin GPIO_PIN_10
#define TERPS_IF_UART3_I2C2_TX_PB10_GPIO_Port GPIOB
#define TERPS_IF_UART3_I2C2_RX_PB11_Pin GPIO_PIN_11
#define TERPS_IF_UART3_I2C2_RX_PB11_GPIO_Port GPIOB
#define P6V_EN_PB15_Pin GPIO_PIN_15
#define P6V_EN_PB15_GPIO_Port GPIOB
#define BAT_LEVEL5_PD8_Pin GPIO_PIN_8
#define BAT_LEVEL5_PD8_GPIO_Port GPIOD
#define STEPPER_SW_PD9_Pin GPIO_PIN_9
#define STEPPER_SW_PD9_GPIO_Port GPIOD
#define BAT_LEVEL4_PC9_Pin GPIO_PIN_9
#define BAT_LEVEL4_PC9_GPIO_Port GPIOC
#define BAROM_INT_DRDY_PD11_Pin GPIO_PIN_11
#define BAROM_INT_DRDY_PD11_GPIO_Port GPIOD
#define I2C4_SCL_PD12_Pin GPIO_PIN_12
#define I2C4_SCL_PD12_GPIO_Port GPIOD
#define I2C4_SDA_PD13_Pin GPIO_PIN_13
#define I2C4_SDA_PD13_GPIO_Port GPIOD
#define VALVE2_PWM_PD14_Pin GPIO_PIN_14
#define VALVE2_PWM_PD14_GPIO_Port GPIOD
#define VALVE3_PWM_PD15_Pin GPIO_PIN_15
#define VALVE3_PWM_PD15_GPIO_Port GPIOD
#define STEPPER_SPI1_SCK_PG2_Pin GPIO_PIN_2
#define STEPPER_SPI1_SCK_PG2_GPIO_Port GPIOG
#define STEPPER_SPI1_MISO_PG3_Pin GPIO_PIN_3
#define STEPPER_SPI1_MISO_PG3_GPIO_Port GPIOG
#define STEPPER_SPI1_MOSI_PG4_Pin GPIO_PIN_4
#define STEPPER_SPI1_MOSI_PG4_GPIO_Port GPIOG
#define STEPPER_SPI1_NSS_PG5_Pin GPIO_PIN_5
#define STEPPER_SPI1_NSS_PG5_GPIO_Port GPIOG
#define TEMP_ALERT_PG6_Pin GPIO_PIN_6
#define TEMP_ALERT_PG6_GPIO_Port GPIOG
#define TEMP_MON_I2C3_SCL_PG7_Pin GPIO_PIN_7
#define TEMP_MON_I2C3_SCL_PG7_GPIO_Port GPIOG
#define TEMP_MON_I2C3_SDA_PG8_Pin GPIO_PIN_8
#define TEMP_MON_I2C3_SDA_PG8_GPIO_Port GPIOG
#define USB_ENUM_PA8_Pin GPIO_PIN_8
#define USB_ENUM_PA8_GPIO_Port GPIOA
#define VBUS_DET_PA9_Pin GPIO_PIN_9
#define VBUS_DET_PA9_GPIO_Port GPIOA
#define USB_ID_PA10_Pin GPIO_PIN_10
#define USB_ID_PA10_GPIO_Port GPIOA
#define USB_uC_DATN_PA11_Pin GPIO_PIN_11
#define USB_uC_DATN_PA11_GPIO_Port GPIOA
#define USB_uC_DATP_PA12_Pin GPIO_PIN_12
#define USB_uC_DATP_PA12_GPIO_Port GPIOA
#define DPI620G_IF_UART4_TX_PC10_Pin GPIO_PIN_10
#define DPI620G_IF_UART4_TX_PC10_GPIO_Port GPIOC
#define DPI620G_IF_UART4_RX_PC11_Pin GPIO_PIN_11
#define DPI620G_IF_UART4_RX_PC11_GPIO_Port GPIOC
#define EXT_UART5_TX_PC12_Pin GPIO_PIN_12
#define EXT_UART5_TX_PC12_GPIO_Port GPIOC
#define MAIN_SPI2_NSS_PD0_Pin GPIO_PIN_0
#define MAIN_SPI2_NSS_PD0_GPIO_Port GPIOD
#define MAIN_SPI2_SCK_PD1_Pin GPIO_PIN_1
#define MAIN_SPI2_SCK_PD1_GPIO_Port GPIOD
#define EXT_UART5_RX_PD2_Pin GPIO_PIN_2
#define EXT_UART5_RX_PD2_GPIO_Port GPIOD
#define MAIN_SPI2_MISO_PD3_Pin GPIO_PIN_3
#define MAIN_SPI2_MISO_PD3_GPIO_Port GPIOD
#define MAIN_SPI2_MOSI_PD4_Pin GPIO_PIN_4
#define MAIN_SPI2_MOSI_PD4_GPIO_Port GPIOD
#define DPI620G_IF_RXEN_PD5_Pin GPIO_PIN_5
#define DPI620G_IF_RXEN_PD5_GPIO_Port GPIOD
#define CHGEN_PG10_Pin GPIO_PIN_10
#define CHGEN_PG10_GPIO_Port GPIOG
#define AC_PRESENT_PG12_Pin GPIO_PIN_12
#define AC_PRESENT_PG12_GPIO_Port GPIOG
#define BAT_I2C1_SDA_PG13_Pin GPIO_PIN_13
#define BAT_I2C1_SDA_PG13_GPIO_Port GPIOG
#define BAT_I2C1_SCL_PG14_Pin GPIO_PIN_14
#define BAT_I2C1_SCL_PG14_GPIO_Port GPIOG
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define BT_UART1_TX_PB6_Pin GPIO_PIN_6
#define BT_UART1_TX_PB6_GPIO_Port GPIOB
#define BT_UART1_RX_PB7_Pin GPIO_PIN_7
#define BT_UART1_RX_PB7_GPIO_Port GPIOB
#define BT_PROGRAM_PB8_Pin GPIO_PIN_8
#define BT_PROGRAM_PB8_GPIO_Port GPIOB
#define BT_ENABLE_PB9_Pin GPIO_PIN_9
#define BT_ENABLE_PB9_GPIO_Port GPIOB
#define P5V0_EN_PB15_Pin GPIO_PIN_15
#define CHGEN_PB0_Pin GPIO_PIN_0
#define VALVE2_ENABLE_Pin GPIO_PIN_10
#define BT_RED_PE5_Pin GPIO_PIN_5
#define VALVE1_ENABLE_Pin GPIO_PIN_1
#define BT_BLUE_PE4_Pin GPIO_PIN_4
#define BAT_LEVEL4_PC9_Pin GPIO_PIN_9
#define IR_SENS_DRIVE_PC6_Pin GPIO_PIN_6
#define VALVE1_DIR_PC7_Pin GPIO_PIN_7
#define VALVE2_DIR_PC8_Pin GPIO_PIN_8
#define VALVE3_DIR_PF3_Pin GPIO_PIN_3
#define VALVE3_ENABLE_Pin GPIO_PIN_6
#define FLASH_RESET_PF14_Pin GPIO_PIN_14
#define VALVE3_PWM_PE3_Pin GPIO_PIN_3
#define VALVE3_PWM_PE3_GPIO_Port GPIOE
#define VALVE1_ENABLE_Pin GPIO_PIN_1
#define VALVE1_ENABLE_GPIO_Port GPIOE
#define VALVE3_DIR_PF3_Pin GPIO_PIN_3
#define VALVE3_DIR_PF3_GPIO_Port GPIOF
#define VALVE1_DIR_PC7_Pin GPIO_PIN_7
#define VALVE1_DIR_PC7_GPIO_Port GPIOC
#define VALVE2_DIR_PC8_Pin GPIO_PIN_8
#define VALVE2_DIR_PC8_GPIO_Port GPIOC
#define VALVE3_ENABLE_Pin GPIO_PIN_6
#define VALVE3_ENABLE_GPIO_Port GPIOF
#define VALVE2_PWM_PF7_Pin GPIO_PIN_7
#define VALVE2_PWM_PF7_GPIO_Port GPIOF
#define VALVE2_ENABLE_Pin GPIO_PIN_10
#define VALVE2_ENABLE_GPIO_Port GPIOD
#define VALVE1_PWM_PE9_Pin GPIO_PIN_9
#define VALVE1_PWM_PE9_GPIO_Port GPIOE
#define BT_INDICATION_PE4_Pin GPIO_PIN_4
#define BT_INDICATION_PE4_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
