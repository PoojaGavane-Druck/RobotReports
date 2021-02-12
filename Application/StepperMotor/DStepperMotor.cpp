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
#include "dspin.h"
MISRAC_ENABLE

#include "spi.h"
#include "DStepperMotor.h"

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
DStepperMotor::DStepperMotor(TIM_HandleTypeDef* hPulseCnt,
                  uint32_t timerChForCnt,
                  TIM_HandleTypeDef* hPwmTmr,
                  uint32_t timerChForPwm,
                  float32_t motorClkFrq)
{
    hPulseCounter = hPulseCnt;
    hPwmTimer = hPwmTmr;
    timerChForPulseCounter = timerChForCnt;
    timerChannelForPwm = timerChForPwm;
    motorClockFreq = motorClkFrq;
    
    initializeMotorParams();
    
    /* Make ready for SPI communication */
    SPI_SetNss();

    initController();
    
    setStepSize(eStepSizeHalfStep);
    
#ifdef 0       
     uint32_t counter = (uint32_t)(0);
#endif  
   
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 Error[Pm100]: dynamic heap memory allocation shall not be used (MISRA C 2004 rule 20.4)
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm148")


/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 Error[Pm100]: dynamic heap memory allocation shall not be used (MISRA C 2004 rule 20.4)
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm148")


/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   This function initializes all the variables in the application
 * @param   void
 * @return  void
 */
void DStepperMotor::initializeMotorParams(void)
{
    motorParms.acclAlpha = (float)(MOTOR_DEFAULT_ACCL_ALPHA);
    motorParms.acclBeta = (float)(MOTOR_DEFAULT_ACCL_BETA);
    motorParms.decclAlpha = (float)(MOTOR_DEFAULT_DECEL_ALPHA);
    motorParms.decclBeta = (float)(MOTOR_DEFAULT_DECEL_BETA);
    motorParms.motorMoveCommand = (uint32_t)(0);
    motorParms.steps = (uint32_t)(0);
    motorParms.motorRunning = (uint32_t)(0);
    motorParms.motorMoveComplete = (uint32_t)(0);
    motorParms.motorAccelerate = (uint32_t)(0);
    motorParms.motorDecelerate = (uint32_t)(0);
    motorParms.absoluteStepCounter = (int32_t)(0);
    motorParms.minimumSpeed = (uint32_t)(MOTOR_DEFAULT_MIN_SPEED);
    motorParms.maximumSpeed = (uint32_t)(MOTOR_DEFAULT_MAX_SPEED);
    motorParms.decelFactor = (float)(MOTOR_DEFAULT_DECEL_FACTOR);    
    motorParms.motorStopFlag = (uint32_t)(0);
    motorParms.previousSteps = (int32_t)(0);
    motorParms.overrunSteps = (uint32_t)(0);
    motorParms.motorSpeed = (uint32_t)(0);
    motorParms.motorCurrent = (uint32_t)(0);    
    motorParms.opticalSensorVal = (uint32_t)(0);      

    motorParms.currDecel = (float)(1000);
    motorParms.currAccl = (float)(1000);
    motorParms.currHold = (float)(200);
   
    /* Initialize file statics */
    motorParms.stepsBeforeDecel = (int32_t)(0);
    motorParms.stepsAfterDecel = (int32_t)(0);      

    motorParms.stepsCounter = (int32_t)(0);
    
}

/**
 * @brief   This function stops the PWM to the motor
 * @param   void
 * @return  void
 */
void DStepperMotor::stop(void)
{
    // Access the timer generating PWM to stop the same
    HAL_TIM_PWM_Stop(hPwmTimer, timerChannelForPwm);
}

/**
 * @brief   Resets the square wave generator to startup frequency
 * @param   void
 * @return  void
 */
void DStepperMotor::squareWaveFreqReset(void)
{
    // Startup frequency set here is 1kHz
    hPwmTimer->Instance->ARR = BASE_FREQ_ARR;        // Use variables
    hPwmTimer->Instance->CCR1 = BASE_FREQ_CCR;        // Use variables        
}

/**
 * @brief   This function stops the compare match module timer
 * @param   void
 * @return  void
 */
void DStepperMotor::stopCapture(void)
{
    HAL_TIM_OC_Stop(hPulseCounter, timerChForPulseCounter);
}



/**
 * @brief   This function accelerates the motor based on alpha and beta 
 *          to generate an S-Curve
 * @param   float alpha, float beta
 * @return  void
 */
void DStepperMotor::accelerate(float alpha, float beta)
{
    uint32_t arr = (uint32_t)(0);
    uint32_t ccr2 = (uint32_t)(0);    
    float newRate = (float)(0);
    
    /* change the PWM frequency here */
    arr =  hPwmTimer->Instance->ARR;
    ccr2 = hPwmTimer->Instance->CCR1;
    
    
    arr = arr + (uint32_t)(1);
    motorParms.motorSpeed = (static_cast<uint32_t>(motorClockFreq / (float)(arr)));
    
#if 0    
    if((uint32_t)(LENGTH) > counter)
    {
        speed[counter] = (static_cast<float32_t> (motorParms.motorSpeed));
        counter++;
    }
#endif
    newRate = (float)(arr) / (float)(MICROS_FACTOR);
    newRate = newRate * alpha;
    newRate = newRate + beta;
    newRate = newRate * (float)(MICROS_FACTOR);
    
    arr = (static_cast<uint32_t>(newRate - (float)(1)));
 
#if 0    
    motorFrequency = motorClockFreq / (float)(arr + (uint32_t)(1));
    
    rate = motorClockFreq / motorFrequency;
    rate = rate * alpha;
    rate = rate + beta;
    
    motorFrequency = motorClockFreq / rate;    
    
    if(motorFrequency >= (float)(motorParms.maximumSpeed))
    {
        stopAcclTimer();
        htim5.Instance->CNT = (uint32_t)(0);
        motorFrequency = (float)(motorParms.maximumSpeed);
        motorParms.accelComplete = (uint32_t)(1);
    }
    
    arr = (uint32_t)((motorClockFreq / motorFrequency) - (float)(1));
#endif
    ccr2 = arr >> 1; // always 50% duty
    
    hPwmTimer->Instance->ARR = arr;
    hPwmTimer->Instance->CCR1 = ccr2;   
}

/**
 * @brief   This function is used to receive commands from the PC
 * @param   uint32_t steps
 * @return  float timeLeft
 */
float DStepperMotor::calculateTimeRemaining(uint32_t steps)
{
    uint32_t currentStep = (uint32_t)(0);
    uint32_t remainingSteps = (uint32_t)(0);
    uint32_t arr = (uint32_t)(0);
    float motorFrequency = (float)(0);    
    float rate = (float)(0);
    float timeLeft = (float)(0);
    
    currentStep = (uint32_t)(hPulseCounter->Instance->CNT);
    remainingSteps = steps - currentStep;
    
    /* change the PWM frequency here */
    arr = hPwmTimer->Instance->ARR;
    
    motorFrequency = motorClockFreq / (static_cast<float32_t>((arr + (uint32_t)(1))));
    
    rate = (float)(1) / motorFrequency;
    
    timeLeft = (static_cast<float32_t>(remainingSteps)) * rate;
    
    return timeLeft;
}

/**
 * @brief   This function decelerates the motor using the alpha and beta
 *          parameters to generate a decel curve
 * @param   float alpha, float beta
 * @return  void
 */  
void DStepperMotor::decelerate(float alpha, float beta)
{
    uint32_t arr = (uint32_t)(0);
    uint32_t ccr2 = (uint32_t)(0);
    float motorFrequency = (float)(0);    
    float newRate = (float)(0);
    
    /* change the PWM frequency here */
    arr = hPwmTimer->Instance->ARR;
    ccr2 = hPwmTimer->Instance->CCR1;
    
    
    arr = arr + (uint32_t)(1);
    motorParms.motorSpeed = (static_cast<uint32_t>(motorClockFreq / (static_cast<float32_t>((arr + (uint32_t)(1))))));
    newRate = (float)(arr) / (float)(MICROS_FACTOR);
    newRate = newRate - beta;
    newRate = newRate * alpha;
    
    arr = (static_cast<uint32_t>(newRate * (float)(MICROS_FACTOR)));
    arr = arr - (uint32_t)(1);
    
#if 0
    motorFrequency = motorClockFreq / (float)(arr + (uint32_t)(1));
    motorParms.motorSpeed = (uint32_t)(motorFrequency);
    rate = motorClockFreq / motorFrequency;
    rate = rate * alpha;
    rate = rate - beta;
#endif
    motorFrequency = motorClockFreq / (float)(arr);    
#if 0    
    if((uint32_t)(999) > counter)
    {
        speed[counter] = (static_cast<float32_t>(motorParms.motorSpeed));
        counter++;
    }
#endif
    if(motorFrequency <= (float)(motorParms.minimumSpeed))
    {
      
#if 0      
        counter = (uint32_t)(0);
#endif
        //stopDecelTimer();
        motorFrequency = (float)(motorParms.minimumSpeed);
        motorParms.decelComplete = (uint32_t)(1);        
        
    }
    
    //arr = (uint32_t)((motorClockFreq / motorFrequency) - (float)(1));
    ccr2 = arr >> 1; // always 50% duty
    
    hPwmTimer->Instance->ARR = arr;
    hPwmTimer->Instance->CCR1 = ccr2;   
}

/**
 * @brief   This function updates the steps to the compare match timer
 * @param   uint32_t steps
 * @return  void
 */
void DStepperMotor::updateSteps(uint32_t steps)
{
    hPulseCounter->Instance->CCR2 = steps;
}

/**
 * @brief   This function starts the compare match timer
 * @param   void
 * @return  void
 */
void DStepperMotor::startCapture(void)
{
    HAL_TIM_OC_Start_IT(hPulseCounter, timerChForPulseCounter);
}

/**
 * @brief   This function starts the PWM output from the square wave generating timer
 * @param   void
 * @return  void
 */
void DStepperMotor::run(void)
{
    HAL_TIM_PWM_Start(hPwmTimer, timerChannelForPwm);
}


/**
 * @brief   This function starts the PWM output from the square wave generating timer with interrupt
 * @param   void
 * @return  void
 */
void DStepperMotor::run_IT(void)
{
    HAL_TIM_PWM_Start_IT(hPwmTimer, timerChannelForPwm);
}

/**
 * @brief   This function resets the compare match timer counts
 * @param   void
 * @return  void
 */
void DStepperMotor::resetCapture(void)
{
    hPulseCounter->Instance->CNT = (uint32_t)(0);
    
}

/**
 * @brief   This function reads the current step of the motor based on the timer counter
 * @param   void
 * @return  uint32_t steps
 */
uint32_t DStepperMotor::getCurrentStep(void)
{
    uint32_t value = (uint32_t)(0);

    value = (uint32_t)(hPulseCounter->Instance->CNT);

    return value;
}

/**
 * @brief   This function reads the number of steps taken by the motor
 * @param   void
 * @return  void
 */
int32_t DStepperMotor::getNumberOfStepsTakenByMotor(void)
{
    int32_t value = (int32_t)(0);

    /* Read the motor current position from the absolute position register in
    the L6472 */

    value = (int32_t)dSPIN_Get_Param((dSPIN_Registers_TypeDef)(dSPIN_ABS_POS));

    return value;
}



/**
 * @brief   This function initializes the controller at power on and on command
 * @param   void
 * @return  void
 */
void DStepperMotor::initController(void)
{
    OS_ERR os_error = OS_ERR_NONE;
    /* HW reset the controller */
    controllerResetHW();
    OSTimeDlyHMSM(0u, 0u, 0u, 10u, OS_OPT_TIME_HMSM_STRICT, &os_error);
    /* Software reset the controller */
    controllerResetSW();
    OSTimeDlyHMSM(0u, 0u, 0u, 10u, OS_OPT_TIME_HMSM_STRICT, &os_error);
    /* Read the status regsiter */
    controllerReadStatus();
    OSTimeDlyHMSM(0u, 0u, 0u, 10u, OS_OPT_TIME_HMSM_STRICT, &os_error);
    /* Set motor default direction */
    //dSPIN_Step_Clock((dSPIN_Direction_TypeDef)(0x59));
}

/**
 * @brief   This function HW resets the L6472
 * @param   void
 * @return  void
 */
void DStepperMotor::controllerResetHW(void)
{
    OS_ERR os_error = OS_ERR_NONE;
    /* Generate a high to low going pulse on the controller reset pin */
    HAL_GPIO_WritePin(STEPPER_STBY_RST_PG1_GPIO_Port, STEPPER_STBY_RST_PG1_Pin, GPIO_PIN_SET);
    //HAL_Delay(10u);
    OSTimeDlyHMSM(0u, 0u, 0u, 10u, OS_OPT_TIME_HMSM_STRICT, &os_error);
    HAL_GPIO_WritePin(STEPPER_STBY_RST_PG1_GPIO_Port, STEPPER_STBY_RST_PG1_Pin, GPIO_PIN_RESET);
    OSTimeDlyHMSM(0u, 0u, 0u, 10u, OS_OPT_TIME_HMSM_STRICT, &os_error);
    //HAL_Delay(10u);
    HAL_GPIO_WritePin(STEPPER_STBY_RST_PG1_GPIO_Port, STEPPER_STBY_RST_PG1_Pin, GPIO_PIN_SET);
    OSTimeDlyHMSM(0u, 0u, 0u, 100u, OS_OPT_TIME_HMSM_STRICT, &os_error);
    //HAL_Delay(100u); /* Give some time for the controller to start */
}

/**
 * @brief   THis function SW resets the L6472 contrller
 * @param   void
 * @return  void
 */
void DStepperMotor::controllerResetSW(void)
{
    /* Send the reset command to the controller */
    dSPIN_Reset_Device();
}

/**
 * @brief   This function reads the status of the controller
 * @param   void
 * @return  void
 */
uint16_t DStepperMotor::controllerReadStatus(void)
{
    uint16_t status = (uint16_t)(0);

    /* Read the status register of the controller */
    /* Reading the regsiter clears it */
    status = dSPIN_Get_Status();
    return status;
}



/**
 * @brief   This function updates the absolute step counter
 * @param   void
 * @return  void
 */
void DStepperMotor::updateAbsoluteStepCounter(void)
{
#if 0
    uint32_t steps = (uint32_t)(0);
    
    steps = hPulseCounter->Instance->CNT;
    
    if(motorParms.currentDirection == (uint32_t)(1))
    {
        motorParms.absoluteStepCounter = motorParms.absoluteStepCounter + steps;
    }
    else
    {
        motorParms.absoluteStepCounter = motorParms.absoluteStepCounter - steps;
    }
#endif
}

/**
 * @brief   This function is used to receive commands from the PC
 * @param   uint32_t totalSteps, uint32_t minSpeed, float DecelFactor
 * @return  uint32_t decelStart
 */
uint32_t DStepperMotor::checkDecelerationRequired(uint32_t totalSteps, 
                                         uint32_t minSpeed,
                                         float decelFactor,
                                         float accAlpha,
                                         float accBeta)
{
    uint32_t currentStep = (uint32_t)(0);
    uint32_t remainingSteps = (uint32_t)(0);
    uint32_t decelStart = (uint32_t)(0);
    uint32_t decelSteps = (uint32_t)(0);
    
    decelSteps = calculateDecelSteps(minSpeed, 
                                           decelFactor, 
                                           accAlpha, 
                                           accBeta);
    currentStep = (uint32_t)(hPulseCounter->Instance->CNT);
    
    if(totalSteps > currentStep)
    {
        remainingSteps = totalSteps - currentStep;
    }
    else
    {
        remainingSteps = currentStep - totalSteps;
    }
    
    
    if((decelSteps + (uint32_t)(STEPS_BUFFER)) >= remainingSteps)
    {
        decelStart = (uint32_t)(1);
    }       
    
    return decelStart;
}       

/**
 * @brief   This function calculates the steps required for deceleration
 * @param   uint32_t minSpeed, float decelFactor
 * @return  uint32_t steps
 */
uint32_t DStepperMotor::calculateDecelSteps(uint32_t minSpeed, 
                                   float decelFactor, 
                                   float accAlpha, 
                                   float accBeta)
{
    uint32_t steps = (uint32_t)(0);
    float speed = (float)(0);
    
    uint32_t arr = (uint32_t)(0);
    uint32_t timClk = (uint32_t)(12000000);     // change to define
    float newRate = (float)(0);
    
    /* Change this to count based on next pulse not the current pulse */
    
    arr = hPwmTimer->Instance->ARR;
        
    newRate = (float)(arr) / (float)(MICROS_FACTOR);
    newRate = newRate * accAlpha;
    newRate = newRate + accBeta;
    newRate = newRate * (float)(MICROS_FACTOR); 
    
    arr = (static_cast<uint32_t>(newRate - (float)(1.0f)));
    
    speed = (float)((float)(timClk) / (static_cast<float32_t>(arr + (uint32_t)(1))));
    
    while(speed > (float)(minSpeed))
    {
        steps = steps + 1u;
        speed = speed / decelFactor;
    }
                                                                                                     
    return steps;
}

/**
 * @brief   This function toggles the direction of the motor using GPIO
 * @param   uint32_t direction
 * @return  void
 */
void DStepperMotor::updateDirection(uint32_t direction)
{
      /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, (GPIO_PinState)(direction));
}

