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
* @file     DController.c
* @version  1.00.00
* @author   Makarand Deshmukh / Nageswara Pydisetty
* @date     31-08-2021
*
* @brief    Source file for pressure control algorithm
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include "math.h"
#include <rtos.h>
MISRAC_ENABLE
#include "memory.h"
#include "smbus.h"
#include "DLock.h"
#include "DPV624.h"
#include "main.h"
#include "Controller.h"
#include "utilities.h"

/* Error handler instance parameter starts from 2301 to 2400 */
/* Defines and constants --------------------------------------------------------------------------------------------*/
#define ENABLE_VALVES
//#define ENABLE_MOTOR
#define ENABLE_MOTOR_CC

#define DUMP_PID_DATA
#define DUMP_BAYES_DATA
#define ALGO_V2
//#define DUMP_CONTROLLER_STATE

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Global Variables -------------------------------------------------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;

/* File Statics -----------------------------------------------------------------------------------------------------*/
static const float32_t EPSILON = 1.0E-10f;  //arbitrary 'epsilon' value
static const float32_t piValue = 3.14159f;

/* User Code --------------------------------------------------------------------------------------------------------*/

/**
* @brief    Controller class constructor
* @param    void
* @retval   void
*/
DController::DController()
{
    initialize();
}

/**
* @brief    Controller class destructor
* @param    void
* @retval   void
*/
DController::~DController()
{

}

/**
* @brief    Initialize all the parameters and variable values
* @param    void
* @retval   void
*/
void DController::initialize(void)
{
    initMainParams();
    initSensorParams();
    initPidParams();
    initMotorParams();
    initScrewParams();
    initBayesParams();
    initTestParams();
    initCenteringParams();
}

/**
* @brief    Initializes all high level pressure control parameters
* @param    void
* @retval   void
*/
void DController::initMainParams(void)
{
    controllerState = eCoarseControlLoopEntry;

    msTimer = 0u;
    entryState = 0u;

    setPointG = 0.0f;
    gaugePressure = 0.0f;
    absolutePressure = 0.0f;
    atmosphericPressure = 0.0f;
    pressureSetPoint = 0.0f;

    controllerStatus.bytes = 0u;

    sensorFsValue = 20000.0f;
    fsValue = 7000.0f;

    prevVentState = 0u;
    entryIterations = 0u;
    entryInitPressureG = 0.0f;
    entryFinalPressureG = 0.0f;

    myMode = E_CONTROLLER_MODE_VENT;
    myPrevMode = E_CONTROLLER_MODE_VENT;

    stateCentre = eCenteringStateNone;
}

/**
* @brief    Init sensor parameters used by the control algorithm
* @param    void
* @retval   void
*/
void DController::initSensorParams(void)
{
    sensorParams.fullScalePressure = 7000.0f;   // Starting with a standard range to avoid divide by 0 problems
    sensorParams.pressureType = 1u;
    sensorParams.sensorType = 1u;
    sensorParams.terpsPenalty = 1u;
    sensorParams.minGaugeUncertainty = 5.0f;
    sensorParams.gaugeUncertainty = sensorParams.minGaugeUncertainty;
    sensorParams.maxOffset = 60.0f;
    sensorParams.offset = 0.0f;
    sensorParams.offsetSafetyFactor = 1.5f;
}


/**
* @brief    Init PID controller parameters
* @param    void
* @retval   void
*/
void DController::initPidParams(void)
{
    // elapsed time for log file(s)
    pidParams.elapsedTime = 0u;
    // pressure setpoint(mbar)
    pidParams.pressureSetPoint = 0.0f;
    // setpoint type from GENII(0 = gauge, 1 = abs, 2 = baro)
    pidParams.setPointType = eGauge;
    // number of motor pulses delivered since last stepSize request
    pidParams.stepCount = 0;
    // pressure error for PID(mbar), +ve == below pressure setpoint
    pidParams.pressureError = 0.0f;
    // total step count since start
    pidParams.totalStepCount = 0;
    // controlled pressure(mbar gage or mbar abs)
    pidParams.controlledPressure = 0.0f;
    // requested number of steps to turn motor
    pidParams.stepSize = 0;
    // leak - adjusted pressure correction target(mbar)
    pidParams.pressureCorrectionTarget = 0.0f;
    // optical piston position(steps), 0 == fully retracted / max volume
    pidParams.pistonPosition = screwParams.centerPositionCount;
    // max relative distance from setpoint before pumping is required, e.g. 0.1 == 10 % of setpoint
    pidParams.pumpTolerance = 0.005f;
    // minimum pump tolerance for largest volumes
    pidParams.minPumpTolerance = 0.001f;
    // Maximum pump tolerance for smallest volumes
    pidParams.maxPumpTolerance = 0.01f;
    // number of pump up or down attempts for current setpoint value
    pidParams.pumpAttempts = 0u;
    // amount to overshoot setpoint when pumping
    pidParams.overshoot = 0.0f;
    // pumpTolerance scaling factor when setting nominal overshoot
    pidParams.overshootScaling = 3.0f;
    // target vent rate, mbar / iteration
    pidParams.ventRate = screwParams.minVentRate;
    // iteration count when holding vent open after vent complete
    pidParams.holdVentCount = screwParams.holdVentInterval;
    // measure mode variable for decision making
    pidParams.modeMeasure = 0u;
    // control mode variable for decision making
    pidParams.modeControl = 0u;
    // vent mode variable for deciesion making
    pidParams.modeVent = 0u;
    // Initialize status variables
    pidParams.mode = 2u;
    pidParams.status = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.control = 0u;
    pidParams.venting = 0u;
    pidParams.stable = 0u;
    pidParams.vented = 0u;
    pidParams.excessLeak = 0u;
    pidParams.excessVolume = 0u;
    pidParams.overPressure = 0u;
    pidParams.excessOffset = 0u;
    pidParams.measure = 0u;
    pidParams.fineControl = 0u;
    pidParams.pistonCentered = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.rangeExceeded = 0u;
    pidParams.ccError = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
    pidParams.maxLim = 1u;
    pidParams.minLim = 1u;
    pidParams.fastVent = 0u;
}

/**
* @brief    Init motor parameters
* @param    void
* @retval   void
*/
void DController::initMotorParams(void)
{
    // full step angle of motor(deg)
    motorParams.motorStepSize = 1.8f;
    // number of microsteps(2 == halfstep)
    motorParams.microStepSize = 4.0f;
    // maximum number of steps that can be taken in one control iteration, used in coarse control loop
    motorParams.maxStepSize = MAX_MOTOR_STEPS_POS;
}

// Write reason of suppressing MISRA rule TODO
#pragma diag_suppress=Pm137 /* Disable MISRA C 2004 rule 18.4 */
/**
* @brief    Init screw parameters used in the screw press
* @param    void
* @retval   void
*/
void DController::initScrewParams(void)
{
    screwParams.motorStepSize = 1.8f;
    screwParams.maxStepSize = MAX_MOTOR_STEPS_POS;
    screwParams.microStep = 4.0f;
    // gear ratio of motor
    screwParams.gearRatio = 1.0f;
    // piston diameter(mm), Helix0.6 press
    screwParams.pistonDiameter = 12.2f;
    // lead screw pitch(mm / rotation)
    screwParams.leadScrewPitch = 0.6f;
    // travel piston(mm)
    screwParams.leadScrewLength = 40.0f;
    // piston area(mm ^ 2)
    screwParams.pistonArea = piValue * (screwParams.pistonDiameter / 2.0f) * (screwParams.pistonDiameter / 2.0f);
    // volume change per control pulse(mL / pulse)
    screwParams.changeInVolumePerPulse = (motorParams.motorStepSize * screwParams.leadScrewPitch *
                                          screwParams.pistonArea * 1.0e-3f) / (motorParams.microStepSize *
                                                  screwParams.gearRatio * 360.0f);
    // maximum system pressure(mbar) for accel / decel current scaling
    screwParams.maxPressure = 21000.0f;
    // pulse count for full compression of piston, with 2 % safety factor to avoid collisions
    screwParams.maxPosition = int32_t((screwParams.leadScrewLength / screwParams.leadScrewPitch) *
                                      (360.0f / screwParams.motorStepSize) * screwParams.microStep);
    screwParams.minPosition = 0;
    // step count at nominal center position(home)
    screwParams.centerPositionCount = (int32_t)(screwParams.maxPosition / 2);
    // tolerance for finding center position(counts)
    screwParams.centerTolerance = 3000;
    // shunt resistance for current sensor, motor testing(ohm)
    screwParams.shuntResistance = 0.05f;
    // V / V gain of current sensor, with 20k Rload sensor and 50k ADC input impedance
    screwParams.shuntGain = 0.2f * (20.0f * 50.0f / (20.0f + 50.0f));
    // conversion factor from shunt ADC counts to current(mA)
    screwParams.readingToCurrent = 3.3f / (4096.0f * screwParams.shuntGain * screwParams.shuntResistance) * 1000.0f;
    // maximum leak rate bound(mbar / interation) at 20 bar and 10 mA, PRD spec is < 0.2
    screwParams.maxLeakRate = 0.3f;
    // maximum allowed pressure in screw press(PM independent) (mbar), for max leak rate adjustments
    screwParams.maxAllowedPressure = 21000.0f;
    // nominal total volume(mL), for max leak rate adjustments
    screwParams.nominalTotalVolume = 10.0f;

    //screw['orificeK'] = 3.5  # vent orifice flow constant, empirically determined (mL/s)
    screwParams.orificeK = 3.5f;
    //screw['ventSR'] = 0.075  # nominal control loop iteration time during controlled vent operation (s)
    screwParams.ventSr = 0.075f;
    //screw['ventUncertainty'] = 0.4  # uncertainty in volume estimate during controlled vent, 0.4 = +/- 40%
    screwParams.ventUncertainty = 0.4f;
    //screw['ventDelay'] = 2  number of control iterations to wait after switching vent valves before estimating volume
    screwParams.ventDelay = 2u;

    // Added for v6 control algorithm
    screwParams.minVentDutyCycle = 500u;  // Min vent duty cycle during vent, in us
    screwParams.maxVentDutyCycle = 6000u;  // Max vent duty cycle during vent in us
    screwParams.ventDutyCycleIncrement = 10u;    // Duty cycle increment per iteration
    screwParams.holdVentDutyCycle = 200u; // vent duty cycle when holding vent at 20% pwm
    screwParams.maxVentDutyCyclePwm = 500u;   // Maximum vent duty while venting
    screwParams.holdVentInterval = 50u;  //number of control iterations between applying vent pulse
    screwParams.ventResetThreshold = 0.5f; // reset threshold for setting bayes ventDutyCycle to reset
    screwParams.maxVentRate = 1000.0f;   // Maximum controlled vent rate
    screwParams.minVentRate = 1.0f;   // Minimum controlled vent rate
    screwParams.ventModePwm = 1u;   // for setting valve 3 to be pwm
    screwParams.ventModeTdm = 0u;   // for setting valve 3 to be tdm
    screwParams.holdVentIterations = (uint32_t)((float32_t)(screwParams.holdVentInterval) * 0.2f); // cycs to hold vent

    // For distance travelled
    screwParams.distancePerStep = (float32_t)(screwParams.leadScrewPitch) /
                                  (float32_t)((360.0f * screwParams.microStep) / screwParams.motorStepSize);
    screwParams.distanceTravelled = 0.0f;
}
#pragma diag_default=Pm137 /* Disable MISRA C 2004 rule 18.4 */

/**
* @brief    Init bayes parameters
* @param    void
* @retval   void
*/
void DController::initBayesParams(void)
{
    // minimum system volume estimate value(mL)
    bayesParams.minSysVolumeEstimate = 2.0f;
    // maximum system volume estimate value(mL)
    bayesParams.maxSysVolumeEstimate = 100.0f;
    // minimum estimated leak rate(mbar)
    bayesParams.minEstimatedLeakRate = 0.0f;
    // maximum absolute value of estimated leak rate(+/ -mbar / iteration)
    bayesParams.maxEstimatedLeakRate  = 0.2f;
    // measured pressure(mbar)
    bayesParams.measuredPressure = 1000.0f;
    // smoothed pressure, depends on controlled pressure stability not sensor uncertainty, (mbar)
    bayesParams.smoothedPresure = 1000.0f;
    // measured change in pressure from previous iteration(mbar)
    bayesParams.changeInPressure = 0.0f;
    // previous dP value(mbar)
    bayesParams.prevChangeInPressure = 0.0f;

    bayesParams.dP2 = 0.0f;
    // etimate of volume(mL), set to minV to give largest range estimate on startup
    bayesParams.estimatedVolume = bayesParams.maxSysVolumeEstimate;
    // algorithm used to calculate V
    bayesParams.algorithmType = eMethodNone;
    // volume change from previous stepSize command(mL)
    bayesParams.changeInVolume = 0.0f;
    // previous dV value(mL), used in regression method
    bayesParams.prevChangeInVolume = 0.0f;

    bayesParams.dV2 = 0.0f;
    // volume estimate using Bayes regression(mL)
    bayesParams.measuredVolume = bayesParams.maxSysVolumeEstimate;
    // estimate in leak rate(mbar / iteration), from regression method
    bayesParams.estimatedLeakRate = bayesParams.minEstimatedLeakRate;
    //measured leak rate using Bayes regression(mbar / iteration)
    bayesParams.measuredLeakRate = bayesParams.minEstimatedLeakRate;
    // estim. kP(steps/mbar) that will reduce pressure error to zero in one iteration, large for fast initial response
    bayesParams.estimatedKp = 500.0f;
    /*
    bayes['measkP'] = bayes['kP']
    measured optimal kP(steps / mbar) that will reduce pressure error to zero in one iteration
    state value variances
    bayesParams.sensorUncertainity = (10e-6 * sensorFsValue) * (10e-6 * sensorFsValue)
    uncertainty in pressure measurement(mbar), sigma ~= 10 PPM of FS pressure @ 13 Hz read rate

    23-06-2022 - Modified to 20 ppm
    */

    bayesParams.sensorUncertainity = (20e-6f * fsValue) * (20e-6f * fsValue);   // 20 ppm
    bayesParams.uncertaintyPressureDiff = 2.0f * bayesParams.sensorUncertainity;
    // uncertainty in measured pressure differences(mbar)
    bayesParams.uncertaintyVolumeEstimate = bayesParams.maxSysVolumeEstimate * 1e6f;
    // uncertainty in volume estimate(mL), large because initial volume is unknown, from regression method
    bayesParams.uncertaintyMeasuredVolume = (screwParams.changeInVolumePerPulse * 10.0f) *
                                            (screwParams.changeInVolumePerPulse * 10.0f);
    //uncertainty in volume estimate from latest measurement using bayes regression(mL)
    bayesParams.uncertaintyVolumeChange = (screwParams.changeInVolumePerPulse * 10.0f) *
                                          (screwParams.changeInVolumePerPulse * 10.0f);
    // uncertainty in volume change, depends mostly on backlash ~= +/ -10 half - steps, constant, mL
    bayesParams.uncertaintyEstimatedLeakRate = bayesParams.uncertaintyPressureDiff;
    //uncertainty in leak rate from bayes estimateand gas law(mbar / iteration), from regression method
    bayesParams.uncertaintyMeasuredLeakRate  = bayesParams.uncertaintyPressureDiff;
    //measured leak rate uncertainty from bayes regresssion estimate(mbar / iteration)
    bayesParams.maxZScore = 2.0f;
    // maximum variance spread between measuredand estimated values before estimated variance is increased
    bayesParams.lambda = 0.1f; // forgetting factor for smoothE
    bayesParams.uncerInSmoothedMeasPresErr  = 0.0f; //smoothed measured pressure error variance(mbar * *2)
    bayesParams.targetdP = 0.0f; // target correction from previous control iteration(mbar)
    bayesParams.smoothedPressureErr  = 0.0f; // smoothed pressure error(mbar)
    bayesParams.smoothedSquaredPressureErr = 0.0f; // smoothed squared pressure error(mbar * *2)
    bayesParams.uncerInSmoothedMeasPresErr  = 0.0f; // smoothed measured pressure error variance(mbar * *2)
    bayesParams.gamma = 0.98f;
    /*
    volume scaling factor for nudging estimated volume with predictionError, (0.90, 0.98),
    larger = faster response but noisier estimate
    */
    // prediction error from previous control iteration(mbar)
    bayesParams.predictionError  = 0.0f;
    // prediction error type(+/ -1), for volume estimate adjustment near setpoint
    bayesParams.predictionErrType = 0;
    // maximum iterations for leak rate integration filter in PE correction method
    //bayesParams.maxIterationsForIIRfilter  = (uint32_t)100;
    bayesParams.maxIterationsForIIRfilter  = 300u; // Updated from latest python code
    // minimum iterations for leak rate integration filter in PE correction method
    bayesParams.minIterationsForIIRfilter  = 10u;
    // change to estimated leak rate for PE correction method(mbar / iteration)
    bayesParams.changeToEstimatedLeakRate = 0.0f;
    // low - pass IIR filter memory factor for PE correction method(0.1 to 0.98)
    bayesParams.alpha = 0.1f;
    // smoothed pressure error for PE correction method(mbar)
    bayesParams.smoothedPressureErrForPECorrection = 0.0f;
    // acceptable residual fractional error in PE method leak rate estimate(-2 = +/ -1 %, -1 = 10 %, -0.7 = 20 %)
    bayesParams.log10epsilon = -0.7f;
    bayesParams.residualLeakRate = 0.0f;
    bayesParams.measuredLeakRate1 = 0.0f;
    bayesParams.numberOfControlIterations = bayesParams.minIterationsForIIRfilter;

    bayesParams.ventIterations = 0u; // number of control iterations since start of controlled vent
    bayesParams.ventInitialPressure = 0.0f; // initial pressure at start of controlled vent (mbar G)
    bayesParams.ventFinalPressure = 0.0f; // final pressure at end of controlled vent (mbar G)

    bayesParams.ventDutyCycle = screwParams.minVentDutyCycle; // energized time of vent valve solenoid (us)
    bayesParams.totalVentTime = 0.0f; // total time non-latching vent valve has been energized during controlled vent
    bayesParams.dwellCount = 0u; // number of control iterations into adiabatic dwell at end of coarse control
}

