/**
* BHGE Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DSlot.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlot class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <os.h>
#include "dSpin.h"
MISRAC_ENABLE
#include "DPowerManager.h"
#include "DErrorhandler.h"
#include "DStepperController.h"
#include "memory.h"
#include "DLock.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlot class constructor
 * @note    An owner is needed if sensor events are to be sent back. This is expected to be a DFunction instance.
 * @param   owner: the task that created this slot
 * @retval  void
 */
DStepperController::DStepperController(TIM_HandleTypeDef* hPulseCnt,
                       uint32_t timerChForCnt,
                       TIM_HandleTypeDef* hPwmTmr,
                       uint32_t timerChForPwm,
                       float32_t motorClkFrq)
: DTask()
{
    OS_ERR osError;


    //create mutex for resource locking
    char *name = "StepperMotor";
    
    stepperMotor = new DStepperMotor(hPulseCnt,
                                     timerChForCnt,
                                     hPwmTmr,
                                     timerChForPwm,
                                     motorClkFrq);
    
    acclAlpha = (float)(0);
    acclBeta = (float)(0);
    decelAlpha = (float)(0);
    decelBeta = (float)(0);

    
    
    stateAcclCurrent = (uint32_t)(0);
    stateDecelCurrent = (uint32_t)(0);
    stateIdleCurrent = (uint32_t)(0);
    
    updatedDirection = (uint32_t)(0);
    
    //specify the flags that this function must respond to (add more as necessary in derived class)
    
    myWaitFlags = EV_FLAG_TASK_SHUTDOWN;
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &osError);

    if (osError != (OS_ERR)OS_ERR_NONE)
    {
        //Error handler?        
    }
    
     //get stack area from the memory partition memory block for function tasks
    myTaskStack = (CPU_STK*)OSMemGet((OS_MEM*)&memPartition, (OS_ERR*)&osError);   
    
    if (osError == (OS_ERR)OS_ERR_NONE)
    {
        
        //memory block from the partition obtained, so can go ahead and run
        //activate(myName, (CPU_STK_SIZE)MEM_PARTITION_BLK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)10u, &osError);
        
    }
    else
    {
        //report error
    }
    
 
}


void DStepperController::initialise(void)
{
 
  state = eAppStateStartup;
}

/**
 * @brief   Run DBattery task funtion
 * @param   void
 * @retval  void
 */
