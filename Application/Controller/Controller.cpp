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
    // Initialise all the pressure control algorithm parameters
    stateCentre = eCenteringStateNone;  // Only at startup
    totalSteps = 0; // only at startup
    moveTimeout = 0u;
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
    initMainParams();       // State machine states, global pressure variables
    initSensorParams();     // Sensor parameters of PM620 / PM620T
    initPidParams();        // PID controller variables, status bits
    initMotorParams();      // Motor parameters, speed, current, steps of rotation etc
    initScrewParams();      // Screw press parameters, pitch, length, turns etc
    initBayesParams();      // Bayes parameters to determine the probability of the measurement being corect
    initTestParams();       // Parameters used for testing for boundary conditions
    initCenteringParams();  // Motor centering parameters
}

/**
* @brief    Initializes all high level pressure control parameters
* @param    void
* @retval   void
*/
void DController::initMainParams(void)
{
    /*Initialise the controller state machine to be started at coarse control entry state. This will do excess pressure
    venting if the base is pressurized at startup and also detect if the connected sensor has an offset */
    controllerState = eCoarseControlLoopEntry;

    msTimer = 0u;               // msTimer variable used for coarse control loop iteration time keeping, init to 0
    entryState = 0u;            // Coarse control entry state machine is initialized to state 1

    // Pressure parameters initialization
    setPointG = 0.0f;           // Set point value in mbar g
    pressureSetPoint = 0.0f;    // Set point value as received from DPI620G based on abs or gauge pressure type
    absolutePressure = 0.0f;    // Current measured pressure in mbar abs
    gaugePressure = 0.0f;       // Current measured pressure in bar gauge
    atmosphericPressure = 0.0f; // Current atmospheric pressure read from the barometer

    // Controller status
    controllerStatus.bytes = 0u;    // Controller status bits union contains current status of the controller

    // Sensor parameters
    fsValue = 7000.0f;              // Initialization of a full scale value to a non zero number to avoid division by 0
    sensorFsValue = 20000.0f;       // Init to 0.0 TODO

    // Startup venting parameters
    myMode = E_CONTROLLER_MODE_VENT;    // Initialize the starting mode to VENT to vent out excess pressure
    entryIterations = 0u;           // Number of iterations to be waited for to check vent detection
    entryInitPressureG = 0.0f;      // Entry pressure before start of venting
    entryFinalPressureG = 0.0f;     // Entry pressure at the end of venting

    // Startup centering parameters
    //stateCentre = eCenteringStateNone;  // Init the starting state for centering to none
    //totalSteps = 0;                     // Set the total steps taken by the motor to 0

    // LED control in coarse control
    previousError = 0u;                 // Previous error variable to check if same errors are occuring again
    ledFineControl = 0u;                // Used to set LED in fine control state
    myPrevMode = E_CONTROLLER_MODE_VENT;    // Vent mode set as previous operational mode for LED control
}

/**
* @brief    Init sensor parameters used by the control algorithm
* @param    void
* @retval   void
*/
void DController::initSensorParams(void)
{
    sensorParams.fullScalePressure = 7000.0f;   // Set to non zero number to avoid div by 0 problems
    sensorParams.pressureType = 1u;             // 0 - Abs, 1 for Gauge
    sensorParams.sensorType = 1u;               // 0 - TERPS, 1 - PIEZO
    sensorParams.terpsPenalty = 1u;             // ppm penalty for the terps as compared to the piezo pm
    sensorParams.minGaugeUncertainty = 5.0f;    // Minimum uncertainty in measured pressure from pm sensor
    sensorParams.gaugeUncertainty = sensorParams.minGaugeUncertainty;
    sensorParams.maxOffset = 60.0f;             // Maximum offset to set excess offset flag
    sensorParams.offset = 0.0f;                 // Measured offset variable
    sensorParams.offsetSafetyFactor = 1.5f;     // Offset safety factor, used as offset estimation may be incorrect
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

    // Initialize status bit variables to 0
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

    /* The vent valve now operates in time modulation and PWM modes based on the required action
    TDM mode is used to perform controlled venting in coarse control and rate control modes, where the valve is
    pulsed at pulses ranging from 500 us to 6000 us in increments of 10 us based on pressure change required and
    measured. This happens at PM slow read iteration rate of 70ms for PM620 and 80ms for PM620T */
    screwParams.minVentDutyCycle = 500u;  // Min vent duty cycle during vent, in us
    screwParams.maxVentDutyCycle = 6000u;  // Max vent duty cycle during vent in us
    screwParams.ventDutyCycleIncrement = 10u;    // Duty cycle increment per iteration

    // Following parameters for vent valve operated in PWM mode while venting and to save battery
    /* Clock to the timer module in PWM mode is changed to 48 MHz as we are using a PWM frequency of 25kHz.
    In fast vent mode 50% duty is applied to the valve. Duty cycles are represented as % x 10.
    In Hold vent mode which is triggered after completion of venting, 20% pwm is applied to the to keep it just open */
    screwParams.holdVentDutyCycle = 200u; // vent duty cycle when holding vent at 20% pwm
    screwParams.maxVentDutyCyclePwm = 500u;   // Maximum vent duty while venting
    screwParams.holdVentInterval = 50u;  //number of control iterations between applying vent pulse
    screwParams.ventResetThreshold = 0.5f; // reset threshold for setting bayes ventDutyCycle to reset
    screwParams.maxVentRate = 1000.0f;   // Maximum controlled vent rate / iteration in mbar / iteration
    screwParams.minVentRate = 1.0f;   // Minimum controlled vent rate / iteration in mbar / iteration
    screwParams.ventModePwm = 1u;   // for setting valve 3 to be pwm as status bit
    screwParams.ventModeTdm = 0u;   // for setting valve 3 to be tdm as status bit
    screwParams.holdVentIterations = (uint32_t)((float32_t)(screwParams.holdVentInterval) * 0.2f); // cycs to hold vent

    // For distance travelled
    /*
    screwParams.distancePerStep = (float32_t)(screwParams.leadScrewPitch) /
                                  (float32_t)((360.0f * screwParams.microStep) / screwParams.motorStepSize); */

    screwParams.distancePerStep = (screwParams.leadScrewPitch / 360.0f) *
                                  (screwParams.motorStepSize / screwParams.microStep);

    screwParams.distanceTravelled = 0.0f;   // init to 0
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
    // Aadded as debug variable for ease of calculation
    bayesParams.dP2 = 0.0f;
    // etimate of volume(mL), set to minV to give largest range estimate on startup
    bayesParams.estimatedVolume = bayesParams.maxSysVolumeEstimate;
    // algorithm used to calculate V
    bayesParams.algorithmType = eMethodNone;
    // volume change from previous stepSize command(mL)
    bayesParams.changeInVolume = 0.0f;
    // previous dV value(mL), used in regression method
    bayesParams.prevChangeInVolume = 0.0f;
    // Added as a debug variable for ease of calculation
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
    bayesParams.sensorUncertainity = (20e-6f * fsValue) * (20e-6f * fsValue);
    // Calculated expected uncertainty in the pressure difference
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
    //totalSteps = 0;
}

