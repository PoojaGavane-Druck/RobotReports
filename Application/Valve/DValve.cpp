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
* @file     DValve.c
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     31-08-2021
*
* @brief    Source file for valve class
*/

/* Includes -----------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <main.h>
#include <Types.h>

MISRAC_ENABLE

#include "DValve.h"
#include "memory.h"

/* Defines and constants ----------------------------------------------------*/

/* Types --------------------------------------------------------------------*/

/* Global Variables ---------------------------------------------------------*/

/* File Statics -------------------------------------------------------------*/

/* User Code ----------------------------------------------------------------*/

/**
* @brief    DValve class constructor
* @param    void
* @retval   void
*/
DValve::DValve(TIM_HandleTypeDef *tim,
               uint16_t channel,
               GPIO_TypeDef *dirPort,
               uint16_t dirPin,
               GPIO_TypeDef *enPort,
               uint16_t enPin)
{
    timer = tim;
    //timChannel = channel;
    dirPortName = dirPort;
    dirPinNumber = dirPin;
    enablePortName = enPort;
    enablePinNumber = enPin;
    enableValve();
    HAL_TIM_OnePulse_Start_IT(tim, (uint32_t)(channel));
}

/**
* @brief    DValve class destructor
* @param    void
* @retval   void
*/
DValve::~DValve()
{

}

/**
* @brief    Enables the valve operation by setting the valve driver pin to high
* @param    void
* @retval   void
*/
void DValve::enableValve(void)
{
    HAL_GPIO_WritePin(enablePortName, enablePinNumber, GPIO_PIN_RESET);
}

/**
* @brief    Disables the valve operation by setting the valve driver pin to high
* @param    void
* @retval   void
*/
void DValve::disableValve(void)
{
    HAL_GPIO_WritePin(enablePortName, enablePinNumber, GPIO_PIN_SET);
}

/**
* @brief    Trigger valve function
* @param    void
* @retval   void
*/
void DValve::triggerValve(eValveState_t valveState)
{
    if((eValveState_t)VALVE_STATE_ON == valveState)
    {
        HAL_GPIO_WritePin(dirPortName, dirPinNumber, GPIO_PIN_RESET);
        currentValveState = (eValveState_t)VALVE_STATE_ON;
    }

    else if((eValveState_t)VALVE_STATE_OFF == valveState)
    {
        HAL_GPIO_WritePin(dirPortName, dirPinNumber, GPIO_PIN_SET);
        currentValveState = (eValveState_t)VALVE_STATE_OFF;
    }

    else
    {
        //NOP
    }
}

/**
* @brief    Run tests for valves
* @param    void
* @retval   void
*/
void DValve::valveTest(eValveFunctions_t valFunction)
{
    switch(valFunction)
    {

    case(E_VALVE_FUNCTION_REVERSE):
        HAL_GPIO_WritePin(dirPortName, dirPinNumber, GPIO_PIN_RESET);
        __HAL_TIM_ENABLE(timer);
        break;

    case(E_VALVE_FUNCTION_FORWARD):
        HAL_GPIO_WritePin(dirPortName, dirPinNumber, GPIO_PIN_SET);
        __HAL_TIM_ENABLE(timer);
        break;

    default:
        break;
    }
}

/**
* @brief    Run tests for valves
* @param    void
* @retval   void
*/
void DValve::setValveTimer(uint32_t valveTime)
{
    timer->Instance->ARR = valveTime;
}


