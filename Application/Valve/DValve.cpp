/**
* @file     DValve.cpp
* @version  1.00.00
* @author   Pooja Gavane
* @date     28 January 2020
*
* @brief    The DSlot class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <main.h>
#include <Types.h>

MISRAC_ENABLE

#include "DValve.h"
#include "memory.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
/* Prototypes -------------------------------------------------------------------------------------------------------*/
/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DValve class constructor
 * @note    
 * @param   owner: the task that created this slot
 * @retval  void
 */
//DValve::DValve(sValveParameters_t *valveParams)
DValve::DValve(TIM_HandleTypeDef* tim,uint32_t channel,GPIO_TypeDef* dirPort, uint16_t dirPin)
{ 
   timer = tim;
   timChannel = channel;
   portName = dirPort;
   pinNumber = dirPin;
}
/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */

void DValve::triggerValve(eValveState_t valveState)
{      
  if ((eValveState_t)VALVE_STATE_ON == valveState)
  {
     HAL_GPIO_WritePin(portName, pinNumber, GPIO_PIN_RESET);
     HAL_TIM_Base_Init(timer);
     HAL_TIM_Base_Start(timer);
     HAL_TIM_OnePulse_Start_IT(timer, timChannel);
     currentValveState = (eValveState_t)VALVE_STATE_ON;
  }
  else if ((eValveState_t)VALVE_STATE_OFF == valveState)
  {
    HAL_GPIO_WritePin(portName, pinNumber, GPIO_PIN_SET);
    HAL_TIM_Base_Init(timer);
    HAL_TIM_Base_Start(timer);
    HAL_TIM_OnePulse_Start_IT(timer, timChannel);
    currentValveState = (eValveState_t)VALVE_STATE_OFF;
  }
  else
  {
  //NOP
  }
  
  //if add on off enum for valve state
}


void DValve::valveTest(eValveFunctions valFunction)
{   
  switch (valFunction)
  {
  case (E_VALVE_FUNCTION_SHUTDOWN):
       HAL_GPIO_WritePin(portName, pinNumber, GPIO_PIN_RESET);
  break;
  
  case (E_VALVE_FUNCTION_BRAKE):
       HAL_GPIO_WritePin(portName, pinNumber, GPIO_PIN_RESET);
  break;  
  
  case (E_VALVE_FUNCTION_REVERSE):
    HAL_GPIO_WritePin(portName, pinNumber, GPIO_PIN_RESET);  
    HAL_TIM_Base_Init(timer);
    HAL_TIM_Base_Start(timer);
    HAL_TIM_OnePulse_Start_IT(timer, timChannel);
    break;  
    
   case (E_VALVE_FUNCTION_FORWARD):
    HAL_GPIO_WritePin(portName, pinNumber, GPIO_PIN_SET);  
    HAL_TIM_Base_Init(timer);
    HAL_TIM_Base_Start(timer);
    HAL_TIM_OnePulse_Start_IT(timer, timChannel);
    break;
    
   case (E_VALVE_FUNCTION_CURRUENT_REG1):
    HAL_GPIO_WritePin(portName, pinNumber, GPIO_PIN_RESET);
    HAL_TIM_Base_Init(timer);
    HAL_TIM_Base_Start(timer);
    HAL_TIM_OnePulse_Start_IT(timer, timChannel);
    break;
    
   case (E_VALVE_FUNCTION_CURRENT_REG2):
    HAL_TIM_Base_Init(timer);
    HAL_TIM_Base_Start(timer);
    HAL_TIM_OnePulse_Start_IT(timer, timChannel);
    HAL_GPIO_WritePin(portName, pinNumber, GPIO_PIN_SET);
    break;
    
  default:  
    break;
  }
 }