/**
 * @brief   This function sets the stepper motor step size
 * @param   MtrStepSize_t stepSize
 * @return  void
 */
bool DStepperMotor::setStepSize(MtrStepSize_t stepSize)
{
    uint8_t regAddr = (uint8_t)(eL6472_RegStepMode);
    uint32_t data = (uint32_t)(stepSize);
    bool successFlag = false;
    
    successFlag = writeVerifyRegister(regAddr, data);   
    
    return successFlag;
}

/**
 * @brief   Writes and verifies regsiter in the motor driver chip
 * @param   uint8_t regAddr, uint32_t data
 * @return  void
 */
bool DStepperMotor::writeVerifyRegister(uint8_t regAddr, uint32_t data)
{
    uint32_t rdData = (uint32_t)(0);
    bool successFlag = false;
    
    writeRegister(regAddr, data);
    
    readRegister(regAddr, &rdData);
    
    if(data == rdData)
    {
        successFlag = true;
    }
    
    return successFlag;
}

/**
 * @brief   Writes a regsiter in the motor driver chip
 * @param   uint8_t regAddr, uint32_t data
 * @return  void
 */
void DStepperMotor::writeRegister(uint8_t regAddr, uint32_t data)
{
    dSPIN_Set_Param((dSPIN_Registers_TypeDef)(regAddr),data);
}
                             