void DStepperController::runFunction(void)
{
    bool runFlag = true;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
    uint32_t isDecelRequired = (uint32_t)(0);
    while (runFlag == true)
    {
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)500u, //runs, nominally, at 2Hz by default
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
        //check for events
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
          switch(state)
          {
              case eAppStateNone:
              break;

              case eAppStateStartup:
                  state = eAppStateIdle;
                  //__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC1);
                  /* This enables the 24 volts supply */
                  HAL_GPIO_WritePin(P24V_EN_PA7_GPIO_Port, P24V_EN_PA7_Pin, GPIO_PIN_SET);
                  HAL_GPIO_WritePin(TEMP_ALERT_PG6_GPIO_Port, TEMP_ALERT_PG6_Pin, GPIO_PIN_SET);
                  
                  stepperMotor->enablePulseCounterTimerInterrupt();
                  /* For test only */
                  //stepperMotor->setSteps((uint32_t)(1000));              
                  //stepperMotor->setDirection(eMtrDirectionForwards);
              break;

              case eAppStateIdle:
                 setCurrent(eAppStateIdle);  

                 // sApplication.sMotorParms.motorSpeed = (uint32_t)(0);
                  stepperMotor->setMotorSpeedStatus((uint32_t)(0));
                  //if(sApplication.sMotorParms.directionChanged == (uint32_t)(1))
                  if((uint32_t)(1) == stepperMotor->getDirectionChangedStatus())  
                  {
                      updateDirection();             
                  }              
                
                  //if((uint32_t)(1) == sApplication.sMotorParms.motorMoveCommand)
                  if((uint32_t)(1) == stepperMotor->getMotorMoveCommandStatus())
                  {
                      setDirection();
                      //sApplication.sMotorParms.motorMoveCommand = (uint32_t)(0);
                      stepperMotor->setMotorMoveCommandStatus((uint32_t)(0));
                      //if((uint32_t)(0) == sApplication.sMotorParms.steps)
                      if((uint32_t)(0) == stepperMotor->getSteps())
                      {
                          state = stepsNoneMotorStopped();
                      }
                      //else if((uint32_t)(1) == sApplication.sMotorParms.steps)
                      else if((uint32_t)(1) == stepperMotor->getSteps())
                      {
                          state = stepsOneMotorStopped();
                      }
                      else
                      {
                          state = stepsUpdatedMotorStopped();
                      }
                  }
              break;

              case eAppStateMotorAccelerate:
                  setCurrent(eAppStateMotorAccelerate);  
#if 0
                  isDecelRequired = stepperMotor->checkDecelerationRequired(sApplication.sMotorParms.steps,
                                                          sApplication.sMotorParms.minimumSpeed,
                                                          sApplication.sMotorParms.decelFactor,
                                                          sApplication.sMotorParms.acclAlpha,
                                                          sApplication.sMotorParms.acclBeta);  
#endif
                  isDecelRequired = stepperMotor->checkDecelerationRequired();  
                  if((uint32_t)(1) == isDecelRequired)
                  {
                      /* Read the steps completed till now */
                      //stepsBeforeDecel = Motor_GetCurrentStep();
                      state = decelRequired();
                  }  
                  else
                  {
                      //if((uint32_t)(1) == sApplication.sMotorParms.motorAccelerate)
                      if((uint32_t)(1) == stepperMotor->getMotorAccelerateStatus())
                      {
                          //sApplication.sMotorParms.motorAccelerate = (uint32_t)(0);
                          stepperMotor->setMotorAccelerateStatus((uint32_t)(0));
                          stepperMotor->accelerate(acclAlpha, acclBeta);
                      }            
                          
#if 0
                      if((uint32_t)(1) == sApplication.sWatchdog.watchdogFlag)
                      {
                          /* Read the steps completed till now */
                          //sApplication.sMotorParms.stepsBeforeDecel = Motor_GetCurrentStep();                
                          stepperMotor->setNumOfStepsBeforeDecel(stepperMotor->getCurrentStep());
                          /* Reset all timers */
                          state = watchdogTimedOut();
                      } 
#endif
                      //else if((uint32_t)(1) == sApplication.sMotorParms.directionChanged)
                      if((uint32_t)(1) == stepperMotor->getDirectionChangedStatus())
                      {
                          /* Read the steps completed till now */
                          //stepsBeforeDecel = Motor_GetCurrentStep();     
                          /* Run the direction changed routine */          
                          state = directionChanged();
                      }      
                      //else if((uint32_t)(1) == sApplication.sMotorParms.motorMoveCommand)
                      else if((uint32_t)(1) == stepperMotor->getMotorMoveCommandStatus())
                      {
                          //sApplication.sMotorParms.motorMoveCommand = (uint32_t)(0);
                          stepperMotor->setMotorMoveCommandStatus((uint32_t)(0));
                          //if((uint32_t)(0) == sApplication.sMotorParms.steps)
                          if((uint32_t)(0) ==  stepperMotor->getSteps())
                          {
                              state = stepsNoneMotorRunning();                   
                          }
                          else
                          {
                              state = stepsUpdatedMotorRunning();
                          }
                      }
                      else
                      {
                        /* Do Nothing */
                      }
                  }

                                                                
              break;
              
              case eAppStateMotorDecelerate:          
                  setCurrent(eAppStateMotorDecelerate);         

                  //if((uint32_t)(1) == sApplication.sMotorParms.motorDecelerate)
                  if((uint32_t)(1) == stepperMotor->getMotorDecelerateStatus())
                  {
                      //sApplication.sMotorParms.motorDecelerate = (uint32_t)(0);
                      stepperMotor->setMotorDecelerateStatus((uint32_t)(0));
                      stepperMotor->decelerate(decelAlpha, decelBeta);  
                  }           
                  
                  //if(sApplication.sMotorParms.decelComplete == (uint32_t)(1))
                  if((uint32_t)(1) == stepperMotor->getDecelComplettionStatus())
                  {          
                      //sApplication.sMotorParms.motorSpeed = (uint32_t)(0);
                       stepperMotor->setMotorSpeedStatus((uint32_t)(0));
                      //if ((uint32_t)(1) == sApplication.sMotorParms.motorStopFlag)
                      if ((uint32_t)(1) == stepperMotor->getMotorStopFlagStatus())
                      {
                        
                          /* Read the steps after decel completed */
                         // sApplication.sMotorParms.stepsAfterDecel = (int32_t)(stepperMotor->getCurrentStep());
                          stepperMotor->setNumOfStepsAfterDecel((static_cast<int32_t>(stepperMotor->getCurrentStep())));
                          //checkOverrunSteps(sApplication.sMotorParms.stepsAfterDecel, sApplication.sMotorParms.stepsBeforeDecel);
                          checkOverrunSteps(stepperMotor->getNumOfStepsAfterDecel(), 
                                            stepperMotor->getNumOfStepsBeforeDecel());
                          state = decelComplete();
                          //sApplication.sMotorParms.motorStopFlag = (uint32_t)(0);       
                          stepperMotor->setMotorStopFlagStatus((uint32_t)(0));
                      }
                      //else if((uint32_t)(1) == sApplication.sMotorParms.directionChanged)
                      else if((uint32_t)(1) == stepperMotor->getDirectionChangedStatus())
                      {
                          /* Read the steps after decel completed */
                          //sApplication.sMotorParms.stepsAfterDecel = (sint32_t)(stepperMotor->getCurrentStep());                    
                          stepperMotor->setNumOfStepsAfterDecel((static_cast<int32_t>(stepperMotor->getCurrentStep())));
                          //checkOverrunSteps(sApplication.sMotorParms.stepsAfterDecel, sApplication.sMotorParms.stepsBeforeDecel);
                          checkOverrunSteps(stepperMotor->getNumOfStepsAfterDecel(), 
                                            stepperMotor->getNumOfStepsBeforeDecel());
                          state = decelComplete();
                          //sApplication.sMotorParms.motorMoveCommand = (uint32_t)(1);
                          stepperMotor->setMotorMoveCommandStatus((uint32_t)(1));
                      }
#if 0
                      else if((uint32_t)(1) == sApplication.sWatchdog.watchdogFlag)
                      {
                          // Read the steps after decel completed 
                          sApplication.sMotorParms.stepsAfterDecel = (sint32_t)(Motor_GetCurrentStep());                    
                          Application_CheckOverrunSteps(sApplication.sMotorParms.stepsAfterDecel, sApplication.sMotorParms.stepsBeforeDecel);
                          sApplication.sWatchdog.watchdogFlag = (uint32_t)(0);
                          state = decelComplete();
      ;
                      }
#endif
                      //else if((uint32_t)(1) == sApplication.sMotorParms.motorMoveComplete)
                      else if((uint32_t)(1) == stepperMotor->getMotorMoveCompletionStatus())
                      {
                          state = motorMoveComplete();
                      }
                      //else if((uint32_t)(1) == sApplication.sMotorParms.motorMoveCommand)
                      else if((uint32_t)(1) == stepperMotor->getMotorMoveCommandStatus())
                      {
                          //sApplication.sMotorParms.motorMoveCommand = (uint32_t)(0);
                          stepperMotor->setMotorMoveCommandStatus((uint32_t)(0));
                          //if((uint32_t)(0) == sApplication.sMotorParms.steps)
                          if((uint32_t)(0) == stepperMotor->getSteps())
                          {
                              state = stepsNoneMotorRunning();                   
                          }
                          else
                          {
                              state = stepsUpdatedMotorRunning();
                          }
                      }
                      else
                      {
                        /* Do Nothing */
                      }
                  }          
                  //else if((uint32_t)(0) == sApplication.sMotorParms.directionChanged)
                  else if((uint32_t)(0) == stepperMotor->getDirectionChangedStatus())
                  {
                      //if((uint32_t)(1) == sApplication.sMotorParms.motorMoveCommand)
                      if((uint32_t)(1) == stepperMotor->getMotorMoveCommandStatus())
                      {
                          //sApplication.sMotorParms.motorMoveCommand = (uint32_t)(0);
                          stepperMotor->setMotorMoveCommandStatus((uint32_t)(0));
                          //if((uint32_t)(0) == sApplication.sMotorParms.steps)
                          if((uint32_t)(0) == stepperMotor->getSteps())
                          {
                              state = stepsNoneMotorRunning();                   
                          }
                          else
                          {
                              state = stepsUpdatedMotorRunning();
                          }
                      }  
                  }
                  else
                  {
                    /* Do nothing */
                  }
                  break;   
          default:
            break;
          }
        }
    }
    
}


