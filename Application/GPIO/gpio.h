/**
* BHGE Confidential
* Copyright 2019.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are protected by trade
* secret or copyright law.  Dissemination of this information or reproduction of this material is strictly forbidden unless prior written
* permission is obtained from Baker Hughes.
*
*
* @file     gpio.h
* @version  1.0
* @author   Julio Andrade
* @date     Jun 22, 2018
* @brief
*/

#ifndef  __GPIO_H
#define  __GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <stdbool.h>
#include <stm32l4xx_hal.h>
//#include <HAL_I2C_Callback.h>
#include <stdio.h>
MISRAC_ENABLE

#if defined(STM32L431xx)
#define BRDV0_GPIO_PIN          GPIO_PIN_13
#define BRDV0_GPIO_PORT         GPIOC

#define BRDV1_GPIO_PIN          GPIO_PIN_5
#define BRDV1_GPIO_PORT         GPIOA

#define BRDV2_GPIO_PIN          GPIO_PIN_6
#define BRDV2_GPIO_PORT         GPIOA

#define BRDV3_GPIO_PIN          GPIO_PIN_7
#define BRDV3_GPIO_PORT         GPIOA

#define DSP0_GPIO_PORT          GPIOH
#define DSP0_GPIO_PIN           GPIO_PIN_0

#define DSP1_GPIO_PORT          GPIOH
#define DSP1_GPIO_PIN           GPIO_PIN_1

#define RS485_TEN_GPIO_PIN      GPIO_PIN_12
#define RS485_TEN_GPIO_PORT     GPIOA

#define DISP_RST_GPIO_PIN       GPIO_PIN_12
#define DISP_RST_GPIO_PORT      GPIOB

#define RS485_REN_GPIO_PIN      GPIO_PIN_11
#define RS485_REN_GPIO_PORT     GPIOA

#define INTPS_ON_OFF_PORT       GPIOB
#define INTPS_ON_OFF_PIN        GPIO_PIN_7

#define EXTSEN_ON_OFF_GPIO_PIN  GPIO_PIN_6
#define EXTSEN_ON_OFF_GPIO_PORT GPIOB

#define EXTSENAL_GPIO_PIN       GPIO_PIN_5
#define EXTSENAL_GPIO_PORT      GPIOB
#define EXTSENAL_EXTI_IRQn      EXTI9_5_IRQn

#define EXTSENCURR_GPIO_PIN     GPIO_PIN_8
#define EXTSENCURR_GPIO_PORT    GPIOA

#define ON_OFF_GPIO_PIN         GPIO_PIN_4
#define ON_OFF_GPIO_PORT        GPIOA
#define ON_OFF_EXTI_IRQn        EXTI4_IRQn

#define ON_OFF_SW_GPIO_PIN      GPIO_PIN_3
#define ON_OFF_SW_GPIO_PORT     GPIOA

#define FILTER_GPIO_PIN         GPIO_PIN_2
#define FILTER_GPIO_PORT        GPIOB
#define FILTER_EXTI_IRQn        EXTI2_IRQn

#define LEAK_GPIO_PIN           GPIO_PIN_10
#define LEAK_GPIO_PORT          GPIOB
#define LEAK_EXTI_IRQn          EXTI15_10_IRQn

#define TARE_GPIO_PIN           GPIO_PIN_1
#define TARE_GPIO_PORT          GPIOB
#define TARE_EXTI_IRQn          EXTI1_IRQn

#define UNITS_GPIO_PIN          GPIO_PIN_0
#define UNITS_GPIO_PORT         GPIOB
#define UNITS_EXTI_IRQn         EXTI0_IRQn

#define BACKLIGHT_GPIO_PIN      GPIO_PIN_11
#define BACKLIGHT_GPIO_PORT     GPIOB
#define BACKLIGHT_EXTI_IRQn     EXTI15_10_IRQn


#define LCD_BACKLIGHT_GPIO_PIN  GPIO_PIN_15
#define LCD_BACKLIGHT_GPIO_PORT GPIOB

