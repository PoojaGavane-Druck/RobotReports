/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     Leds.cpp
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     25 May 2021
*
* @brief    Leds source file
*/
//*********************************************************************************************************************

/* Includes -----------------------------------------------------------------*/
#include "leds.h"
#include "main.h"
/* Typedefs -----------------------------------------------------------------*/

/* Defines ------------------------------------------------------------------*/

/* Macros -------------------------------------------------------------------*/

/* Variables ----------------------------------------------------------------*/

/* Prototypes ---------------------------------------------------------------*/

/* User code ----------------------------------------------------------------*/
/*
 * @brief   Leds Class Constructor
 * @param   smBus reference
 * @retval  void
 */
LEDS::LEDS()
{
    ledOnState = GPIO_PIN_SET;
    ledOffState = GPIO_PIN_RESET;

    /* Battery leds */
    redPort = BAT_LEVEL1_PF2_GPIO_Port;
    greenOnePort = BAT_LEVEL2_PF4_GPIO_Port;
    greenTwoPort = BAT_LEVEL3_PF5_GPIO_Port;
    greenThreePort = BAT_LEVEL4_PD10_GPIO_Port; 
    greenFourPort = BAT_LEVEL5_PD8_GPIO_Port;

    redPin = BAT_LEVEL1_PF2_Pin;
    greenOnePin = BAT_LEVEL2_PF4_Pin;
    greenTwoPin = BAT_LEVEL3_PF5_Pin;
    greenThreePin = BAT_LEVEL4_PD10_Pin;
    greenFourPin = BAT_LEVEL5_PD8_Pin;

    /* Status leds */
    statusRedPort = GPIOE;
    statusGreenPort = GPIOF;
    statusBluePort = GPIOE;

    statusRedPin = STATUS_RED_PE2_Pin;
    statusGreenPin = STATUS_GREEN_PF10_Pin;
    statusBluePin = STATUS_BLUE_PE4_Pin;

    /* Bluetooth led */
    bluetoothPort = BT_INDICATION_PE5_GPIO_Port;
    bluetoothPin = BT_INDICATION_PE5_Pin;
    /* Reset all leds */
    ledsOffAll();
    /* Run startup check */
    ledsStartup();
    /* All off */
    ledsOffAll();
}

/*
 * @brief   Leds Class destructor
 * @param   smBus reference
 * @retval  void
 */
LEDS::~LEDS()
{
       
}


void LEDS::ledsStartup(void)
{
    ledsOnAll();
}

/*
 * @brief   Updates the battery percentage on leds
 * @param   smBus reference
 * @retval  void
 */
void LEDS::batteryLedsOff(void)
{
     ledOff(eBatteryLedRed);
     ledOff(eBatteryLedGreenOne);
     ledOff(eBatteryLedGreenTwo);
     ledOff(eBatteryLedGreenThree);
     ledOff(eBatteryLedGreenFour);
}

/*
 * @brief   Updates the battery percentage on leds
 * @param   smBus reference
 * @retval  void
 */
void LEDS::updateBatteryLeds(float percentCharge, uint32_t chargeStatus)
{
    uint32_t maxLed = (uint32_t)(0);

    maxLed = getMaxLed(percentCharge);

    if((uint32_t)(1) == maxLed)
    {
        ledOff(eBatteryLedGreenOne);
        ledOff(eBatteryLedGreenTwo);
        ledOff(eBatteryLedGreenThree);
        ledOff(eBatteryLedGreenFour);

        if((uint32_t)(DISCHARGING) == chargeStatus)
        {
            ledOn(eBatteryLedRed);
        }
        else
        {
            ledBlink(eBatteryLedRed);
        }
    }
    else if((uint32_t)(2) == maxLed)
    {
        ledOn(eBatteryLedRed);
        ledOff(eBatteryLedGreenTwo);
        ledOff(eBatteryLedGreenThree);
        ledOff(eBatteryLedGreenFour);
        if((uint32_t)(DISCHARGING) == chargeStatus)
        {
            ledOn(eBatteryLedGreenOne);
        }
        else
        {
            ledBlink(eBatteryLedGreenOne);
        }            
    }
    else if((uint32_t)(3) == maxLed)
    {
        ledOn(eBatteryLedRed);
        ledOn(eBatteryLedGreenOne);
        ledOff(eBatteryLedGreenThree);
        ledOff(eBatteryLedGreenFour);  
        if((uint32_t)(DISCHARGING) == chargeStatus)
        {
            ledOn(eBatteryLedGreenTwo);
        }
        else
        {
            ledBlink(eBatteryLedGreenTwo);
        }                         
    }
    else if((uint32_t)(4) == maxLed)
    {
        ledOn(eBatteryLedRed);
        ledOn(eBatteryLedGreenOne);
        ledOn(eBatteryLedGreenTwo);
        ledOff(eBatteryLedGreenFour); 
        if((uint32_t)(DISCHARGING) == chargeStatus)
        {
            ledOn(eBatteryLedGreenThree);
        }
        else
        {
            ledBlink(eBatteryLedGreenThree);
        }                          
    }
    else if((uint32_t)(5) == maxLed)
    {
        ledOn(eBatteryLedRed);
        ledOn(eBatteryLedGreenOne);
        ledOn(eBatteryLedGreenTwo);
        ledOn(eBatteryLedGreenThree);              
        if((uint32_t)(DISCHARGING) == chargeStatus)
        {
            ledOn(eBatteryLedGreenFour);
        }
        else
        {
            ledBlink(eBatteryLedGreenFour);
        }                        
    }    
    else
    {
        /* For misra */
    }                        
}
/*****************************************************************************/
/* SUPPRESS: floating point values shall not be tested for exact equality or 
inequality (MISRA C 2004 rule 13.3) */
/*****************************************************************************/
_Pragma ("diag_suppress=Pm046")