/**
 * @brief   Clean up after termination of task
 * @param   void
 * @return  void
 */
void DStepperController::cleanUp(void)
{
    OS_ERR err;

    //if a stack was allocated then free that memory to the partition
    if (myTaskStack != NULL)
    {
        //Return the stack memory block back to the partition
        OSMemPut((OS_MEM*)&memPartition, (void*)myTaskStack, (OS_ERR*)&err);

        if (err == (OS_ERR)OS_ERR_NONE)
        {
            //memory block from the partition obtained
        }

        myTaskStack = NULL;
    }
}

bool DStepperController::getValue(eValueIndex_t index, float32_t *value)   //get specified floating point function value    
{
  bool successFlag = false;
   switch(index)
  {
  case 1u:
    break;
    default:
      successFlag = false;
    break;
  }
  return successFlag;
}

bool DStepperController::getValue(eValueIndex_t index, uint32_t *value)    //get specified integer function value
{
  bool successFlag = false;
  DLock is_on(&myMutex);
  successFlag = true;
  switch(index)
  {
  case 1u:
    break;
    default:
      successFlag = false;
    break;
  }
    
    
  return successFlag;
}

/**
 * @brief   This function updates the direction of rotation of the motor
 * @param   void
 * @return  void
 */
void DStepperController::updateDirection(void)
{
    //sApplication.sMotorParms.directionChanged = (uint32_t)(0);
    stepperMotor->setDirectionChangedStatus((uint32_t)(0));
    //if(sApplication.sMotorParms.currentDirection == (uint32_t)(1))
    if(stepperMotor->getDirection() == (uint32_t)(1))
    {
        dSPIN_Step_Clock((dSPIN_Direction_TypeDef)(0x58));
        updatedDirection = (uint32_t)(1);
    }
    else
    {
        dSPIN_Step_Clock((dSPIN_Direction_TypeDef)(0x59));
        updatedDirection = (uint32_t)(0);
    } 

#ifdef OLD_CTRL_CHIP
    UUpdateDirection(sApplication.sMotorParms.currentDirection);
#endif  
}