/**
* @brief    Moves motor to the maximum end to the fully extended position
* @param    void
* @retval   uint32_t motorMax - 0 if not maxed, 1 if maxed
*/
uint32_t DController::moveMotorMax(void)
{
    uint32_t optMax = 1u;       // Set to one as this is an active low signal read from GPIO
    uint32_t motorMax = 0u;     // Variable to hold if motor has hit max end stop
    int32_t readSteps = 0;      // Read steps taken by the motor during the last command
    int32_t steps = 0;          // Steps expected by the motor to be taken
    optMax = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

    if(0u == optMax)
    {
        // Motor has hit the end stop, count last number of steps taken
        steps = 0;
        PV624->stepperMotor->move(steps, &readSteps); // Write 0 steps so motor will stop and returned value will be 0
        calcDistanceTravelled(readSteps);   // Since motor has moved earlier, update the total distance travelled
        readSteps = 0;
        motorMax = 1u;  // Set the max position flag
    }

    else
    {
        // Set the motor step size to max, to move the motor at the max speed to the fully compressed end stop position
        steps = screwParams.maxStepSize;
        PV624->stepperMotor->move(steps, &readSteps);
        calcDistanceTravelled(readSteps); // Since motor has moved earlier, update the total distance travelled
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
        // Motor has hit the end stop, count last number of steps taken
        steps = 0;
        PV624->stepperMotor->move(steps, &readSteps);   // Write 0 steps so motor will stop and returned value will be 0
        calcDistanceTravelled(readSteps);   // Since motor has moved earlier, update the total distance travelled
        readSteps = 0;
        motorMin = 1u;  // Set the min position flag
    }

    else
    {
        // Set the motor step size to max, but negative, to move the motor at the max speed to the fully retracted
        // end stop position
        steps = -1 * screwParams.maxStepSize;
        PV624->stepperMotor->move(steps, &readSteps);
        calcDistanceTravelled(readSteps);   // Since motor has moved earlier, update the total distance travelled
    }

    return motorMin;
}