/*
 * @brief   Updates the battery percentage on leds
 * @param   smBus reference
 * @retval  void
 */
uint32_t LEDS::getMaxLed(float percentCap)
{
    uint32_t maxLeds = (uint32_t)(0);

    if((float)(BATTERY_CAP_NULL) >= percentCap)
    {
        /* Raise error */
        maxLeds = (uint32_t)(0);
    }
    else if((float)(BATTERY_CAP_5_PC) >= percentCap)
    {
        /* Red Led */
        maxLeds = (uint32_t)(1);
    }
    else if((float)(BATTERY_CAP_20_PC) >= percentCap)
    {
        /* Red Led Solid On */
        maxLeds = (uint32_t)(1);
    }
    else if((float)(BATTERY_CAP_40_PC) >= percentCap)
    {
        /* Green Led 1 Solid On */
        maxLeds = (uint32_t)(2);
    }
    else if((float)(BATTERY_CAP_60_PC) >= percentCap)
    {
        /* Green Led 1 Solid On */
        maxLeds = (uint32_t)(3);
    }
    else if((float)(BATTERY_CAP_80_PC) >= percentCap)
    {
        /* Green Led 1 Solid On */
        maxLeds = (uint32_t)(4);
    }
    else if((float)(BATTERY_CAP_100_PC) >= percentCap)
    {
        /* Green Led 1 Solid On */
        maxLeds = (uint32_t)(5);
    }    
    else
    {
        /* For misra */
    }  

    return maxLeds;
}

/*****************************************************************************/
/* DEFAULT : floating point values shall not be tested for exact equality or 
inequality (MISRA C 2004 rule 13.3) */
/*****************************************************************************/
_Pragma ("diag_default=Pm046")
/*
 * @brief   Glows an LED
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledOn(eLeds_t led)
{
    switch(led)
    {
    case eBatteryLedNone:
        /* No action */
        break;
    case eBatteryLedRed:
        HAL_GPIO_WritePin(redPort, redPin, ledOnState);
        break;
    case eBatteryLedGreenOne:
        HAL_GPIO_WritePin(greenOnePort, greenOnePin, ledOnState);
        break;
    case eBatteryLedGreenTwo:
        HAL_GPIO_WritePin(greenTwoPort, greenTwoPin, ledOnState);
        break;
    case eBatteryLedGreenThree:
        HAL_GPIO_WritePin(greenThreePort, greenThreePin, ledOnState);
        break;
    case eBatteryLedGreenFour:
        HAL_GPIO_WritePin(greenFourPort, greenFourPin, ledOnState);
        break;
    case eStatusLedRed:
        HAL_GPIO_WritePin(statusRedPort, statusRedPin, ledOnState);
        break;
    case eStatusLedGreen:
        HAL_GPIO_WritePin(statusGreenPort, statusGreenPin, ledOnState);
        break;
    case eStatusLedBlue:
        HAL_GPIO_WritePin(statusBluePort, statusBluePin, ledOnState);
        break;
    case eBluetoothLed:
        HAL_GPIO_WritePin(bluetoothPort, bluetoothPin, ledOnState);
        break;
    default:
        break;
    }
}

/*
 * @brief   Turns LED off
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledOff(eLeds_t led)
{
    switch(led)
    {
    case eBatteryLedNone:
        /* No action */
        break;
    case eBatteryLedRed:
        HAL_GPIO_WritePin(redPort, redPin, ledOffState);
        break;
    case eBatteryLedGreenOne:
        HAL_GPIO_WritePin(greenOnePort, greenOnePin, ledOffState);
        break;
    case eBatteryLedGreenTwo:
        HAL_GPIO_WritePin(greenTwoPort, greenTwoPin, ledOffState);
        break;
    case eBatteryLedGreenThree:
        HAL_GPIO_WritePin(greenThreePort, greenThreePin, ledOffState);
        break;
    case eBatteryLedGreenFour:
        HAL_GPIO_WritePin(greenFourPort, greenFourPin, ledOffState);
        break;
    case eStatusLedRed:
        HAL_GPIO_WritePin(statusRedPort, statusRedPin, ledOffState);
        break;
    case eStatusLedGreen:
        HAL_GPIO_WritePin(statusGreenPort, statusGreenPin, ledOffState);
        break;
    case eStatusLedBlue:
        HAL_GPIO_WritePin(statusBluePort, statusBluePin, ledOffState);
        break;
    case eBluetoothLed:
        HAL_GPIO_WritePin(bluetoothPort, bluetoothPin, ledOffState);
        break;
    default:
        break;
    }
}