/**
 * @brief   Reads a regsiter in the motor driver chip
 * @param   uint8_t regAddr, uint32_t *data
 * @return  void
 */
void DStepperMotor::readRegister(uint8_t regAddr, uint32_t *data)
{
    *data = dSPIN_Get_Param((dSPIN_Registers_TypeDef)(regAddr));
}

/**
 * @brief   This function sets the direction of rotation of the motor
 * @param   MtrDirection_t direction
 * @return  void
 */
void DStepperMotor::setDirection(MtrDirection_t direction)
{
    MtrDirection_t newDirection = direction;
    
    if(motorParms.currentDirection != newDirection)
    {
      motorParms.currentDirection = newDirection;
      motorParms.directionChanged = (uint32_t)(1);
    }
}

/**
 * @brief   This function writes the current value for the passed stage such as hold / run etc.
 * @param   MotorCurrentTypedef_t currentType, uint32_t currentValue
 * @return  void
 */
void DStepperMotor::writeCurrent(MotorCurrentTypedef_t currentType)
{
    uint32_t value = (uint32_t)(0);
    
    float currentValue = 0.0f;
      
    switch(currentType)
    {
      case eCurrHold: 
        currentValue = motorParms.currHold;
        break;
      
      case eCurrRun:
        currentValue = motorParms.currRun;
      break;
      
      case eCurrAccl:
        currentValue = motorParms.currAccl;
      break;
      
      case eCurrDecel:
        currentValue = motorParms.currDecel;
      break;
      
      default:
          currentType = eCurrNone;
      break;
    }
    
    if((MotorCurrentTypedef_t)eCurrNone != currentType)
    {
      if(currentValue >= (float)(MAX_CURRENT))
      {
          currentValue = (float)(MAX_CURRENT);
      }
      
      value = (static_cast<uint32_t>((currentValue) / (float)(CURR_RESOLUTION)));      
      dSPIN_Set_Param((dSPIN_Registers_TypeDef)(dSPIN_KVAL_HOLD), value);
#if 0
      if((MotorCurrentTypedef_t)eCurrHold == currentType)
      {
          dSPIN_Set_Param((dSPIN_Registers_TypeDef)(eCurrHold), value);
      }
      else if((MotorCurrentTypedef_t)eCurrRun == currentType)
      {
          dSPIN_Set_Param((dSPIN_Registers_TypeDef)(eCurrRun), value);
      }
      else if((MotorCurrentTypedef_t)eCurrAccl == currentType)
      {
          dSPIN_Set_Param((dSPIN_Registers_TypeDef)(eCurrAccl), value);
      }
      else if((MotorCurrentTypedef_t)eCurrDecel == currentType)
      {
          dSPIN_Set_Param((dSPIN_Registers_TypeDef)(eCurrDecel), value);
      }
      else
      {
        /*Do Nothing*/
      }
#endif
    }
}