/**
* @brief    Moves motor to the center position
* @param    int32_t steps - positive max steps if at retracted end, negative if at compressed end
* @retval   uint32_t centered - 0 if not centered, 1 if centered
*/
uint32_t DController::moveMotorCenter(int32_t setSteps)
{
    uint32_t centered = 0u;
    int32_t readSteps = 0;
    int32_t steps = 0;
    //float32_t totalDistanceTravelled = 0.0f;

    // Check if the total steps accumlated while returning to the center position are within the center tolerance of
    // 3000 steps. For a 40mm screw length the typical total steps the motor would have to take is 53k steps. The center
    // position of the screw would be at 26500. With a tolerance of 3000 steps the center position would be between
    // 23500 and 29500 steps
    if((totalSteps >= (screwParams.centerPositionCount - screwParams.centerTolerance)) &&
            (totalSteps <= (screwParams.centerPositionCount + screwParams.centerTolerance)))
    {
        steps = 0;
        PV624->stepperMotor->move(steps, &readSteps);   // Write 0 steps so motor will stop and returned value will be 0
        calcDistanceTravelled(readSteps);       // Since motor has moved update the distance travelled
        pidParams.totalStepCount = totalSteps;  // After centering, set the pid counter to the totalSteps as the center
        totalSteps = 0;
        readSteps = 0;
        centered = 1u;  // Set the centered flag

        /* Write the value of the distance travelled by the piston to the eeprom here as this is only called at
        startup */
        PV624->updateDistanceTravelled(screwParams.distanceTravelled);
        screwParams.distanceTravelled = 0.0f;
    }

    else
    {
        steps = setSteps;
        PV624->stepperMotor->move(steps, &readSteps);   // Move the motor to the center as per the steps set
        calcDistanceTravelled(readSteps);       // Since motor has moved update the distance travelled
        totalSteps = readSteps + totalSteps;    // Accumulate the last taken steps
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
    uint32_t centered = 0u;

    uint32_t stopReached = 0u;

    switch(stateCentre)
    {
    case eCenteringStateNone:
        // Earlier state was none, so start centering to the minimum (fully retracted end stop)
        stateCentre = eCenteringStateMin;
        moveTimeout = 0u;
        break;

    case eCenteringStateMin:
        // Move the motor towards the minimum end stop
        stopReached = moveMotorMin();

        // If timeout has exceeded, then there is a HW failure before the motor has reached the end
        if(END_STOP_TIMEOUT <= moveTimeout)
        {
            // Signal failure, this is a non recoverable error
            moveTimeout = 0u;
            stateCentre = eCenterFailed;
        }

        else
        {
            // Increment timeout, motor typically takes about 12s to move the max screw length
            moveTimeout = moveTimeout + 1u;

            if(1u == stopReached)
            {
                // If stop is reached within time, start the motor centering
                stateCentre = eCenterMotor;
                moveTimeout = 0u;
            }
        }

        break;

    case eCenterMotor:
        // Start centering the motor from minimum position in this startup routine, so steps are positive as we
        // are moving towards the fully extended position
        stopReached = moveMotorCenter(MAX_MOTOR_STEPS_POS);

        // Centering timeout is about half of the full length timeout
        if(CENTER_TIMEOUT <= moveTimeout)
        {
            // Signal failure to center if not reached within time, this is a non recoverable error
            moveTimeout = 0u;
            stateCentre = eCenterFailed;
        }

        else
        {
            // Inrement timeout, motor typically takes about 6 seconds to center from either end stop
            moveTimeout = moveTimeout + 1u;

            if(1u == stopReached)
            {
                // Stop reached successfully, set the centering state to none if there is a re centering need
                stateCentre = eCenteringStateNone;
                stopReached = 0u;
                moveTimeout = 0u;
                centered = 1u;  // Set the centered flag to 1
            }
        }

        break;

    case eCenterFailed:
        // actions when centering has failed TODO
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
    // Only the following parameters are required to be reset at the change of control state
    // Other parameters are calculated based on the values of these
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
* @brief    Returns the sign of a float variable. The return from this function is used for performing float signed
            calculations
* @param    void
* @retval   float32_t sign 1.0 if positive, -1.0 if negative
*/
float32_t DController::getSign(float32_t value)
{
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
* @brief    Run the controller in measure mode. In this mode, the pump is isolated and only customer volume is
            connected to the manifold. The PM620 / PM620T measures pressure at the customer port
            The PM is read at slow rate in this mode
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

    // Set the PID status variables as in measure mode, only the measure mode bit needs to be 1
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
* @brief    Set controller state and valves when pump up is required. In this mode, the inlet valve is opened and the
            manual pump is connected to the manifold. Any pumping on the pump shall result in pressure increase in the
            manifold and thereby the customer volume
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

    // Set the PID status variables when a customer pumping action is required
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
* @brief    Set controller state and valves when pump down is required. In this mode the outlet valve is opened and the
            pump is connected to the manifold. Any pump action shall result in the pressure decrease in the manifold
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

    // Set the PID status variables when a customer pumping down action is required
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
* @brief    Set controller state and valves when pump isolation is required, mostly in fine control. In this case,
            the stepper motor shall be operated to move the piston in order to control pressure
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

    // Set the PID status variables when customer volume is isolated from the pump. This is similar to measure mode
    // only could be running fine control or controlled venting
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
* @brief    Set controller state and valves when piston centering is required. This state shall arise when there is a
            set point change and the previous set point has caused the piston to be left off centre. Centering the
            piston shall provide the largest possible range of the lead screw for the control action in either
            pressure or vacuum
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

    // Status variables when the piston is being centered in control mode
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
* @brief    Sets the vent configuration. This configures the vent valve in the PWM state and opens the same to vent
            any pressure in the manifold.
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

    // Status variables set when in vent mode
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
* @brief    Sets the valves in controlled vent configuration. The vent valve is operated in pulsed mode and pressure is
            released based on the time for which the valve is kept open.
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

    // Status variables set when in controlled vent mode
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

    // Status variables set in fast vent mode
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

    // Status variable set in control rate mode
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
* @param    int32_t stepsMoved - last number of steps taking by the motor
* @retval   void
*/
void DController::calcDistanceTravelled(int32_t stepsMoved)
{
    float32_t absSteps = 0.0f;

    /* Distance can only be positive, so take absolute of the last steps taken by the motor, which could be negative
    if the motor has moved in the retracted direction */
    absSteps = (float32_t)(stepsMoved);
    absSteps = fabs(absSteps);

    // Distance = stepsTaken * distancePerStep
    // Set the distance travelled in the screw structure
    screwParams.distanceTravelled = screwParams.distanceTravelled +
                                    (absSteps * screwParams.distancePerStep);
}

/**
* @brief    Pulses the vent valve in its set configuration from other functions
            setVent - PWM
            setControlledRate - TDM
            setFastVent - PWM
            setControlledVent - TDM
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
* @brief    This function estimates calculated parameters as follows:
            Volume
            Leak rate
            New required Kp
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
    float32_t residualL = 0.0f; // Local variable to hold residual leak rate calculation

    // Set the measured volume to 0
    bayesParams.measuredVolume = 0.0f;
    bayesParams.uncertaintyMeasuredVolume = bayesParams.maxSysVolumeEstimate;   // Set the uncertainty to max
    bayesParams.algorithmType = eMethodNone;    // Set algorithm type to none, to be selected later

    /*
    if PID['controlledVent'] == 1 and bayes['ventIterations'] > screw['ventDelay'] and \
    bayes['initialP'] / bayes['finalP'] > 1:
    */

    if(1u == pidParams.fineControl)
    {
        /*
        do this in fine control mode only
        calculate prediction error and error type from last control iteration
        prediction error(mbar)
        */
        bayesParams.predictionError = bayesParams.changeInPressure - bayesParams.targetdP;
        /*
        predictionErrorType > 0 -- > excessive correction, reduce system gain to fix(decrease volume estimate)
        predictionErrorType < 0 --> insufficient correction, increase system gain to fix(increase volume estimate)
        bayesParams.predictionErrType = np.sign(bayes['targetdP']) * np.sign(bayes['predictionError'])
        */

        float32_t signTargetDp = 0.0f;  // Hold the sign of the target pressure correction difference
        float32_t signPredictionError = 0.0f;   // Holds the sign of the prediction error

        signTargetDp = getSign(bayesParams.targetdP);
        signPredictionError = getSign(bayesParams.predictionError);

        bayesParams.predictionErrType = (int32_t)(signTargetDp * signPredictionError);
        // low pass filtered pressure error(mbar)
        bayesParams.smoothedPressureErr = pidParams.pressureError * bayesParams.lambda +
                                          bayesParams.smoothedPressureErr * (1.0f - bayesParams.lambda);

        /*
        correction to varE estimate Nov 18 2021
        low pass filtered squared error (mbar**2)
        (not used)
        bayes['smoothE2'] = (PID['E']**2) * bayes['lambda'] + bayes['smoothE2'] * (1-bayes['lambda'])
        dynamic estimate of pressure error variance
        see https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        http://cpsc.yale.edu/sites/default/files/files/tr222.pdf
        Chan, Tony F.; Golub, Gene H.; LeVeque, Randall J. (1983).
        "Algorithms for computing the sample variance: Analysis and recommendations" (PDF).
        The American Statistician. 37 (3): 242–247. doi:10.1080/00031305.1983.10483115. JSTOR 2683386.
        Note: varE estimate not valid when smoothE2 >> varE
        bayes['varE'] = bayes['smoothE2'] - bayes['smoothE']**2
        Use Expontially Weighted Moving variance estimate
        https://en.wikipedia.org/wiki/Moving_average#cite_note-13
        Finch, Tony. "Incremental calculation of weighted mean and variance" (PDF).
        University of Cambridge. Retrieved 19 December 2019.
        */

        // Following variables are added for aiding in debugging the correctness of the calculations
        float32_t pressureErrTemp = 0.0f;           // Holds the pressure error
        float32_t pressureErrorSquareTemp = 0.0f;   // Hold the squared pressure error
        float32_t lambdaTemp = 0.0f;                // Holds the temporary lambda value

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
        // Following variables are added for aiding in debugging the correctness of the calculations
        float32_t dP2 = 0.0f;
        float32_t dV2 = 0.0f;
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
        // Following variables are added for aiding in debugging the correctness of the calculations
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

            // Following variables are added for aiding in debugging the correctness of the calculations
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
            // Following variables are added for aiding in debugging the correctness of the calculations

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
* @retval   float32_t pressureValue - the calculated pressure value as per set point type
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
 * @param   int32*t *position - pointer to the value of the position of the piston
 * @retval  uint32_t status - Is this required? TODO
 */
uint32_t DController::getPistonPosition(int32_t *position)
{
    uint32_t status = 0u;

    *position = pidParams.totalStepCount;

    return status;
}

/**
 * @brief   Returns if two float values are almost equal - float comparison upto 10 ^-10.
 * @param   None
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
void DController::fineControlLoop()
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
        fineControlLed();   // Glow the status LED to indicate PV624 is doing fine control on pressure

        // If gaurds if the mode has changed while running fine control, or one of excess Leak, excess Volume
        // over pressure flags have been set
        // If while controlling the piston hits one of the end stops, the range exceeded flag is set
        // In all above cases, exit fine control
        if((myMode == E_CONTROLLER_MODE_CONTROL) &&
                (pidParams.excessLeak != 1u) &&
                (pidParams.excessVolume != 1u) &&
                (pidParams.overPressure != 1u) &&
                (pidParams.rangeExceeded != 1u))
        {
            // Read all the input parameters into the pid and bayes structures required for fine control
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            pidParams.pressureSetPoint = pressureSetPoint;

            bayesParams.prevChangeInPressure = bayesParams.changeInPressure;    // previous change in pressure
            bayesParams.prevChangeInVolume = bayesParams.changeInVolume;    // previously calculated change in volume
            /*
            change in pressure from last measurement(mbar), +ve == increasing pressure
            fake leak is only introduced to check that excessive leak has an effect in estimation and generates excess
            leak flag as set. In operational cases it is set to 0
            */
            bayesParams.changeInPressure = absolutePressure + testParams.fakeLeak - bayesParams.measuredPressure;

            // use absolute pressure for volume estimates using gas law
            bayesParams.measuredPressure = absolutePressure + testParams.fakeLeak;

            // store previous pressure error for prediction error calculation,
            bayesParams.targetdP = pidParams.pressureError;

            /* Note: Unlike pidParams.targetDp, bayesParams.targetDp is not adjusted for leak estimate.
            The leak estimate may be incorrect and the bayes prediction error
            should therefore be non - zero even if the PID['targetdP'] was achieved.
            pressure error error(mbar), -ve if pressure above setpoint */
            pidParams.pressureError = pidParams.pressureSetPoint - pidParams.controlledPressure;
            /* leak estimation is performed in estimate and then used for pressure compensation and set as the pressure
            correction target after anticipating leak effect(mbar)
            PID['targetdP'] = PID['E'] - bayes['leak']
            */
            pidParams.pressureCorrectionTarget = pidParams.pressureError - bayesParams.estimatedLeakRate;
            /* steps to take to achieve zero pressure error in one control iteration
            PID['stepSize'] = int(round(bayes['kP'] * PID['targetdP']))
            stepSize is always an integer hence the closest value from a floating point typecast would be to round
            the data before casting */

            // Document reference: TODO
            pidParams.stepSize = int(round(bayesParams.estimatedKp * pidParams.pressureCorrectionTarget));
            /* abort correction if pressure error is within the measurement noise floor to save power,
            also reduces control noise when at setpoint without a leak */

            // Local variables added for storing absolute values and square roots and help in debugging
            float32_t fabsPressCorrTarget = fabs(pidParams.pressureCorrectionTarget);
            float32_t sqrtUncertaintyPressDiff = sqrt(bayesParams.uncertaintyPressureDiff);
            float32_t fabsPressureError = fabs(pidParams.pressureError);


            if((fabsPressCorrTarget < sqrtUncertaintyPressDiff) &&
                    (fabsPressureError < sqrtUncertaintyPressDiff))
            {
                pidParams.pressureCorrectionTarget = 0.0f;
                pidParams.stepSize = 0u;
            }

            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);
            calcDistanceTravelled(pidParams.stepCount);
            pidParams.pistonPosition = pidParams.pistonPosition + pidParams.stepCount;
            // change in volume(mL)
            bayesParams.changeInVolume = -1.0f * screwParams.changeInVolumePerPulse * (float32_t)(pidParams.stepCount);

            checkPiston();

            estimate();
            calcStatus();
            /* Code below used for testing controller performance, not for product use :
            ALL VALUES ARE SET TO 0
            -----------------------------------------------------------------------
            scale maxFakeLeakRate by measured gage pressureand test volume
            Don't use volume estimate to scale leak rate
            so that errors in the estimate do not affect applied leak rate
            and the true leak rate can be back - calculated from pressure and nominal test volume. */

            testParams.fakeLeakRate = -fabs(testParams.maxFakeLeakRate) * \
                                      gaugePressure / screwParams.maxAllowedPressure *
                                      screwParams.nominalTotalVolume / testParams.volumeForLeakRateAdjustment;

            testParams.fakeLeak = testParams.fakeLeak + testParams.fakeLeakRate;
            // Keep running in fine control loop until one of the error flags is set
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
                // Three lines of code only for verification while debug, can be changed to one line TODO
                pidParams.pumpTolerance = bayesParams.maxSysVolumeEstimate / bayesParams.estimatedVolume;
                pidParams.pumpTolerance = pidParams.minPumpTolerance * pidParams.pumpTolerance;
                pidParams.pumpTolerance = min(pidParams.pumpTolerance, pidParams.maxPumpTolerance);
            }

            else
            {
                /* Range exceeded in the previous fine control attempt, volume estimate cannot be trusted and cannot
                be used for further pump tolerance calculations. Hence set to minimum pump tolerance that will
                require the pressure to be within smallest amount of tolerance band to re enter fine control */
                pidParams.pumpTolerance = pidParams.minPumpTolerance;
            }

            // Change the controller state to run the coarse control loop
            controllerState = eCoarseControlLoop;
            // Before exiting, calculate the controller status to be sent to the DPI620G on request
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
            Controlling - Yellow blinking @ 4 Hz
            Venting - Yellow blinking at 1Hz
            Rate control - Yellow blinking at 4 Hz
 * @param   None
 * @retval  None
 */
void DController::coarseControlLed(void)
{
    // Check if any other errors exist in the system
    deviceStatus_t devStat;     // actual errors in the device
    deviceStatus_t tempStatus;  // Temp error variable as a mask

    eSysMode_t sysMode = E_SYS_MODE_NONE;

    devStat.bytes = 0u;
    tempStatus.bytes = 0u;

    devStat = PV624->errorHandler->getDeviceStatus();

    uint32_t systemError = 0u;

    // Following bits must not be checked by the controller code to know if the system is any error, set those bits
    tempStatus.bit.dueForService = 1u;
    tempStatus.bit.chargingStatus = 1u;
    tempStatus.bit.remoteRequestFromBtMaster = 1u;
    tempStatus.bit.remoteRequestFromOwiMaster = 1u;
    // Invert the value to get the mask
    tempStatus.bytes = ~(tempStatus.bytes);

    /* Don't update LEDs if power status of the controller is oFF */
    sysMode = PV624->getSysMode();

    if((eSysMode_t)E_SYS_MODE_RUN == sysMode)
    {
        if(tempStatus.bytes & devStat.bytes)
        {
            // System is in an error state
            systemError = 1u;
        }

        else
        {
            // System is free of any errors
            systemError = 0u;
        }

        if(tempStatus.bytes & devStat.bytes)
        {
            // For any errors other than charging, owi or ble remote request do not override error handler
            previousError = 1u;
        }

        else
        {
            /* Only update leds if mode has changed, if the system has recovered from an error state, or if the control
            loop has moved from fine control state to coarse control */
            if((myMode != myPrevMode) ||
                    ((0u == systemError) && (1u == previousError)) ||
                    (1u == ledFineControl))
            {
                if(1u == previousError)
                {
                    // Clear as the system was in error previously
                    previousError = 0u;
                }

                if(1u == ledFineControl)
                {
                    // Reset the fine control led variable to be used again while glowing leds when in fine control state
                    ledFineControl = 0u;
                }

                if((eControllerMode_t)E_CONTROLLER_MODE_MEASURE == myMode)
                {
                    // Replace 65535u with max time
                    // In measure mode, turn on the green LED
                    PV624->userInterface->statusLedControl(eStatusOkay,
                                                           E_LED_OPERATION_SWITCH_ON,
                                                           65535u,
                                                           E_LED_STATE_SWITCH_ON,
                                                           1u);
                }

                else if(((eControllerMode_t)E_CONTROLLER_MODE_CONTROL == myMode) ||
                        ((eControllerMode_t)E_CONTROLLER_MODE_RATE == myMode))
                {
                    /* Since the earlier state of the LEDs is not known, turning on a multicolor LED may happen when one
                    of the LEDs is already ON. To protect against this, turn on the LED first then light in the yellow
                    colour and blink at fast rate to indicate coarse control / rate control */
                    PV624->userInterface->statusLedControl(eStatusProcessing,
                                                           E_LED_OPERATION_SWITCH_OFF,
                                                           65535u,
                                                           E_LED_STATE_SWITCH_OFF,
                                                           2u);
                    PV624->userInterface->statusLedControl(eStatusProcessing,
                                                           E_LED_OPERATION_TOGGLE,
                                                           65535u,
                                                           E_LED_STATE_SWITCH_OFF,
                                                           1u);
                }

                else
                {
                    /* Since the earlier state of the LEDs is not known, turning on a multicolor LED may happen when one
                    of the LEDs is already ON. To protect against this, turn on the LED first then light in the yellow
                    colour and blink at slow rate to indicate vent mode */
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
            }

            // Set the previous mode as mode to check in the next iteration if the mode has changed
            myPrevMode = myMode;
        }
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
    // When in fine control, glow the yellow LED
    eSysMode_t sysMode = E_SYS_MODE_NONE;

    sysMode = PV624->getSysMode();

    if((eSysMode_t)E_SYS_MODE_RUN == sysMode)
    {
        if(0u == ledFineControl)
        {
            PV624->userInterface->statusLedControl(eStatusProcessing,
                                                   E_LED_OPERATION_SWITCH_ON,
                                                   65535u,
                                                   E_LED_STATE_SWITCH_OFF,
                                                   1u);
            ledFineControl = 1u;    // Set the LED on flag to 1, is used in coarse control LED that state has switched
        }
    }
}

/**
 * @brief   Measure mode in coarase control - slow measurement of pressure with isolated pump
            Pumping has no effect in this mode
            Control action and motor are disabled
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlMeasure()
{
    uint32_t status = 0u;
    controllerStatus.bytes = 0u;
    pidParams.ccError = eCoarseControlErrorReset;
    return status;
}

/**
 * @brief   Checks and returns if the piston is in range by comparing it with the screw min and max positions
 * @param   int32_t position - piston position from pid params in terms of number of steps
 * @retval  ePistonRange_t isInRange
            outside of min / max position - ePistonOutOfRange
            Within limits - ePistonInRange
 */
ePistonRange_t DController::validatePistonPosition(int32_t position)
{
    ePistonRange_t isInRange = ePistonOutOfRange;   // Init as out of range (in error)

    if(screwParams.maxPosition > position)          // Check if last position was greater than the screw max
    {
        if(screwParams.minPosition < position)      // Check if the last position was less than the screw min
        {
            isInRange = ePistonInRange;     // If not, piston is in range
        }
    }

    return isInRange;
}

/**
 * @brief   Checks and returns if the piston is in center by comparing it with center position with a tolerance
 * @param   int32_t position - piston position from pid params in terms of steps
 * @retval  ePistonCentreStatus_t isInCentre
            If within tolerance - ePistonCentered
            Outside of tolerance - ePistonOutOfCentre
 */
ePistonCentreStatus_t DController::isPistonCentered(int32_t position)
{
    int32_t positionLeft = 0;
    int32_t positionRight = 0;
    ePistonCentreStatus_t isInCentre = ePistonOutOfCentre;

    // Calculate the left and right (extended and retracted) side tolerances to determine if the piston is centered
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
    ePistonCentreStatus_t isCentered = ePistonOutOfCentre;  // Init as out of centre

    /* Read the optical sensors to find if any of the sensors has triggered. The motor isn't fast enough to miss the
    optical sensor catching the piston flag within time even with a 70 ms iteration rate. The 4mm flag will take at
    least 5 iterations of the 70 ms read rate at maximum motor speed. Hence, we can safely detect the flag while
    preventing the motor from overshooting */
    getOptSensors(&optSensorMin, &optSensorMax);

    if(0u == optSensorMax)
    {
        /* If any of the two optical sensors is triggered, that means the piston range has exceeded. In this case,
        set the rangeExceeded flag in the pid structure. */
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
        pidParams.rangeExceeded = 0u;   // Otherwise, range has not exceeded
    }

    // Check if piston is centered, this should be done only if range has not exceeded
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
    uint32_t epochTime = 0u;    // PV624 time keeping is in epoch time
    uint32_t status = false;    // Case execution status
    uint32_t sensorType = 0u;           // 1 - PM620 Piezo, 2 - TERPS
    float32_t absSensorOffset = 0.0f;   // Sensor offset can be negative, need positive value for computation
    float32_t varDpTemp = 0.0f;         // Uncertainty in the difference in pressure, local variable
    float32_t uncertaintyScaling = 0.0f;//   uncertainty scaling factor for sensor

    status = getEpochTime(&epochTime);

    if(true == status)
    {
        coarseControlLogParams.startTime = epochTime;
        pidParams.elapsedTime = epochTime;
    }

    /* Reset the pid param error flags */
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
        /* Run this loop in a state machine until pressure stops changing by 2 sigma per iteration. Higher variation
        than that could mean that the unit is not vented and could contain pressure. Run the check for a 100 iterations
        at max and if there is higher variance, exit and see if there is excessive sensor offset

        May not run a 100 iterations if the unit starts fully vented and change in pressure is less than 2 sigma and
        will startup faster

        Maximum startup time will be 100 * 80ms read rate = 8 seconds */
        varDpTemp = sqrt(bayesParams.uncertaintyPressureDiff);
        varDpTemp = 2.0f * varDpTemp;

        if(bayesParams.changeInPressure > varDpTemp)
        {
            entryIterations = entryIterations + 1u;
            entryInitPressureG = entryFinalPressureG;   // Equate to previous measured pressure
            entryFinalPressureG = gaugePressure;        // Current measured pressure in this iteration
            bayesParams.changeInPressure = fabs(entryFinalPressureG - entryInitPressureG);

            if(entryIterations > 100u)
            {
                /* Set sensor offset to 0 as offset could not be measured in that last 100 iterations possibly
                due to unstable pressure conditions */
                sensorParams.offset = 0.0f;
                absSensorOffset = fabs(sensorParams.offset * 1.5f);
                entryState = 3u;
            }

            else
            {
                // Set sensor offset to last measured pressure
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

        /* If the offset including safety factor is greater than max offset of 60 mbar, then set the excess offset
        flag. WHile this does not stop the unit operation, accuracy in the pressure control could be impacted */
        if((absSensorOffset * sensorParams.offsetSafetyFactor) > sensorParams.maxOffset)
        {
            pidParams.excessOffset = 1u;
        }

        setMeasure();   // Set the unit into measure mode to save power spent in switching the vent valve
        calcStatus();   // Calculate the controller status to be sent to the DPI620G

        // Clear all PID error flags
        pidParams.stable = 0u;
        pidParams.excessLeak = 0u;
        pidParams.excessVolume = 0u;
        pidParams.overPressure = 0u;

        /* Reset the startup entry states, as if the controller is re initialized possibly after recovery from an error,
        the offset detection will have to be run again for a new PM sensor */

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
        // Finally enter the coarse control loop in which the unit will take measurement and control decisions
        controllerState = eCoarseControlLoop;
    }

    else
    {
        // For misra, no case
    }
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
    uint32_t caseStatus = 0u;   // Flag used to detect if a coarse control case has executed while in control mode
    uint32_t epochTime = 0u;    // Epoch time used for time keeping
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

    /* Run coarse control only if fine control is not enabled, situation could only occur if a mode change has occured
    before the unit initialization operations are not complete */
    if(0u == pidParams.fineControl)
    {
        if((E_CONTROLLER_MODE_MEASURE == myMode) && (1u == pidParams.control))
        {
            /* Mode set by genii is measure but PID is in control mode
            Change mode to measure */
            setMeasure();
            /* Mode changed to measure, write the distance travelled by the controller to EEPROM. Only write the EEPROM
            in measure mode as no control actions and motor movement is allowed. Hence the distance changed values stay
            constant */
            PV624->updateDistanceTravelled(screwParams.distanceTravelled);
            screwParams.distanceTravelled = 0.0f;// Reset the total distance travelled as accumulation happens elsewhere
        }

        else if((E_CONTROLLER_MODE_MEASURE == myMode) && (1u == pidParams.venting))
        {
            /* Mode set by genii is measure but PID is venting
            Change mode to measure */
            setMeasure();
            /* Mode changed to measure, write the distance travelled by the controller to EEPROM. Only write the EEPROM
            in measure mode as no control actions and motor movement is allowed. Hence the distance changed values stay
            constant */
            PV624->updateDistanceTravelled(screwParams.distanceTravelled);
            screwParams.distanceTravelled = 0.0f;
        }

        else if((E_CONTROLLER_MODE_MEASURE == myMode) && (1u == pidParams.controlRate))
        {
            /* Mode set by genii is measure but PID is doing control rate
            Change mode to measure */
            setMeasure();
            /* Mode changed to measure, write the distance travelled by the controller to EEPROM. Only write the EEPROM
            in measure mode as no control actions and motor movement is allowed. Hence the distance changed values stay
            constant */
            PV624->updateDistanceTravelled(screwParams.distanceTravelled);
            screwParams.distanceTravelled = 0.0f;
        }

        else if((E_CONTROLLER_MODE_CONTROL == myMode) && (1u == pidParams.measure))
        {
            /* Mode is changed to control, pump should be isolated from the manifold and only connected if a pump up
            or pump down is required */
            setControlIsolate();
        }

        else if((E_CONTROLLER_MODE_CONTROL == myMode) && (1u == pidParams.venting))
        {
            /* Mode is changed to control, pump should be isolated from the manifold and only connected if a pump up
            or pump down is required */
            setControlIsolate();
        }

        else if((E_CONTROLLER_MODE_CONTROL == myMode) && (1u == pidParams.controlRate))
        {
            /* Mode is changed to control, pump should be isolated from the manifold and only connected if a pump up
            or pump down is required */
            setControlIsolate();
        }

        else if((E_CONTROLLER_MODE_VENT == myMode) && (1u == pidParams.measure))
        {
            /* Mode is changed to vent, open the vent valve to release pressure. This opens the vent valve maximum for
            the fastest release of pressure to atmosphere */
            setVent();
        }

        else if((E_CONTROLLER_MODE_VENT == myMode) && (1u == pidParams.control))
        {
            /* Mode is changed to vent, open the vent valve to release pressure. This opens the vent valve maximum for
            the fastest release of pressure to atmosphere */
            setVent();
        }

        else if((E_CONTROLLER_MODE_VENT == myMode) && (1u == pidParams.controlRate))
        {
            /* Mode is changed to vent, open the vent valve to release pressure. This opens the vent valve maximum for
            the fastest release of pressure to atmosphere */
            setVent();
        }

        else if((E_CONTROLLER_MODE_RATE == myMode) && (1u == pidParams.measure))
        {
            /* Mode is changed to rate control, set the vent valves in the control rate TDM mode. In this case, the
            vent valve will be pulsed according to the vent rate set by the user */
            setControlRate();
        }

        else if((E_CONTROLLER_MODE_RATE == myMode) && (1u == pidParams.venting))
        {
            /* Mode is changed to rate control, set the vent valves in the control rate TDM mode. In this case, the
            vent valve will be pulsed according to the vent rate set by the user */
            setControlRate();
        }

        else if((E_CONTROLLER_MODE_RATE == myMode) && (1u == pidParams.control))
        {
            /* Mode is changed to rate control, set the vent valves in the control rate TDM mode. In this case, the
            vent valve will be pulsed according to the vent rate set by the user */
            setControlRate();
        }

        else if(E_CONTROLLER_MODE_MEASURE == myMode)
        {
            // Mode is correct, so reset coarse control error
            pidParams.ccError = eCoarseControlErrorReset;
            // Set the pid parameters to those read from the PM 620 and the pressure and set point type received
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
            // Control action happens on gauge pressure, so calculate it using atmospheric pressure if mode is abs
            setPointG = pidParams.pressureSetPoint - (atmosphericPressure * setPointType);

            // Check if piston is within range
            checkPiston();
            pidParams.mode = myMode;

            /* Set point centering checks may not happen around 0 mbar gauge pressure if the sensor has an offset.
            Calculate the offsets aroud the sensor offset using set sensor gauge uncertainty value to validate if the
            set point value lies in between. In this case, pressure control could be done without the pump action if the
            volume is small
            Add example
            50 mbar offset - GU - 5 mbar, -5 mbar to 5 mbar automatically

            45 to 55 mbar - auto controlled  ERASE LATER
            */
            offsetCentrePos = sensorParams.offset + sensorParams.gaugeUncertainty;
            offsetCentreNeg = sensorParams.offset - sensorParams.gaugeUncertainty;
            setPointA = setPointG + atmosphericPressure;

            /* If already requires a pump action, only then calculate the value to overshoot the set point. In this case
            set point lies outside of the center offset values calculated above, which would require the user pumping
            action of either pump up or down */
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
                // In all other cases, pump action is not required, hence set the overshoot to 0
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
            /* Case 1 - check that the measured pressure is within pump tolerance limits and fine control can be
            entered. If this case passes, caseStatus variable is set to 1 and further cases are not executed */
            caseStatus = coarseControlCase1();

            if(0u == caseStatus)
            {
                /* Case 2 - This case checks whether the set point is greater than current measured pressure, but the
                piston is off center towards the retracted end and can increase pressure possibly acheiving the set
                point without user intervention */
                caseStatus = coarseControlCase2();

                if(0u == caseStatus)
                {
                    /* Case 3 - This case checks whether the set point is smaller than current measured pressure, but
                    piston is off center towards the extended end and can decrease pressure possibly acheiving the set
                    point without user intervention */
                    caseStatus = coarseControlCase3();

                    if(0u == caseStatus)
                    {
                        /* Case 4 - check if offset calculated as sensoroffset + gaugeUncertainty is in between
                        measured pressure and set point value. Pressure here can be vented to get close to the setpoint
                        or shift from automatic control to a pump up / down action requirement */
                        caseStatus = coarseControlCase4();

                        if(0u == caseStatus)
                        {
                            /* Case 5 - Check if totalOvershoot required is centered around gauge pressure measured
                            and offset calculated as sensor offset + gauge uncertainty. In this case set the vent
                            direction based on expected movement in pressure to be going high or low. Set vent direction
                            down if going lower in pressure and vent direction up if going up in pressure from vacuum */
                            caseStatus = coarseControlCase5();

                            if(0u == caseStatus)
                            {
                                /* Case 6 - motor is not centered in coarse control - and lies on the retracted end. For
                                acheiving the maximum range of screw controller for pressure control motor needs to be
                                centered before starting fine control of a set point / pump up / down. */
                                caseStatus = coarseControlCase6();

                                if(0u == caseStatus)
                                {
                                    /* Case 7 - motor is not centered in coarse control - and lies on the extended end.
                                    For acheiving the maximum range of screw controller for pressure control motor needs
                                    to be centered before starting fine control of a set point / pump up / down. */
                                    caseStatus = coarseControlCase7();

                                    if(0u == caseStatus)
                                    {
                                        /* Case 8 - Check if a pump up action is required to increase pressure. Pump
                                        action will be required when total overshoot is significantly greater than
                                        measured pressure and cannot be acheived by any of the previous cases */
                                        caseStatus = coarseControlCase8();

                                        if(0u == caseStatus)
                                        {
                                            /* Case 9 - Check if a pump down action is required to decrease pressure.
                                            Pump action will be required when total overshoot is significantly greater
                                            than measured pressure and cannot be acheived by any of previous cases */
                                            coarseControlCase9();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // move motor by the number of set steps in the earlier executed cases
            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);
            // Motor may have moved the last time, so use that step count to calculate the distance travelled
            calcDistanceTravelled(pidParams.stepCount);
            // Update the piston position based on the number of steps travelled by the motor last iteration
            pidParams.pistonPosition = pidParams.pistonPosition + pidParams.stepCount;
        }

        else if(E_CONTROLLER_MODE_VENT == myMode)
        {
            // Mode is correct, so reset coarse control error
            pidParams.ccError = 0u;
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.setPointType = setPointType;
            pidParams.mode = myMode;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            /* Run the coarse control vent state. This state vents pressure from the PV624 and also centers the motor
            if it is out of centre position. */
            coarseControlVent();
        }

        else if(E_CONTROLLER_MODE_RATE == myMode)
        {
            pidParams.ccError = 0u;
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.setPointType = setPointType;
            pidParams.mode = myMode;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            /* Run the coarse control rate state. Controlled venting is performed in switch testing mode where pressure
            reduction could be required at a certain set rate. The VR DUCI command is used to set the rate of pressure
            reduction from 1.0 mbar / Pm 620 iteration to 1000 mbar / iteration. Motor operation is disabled in rate
            control mode */
            coarseControlRate();
        }

        else
        {
            // Mode is not either of measure, vent, control pressure or control rate, set CC error
            pidParams.ccError = eCoarseControlErrorSet;
            calcStatus();
        }

        // Calculate PID status
        calcStatus();
        // Glow the coarse control LEDS as per mode
        coarseControlLed();
    }
}

#pragma diag_suppress=Pm031

/**
 * @brief   Coarse control rate state - used when a measured rate is required while going down in pressure or up
            in vacuum. The vent valve is operated in TDM mode and is pulsed from 500us to 6000 us in increments of
            10 us until the pressure rate is foudn to be correct
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlRate(void)
{
    uint32_t status = 0u;

    float32_t absDp = 0.0f;         // Absolute value of difference in pressure as it could be negative
    float32_t maxRate = 0.0f;       // Maximum rate of reduction of pressure
    float32_t calcVarDp = 0.0f;     // Calculated variance in pressure as 3 sigma of pressure uncertainty difference
    float32_t offsetPos = 0.0f;     // Offset positive calculated as sum of sensor offset and gauge uncertainty
    float32_t offsetNeg = 0.0f;     // Offset negative calculated as difference of sensor offset and gauge uncertainty

    // Read the vent rate set by the GENII.
    PV624->getVentRate(&pidParams.ventRate);
    // Set the step size to 0 as the motor motion is not allowed in rate mode.
    pidParams.stepSize = 0;

    // If controlled vent status is not, set, set it, first time in this case
    if(0u == pidParams.controlledVent)
    {
        // Set the vent iterations to 0 at first time in this case
        pidParams.controlledVent = 1u;
        bayesParams.ventIterations = 0u;
        /* Set the final pressure during first time in this case to gauge pressure as this will be used as previous
        pressure reading */
        // Add notes and calculations TODO MAK
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
        // Gauge pressure is centered around positive and negative offset values, set that the PV624 is vented
        // Since the PV624 is vented, configure the vent valve in PWM mode to save power
        PV624->valve3->reConfigValve(E_VALVE_MODE_PWMA);
        pidParams.vented = 1u;  // Set the vented status


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
        /* increase vent on-time to maxVentDutyCycle until previous vent effect is greater than the smaller of target
        ventRate or pressure uncertainty, the vent rate may not be exactly accurate, so a reduction in the vent
        pulse time may be required as the abs pressure difference could be higher than max rate set. */
        PV624->valve3->reConfigValve(E_VALVE_MODE_TDMA);
        bayesParams.ventDutyCycle = min((bayesParams.ventDutyCycle + screwParams.ventDutyCycleIncrement),
                                        screwParams.maxVentDutyCycle);
        pidParams.vented = 0u;
        pulseVent();
    }

    else
    {
        /* decrease vent on-time to minVentDutyCycle until previous vent effect is less than or equal to target ventRate
        the vent rate may not be exactly accurate, so a reduction in the vent pulse time may be required as the abs
        pressure difference could be higher than max rate set. */
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
    uint32_t status = 0u;

    float32_t absDp = 0.0f;         // Absolute value of difference in pressure as it could be negative
    float32_t offsetPos = 0.0f;     // Offset positive calculated as sum of sensor offset and gauge uncertainty
    float32_t offsetNeg = 0.0f;     // Offset negative calculated as difference of sensor offset and gauge uncertainty
    float32_t tempPresUncertainty = 0.0f;   // temporary varialbe to hold 2 sigma of uncertainty pressure difference

    checkPiston();

    // First time through this case, check if fast vent is set
    if(0u == pidParams.fastVent)
    {
        // Set the vent valve to vent the PV624 at the fastest rate
        setFastVent();
        bayesParams.ventIterations = 0u;
        /* Set the final pressure during first time in this case to gauge pressure as this will be used as previous
        pressure reading */
        bayesParams.ventFinalPressure = gaugePressure;
    }

    bayesParams.ventInitialPressure = bayesParams.ventFinalPressure;
    bayesParams.ventFinalPressure = gaugePressure;
    bayesParams.changeInPressure = bayesParams.ventFinalPressure - bayesParams.ventInitialPressure;

    absDp = fabs(bayesParams.changeInPressure); // Calculate abs of change in pressure
    tempPresUncertainty = 2.0f * sqrt(bayesParams.uncertaintyPressureDiff);

    offsetPos = sensorParams.offset + sensorParams.gaugeUncertainty;
    offsetNeg = sensorParams.offset - sensorParams.gaugeUncertainty;

    /* If the gauge pressure is between the offsets calculated, and the change in pressure from the previous iteration
    is less than the uncertainty in the pressure difference (2 sigma), then the system is vented */
    if((offsetNeg < gaugePressure) && (gaugePressure < offsetPos) && (absDp < tempPresUncertainty))
    {
        /* Cannot hold the vent valve open continuously as it draws a large amount of current. The logic here pulses the
        vent valve if:
            1. The pressure is out of offset band OR the change in pressure is large - may happen during an adiabatic
            recovery
            2. A certain amount of time has passed, so open the valve and vent irrespective whether the PV624 is already
            vented
        The hold vent iterations provide a time = iterations * read rate of PM e.g. for piezo it is 10 * 70 = 700ms.
        The hold vent interval provides a time to wait until the next trigger
        If the vent count is in between the iterations and the interval, close the valve and save power */

        if(pidParams.holdVentCount < screwParams.holdVentIterations)
        {
            pidParams.holdVentCount = pidParams.holdVentCount + 1u;
            bayesParams.ventDutyCycle = screwParams.holdVentDutyCycle;
            pulseVent();

            // First time through this case
            if(0u == pidParams.vented)
            {
                // Only vented when the valve is opened
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
            // Turn off the valve to save power
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

    /* Piston may not be centered while venting, start centering the piston. This has no effect on the pressure as the
    vent valve is open */
    if(pidParams.pistonPosition > (screwParams.centerPositionCount + screwParams.centerTolerance))
    {
        // If the motor is off center towards the extended end, drive the motor towards retracted end with neg steps
        pidParams.stepSize = -1 * screwParams.maxStepSize;

        // If centering while venting, set the centering vent flag
        if(0u == pidParams.centeringVent)
        {
            pidParams.centeringVent = 1u;
        }
    }

    else if(pidParams.pistonPosition < (screwParams.centerPositionCount - screwParams.centerTolerance))
    {
        // If the motor is off center towards the retracted end, drive the motor towards extended end with pos steps
        pidParams.stepSize = screwParams.maxStepSize;

        // If centering while venting, set the centering vent flag
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

    PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);
    pidParams.pistonPosition = pidParams.pistonPosition + pidParams.stepCount;
    calcDistanceTravelled(pidParams.stepCount);

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
    pump up, in small volumes a previous controlled vent may have overshot setpoint which could trigger an endless loop
    of vent and pump.  Break this loop by detecting if the the pump condition was preceded by a vent in the opposite
    direction */

    totalOvershoot = setPointG + pidParams.overshoot;

    if(totalOvershoot > gaugePressure)
    {
        status = 1u;
        pidParams.stepSize = 0;

        if(0u == pidParams.pumpUp)
        {
            if((1u != pidParams.ventDirDown) && (0u == pidParams.pumpAttempts))
            {
                // previous controlled vent was not in the opposite direction to pump action
                setControlUp();
            }

            else
            {
                /* previous control vent overshot setpoint by a more than pumpTolerance This can happen if the volume is
                too small to catch the setpoint during one fast control iteration, in which case the control range is
                considerably larger than pumpTolerance anyway.  Assume that is the case and attempt to move to fine
                control even though pressure is not within pumpTolerance range. coarse adjustment complete, last
                iteration of coarse control loop */

                setControlIsolate();
                pidParams.fineControl = 1u;
                pidParams.stepSize = 0;
                // Go to fine control
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
            if((1u != pidParams.ventDirUp) && (0u == pidParams.pumpAttempts))
            {
                /* previous controlled vent was not in the opposite direction to pump action */
                setControlDown();
            }

            else
            {
                /* previous control vent overshot setpoint by a more than pumpTolerance This can happen if the volume is
                too small to catch the setpoint during one fast control iteration, in which case the control range is
                considerably larger than pumpTolerance anyway.  Assume that is the case and attempt to move to fine
                control even though pressure is not within pumpTolerance range. coarse adjustment complete, last
                iteration of coarse control loop */
                setControlIsolate();
                pidParams.fineControl = 1u;
                pidParams.stepSize = 0;
                // GO to fine control
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
* @brief    Pressure control loop - This is the main pressure control state machine
            It operates in 3 states
            1. Entry State - startup of controller, or change in PM620 sensor and initializes all variables
            2. Coarse control - Runs to check if pressure value is very close to set point, controlled venting
            PM620 is polled at a faster rate here as accuracy is not a requirement here
            3. Fine control - Runs to control the pressure using the stepper motor driving the piston using a
            proportional control algo. PM620 is polled at slow rate to ensure better accuracy
* @param    pressureInfo_t *ptrPressureInfo - Structure containing pressure data from Measure and Control task
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
            /* Handle the entry to the coarse control loop - This is called the first time the controller is started
            up, or if there is a PM620 sensor change. */
            coarseControlSmEntry();
            break;

        case eCoarseControlLoop:
            // run coarse control loop until close to setpoint
            coarseControlLoop();
            break;

        case eFineControlLoop:
            // run fine control loop until Genii mode changes from control, or range exceeds or excessive volume or leak
            fineControlLoop();
            break;

        default:
            break;
        }

        dumpData();
    }
}