/**
* @brief    Init Test parameters
* @param    void
* @retval   void
*/
void DController::initTestParams(void)
{
    // simulated leak rate(mbar / iteration) at 10mL and 20 bar
    testParams.forceOvershoot = 1u;
    testParams.maxFakeLeakRate = screwParams.maxLeakRate * 0.5f;
    testParams.maxFakeLeakRate = 0.0f;
    // simulated leak rate, adjusted by current pressure and volume(mbar / iteration)
    testParams.fakeLeakRate = testParams.maxFakeLeakRate;
    // calibrate optical sensor if True
    testParams.isOpticalSensorCalibrationRequired = true;
    testParams.fakeLeak = 0.0f;  //# simulated cumulative leak effect(mbar)
    testParams.volumeForLeakRateAdjustment = 10.0f;  // fixed volume value used for leak rate adjustment(mL)
}

/**
* @brief    Initializes parameters used by centering of the motor at power up
* @param    void
* @retval   void
*/
void DController::initCenteringParams(void)
{
    totalSteps = 0;
}

/**
* @brief    Moves motor to the maximum end to the fully extended position
* @param    void
* @retval   uint32_t motorMax - 0 if not maxed, 1 if maxed
*/
uint32_t DController::moveMotorMax(void)
{
    uint32_t optMax = 1u;
    uint32_t motorMax = 0u;
    int32_t readSteps = 0;
    int32_t steps = 0;
    optMax = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

    if(0u == optMax)
    {
        /* Motor is now at one end stop, travel to the other end stop, while doing so, count total steps */
        steps = 0;
        PV624->stepperMotor->move(steps, &readSteps);
        calcDistanceTravelled(readSteps);
        motorMax = 1u;
    }

    else
    {
        steps = screwParams.maxStepSize;
        PV624->stepperMotor->move(steps, &readSteps);
        calcDistanceTravelled(readSteps);
        totalSteps = readSteps + totalSteps;
        readSteps = 0;
    }

    return motorMax;
}

/**
* @brief    Moves motor to the minimum end to the fully retracted position
* @param    void
* @retval   uint32_t motorMin - 0 if not at minimum position, 1 if at min position
*/
uint32_t DController::moveMotorMin(void)
{
    uint32_t optMin = 1u;
    uint32_t motorMin = 0u;
    int32_t readSteps = 0;
    int32_t steps = 0;
    optMin = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);

    if(0u == optMin)
    {
        /* Motor is now at one end stop, travel to the other end stop, while doing so, count total steps */
        steps = 0;
        PV624->stepperMotor->move(steps, &readSteps);
        calcDistanceTravelled(readSteps);
        readSteps = 0;
        motorMin = 1u;
    }

    else
    {
        steps = -1 * screwParams.maxStepSize;
        PV624->stepperMotor->move(steps, &readSteps);
        totalSteps = totalSteps - totalSteps;
        calcDistanceTravelled(readSteps);
    }

    return motorMin;
}

/**
* @brief    Moves motor to the center position
* @param    void
* @retval   uint32_t centered - 0 if not centered, 1 if centered
*/
uint32_t DController::moveMotorCenter(int32_t setSteps)
{
    uint32_t centered = 0u;
    int32_t readSteps = 0;
    int32_t steps = 0;

    if((totalSteps >= (screwParams.centerPositionCount - screwParams.centerTolerance)) &&
            (totalSteps <= (screwParams.centerPositionCount + screwParams.centerTolerance)))
    {
        steps = 0;
        PV624->stepperMotor->move(steps, &readSteps);
        calcDistanceTravelled(readSteps);
        pidParams.totalStepCount = totalSteps;
        readSteps = 0;
        centered = 1u;
    }

    else
    {
        steps = setSteps;
        PV624->stepperMotor->move(steps, &readSteps);
        calcDistanceTravelled(readSteps);
        totalSteps = readSteps + totalSteps;
    }

    return centered;
}

/**
* @brief    Centers the piston at power up from its current location by driving it to one end of the lead screw
            Then calculate the steps travelled. The centre location is available from the lead screw length and
            total steps to complete the screw length from screw parameters. After hitting one end of the screw, the
            motor drives the piston to the centre by counting steps at every cycle
* @param    void
* @retval   uint32_t centered - 0 if not centered, 1 if centered
*/
uint32_t DController::centreMotor(void)
{
    uint32_t optBoardAvailable = 1u;
    uint32_t centered = 0u;
    int32_t steps = 0; // set to max negative to acheive full speed
    int32_t readSteps = 0;
    uint32_t optMin = 1u;
    uint32_t optMax = 1u;

    /* Check whether optical board is installed, if not, motor and controller operations are not permitted */
    optBoardAvailable = HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_14);
    // TODO, if optical board is not available, control algorithm must not run

    if(1u == optBoardAvailable)
    {
    }

    switch(stateCentre)
    {
    case eCenteringStateNone:
        // Pend sema and start centering the motor
        stateCentre = eCenteringStateMin;
        break;

    case eCenteringStateMin:
        optMin = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);

        if(0u == optMin)
        {
            /* Motor is now at one end stop, travel to the other end stop, while doing so, count total steps */
            steps = 0;
            PV624->stepperMotor->move(steps, &readSteps);
            calcDistanceTravelled(readSteps);
            readSteps = 0;
            stateCentre = eCenterMotor;
        }

        else
        {
            steps = -1 * screwParams.maxStepSize;
            PV624->stepperMotor->move(steps, &readSteps);
            calcDistanceTravelled(readSteps);
        }

        break;

    case eCenteringStateMax:
        optMax = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

        if(0u == optMax)
        {
            /* Motor is now at one end stop, travel to the other end stop, while doing so, count total steps */
            steps = 0;
            PV624->stepperMotor->move(steps, &readSteps);
            calcDistanceTravelled(readSteps);
            stateCentre = eCenterMotor;
        }

        else
        {
            steps = screwParams.maxStepSize;
            PV624->stepperMotor->move(steps, &readSteps);
            calcDistanceTravelled(readSteps);
            totalSteps = readSteps + totalSteps;
            readSteps = 0;
        }

        break;

    case eCenterMotor:
        if((totalSteps >= (screwParams.centerPositionCount - screwParams.centerTolerance)) &&
                (totalSteps <= (screwParams.centerPositionCount + screwParams.centerTolerance)))
        {
            steps = 0;
            PV624->stepperMotor->move(steps, &readSteps);
            calcDistanceTravelled(readSteps);
            pidParams.totalStepCount = totalSteps;
            readSteps = 0;
            centered = 1u;
            stateCentre = eCenteringStateNone;
            totalSteps = 0;
        }

        else
        {
            steps = screwParams.maxStepSize;
            PV624->stepperMotor->move(steps, &readSteps);
            calcDistanceTravelled(readSteps);
            totalSteps = readSteps + totalSteps;
        }

        break;

    default:
        break;
    }

    return centered;
}

/**
* @brief    Reset the bayes estimation parameters, required at every set point completion or exit from fine control
            to make the these parameters available for the next set point
* @param    void
* @retval   void
*/
void DController::resetBayesParameters(void)
{
    bayesParams.changeInPressure = 0.0f;
    bayesParams.prevChangeInPressure = 0.0f;
    bayesParams.changeInVolume = 0.0f;
    bayesParams.prevChangeInVolume = 0.0f;
    bayesParams.targetdP = 0.0f;
    bayesParams.estimatedLeakRate = 0.0f;
    bayesParams.uncertaintyEstimatedLeakRate = bayesParams.uncertaintyPressureDiff;
    testParams.fakeLeak = 0.0f;
    bayesParams.measuredPressure = absolutePressure;
    bayesParams.smoothedPresure = absolutePressure;
    bayesParams.smoothedPressureErr = 0.0f;
    bayesParams.smoothedSquaredPressureErr = 0.0f;
    bayesParams.uncerInSmoothedMeasPresErr = 0.0f;
    bayesParams.smoothedPressureErrForPECorrection = 0.0f;
}

/**
* @brief    Returns the sign of a float variable
* @param    float32_t value of which sign needs to be returned
* @retval   float32_t sign 1.0 if positive, -1.0 if negative
*/
float32_t DController::getSign(float32_t value)
{
    // There is a better way to implement this, check the sign bit on the value, can remove floating comparison TODO
    float32_t sign = 0.0f;

    if(value > 0.0f)
    {
        sign = 1.0f;
    }

    else
    {
        sign = -1.0f;
    }

    return sign;
}

/**
* @brief    Run the controller in measure mode
* @param    void
* @retval   void
*/
void DController::setMeasure(void)
{
    // Isolate pump and close vent valve in measure mode
    // Close vent valve
    PV624->valve3->triggerValve(VALVE_STATE_OFF);
    // Re configure vent valve in TDM mode - could be required to to do controlled vent after initial pump up / down
    PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
    // Close outlet valve - isolate pump from generating vaccum
    PV624->valve1->triggerValve(VALVE_STATE_OFF);
    // Close inlet valve - isolate pump from generating pressure
    PV624->valve2->triggerValve(VALVE_STATE_OFF);

    // With new status variables
    pidParams.control = 0u;
    pidParams.measure = 1u;
    pidParams.venting = 0u;
    pidParams.controlRate = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.fastVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
    pidParams.pumpAttempts = 0u;
}

/**
* @brief    Set controller state and valves when pump up is required
* @param    void
* @retval   void
*/
void DController::setControlUp(void)
{
    // Set valves for pump up mode - open inlet valve to generate pressure
    // Close vent valve
    PV624->valve3->triggerValve(VALVE_STATE_OFF);
    // Re configure vent valve in TDM mode - could be required to to do controlled vent after initial pump up / down
    PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
    // Close outlet valve - isolate pump from generating vaccum
    PV624->valve1->triggerValve(VALVE_STATE_OFF);
    // Open inlet valve - connect pump to generate pressure
    PV624->valve2->triggerValve(VALVE_STATE_ON);

    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.controlRate = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 1u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.fastVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
    pidParams.pumpAttempts = pidParams.pumpAttempts + 1u;
}

/**
* @brief    Set controller state and valves when pump down is required
* @param    void
* @retval   void
*/
void DController::setControlDown(void)
{
    // Set valves for generating vacuum - open outlet valve
    // Close vent valve
    PV624->valve3->triggerValve(VALVE_STATE_OFF);
    // Re configure vent valve in TDM mode - could be required to to do controlled vent after initial pump up / down
    PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
    // Open outlet valve - connect pump to generate vacuum
    PV624->valve1->triggerValve(VALVE_STATE_ON);
    // Close inlet valve - isolate pump from generating pressure
    PV624->valve2->triggerValve(VALVE_STATE_OFF);

    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.controlRate = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 1u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.fastVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
    pidParams.pumpAttempts = pidParams.pumpAttempts + 1u;
}

/**
* @brief    Set controller state and valves when pump isolation is required, mostly in fine control
* @param    void
* @retval   void
*/
void DController::setControlIsolate(void)
{
    // Isolate pump and close vent valve in fine control mode
    // Close vent valve
    PV624->valve3->triggerValve(VALVE_STATE_OFF);
    // Re configure vent valve in TDM mode - could be required to to do controlled vent after initial pump up / down
    PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
    // Close outlet valve - isolate pump from generating vaccum
    PV624->valve1->triggerValve(VALVE_STATE_OFF);
    // Close inlet valve - isolate pump from generating pressure
    PV624->valve2->triggerValve(VALVE_STATE_OFF);

    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.controlRate = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.fastVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
}

/**
* @brief    Set controller state and valves when piston centering is required
* @param    void
* @retval   void
*/
void DController::setControlCentering(void)
{
    // Isolate pump and close vent valve if piston not centered, enable the centering flags
    // Close vent valve
    PV624->valve3->triggerValve(VALVE_STATE_OFF);
    // Re configure vent valve in TDM mode - could be required to to do controlled vent after initial pump up / down
    PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
    // Close outlet valve - isolate pump from generating vaccum
    PV624->valve1->triggerValve(VALVE_STATE_OFF);
    // Close inlet valve - isolate pump from generating pressure
    PV624->valve2->triggerValve(VALVE_STATE_OFF);

    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.controlRate = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 1u;
    pidParams.controlledVent = 0u;
    pidParams.fastVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
}

/**
* @brief    Run controller in vent mode
* @param    void
* @retval   void
*/
void DController::setVent(void)
{
    // Isolate pump in vent mode, open the vent valve in PWM mode to save power after venting is complete
    // Close vent valve
    PV624->valve1->triggerValve(VALVE_STATE_OFF);
    // Close outlet valve - isolate pump from generating vaccum
    PV624->valve2->triggerValve(VALVE_STATE_OFF);
    // Close inlet valve - isolate pump from generating pressure
    PV624->valve3->triggerValve(VALVE_STATE_OFF);
    // Set Duty cycle for the vent valve
    PV624->valve3->setValveTime(bayesParams.ventDutyCycle);
    // Set the vent valve into PWM drive mode
    PV624->valve3->reConfigValve(E_VALVE_MODE_PWMA);
    // Open vent valve at above set duty cycle
    PV624->valve3->triggerValve(VALVE_STATE_ON);

    // With new status variables
    pidParams.control = 0u;
    pidParams.measure = 0u;
    pidParams.venting = 1u;
    pidParams.controlRate = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.fastVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
    pidParams.pumpAttempts = 0u;
}

/**
* @brief    Run controller in controlled vent mode
* @param    void
* @retval   void
*/
void DController::setControlVent(void)
{
    // Run a controlled vent if reducing pressure or decreasing vacuum, run vent valve in TDM mode with increasing
    // pulse width for faster controlled vent action to reach desired pressure faster
    // Close vent valve
    PV624->valve1->triggerValve(VALVE_STATE_OFF);
    // Close outlet valve - isolate pump from generating vaccum
    PV624->valve2->triggerValve(VALVE_STATE_OFF);
    // Close inlet valve - isolate pump from generating pressure
    PV624->valve3->triggerValve(VALVE_STATE_OFF);
    // Re configure the vent valve to run in tdm mode - set at initial pulse width of 500us
    PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
    // Set duty cycle at initial pulse width
    bayesParams.ventDutyCycle = screwParams.minVentDutyCycle;
    // Set the pulse width time in timer
    PV624->valve3->setValveTime(bayesParams.ventDutyCycle);
    // Open vent valve
    PV624->valve3->triggerValve(VALVE_STATE_ON);

    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.controlRate = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 1u;
    pidParams.fastVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
}