MtrDirection_t DStepperMotor::getDirection(void)
{
 return (MtrDirection_t)motorParms.currentDirection;
}


uint32_t DStepperMotor::getMotorMoveCompletionStatus(void)
{
 return motorParms.motorMoveComplete;
}

void DStepperMotor::setMotorMoveCompletionStatus(uint32_t moveStatus)     
{
 motorParms.motorMoveComplete = moveStatus;
}

uint32_t DStepperMotor::getMotorRunningStatus(void)
{
 return motorParms.motorRunning;
}

void DStepperMotor::setMotorRunningStatus(uint32_t runStatus)
{
  motorParms.motorRunning = runStatus;
}
 

uint32_t DStepperMotor::getDecelComplettionStatus(void)
{
  return motorParms.decelComplete;
}

void DStepperMotor::setDecelComplettionStatus(uint32_t newStatus)
{
  motorParms.decelComplete = newStatus;
}

uint32_t DStepperMotor::getAecelComplettionStatus(void)
{
  return motorParms.accelComplete;
}

void DStepperMotor::setAecelComplettionStatus(uint32_t newStatus)
{
  motorParms.accelComplete = newStatus;
}

float32_t DStepperMotor::getAccelerationAlpha(void)
{
  return motorParms.acclAlpha;
}

