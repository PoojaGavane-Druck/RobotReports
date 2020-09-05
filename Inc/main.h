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
extern SMBUS_HandleTypeDef  hsmbus1;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* USER CODE BEGIN Private defines */
//Version Information Details
#define BUILD_NUMBER 1u
#define MAJOR_VERSION_NUMBER 2u
#define MINOR_VERSION_NUMBER 0u 

#define USART1_TX_ENABLE_PIN                     GPIO_PIN_2
#define USART1_TX_ENABLE_GPIO_Port                GPIOE

#define USART2_PM620_TX_ENABLE_PIN                GPIO_PIN_4
#define USART2_PM620_TX_ENABLE_GPIO_Port          GPIOA

#define USART3_DPI620G_TX_ENABLE_PIN              GPIO_PIN_1
#define USART3_DPI620G_TX_ENABLE_GPIO_Port        GPIOA

#define USART4_TX_ENABLE_PIN                      GPIO_PIN_2
#define USART4_TX_ENABLE_GPIO_Port                GPIOE

#define USART5_RS485_TX_ENABLE_PIN                GPIO_PIN_10
#define USART5_RS485_TX_ENABLE_GPIO_Port          GPIOD

#define USART5_RS485_RX_ENABLE_PIN                GPIO_PIN_10
#define USART5_RS485_RX_ENABLE_GPIO_Port          GPIOD

#define BAT_INTERFACE I2C1

#define BAT_INTERFACE_I2C1_SDA_Pin          GPIO_PIN_13
#define BAT_INTERFACE_I2C1_SDA_GPIO_Port    GPIOG

#define BAT_INTERFACE_I2C1_SCL_Pin          GPIO_PIN_14
#define BAT_INTERFACE_I2C1_SCL_GPIO_Port    GPIOG

#define BAT_INTERFACE_I2C1_AF_NUM           GPIO_AF4_I2C1
#ifdef CONTROLLER_BOARD 
#define BAROMETER_EEPROM_INTERFACE          I2C4

#define BAROMETER_EEPROM_INTERFACE_I2C4_SDA_Pin          GPIO_PIN_13
#define BAROMETER_EEPROM_INTERFACE_I2C4_SDA_GPIO_Port    GPIOD

#define  BAROMETER_EEPROM_INTERFACE_I2C4_SCL_Pin          GPIO_PIN_12
#define  BAROMETER_EEPROM_INTERFACE_I2C4_SCL_GPIO_Port    GPIOD
#define  BAROMETER_EEPROM_INTERFACE_I2C4_AF_NUM           GPIO_AF4_I2C4
#define  BAROMETER_EEPROM_I2C4_GPIO_PORT_CLOCK_ENABLE     __HAL_RCC_GPIOD_CLK_ENABLE();

#else

// TODO changed for barometer testing
#define  BAROMETER_EEPROM_INTERFACE_I2C4_AF_NUM           GPIO_AF4_I2C2

#define BAROMETER_EEPROM_INTERFACE          I2C2 //CHANGED FROM 4 TO 2

#define BAROMETER_EEPROM_INTERFACE_I2C4_SDA_Pin          GPIO_PIN_0
#define BAROMETER_EEPROM_INTERFACE_I2C4_SDA_GPIO_Port    GPIOF

#define  BAROMETER_EEPROM_INTERFACE_I2C4_SCL_Pin          GPIO_PIN_1
#define  BAROMETER_EEPROM_INTERFACE_I2C4_SCL_GPIO_Port    GPIOF
                       
#endif

#define DEVICE_STATUS_LED_GPIO_Port            GPIOE
#define DEVICE_STATUS_LED1_Pin                 GPIO_PIN_2
#define DEVICE_STATUS_LED1_GPIO_Port           GPIOE

#define DEVICE_STATUS_LED2_Pin                 GPIO_PIN_3
#define DEVICE_STATUS_LED2_GPIO_Port           GPIOE

#define DEVICE_STATUS_LED3_Pin                 GPIO_PIN_4
#define DEVICE_STATUS_LED3_GPIO_Port           GPIOE

#define DEVICE_STATUS_LED4_Pin                 GPIO_PIN_5
#define DEVICE_STATUS_LED4_GPIO_Port           GPIOE

#define DEVICE_STATUS_LED5_Pin                 GPIO_PIN_6
#define DEVICE_STATUS_LED5_GPIO_Port           GPIOE




#define BATTERY_STATUS_LED_GPIO_Port            GPIOF
#define BATTERY_STATUS_LED1_Pin                 GPIO_PIN_2
#define BATTERY_STATUS_LED1_GPIO_Port           GPIOF

#define BATTERY_STATUS_LED2_Pin                 GPIO_PIN_3
#define BATTERY_STATUS_LED2_GPIO_Port           GPIOF

#define BATTERY_STATUS_LED3_Pin                 GPIO_PIN_4
#define BATTERY_STATUS_LED3_GPIO_Port           GPIOF

#define BATTERY_STATUS_LED4_Pin                 GPIO_PIN_5
#define BATTERY_STATUS_LED4_GPIO_Port           GPIOF

#define BATTERY_STATUS_LED5_Pin                 GPIO_PIN_6
#define BATTERY_STATUS_LED5_GPIO_Port           GPIOF

#define BATTERY_STATUS_LED6_Pin                 GPIO_PIN_7
#define BATTERY_STATUS_LED6_GPIO_Port           GPIOF



#define STEPPER_MOTOR_CK_Pin                      GPIO_PIN_0
#define STEPPER_MOTOR_CK_GPIO_Port                GPIOG

