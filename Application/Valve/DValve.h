/**
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file		DValve.c
* @version	1.00.00
* @author	Makarand Deshmukh
* @date		31-08-2021
*
* @brief	Header File for Valve Class
*/

#ifndef __DVALVE_H__
#define __DVALVE_H__

/* Includes -----------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_def.h>
#include <os.h>
#include <Types.h>
MISRAC_ENABLE

/* Defines and constants ----------------------------------------------------*/

/* Types --------------------------------------------------------------------*/
typedef enum : uint8_t
{
    E_VALVE_FUNCTION_SHUTDOWN =0,
    E_VALVE_FUNCTION_BRAKE,
    E_VALVE_FUNCTION_REVERSE,
    E_VALVE_FUNCTION_FORWARD,
    E_VALVE_FUNCTION_CURRUENT_REG1,
    E_VALVE_FUNCTION_CURRENT_REG2
}eValveFunctions_t;

typedef enum: uint8_t
{  
    VALVE_STATE_ON = 0,
    VALVE_STATE_OFF
}eValveState_t;

/* Variables ----------------------------------------------------------------*/

class DValve
{
private:
   TIM_HandleTypeDef* timer;
   uint32_t timChannel;
   
   GPIO_TypeDef* dirPortName ;
   uint16_t dirPinNumber ;
   
   GPIO_TypeDef* pwmPortName ;
   uint16_t pwmPinNumber;
   eValveState_t currentValveState;
   
public:
   DValve(TIM_HandleTypeDef* tim, 
           GPIO_TypeDef* pwmPort, uint16_t pwmPin, 
           GPIO_TypeDef* dirPort, uint16_t dirPin);
   ~DValve();
   void triggerValve(eValveState_t valveState);
   void valveTest(eValveFunctions_t valFunction);
   void setValveTimer(uint32_t valveTime);
};

#endif /* DValve.h*/
