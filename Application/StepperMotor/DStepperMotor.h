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

#ifndef _DSTEPPER_MOTOR_H
#define _DSTEPPER_MOTOR_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stdint.h>
MISRAC_ENABLE
#include "Types.h"



/* Defines ----------------------------------------------------------------------------------------------------------*/

#define MOTOR_DEFAULT_MIN_SPEED 1000u
#define MOTOR_DEFAULT_MAX_SPEED 7000u
#define MOTOR_DEFAULT_ACCL_ALPHA 0.98f
#define MOTOR_DEFAULT_ACCL_BETA 2.86f
#define MOTOR_DEFAULT_DECEL_ALPHA 0.98f
#define MOTOR_DEFAULT_DECEL_BETA 0.0f
#define STEPS_BUFFER 3u
#define MOTOR_DEFAULT_DECEL_FACTOR 1.02f
#define MAX_CURRENT 2000.0f
#define CURR_RESOLUTION 31.25f
#define MICROS_FACTOR 12u
#define BASE_FREQ_ARR 11999u
#define BASE_FREQ_CCR 5999u

#ifdef DEBUG
     #define LENGTH 4000u
#endif
/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum 
{
    eMtrDirectionBackwards = 0,
    eMtrDirectionForwards
}MtrDirection_t;

typedef struct 
{
    float acclAlpha;
    float acclBeta;
    float decclAlpha;
    float decclBeta;
    uint32_t motorRunning;
    uint32_t steps;
    uint32_t overrunSteps;
    uint32_t decelSteps;
    float decelFactor;
    volatile uint32_t motorMoveCommand;
    volatile uint32_t motorStopFlag;
    volatile uint32_t motorMoveComplete;
    volatile uint32_t motorAccelerate;
    volatile uint32_t motorDecelerate;
    uint32_t minimumSpeed;
    uint32_t maximumSpeed;
    uint32_t decelComplete;
    uint32_t accelComplete;
    volatile uint32_t currentDirection;
    volatile uint32_t previousDirection;
    volatile uint32_t directionChanged;
    int32_t absoluteStepCounter;
    int32_t previousSteps;
    int32_t stepsBeforeDecel;
    int32_t stepsAfterDecel;
    int32_t stepsCounter;
    
    uint32_t motorSpeed;
    uint32_t motorCurrent;
    uint32_t opticalSensorVal;

    float32_t currHold;
    float32_t currRun;
    float32_t currAccl;
    float32_t currDecel;

}sMotorParms_t;

typedef enum 
{
    eCurrNone = 0x00,
    eCurrHold,
    eCurrRun,
    eCurrAccl,
    eCurrDecel
}MotorCurrentTypedef_t;
/* Variables --------------------------------------------------------------------------------------------------------*/

class DStepperMotor 
{
    TIM_HandleTypeDef* hPulseCounter;
    TIM_HandleTypeDef* hPwmTimer;
    uint32_t timerChForPulseCounter;
    uint32_t timerChannelForPwm;
    sMotorParms_t motorParms;
    float32_t motorClockFreq;
    
#ifdef DEBUG
     
