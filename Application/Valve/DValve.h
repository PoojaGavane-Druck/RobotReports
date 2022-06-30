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
* @brief    Header File for Valve Class
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
    VALVE_STATE_ON = 0,
    VALVE_STATE_OFF
} eValveState_t;

typedef enum : uint32_t
{
    E_VALVE_MODE_PWM = 0u,
    E_VALVE_MODE_SINGLE_SHOT
} eValveModes_t;

/* Variables ----------------------------------------------------------------*/

class DValve
{
private:
    TIM_HandleTypeDef *timer;
    uint32_t timChannel;

    GPIO_TypeDef *dirPortName ;
    uint16_t dirPinNumber ;

    GPIO_TypeDef *enablePortName ;
    uint16_t enablePinNumber;

    TIM_TypeDef *timerInstance;
    uint32_t isValveNonLatching;
    eValveState_t currentValveState;

    uint32_t pulsePrescaler;
    uint32_t pulseArr;
    uint32_t pulseCcr;
    uint32_t pwmPrescaler;
    uint32_t pwmDuty;
    uint32_t pwmArr;
    uint32_t pwmCcr;

    eValveModes_t valveMode;


    void configTimerOnePulseMode(uint32_t arr, uint32_t ccr);
    void configTimerPwmMode(uint32_t arr, uint32_t ccr);
    void resetTimerRegisters(void);
    void calcOnePulseParameters(void);
    void calcPwmParameters(void);

public:
    DValve(TIM_HandleTypeDef *tim,
           TIM_TypeDef *timInstance,
           uint16_t channel,
           GPIO_TypeDef *dirPort,
           uint16_t dirPin,
           GPIO_TypeDef *enPort,
           uint16_t enPin,
           uint32_t isNonLatching);
    ~DValve();
    void enableValve(void);
    void disableValve(void);
    void triggerValve(eValveState_t valveState);
    void valveTest(eValveState_t valFunction);
    void setValveTime(uint32_t valveTime);
    void setValveDutyCycle(uint32_t dutyCycle);
    void triggerValvePwm(eValveState_t valveState);
    void reConfigValve(uint32_t config);
};

#endif /* DValve.h*/