/**
 * @brief   This function updates the direction of rotation of the motor
 * @param   void
 * @return  void
 */
void DStepperController::setDirection(void)
{
    if((uint32_t)(1) == (uint32_t)stepperMotor->getDirection())
    {
        dSPIN_Step_Clock((dSPIN_Direction_TypeDef)(0x58));
        updatedDirection = (uint32_t)(1);
    }
    else
    {
        dSPIN_Step_Clock((dSPIN_Direction_TypeDef)(0x59));
        updatedDirection = (uint32_t)(0);
    }   
}

/**
 * @brief   This function handles the state change if steps are set to 0 
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::stepsNoneMotorStopped(void)
{
    eApplicationStates state = eAppStateNone;

    stepperMotor->setMotorMoveCompletionStatus((uint32_t)(0));
    stepperMotor->setMotorRunningStatus((uint32_t)(0));
    stepperMotor->stop();
    stepperMotor->stopCapture();
    
   
    stepperMotor->resetCapture();
    stepperMotor->squareWaveFreqReset(); 
    state = eAppStateIdle;
                    
    return state;
}

/**
 * @brief   This function handles the state change if steps are updated when motor is stopped 
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::stepsOneMotorStopped(void)
{
    eApplicationStates state = eAppStateNone;
    uint32_t totalSteps = (uint32_t)(0);
    
    acclAlpha = stepperMotor->getAccelerationAlpha();
    acclBeta = stepperMotor->getAccelerationBeta();
    stepperMotor->setMotorRunningStatus((uint32_t)(1));
    stepperMotor->setDecelComplettionStatus((uint32_t)(1));
    stepperMotor->setAecelComplettionStatus( (uint32_t)(1));
    stepperMotor->resetCapture();
    //totalSteps = sApplication.sMotorParms.steps + sApplication.sMotorParms.overrunSteps;
    totalSteps =  (stepperMotor->getSteps()) +  stepperMotor->getOverrunStepsCount();
    
    //sApplication.sMotorParms.overrunSteps = (uint32_t)(0);
    stepperMotor->setOverrunStepsCount((uint32_t)(0));
    stepperMotor->updateSteps(totalSteps);                
    stepperMotor->startCapture();
    stepperMotor->run();
    
 
    state = eAppStateMotorAccelerate;
                    
    return state;
}

/**
 * @brief   This function handles the state change if steps are updated when motor is stopped 
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::stepsUpdatedMotorStopped(void)
{
    eApplicationStates state = eAppStateNone;
    uint32_t totalSteps = (uint32_t)(0);

    //acclAlpha = sApplication.sMotorParms.acclAlpha;
    acclAlpha = stepperMotor->getAccelerationAlpha();
    //acclBeta = sApplication.sMotorParms.acclBeta;
     acclBeta = stepperMotor->getAccelerationBeta();
    //sApplication.sMotorParms.motorRunning = (uint32_t)(1);
    stepperMotor->setMotorRunningStatus((uint32_t)(1));
   // sApplication.sMotorParms.decelComplete = (uint32_t)(0);
    stepperMotor->setDecelComplettionStatus((uint32_t)(0));
    //sApplication.sMotorParms.accelComplete = ((uint32_t)(0));;
    stepperMotor->setAecelComplettionStatus((uint32_t)(0));
    stepperMotor->resetCapture();

    //totalSteps = sApplication.sMotorParms.steps + sApplication.sMotorParms.overrunSteps;
    totalSteps = stepperMotor->getSteps() +  stepperMotor->getOverrunStepsCount();
    
    //sApplication.sMotorParms.overrunSteps = (uint32_t)(0);
    stepperMotor->setOverrunStepsCount((uint32_t)(0));
    stepperMotor->updateSteps(totalSteps);                
    stepperMotor->startCapture();
    stepperMotor->run_IT();
    
    state = eAppStateMotorAccelerate;
                    
    return state;
}

/**
 * @brief   This function handles the state change if steps are set to 0 while motor is running 
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::stepsNoneMotorRunning(void)
{
    eApplicationStates state = eAppStateNone;
    
    //sApplication.sMotorParms.motorMoveCommand = (uint32_t)(0);
    //sApplication.sMotorParms.motorStopFlag = (uint32_t)(1);
    
    
    
    stepperMotor->updateAbsoluteStepCounter();
    //stepperMotor->stopWatchdog();
    //stepperMotor->watchdogReset();
    //decelAlpha = sApplication.sMotorParms.decelFactor;
    stepperMotor->setDecelerationAlpha(stepperMotor->getDecelerationFactor());
    //decelBeta = (float)(0);
    stepperMotor->setDecelerationBeta((float)(0));
    
   
    state = eAppStateMotorDecelerate; 
                    
    return state;  
}

/**
 * @brief   This function handles the state change if steps are set to 0 while motor is running 
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::stepsOneMotorRunning(void)
{
    eApplicationStates state = eAppStateNone;

    //sApplication.sMotorParms.motorMoveCommand = (uint32_t)(0);
    stepperMotor->setMotorMoveCommandStatus((uint32_t)(0));
    stepperMotor->stopCapture();
    stepperMotor->updateAbsoluteStepCounter();
    stepperMotor->resetCapture();
    stepperMotor->updateSteps(stepperMotor->getSteps());
    stepperMotor->startCapture();
    state = eAppStateMotorDecelerate;

    return state;   
}

/**
 * @brief   This function handles the state change if steps are updated while motor is running
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::stepsUpdatedMotorRunning(void)
{
    eApplicationStates state = eAppStateNone;

    //sApplication.sMotorParms.motorMoveCommand = (uint32_t)(0);
    stepperMotor->setMotorMoveCommandStatus((uint32_t)(0));
    stepperMotor->updateAbsoluteStepCounter();
    stepperMotor->updateSteps(stepperMotor->getSteps());

    if((uint32_t)(1) != stepperMotor->getDirectionChangedStatus())
    {
        stepperMotor->stopCapture();
        stepperMotor->resetCapture();
        stepperMotor->startCapture();
    }
    state = eAppStateMotorAccelerate;

    return state;   
}

/**
 * @brief   This function handles the state change after deceleration is required
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::decelRequired(void)
{
    eApplicationStates state = eAppStateNone;
    
    //stepperMotor->stopWatchdog();
    //stepperMotor->watchdogReset();
    //decelAlpha = sApplication.sMotorParms.decelFactor;
    stepperMotor->setDecelerationAlpha(stepperMotor->getDecelerationFactor());
    //decelBeta = (float)(0);
    stepperMotor->setDecelerationBeta((float)(0));
    
    state = eAppStateMotorDecelerate;    
    
    return state;
}



/**
 * @brief   This function handles the state change after direction has changed
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::directionChanged(void)
{
    eApplicationStates state = eAppStateNone;
    
   // stepperMotor->stopWatchdog();
   // stepperMotor->watchdogReset();
    stepperMotor->updateAbsoluteStepCounter();
    //decelAlpha = sApplication.sMotorParms.decelFactor;
    stepperMotor->setDecelerationAlpha(stepperMotor->getDecelerationFactor());
    //decelBeta = (float)(0); //sApplication.sMotorParms.decclBeta;
    stepperMotor->setDecelerationBeta((float)(0));    
    state = eAppStateMotorDecelerate;    
    
    return state;   
}

/**
 * @brief   This function handles the state change after deceleration is complete
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::decelComplete(void)
{
    eApplicationStates state = eAppStateNone;
        
    //sApplication.sMotorParms.decelComplete = (uint32_t)(0);
    stepperMotor->setDecelComplettionStatus((uint32_t)(0));
    //sApplication.sMotorParms.motorRunning = (uint32_t)(0);
    stepperMotor->setMotorRunningStatus((uint32_t)(0));
    stepperMotor->updateAbsoluteStepCounter();
    stepperMotor->stop();
    stepperMotor->stopCapture();
    
    //Motor_StopDecelTimer();                    
    stepperMotor->squareWaveFreqReset();
    state = eAppStateIdle;
    
    return state;
}

/**
 * @brief   This function handles the state change after a motor move is successfully completed
 * @param   void
 * @return  eApplicationStates state
 */