     float speed[LENGTH];
     uint32_t counter;
#endif


   
public:
    DStepperMotor(TIM_HandleTypeDef* hPulseCnt,
                  uint32_t timerChForCnt,
                  TIM_HandleTypeDef* hPwmTmr,
                  uint32_t timerChForPwm,
                  float32_t motorClkFrq);
    void initializeMotorParams(void);
    void stop(void);
    void run(void);
    void squareWaveFreqReset(void);
    void stopCapture(void);
    void startCapture(void);
    
    
   
    
    void accelerate(float alpha, float beta);
    float calculateTimeRemaining(uint32_t steps);
    void decelerate(float alpha, float beta);
    void updateSteps(uint32_t steps);
    void resetCapture(void);
    uint32_t getCurrentStep(void);
    void startWatchdog(void);
    //void stopWatchdog(void);
    //void watchdogReset(void);
    void initController(void);
    void controllerResetHW(void);
    void controllerResetSW(void);
    uint16_t controllerReadStatus(void);
    void writeAccelerationTime(uint32_t time);
    void updateAbsoluteStepCounter(void);
    uint32_t checkDecelerationRequired(uint32_t totalSteps, 
                                             uint32_t minSpeed,
                                             float decelFactor,
                                             float accAlpha,
                                             float accBeta);
    uint32_t checkDecelerationRequired(void);
    uint32_t calculateDecelSteps(uint32_t minSpeed, 
                                       float decelFactor, 
                                       float accAlpha, 
                                       float accBeta);
    uint32_t calculateDecelSteps(void);
    void updateDirection(uint32_t direction);
    void run_IT(void);
    void setDirection(MtrDirection_t direction);
    MtrDirection_t getDirection(void);
    int32_t getNumberOfStepsTakenByMotor(void);
    void writeCurrent(MotorCurrentTypedef_t currentType);
    
    uint32_t getMotorMoveCompletionStatus(void);
    void setMotorMoveCompletionStatus(uint32_t moveStatus);
    
    uint32_t getMotorRunningStatus(void);    
    void setMotorRunningStatus(uint32_t runStatus);
    
    uint32_t getDecelComplettionStatus(void);
    void setDecelComplettionStatus(uint32_t newStatus);
   
    uint32_t getAecelComplettionStatus(void);
    void setAecelComplettionStatus(uint32_t newStatus);
   
    float32_t getAccelerationAlpha(void);
    void setAccelerationAlpha(float32_t newAlpha);
   
    float32_t getAccelerationBeta(void);
    void setAccelerationBeta(float32_t newBeta);
   
    float32_t getDecelerationAlpha(void);
    void setDecelerationAlpha(float32_t newAlpha);
   
    float32_t getDecelerationBeta(void); 
    void setDecelerationBeta(float32_t newBeta);
    
    uint32_t getNumberOfsteps(void);
    void setNumberOfsteps(uint32_t newNumberOfSteps);
    
    uint32_t getOverrunStepsCount(void);
    void setOverrunStepsCount(uint32_t newOverrunStepsCount);

    
    uint32_t getDecelerationStepsCount(void);
    void setDecelerationStepsCount(uint32_t newDecelerationSteps);
    
    float32_t getDecelerationFactor(void);
    void setDecelerationFactor(float32_t newFactor);
    
    int32_t getStepsCounter(void);
    void setStepsCounter(int32_t newStepsCnt);
    
    int32_t getAbsoluteStepCount(void);
    void setAbsoluteStepCount(int32_t newStepCount);
    
    uint32_t getDirectionChangedStatus(void);
    void setDirectionChangedStatus(uint32_t newDirChangeStatus);
    
    uint32_t getMotorMoveCommandStatus();
    void setMotorMoveCommandStatus(uint32_t flagStatus);
    
    uint32_t getSteps(void);
    void setSteps(uint32_t newStepsCount);
    
    void incrementStepCounter(void);
    void decrementStepCounter(void);
    
    
    uint32_t getMotorAccelerateStatus(void);
    void setMotorAccelerateStatus(uint32_t newStatus);
    
    uint32_t getMotorDecelerateStatus(void);
    void setMotorDecelerateStatus(uint32_t newStatus);
    
    uint32_t getMotorStopFlagStatus(void);
    void setMotorStopFlagStatus(uint32_t flagStatus);
    
    uint32_t getMotorSpeedStatus(void);
    void setMotorSpeedStatus(uint32_t newStatus);
    
    int32_t getNumOfStepsBeforeDecel(void);
    void setNumOfStepsBeforeDecel(int32_t newStepsCount);
    
    int32_t getNumOfStepsAfterDecel(void);
    void setNumOfStepsAfterDecel(int32_t newStepsCount);
    
    void enablePulseCounterTimerInterrupt(void);
};

#endif // _DSLOT_H