/**
* @brief    Run controller in when fast vent mode
* @param    void
* @retval   void
*/
void DController::setFastVent(void)
{
    // Fast vent mode close all other valves and opens vent valve at high duty cycle to vent out the pressure in the
    // system as fast as possible
    // Close vent valve
    PV624->valve1->triggerValve(VALVE_STATE_OFF);
    // Close outlet valve - isolate pump from generating vaccum
    PV624->valve2->triggerValve(VALVE_STATE_OFF);
    // Close inlet valve - isolate pump from generating pressure
    PV624->valve3->triggerValve(VALVE_STATE_OFF);
    // Set the vent valve to operate at max duty cycle for venting the pressure as fast as possible
    bayesParams.ventDutyCycle = screwParams.maxVentDutyCyclePwm;
    // Sets the timer to generate a high duty cycle PWM
    PV624->valve3->setValveTime(bayesParams.ventDutyCycle);
    // Set the mode of the valve to PWM
    PV624->valve3->reConfigValve(E_VALVE_MODE_PWMA);
    // Open vent valve
    PV624->valve3->triggerValve(VALVE_STATE_ON);

    // With new status variables
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.controlledVent = 0u;
    pidParams.fastVent = 1u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
}

/**
* @brief    Run controller in when control rate mode
* @param    void
* @retval   void
*/
void DController::setControlRate(void)
{
    // Rate control mode will be used for switch testing where pressure needs to be reduced at a fixed rate
    // in mbar / iteration - this will be useful when catching the switch point accurately by the DPI620G
    // Close vent valve
    PV624->valve1->triggerValve(VALVE_STATE_OFF);
    // Close outlet valve - isolate pump from generating vaccum
    PV624->valve2->triggerValve(VALVE_STATE_OFF);
    // Close inlet valve - isolate pump from generating pressure
    PV624->valve3->triggerValve(VALVE_STATE_OFF);
    // Re configure the vent valve in tdm mode - changing the pulse width can adjust rate of change of pressure
    PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
    // Set to minimum pulse width of 500 us
    bayesParams.ventDutyCycle = screwParams.minVentDutyCycle;
    // Set the timer to generate pulse
    PV624->valve3->setValveTime(bayesParams.ventDutyCycle);
    // Open vent valve
    PV624->valve3->triggerValve(VALVE_STATE_ON);

    // With new status variables
    pidParams.control = 0u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.controlRate = 1u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.fastVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
    pidParams.pumpAttempts = 0u;
}

/**
* @brief    Calculate distance travelled by controller
* @param    void
* @retval   void
*/
void DController::calcDistanceTravelled(int32_t stepsMoved)
{
    float32_t absSteps = 0.0f;

    absSteps = (float32_t)(stepsMoved);
    absSteps = fabs(absSteps);
    screwParams.distanceTravelled = screwParams.distanceTravelled +
                                    (absSteps * screwParams.distancePerStep);
}

/**
* @brief    Pulse the vent value at the set venting duty cycle
* @param    void
* @retval   void
*/
void DController::pulseVent(void)
{
    PV624->valve3->setValveTime(bayesParams.ventDutyCycle);
    PV624->valve3->triggerValve(VALVE_STATE_ON);
}

