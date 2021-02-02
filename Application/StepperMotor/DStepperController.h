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
* @file     DSlot.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlot base class header file
*/

#ifndef _DSTEPPER_CONTROLLER_H
#define _DSTEPPER_CONTROLLER_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
MISRAC_ENABLE
#include "Types.h"
#include "DStepperMotor.h"
#include "DTask.h"


/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum 
{
    eAppStateNone = 0,
    eAppStateStartup,
    eAppStateIdle,
    eAppStateMotorAccelerate,
    eAppStateMotorControl,
    eAppStateMotorDecelerate,
    eAppStateMotorStop
}eApplicationStates;
/* Variables --------------------------------------------------------------------------------------------------------*/

class DStepperController : public DTask
{
    DStepperMotor *stepperMotor;
    uint32_t updatedDirection;
    float32_t acclAlpha;
    float32_t acclBeta;  
    float32_t decelAlpha;
    float32_t decelBeta;

    volatile uint32_t pulseCounter;
    
     uint32_t stateAcclCurrent;
     uint32_t stateDecelCurrent;
     uint32_t stateIdleCurrent;
     eApplicationStates state;
protected:
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond
    OS_MUTEX myMutex;

    

public:
    DStepperController(TIM_HandleTypeDef* hPulseCnt,
                       uint32_t timerChForCnt,
                       TIM_HandleTypeDef* hPwmTmr,
                       uint32_t timerChForPwm,
                       float32_t motorClkFrq);
    virtual void initialise(void);
    virtual void runFunction(void);
    virtual void cleanUp(void);
   
    void updateDirection(void);
    void setDirection(void);
    eApplicationStates stepsNoneMotorStopped(void);
    eApplicationStates stepsOneMotorStopped(void);
    eApplicationStates stepsUpdatedMotorStopped(void);
    eApplicationStates stepsOneMotorRunning(void);

    eApplicationStates stepsNoneMotorRunning(void);
    eApplicationStates stepsUpdatedMotorRunning(void);
    eApplicationStates decelRequired(void);
    eApplicationStates directionChanged(void);
    eApplicationStates decelComplete(void);
    eApplicationStates motorMoveComplete(void);
    void checkOverrunSteps(int32_t stepsCount1, int32_t stepsCount2);
    void setCurrent(eApplicationStates newState);
    void startAdc(uint32_t *counts);
    bool getValue(eValueIndex_t index, float32_t *value);    //get specified floating point function value    
    bool getValue(eValueIndex_t index, uint32_t *value);    //get specified integer function value
    friend void pulseFinishedCallback(DStepperController *hStepperController);
    friend void delayElapsedCallback(DStepperController *hStepperController);
};

#endif // _DSLOT_H