#define gpioGetFilter()      HAL_GPIO_ReadPin((GPIO_TypeDef *)FILTER_GPIO_PORT,     FILTER_GPIO_PIN)
#define gpioGetLeak()        HAL_GPIO_ReadPin((GPIO_TypeDef *)LEAK_GPIO_PORT,       LEAK_GPIO_PIN)
#define gpioGetTare()        HAL_GPIO_ReadPin((GPIO_TypeDef *)TARE_GPIO_PORT,       TARE_GPIO_PIN)
#define gpioGetUnits()       HAL_GPIO_ReadPin((GPIO_TypeDef *)UNITS_GPIO_PORT,      UNITS_GPIO_PIN)
#define gpioGetBacklight()   HAL_GPIO_ReadPin((GPIO_TypeDef *)BACKLIGHT_GPIO_PORT,  BACKLIGHT_GPIO_PIN)
#define gpioGetOnOff()       HAL_GPIO_ReadPin((GPIO_TypeDef *)ON_OFF_GPIO_PORT,     ON_OFF_GPIO_PIN)

#define gpioGetBRV0()        HAL_GPIO_ReadPin((GPIO_TypeDef *)BRDV0_GPIO_PORT,      BRDV0_GPIO_PIN)
#define gpioGetBRV1()        HAL_GPIO_ReadPin((GPIO_TypeDef *)BRDV1_GPIO_PORT,      BRDV1_GPIO_PIN)
#define gpioGetBRV2()        HAL_GPIO_ReadPin((GPIO_TypeDef *)BRDV2_GPIO_PORT,      BRDV2_GPIO_PIN)
#define gpioGetBRV3()        HAL_GPIO_ReadPin((GPIO_TypeDef *)BRDV2_GPIO_PORT,      BRDV3_GPIO_PIN)

#define gpioGetDISP0()       HAL_GPIO_ReadPin((GPIO_TypeDef *)DSP0_GPIO_PORT,       DSP0_GPIO_PIN)
#define gpioGetDISP1()       HAL_GPIO_ReadPin((GPIO_TypeDef *)DSP1_GPIO_PORT,       DSP1_GPIO_PIN)

#define gpioGetExtPin()      HAL_GPIO_ReadPin((GPIO_TypeDef *)EXTSENAL_GPIO_PORT,   EXTSENAL_GPIO_PIN)

#endif

#define overcurrentControlEnable()    HAL_GPIO_WritePin((GPIO_TypeDef *)EXTSENCURR_GPIO_PORT, EXTSENCURR_GPIO_PIN, GPIO_PIN_SET)
#define overcurrentControlDisable()   HAL_GPIO_WritePin((GPIO_TypeDef *)EXTSENCURR_GPIO_PORT, EXTSENCURR_GPIO_PIN, GPIO_PIN_RESET)

#define onOffSwEnable()               HAL_GPIO_WritePin((GPIO_TypeDef *)ON_OFF_SW_GPIO_PORT,  ON_OFF_SW_GPIO_PIN, GPIO_PIN_SET )

#define intPsEnable()                 HAL_GPIO_WritePin((GPIO_TypeDef *)INTPS_ON_OFF_PORT,    INTPS_ON_OFF_PIN, GPIO_PIN_SET )
#define intPsDisable()                HAL_GPIO_WritePin((GPIO_TypeDef *)INTPS_ON_OFF_PORT,    INTPS_ON_OFF_PIN, GPIO_PIN_RESET )
#define i2c1SCLSet()                  HAL_GPIO_WritePin((GPIO_TypeDef *)DPI705E_I2C1_SCL_GPIO_PORT,  DPI705E_I2C1_SCL_PIN, GPIO_PIN_SET )
#define i2c1SCLReset()                HAL_GPIO_WritePin((GPIO_TypeDef *)DPI705E_I2C1_SCL_GPIO_PORT,  DPI705E_I2C1_SCL_PIN, GPIO_PIN_RESET )
#define i2c1SDASet()                  HAL_GPIO_WritePin((GPIO_TypeDef *)DPI705E_I2C1_SDA_GPIO_PORT,  DPI705E_I2C1_SDA_PIN, GPIO_PIN_SET )
#define i2c1SDAReset()                HAL_GPIO_WritePin((GPIO_TypeDef *)DPI705E_I2C1_SDA_GPIO_PORT,  DPI705E_I2C1_SDA_PIN, GPIO_PIN_RESET )