// Mention reason for suppressing the misra rules TODO
#pragma diag_suppress=Pm128 /* Disable MISRA C 2004 rule 10.1 */
#pragma diag_suppress=Pm136 /* Disable MISRA C 2004 rule 10.4 */
#pragma diag_suppress=Pm137 /* Disable MISRA C 2004 rule 10.4 */
#pragma diag_suppress=Pm046 /* Disable MISRA C 2004 rule 13.3*/
/**
* @brief    Bayes estimation for estimating leak rate and volume connected to the customer port
            Uses different estimation techniques to accurately estimate these parameters
* @param    void
* @retval   void
*/
void DController::estimate(void)
{
    /*
    Estimate volume and leak rate using Bayesian conditional probability to merge new measurement
    data with previous parameter estimates(aka extended Kalman filter).
    Added empirical predictionError adjustment to volumeand leak estimates for smaller / slower corrections where gas
    law estimation methods are noisy.
    Jun 10 2021
    updated to use with fineControl.py and coarseControl.py modules
    improved comment clarity and code flow

    defaults when measV cannot be calculated
    */
    float32_t residualL = 0.0f;

    bayesParams.measuredVolume = 0.0f;
    bayesParams.uncertaintyMeasuredVolume = bayesParams.maxSysVolumeEstimate;
    bayesParams.algorithmType = eMethodNone;

    /*
    if PID['controlledVent'] == 1 and bayes['ventIterations'] > screw['ventDelay'] and \
    bayes['initialP'] / bayes['finalP'] > 1:
    */

    if(1u == pidParams.fineControl)
    {
        /*
        do this in fine control mode only
        calculate prediction errorand error type from last control iteration
        prediction error(mbar)
        */
        bayesParams.predictionError = bayesParams.changeInPressure - bayesParams.targetdP;
        /*
        predictionErrorType > 0 -- > excessive correction, reduce system gain to fix(decrease volume estimate)
        predictionErrorType < 0 --> insufficient correction, increase system gain to fix(increase volume estimate)
        bayesParams.predictionErrType = np.sign(bayes['targetdP']) * np.sign(bayes['predictionError'])
        */
        float32_t signTargetDp = 0.0f;
        float32_t signPredictionError = 0.0f;

        signTargetDp = getSign(bayesParams.targetdP);
        signPredictionError = getSign(bayesParams.predictionError);
        bayesParams.predictionErrType = (int32_t)(signTargetDp * signPredictionError);
        // low pass filtered pressure error(mbar)
        bayesParams.smoothedPressureErr = pidParams.pressureError * bayesParams.lambda +
                                          bayesParams.smoothedPressureErr * (1.0f - bayesParams.lambda);

        /*
        # correction to varE estimate Nov 18 2021
        # low pass filtered squared error (mbar**2)
        # (not used)
        # bayes['smoothE2'] = (PID['E']**2) * bayes['lambda'] + bayes['smoothE2'] * (1-bayes['lambda'])
        # dynamic estimate of pressure error variance
        # see https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        # http://cpsc.yale.edu/sites/default/files/files/tr222.pdf
        # Chan, Tony F.; Golub, Gene H.; LeVeque, Randall J. (1983).
        # "Algorithms for computing the sample variance: Analysis and recommendations" (PDF).
        # The American Statistician. 37 (3): 242–247. doi:10.1080/00031305.1983.10483115. JSTOR 2683386.
        # Note: varE estimate not valid when smoothE2 >> varE
        #bayes['varE'] = bayes['smoothE2'] - bayes['smoothE']**2
        # Use Expontially Weighted Moving variance estimate
        # https://en.wikipedia.org/wiki/Moving_average#cite_note-13
        # Finch, Tony. "Incremental calculation of weighted mean and variance" (PDF).
        # University of Cambridge. Retrieved 19 December 2019.
        */

        float32_t pressureErrTemp = 0.0f;
        float32_t pressureErrorSquareTemp = 0.0f;
        float32_t lambdaTemp = 0.0f;

        pressureErrTemp = (pidParams.pressureError - bayesParams.smoothedPressureErr);
        pressureErrorSquareTemp = pressureErrTemp * pressureErrTemp;
        lambdaTemp = (1.0f - bayesParams.lambda);

        // bayes['varE'] = (PID['E'] - bayes['smoothE'])**2 * bayes['lambda'] + bayes['varE'] * (1 - bayes['lambda'])

        bayesParams.uncerInSmoothedMeasPresErr = (pressureErrorSquareTemp * bayesParams.lambda) +
                (bayesParams.uncerInSmoothedMeasPresErr * lambdaTemp);

        /*
        decide if pressure is stable, based on smoothed variance of pressure readings
        if bayes['varE'] * *0.5 < 5 * bayes['varP'] * *0.5 and bayes['smoothE'] < 5 * bayes['varP'] * *0.5:
        */

        /* Update V6 - bayes['varE']**0.5 < 3 * bayes['varP']**0.5 and bayes['smoothE'] < 3 * bayes['varP']**0.5: */
        float32_t sqrtUncertaintySmoothedPresErr = 0.0f;
        float32_t sqrtSensorUncertainty = 0.0f;
        float32_t tempSensorUncertainty = 0.0f;

        sqrtUncertaintySmoothedPresErr = sqrt(bayesParams.uncerInSmoothedMeasPresErr);
        sqrtSensorUncertainty = sqrt(bayesParams.sensorUncertainity);
        tempSensorUncertainty = 3.0f * sqrtSensorUncertainty;

        if((sqrtUncertaintySmoothedPresErr < tempSensorUncertainty) &&
                (bayesParams.smoothedPressureErr < tempSensorUncertainty))
        {
            pidParams.stable = 1u;
        }

        else
        {
            pidParams.stable = 0u;
        }

        /*
        Step(1)
        Estimate volume and leak rate from isothermal gas law
        Vmeas = -P * dV / (dP - Lmeas)
        Three methods : regression, single point, or prediction error nudge
        */

        /*
        Regression calc :
        Observe that Vmeas is the equation of a line : y = m * x + b
        with y == dP, x == dV, slope m == -P / Vmeas and intercept b == Lmeas
        A unique solution for both(Vmeas, Lmeas) from gas law regression
        requires a minimum of two measurements(dP, dV) at constant(P, Lmeas, Vmeas)

        Single Point calc :
        For single - point method assume Lmeas = 0 and calculate Vmeas from a single control iteration.

        Prediction Error calc :
        Use prediction error, smoothed prediction error, and prediction error type
        to nudge volumeand leak estimates instead of gas law calculation.
        This is a recursive correction method than can take many control iterations to converge to correct
        parameter estimates if the initial estimation error is large,
        hence use of the other two methods for initial estimates
        when relatively far from setpoint.

        difference in pressure change from previous pressure change(mbar)
        */
        float32_t dP2, dV2;
        dP2 = bayesParams.changeInPressure - bayesParams.prevChangeInPressure;
        bayesParams.dP2 = dP2;

        // difference in volume change from previous volume change(mL)
        dV2 = bayesParams.changeInVolume - bayesParams.prevChangeInVolume; // bayes['dV'] - bayes['dV_']
        bayesParams.dV2 = dV2;

        /*
        adjust previous volume estimate by most recent dV
        bayes['V'] = bayes['V'] + bayes['dV']
        */
        bayesParams.estimatedVolume = bayesParams.estimatedVolume + bayesParams.changeInVolume;
        /*
        uncertainty of V before combining with new measured value
        bayes['varV'] = bayes['varV'] + bayes['vardV']
        */
        bayesParams.uncertaintyVolumeEstimate = bayesParams.uncertaintyVolumeEstimate +
                                                bayesParams.uncertaintyVolumeChange;

        //if abs(dV2) > 10 * bayes['vardV'] * *0.5 and abs(dP2) > bayes['vardP'] * *0.5 and PID['fineControl'] == 1:
        float32_t fabsDv2 = 0.0f;
        float32_t fabsDp2 = 0.0f;
        float32_t sqrtUncertaintyVolChange = 0.0f;
        float32_t tempUncertaintyVolChange = 0.0f;
        float32_t sqrtUncertaintyPressDiff = 0.0f;
        float32_t fabsChangeInVol = fabs(bayesParams.changeInVolume);
        float32_t sqrtUncertaintyVolumeChange = sqrt(bayesParams.uncertaintyVolumeChange);
        float32_t tempUncertaintyVolumeChange = 10.0f * sqrtUncertaintyVolumeChange;
        float32_t fabsChangeInPress = fabs(bayesParams.changeInPressure);

        fabsDv2 = fabs(dV2);
        fabsDp2 = fabs(dP2);

        sqrtUncertaintyVolChange = sqrt(bayesParams.uncertaintyVolumeChange);
        tempUncertaintyVolChange = 10.0f * sqrtUncertaintyVolChange;
        sqrtUncertaintyPressDiff = sqrt(bayesParams.uncertaintyPressureDiff);

        //bayes['algoV'] = 1
        if((fabsDv2 > tempUncertaintyVolChange) &&
                (fabsDp2 > sqrtUncertaintyPressDiff) &&
                (1u == pidParams.fineControl))
        {
            bayesParams.algorithmType = eRegressionMethod;

            /*
            print('*')
            Estimate volume with linear fit to gas law equation.
            Done only during fine control where
            the two most recent measurements(dP, dV) and (dP_, dV_)
            are significantly different from each other so that
            slope and intercept of a regression line are uniquely determined.
            This method tends to underestimate volume if dV2 is small
            or at low pressures because of non - linear noise - shaping near the gas - law equation singularity.
            Unclear how to improve it, hence restricting method to "large" dV2 only, where the
            method gives more accurate estimates.
            Empirical results indicate regression method leak estimate is
            very noisy compared to the volume estimate.
            Unclear why, possibly because leak parameter effect is less observable in short - term data,
            and so cannot be accurately estimated from only two consecutive measurements.Adiabatic
            decay also looks like a large, time - varying leak,
            which would lead to an inaccurate estimate of the steady - state leak rate.
            The long - term effect of an inaccurate leak estimate can be substantial and take a long time to decay to
            zero via the prediction error adjustment method.It is therefore safer to assume the leak rate is zero and
            adjust it up rather than adjust a large inaccurate value down to zero.If the leak rate is truly a large
            value then the zero - assumption will cause longer settling time, but for a good reason
            (there is large leak).
            Hence, ignore the regression method leak estimateand just use the volume estimate.This minimizes the
            effect of transient adiabatic effects or large leaks on volume estimation error.
            Steady - state leak estimation done by another method(filtered prediction error) in Step 2.

            measured volume estimate with linear fit(slope), (mL)
            */

            //bayes['measV'] = -bayes['P'] * (dV2 / dP2)
            bayesParams.measuredVolume = -bayesParams.measuredPressure * (dV2 / dP2);

            /*
            # uncertainty in measured volume estimate with Bayes regression(mL)
            bayes['varMeasV'] = bayes['varP'] * (dV2 / dP2) * *2 + \
            2 * bayes['vardP'] * (bayes['P'] * dV2 / dP2 * *2) * *2 + \
            2 * bayes['vardV'] * (bayes['P'] / dP2) * *2
            */

            float32_t temporaryVariable1 = 0.0f;
            float32_t temporaryVariable2 = 0.0f;
            float32_t temporaryVariable3 = 0.0f;

            /* Updated by mak on 13/08/2021 */
            temporaryVariable1 = dV2 / dP2;
            temporaryVariable1 = temporaryVariable1 * temporaryVariable1;
            temporaryVariable1 = bayesParams.sensorUncertainity * temporaryVariable1;

            temporaryVariable2 = dP2 * dP2;
            temporaryVariable2 = dV2 / temporaryVariable2;
            temporaryVariable2 = bayesParams.measuredPressure * temporaryVariable2;
            temporaryVariable2 = temporaryVariable2 * temporaryVariable2;
            temporaryVariable2 = bayesParams.uncertaintyPressureDiff * temporaryVariable2;
            temporaryVariable2 = 2.0f * temporaryVariable2;

            temporaryVariable3 = bayesParams.measuredPressure / dP2;
            temporaryVariable3 = temporaryVariable3 * temporaryVariable3;
            temporaryVariable3 = bayesParams.uncertaintyVolumeChange * temporaryVariable3;
            temporaryVariable3 = 2.0f * temporaryVariable3;

            bayesParams.uncertaintyMeasuredVolume = temporaryVariable1 + temporaryVariable2 + temporaryVariable3;
        }

        //elif abs(bayes['dV']) > 10 * bayes['vardV'] * *0.5 and abs(bayes['dP']) > bayes['vardP'] * *0.5:
        else if((fabsChangeInVol > tempUncertaintyVolumeChange) &&
                (fabsChangeInPress > sqrtUncertaintyPressDiff))
            /* commented to remove misra error, replaced by ELSE IF above
            else if ((fabs(bayesParams.changeInVolume) > (10.0f * sqrt((double)bayesParams.uncertaintyVolumeChange)))
                            && (fabs((double)bayesParams.changeInPressure) > sqrt(bayesParams.uncertaintyPressureDiff)))
            */
        {
            bayesParams.algorithmType = eGasLawMethod; //bayes['algoV'] = 2
            /*
            # print('**')
            # Estimate volume with gas law equation, assuming leak / adiabatic effect is zero.
            # Done when far from setpoint and during coarse control piston centering.
            # Valid when volume adjustment is large compared to leak rate effect or adiabatic,
            # e.g.when moving at max speed towards setpoint.
            # This estimate is has better SNR than other methods because dPand dV are large,
            # but dP will be a time - averaged value over the
            # duration of dV adjustment, rather than the final value(integration error).
            # If the applied dV is repeated(i.e.motor rotating at max speed) then
            # the error from pressure sensor integration time is greatly reduced
            # because lag - error in previous measured dP cancels in the following one when it is the same value
            # as before.
            # The range estimate is most accurate in this case, which allows for
            # sensible decisions on when a setpoint is achievable long before reaching
            # the maximum extent of piston travel.The method is also useful when a linear fit is not possible
            # because of repeated dPand dV.
            # Since the leak / adiabatic is assumed to be zero, this method
            # can yield inaccurate volume estimates when the transient adiabatic pressure change is large compared
            # to the isothermal pressure change.However, on final approach to setpoint adiabatic effect
            # should be addressed by the regression calculation method that adjusts measured volume for leakand
            # adiabatic effects.

            # measured volume(mL), ignore negative volume estimates
            # negative values can result from a large adiabatic or
            # leak rate effects when(dV, dP) are relatively small
            */
            //bayes['measV'] = max(-bayes['P'] * (bayes['dV'] / bayes['dP']), 0)
            bayesParams.measuredVolume = (float32_t)(fmax((-1.0f) * bayesParams.measuredPressure *
                                         (bayesParams.changeInVolume / bayesParams.changeInPressure), 0.0));
            // uncertainty in measured volume(mL)
            // bayes['varMeasV'] = bayes['varP'] * (bayes['dV'] / bayes['dP']) * *2 + \
            // bayes['vardP'] * (bayes['P'] * bayes['dV'] / bayes['dP'] * *2) * *2 + \
            // bayes['vardV'] * (bayes['P'] / bayes['dP']) * *2
            float32_t temporaryVariable1 = 0.0f;
            float32_t temporaryVariable2 = 0.0f;
            float32_t temporaryVariable3 = 0.0f;

            /* Modified by mak on 12/08/2021*/
            temporaryVariable1 = bayesParams.changeInVolume / bayesParams.changeInPressure;
            temporaryVariable1 = temporaryVariable1 * temporaryVariable1;
            temporaryVariable1 = temporaryVariable1 * bayesParams.sensorUncertainity;
            temporaryVariable2 = bayesParams.changeInPressure * bayesParams.changeInPressure;
            temporaryVariable2 = bayesParams.measuredPressure * bayesParams.changeInVolume / temporaryVariable2;
            temporaryVariable2 = temporaryVariable2 * temporaryVariable2;
            temporaryVariable2 = bayesParams.uncertaintyPressureDiff * temporaryVariable2;
            temporaryVariable3 = bayesParams.measuredPressure / bayesParams.changeInPressure;
            temporaryVariable3 = temporaryVariable3 * temporaryVariable3;
            temporaryVariable3 = bayesParams.uncertaintyVolumeChange * temporaryVariable3;

            bayesParams.uncertaintyMeasuredVolume = temporaryVariable1 + temporaryVariable2 + temporaryVariable3;
        }

        else if(1u == pidParams.fineControl)
        {
            // use Prediction Error method to slowly adjust estimates of volume and leak rate
            // during fine control loop when close to setpoint
            bayesParams.algorithmType = ePredictionErrorMethod; //bayes['algoV'] = 3

            // measured residual leak rate from steady - state pressure error(mbar / iteration)
            float32_t measL = 0.0f;
            //residualL = ((-1.0f) * bayesParams.smoothedPressureErrForPECorrection) / bayesParams.estimatedKp;
            // bayes['residualL'] = -bayes['smoothE_PE'] / bayes['n']  # fixed error in calc
            residualL = ((-1.0f) * bayesParams.smoothedPressureErrForPECorrection) /
                        bayesParams.numberOfControlIterations;
            bayesParams.residualLeakRate = residualL;

            // estimate of true leak rate magnitude(mbar / iteration)
            measL = fabs(residualL + bayesParams.estimatedLeakRate);
            bayesParams.measuredLeakRate1 = measL;

            if(1 == bayesParams.predictionErrType)
            {
                /*
                # Excessive gain, decrease Vand increase leak estimates to keep same net leak rate control effect.
                # Decrease volume estimate only when steady state pressure errorand leak have the same sign
                #and the measured error in leak rate estimate is less than + / -50 %
                # Otherwise keep volume estimate as is until leak estimate is closer to steady state value
                # to prevent underestimating volume when leak is inaccurate(can cause sluggish control).
                # The effect of a leak on PE will grow if a small dV correction has been skipped, e.g.in
                # low - power mode, so do not make volume estimate corrections unless the
                # the control has applied a correction at least twice consecutively(dV > 0 and dV_ > 0)
                */
                float32_t tempResidualL = residualL * bayesParams.estimatedLeakRate;
                float32_t tempEstLeakRate = 2.0f * fabs(bayesParams.estimatedLeakRate);

                if((tempResidualL >= 0.0f) &&
                        (measL < tempEstLeakRate) &&
                        (bayesParams.changeInVolume != 0.0f) &&
                        (bayesParams.prevChangeInVolume != 0.0f))
                    /* Commented for MISRA, replaced by above if
                    if (((residualL * bayesParams.estimatedLeakRate) >= 0.0f) &&
                        (measL < (2.0f * fabs(bayesParams.estimatedLeakRate))) &&
                        (bayesParams.changeInVolume != 0.0f) &&
                        (bayesParams.changeInVolume != 0.0f))
                    */
                {
                    //bayes['measV'] = bayes['V'] * bayes['gamma']
                    bayesParams.measuredVolume = bayesParams.estimatedVolume * bayesParams.gamma;
                    //bayes['varMeasV'] = bayes['varV']
                    bayesParams.uncertaintyMeasuredVolume = bayesParams.uncertaintyVolumeEstimate;
                    //bayes['leak'] = bayes['leak'] / bayes['gamma']
                    bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate / bayesParams.gamma;
                }

                //# print('***-')
            }

            if(-1 == bayesParams.predictionErrType)
            {
                /*
                # Insufficient gain, increase Vand decrease leak estimates to keep same net leak rate control effect.
                # Increase volume estimate only when steady state pressure errorand leak have the same sign
                #and the measured error in leak rate estimate is less than + / -50 %
                # Otherwise keep volume estimate as is until L estimate is closer to steady state value
                # to prevent overestimating volume when leak is inaccurate(can cause control oscillation).
                # The effect of a leak on PE will be amplified if no correction has been made(leak is not being fed,
                    # low - power mode), so do not make volume estimate corrections unless the
                # the control has applied a correction at least twice in a row(dV > 0 and dV_ > 0)
                */
                float32_t tempResidualL = residualL * bayesParams.estimatedLeakRate;
                float32_t tempEstLeakRate = 2.0f * fabs(bayesParams.estimatedLeakRate);

                // Insufficient gain, increase V and decrease leak estimates to keep same net leak rate control effect.
                // Increase volume estimate only when steady state pressure error and leak have the same sign
                // and the measured error in leak rate estimate is less than +/- 50%
                // Otherwise keep volume estimate as is until L estimate is closer to steady state value
                // to prevent overestimating volume when leak is inaccurate (can cause control oscillation).
                // The effect of a leak on PE will be amplified if no correction has been made (leak is not being fed,
                // low-power mode), so do not make volume estimate corrections unless the
                // the control has applied a correction at least twice in a row (dV >0 and dV_ > 0)
                // May 27 2002 added condition reduce estimatedVolume if oscillation is detected
                int32_t signChangeInVolume = 0;
                int32_t signPrevChangeInVolume = 0;
                float32_t absPresError = 0.0f;
                float32_t tempUncertainty = 0.0f;

                signChangeInVolume = (int32_t)(getSign(bayesParams.changeInVolume));
                signPrevChangeInVolume = (int32_t)(getSign(bayesParams.prevChangeInVolume));
                absPresError = fabs(pidParams.pressureError);
                tempUncertainty = sqrt(bayesParams.sensorUncertainity);
                tempUncertainty = tempUncertainty * 2.0f;

                if((signChangeInVolume != signPrevChangeInVolume) &&
                        (absPresError > tempUncertainty))
                {
                    bayesParams.measuredVolume = bayesParams.estimatedVolume * (bayesParams.gamma * bayesParams.gamma);
                    bayesParams.uncertaintyMeasuredVolume = bayesParams.uncertaintyVolumeEstimate;
                    bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate *
                                                    (bayesParams.gamma * bayesParams.gamma);
                }


                else if((tempResidualL >= 0.0f) &&
                        (measL < tempEstLeakRate) &&
                        (bayesParams.changeInVolume != 0.0f) &&
                        (bayesParams.prevChangeInVolume != 0.0f))
                    /* Commented for MISRA, replaced by above if
                    if (((residualL * bayesParams.estimatedLeakRate) >= 0.0f) &&
                        (measL < (2.0f * fabs(bayesParams.estimatedLeakRate))) &&
                        (bayesParams.changeInVolume != 0.0f) &&
                        (bayesParams.prevChangeInVolume != 0.0f))
                    */
                {
                    //bayes['measV'] = bayes['V'] / bayes['gamma']
                    bayesParams.measuredVolume = bayesParams.estimatedVolume / bayesParams.gamma;
                    //bayes['varMeasV'] = bayes['varV']
                    bayesParams.uncertaintyMeasuredVolume = bayesParams.uncertaintyVolumeEstimate;
                    //bayes['leak'] = bayes['leak'] * bayes['gamma']
                    bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate * bayesParams.gamma;
                }

                else
                {
                    /* Completing statements for MISRA */
                }

            }
        }

        else
        {
            /* Required for MISRA */
        }

        // combine prior volume estimate with latest measured volume, if available
        if(bayesParams.measuredVolume != 0.0f)
        {
            /*
            probability weighted average of prior and measured values(mL)
            bayes['V'] = (bayes['measV'] * bayes['varV'] + bayes['V'] * bayes['varMeasV']) /
            (bayes['varMeasV'] + bayes['varV'])
            */
            bayesParams.estimatedVolume = ((bayesParams.measuredVolume * bayesParams.uncertaintyVolumeEstimate) +
                                           (bayesParams.estimatedVolume * bayesParams.uncertaintyMeasuredVolume)) /
                                          (bayesParams.uncertaintyMeasuredVolume +
                                           bayesParams.uncertaintyVolumeEstimate);

            /*
            new uncertainty in estimated volume(mL)
            bayes['varV'] = bayes['varMeasV'] * bayes['varV'] / (bayes['varMeasV'] + bayes['varV'])
            */
            bayesParams.uncertaintyVolumeEstimate = (bayesParams.uncertaintyMeasuredVolume *
                                                    bayesParams.uncertaintyVolumeEstimate) /
                                                    (bayesParams.uncertaintyMeasuredVolume +
                                                            bayesParams.uncertaintyVolumeEstimate);
            /*
            increase varV if measV is not within maxZScore standard deviations of V
            increases convergence rate to true value in the event of a step change in system volume
            */
            float32_t du = 0.0f;
            //du = abs(bayes['V'] - bayes['measV'])
            du = fabs(bayesParams.estimatedVolume - bayesParams.measuredVolume);

            //bayes['varV'] = max(bayes['varV'], du / bayes['maxZScore'] - bayes['varMeasV'])
            bayesParams.uncertaintyVolumeEstimate = fmax(bayesParams.uncertaintyVolumeEstimate, du /
                                                    (bayesParams.maxZScore -
                                                            bayesParams.uncertaintyMeasuredVolume));
            /*
            bound updated volume estimate
            bayes['V'] = max(min(bayes['V'], bayes['maxV']), bayes['minV'])
            */
            bayesParams.estimatedVolume = fmax(fmin(bayesParams.estimatedVolume,
                                                    bayesParams.maxSysVolumeEstimate),
                                               bayesParams.minSysVolumeEstimate);
        }

        // Step 2
        // Adjust leak rate estimate
        // if bayes['algoV'] == 3:
        if(ePredictionErrorMethod == bayesParams.algorithmType)
        {
            /*
            Use dynamically averaged pressure error when near setpoint to adjust leak rate.
            Leak rate averaging window length is inversely proportional to the
            larger of the estimated leak rate or pressure error.
            This makes the filter react faster when either the pressure error or
            leak rate estimate is "large" to improve convergence rate to true values,
            while providing accurate leak rate correction for small leaks.
            */
            // limit filter length to maxN control iterations
            float32_t minError = 0.0f;
            float32_t maxError = 0.0f;
            uint32_t numOfIterations = 0u;
            // minError = bayes['vardP'] * *0.5 / bayes['maxN']
            minError = sqrt(bayesParams.uncertaintyPressureDiff) / ((float32_t)bayesParams.maxIterationsForIIRfilter);

            /*
            use largest of pressure error or leak rate estimate to define IIR filter length
            larger values means shorter observation time / faster response
            maxError = max(abs(bayes['smoothE_PE']), abs(bayes['leak']), minError)
            */
            maxError = fmax(fmax(fabs(bayesParams.smoothedPressureErrForPECorrection),
                                 fabs(bayesParams.estimatedLeakRate)), minError);
            /*
            number of control iterations to average over(minN, maxN)
            n = max(int(bayes['vardP'] * *0.5 / maxError), bayes['minN'])
            */
            numOfIterations = (uint32_t)(max((sqrt(bayesParams.uncertaintyPressureDiff) / maxError),
                                             (float32_t)(bayesParams.minIterationsForIIRfilter)));
            bayesParams.numberOfControlIterations = numOfIterations;
            /*
            Average E over more iterations when leak rate estimate is small and averaged pressure error is small
            so that the expected value of the pressure error from a leak
            is at least as big as pressure measurement noise.
            IIR filter memory factor to achieve epsilon * initial condition residual after n iterations
            bayes['alpha'] = 10 * *(bayes['log10epsilon'] / n)
            */
            bayesParams.alpha = (float32_t)(pow(10, (bayesParams.log10epsilon / (float32_t)(numOfIterations))));
            /*
            smoothed pressure error with dynamic IIR filter
            bayes['smoothE_PE'] = (1 - bayes['alpha']) * PID['E'] + bayes['alpha'] * bayes['smoothE_PE']
            */
            bayesParams.smoothedPressureErrForPECorrection = (1.0f - bayesParams.alpha) * pidParams.pressureError +
                    (bayesParams.alpha *
                     bayesParams.smoothedPressureErrForPECorrection);
            /*
            measured residual leak rate from steady - state pressure error(mbar / iteration)
            residualL = -bayes['smoothE_PE'] / bayes['kP']
            */
            //residualL = (-1.0f) * bayesParams.smoothedPressureErrForPECorrection / bayesParams.estimatedKp;
            residualL = (-1.0f) * bayesParams.smoothedPressureErrForPECorrection /
                        bayesParams.numberOfControlIterations;
            bayesParams.residualLeakRate = residualL;
            /*
            Apply 1 / n of total correction to reduce remaining leak error gradually(mbar / iteration)
            Correction to leak estimate must be gradual because leak effect takes time to accurately measure.
            Applying entire dL correction to leak rate after
            each control iteration could cause sustained oscillation in pressure error,
            similar to "integrator windup" in a PI controller when the "kI" term is too large.
            This controller's correction for leak rate is a feed-forward compensation with gain kP and
            leak rate estimation error inferred from smoothed pressure error.
            At steady - state, feed - forward control allows the controller to anticipate the effect
            of the leak before it accumulates as a significant pressure error, using historical leak disturbance
            to compensate for future effects.This is particularly helpful for minimizing
            steady - state pressure error for leak rates greater than the measurement
            noise when the iteration rate of the control loop is slow.

            bayes['dL'] = residualL / n
            */
            bayesParams.changeToEstimatedLeakRate = (residualL / (float32_t)numOfIterations);

            // Special cases:
            if(residualL * bayesParams.estimatedLeakRate < 0.0f)
            {
                /*
                 Residual pressure error is opposite sign to estimated leak rate
                 It could take a long time to correct this estimation error while the
                 pressure error continues to accumulate.
                 Speed - up convergence to the correct leak rate value by removing all dL in one iteration
                 This may temporarily cause the pressure error to
                 oscillate, but reduces the maximum error
                 that would otherwise accumulate.
                 This situation could happen if the true leak rate is suddenly changed
                 while hovering around setpoint, e.g.by user stopping the leak.
                 */
                bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate +
                                                (bayesParams.changeToEstimatedLeakRate *
                                                 (float32_t)(numOfIterations));
            }

            else
            {
                /*
                Residual pressure error is the same sign as estimated leak.
                Adjust leak rate estimate a small amount to minimize its effect

                bayes['leak'] = bayes['leak'] + bayes['dL']
                */
                bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate + bayesParams.changeToEstimatedLeakRate;
                /*
                Note: If dV == 0 because of low - power mode control operation then the leak rate correction will grow
                larger than the "correct" value because the leak effect is not being offset with feed - forward control.
                Prolonged operation with non - zero leak rate and dV == 0 may cause leak estimate to increase
                above true value.This is ok because effectively the control loop iteration time is increased as well,
                and so the leak rate per control iteration is truly increasing.However, if there is a disturbance
                the controller will revert back to a faster iteration rate, which may cause temporary oscillation while
                the leak rate estimate is nudged back down.

                varLeak the variance of low - pass filtered pressure sensor noise,
                which decreases as 1 / n for large n(Poisson statistics)
                n is the low - pass filter observation time(iterations)

                bayes['varLeak'] = (bayes['vardP'] / n)
                */
                bayesParams.uncertaintyEstimatedLeakRate = bayesParams.uncertaintyPressureDiff /
                        (float32_t)(numOfIterations);

                /*
                increase leak uncertainty if measLeak is not within maxZScore standard deviations of leak
                increases convergence to true value in the event of a step change in leak rate
                du = fabs(bayes['leak'] - bayes['measLeak'])

                bayes['varLeak'] = max(bayes['varLeak'], du / bayes['maxZScore'] - bayes['varMeasLeak'])
                */
                float32_t du = fabs(bayesParams.estimatedLeakRate - bayesParams.measuredLeakRate);
                bayesParams.uncertaintyEstimatedLeakRate = max(bayesParams.uncertaintyEstimatedLeakRate, du
                        / (bayesParams.maxZScore -
                           bayesParams.uncertaintyMeasuredLeakRate));
            }
        }

        else
        {

        }

        /*
        volume estimated with gas law
        keep previous leak estimate
        estimatedLeakRate = //bayes['leak'] = bayes['leak']
        bayes['varLeak'] = bayes['varLeak']

        bound leak correction to reasonable values, scaled by volume estimate and pressure
        higher pressure and lower volume can have larger leak rate estimate
        bayes['maxLeak'] = screw['maxLeak'] / bayes['V'] * bayes['P'] / screw['maxP'] * screw['nominalV']
        (screw['maxLeak'] / bayes['V'])* (bayes['P'] / screw['maxP'])* screw['nominalV']
        */
        bayesParams.maxEstimatedLeakRate = (screwParams.maxLeakRate / bayesParams.estimatedVolume) *
                                           (bayesParams.measuredPressure / screwParams.maxAllowedPressure) *
                                           screwParams.nominalTotalVolume;
        // bayes['leak'] = min(max(bayes['leak'], -bayes['maxLeak']), bayes['maxLeak'])
        bayesParams.estimatedLeakRate = min(max(bayesParams.estimatedLeakRate,
                                                (-1.0f * bayesParams.maxEstimatedLeakRate)),
                                            bayesParams.maxEstimatedLeakRate);
        /*
        Calculate optimal controller gain for next control iteration
        optimal kP to achieve correction of pressure error in one step(steps / mbar)

        bayes['kP'] = bayes['V'] / (bayes['P'] * screw['dV'])
        */
        bayesParams.estimatedKp = bayesParams.estimatedVolume / (bayesParams.measuredPressure *
                                  screwParams.changeInVolumePerPulse);
    }
}