eApplicationStates DStepperController::motorMoveComplete(void)
{
    eApplicationStates state = eAppStateNone;
      
   // sApplication.sMotorParms.motorMoveComplete = (uint32_t)(0);
    stepperMotor->setMotorMoveCompletionStatus((uint32_t)(0));
   // sApplication.sMotorParms.motorRunning = (uint32_t)(0);
    stepperMotor->setMotorRunningStatus((uint32_t)(0));
    stepperMotor->updateAbsoluteStepCounter();
    stepperMotor->stop();
    stepperMotor->stopCapture();
    
    //Motor_StopDecelTimer();                    
    stepperMotor->squareWaveFreqReset();
    state = eAppStateIdle;
    
    return state;
}

/**
 * @brief   This function checks if the motor has overrun extra steps from what was set
 * @param   void
 * @return  void
 */
void DStepperController::checkOverrunSteps(int32_t stepsCount1, 
                                           int32_t stepsCount2)
{
    int32_t difference = (int32_t)(0);

    difference = stepsCount1 - stepsCount2;

    //sApplication.sMotorParms.overrunSteps = (uint32_t)(difference);
    stepperMotor->setOverrunStepsCount((uint32_t)(difference));
}

/**
 * @brief   This function writes the current value to the L6472 based on the state
 * @param   void
 * @return  void
 */