#define dispRetEnable()               HAL_GPIO_WritePin((GPIO_TypeDef *) DISP_RST_GPIO_PORT,  DISP_RST_GPIO_PIN, GPIO_PIN_SET)
#define dispRetDisable()              HAL_GPIO_WritePin((GPIO_TypeDef *) DISP_RST_GPIO_PORT,  DISP_RST_GPIO_PIN, GPIO_PIN_RESET)


#define  rs485SetTxEnable()           HAL_GPIO_WritePin((GPIO_TypeDef *) RS485_REN_GPIO_PORT,  RS485_REN_GPIO_PIN,  GPIO_PIN_SET ); HAL_GPIO_WritePin((GPIO_TypeDef *) RS485_TEN_GPIO_PORT,  RS485_TEN_GPIO_PIN, GPIO_PIN_SET )
#define  rs485SetTxDisable()          HAL_GPIO_WritePin((GPIO_TypeDef *) RS485_TEN_GPIO_PORT,  RS485_TEN_GPIO_PIN, GPIO_PIN_RESET); HAL_GPIO_WritePin((GPIO_TypeDef *) RS485_REN_GPIO_PORT,  RS485_REN_GPIO_PIN, GPIO_PIN_RESET )

#define  extSensorOnOffLatchEnable()    HAL_GPIO_WritePin((GPIO_TypeDef *) EXTSEN_ON_OFF_GPIO_PORT,  EXTSEN_ON_OFF_GPIO_PIN, GPIO_PIN_SET)
#define  extSensorOnOffLatchDisable()   HAL_GPIO_WritePin((GPIO_TypeDef *) EXTSEN_ON_OFF_GPIO_PORT,  EXTSEN_ON_OFF_GPIO_PIN, GPIO_PIN_RESET)

#define  backlightEnable()         HAL_GPIO_WritePin((GPIO_TypeDef *) LCD_BACKLIGHT_GPIO_PORT,  LCD_BACKLIGHT_GPIO_PIN, GPIO_PIN_SET)
#define  disableBacklight()        HAL_GPIO_WritePin((GPIO_TypeDef *) LCD_BACKLIGHT_GPIO_PORT,  LCD_BACKLIGHT_GPIO_PIN, GPIO_PIN_RESET)

/* BITS
if units key is down then set bit 0 of uint32_t
if tare key is down then set bit 1
if filter key is down then set bit 2

if leak key is down then set bit 10
if backlight key is down then set bit 11
*/
typedef union
{
    struct
    {
        uint32_t TopLeft        : 1;
        uint32_t TopMiddle      : 1;
        uint32_t TopRight       : 1;
        uint32_t BottomLeft     : 1;
        uint32_t BottomMiddle   : 1;
        uint32_t BottomRight    : 1;

        uint32_t LongPress : 1;
        uint32_t remote    : 1;
        uint32_t reserved  : 24;

    } bit;

    uint32_t bytes;

} gpioButtons_t;

typedef union
{
    struct
    {
        uint32_t bv0   : 1;
        uint32_t bv1   : 1;
        uint32_t bv2   : 1;
        uint32_t bv3   : 1;

    } bit;

    uint32_t bytes;

} gpioBV_t;

typedef union
{
    struct
    {
        uint32_t dsp0  : 1;
        uint32_t dsp1  : 1;
    } bit;

    uint32_t bytes;

} gpioDISPV_t;

gpioBV_t gpioGetBoardVer(void);
gpioDISPV_t gpioGetDispVer(void);

void  gpioInit(void);
void GPIOsleep(void);

OS_ERR gpioKeyPost (bool remote, gpioButtons_t keys);
OS_ERR gpio_IRQWait(uint32_t max, gpioButtons_t *buttonFflag);

void gpioEnIrq(void);

/* gpio interrupt handlers */
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI0_IRQHandler(void);
void GPOIO_DisIRQ(void);
void EXTI15_10_IRQHandler(void);

gpioButtons_t GPIO_getButtons(void);

void powerEnableInit(void);
void onOffSwDisable(void);

#ifdef __cplusplus
}
#endif

#endif  /* End of module include. */