/**
* @brief    Get the pressure value based on the type of pressure set point being used
            i.e. gaugePressure or absolute pressure
* @param    void
* @retval   void
*/
float32_t DController::pressureAsPerSetPointType(void)
{
    float32_t pressureValue = 0.0f;

    if((eSetPointType_t)eGauge == setPointType)
    {
        pressureValue = gaugePressure;
    }

    else if((eSetPointType_t)eAbsolute == setPointType)
    {
        pressureValue = absolutePressure;
    }

    else
    {
        pressureValue = atmosphericPressure;
    }

    return pressureValue;
}


/**
 * @brief   Read the piston position equal to the total current step counts of the motor rotation
 * @param   int32_t *position - reads the value of the piston position from the class variable
 * @retval  uint32_t status - 0 - always 0 - TODO
 */
uint32_t DController::getPistonPosition(int32_t *position)
{
    uint32_t status = 0u;

    *position = pidParams.totalStepCount;

    return status;
}

/**
 * @brief   Returns if two float values are almost equal - float comparison upto 10 ^-10.
 * @param   float32_t a, float32_t b - two numbers to be compared
 * @retval  uint32_t status, false if not equal, true if equal
 */
uint32_t DController::floatEqual(float32_t a, float32_t b)
{
    uint32_t status = false;
    float32_t temp = 0.0f;

    if(a > b)
    {
        temp = a - b;
    }

    else
    {
        temp = b - a;
    }

    if(temp < EPSILON)
    {
        status = true;
    }

    return status;
}

// Mention reason for suppressing the misra rules TODO
#pragma diag_default=Pm046 /* Disable MISRA C 2004 rule 13.3 */
#pragma diag_default=Pm137 /* Disable MISRA C 2004 rule 10.4*/
#pragma diag_default=Pm128 /* Disable MISRA C 2004 rule 10.1 */
#pragma diag_suppress=Pm128 /* Disable MISRA C 2004 rule 10.1 */
/**
* @brief    Fine control loop - pressure control loop where leak is being adjusted by the screw press using
            proportional control of piston in the screw press
* @param    void
* @retval   void
*/
void DController::fineControlLoop(void)
{
    uint32_t timeStatus = 0u;
    uint32_t epochTime = 0u;

    timeStatus = getEpochTime(&epochTime);

    if(true == timeStatus)
    {
        coarseControlLogParams.startTime = epochTime;
        pidParams.elapsedTime = epochTime;
    }

    if(1u == pidParams.fineControl)
    {
        fineControlLed();

        /*
        read pressure with highest precision
        pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
        PID['mode'] = mode

        if PID['mode'] != 1 or PID['excessLeak'] == 1 or PID['excessVolume'] == 1 or \
        PID['overPressure'] == 1 or PID['rangeExceeded'] == 1:
        */
        if((myMode == E_CONTROLLER_MODE_CONTROL) &&
                (pidParams.excessLeak != 1u) &&
                (pidParams.excessVolume != 1u) &&
                (pidParams.overPressure != 1u) &&
                (pidParams.rangeExceeded != 1u))
        {
            /*
            adjust measured pressure by simulated leak rate effect
            control to pressure in the same units as setpoint
            [pressureG, pressure, atmPressure][spType] + testParams.fakeLeak;
            */
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            pidParams.pressureSetPoint = pressureSetPoint;
            /*
            store previous dP and dV values
            bayes['dP_'] = bayes['dP']  //# previous measured dP(mbar)
            */
            bayesParams.prevChangeInPressure = bayesParams.changeInPressure;
            // bayes['dV_'] = bayes['dV']  //# previous applied dV(mL)
            bayesParams.prevChangeInVolume = bayesParams.changeInVolume;
            /*
            change in pressure from last measurement(mbar), +ve == increasing pressure
            bayes['dP'] = pressure + testing['fakeLeak'] - bayes['P']
            */
            bayesParams.changeInPressure = absolutePressure + testParams.fakeLeak - bayesParams.measuredPressure;
            /*
            use absolute pressure for volume estimates using gas law
            bayes['P'] = pressure + testing['fakeLeak']
            */
            bayesParams.measuredPressure = absolutePressure + testParams.fakeLeak;
            /*
            store previous pressure error for prediction error calculation
            bayes['targetdP'] = PID['E']
            */
            bayesParams.targetdP = pidParams.pressureError;
            /*
            Note: Unlike PID['targetdP'], bayes['targetdP']
            is not adjusted for leak estimate.
            The leak estimate may be incorrectand the bayes prediction error
            should therefore be non - zero even if the PID['targetdP'] was achieved.

            pressure error error(mbar), -ve if pressure above setpoint
            PID['E'] = PID['setpoint'] - PID['pressure']
            */
            pidParams.pressureError = pidParams.pressureSetPoint - pidParams.controlledPressure;
            /*
            target pressure correction after anticipating leak effect(mbar)
            PID['targetdP'] = PID['E'] - bayes['leak']
            */
            pidParams.pressureCorrectionTarget = pidParams.pressureError - bayesParams.estimatedLeakRate;
            /*
            steps to take to achieve zero pressure error in one control iteration
            PID['stepSize'] = int(round(bayes['kP'] * PID['targetdP']))
            */
            pidParams.stepSize = int(round(bayesParams.estimatedKp * pidParams.pressureCorrectionTarget));
            /*
            setpoint achievable status(True / False)
            TBD use this to abort setpoint if not achievable before reaching max / min piston positions
            PID['inRange'] = (PID['E'] > bayes['mindP']) and (PID['E'] < bayes['maxdP'])
            */
            /*
            pidParams.isSetpointInControllerRange = (pidParams.pressureError >
                                                        bayesParams.maxNegativePressureChangeAchievable) &&
                                                        (pidParams.pressureError <
                                                        bayesParams.maxPositivePressureChangeAchievable);
            */
            /*
            abort correction if pressure error is within the measurement noise floor to save power,
            also reduces control noise when at setpoint without a leak
            */
            float32_t fabsPressCorrTarget = fabs(pidParams.pressureCorrectionTarget);
            float32_t sqrtUncertaintyPressDiff = sqrt(bayesParams.uncertaintyPressureDiff);
            float32_t fabsPressureError = fabs(pidParams.pressureError);

            if((fabsPressCorrTarget < sqrtUncertaintyPressDiff) &&
                    (fabsPressureError < sqrtUncertaintyPressDiff))
            {
                // PID['targetdP'] = 0.0f;
                pidParams.pressureCorrectionTarget = 0.0f;
                // PID['stepSize'] = 0.0f;
                pidParams.stepSize = 0u;
            }

#ifdef ENABLE_MOTOR_CC
            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);
            calcDistanceTravelled(pidParams.stepCount);
            pidParams.pistonPosition = pidParams.pistonPosition + pidParams.stepCount;
#endif
            // change in volume(mL)
            bayesParams.changeInVolume = -1.0f * screwParams.changeInVolumePerPulse * (float32_t)(pidParams.stepCount);

            checkPiston();

            estimate();
            calcStatus();
            /*
            report back status to GENII
            calcStatus() imported from dataStructures4.py
            calcStatus(PID = PID)
            pv624.setControllerStatus(PID['status'])
            and abort on error or mode change

            Code below used for testing controller performance, not for product use :
            -----------------------------------------------------------------------
            scale maxFakeLeakRate by measured gage pressureand test volume
            Don't use volume estimate to scale leak rate
            so that errors in the estimate do not affect applied leak rate
            and the true leak rate can be back - calculated from pressure and nominal test volume.
            */
            testParams.fakeLeakRate = -fabs(testParams.maxFakeLeakRate) * \
                                      gaugePressure / screwParams.maxAllowedPressure *
                                      screwParams.nominalTotalVolume / testParams.volumeForLeakRateAdjustment;
            /*
            new cumulative pressure error from the simulated leak(mbar)
            testing['fakeLeak'] = testing['fakeLeak'] + testing['fakeLeakRate']
            */
            testParams.fakeLeak = testParams.fakeLeak + testParams.fakeLeakRate;
            /*
            Code below to be replaced with logging of data to PV624 non - volatile memory
            for diagnostic purposes.# -------------------------------------------------

            if logging['logData']:
            get timestamp
            PID['elapsedTime'] = round((datetime.now() - logging['startTime']).total_seconds(), 3)
            append dynamic data to dataFile
            csvFile = csv.writer(f, delimiter = ',')
            csvFile.writerow(list(PID.values()) + list(bayes.values()) + list(testing.values()))

            write a small subset of global data structure content to screen
            printValues = [round(PID['pressure'], 2),
            round(PID['E'], 2),
            round(bayes['V'], 1),
            round(bayes['leak'], 4),
            round(testing['fakeLeakRate'], 4),
            round(PID['position']),
            round(PID['measCurrent'], 2)
            ]

            formatString = len(printValues) * '{: >8} '
            print(formatString.format(*printValues))
            */
            controllerState = eFineControlLoop;
        }

        else
        {
            /*
            Genii has changed operating mode to vent or measure
            or a control error has occurred
            abort fine control and return to coarse control loop
            */
            pidParams.fineControl = 0u;
            // Before moving to coarse control again, set the initialization parameters
            pidParams.stable = 0u;
            pidParams.excessLeak = 0u;
            pidParams.excessVolume = 0u;
            pidParams.overPressure = 0u;
            pidParams.pumpAttempts = 0u;

            if(0u == pidParams.rangeExceeded)
            {
                // Range was not exceeded in the previous fine control, so volume estimate is ok to use
                pidParams.pumpTolerance = bayesParams.maxSysVolumeEstimate / bayesParams.estimatedVolume;
                pidParams.pumpTolerance = pidParams.minPumpTolerance * pidParams.pumpTolerance;
                pidParams.pumpTolerance = min(pidParams.pumpTolerance, pidParams.maxPumpTolerance);
            }

            else
            {
                // Range exceeded in the previous fine control attempt, volume estimate cannot be trusted
                pidParams.pumpTolerance = pidParams.minPumpTolerance;
            }

            controllerState = eCoarseControlLoop;
            calcStatus();
        }
    }

    else
    {
        /* Exit fine control TODO */
        //controllerState = eCoarseControlLoopEntry;
    }

}
#pragma diag_default=Pm128 /* Disable MISRA C 2004 rule 10.1 */

/**
 * @brief   Runs once before fine control loop is entered, required if any other initializations are required before
            entering the fine control loop
 * @param   None
 * @retval  None
 */
void DController::fineControlSmEntry(void)
{
    controllerState = eFineControlLoop;
}

/**
 * @brief   Calculates the status as required by the DPI, and sets the status in the PV624 global variable for access
            by different modules
 * @param   None
 * @retval  None
 */
void DController::calcStatus(void)
{
    uint32_t tempStatus = 0u;

    tempStatus = pidParams.pumpUp;
    tempStatus = tempStatus | pidParams.pumpDown << 1;
    tempStatus = tempStatus | pidParams.control << 2;
    tempStatus = tempStatus | pidParams.venting << 3;
    tempStatus = tempStatus | pidParams.stable << 4;
    tempStatus = tempStatus | pidParams.vented << 5;
    tempStatus = tempStatus | pidParams.excessLeak << 6;
    tempStatus = tempStatus | pidParams.excessVolume << 7;
    tempStatus = tempStatus | pidParams.overPressure << 8;
    tempStatus = tempStatus | pidParams.excessOffset << 9;
    tempStatus = tempStatus | pidParams.measure << 10;
    tempStatus = tempStatus | pidParams.fineControl << 11;
    tempStatus = tempStatus | pidParams.pistonCentered << 12;
    tempStatus = tempStatus | pidParams.centering << 13;
    tempStatus = tempStatus | pidParams.controlledVent << 14;
    tempStatus = tempStatus | pidParams.centeringVent << 15;
    tempStatus = tempStatus | pidParams.rangeExceeded << 16;
    tempStatus = tempStatus | pidParams.ccError << 17;
    tempStatus = tempStatus | pidParams.ventDirUp << 18;
    tempStatus = tempStatus | pidParams.ventDirDown << 19;
    tempStatus = tempStatus | pidParams.controlRate << 20;
    tempStatus = tempStatus | pidParams.maxLim << 21;
    tempStatus = tempStatus | pidParams.minLim << 22;
    tempStatus = tempStatus | pidParams.fastVent << 23;

    controllerStatus.bytes = tempStatus;

    PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
}

/**
 * @brief   Led indications during different modes of coarse control
            Measure - Green if no errors
            Controlling - slow blink yellow
            Venting - fast blink yellow
            Vented - Green
            Rate control - fast blink yellow
 * @param   None
 * @retval  None
 */