void DStepperController::setCurrent(eApplicationStates newState)
{
    if((eApplicationStates)eAppStateMotorDecelerate == newState)
    {
        if((uint32_t)(0) == stateDecelCurrent)
        {
            stateDecelCurrent = (uint32_t)(1);
            stateAcclCurrent = (uint32_t)(0);
            stateIdleCurrent = (uint32_t)(0);
            stepperMotor->writeCurrent(eCurrDecel);
        }
        
    }
    else if ((eApplicationStates)eAppStateMotorAccelerate == newState)
    {
        if((uint32_t)(0) == stateAcclCurrent)
        {
            stateAcclCurrent = (uint32_t)(1);
            stateIdleCurrent = (uint32_t)(0);
            stateDecelCurrent = (uint32_t)(0);
            stepperMotor->writeCurrent(eCurrAccl);
        }        
    }
    else if((eApplicationStates)eAppStateIdle == newState)
    {
        if((uint32_t)(0) == stateIdleCurrent)
        {
            stateIdleCurrent = (uint32_t)(1);
            stateAcclCurrent = (uint32_t)(0);
            stateDecelCurrent = (uint32_t)(0);
            stepperMotor->writeCurrent(eCurrHold);
        }
    } 
    else
    {
      /* Do Nothing */
    }
}