void DStepperMotor::setAccelerationAlpha(float32_t newAlpha)
{
  motorParms.acclAlpha = newAlpha;
}

float32_t DStepperMotor::getAccelerationBeta(void)
{
  return motorParms.acclBeta;
}

void DStepperMotor::setAccelerationBeta(float32_t newBeta)
{
  motorParms.acclBeta = newBeta;
}

float32_t DStepperMotor::getDecelerationAlpha(void)
{
  return motorParms.decclAlpha;
}

void DStepperMotor::setDecelerationAlpha(float32_t newAlpha)
{
  motorParms.decclAlpha = newAlpha;
}

float32_t DStepperMotor::getDecelerationBeta(void)
{
  return motorParms.decclBeta;
}

void DStepperMotor::setDecelerationBeta(float32_t newBeta)
{
  motorParms.decclBeta = newBeta;
}

void DStepperMotor::setNumberOfsteps(uint32_t newNumberOfSteps)
{
  motorParms.steps = newNumberOfSteps;
}

uint32_t DStepperMotor::getOverrunStepsCount(void)
{
  return motorParms.overrunSteps;
}

void DStepperMotor::setOverrunStepsCount(uint32_t newOverrunStepsCount)
{
  motorParms.overrunSteps = newOverrunStepsCount;
}


