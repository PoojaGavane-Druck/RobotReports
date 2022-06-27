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
#define VALVE_EN_GPIO_STATE GPIO_PIN_RESET
#define VALVE_DIS_GPIO_STATE GPIO_PIN_SET
/* Types --------------------------------------------------------------------*/

/* Global Variables ---------------------------------------------------------*/

/* File Statics -------------------------------------------------------------*/
extern TIM_HandleTypeDef htim3;
/* User Code ----------------------------------------------------------------*/

/**
* @brief    DValve class constructor
* @param    void
* @retval   void
*/
DValve::DValve(TIM_HandleTypeDef *tim,
               TIM_TypeDef *timInstance,
               uint16_t channel,
               GPIO_TypeDef *dirPort,
               uint16_t dirPin,
               GPIO_TypeDef *enPort,
               uint16_t enPin,
               uint32_t isNonLatching)
{
    timer = tim;
    timerInstance = timInstance;
    timChannel = channel;
    dirPortName = dirPort;
    dirPinNumber = dirPin;
    enablePortName = enPort;
    enablePinNumber = enPin;
    isValveNonLatching = isNonLatching;
    currentValveState = (eValveState_t)VALVE_STATE_OFF;

    calcOnePulseParameters();

    calcPwmParameters();

    enableValve();

    valveMode = E_VALVE_MODE_SINGLE_SHOT;
    HAL_TIM_OnePulse_Start(tim, (uint32_t)(channel));
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
* @brief    Calculates one Pulse mode parameters
* @param    void
* @retval   void
*/
void DValve::calcOnePulseParameters(void)
{
    pulsePrescaler = 47u;   // fixed
    pulseArr = 2000u;
    pulseCcr = 1000u;
}

/**
* @brief    calculates PWM parameters
* @param    void
* @retval   void
*/
void DValve::calcPwmParameters(void)
{
    pwmPrescaler = 0u;
    pwmArr = 1920u;
    pwmCcr = 384u;
}

/**
* @brief    Enables the valve operation by setting the valve driver pin to high
* @param    void
* @retval   void
*/
void DValve::enableValve(void)
{
    HAL_GPIO_WritePin(enablePortName, enablePinNumber, VALVE_EN_GPIO_STATE);
}

/**
* @brief    Disables the valve operation by setting the valve driver pin to high
* @param    void
* @retval   void
*/
void DValve::disableValve(void)
{
    HAL_GPIO_WritePin(enablePortName, enablePinNumber, VALVE_DIS_GPIO_STATE);
}

/**
* @brief    Trigger valve function
* @param    void
* @retval   void
*/
void DValve::triggerValve(eValveState_t valveState)
{

    if(E_VALVE_MODE_SINGLE_SHOT == valveMode)
    {
        /* In single shot mode, repeated request of valve on can be accepted */
        if((eValveState_t)VALVE_STATE_ON == valveState)
        {
            HAL_GPIO_WritePin(dirPortName, dirPinNumber, GPIO_PIN_SET);
            __HAL_TIM_ENABLE(timer);
        }

        if((eValveState_t)VALVE_STATE_OFF == valveState)
        {
            HAL_GPIO_WritePin(dirPortName, dirPinNumber, GPIO_PIN_RESET);

            // If valve is latching, then do not enable timer to pulse it
            if(isValveNonLatching == 0u)
            {
                __HAL_TIM_ENABLE(timer);
            }
        }

        currentValveState = (eValveState_t)VALVE_STATE_OFF;
    }

    else
    {
        if((eValveState_t)VALVE_STATE_ON == valveState)
        {
            HAL_TIM_PWM_Start(timer, timChannel);
            currentValveState = (eValveState_t)VALVE_STATE_ON;
        }

        if((eValveState_t)VALVE_STATE_OFF == valveState)
        {
            HAL_TIM_PWM_Stop(timer, timChannel);
            currentValveState = (eValveState_t)VALVE_STATE_OFF;
        }
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
void DValve::setValveTime(uint32_t valveTime)
{
    if(E_VALVE_MODE_SINGLE_SHOT == valveMode)
    {
        timer->Instance->CCR1 = valveTime - 1u;
        timer->Instance->ARR = (valveTime << 1) - 1u;
        timer->Instance->EGR = 1u; // Write the update event bit
        HAL_TIM_OnePulse_Start(timer, timChannel);
    }

    else
    {
        setValveDutyCycle(valveTime);
    }

}

/**
* @brief    Enables a PWM based valve
* @param    void
* @retval   void
*/
void DValve::triggerValvePwm(eValveState_t valveState)
{
    if((eValveState_t)VALVE_STATE_ON == valveState)
    {
        HAL_TIM_PWM_Start(timer, timChannel);
    }

    else if((eValveState_t)VALVE_STATE_OFF == valveState)
    {
        HAL_TIM_PWM_Stop(timer, timChannel);
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
void DValve::setValveDutyCycle(uint32_t dutyCycle)
{
    uint32_t pulseWidth = 0u;
    uint32_t maxPulseWidth = 0u;

    float pcDuty = 0.0f;

    if((dutyCycle >= 200u) && (dutyCycle <= 10000u))
    {
        //pcDuty = ((float)(dutyCycle) * (0.0182f)) - 9.0909f;
        pcDuty = (float32_t)(dutyCycle) * 0.1f;
    }

    else
    {
        if(dutyCycle == 0u)
        {
            pcDuty = 0.0f;
        }
    }

    maxPulseWidth = timer->Instance->ARR;
    pulseWidth = ((uint32_t)(pcDuty) * maxPulseWidth) / 100u;

    if(TIM_CHANNEL_1 == timChannel)
    {
        timer->Instance->CCR1 = pulseWidth;
    }

    timer->Instance->EGR = 1u; // Write the update event bit
}


/**
* @brief    Re configures timer handling the valve between a PWM / one pulse timer
            This is required for non latching valves where a TDM based function could be required for handling
            fast venting and and open vent may require a minimal pwm for lowering power consumption
* @param    config - 1 - PWM, 0 - One Pulse mode
* @retval   void
*/
void DValve::reConfigValve(uint32_t config)
{
    /* This function is only available if valve is of pwm type */

    if(E_VALVE_MODE_PWM == valveMode)
    {
        // Only configure in one pulse mode if valve mode is already PWM
        if(0u == config)
        {
            HAL_TIM_PWM_Stop(timer, timChannel);
            resetTimerRegisters();
            configTimerOnePulseMode(pulseArr, pulseCcr);
            HAL_TIM_OnePulse_Start(timer, timChannel);
            valveMode = E_VALVE_MODE_SINGLE_SHOT;
        }
    }

    if(E_VALVE_MODE_SINGLE_SHOT == valveMode)
    {
        // Only configure in PWM if valve mode is already PWM
        if(1u == config)
        {
            HAL_TIM_OnePulse_Stop(timer, TIM_CHANNEL_1);
            resetTimerRegisters();
            configTimerPwmMode(pwmArr, pwmCcr);
            //HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
            valveMode = E_VALVE_MODE_PWM;
        }
    }
}

/**
* @brief    Resets all timer registers to reset value - 0
* @param    void
* @retval   void
*/
void DValve::resetTimerRegisters(void)
{
    timer->Instance->CR1 = 0u;
    timer->Instance->CR2 = 0u;
    timer->Instance->SMCR = 0u;
    timer->Instance->DIER = 0u;
    timer->Instance->SR = 0u;
    timer->Instance->EGR = 0u;
    timer->Instance->CCMR1 = 0u;
    timer->Instance->CCMR2 = 0u;
    timer->Instance->CCER = 0u;
    timer->Instance->CNT = 0u;
    timer->Instance->PSC = 0u;
    timer->Instance->ARR = 0u;
    timer->Instance->RCR = 0u;
    timer->Instance->CCR1 = 0u;
    timer->Instance->CCR2 = 0u;
    timer->Instance->CCR3 = 0u;
    timer->Instance->CCR4 = 0u;
    timer->Instance->BDTR = 0u;
    timer->Instance->DCR = 0u;
    timer->Instance->DMAR = 0u;
    timer->Instance->OR1 = 0u;
    timer->Instance->CCMR3 = 0u;
    timer->Instance->CCR5 = 0u;
    timer->Instance->CCR6 = 0u;
    timer->Instance->OR2 = 0u;
    timer->Instance->OR3 = 0u;
}

/**
* @brief    Configures the timer to run in PWM mode
* @param    void
* @retval   void
*/
void DValve::configTimerPwmMode(uint32_t arr, uint32_t ccr)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    MISRAC_DISABLE  // Disabling misra warning here as code has been taken from STM32 HAL Libraries

    timer->Instance = timerInstance;
    timer->Init.Prescaler = pwmPrescaler;
    timer->Init.CounterMode = TIM_COUNTERMODE_UP;
    timer->Init.Period = arr;
    timer->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timer->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if(HAL_TIM_Base_Init(timer) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

    if(HAL_TIM_ConfigClockSource(timer, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if(HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if(HAL_TIMEx_MasterConfigSynchronization(timer, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = ccr;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if(HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_TIM_MspPostInit(timer);
    MISRAC_ENABLE   // Enable MISRA
}

/**
* @brief    Configures the timer to run in PWM mode
* @param    void
* @retval   void
*/
void DValve::configTimerOnePulseMode(uint32_t arr, uint32_t ccr)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    timer->Instance = timerInstance;
    timer->Init.Prescaler = pulsePrescaler;
    timer->Init.CounterMode = TIM_COUNTERMODE_UP;
    timer->Init.Period = arr;
    timer->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timer->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    MISRAC_DISABLE  // Disabling misra warning here as code has been taken from STM32 HAL Libraries

    if(HAL_TIM_Base_Init(timer) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

    if(HAL_TIM_ConfigClockSource(timer, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if(HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }

    if(HAL_TIM_OnePulse_Init(timer, TIM_OPMODE_SINGLE) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if(HAL_TIMEx_MasterConfigSynchronization(timer, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = ccr;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if(HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_TIM_MspPostInit(timer);

    MISRAC_ENABLE   // Enable MISRA
}





