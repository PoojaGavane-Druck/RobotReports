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
/**
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
    greenThreePort = BAT_LEVEL4_PC9_GPIO_Port;
    greenFourPort = BAT_LEVEL5_PD8_GPIO_Port;
    
    redPin = BAT_LEVEL1_PF2_Pin;
    greenOnePin = BAT_LEVEL2_PF4_Pin;
    greenTwoPin = BAT_LEVEL3_PF5_Pin;
    greenThreePin = BAT_LEVEL4_PC9_Pin;
    greenFourPin = BAT_LEVEL5_PD8_Pin;
    
    /* Status leds */
    statusGreenPort = GPIOF;
    statusRedPort = GPIOE;
    statusBluePort = GPIOE;

    statusGreenPin = STATUS_GREEN_PF10_Pin;
    statusRedPin = STATUS_RED_PE2_Pin;
    statusBluePin = STATUS_BLUE_PE4_Pin;

   
    /* Bluetooth led */
    bluetoothPort = BT_INDICATION_PE5_GPIO_Port;
    bluetoothPin = BT_INDICATION_PE5_Pin;
    
    bluetoothBluePort= BT_INDICATION_PE5_GPIO_Port;
    bluetoothRedPort= BT_INDICATION_PE5_GPIO_Port;
    
    bluetoothBluePin = BT_INDICATION_PE4_Pin;
    bluetoothRedPin = BT_INDICATION_PE5_Pin;
    
    /* Reset all leds */
    ledsOffAll();
    /* Run startup check */
    ledsStartup();
    /* All off */
    ledsOffAll();
}

/**
 * @brief   Leds Class destructor
 * @param   smBus reference
 * @retval  void
 */
LEDS::~LEDS()
{
       
}

/**
 * @brief   Define startup state for LEDs
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledsStartup(void)
{
    //ledsOnAll();
}

/**
 * @brief   Updates the battery percentage on leds
 * @param   smBus reference
 * @retval  void
 */
void LEDS::updateBatteryLeds(float percentCharge, uint32_t chargeStatus)
{
    uint32_t maxLed = 0u;

    maxLed = getMaxLed(percentCharge);

    if(1u == maxLed)
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
    else if(2u == maxLed)
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
    else if(3u == maxLed)
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
    else if(4u == maxLed)
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
    else if(5u == maxLed)
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

/**
 * @brief   Updates the battery percentage on leds
 * @param   smBus reference
 * @retval  void
 */
uint32_t LEDS::getMaxLed(float32_t percentCap)
{
    uint32_t maxLeds = 0u;

    if((float32_t)(BATTERY_CAP_NULL) >= percentCap)
    {
        /* Raise error */
        maxLeds = 0u;
    }
    else if((float32_t)(BATTERY_CAP_5_PC) >= percentCap)
    {
        /* Red Led */
        maxLeds = 1u;
    }
    else if((float32_t)(BATTERY_CAP_20_PC) >= percentCap)
    {
        /* Red Led Solid On */
        maxLeds = 1u;
    }
    else if((float32_t)(BATTERY_CAP_40_PC) >= percentCap)
    {
        /* Green Led 1 Solid On */
        maxLeds = 2u;
    }
    else if((float32_t)(BATTERY_CAP_60_PC) >= percentCap)
    {
        /* Green Led 1 Solid On */
        maxLeds = 3u;
    }
    else if((float32_t)(BATTERY_CAP_80_PC) >= percentCap)
    {
        /* Green Led 1 Solid On */
        maxLeds = 4u;
    }
    else if((float32_t)(BATTERY_CAP_100_PC) >= percentCap)
    {
        /* Green Led 1 Solid On */
        maxLeds = 5u;
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
/**
 * @brief   Glows an LED
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledOn(eLeds_t led, eLedColour_t colour )
{
    switch(led)
    {
      case eStatusLed:

        if((eLedColour_t)eLedColourGreen == colour)
        {
            HAL_GPIO_WritePin(statusRedPort, statusRedPin, ledOffState);
            HAL_GPIO_WritePin(statusGreenPort, statusGreenPin, ledOnState);
        }
        else if((eLedColour_t)eLedColourYellow == colour)
        {
            HAL_GPIO_WritePin(statusRedPort, statusRedPin, ledOnState);
            HAL_GPIO_WritePin(statusGreenPort, statusGreenPin, ledOnState);          
        }
        else if((eLedColour_t)eLedColourRed == colour)
        {
            HAL_GPIO_WritePin(statusRedPort, statusRedPin, ledOnState);
            HAL_GPIO_WritePin(statusGreenPort, statusGreenPin, ledOffState);
        }
        else
        {
          /* Do Nothing */
        }
      break;
      
      case eBluetoothLed:
        
        if((eLedColour_t)eLedColourBlue == colour)
        {
          HAL_GPIO_WritePin(bluetoothBluePort, bluetoothBluePin, ledOnState);
          HAL_GPIO_WritePin(bluetoothRedPort, bluetoothRedPin, ledOffState);
          
        }
        else if((eLedColour_t)eLedColourPurple == colour)
        {
          HAL_GPIO_WritePin(bluetoothBluePort, bluetoothBluePin, ledOnState);
          HAL_GPIO_WritePin(bluetoothRedPort, bluetoothRedPin, ledOnState);
        }
        else if((eLedColour_t)eLedColourRed == colour)
        {
          HAL_GPIO_WritePin(bluetoothBluePort, bluetoothBluePin, ledOffState);
          HAL_GPIO_WritePin(bluetoothRedPort, bluetoothRedPin, ledOnState);
        }
        else
        {
          /* Do Nothing */
        }
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
          
      default:
      break;
    }

}