uint32_t DStepperMotor::getDecelerationStepsCount(void)
{
  return motorParms.decelSteps;
}

void DStepperMotor::setDecelerationStepsCount(uint32_t newDecelerationSteps)
{
  motorParms.decelSteps = newDecelerationSteps;
}

float32_t DStepperMotor::getDecelerationFactor(void)
{
  return motorParms.decelFactor;
}

void DStepperMotor::setDecelerationFactor(float32_t newFactor)
{
  motorParms.decelFactor = newFactor;
}


int32_t DStepperMotor::getStepsCounter(void)
{
  return motorParms.stepsCounter;
}

void DStepperMotor::setStepsCounter(int32_t newStepsCnt)
{
  motorParms.stepsCounter = newStepsCnt;
}

int32_t DStepperMotor::getAbsoluteStepCount(void)
{
  return motorParms.absoluteStepCounter;
}

void DStepperMotor::setAbsoluteStepCount(int32_t newStepCount)
{
  motorParms.absoluteStepCounter = newStepCount;
}

uint32_t DStepperMotor::getDirectionChangedStatus(void)
{
  return motorParms.directionChanged;
}

void DStepperMotor::setDirectionChangedStatus(uint32_t newDirChangeStatus)
{
  motorParms.directionChanged = newDirChangeStatus;
}

uint32_t DStepperMotor::getMotorMoveCommandStatus(void)
{
  return motorParms.motorMoveCommand;
}

void DStepperMotor::setMotorMoveCommandStatus(uint32_t cmdStatus)
{
  motorParms.motorMoveCommand = cmdStatus;
}

uint32_t DStepperMotor::getMotorStopFlagStatus(void)
{
  return motorParms.motorStopFlag;
}

void DStepperMotor::setMotorStopFlagStatus(uint32_t flagStatus)
{
  motorParms.motorStopFlag = flagStatus;
}