void delayElapsedCallback(DStepperController *hStepperController)
{
  hStepperController->stepperMotor->setMotorRunningStatus((uint32_t)(0));
  hStepperController->stepperMotor->setMotorMoveCompletionStatus((uint32_t)(1));
}

/**
 * @brief   THis function is the callback from a compare match interrupt
 * @param   void
 * @return  void
 */

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM3)
  {
      delayElapsedCallback(PV624->stepperController);
  }
   if (htim->Instance == TIM1)
  {
    HAL_TIM_OnePulse_Stop_IT(htim, TIM_CHANNEL_1);
  }

  if (htim->Instance == TIM4)
  {
    if (htim->Channel == (HAL_TIM_ActiveChannel)HAL_TIM_ACTIVE_CHANNEL_4)
    {
      HAL_TIM_OnePulse_Stop_IT(htim, TIM_CHANNEL_4);
    }
    else if(htim->Channel == (HAL_TIM_ActiveChannel)HAL_TIM_ACTIVE_CHANNEL_3)
    {
      HAL_TIM_OnePulse_Stop_IT(htim, TIM_CHANNEL_3);
    }
    else
    {
      /* Do nothing */
    }
  }
}




void pulseFinishedCallback(DStepperController *hStepperController)
{
    if((uint32_t)(1) == hStepperController->updatedDirection)
    {
        hStepperController->stepperMotor->incrementStepCounter();
    }
    else 
    {
        hStepperController->stepperMotor->decrementStepCounter();
    }
    /* Trying to use pulse finished call to accelerate and decel the motor */
    hStepperController->stepperMotor->setMotorAccelerateStatus((uint32_t)(1));
    hStepperController->stepperMotor->setMotorDecelerateStatus((uint32_t)(1));
}
/**
 * @brief   TThis function is the callback for a complete PWM pulse
 * @param   void
 * @return  void
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
   
    if(htim->Instance == TIM2)
    {
        pulseFinishedCallback(PV624->stepperController);       
    }
}