/*
 * @brief   Blinks LED
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledBlink(eLeds_t led)
{
    switch(led)
    {
    case eBatteryLedNone:
        /* No action */
        break;
    case eBatteryLedRed:
        HAL_GPIO_TogglePin(redPort, redPin);
        break;
    case eBatteryLedGreenOne:
        HAL_GPIO_TogglePin(greenOnePort, greenOnePin);
        break;
    case eBatteryLedGreenTwo:
        HAL_GPIO_TogglePin(greenTwoPort, greenTwoPin);
        break;
    case eBatteryLedGreenThree:
        HAL_GPIO_TogglePin(greenThreePort, greenThreePin);
        break;
    case eBatteryLedGreenFour:
        HAL_GPIO_TogglePin(greenFourPort, greenFourPin);
        break;
    case eStatusLedRed:
        HAL_GPIO_TogglePin(statusRedPort, statusRedPin);
        break;
    case eStatusLedGreen:
        HAL_GPIO_TogglePin(statusGreenPort, statusGreenPin);
        break;
    case eStatusLedBlue:
        HAL_GPIO_TogglePin(statusBluePort, statusBluePin);
        break;
    case eBluetoothLed:
        HAL_GPIO_TogglePin(bluetoothPort, bluetoothPin);
        break;
    default:
        break;
    }
}

/*
 * @brief   Glows all LEDS
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledsOffAll(void)
{
    ledOff(eBatteryLedRed);
    ledOff(eBatteryLedGreenOne);
    ledOff(eBatteryLedGreenTwo);
    ledOff(eBatteryLedGreenThree);
    ledOff(eBatteryLedGreenFour);
    ledOff(eStatusLedRed);
    ledOff(eStatusLedGreen);
    ledOff(eStatusLedBlue);
    ledOff(eBluetoothLed);
}

/*
 * @brief   Turns off all LEDS
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledsOnAll(void)
{
    ledOn(eBatteryLedRed);
    ledOn(eBatteryLedGreenOne);
    ledOn(eBatteryLedGreenTwo);
    ledOn(eBatteryLedGreenThree);
    ledOn(eBatteryLedGreenFour);
    ledOn(eStatusLedRed);
    ledOn(eStatusLedGreen);
    ledOn(eStatusLedBlue);
    ledOn(eBluetoothLed);
}

/*
 * @brief   Turns off all LEDS
 * @param   smBus reference
 * @retval  void
 */
void LEDS::statusLed(eStatusLed_t status)
{
    switch(status)
    {
        case eStatusNone:
            statusLedControl(eStatusOff);
            break;

        case eStatusOkay:
            statusLedControl(eStatusGreen);
            break;

        case eStatusProcessing:
            statusLedControl(eStatusYellow);
            break;

        case eStatusError:
            statusLedControl(eStatusRed);
            break;

        default:
            statusLedControl(eStatusRed);
            break;
    }
}

/*
 * @brief   Turns off all LEDS
 * @param   smBus reference
 * @retval  void
 */
void LEDS::statusLedControl(eLedColour_t colour)
{
    switch(colour)
    {
    case eStatusOff:
        ledOff(eStatusLedBlue);        
        ledOff(eStatusLedRed);
        ledOff(eStatusLedGreen);             
        break;

    case eStatusRed:
        ledOff(eStatusLedBlue);        
        ledOff(eStatusLedGreen);
        ledOn(eStatusLedRed);                        
        break;
        
    case eStatusGreen:
        ledOff(eStatusLedBlue);        
        ledOff(eStatusLedRed);
        ledOn(eStatusLedGreen);         
        break;

    case eStatusBlue:
        ledOff(eStatusLedGreen);        
        ledOff(eStatusLedRed);
        ledOn(eStatusLedBlue);
        break;
            
    case eStatusCyan:
        ledOff(eStatusLedRed);        
        ledOn(eStatusLedBlue);
        ledOn(eStatusLedGreen);      
        break;
        
    case eStatusYellow:
        ledOff(eStatusLedBlue);        
        ledOn(eStatusLedRed);
        ledOn(eStatusLedGreen);      
        break;

    case eStatusMagenta:
        ledOff(eStatusLedGreen);        
        ledOn(eStatusLedRed);
        ledOn(eStatusLedBlue);      
        break;

    case eStatusWhite:
        ledOn(eStatusLedBlue);        
        ledOn(eStatusLedRed);
        ledOn(eStatusLedGreen);      
        break;

    default:
        break;
    }
}