/**
 * @brief   Turns LED off
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledOff(eLeds_t led)
{
    switch(led)
    {
      case eStatusLed:
        HAL_GPIO_WritePin(statusRedPort, statusRedPin, ledOffState);
        HAL_GPIO_WritePin(statusGreenPort, statusGreenPin, ledOffState);
      break;
      
      case eBluetoothLed:
        HAL_GPIO_WritePin(bluetoothPort, bluetoothPin, ledOffState);          
        HAL_GPIO_WritePin(bluetoothBluePort, bluetoothBluePin, ledOffState);
        HAL_GPIO_WritePin(bluetoothRedPort, bluetoothRedPin, ledOffState);
      break;
      
      case eBatteryLed:
          HAL_GPIO_WritePin(redPort, redPin, ledOffState);
          HAL_GPIO_WritePin(greenOnePort, greenOnePin, ledOffState);
          HAL_GPIO_WritePin(greenTwoPort, greenTwoPin, ledOffState);
          HAL_GPIO_WritePin(greenThreePort, greenThreePin, ledOffState);
          HAL_GPIO_WritePin(greenFourPort, greenFourPin, ledOffState);
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

      default:
      break;
    }
}



/**
 * @brief   Glows all LEDS
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledsOffAll(void)
{
    ledOff(eBatteryLed);
    ledOff(eStatusLed);
    ledOff(eBluetoothLed);
}

/**
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
    ledOn(eStatusLed, eLedColourYellow);
    ledOn(eBluetoothLed, eLedColourPurple);
}

/**
 * @brief   Blinks LED
 * @param   smBus reference
 * @retval  void
 */
void LEDS::ledBlink(eLeds_t led, eLedColour_t colour )
{
    switch(led)
    {
    case eStatusLed:
      if((eLedColour_t)eLedColourGreen == colour)
      {
        HAL_GPIO_WritePin(statusRedPort, statusRedPin, ledOffState);
        HAL_GPIO_TogglePin(statusGreenPort, statusGreenPin);
      }
      else if((eLedColour_t)eLedColourYellow == colour)
      {
        HAL_GPIO_TogglePin(statusGreenPort, statusGreenPin);
        HAL_GPIO_TogglePin(statusRedPort, statusRedPin);
      }
      else if((eLedColour_t)eLedColourRed == colour)
      {
         HAL_GPIO_WritePin(statusGreenPort, statusGreenPin, ledOffState);  
         HAL_GPIO_TogglePin(statusRedPort, statusRedPin);
      }
      else
      {
        /* Do Nothing */
      }
    break;

    case eBluetoothLed:
      if((eLedColour_t)eLedColourBlue == colour)
      {
        HAL_GPIO_WritePin(bluetoothRedPort, bluetoothRedPin, ledOffState);
        HAL_GPIO_TogglePin(bluetoothBluePort, bluetoothBluePin);
      }
      else if((eLedColour_t)eLedColourPurple == colour)
      {
        HAL_GPIO_TogglePin(bluetoothBluePort, bluetoothBluePin);
        HAL_GPIO_TogglePin(bluetoothRedPort, bluetoothRedPin);
      }
      else if((eLedColour_t)eLedColourRed == colour)
      {
         HAL_GPIO_WritePin(bluetoothBluePort, bluetoothBluePin, ledOffState);
         HAL_GPIO_TogglePin(bluetoothRedPort, bluetoothRedPin);
      }
      else
      {
        /* Do Nothing */
      }
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
        
    default:
    break;
    }
}