#define STEPPER_MOTOR_DIRECTION_Pin               GPIO_PIN_1
#define STEPPER_MOTOR_DIRECTION_GPIO_Port         GPIOG


#define STEPPER_MOTOR_MODE_GPIO_Port              GPIOE
#define STEPPER_MOTOR_MODE1_Pin                   GPIO_PIN_7
#define STEPPER_MOTOR_MODE2_Pin                   GPIO_PIN_8
#define STEPPER_MOTOR_MODE3_Pin                   GPIO_PIN_9

#define BLUETOOTH_ENABLE_Pin                     GPIO_PIN_6
#define BLUETOOTH_ENABLE_GPIO_Port               GPIOE

#define BLUETOOTH_RUN_MODE_Pin                   GPIO_PIN_7
#define BLUETOOTH_RUN_MODE_GPIO_Port             GPIOE

#define POWER_BUTTON_Pin                         GPIO_PIN_8
#define POWER_BUTTON_GPIO_Port                   GPIOF

#define BLUETOOTH_BUTTON_Pin                     GPIO_PIN_9
#define BLUETOOTH_BUTTON_GPIO_Port               GPIOF

#define VENT_VALVE_CONTROL_Pin                   GPIO_PIN_8
#define VENT_VALVE_CONTROL_GPIO_Port             GPIOC

#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB

#define USB_PowerSwitchOn_Pin               GPIO_PIN_5
#define USB_PowerSwitchOn_GPIO_Port         GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_6
#define USB_OverCurrent_GPIO_Port GPIOG

#define USB_PEN_GPIO_Pin GPIO_PIN_9
#define USB_PEN_GPIO_GPIO_Port GPIOC
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA

#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB



#define USART1_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()

#define USART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define USART1_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USART1_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USART1_TX_PIN                    GPIO_PIN_6
#define USART1_TX_GPIO_PORT              GPIOB
#define USART1_TX_AF                     GPIO_AF7_USART1
#define USART1_RX_PIN                    GPIO_PIN_7
#define USART1_RX_GPIO_PORT              GPIOB
#define USART1_RX_AF                     GPIO_AF7_USART1

#define USART1_RX_PULL                   GPIO_PULLUP
#define USART1_TX_PULL                   GPIO_NOPULL









#define USART2_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()

#define USART2_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USART2_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USART2_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USART2_TX_PIN                    GPIO_PIN_2
#define USART2_TX_GPIO_PORT              GPIOA
#define USART2_TX_AF                     GPIO_AF7_USART2
#define USART2_RX_PIN                    GPIO_PIN_3
#define USART2_RX_GPIO_PORT              GPIOA
#define USART2_RX_AF                     GPIO_AF7_USART2

#define USART2_RX_PULL                   GPIO_PULLUP
#define USART2_TX_PULL                   GPIO_NOPULL

/* Definition for USARTx's NVIC */

#define USART3_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE()

#define USART3_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USART3_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()

#define USART3_FORCE_RESET()             __HAL_RCC_USART3_FORCE_RESET()
#define USART3_RELEASE_RESET()           __HAL_RCC_USART3_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USART3_TX_PIN              GPIO_PIN_8
#define USART3_TX_GPIO_PORT              GPIOD
#define USART3_TX_AF                     GPIO_AF7_USART3
#define USART3_RX_PIN                    GPIO_PIN_9
#define USART3_RX_GPIO_PORT              GPIOD
#define USART3_RX_AF                     GPIO_AF7_USART3

#define USART3_RX_PULL                   GPIO_PULLUP
#define USART3_TX_PULL                   GPIO_NOPULL


#define UART4_CLK_ENABLE()             __HAL_RCC_UART4_CLK_ENABLE()
#define UART4_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define UART4_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()

#define UART4_FORCE_RESET()           __HAL_RCC_UART4_FORCE_RESET()
#define UART4_RELEASE_RESET()         __HAL_RCC_UART4_RELEASE_RESET()

/* Definition for USARTx Pins */
#define UART4_TX_PIN                    GPIO_PIN_9
#define UART4_TX_GPIO_PORT              GPIOA
#define UART4_TX_AF                     GPIO_AF8_UART4
#define UART4_RX_PIN                    GPIO_PIN_10
#define UART4_RX_GPIO_PORT              GPIOA
#define UART4_RX_AF                     GPIO_AF8_UART4

#define UART4_RX_PULL                   GPIO_PULLUP
#define UART4_TX_PULL                   GPIO_NOPULL

#define UART5_CLK_ENABLE()             __HAL_RCC_UART5_CLK_ENABLE()

#define UART5_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define UART5_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOD_CLK_ENABLE()

#define UART5_FORCE_RESET()           __HAL_RCC_UART5_FORCE_RESET()
#define UART5_RELEASE_RESET()         __HAL_RCC_UART5_RELEASE_RESET()

/* Definition for USARTx Pins */
#define UART5_TX_PIN                    GPIO_PIN_12
#define UART5_TX_GPIO_PORT              GPIOC
#define UART5_TX_AF                     GPIO_AF8_UART5
#define UART5_RX_PIN                    GPIO_PIN_2
#define UART5_RX_GPIO_PORT              GPIOD
#define UART5_RX_AF                     GPIO_AF8_UART5

#define UART5_RX_PULL                   GPIO_PULLUP
#define UART5_TX_PULL                   GPIO_NOPULL


/* Global variables */
extern RTC_HandleTypeDef hrtc;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