uint32_t DStepperMotor::getSteps(void)
{
  return motorParms.steps;
}

void DStepperMotor::setSteps(uint32_t newStepsCount)
{
  motorParms.steps = newStepsCount;
  setMotorMoveCommandStatus((uint32_t)(1));
}

void DStepperMotor::incrementStepCounter(void)
{
  motorParms.stepsCounter = motorParms.stepsCounter + 1;
}

void DStepperMotor::decrementStepCounter(void)
{
  motorParms.stepsCounter = motorParms.stepsCounter - 1;
}

uint32_t DStepperMotor::getMotorAccelerateStatus(void)
{
  return motorParms.motorAccelerate;
}
void DStepperMotor::setMotorAccelerateStatus(uint32_t newStatus)
{
  motorParms.motorAccelerate = newStatus;
}
    
uint32_t DStepperMotor::getMotorDecelerateStatus(void)
{
  return motorParms.motorDecelerate;
}

void DStepperMotor::setMotorDecelerateStatus(uint32_t newStatus)
{
  motorParms.motorDecelerate = newStatus;
}

uint32_t DStepperMotor::getMotorSpeedStatus(void)
{
  return motorParms.motorSpeed;       
}

void DStepperMotor::setMotorSpeedStatus(uint32_t newStatus)
{
  motorParms.motorSpeed = newStatus;
}


int32_t DStepperMotor::getNumOfStepsBeforeDecel(void)
{
   return motorParms.stepsBeforeDecel;
}

void DStepperMotor::setNumOfStepsBeforeDecel(int32_t newStepsCount)
{
   motorParms.stepsBeforeDecel = newStepsCount;
}
    
int32_t DStepperMotor::getNumOfStepsAfterDecel(void)
{
  return motorParms.stepsAfterDecel;
}

void DStepperMotor::setNumOfStepsAfterDecel(int32_t newStepsCount)
{
  motorParms.stepsAfterDecel = newStepsCount;
}
/**
 * @brief   This function is used to receive commands from the PC
 * @param   uint32_t totalSteps, uint32_t minSpeed, float DecelFactor
 * @return  uint32_t decelStart
 */
uint32_t DStepperMotor::checkDecelerationRequired(void)
{
    uint32_t currentStep = (uint32_t)(0);
    uint32_t remainingSteps = (uint32_t)(0);
    uint32_t decelStart = (uint32_t)(0);
    uint32_t decelSteps = (uint32_t)(0);
    
    decelSteps = calculateDecelSteps();
    currentStep = (uint32_t)(hPulseCounter->Instance->CNT);
    
    if(motorParms.steps > currentStep)
    {
        remainingSteps = motorParms.steps  - currentStep;
    }
    else
    {
        remainingSteps = currentStep - motorParms.steps;
    }
    
    
    if((decelSteps + (uint32_t)(STEPS_BUFFER)) >= remainingSteps)
    {
        decelStart = (uint32_t)(1);
    }       
    
    return decelStart;
}       

/**
 * @brief   This function calculates the steps required for deceleration
 * @param   uint32_t minSpeed, float decelFactor
 * @return  uint32_t steps
 */
uint32_t DStepperMotor::calculateDecelSteps(void)
{
    uint32_t steps = (uint32_t)(0);
    float speed = (float)(0);
    
    uint32_t arr = (uint32_t)(0);
    uint32_t timClk = (uint32_t)(12000000);     // change to define
    float newRate = (float)(0);
    
    /* Change this to count based on next pulse not the current pulse */
    
    arr = hPwmTimer->Instance->ARR;
        
    newRate = (float)(arr) / (float)(MICROS_FACTOR);
    newRate = newRate * motorParms.acclAlpha;
    newRate = newRate + motorParms.acclBeta;
    newRate = newRate * (float)(MICROS_FACTOR); 
    
    arr = (static_cast<uint32_t>(newRate - (float)(1.0f)));
    
    speed = (float)((float)(timClk) / (static_cast<float32_t>(arr + (uint32_t)(1))));
    
    while(speed > (float)(motorParms.minimumSpeed))
    {
        steps = steps + 1u;
        speed = speed / motorParms.decelFactor;
    }
                                                                                                     
    return steps;
}

void DStepperMotor::enablePulseCounterTimerInterrupt(void)
{
  __HAL_TIM_ENABLE_IT(hPulseCounter, TIM_IT_CC1);
}