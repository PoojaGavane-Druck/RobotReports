#ifndef INC_MAINAPP_H_
#define INC_MAINAPP_H_

void MainApp(void);
void EnterLowPowerMode(void);
void ExitLowPowerMode(void);

#ifdef NUCLEO_BOARD
#define POWER_ON_OFF_BUTTON_Pin                 GPIO_PIN_13
#define POWER_ON_OFF_BUTTON_GPIO_Port           GPIOC

#define USART2_PM620_TX_ENABLE_PIN                GPIO_PIN_4
#define USART2_PM620_TX_ENABLE_GPIO_Port          GPIOA

#define USART3_DPI620G_TX_ENABLE_PIN              GPIO_PIN_1
#define USART3_DPI620G_TX_ENABLE_GPIO_Port        GPIOA
#endif

#endif /* INC_MAINAPP_H_ */