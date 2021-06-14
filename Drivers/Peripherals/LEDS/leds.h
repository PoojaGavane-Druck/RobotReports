/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     leds.h
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     25 May 2021
*
* @brief    Leds header file
*/

#ifndef _LEDS_H_
#define _LEDS_H_

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <stdbool.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
MISRAC_ENABLE

/* Defines and constants  ---------------------------------------------------*/
#define BATTERY_CAP_NULL 0
#define DISCHARGING 0
#define CHARGING 1

/* Battery capacity percentages */
#define BATTERY_CAP_5_PC 5
#define BATTERY_CAP_20_PC 20
#define BATTERY_CAP_40_PC 40
#define BATTERY_CAP_60_PC 60
#define BATTERY_CAP_80_PC 80
#define BATTERY_CAP_100_PC 100 

/* Types --------------------------------------------------------------------*/
typedef enum: uint32_t
{
    eBatteryLedNone = 0x0000,
    eBatteryLedRed = 0x0001,
    eBatteryLedGreenOne = 0x0002,
    eBatteryLedGreenTwo = 0x0004,
    eBatteryLedGreenThree = 0x0008,
    eBatteryLedGreenFour = 0x0010,
    eStatusLedRed = 0x0020,
    eStatusLedGreen = 0x0040, 
    eStatusLedBlue = 0x0080,
    eBluetoothLed = 0x0100
}eLeds_t;

/* Variables ----------------------------------------------------------------*/
class LEDS
{
public:
    LEDS();
    ~LEDS();

    void updateBatteryLeds(float capacity, uint32_t chargingStatus);

private:
    GPIO_TypeDef *redPort;
    GPIO_TypeDef *greenOnePort;
    GPIO_TypeDef *greenTwoPort;
    GPIO_TypeDef *greenThreePort;
    GPIO_TypeDef *greenFourPort;
    GPIO_TypeDef *statusRedPort;
    GPIO_TypeDef *statusGreenPort;    
    GPIO_TypeDef *statusBluePort;
    GPIO_TypeDef *bluetoothPort;

    uint16_t redPin;
    uint16_t greenOnePin;
    uint16_t greenTwoPin;
    uint16_t greenThreePin;
    uint16_t greenFourPin;
    uint16_t statusRedPin;
    uint16_t statusGreenPin;
    uint16_t statusBluePin;
    uint16_t bluetoothPin;

    GPIO_PinState ledOnState;
    GPIO_PinState ledOffState;

    void ledsStartup(void);
    void ledBlink(eLeds_t led);
    void ledOn(eLeds_t led);
    void ledOff(eLeds_t led);
    void ledsOnAll(void);
    void ledsOffAll(void);
    uint32_t getMaxLed(float charge);
};
#endif