void DController::coarseControlLed(void)
{
    // Check if any other errors exist in the system
    deviceStatus_t devStat;
    deviceStatus_t tempStatus;
    devStat.bytes = 0u;
    devStat = PV624->errorHandler->getDeviceStatus();

    // Which means anything other than charging,or ble or owi requests
    tempStatus.bytes = 0x017FFFFu;

    if(tempStatus.bytes & devStat.bytes)
    {
        // For any errors other than charging, owi or ble remote request do not override error handler
    }
    else
    {

        if((myMode != myPrevMode) ||
                ((eControllerMode_t)E_CONTROLLER_MODE_VENT == myMode) ||
                ((eControllerMode_t)E_CONTROLLER_MODE_RATE == myMode))
        {
            // Update LEDs only if mode has changed
            if((eControllerMode_t)E_CONTROLLER_MODE_MEASURE == myMode)
            {
                PV624->userInterface->statusLedControl(eStatusOkay,
                                                       E_LED_OPERATION_SWITCH_ON,
                                                       65535u,
                                                       E_LED_STATE_SWITCH_ON,
                                                       1u);
            }

            else if((eControllerMode_t)E_CONTROLLER_MODE_CONTROL == myMode)
            {
                PV624->userInterface->statusLedControl(eStatusProcessing,
                                                       E_LED_OPERATION_SWITCH_OFF,
                                                       65535u,
                                                       E_LED_STATE_SWITCH_OFF,
                                                       2u);
                PV624->userInterface->statusLedControl(eStatusProcessing,
                                                       E_LED_OPERATION_TOGGLE,
                                                       65535u,
                                                       E_LED_STATE_SWITCH_OFF,
                                                       2u);
            }

            else
            {
                // Mode is control, vent or controlled vent
                // Between vent modes, system could be venting or vented

                if(pidParams.vented != prevVentState)
                {
                    if(1u == pidParams.vented)
                    {
                        PV624->userInterface->statusLedControl(eStatusProcessing,
                                                               E_LED_OPERATION_SWITCH_OFF,
                                                               65535u,
                                                               E_LED_STATE_SWITCH_OFF,
                                                               2u);
                        PV624->userInterface->statusLedControl(eStatusProcessing,
                                                               E_LED_OPERATION_TOGGLE,
                                                               65535u,
                                                               E_LED_STATE_SWITCH_OFF,
                                                               4u);
                    }

                    else
                    {
                        PV624->userInterface->statusLedControl(eStatusProcessing,
                                                               E_LED_OPERATION_SWITCH_OFF,
                                                               65535u,
                                                               E_LED_STATE_SWITCH_OFF,
                                                               2u);
                        PV624->userInterface->statusLedControl(eStatusProcessing,
                                                               E_LED_OPERATION_TOGGLE,
                                                               65535u,
                                                               E_LED_STATE_SWITCH_OFF,
                                                               2u);
                    }
                }

                prevVentState = pidParams.vented;
            }
        }


        myPrevMode = myMode;
    }

}

/**
 * @brief   LED indication in fine control
            When fine control is enabled - steady yellow
 * @param   None
 * @retval  None
 */
void DController::fineControlLed(void)
{
    PV624->userInterface->statusLedControl(eStatusProcessing,
                                           E_LED_OPERATION_SWITCH_ON,
                                           65535u,
                                           E_LED_STATE_SWITCH_OFF,
                                           1u);
}

/**
 * @brief   Measure mode in coarase control - slow measurement of pressure with isolated pump
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlMeasure(void)
{
    uint32_t status = 0u;
    controllerStatus.bytes = 0u;
    pidParams.ccError = eCoarseControlErrorReset;
    return status;
}

/**
 * @brief   Checks and returns if the piston is in range by comparing it with the screw min and max positions
 * @param   int32_t position - piston position from pid params
 * @retval  ePistonRange_t isInRange
            outside of min / max position - ePistonOutOfRange
            Within limits - ePistonInRange
 */
ePistonRange_t DController::validatePistonPosition(int32_t position)
{
    ePistonRange_t isInRange = ePistonOutOfRange;

    if(screwParams.maxPosition > position)
    {
        if(screwParams.minPosition < position)
        {
            isInRange = ePistonInRange;
        }
    }

    return isInRange;
}

/**
 * @brief   Checks and returns if the piston is in center by comparing it with center position with a tolerance
 * @param   int32_t position - piston position from pid params
 * @retval  ePistonCentreStatus_t isInCentre
            If within tolerance - ePistonCentered
            Outside of tolerance - ePistonOutOfCentre
 */
ePistonCentreStatus_t DController::isPistonCentered(int32_t position)
{
    int32_t positionLeft = 0;
    int32_t positionRight = 0;
    ePistonCentreStatus_t isInCentre = ePistonOutOfCentre;

    positionLeft = screwParams.centerPositionCount - screwParams.centerTolerance;
    positionRight = screwParams.centerPositionCount + screwParams.centerTolerance;

    if(positionLeft < position)
    {
        if(positionRight > position)
        {
            isInCentre = ePistonCentered;
        }
    }

    return isInCentre;
}

/**
 * @brief   Reads and returns the value of the optical sensors by reading the GPIO pins
 * @param   uint32_t *opt1 and uint32_t *opt2 - returns high if the sensor is not triggered and low if it is
 * @retval  None
 */
void DController::getOptSensors(uint32_t *opt1, uint32_t *opt2)
{
    // Pin and port names need to be replaced by variables TODO
    *opt1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
    *opt2 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
}

/**
 * @brief   Check if the piston has hit any extreme, or is centered and sets controller class variables
 * @param   None
 * @retval  None
 */
void DController::checkPiston(void)
{
    uint32_t optSensorMin = 0u;
    uint32_t optSensorMax = 0u;
    ePistonCentreStatus_t isCentered = ePistonOutOfCentre;

    getOptSensors(&optSensorMin, &optSensorMax);

    if(0u == optSensorMax)
    {
        // Max limit switch triggered
        pidParams.pistonPosition = screwParams.maxPosition;
        pidParams.rangeExceeded = 1u;
    }

    else if(0u == optSensorMin)
    {
        pidParams.pistonPosition = screwParams.minPosition;
        pidParams.rangeExceeded = 1u;
    }

    else
    {
        pidParams.rangeExceeded = 0u;
    }

    // Check if piston is centered
    isCentered = isPistonCentered(pidParams.pistonPosition);
    pidParams.pistonCentered = (uint32_t)(isCentered);
}

/**
 * @brief   Initial case before entering coarse control loop - sets and initializes variables for every new coarse
            control run
 * @param   None
 * @retval  None
 */
void DController::coarseControlSmEntry(void)
{
    uint32_t epochTime = 0u;
    uint32_t status = false;

    uint32_t sensorType = 0u;
    float32_t absSensorOffset = 0.0f;
    float32_t varDpTemp = 0.0f;

    status = getEpochTime(&epochTime);

    if(true == status)
    {
        coarseControlLogParams.startTime = epochTime;
        pidParams.elapsedTime = epochTime;
    }

    float uncertaintyScaling = 0.0f;

#ifdef RUN_ON_MICRO
    sprintf_s(coarseControlLogParams.fileName, 80u, "% d-%02d-%02d%02d:%02d:%02d",
              ltm->tm_year + 1900,
              ltm->tm_mon + 1,
              ltm->tm_mday,
              ltm->tm_hour,
              ltm->tm_min,
              ltm->tm_sec);
#else
    //strcpy_s(coarseControlLogParams.fileName, coarseControlFileName);
#endif
    /*
    reset control status flags
    TBD action when control error flags != 0 ?
    */
    pidParams.stable = 0u;
    pidParams.excessLeak = 0u;
    pidParams.excessVolume = 0u;
    pidParams.overPressure = 0u;
    pidParams.rangeExceeded = 0u;

    setVent();          // Open the vent valve
    checkPiston();      // Check and set if piston is within screw limits and if centered

    PV624->getPosFullscale(&sensorFsValue);             // Read the full scale sensor value and set class variable
    sensorParams.fullScalePressure = sensorFsValue;
    PV624->getPM620Type(&sensorType);                   // Read the type of the sensor being used and set class var
    sensorParams.sensorType = sensorType;

    // Calculate uncertainty scaling required by sensor
    uncertaintyScaling = 1.0f * (sensorParams.fullScalePressure / fsValue) * (sensorParams.fullScalePressure / fsValue);
    // PM full scale pressure(mbar)
    fsValue = sensorParams.fullScalePressure;
    /* bayes['varP'] uncertainty in pressure measurement(mbar)  */
    bayesParams.sensorUncertainity = uncertaintyScaling * bayesParams.sensorUncertainity;
    // uncertainty in measured pressure changes(mbar)
    bayesParams.uncertaintyPressureDiff = uncertaintyScaling * bayesParams.uncertaintyPressureDiff;
    pidParams.fineControl = 0u;     // disable fine pressure control
    pidParams.stable = 0u;          // set pressure not stable since we are not doing fine control

    // Wait for the system vent to complete
    // Run a state machine here
    if(entryState == 0u)
    {
        // start by setting initial starting pressure to mesaured gauge pressure
        entryInitPressureG = gaugePressure;
        // This needs to be replaced by enumerations TODO
        entryState = 1u;
    }

    else if(entryState == 1u)
    {
        entryFinalPressureG = gaugePressure; // Final pressure after initial measurement
        bayesParams.changeInPressure = fabs(entryFinalPressureG - entryInitPressureG); // measure change in pressure
        sensorParams.offset = entryFinalPressureG; // set the sensor offset to the difference * 1.5 for tolerance
        // Set the sensor gauge uncertainty to max of measured sensor offset or initialized gauge uncertainty
        sensorParams.gaugeUncertainty = sensorParams.minGaugeUncertainty;
        entryIterations = 0u;   // Set the number of iterations to 0 to run in the next state
        entryState = 2u;        // Change state to measure actual offset
    }

    else if(entryState == 2u)
    {
        varDpTemp = sqrt(bayesParams.uncertaintyPressureDiff);
        varDpTemp = 2.0f * varDpTemp;

        if(bayesParams.changeInPressure > varDpTemp)
        {
            entryIterations = entryIterations + 1u;
            entryInitPressureG = entryFinalPressureG;
            entryFinalPressureG = gaugePressure;
            bayesParams.changeInPressure = fabs(entryFinalPressureG - entryInitPressureG);

            if(entryIterations > 100u)
            {
                sensorParams.offset = 0.0f;
                absSensorOffset = fabs(sensorParams.offset * 1.5f);
                entryState = 3u;
            }

            else
            {
                sensorParams.offset = entryFinalPressureG;
                absSensorOffset = fabs(sensorParams.offset * 1.5f);
            }
        }

        else
        {
            entryState = 3u;
        }
    }

    else if(entryState == 3u)
    {
        absSensorOffset = fabs(sensorParams.offset);

        if((absSensorOffset * sensorParams.offsetSafetyFactor) > sensorParams.maxOffset)
        {
            pidParams.excessOffset = 1u;
        }

        setMeasure();
        calcStatus();

        pidParams.stable = 0u;
        pidParams.excessLeak = 0u;
        pidParams.excessVolume = 0u;
        pidParams.overPressure = 0u;
#if 0

        if(pidParams.rangeExceeded == 1u)
        {
            // Possibly return from fine control
            checkPiston();

            if(((-1.0f * sensorParams.gaugeUncertainty) <= pidParams.pressureGauge) &&
                    (pidParams.pressureGauge <= sensorParams.gaugeUncertainty))
            {
                sensorParams.gaugeUncertainty = max(sensorParams.gaugeUncertainty * 0.5f,
                                                    sensorParams.minGaugeUncertainty);
            }
        }

        pidParams.rangeExceeded = 0u;
        pidParams.excessOffset = 0u;
#endif
        entryState = 0u; // TODO - rename to proper controller init state names
        // Show green led indicating that power up, centering and venting is complete
        PV624->userInterface->statusLedControl(eStatusOkay,
                                               E_LED_OPERATION_SWITCH_ON,
                                               65535u,
                                               E_LED_STATE_SWITCH_ON,
                                               0u);
        // Before moving to coarse control, set the initialization parameters
        pidParams.stable = 0u;
        pidParams.excessLeak = 0u;
        pidParams.excessVolume = 0u;
        pidParams.overPressure = 0u;
        pidParams.pumpAttempts = 0u;

        // Unlikely that range will be exceeded at this point, is this check really required here? TODO
        if(0u == pidParams.rangeExceeded)
        {
            // Range was not exceeded in the previous fine control, so volume estimate is ok to use
            pidParams.pumpTolerance = bayesParams.maxSysVolumeEstimate / bayesParams.estimatedVolume;
            pidParams.pumpTolerance = pidParams.minPumpTolerance * pidParams.pumpTolerance;
            pidParams.pumpTolerance = min(pidParams.pumpTolerance, pidParams.maxPumpTolerance);
        }

        else
        {
            // Range exceeded in the previous fine control attempt, volume estimate cannot be trusted
            pidParams.pumpTolerance = pidParams.minPumpTolerance;
        }

        HAL_TIM_Base_Start(&htim2);
        controllerState = eCoarseControlLoop;
    }

    else
    {
        // For misra, no case
    }
}

/**
 * @brief   Runs once after exiting from coarse control
 * @param   None
 * @retval  None
 */
void DController::coarseControlSmExit(void)
{
    // coarse adjustment complete, last iteration of coarse control loop
    pidParams.fineControl = 1u;  //# exiting coarse control after this iteration
    calcStatus();
    pidParams.stepSize = 0;
    pidParams.rangeExceeded = ePistonInRange; //# reset rangeExceeded flag set by fineControl()
    calcStatus();
    //controllerState = eFineControlLoopEntry;
}
// Mention reason for suppressing the misra rules TODO
#pragma diag_suppress=Pm128 /* Disable MISRA C 2004 rule 10.1 */
/**
 * @brief   Coarse control loop
            This is a continuous control loop which operates the PV624 in measure, control or vent modes
            Samples the pressure sensor at 13 Hz in measure and vent modes
            Samples the pressure sensor at 50 Hz in control mode (coarse control only)
            Makes decisions on pump up, pump down, controlled vent, fast vent, or controlled rate and to enter fine
            control mode
 * @param   None
 * @retval  None
 */
void DController::coarseControlLoop(void)
{
    uint32_t caseStatus = 0u;
    uint32_t epochTime = 0u;
    uint32_t timeStatus = 0u;
    float32_t offsetCentrePos = 0.0f;
    float32_t offsetCentreNeg = 0.0f;
    float32_t setPointA = 0.0f;

    timeStatus = getEpochTime(&epochTime);

    if(true == timeStatus)
    {
        coarseControlLogParams.startTime = epochTime;
        pidParams.elapsedTime = epochTime;
    }

    // Run coarse control only if fine control is not enabled
    if(0u == pidParams.fineControl)
    {
        if((E_CONTROLLER_MODE_MEASURE == myMode) && (1u == pidParams.control))
        {
            /*
            Mode set by genii is measure but PID is in control mode
            Change mode to measure
            */
            setMeasure();
            // Mode changed to measure, write the distance travelled by the controller to EEPROM
            PV624->updateDistanceTravelled(screwParams.distanceTravelled);
            screwParams.distanceTravelled = 0.0f;
        }

        else if((E_CONTROLLER_MODE_MEASURE == myMode) && (1u == pidParams.venting))
        {
            /*
            Mode set by genii is measure but PID is venting
            Change mode to measure
            */
            setMeasure();
            PV624->updateDistanceTravelled(screwParams.distanceTravelled);
            screwParams.distanceTravelled = 0.0f;
        }

        else if((E_CONTROLLER_MODE_MEASURE == myMode) && (1u == pidParams.controlRate))
        {
            /*
            Mode set by genii is measure but PID is doing control rate
            Change mode to measure
            */
            setMeasure();
            PV624->updateDistanceTravelled(screwParams.distanceTravelled);
            screwParams.distanceTravelled = 0.0f;
        }

        else if((E_CONTROLLER_MODE_CONTROL == myMode) && (1u == pidParams.measure))
        {
            /*
            Mode set by genii is control but PID is in measure
            Change mode to measure
            */
            setControlIsolate();
        }

        else if((E_CONTROLLER_MODE_CONTROL == myMode) && (1u == pidParams.venting))
        {
            /*
            Mode set by genii is control but PID is in measure
            Change mode to measure
            */
            setControlIsolate();
        }

        else if((E_CONTROLLER_MODE_CONTROL == myMode) && (1u == pidParams.controlRate))
        {
            /*
            Mode set by genii is control but PID is in control Rate
            Change mode to measure
            */
            setControlIsolate();
        }

        else if((E_CONTROLLER_MODE_VENT == myMode) && (1u == pidParams.measure))
        {
            /*
            Mode set by genii is control but PID is in measure
            Change mode to measure
            */
            setVent();
        }

        else if((E_CONTROLLER_MODE_VENT == myMode) && (1u == pidParams.control))
        {
            /*
            Mode set by genii is control but PID is in measure
            Change mode to measure
            */
            setVent();
        }

        else if((E_CONTROLLER_MODE_VENT == myMode) && (1u == pidParams.controlRate))
        {
            /*
            Mode set by genii is control but PID is in measure
            Change mode to measure
            */
            setVent();
        }

        else if((E_CONTROLLER_MODE_RATE == myMode) && (1u == pidParams.measure))
        {
            /*
            Mode set by genii is control but PID is in measure
            Change mode to measure
            */
            setControlRate();
        }

        else if((E_CONTROLLER_MODE_RATE == myMode) && (1u == pidParams.venting))
        {
            /*
            Mode set by genii is control but PID is in measure
            Change mode to measure
            */
            setControlRate();
        }

        else if((E_CONTROLLER_MODE_RATE == myMode) && (1u == pidParams.control))
        {
            /*
            Mode set by genii is control but PID is in measure
            Change mode to measure
            */
            setControlRate();
        }

        else if(E_CONTROLLER_MODE_MEASURE == myMode)
        {
            // Mode is correct, so reset coarse control error
            pidParams.ccError = eCoarseControlErrorReset;
            // [pressureG, pressure, atmPressure][spType] + testParams.fakeLeak;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.setPointType = setPointType;
            coarseControlMeasure();
        }

        else if(E_CONTROLLER_MODE_CONTROL == myMode)
        {
            // Mode is correct, so reset coarse control error
            pidParams.ccError = eCoarseControlErrorReset;

            // Read gauge pressure also
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType();

            pidParams.pressureSetPoint = pressureSetPoint;
            //pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
            setPointG = pidParams.pressureSetPoint - (atmosphericPressure * setPointType);

            // Check if piston is within range
            checkPiston();
            pidParams.mode = myMode;

            offsetCentrePos = sensorParams.offset + sensorParams.gaugeUncertainty;
            offsetCentreNeg = sensorParams.offset - sensorParams.gaugeUncertainty;
            setPointA = setPointG + atmosphericPressure;

            if(1u == pidParams.pumpUp)
            {
                if((setPointG <= offsetCentreNeg) || (setPointG >= offsetCentrePos))
                {
                    pidParams.overshoot = setPointA * pidParams.overshootScaling * pidParams.pumpTolerance;
                }
            }

            else if(1u == pidParams.pumpDown)
            {
                if((setPointG <= offsetCentreNeg) || (setPointG >= offsetCentrePos))
                {
                    pidParams.overshoot = -1.0f * setPointA * pidParams.overshootScaling * pidParams.pumpTolerance;
                }
            }

            else
            {
                pidParams.overshoot = 0.0f;
            }

            /*
            The coarse contol algorithm runs in one of 8 cases as below
            Case 1, check if current pressure is within pump tolerance
            and piston is also centered
            Case 2, set point is on the way to center while increasing pressure
            Case 3, set point is on the way to center while decreasing pressure
            Case 4, set point is crossing from pressure to vacuum or from vacuum
            to pressure
            Case 5, piston is not in center and moving it towards center
            shall increase pressure error and require manual pump down action
            Case 6, piston is not in center and moving it towards center will
            increase pressure error and require manual pump up action
            Case 7, Set point is greater than pressure
            Case 8, Set point is less than pressure
            */
            caseStatus = coarseControlCase1();

            if(0u == caseStatus)
            {
                caseStatus = coarseControlCase2();

                if(0u == caseStatus)
                {
                    caseStatus = coarseControlCase3();

                    if(0u == caseStatus)
                    {
                        caseStatus = coarseControlCase4();

                        if(0u == caseStatus)
                        {
                            caseStatus = coarseControlCase5();

                            if(0u == caseStatus)
                            {
                                caseStatus = coarseControlCase6();

                                if(0u == caseStatus)
                                {
                                    caseStatus = coarseControlCase7();

                                    if(0u == caseStatus)
                                    {
                                        caseStatus = coarseControlCase8();

                                        if(0u == caseStatus)
                                        {
                                            caseStatus = coarseControlCase9();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

#ifdef ENABLE_MOTOR_CC
            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);
            calcDistanceTravelled(pidParams.stepCount);
            pidParams.pistonPosition = pidParams.pistonPosition + pidParams.stepCount;
#endif
        }

        else if(E_CONTROLLER_MODE_VENT == myMode)
        {
            // Mode is correct, so reset coarse control error
            pidParams.ccError = 0u;
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.setPointType = setPointType;
            pidParams.mode = myMode;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            coarseControlVent();
        }

        else if(E_CONTROLLER_MODE_RATE == myMode)
        {
            pidParams.ccError = 0u;
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.setPointType = setPointType;
            pidParams.mode = myMode;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            coarseControlRate();
        }

        else
        {
            /*
            Mode is neither of measure, control or vent
            This is an invalid case, hence signal error
            */
            pidParams.ccError = eCoarseControlErrorSet;
            calcStatus();
        }

        calcStatus();
        coarseControlLed();
    }
}

#pragma diag_suppress=Pm031

/**
 * @brief   Coarse control rate state - used when a measured rate is required while going down in pressure or up
            in vacuum
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlRate(void)
{
    uint32_t status = 0u;
    float32_t absDp = 0.0f;
    float32_t maxRate = 0.0f;
    float32_t calcVarDp = 0.0f;
    float32_t offsetPos = 0.0f;
    float32_t offsetNeg = 0.0f;

    PV624->getVentRate(&pidParams.ventRate);
    pidParams.stepSize = 0;

    if(0u == pidParams.controlledVent)
    {
        pidParams.controlledVent = 1u;
        bayesParams.ventIterations = 0u;
        bayesParams.ventFinalPressure = gaugePressure;
    }

    bayesParams.ventInitialPressure = bayesParams.ventFinalPressure;
    bayesParams.ventFinalPressure = gaugePressure;
    bayesParams.changeInPressure = bayesParams.ventFinalPressure - bayesParams.ventInitialPressure;

    absDp = fabs(bayesParams.changeInPressure);
    calcVarDp = 3.0f * (bayesParams.uncertaintyPressureDiff * bayesParams.uncertaintyPressureDiff);
    maxRate = max(pidParams.ventRate, calcVarDp);

    offsetPos = sensorParams.offset + sensorParams.gaugeUncertainty;
    offsetNeg = sensorParams.offset - sensorParams.gaugeUncertainty;

    if((offsetNeg < gaugePressure) && (gaugePressure < offsetPos))
    {
        // System is vented, maintain vented state and reduce power consumption
        PV624->valve3->reConfigValve(E_VALVE_MODE_PWMA);
        pidParams.vented = 1u;

        if(pidParams.holdVentCount < screwParams.holdVentIterations)
        {
            pidParams.holdVentCount = pidParams.holdVentCount + 1u;
            bayesParams.ventDutyCycle = screwParams.holdVentDutyCycle;
            pulseVent();
        }

        else if(pidParams.holdVentCount > screwParams.holdVentInterval)
        {
            pidParams.holdVentCount = 0u;
            bayesParams.ventDutyCycle = screwParams.holdVentDutyCycle;
            pulseVent();
        }

        else
        {
            pidParams.holdVentCount = pidParams.holdVentCount + 1u;
            bayesParams.ventDutyCycle = 0u;
            PV624->valve3->triggerValve(VALVE_STATE_OFF);
        }

    }

    else if(absDp < maxRate)
    {
        // increase vent on-time to maxVentDutyCycle until previous vent effect is greater
        // than the smaller of target ventRate or pressure uncertainty
        PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
        bayesParams.ventDutyCycle = min((bayesParams.ventDutyCycle + screwParams.ventDutyCycleIncrement),
                                        screwParams.maxVentDutyCycle);
        pidParams.vented = 0u;
        pulseVent();
    }

    else
    {
        // decrease vent on-time to minVentDutyCycle until previous vent effect is less than or
        // equal to target ventRate
        PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
        bayesParams.ventDutyCycle = max((bayesParams.ventDutyCycle - screwParams.ventDutyCycleIncrement),
                                        screwParams.minVentDutyCycle);
        pidParams.vented = 0u;
        pulseVent();
    }

    return status;
}

/**
 * @brief   Coarse control vent state - used when venting down from pressure to atmosphere or from vacuum to atm
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlVent(void)
{
    // ePistonRange_t pistonRange = ePistonOutOfRange;
    uint32_t status = 0u;
    float32_t tempPresUncertainty = 0.0f;
    float32_t absDp = 0.0f;
    float32_t offsetPos = 0.0f;
    float32_t offsetNeg = 0.0f;

    checkPiston();

    if(0u == pidParams.fastVent)
    {
        setFastVent();
        bayesParams.ventIterations = 0u;
        bayesParams.ventFinalPressure = gaugePressure;
    }

    bayesParams.ventInitialPressure = bayesParams.ventFinalPressure;
    bayesParams.ventFinalPressure = gaugePressure;
    bayesParams.changeInPressure = bayesParams.ventFinalPressure - bayesParams.ventInitialPressure;

    absDp = fabs(bayesParams.changeInPressure);
    tempPresUncertainty = 2.0f * sqrt(bayesParams.uncertaintyPressureDiff);

    offsetPos = sensorParams.offset + sensorParams.gaugeUncertainty;
    offsetNeg = sensorParams.offset - sensorParams.gaugeUncertainty;

    if((offsetNeg < gaugePressure) && (gaugePressure < offsetPos) && (absDp < tempPresUncertainty))
    {
        if(pidParams.holdVentCount < screwParams.holdVentIterations)
        {
            pidParams.holdVentCount = pidParams.holdVentCount + 1u;
            bayesParams.ventDutyCycle = screwParams.holdVentDutyCycle;
            pulseVent();

            if(0u == pidParams.vented)
            {
                pidParams.vented = 1u;
            }
        }

        else if(pidParams.holdVentCount > screwParams.holdVentInterval)
        {
            pidParams.holdVentCount = 0u;
            bayesParams.ventDutyCycle = screwParams.holdVentDutyCycle;
            pulseVent();
        }

        else
        {
            pidParams.holdVentCount = pidParams.holdVentCount + 1u;
            bayesParams.ventDutyCycle = 0u;
            PV624->valve3->triggerValve(VALVE_STATE_OFF);

            if(1u == pidParams.vented)
            {
                pidParams.vented = 0u;
            }
        }
    }

    else
    {
        // System is not vented, vent as quickly as possible
        bayesParams.ventDutyCycle = screwParams.maxVentDutyCyclePwm;
        pidParams.holdVentCount = 0u;
        pidParams.vented = 0u;
        pulseVent();
    }

    // Centre piston while venting
    if(pidParams.pistonPosition > (screwParams.centerPositionCount + screwParams.centerTolerance))
    {
        pidParams.stepSize = -1 * screwParams.maxStepSize;

        if(0u == pidParams.centeringVent)
        {
            pidParams.centeringVent = 1u;
        }
    }

    else if(pidParams.pistonPosition < (screwParams.centerPositionCount - screwParams.centerTolerance))
    {
        pidParams.stepSize = screwParams.maxStepSize;

        if(0u == pidParams.centeringVent)
        {
            pidParams.centeringVent = 1u;
        }
    }

    else
    {
        pidParams.stepSize = 0;
        pidParams.centeringVent = 0u;
    }

#ifdef ENABLE_MOTOR_CC
    PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);
    pidParams.pistonPosition = pidParams.pistonPosition + pidParams.stepCount;
    calcDistanceTravelled(pidParams.stepCount);
#endif

    return status;
}
#pragma diag_default=Pm031
#pragma diag_default=Pm128 /* Disable MISRA C 2004 rule 10.1 */

/**
 * @brief   Returns the absolute pressure value from 2 values
 * @param   float32_t p1 - pressure value 1
            float32_t p2 - pressure value 2
            float32_t *absValue - contains the value to be returned
 * @retval  uint32_t status - always returns 1, never fails
 */
uint32_t DController::getAbsPressure(float32_t p1, float32_t p2, float32_t *absVal)
{
    uint32_t status = 1u;

    if(p1 > p2)
    {
        *absVal = p1 - p2;
    }

    else
    {
        *absVal = p2 - p1;
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 1
            Write definition of the this case and what it handles TODO
 * @param   None
 * @retval  uint32_t status - 1 if the case is executed to pass, 0 if case is not executed to pass
 */
uint32_t DController::coarseControlCase1(void)
{
    uint32_t status = 0u;

    uint32_t conditionPassed = 0u;
    float32_t pressurePumpTolerance = 0.0f;
    float32_t absValue = 0.0f;
    float32_t totalOvershoot = 0.0f;

    totalOvershoot = setPointG + pidParams.overshoot;

    getAbsPressure(totalOvershoot, gaugePressure, &absValue);
    pressurePumpTolerance = absValue / absolutePressure;

    if(pressurePumpTolerance < pidParams.pumpTolerance)
    {
        // If pressure is within the pump tolerance
        if(1u == pidParams.pistonCentered)
        {
            // If piston is centered directly jump to FC
            conditionPassed = 1u;
        }

        if(pidParams.pumpTolerance > pidParams.minPumpTolerance)
        {
            if(((setPointG > gaugePressure) && (pidParams.pistonPosition < screwParams.centerPositionCount)) ||
                    ((setPointG < gaugePressure) && (pidParams.pistonPosition > screwParams.centerPositionCount)))
            {
                conditionPassed = 1u;
            }
        }
    }

    if(1u == conditionPassed)
    {
        pidParams.stepSize = 0;
        PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);
        pidParams.pistonPosition = pidParams.pistonPosition + pidParams.stepCount;
        calcDistanceTravelled(pidParams.stepCount);
        setControlIsolate();

        if(floatEqual(0.0f, pidParams.overshoot))
        {
            status = 1u;
            pidParams.fineControl = 1u;
            controllerState = eFineControlLoop;
        }
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 2
            Write definition of the this case and what it handles TODO
 * @param   None
 * @retval  uint32_t status - 1 if the case is executed to pass, 0 if case is not executed to pass
 */
uint32_t DController::coarseControlCase2()
{
    uint32_t status = 0u;
    int32_t pistonCentreLeft = 0;

    pistonCentreLeft = screwParams.centerPositionCount - screwParams.centerTolerance;

    if((setPointG > gaugePressure) && (pidParams.pistonPosition < pistonCentreLeft))
    {
        // Make the status 1 if this case is executed
        status = 1u;
        pidParams.stepSize = motorParams.maxStepSize;

        if(0u == pidParams.centering)
        {
            setControlIsolate();
            pidParams.centering = 1u;
            bayesParams.changeInVolume = 0.0f;
            bayesParams.measuredPressure = absolutePressure;
            bayesParams.changeInPressure = 0.0f;
        }

        else
        {
            bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
            bayesParams.changeInVolume = bayesParams.changeInVolume -
                                         ((float32_t)(pidParams.stepCount) * screwParams.changeInVolumePerPulse);
        }

        estimate();
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 3
            Write definition of the this case and what it handles TODO
 * @param   None
 * @retval  uint32_t status - 1 if the case is executed to pass, 0 if case is not executed to pass
 */
uint32_t DController::coarseControlCase3()
{
    uint32_t status = 0u;
    int32_t pistonCentreRight = 0;

    pistonCentreRight = screwParams.centerPositionCount + screwParams.centerTolerance;

    if((setPointG < gaugePressure) && (pidParams.pistonPosition > pistonCentreRight))
    {
        status = 1u;
        pidParams.stepSize = -1 * (motorParams.maxStepSize);

        if(0u == pidParams.centering)
        {
            setControlIsolate();
            pidParams.centering = 1u;
            bayesParams.changeInVolume = 0.0f;
            pidParams.stepCount = 0;
            bayesParams.measuredPressure = absolutePressure;
            bayesParams.changeInPressure = 0.0f;
        }

        else
        {
            bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
            bayesParams.changeInVolume = bayesParams.changeInVolume -
                                         ((float32_t)(pidParams.stepCount) * screwParams.changeInVolumePerPulse);
        }

        estimate();
    }

    return status;
}


/**
 * @brief   Control mode CC CASE 4
            Write definition of the this case and what it handles TODO
 * @param   None
 * @retval  uint32_t status - 1 if the case is executed to pass, 0 if case is not executed to pass
 */
uint32_t DController::coarseControlCase4()
{
    uint32_t status = 0u;
    float32_t offsetPos = 0.0f;
    float32_t offsetNeg = 0.0f;

    offsetPos = sensorParams.offset + sensorParams.gaugeUncertainty;
    offsetNeg = sensorParams.offset - sensorParams.gaugeUncertainty;

    // Newly added fast vent method
    if(((setPointG <= offsetPos) && (offsetPos < gaugePressure) && (0u == pidParams.pumpDown)) ||
            ((setPointG >= offsetNeg) && (offsetNeg > gaugePressure) && (0u == pidParams.pumpUp)))
    {
        // Vent as quickly as possible towards gauge uncertainty with offset pressure using pwm mode
        status = 1u;
        pidParams.stepSize = 0;

        if(0u == pidParams.fastVent)
        {
            setFastVent();

            if(gaugePressure > setPointG)
            {
                pidParams.ventDirDown = 1u;
                pidParams.ventDirUp = 0u;
            }

            else
            {
                pidParams.ventDirDown = 0u;
                pidParams.ventDirUp = 1u;
            }

            pulseVent();
        }
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 5
            Write definition of the this case and what it handles TODO
 * @param   None
 * @retval  uint32_t status - 1 if the case is executed to pass, 0 if case is not executed to pass
 */
uint32_t DController::coarseControlCase5()
{
    uint32_t status = 0u;
#if 0
    float effPressureNeg = 0.0f;
    float effPressurePos = 0.0f;
#endif
    float32_t totalOvershoot = 0.0f;
    float32_t absPressure = 0.0f;
    float32_t tempPressure = 0.0f;
    float32_t offsetPos = 0.0f;
    float32_t offsetNeg = 0.0f;

    int32_t pistonCentreLeft = 0;
    int32_t pistonCentreRight = 0;

    totalOvershoot = setPointG + pidParams.overshoot;
    offsetPos = sensorParams.offset + sensorParams.gaugeUncertainty;
    offsetNeg = sensorParams.offset - sensorParams.gaugeUncertainty;
#if 0
    effPressureNeg = gaugePressure - sensorParams.gaugeUncertainty;
    effPressurePos = gaugePressure + sensorParams.gaugeUncertainty;
#endif
    pistonCentreLeft = screwParams.centerPositionCount - screwParams.centerTolerance;
    pistonCentreRight = screwParams.centerPositionCount + screwParams.centerTolerance;

    if(((gaugePressure < totalOvershoot) && (totalOvershoot <= offsetNeg)) ||
            ((offsetPos <= totalOvershoot) && (totalOvershoot < gaugePressure)))
    {
        status = 1u;

        if(0u == pidParams.controlledVent)
        {
            /*
            store vent direction for overshoot detection on completion
            estimate system volume during controlled vent
            */
            setControlVent();

            if(gaugePressure > setPointG)
            {
                pidParams.ventDirDown = 1u;
                pidParams.ventDirUp = 0u;
            }

            else
            {
                pidParams.ventDirDown = 0u;
                pidParams.ventDirUp = 1u;
            }

            bayesParams.ventIterations = 0u;
            bayesParams.ventInitialPressure = gaugePressure;
            bayesParams.ventFinalPressure = gaugePressure;
        }

        bayesParams.ventInitialPressure = bayesParams.ventFinalPressure;
        bayesParams.ventFinalPressure = gaugePressure;
        bayesParams.changeInPressure = bayesParams.ventFinalPressure - bayesParams.ventInitialPressure;

        absPressure = fabs(setPointG - gaugePressure);
        tempPressure = screwParams.ventResetThreshold * absPressure;

        if(bayesParams.changeInPressure < tempPressure)
        {
            bayesParams.ventDutyCycle = min((screwParams.ventDutyCycleIncrement + bayesParams.ventDutyCycle),
                                            screwParams.maxVentDutyCycle);
        }

        else
        {
            bayesParams.ventDutyCycle = screwParams.minVentDutyCycle;
        }

        pulseVent();


        if(pidParams.pistonPosition > pistonCentreRight)
        {
            pidParams.stepSize = -1 * (motorParams.maxStepSize);

            if(0u == pidParams.centeringVent)
            {
                pidParams.centeringVent = 1u;
            }
        }

        else if(pidParams.pistonPosition < pistonCentreLeft)
        {
            pidParams.stepSize = motorParams.maxStepSize;

            if(0u == pidParams.centeringVent)
            {
                pidParams.centeringVent = 1u;
            }
        }

        else
        {
            pidParams.stepSize = 0;
            pidParams.centeringVent = 0u;
            bayesParams.ventIterations = bayesParams.ventIterations + 1u;
        }
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 5
            Write definition of the this case and what it handles TODO
 * @param   None
 * @retval  uint32_t status - 1 if the case is executed to pass, 0 if case is not executed to pass
 */
uint32_t DController::coarseControlCase6()
{
    uint32_t status = 0u;

    int32_t pistonCentreRight = 0;

    pistonCentreRight = screwParams.centerPositionCount - screwParams.centerTolerance;

    if(pidParams.pistonPosition < pistonCentreRight)
    {
        status = 1u;
        pidParams.stepSize = motorParams.maxStepSize;

        if(0u == pidParams.centering)
        {
            setControlIsolate();
            pidParams.centering = 1u;
            bayesParams.changeInVolume = 0.0f;
            bayesParams.measuredPressure = absolutePressure;
            bayesParams.changeInPressure = 0.0f;

        }

        else
        {
            bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
            bayesParams.changeInVolume = bayesParams.changeInVolume -
                                         ((float32_t)(pidParams.stepCount) * screwParams.changeInVolumePerPulse);
            estimate();
        }
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 6
            Write definition of the this case and what it handles TODO
 * @param   None
 * @retval  uint32_t status - 1 if the case is executed to pass, 0 if case is not executed to pass
 */
uint32_t DController::coarseControlCase7()
{
    uint32_t status = 0u;
    int32_t pistonCentreLeft = 0;

    pistonCentreLeft = screwParams.centerPositionCount + screwParams.centerTolerance;

    if(pidParams.pistonPosition > pistonCentreLeft)
    {
        status = 1u;
        pidParams.stepSize = -1 * (motorParams.maxStepSize);

        if(0u == pidParams.centering)
        {
            setControlIsolate();
            pidParams.centering = 1u;
            bayesParams.changeInVolume = 0.0f;
            bayesParams.measuredPressure = absolutePressure;
            bayesParams.changeInPressure = 0.0f;
        }

        else
        {
            bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
            bayesParams.changeInVolume = bayesParams.changeInVolume -
                                         ((float32_t)(pidParams.stepCount) * screwParams.changeInVolumePerPulse);
            estimate();
        }
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 7
            Write definition of the this case and what it handles TODO
 * @param   None
 * @retval  uint32_t status - 1 if the case is executed to pass, 0 if case is not executed to pass
 */
uint32_t DController::coarseControlCase8()
{
    uint32_t status = 0u;
    float32_t totalOvershoot = 0.0f;
    /*
    pump up
    In small volumes a previous controlled vent may have overshot setpoint
    which could trigger an endless loop
    of vent and pump.  Break this loop by detecting if the the pump condition
    was preceded by a vent in the opposite direction.
    */

    totalOvershoot = setPointG + pidParams.overshoot;

    if(totalOvershoot > gaugePressure)
    {
        status = 1u;
        pidParams.stepSize = 0;

        if(0u == pidParams.pumpUp)
        {

#if 0

            if(((1u != pidParams.ventDirDown) ||
                    ((0u == pidParams.ventDirDown) &&
                     (0u == pidParams.ventDirUp))) &&
                    (0u == pidParams.pumpAttempts))
            {
                // previous controlled vent was not in the opposite direction to pump action
                setControlUp();
            }

#else

            if((1u != pidParams.ventDirDown) && (0u == pidParams.pumpAttempts))
            {
                // previous controlled vent was not in the opposite direction to pump action
                setControlUp();
            }

#endif

            else
            {
                /*
                previous control vent overshot setpoint by a more than pumpTolerance
                This can happen if the volume is too small to catch the setpoint during one
                fast control iteration, in which case the control range is considerably larger than
                pumpTolerance anyway.  Assume that is the case and attempt to move to fine control even
                though pressure is not within pumpTolerance range.
                coarse adjustment complete, last iteration of coarse control loop
                */
                setControlIsolate();
                pidParams.fineControl = 1u;
                pidParams.stepSize = 0;
                // PID['position'] = int(screw['readingToPosition'](PID['opticalADC']))
                //pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
                //getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition);
                controllerState = eFineControlLoop;
            }
        }
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 8
            Write definition of the this case and what it handles TODO
 * @param   None
 * @retval  uint32_t status - 1 if the case is executed to pass, 0 if case is not executed to pass
 */
uint32_t DController::coarseControlCase9()
{
    uint32_t status = 0u;
    float32_t totalOvershoot = 0.0f;
    totalOvershoot = setPointG + pidParams.overshoot;

    if(totalOvershoot < gaugePressure)
    {
        status = 1u;
        pidParams.stepSize = 0;

        if(0u == pidParams.pumpDown)
        {
#if 0

            if(((1u != pidParams.ventDirUp) ||
                    ((0u == pidParams.ventDirDown) &&
                     (0u == pidParams.ventDirUp))) &&
                    (0u == pidParams.pumpAttempts))
            {
                /* previous controlled vent was not in the opposite direction to pump action */
                setControlDown();
            }

#else

            if((1u != pidParams.ventDirUp) && (0u == pidParams.pumpAttempts))
            {
                /* previous controlled vent was not in the opposite direction to pump action */
                setControlDown();
            }

#endif

            else
            {
                /*
                previous control vent overshot setpoint by a more than pumpTolerance
                This can happen if the volume is too small to catch the setpoint during one
                fast control iteration, in which case the control range is considerably larger than
                pumpTolerance anyway.  Assume that is the case and attempt to move to fine control even
                though pressure is not within pumpTolerance range.
                coarse adjustment complete, last iteration of coarse control loop
                */
                setControlIsolate();
                pidParams.fineControl = 1u;
                pidParams.stepSize = 0;
                // PID['position'] = int(screw['readingToPosition'](PID['opticalADC']))
                //pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
                //getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition);
                controllerState = eFineControlLoop;
            }
        }
    }

    return status;
}

/**
* @brief    Used for copying data from a 4 byte variable into a buffer from set address
* @param    uint8_t *from - location to be copied from
            uint8_t *to - location to be copied to
            uint32_t length - how many bytes should be copied
* @retval   uint32_t length - returns the number of bytes copied
*/
uint32_t DController::copyData(uint8_t *from, uint8_t *to, uint32_t length)
{
    uint32_t index = 0u;

    for(index = 0u; index < length; index++)
    {
        from[index] = to[index];
    }

    return length;
}

/**
* @brief    Dumps data on the USB VCP for debugging controller state variables
* @param    void
* @retval   void
*/
void DController::dumpData(void)
{

    uint8_t buff[392];
    uint32_t length = 0u;
    uint32_t totalLength = 0u;
    uint32_t ms = 0u;
    sControllerParam_t param;
    param.uiValue = 0u;
    length = 4u;
    getMilliSeconds(&ms);

    /* add to the milisecond timer */
    msTimer = msTimer + (uint32_t)(htim2.Instance->CNT);
    pidParams.elapsedTime = msTimer;
    htim2.Instance->CNT = 0u;

    /* Write header */
    param.uiValue = 0xFFFFFFFFu;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    param.uiValue = 0xFFFFFFFFu;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
#ifdef DUMP_PID_DATA
    /* Write PID Params */
    // 1
    param.uiValue = pidParams.elapsedTime;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 2
    param.floatValue = pidParams.pressureSetPoint;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 3
    param.uiValue = (uint32_t)setPointType;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 4
    param.iValue = pidParams.stepCount;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 5
    param.floatValue = pidParams.pressureError;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 6
    param.floatValue = pidParams.controlledPressure;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 7
    param.floatValue = pidParams.pressureAbs;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 8
    param.floatValue = pidParams.pressureGauge;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 9
    param.floatValue = pidParams.pressureBaro;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 10
    param.floatValue = pidParams.pressureOld;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 11
    param.iValue = pidParams.stepSize;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 12
    param.floatValue = pidParams.pressureCorrectionTarget;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 13
    param.iValue = pidParams.pistonPosition;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 14
    param.floatValue = pidParams.pumpTolerance;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 15
    param.floatValue = pidParams.overshoot;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 16
    param.floatValue = pidParams.overshootScaling;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 17
    param.floatValue = pidParams.ventRate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 18
    param.uiValue = pidParams.holdVentCount;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 19
    param.uiValue = pidParams.mode;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
#endif

#ifdef DUMP_BAYES_DATA
    // 20
    param.floatValue = bayesParams.minSysVolumeEstimate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 21
    param.floatValue = bayesParams.maxSysVolumeEstimate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 22
    param.floatValue = bayesParams.minEstimatedLeakRate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 23
    param.floatValue = bayesParams.maxEstimatedLeakRate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 24
    param.floatValue = bayesParams.measuredPressure;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 25
    param.floatValue = bayesParams.smoothedPresure;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 26
    param.floatValue = bayesParams.changeInPressure;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 27
    param.floatValue = bayesParams.prevChangeInPressure;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 28
    param.floatValue = bayesParams.dP2;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 29
    param.floatValue = bayesParams.estimatedVolume;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 30
    param.uiValue = (uint32_t)(bayesParams.algorithmType);
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 31
    param.floatValue = bayesParams.changeInVolume;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 32
    param.floatValue = bayesParams.prevChangeInVolume;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 33
    param.floatValue = bayesParams.dV2;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 34
    param.floatValue = bayesParams.measuredVolume;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 35
    param.floatValue = bayesParams.estimatedLeakRate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 36
    param.floatValue = bayesParams.measuredLeakRate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 37
    param.floatValue = bayesParams.estimatedKp;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 38
    param.floatValue = bayesParams.sensorUncertainity;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 39
    param.floatValue = bayesParams.uncertaintyPressureDiff;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 40
    param.floatValue = bayesParams.uncertaintyVolumeEstimate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 41
    param.floatValue = bayesParams.uncertaintyMeasuredVolume;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 42
    param.floatValue = bayesParams.uncertaintyVolumeChange;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 43
    param.floatValue = bayesParams.uncertaintyEstimatedLeakRate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 44
    param.floatValue = bayesParams.uncertaintyMeasuredLeakRate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 45
    param.floatValue = bayesParams.maxZScore;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 46
    param.floatValue = bayesParams.lambda;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 47
    param.floatValue = bayesParams.uncerInSmoothedMeasPresErr;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 48
    param.floatValue = bayesParams.targetdP;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 49
    param.floatValue = bayesParams.smoothedPressureErr;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 50
    param.floatValue = bayesParams.smoothedSquaredPressureErr;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 51
    param.floatValue = bayesParams.gamma;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 52
    param.floatValue = bayesParams.predictionError;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 53
    param.iValue = bayesParams.predictionErrType;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 54
    param.uiValue = bayesParams.maxIterationsForIIRfilter;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 55
    param.uiValue = bayesParams.minIterationsForIIRfilter;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 56
    param.floatValue = bayesParams.changeToEstimatedLeakRate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 57
    param.floatValue = bayesParams.alpha;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 58
    param.floatValue = bayesParams.smoothedPressureErrForPECorrection;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 59
    param.floatValue = bayesParams.log10epsilon;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 60
    param.floatValue = bayesParams.residualLeakRate;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 61
    param.floatValue = bayesParams.measuredLeakRate1;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 62
    param.uiValue = bayesParams.numberOfControlIterations;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 63
    param.uiValue = bayesParams.ventIterations;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 64
    param.floatValue = bayesParams.ventInitialPressure;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 65
    param.floatValue = bayesParams.ventFinalPressure;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 66
    param.uiValue = bayesParams.ventDutyCycle;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 67
    param.floatValue = bayesParams.totalVentTime;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 68
    param.uiValue = bayesParams.dwellCount;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // gauge Uncertainty added
    param.floatValue = sensorParams.offset;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
#endif
    // 70
    param.uiValue = (uint32_t)(controllerStatus.bytes);
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 71
    param.uiValue = 0xFEFEFEFEu;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 72
    param.uiValue = 0xFEFEFEFEu;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    PV624->print((uint8_t *)(buff), totalLength);

}

/**
* @brief    Pressure control loop
* @param    void
* @retval   void
*/
void DController::pressureControlLoop(pressureInfo_t *ptrPressureInfo)
{
    if(NULL != ptrPressureInfo)
    {
        /* use values from the pressure sensor and barometer */
        absolutePressure = ptrPressureInfo->absolutePressure;
        gaugePressure = ptrPressureInfo->gaugePressure;
        atmosphericPressure = ptrPressureInfo->atmosphericPressure;
        pressureSetPoint = ptrPressureInfo->pressureSetPoint;
        myMode = ptrPressureInfo->mode;
        setPointType = ptrPressureInfo->setPointType;
        pidParams.pressureAbs = absolutePressure;
        pidParams.pressureGauge = gaugePressure;
        pidParams.pressureBaro = atmosphericPressure;
        pidParams.elapsedTime = ptrPressureInfo->elapsedTime;

        switch(controllerState)
        {
        case eCoarseControlLoopEntry:
            coarseControlSmEntry();
            //resetBayesParameters();
            break;

        case eCoarseControlLoop:
            // run coarse control loop until close to setpoint
            coarseControlLoop();
            //resetBayesParameters();
            break;

        case eCoarseControlExit:
            coarseControlSmExit();
            resetBayesParameters();
            fineControlSmEntry();
            break;

        case eFineControlLoopEntry:
            fineControlSmEntry();
            break;

        case eFineControlLoop:
            // run fine control loop until Genii mode changes from control, or control error occurs
            fineControlLoop();
            break;

        default:
            break;
        }

        dumpData();
    }
}


