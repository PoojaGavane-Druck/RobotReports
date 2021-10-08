/**
* Baker Hughes Confidential\n* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file		DController.c
* @version	1.00.00
* @author	Makarand Deshmukh / Nageswara Pydisetty
* @date		31-08-2021
*
* @brief	Source file for pressure control algorithm
*/

/* Includes -----------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include "math.h"
#include <os.h>
MISRAC_ENABLE
#include "memory.h"
#include "smbus.h"
#include "DLock.h"
#include "DPV624.h"
#include "main.h"
#include "Controller.h"
#include "utilities.h"
/* Defines and constants ----------------------------------------------------*/

#define ENABLE_VALVES
#define ENABLE_MOTOR

#define DUMP_PID_DATA
#define DUMP_BAYES_DATA

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

/* Types --------------------------------------------------------------------*/

/* Global Variables ---------------------------------------------------------*/

/* File Statics -------------------------------------------------------------*/
static const float EPSILON = (float)1E-10;  //arbitrary 'epsilon' value
static const float gaugeSensorUncertainty = (float)20.0; //uncertainty gage sensor pressure
static const float piValue = (float)3.14159;

/* User Code ----------------------------------------------------------------*/

/**
* @brief	Controller class constructor
* @param	void
* @retval	void
*/
DController::DController()
{
    /* Init controller objects */
    //ventValve = new DValve();
    //valve2 = new DValve();
    //valve3 = new DValve();
    //motor = new DStepperMotor(); /* replace by motor manager */

    controllerState = eCoarseControlLoopEntry;
    pressureSetPoint = 0.0f;
    setPointG = 0.0f;
    absolutePressure = 0.0f;
    gaugePressure = 0.0f;
    atmosphericPressure = 0.0f;
    controllerStatus.bytes = (uint32_t)0;
    ctrlStatusDpi.bytes = 0u;
    controllerStatus.bit.ventDirUp = 0u;
    controllerStatus.bit.ventDirDown = 0u;    
    sensorFsValue = (float)20000;
    fsValue = (float)7000;
    ventReadingNum = (eControlVentReading_t)eControlVentGetFirstReading;
    myMode = E_CONTROLLER_MODE_VENT;

    initialize();
}

/**
* @brief	Controller class destructor
* @param	void
* @retval	void
*/
DController::~DController()
{

}

/**
* @brief	Initialize all the parameters and variable values
* @param	void
* @retval	void
*/
void DController::initialize(void)
{
    initPidParams();
    initMotorParams();
    initScrewParams();
    initBayesParams();
    initTestParams();
    generateSensorCalTable();
}

/**
* @brief	Init PID controller parameters
* @param	void
* @retval	void
*/
void DController::initPidParams(void)
{
    pidParams.elapsedTime = 0u;                     //# elapsed time for log file(s)
    pidParams.pressureSetPoint = (float)0.0;                 //# pressure setpoint(mbar)
    pidParams.setPointType = (eSetPointType_t)0;              //# setpoint type from GENII(0 = gauge, 1 = abs, 2 = baro)
    pidParams.stepCount = (int32_t)0;                        //# number of motor pulses delivered since last stepSize request
    pidParams.pressureError = (float)0.0;                    //# pressure error for PID(mbar), +ve == below pressure setpoint
    pidParams.totalStepCount = (int32_t)0;                   //# total step count since start
    pidParams.controlledPressure = (float)0.0;               //# controlled pressure(mbar gage or mbar abs)
    pidParams.stepSize = (int32_t)0;                         //# requested number of steps to turn motor
    pidParams.pressureCorrectionTarget = (float)0.0;         //# leak - adjusted pressure correction target(mbar)
    pidParams.requestedMeasuredMotorCurrent = (float)0.0;    //# requested measured motor current(mA)
    pidParams.measuredMotorCurrent = (float)0.0;             //# measured motor current(mA)
    pidParams.opticalSensorAdcReading = (uint32_t)0;         //# optical sensor ADC reading(0 to 4096)
    pidParams.pistonPosition = (int32_t)0;                   //# optical piston position(steps), 0 == fully retracted / max volume
    pidParams.motorSpeed = (float)0;                         //# motor speed from motor controller(pps)
    pidParams.isSetpointInControllerRange = (int32_t)(1);    //# setpoint target in controller range, based on bayes range estimate
    pidParams.pumpTolerance = (float)0.005;                  //# max relative distance from setpoint before pumping is required, e.g. 0.1 == 10 % of setpoint
}

/**
* @brief	Init motor controller parameters
* @param	void
* @retval	void
*/
void DController::initMotorParams(void)
{
    motorParams.motorStepSize = (float)1.8;         //#full step angle of motor(deg)
    motorParams.microStepSize = (float)4.0;        //#number of microsteps(2 == halfstep)
    motorParams.accelerationAlpha = (float)0.98;    //#motor controller s - curve alpha acceleration value, from excel calculator, fast to full speed
    motorParams.accelerationBeta = (float)2.86;     //#motor controller s - curve beta acceleration value, from excel calculator, fast to full speed
    motorParams.decellerationAlpha = (float)1.02;   //#motor controller s - curve alpha decelleration value, from excel calculator
    motorParams.decellerationBeta = (float)0.0;     //#motor controller s - curve beta decelleration value, from excel calculator
    motorParams.maxMotorCurrent = (float)2.0;       //#maximum motor current(A)
    motorParams.minMotorCurrrent= (float)0.7;       //#minimum motor current(A), amount required to overcome friction around 0 bar g
    motorParams.holdCurrent = (float)0.2;           //#hold current(A), used when when not moving, minimum = 0.2 A to avoid motor oscillations when controlling
    motorParams.maxStepSize = (int32_t)3000; //# maximum number of steps that can be taken in one control iteration, used in coarse control loop
}

#pragma diag_suppress=Pm137 /* Disable MISRA C 2004 rule 18.4 */
/**
* @brief	Init screw parameters
* @param	void
* @retval	void
*/
void DController::initScrewParams(void)
{
    screwParams.gearRatio = (float)1.0; //#gear ratio of motor
    screwParams.pistonDiameter = (float)12.2; //#piston diameter(mm), Helix0.6 press
    screwParams.leadScrewPitch = (float)0.6; //#lead screw pitch(mm / rotation)
    screwParams.leadScrewLength = (float)38.0; //# travel piston(mm)
    screwParams.pistonArea = piValue * (screwParams.pistonDiameter / 2.0f) * (screwParams.pistonDiameter / 2.0f);  //#piston area(mm ^ 2)
    screwParams.changeInVolumePerPulse = (motorParams.motorStepSize * screwParams.leadScrewPitch * screwParams.pistonArea * (float)1e-3) / (motorParams.microStepSize * screwParams.gearRatio * (float)360.0); //#volume change per control pulse(mL / pulse)
    screwParams.maxPressure = (float)21000.0; //#maximum system pressure(mbar) for accel / decel current scaling
    //# pulse count for full compression of piston, with 2 % safety factor to avoid collisions
    screwParams.maxPosition = int(0.98f * screwParams.leadScrewLength / screwParams.leadScrewPitch * (float)360.0 / motorParams.motorStepSize * motorParams.microStepSize);
    //# pulse count for full retraction of piston, with 2 % safety factor to avoid collisions
    screwParams.minPosition = int((float)(screwParams.maxPosition) / (float)0.98f * (float)0.02);
    //# use measured values instead of calculated
    screwParams.maxPosition = (int32_t)48800;
    screwParams.minPosition = (int32_t)1600;
    screwParams.centerPositionCount = (int32_t)25000; //# step count at nominal center position(home)
    screwParams.centerTolerance = (int32_t)2000; //# tolerance for finding center position(counts)
    screwParams.shuntResistance = (float)0.05;  //# shunt resistance for current sensor, motor testing(ohm)
    screwParams.shuntGain = 0.2f * (20.0f * 50.0f / (20.0f + 50.0f));  //# V / V gain of current sensor, with 20k Rload sensor and 50k ADC input impedance
    screwParams.readingToCurrent = 3.3f / (4096.0f * screwParams.shuntGain * screwParams.shuntResistance) * 1000.0f;  //# conversion factor from shunt ADC counts to current(mA)
    screwParams.maxLeakRate = (float)0.3f;  //# maximum leak rate bound(mbar / interation) at 20 bar and 10 mA, PRD spec is < 0.2
    screwParams.maxAllowedPressure = (float)21000.0;  //# maximum allowed pressure in screw press(PM independent) (mbar), for max leak rate adjustments
    screwParams.nominalTotalVolume = (float)10.0;  //# nominal total volume(mL), for max leak rate adjustments
}
#pragma diag_default=Pm137 /* Disable MISRA C 2004 rule 18.4 */

/**
* @brief	Init bayes parameters
* @param	void
* @retval	void
*/
void DController::initBayesParams(void)
{
    bayesParams.minSysVolumeEstimateValue = (float)5.0; //#minimum system volume estimate value(mL)
    bayesParams.maxSysVolumeEstimateValue = (float)100.0; //#maximum system volume estimate value(mL)
    bayesParams.minEstimatedLeakRate = (float)0.0; //#minimum estimated leak rate(mbar)
    bayesParams.maxEstimatedLeakRate  = (float)0.2; //#maximum absolute value of estimated leak rate(+/ -mbar / iteration)
    bayesParams.measuredPressure = (float)1000.0; //#measured pressure(mbar)
    bayesParams.smoothedPresure = (float)1000.0; //#smoothed pressure, depends on controlled pressure stability not sensor uncertainty, (mbar)
    bayesParams.changeInPressure = (float)0.0; //#measured change in pressure from previous iteration(mbar)
    bayesParams.prevChangeInPressure = (float)0.0; //#previous dP value(mbar)
    bayesParams.estimatedVolume = bayesParams.maxSysVolumeEstimateValue; //#estimate of volume(mL), set to minV to give largest range estimate on startup
    bayesParams.algorithmType = eMethodNone; //#algorithm used to calculate V
    bayesParams.changeInVolume = (float)0.0; //#volume change from previous stepSize command(mL)
    bayesParams.prevChangeInVolume = (float)0.0; //#previous dV value(mL), used in regression method
    bayesParams.measuredVolume = bayesParams.maxSysVolumeEstimateValue; //#volume estimate using Bayes regression(mL)
    bayesParams.estimatedLeakRate = bayesParams.minEstimatedLeakRate; //#estimate in leak rate(mbar / iteration), from regression method
    bayesParams.measuredLeakRate = bayesParams.minEstimatedLeakRate; //#measured leak rate using Bayes regression(mbar / iteration)
    bayesParams.estimatedKp = (float)500.0; //#estimated kP(steps / mbar) that will reduce pressure error to zero in one iteration, large for fast initial response
    //#bayes['measkP'] = bayes['kP'] #measured optimal kP(steps / mbar) that will reduce pressure error to zero in one iteration
    //#state value variances
    //bayesParams.sensorUncertainity = (10e-6 * sensorFsValue) * (10e-6 * sensorFsValue); //#uncertainty in pressure measurement(mbar), sigma ~= 10 PPM of FS pressure @ 13 Hz read rate
    bayesParams.sensorUncertainity = (10e-6f * fsValue) * (10e-6f * fsValue);
    bayesParams.uncertaintyPressureDiff = 2.0f * bayesParams.sensorUncertainity; //#uncertainty in measured pressure differences(mbar)
    bayesParams.uncertaintyVolumeEstimate = bayesParams.maxSysVolumeEstimateValue * 1e6f; //#uncertainty in volume estimate(mL), large because initial volume is unknown, from regression method
    bayesParams.uncertaintyMeasuredVolume = (screwParams.changeInVolumePerPulse * 10.0f) * (screwParams.changeInVolumePerPulse * 10.0f); //#uncertainty in volume estimate from latest measurement using bayes regression(mL)
    bayesParams.uncertaintyVolumeChange = (screwParams.changeInVolumePerPulse * 10.0f) * (screwParams.changeInVolumePerPulse * 10.0f); //# uncertainty in volume change, depends mostly on backlash ~= +/ -10 half - steps, constant, mL
    bayesParams.uncertaintyEstimatedLeakRate = bayesParams.uncertaintyPressureDiff; //#uncertainty in leak rate from bayes estimateand gas law(mbar / iteration), from regression method
    bayesParams.uncertaintyMeasuredLeakRate  = bayesParams.uncertaintyPressureDiff; //#measured leak rate uncertainty from bayes regresssion estimate(mbar / iteration)
    bayesParams.maxZScore = (float)2.0; // #maximum variance spread between measuredand estimated values before estimated variance is increased
    bayesParams.lambda = (float)0.1; //#forgetting factor for smoothE
    bayesParams.uncerInSmoothedMeasPresErr  = (float)0.0; //#smoothed measured pressure error variance(mbar * *2)
    bayesParams.targetdP = (float)0.0; //#target correction from previous control iteration(mbar)
    bayesParams.smoothedPressureErr  = (float)0.0; //#smoothed pressure error(mbar)
    bayesParams.smoothedSqaredPressureErr = (float)0.0; //#smoothed squared pressure error(mbar * *2)
    bayesParams.uncerInSmoothedMeasPresErr  = (float)0.0; //#smoothed measured pressure error variance(mbar * *2)
    bayesParams.gamma = (float)0.98; //#volume scaling factor for nudging estimated volume with predictionError, (0.90, 0.98), larger = faster response but noisier estimate
    bayesParams.predictionError  = (float)0.0; //#prediction error from previous control iteration(mbar)
    bayesParams.predictionErrType = (int32_t)0; //#prediction error type(+/ -1), for volume estimate adjustment near setpoint
    bayesParams.maxAchievablePressure  = (float)0.0; //#maximum achievable pressure, from bayes estimates(mbar)
    bayesParams.minAchievablePressure  = (float)0.0; //#minimum achievable pressure, from bayes estimates(mbar)
    bayesParams.maxPositivePressureChangeAchievable  = (float)1e6; //#maximum positive pressure change achievable, from bayes estimates(mbar)
    bayesParams.maxNegativePressureChangeAchievable = (float)-1e6; //#maximum negative pressure change achievable, from bayes estimates(mbar)
    bayesParams.minPressureAdjustmentRangeFactor = pidParams.pumpTolerance; //#minimum pressure adjustment range factor when at nominalHome, e.g. 0.1 = minimum + / -10 % adjustment range of P at nominalHome piston location
    bayesParams.nominalHomePosition = screwParams.centerPositionCount; //#nomimal "home" position to achieve + / -10 % adjustability
    bayesParams.expectedPressureAtCenterPosition = (float)0.0; //#expected pressure at center piston position(mbar)
    bayesParams.maxIterationsForIIRfilter  = (uint32_t)100; //#maximum iterations for leak rate integration filter in PE correction method
    bayesParams.minIterationsForIIRfilter  = (uint32_t)10; //#minimum iterations for leak rate integration filter in PE correction method
    bayesParams.changeToEstimatedLeakRate = (float)0.0; //#change to estimated leak rate for PE correction method(mbar / iteration)
    bayesParams.alpha = (float)0.1; //#low - pass IIR filter memory factor for PE correction method(0.1 to 0.98)
    bayesParams.smoothedPressureErrForPECorrection = (float)0; //#smoothed pressure error for PE correction method(mbar)
    bayesParams.log10epsilon = (float)-0.7; //#acceptable residual fractional error in PE method leak rate estimate(-2 = +/ -1 %, -1 = 10 %, -0.7 = 20 %)

}

/**
* @brief	Init Test parameters
* @param	void
* @retval	void
*/
void DController::initTestParams(void)
{
    //# simulated leak rate(mbar / iteration) at 10mL and 20 bar
    testParams.maxFakeLeakRate = screwParams.maxLeakRate * 0.5f;
    testParams.maxFakeLeakRate = (float)0.0f;
    //# simulated leak rate, adjusted by current pressure and volume(mbar / iteration)
    testParams.fakeLeakRate = testParams.maxFakeLeakRate;
    //# calibrate optical sensor if True
    testParams.isOpticalSensorCalibrationRequired = true;
    testParams.fakeLeak = (float)0.0f;  //# simulated cumulative leak effect(mbar)
    testParams.volumeForLeakRateAdjustment = (float)10.0f;  //# fixed volume value used for leak rate adjustment(mL)

}
/**
* @brief	Reset the bayes estimation parameters
* @param	void
* @retval	void
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
    bayesParams.smoothedSqaredPressureErr = 0.0f;
    bayesParams.uncerInSmoothedMeasPresErr = 0.0f;
    bayesParams.smoothedPressureErrForPECorrection = 0.0f;
}

/**
* @brief	Returns the sign of a float
* @param	void
* @retval	void
*/
float DController::getSign(float value)
{
    float sign = 0.0f;
    
    if (value > 0.0f)
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
* @brief	Run the controller in measure mode
* @param	void
* @retval	void
*/
void DController::setMeasure(void)
{
    //# isolate pump for measure
#ifdef ENABLE_VALVES
    PV624->ventValve->valveTest(E_VALVE_FUNCTION_FORWARD); // Isolate vent
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump outlet
    /*
    PV624->ventValve->triggerValve(VALVE_STATE_OFF);   //# isolate vent port
    PV624->valve1->triggerValve(VALVE_STATE_OFF);    //# isolate pump inlet, should isolate external source too if vacuum
    PV624->valve2->triggerValve(VALVE_STATE_OFF);     //# isolate pump outlet, should isolate external source too if pressure
    */
#endif
    controllerStatus.bit.control = 0u;
    controllerStatus.bit.measure = 1u;
    controllerStatus.bit.venting = 0u;
    controllerStatus.bit.vented = 0u;
    controllerStatus.bit.pumpUp = 0u;
    controllerStatus.bit.pumpDown = 0u;
    controllerStatus.bit.centering = 0u;
    controllerStatus.bit.controlledVent = 0u;
    controllerStatus.bit.centeringVent = 0u;
    controllerStatus.bit.ventDirUp = 0u;
    controllerStatus.bit.ventDirDown = 0u;
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
}

/**
* @brief	Set controller state and valves when pump up is required
* @param	void
* @retval	void
*/
void DController::setControlUp(void)
{
    // set for pump up
#ifdef ENABLE_VALVES
    PV624->ventValve->valveTest(E_VALVE_FUNCTION_FORWARD); // Isolate vent
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_REVERSE); //connect pump outlet
    /*
    PV624->ventValve->triggerValve(VALVE_STATE_OFF);   //# isolate vent port
    PV624->valve1->triggerValve(VALVE_STATE_OFF);  //# isolate pump inlet
    PV624->valve2->triggerValve(VALVE_STATE_ON);//# connect pump outlet
    */
#endif
    controllerStatus.bit.control = 1u;
    controllerStatus.bit.measure = 0u;
    controllerStatus.bit.venting = 0u;
    controllerStatus.bit.vented = 0u;
    controllerStatus.bit.pumpUp = 1u;
    controllerStatus.bit.pumpDown = 0u;
    controllerStatus.bit.centering = 0u;
    controllerStatus.bit.controlledVent = 0u;
    controllerStatus.bit.centeringVent = 0u;
    controllerStatus.bit.ventDirUp = 0u;
    controllerStatus.bit.ventDirDown = 0u;
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
}

/**
* @brief	Set controller state and valves when pump down is required
* @param	void
* @retval	void
*/
void DController::setControlDown(void)
{
    // set for pump down, TBD safety check initial pressure vs atmosphere
#ifdef ENABLE_VALVES
    PV624->ventValve->valveTest(E_VALVE_FUNCTION_FORWARD); // Isolate vent
    PV624->valve1->valveTest(E_VALVE_FUNCTION_REVERSE); //isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump outlet
    /*
    PV624->ventValve->triggerValve(VALVE_STATE_OFF);     //# isolate vent port
    PV624->valve1->triggerValve(VALVE_STATE_OFF);   //# connect pump inlet
    PV624->valve2->triggerValve(VALVE_STATE_ON);    //# isolate pump outlet
    */
#endif        
    controllerStatus.bit.control = 1u;
    controllerStatus.bit.measure = 0u;
    controllerStatus.bit.venting = 0u;
    controllerStatus.bit.vented = 0u;
    controllerStatus.bit.pumpUp = 0u;
    controllerStatus.bit.pumpDown = 1u;
    controllerStatus.bit.centering = 0u;
    controllerStatus.bit.controlledVent = 0u;
    controllerStatus.bit.centeringVent = 0u;
    controllerStatus.bit.ventDirUp = 0u;
    controllerStatus.bit.ventDirDown = 0u;
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
}

/**
* @brief	Set controller state and valves when pump isolation is required
* @param	void
* @retval	void
*/
void DController::setControlIsolate(void)
{
    //# isolate pump for screw control
#ifdef ENABLE_VALVES
    PV624->ventValve->valveTest(E_VALVE_FUNCTION_FORWARD); // Isolate vent
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump outlet
    /*
    PV624->ventValve->triggerValve(VALVE_STATE_OFF);    //# isolate vent port
    PV624->valve1->triggerValve(VALVE_STATE_OFF);   //# isolate pump inlet
    PV624->valve2->triggerValve(VALVE_STATE_OFF);   //# isolate pump outlet     
    */
    
#endif
    controllerStatus.bit.control = 1u;
    controllerStatus.bit.measure = 0u;
    controllerStatus.bit.venting = 0u;
    controllerStatus.bit.vented = 0u;
    controllerStatus.bit.pumpUp = 0u;
    controllerStatus.bit.pumpDown = 0u;
    controllerStatus.bit.centering = 0u;
    controllerStatus.bit.controlledVent = 0u;
    controllerStatus.bit.centeringVent = 0u;
    controllerStatus.bit.ventDirUp = 0u;
    controllerStatus.bit.ventDirDown = 0u;
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
}       

/**
* @brief	Set controller state and valves when piston centering is required
* @param	void
* @retval	void
*/
void DController::setControlCentering(void)
{
    //# isolate pump for centering piston
#ifdef ENABLE_VALVES    
    PV624->ventValve->valveTest(E_VALVE_FUNCTION_FORWARD); // Isolate vent
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump outlet
    /*
    PV624->ventValve->triggerValve(VALVE_STATE_OFF);   //# isolate vent port
    PV624->valve1->triggerValve(VALVE_STATE_OFF);    //# isolate pump inlet
    PV624->valve2->triggerValve(VALVE_STATE_OFF);   //# isolate pump outlet
    */
#endif
    controllerStatus.bit.control = 1u;
    controllerStatus.bit.measure = 0u;
    controllerStatus.bit.venting = 0u;
    controllerStatus.bit.vented = 0u;
    controllerStatus.bit.pumpUp = 0u;
    controllerStatus.bit.pumpDown = 0u;
    controllerStatus.bit.centering = 1u;
    controllerStatus.bit.controlledVent = 0u;
    controllerStatus.bit.centeringVent = 0u;
    controllerStatus.bit.ventDirUp = 0u;
    controllerStatus.bit.ventDirDown = 0u;
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
}

/**
* @brief	Run controller in when controlled vent mode
* @param	void
* @retval	void
*/
void DController::setControlVent(void)
{
#ifdef ENABLE_VALVES
    //# isolate pump for controlled vent to setpoint    
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump outlet
    PV624->ventValve->valveTest(E_VALVE_FUNCTION_REVERSE); // Isolate vent
    /*
    PV624->valve1->triggerValve(VALVE_STATE_OFF);   //# isolate pump inlet
    PV624->valve2->triggerValve(VALVE_STATE_OFF);   //# isolate pump outlet
    PV624->ventValve->triggerValve(VALVE_STATE_ON);  //# connect vent port
    */
#endif
    controllerStatus.bit.control = 1u;
    controllerStatus.bit.measure = 0u;
    controllerStatus.bit.venting = 0u;  //# set to 0 because vent was not requested by GENII
    controllerStatus.bit.pumpUp = 0u;
    controllerStatus.bit.pumpDown = 0u;
    controllerStatus.bit.centering = 0u;
    controllerStatus.bit.controlledVent = 1u;
    controllerStatus.bit.centeringVent = 0u;  
    controllerStatus.bit.ventDirUp = 0u;
    controllerStatus.bit.ventDirDown = 0u;    
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
}

/**
* @brief	Run controller in vent mode
* @param	void
* @retval	void
*/
void DController::setVent(void)
{
    //# isolate pumpand vent
#ifdef ENABLE_VALVES    
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump outlet
    PV624->ventValve->valveTest(E_VALVE_FUNCTION_REVERSE); // Isolate vent
    /*
    PV624->valve1->triggerValve(VALVE_STATE_OFF);  //# isolate pump inlet
    PV624->valve2->triggerValve(VALVE_STATE_OFF);   //# isolate pump outlet
    PV624->ventValve->triggerValve(VALVE_STATE_ON);  //# connect vent port
    */
#endif
    controllerStatus.bit.control = 0u;
    controllerStatus.bit.measure = 0u;
    controllerStatus.bit.venting = 1u;
    controllerStatus.bit.vented = 0u;
    controllerStatus.bit.pumpUp = 0u;
    controllerStatus.bit.pumpDown = 0u;
    controllerStatus.bit.centering = 0u;
    controllerStatus.bit.controlledVent = 0u;
    controllerStatus.bit.centeringVent = 0u;
    controllerStatus.bit.ventDirUp = 0u;
    controllerStatus.bit.ventDirDown = 0u;
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
}

#pragma diag_suppress=Pm128 /* Disable MISRA C 2004 rule 10.1 */
#pragma diag_suppress=Pm137 /* Disable MISRA C 2004 rule 10.4 */
#pragma diag_suppress=Pm046 /* Disable MISRA C 2004 rule 13.3*/
/**
* @brief	Bayes estimation for different values
* @param	void
* @retval	void
*/
void DController::estimate(void)
{
    /*
    # Estimate volumeand leak rate using Bayesian conditional probability to merge new measurement
    # data with previous parameter estimates(aka extended Kalman filter).
    # Added empirical predictionError adjustment to volumeand leak estimates for smaller / slower corrections where gas
    # law estimation methods are noisy.
    # Jun 10 2021
    # updated to use with fineControl.py and coarseControl.py modules
    # improved comment clarity and code flow

    # defaults when measV cannot be calculated
    */
    float residualL = 0.0f;

    bayesParams.measuredVolume = 0.0f;
    bayesParams.uncertaintyMeasuredVolume = bayesParams.maxSysVolumeEstimateValue;
    bayesParams.algorithmType = eRegressionMethod;

    if (1u == controllerStatus.bit.fineControl)
    {
        //# do this in fine control mode only
        //# calculate prediction errorand error type from last control iteration
        //# prediction error(mbar)
        bayesParams.predictionError = bayesParams.changeInPressure - bayesParams.targetdP;

        //# predictionErrorType > 0 -- > excessive correction, reduce system gain to fix(decrease volume estimate)
        //# predictionErrorType < 0 --> insufficient correction, increase system gain to fix(increase volume estimate)
        //bayesParams.predictionErrType = np.sign(bayes['targetdP']) * np.sign(bayes['predictionError'])
        float signTargetDp = 0.0f;
        float signPredictionError = 0.0f;

        signTargetDp = getSign(bayesParams.targetdP);
        signPredictionError = getSign(bayesParams.predictionError);
        bayesParams.predictionErrType = (int32_t)(signTargetDp * signPredictionError);
        //# low pass filtered pressure error(mbar)
        bayesParams.smoothedPressureErr = pidParams.pressureError * bayesParams.lambda + bayesParams.smoothedPressureErr * (1.0f - bayesParams.lambda);
        //# low pass filtered squared error(mbar * *2)
        bayesParams.smoothedSqaredPressureErr = (pidParams.pressureError * pidParams.pressureError) * bayesParams.lambda + bayesParams.smoothedSqaredPressureErr * (1.0f - bayesParams.lambda);
        //# dynamic estimate of pressure error variance
        //# see https ://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        //# Note : varE estimate not valid when smoothE2 >> varE
        //bayes['varE'] = bayes['smoothE2'] - bayes['smoothE'] * *2
        bayesParams.uncerInSmoothedMeasPresErr = bayesParams.smoothedSqaredPressureErr - (bayesParams.smoothedPressureErr * bayesParams.smoothedPressureErr);

        //# decide if pressure is stable, based on smoothed variance of pressure readings
        //if bayes['varE'] * *0.5 < 5 * bayes['varP'] * *0.5 and bayes['smoothE'] < 5 * bayes['varP'] * *0.5:
        
        float sqrtUncertaintySmoothedPresErr = 0.0f;
        float sqrtSensorUncertainty = 0.0f;
        float tempSensorUncertainty = 0.0f;
        
        sqrtUncertaintySmoothedPresErr = sqrt(bayesParams.uncerInSmoothedMeasPresErr);
        sqrtSensorUncertainty = sqrt(bayesParams.sensorUncertainity);
        tempSensorUncertainty = 5.0f * sqrtSensorUncertainty;
        
        if((sqrtUncertaintySmoothedPresErr < tempSensorUncertainty) && 
            (bayesParams.smoothedPressureErr < tempSensorUncertainty))
        {
            controllerStatus.bit.stable = 1u;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
        }
        else
        {
            controllerStatus.bit.stable = 0u;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
        }
        /* SECTION COMMENTED OUT FOR MISRA REPLACED BY SECTION ABOVE 
        if (((sqrt(bayesParams.uncerInSmoothedMeasPresErr)) < (5.0f * sqrt(bayesParams.sensorUncertainity))) &&
                (bayesParams.smoothedPressureErr < (5.0f * sqrt(bayesParams.sensorUncertainity))))
        {
            //# pressure is stable, within 5 sigma of sensor measurement uncertainty
            controllerStatus.bit.stable = 1u;
        }
        else
        {
            controllerStatus.bit.stable = 0u;
        }
        */
        /*
        # Step(1)
        # Estimate volume and leak rate from isothermal gas law
        # Vmeas = -P * dV / (dP - Lmeas)
        # Three methods : regression, single point, or prediction error nudge
        */

        /*
        # Regression calc :
        # Observe that Vmeas is the equation of a line : y = m * x + b
        # with y == dP, x == dV, slope m == -P / Vmeas and intercept b == Lmeas
        # A unique solution for both(Vmeas, Lmeas) from gas law regression
        # requires a minimum of two measurements(dP, dV) at constant(P, Lmeas, Vmeas)

        # Single Point calc :
        # For single - point method assume Lmeas = 0 and calculate Vmeas from a single control iteration.

        # Prediction Error calc :
        # Use prediction error, smoothed prediction error, and prediction error type
        # to nudge volumeand leak estimates instead of gas law calculation.
        # This is a recursive correction method than can take many control iterations to converge to correct
        # parameter estimates if the initial estimation error is large,
        # hence use of the other two methods for initial estimates
        # when relatively far from setpoint.

        # difference in pressure change from previous pressure change(mbar)
        */
        float dP2, dV2;
        dP2 = bayesParams.changeInPressure - bayesParams.prevChangeInPressure;
        bayesParams.dP2 = dP2;

        //# difference in volume change from previous volume change(mL)
        dV2 = bayesParams.changeInVolume - bayesParams.prevChangeInVolume; // bayes['dV'] - bayes['dV_']
        bayesParams.dV2 = dV2;

        //# adjust previous volume estimate by most recent dV
        bayesParams.estimatedVolume = bayesParams.estimatedVolume + bayesParams.changeInVolume;  //bayes['V'] = bayes['V'] + bayes['dV']

        //# uncertainty of V before combining with new measured value
        bayesParams.uncertaintyVolumeEstimate = bayesParams.uncertaintyVolumeEstimate + bayesParams.uncertaintyVolumeChange;  //bayes['varV'] = bayes['varV'] + bayes['vardV']

        //if abs(dV2) > 10 * bayes['vardV'] * *0.5 and abs(dP2) > bayes['vardP'] * *0.5 and PID['fineControl'] == 1:
        
        float fabsDv2 = 0.0f;
        float fabsDp2 = 0.0f;
        float sqrtUncertaintyVolChange = 0.0f;
        float tempUncertaintyVolChange = 0.0f;
        float sqrtUncertaintyPressDiff = 0.0f;
        float fabsChangeInVol = fabs(bayesParams.changeInVolume);
        float sqrtUncertaintyVolumeChange = sqrt(bayesParams.uncertaintyVolumeChange);
        float tempUncertaintyVolumeChange = 10.0f * sqrtUncertaintyVolumeChange;
        float fabsChangeInPress = fabs(bayesParams.changeInPressure);
        
        fabsDv2 = fabs(dV2);
        fabsDp2 = fabs(dP2);
        
        sqrtUncertaintyVolChange = sqrt(bayesParams.uncertaintyVolumeChange);
        tempUncertaintyVolChange = 10.0f * sqrtUncertaintyVolChange;
        sqrtUncertaintyPressDiff = sqrt(bayesParams.uncertaintyPressureDiff);
        
        if((fabsDv2 > tempUncertaintyVolChange) && 
            (fabsDp2 > sqrtUncertaintyPressDiff) &&
            (1u == controllerStatus.bit.fineControl))
        /* Commented for not satisfying MISRA, replaced by above
        if ((fabs(dV2) > (10.0f * sqrt(bayesParams.uncertaintyVolumeChange))) &&
            (fabs(dP2) > sqrt(bayesParams.uncertaintyPressureDiff)) &&
            ((1u == controllerStatus.bit.fineControl)))
        */
        {
            bayesParams.algorithmType = eRegressionMethod; //bayes['algoV'] = 1

            /*
            # print('*')
            # Estimate volume with linear fit to gas law equation.
            # Done only during fine control where
            # the two most recent measurements(dP, dV) and (dP_, dV_)
            # are significantly different from each other so that
            # slope and intercept of a regression line are uniquely determined.
            # This method tends to underestimate volume if dV2 is small
            # or at low pressures because of non - linear noise - shaping near the gas - law equation singularity.
            # Unclear how to improve it, hence restricting method to "large" dV2 only, where the
            # method gives more accurate estimates.
            # Empirical results indicate regression method leak estimate is
            # very noisy compared to the volume estimate.
            # Unclear why, possibly because leak parameter effect is less observable in short - term data,
            #and so cannot be accurately estimated from only two consecutive measurements.Adiabatic
            # decay also looks like a large, time - varying leak,
            # which would lead to an inaccurate estimate of the steady - state leak rate.
            # The long - term effect of an inaccurate leak estimate can be substantial and take a long time to decay to zero
            # via the prediction error adjustment method.It is therefore safer to assume the leak rate is zero and adjust
            # it up rather than adjust a large inaccurate value down to zero.If the leak rate is truly a large value
            # then the zero - assumption will cause longer settling time, but for a good reason(there is large leak).
            # Hence, ignore the regression method leak estimateand just use the volume estimate.This minimizes the
            # effect of transient adiabatic effects or large leaks on volume estimation error.
            # Steady - state leak estimation done by another method(filtered prediction error) in Step 2.

            # measured volume estimate with linear fit(slope), (mL)
            */
            bayesParams.measuredVolume = -bayesParams.measuredPressure * (dV2 / dP2); //bayes['measV'] = -bayes['P'] * (dV2 / dP2)

            //# uncertainty in measured volume estimate with Bayes regression(mL)
            //bayes['varMeasV'] = bayes['varP'] * (dV2 / dP2) * *2 + \
            //2 * bayes['vardP'] * (bayes['P'] * dV2 / dP2 * *2) * *2 + \
            //2 * bayes['vardV'] * (bayes['P'] / dP2) * *2
            float temporaryVariable1 = 0.0f;
            float temporaryVariable2 = 0.0f;
            float temporaryVariable3 = 0.0f;

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

            /*
            dp2Sqare = dP2 * dP2;
            temporaryVariable1 = bayesParams.sensorUncertainity * (dV2 / dP2) * (dV2 / dP2);
            temporaryVariable2 = 2.0f * bayesParams.uncertaintyPressureDiff * (bayesParams.measuredPressure * dV2 / (dp2Sqare)) * (bayesParams.measuredPressure * dV2 / (dp2Sqare));
            temporaryVariable3 = 2 * bayesParams.uncertaintyVolumeChange * (bayesParams.measuredPressure / dP2) * (bayesParams.measuredPressure / dP2);
            bayesParams.uncertaintyMeasuredVolume = temporaryVariable1 + temporaryVariable2 + temporaryVariable3;
            */
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
            bayesParams.measuredVolume = (float)(fmax((-1.0f) * bayesParams.measuredPressure * (bayesParams.changeInVolume / bayesParams.changeInPressure), 0.0));
            //# uncertainty in measured volume(mL)
            //bayes['varMeasV'] = bayes['varP'] * (bayes['dV'] / bayes['dP']) * *2 + \
            //bayes['vardP'] * (bayes['P'] * bayes['dV'] / bayes['dP'] * *2) * *2 + \
            //bayes['vardV'] * (bayes['P'] / bayes['dP']) * *2
            float temporaryVariable1 = 0.0f;
            float temporaryVariable2 = 0.0f;
            float temporaryVariable3 = 0.0f;

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
        else if (1u == controllerStatus.bit.fineControl)
        {
            //# use Prediction Error method to slowly adjust estimates of volume and leak rate
            //# during fine control loop when close to setpoint
            bayesParams.algorithmType = ePredictionErrorMethod; //bayes['algoV'] = 3

            //# measured residual leak rate from steady - state pressure error(mbar / iteration)
            float measL = 0.0f;
            residualL = ((-1.0f) * bayesParams.smoothedPressureErrForPECorrection) / bayesParams.estimatedKp;
			bayesParams.residualLeakRate = residualL;

            //# estimate of true leak rate magnitude(mbar / iteration)
            measL = fabs(residualL + bayesParams.estimatedLeakRate);
			bayesParams.measuredLeakRate1 = measL;
            if ((int32_t)1 == bayesParams.predictionErrType)
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
                float tempResidualL = residualL * bayesParams.estimatedLeakRate;
                float tempEstLeakRate = 2.0f * fabs(bayesParams.estimatedLeakRate);
                
                if ((tempResidualL >= 0.0f) &&
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
                    bayesParams.measuredVolume = bayesParams.estimatedVolume * bayesParams.gamma;//bayes['measV'] = bayes['V'] * bayes['gamma']
                    bayesParams.uncertaintyMeasuredVolume = bayesParams.uncertaintyVolumeEstimate;//bayes['varMeasV'] = bayes['varV']
                    bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate / bayesParams.gamma; //bayes['leak'] = bayes['leak'] / bayes['gamma']
                }
                //# print('***-')
            }
            if ((int32_t)-1 == bayesParams.predictionErrType)
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
                float tempResidualL = residualL * bayesParams.estimatedLeakRate;
                float tempEstLeakRate = 2.0f * fabs(bayesParams.estimatedLeakRate);
                
                if ((tempResidualL >= 0.0f) &&
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
                    bayesParams.measuredVolume = bayesParams.estimatedVolume / bayesParams.gamma; //bayes['measV'] = bayes['V'] / bayes['gamma']
                    bayesParams.uncertaintyMeasuredVolume = bayesParams.uncertaintyVolumeEstimate; //bayes['varMeasV'] = bayes['varV']
                    bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate * bayesParams.gamma; //bayes['leak'] = bayes['leak'] * bayes['gamma']
                }
                //# print('***+')
            }
        }
        else
        {
            /* Required for MISRA */
        }
        
        //# combine prior volume estimate with latest measured volume, if available
        if (bayesParams.measuredVolume != 0.0f)
        {
            //# probability weighted average of prior and measured values(mL)
            //bayes['V'] = (bayes['measV'] * bayes['varV'] + bayes['V'] * bayes['varMeasV']) / (bayes['varMeasV'] + bayes['varV'])
            bayesParams.estimatedVolume = ((bayesParams.measuredVolume * bayesParams.uncertaintyVolumeEstimate) + (bayesParams.estimatedVolume * bayesParams.uncertaintyMeasuredVolume)) / (bayesParams.uncertaintyMeasuredVolume + bayesParams.uncertaintyVolumeEstimate);

            //# new uncertainty in estimated volume(mL)
            //bayes['varV'] = bayes['varMeasV'] * bayes['varV'] / (bayes['varMeasV'] + bayes['varV'])
            bayesParams.uncertaintyVolumeEstimate = (bayesParams.uncertaintyMeasuredVolume * bayesParams.uncertaintyVolumeEstimate) / (bayesParams.uncertaintyMeasuredVolume + bayesParams.uncertaintyVolumeEstimate);

            //# increase varV if measV is not within maxZScore standard deviations of V
            //# increases convergence rate to true value in the event of a step change in system volume
            float du = 0.0f;
            //du = abs(bayes['V'] - bayes['measV'])
            du = fabs(bayesParams.estimatedVolume - bayesParams.measuredVolume);

            //bayes['varV'] = max(bayes['varV'], du / bayes['maxZScore'] - bayes['varMeasV'])
            bayesParams.uncertaintyVolumeEstimate = fmax(bayesParams.uncertaintyVolumeEstimate, du / (bayesParams.maxZScore - bayesParams.uncertaintyMeasuredVolume));
            //# bound updated volume estimate
            //bayes['V'] = max(min(bayes['V'], bayes['maxV']), bayes['minV'])
            bayesParams.estimatedVolume = fmax(fmin(bayesParams.estimatedVolume, bayesParams.maxSysVolumeEstimateValue), bayesParams.minSysVolumeEstimateValue);
        }
        //# Step 2
        //# Adjust leak rate estimate

        //if bayes['algoV'] == 3:
        if (ePredictionErrorMethod == bayesParams.algorithmType)
        {
            /*
            # Use dynamically averaged pressure error when near setpoint to adjust leak rate.
            # Leak rate averaging window length is inversely proportional to the
            # larger of the estimated leak rate or pressure error.
            # This makes the filter react faster when either the pressure error or
            # leak rate estimate is "large" to improve convergence rate to true values,
            # while providing accurate leak rate correction for small leaks.
            */
            //# limit filter length to maxN control iterations
            float minError = 0.0f;
            float maxError = 0.0f;
            uint32_t numOfIterations = 0u;
            minError = sqrt(bayesParams.uncertaintyPressureDiff) / ((float)bayesParams.maxIterationsForIIRfilter);//minError = bayes['vardP'] * *0.5 / bayes['maxN']

            //# use largest of pressure error or leak rate estimate to define IIR filter length
            //# larger values means shorter observation time / faster response
            maxError = fmax(fmax(fabs(bayesParams.smoothedPressureErrForPECorrection), fabs(bayesParams.estimatedLeakRate)), minError); //maxError = max(abs(bayes['smoothE_PE']), abs(bayes['leak']), minError)

            //# number of control iterations to average over(minN, maxN)
            numOfIterations = (uint32_t)(max((sqrt(bayesParams.uncertaintyPressureDiff) / maxError), (float)(bayesParams.minIterationsForIIRfilter))); //n = max(int(bayes['vardP'] * *0.5 / maxError), bayes['minN'])
            bayesParams.numberOfControlIterations = numOfIterations;
            /*
            # Average E over more iterations when leak rate estimate is small and averaged pressure error is small
            # so that the expected value of the pressure error from a leak
            # is at least as big as pressure measurement noise.
            */

            //# IIR filter memory factor to achieve epsilon * initial condition residual after n iterations
            bayesParams.alpha = (float)(pow(10, (bayesParams.log10epsilon / (float)(numOfIterations))));//bayes['alpha'] = 10 * *(bayes['log10epsilon'] / n)

            //# smoothed pressure error with dynamic IIR filter
            //bayes['smoothE_PE'] = (1 - bayes['alpha']) * PID['E'] + bayes['alpha'] * bayes['smoothE_PE']
            bayesParams.smoothedPressureErrForPECorrection = (1.0f - bayesParams.alpha) * pidParams.pressureError + (bayesParams.alpha * bayesParams.smoothedPressureErrForPECorrection);

            //# measured residual leak rate from steady - state pressure error(mbar / iteration)
            residualL = (-1.0f) * bayesParams.smoothedPressureErrForPECorrection / bayesParams.estimatedKp;//residualL = -bayes['smoothE_PE'] / bayes['kP']
			bayesParams.residualLeakRate = residualL;
            /*
            # Apply 1 / n of total correction to reduce remaining leak error gradually(mbar / iteration)
            # Correction to leak estimate must be gradual because leak effect takes time to accurately measure.
            # Applying entire dL correction to leak rate after
            # each control iteration could cause sustained oscillation in pressure error,
            # similar to "integrator windup" in a PI controller when the "kI" term is too large.
            # This controller's correction for leak rate is a feed-forward compensation with gain kP and
            # leak rate estimation error inferred from smoothed pressure error.
            # At steady - state, feed - forward control allows the controller to anticipate the effect
            # of the leak before it accumulates as a significant pressure error, using historical leak disturbance
            # to compensate for future effects.This is particularly helpful for minimizing
            # steady - state pressure error for leak rates greater than the measurement
            # noise when the iteration rate of the control loop is slow.
            */
            bayesParams.changeToEstimatedLeakRate = (residualL / (float)numOfIterations);//bayes['dL'] = residualL / n

            //# Special cases :

            if (residualL * bayesParams.estimatedLeakRate < 0.0f)
            {
                /*
                 # Residual pressure error is opposite sign to estimated leak rate
                 # It could take a long time to correct this estimation error while the
                 # pressure error continues to accumulate.
                 # Speed - up convergence to the correct leak rate value by removing all dL in one iteration
                 # This may temporarily cause the pressure error to
                 # oscillate, but reduces the maximum error
                 # that would otherwise accumulate.
                 # This situation could happen if the true leak rate is suddenly changed
                 # while hovering around setpoint, e.g.by user stopping the leak.
                 */
                bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate + (bayesParams.changeToEstimatedLeakRate * (float)(numOfIterations));
                //# print('***L++')
            }
            else
            {
                //# Residual pressure error is the same sign as estimated leak.
                //# Adjust leak rate estimate a small amount to minimize its effect
                bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate + bayesParams.changeToEstimatedLeakRate;//bayes['leak'] = bayes['leak'] + bayes['dL']
                /*
                    # Note: If dV == 0 because of low - power mode control operation then the leak rate correction will grow
                # larger than the "correct" value because the leak effect is not being offset with feed - forward control.
                # Prolonged operation with non - zero leak rate and dV == 0 may cause leak estimate to increase
                # above true value.This is ok because effectively the control loop iteration time is increased as well,
                #and so the leak rate per control iteration is truly increasing.However, if there is a disturbance
                # the controller will revert back to a faster iteration rate, which may cause temporary oscillation while
                # the leak rate estimate is nudged back down.

                # varLeak the variance of low - pass filtered pressure sensor noise,
                # which decreases as 1 / n for large n(Poisson statistics)
                # n is the low - pass filter observation time(iterations)
                */
                bayesParams.uncertaintyEstimatedLeakRate = bayesParams.uncertaintyPressureDiff / (float)(numOfIterations); //bayes['varLeak'] = (bayes['vardP'] / n)

                //# increase leak uncertainty if measLeak is not within maxZScore standard deviations of leak
                //# increases convergence to true value in the event of a step change in leak rate
                float du = fabs(bayesParams.estimatedLeakRate - bayesParams.measuredLeakRate);//du = fabs(bayes['leak'] - bayes['measLeak'])
                bayesParams.uncertaintyEstimatedLeakRate = max(bayesParams.uncertaintyEstimatedLeakRate, du / (bayesParams.maxZScore - bayesParams.uncertaintyMeasuredLeakRate));//bayes['varLeak'] = max(bayes['varLeak'], du / bayes['maxZScore'] - bayes['varMeasLeak'])
            }
        }
        else
        {
            
        }
        //# volume estimated with gas law
        //# keep previous leak estimate
        //estimatedLeakRate = //bayes['leak'] = bayes['leak']
        //bayes['varLeak'] = bayes['varLeak']

        //# bound leak correction to reasonable values, scaled by volume estimate and pressure
        //# higher pressure and lower volume can have larger leak rate estimate
        //bayes['maxLeak'] = screw['maxLeak'] / bayes['V'] * bayes['P'] / screw['maxP'] * screw['nominalV']
        //(screw['maxLeak'] / bayes['V'])* (bayes['P'] / screw['maxP'])* screw['nominalV']
        bayesParams.maxEstimatedLeakRate = (screwParams.maxLeakRate / bayesParams.estimatedVolume) * (bayesParams.measuredPressure / screwParams.maxAllowedPressure) * screwParams.nominalTotalVolume;
        //bayes['leak'] = min(max(bayes['leak'], -bayes['maxLeak']), bayes['maxLeak'])
        bayesParams.estimatedLeakRate = min(max(bayesParams.estimatedLeakRate, (-1.0f * bayesParams.maxEstimatedLeakRate)), bayesParams.maxEstimatedLeakRate);

        //# Calculate optimal controller gain for next control iteration
        //# optimal kP to achieve correction of pressure error in one step(steps / mbar)
        bayesParams.estimatedKp = bayesParams.estimatedVolume / (bayesParams.measuredPressure * screwParams.changeInVolumePerPulse); //bayes['kP'] = bayes['V'] / (bayes['P'] * screw['dV'])

        if (1u == controllerStatus.bit.fineControl)
        {
            /*
            # in fine control loop
            # calculate pressure range estimate using optical sensor
            #and estimated volume to judge remaining travel range in pressure units
            # Not currently used to make control decisions but could be useful for future work
            # on rampsand to detect imminent excessiveRange fails before they occur.
            */
            //# maximum volume achievable(mL)
            float maxV = 0.0f;
            float minV = 0.0f;
            float minV2 = 0.0f;
            float maxV2 = 0.0f;
            float maxV2steps = 0.0f;
            float minV2steps = 0.0f;

            //maxV = max(min(bayes['V'] + (PID['position'] - screw['minPosition']) * screw['dV'], bayes['maxV']), bayes['V'])
            maxV = max(min(bayesParams.estimatedVolume + (pidParams.pistonPosition - screwParams.minPosition) * screwParams.changeInVolumePerPulse, bayesParams.maxSysVolumeEstimateValue), bayesParams.estimatedVolume);
            //# minimum volume achievable(mL)
            //minV = min(max(bayes['V'] - (screw['maxPosition'] - PID['position']) * screw['dV'], bayes['minV']), bayes['V'])
            minV = min(max(bayesParams.estimatedVolume - (screwParams.maxPosition - pidParams.pistonPosition) * screwParams.changeInVolumePerPulse, bayesParams.minSysVolumeEstimateValue), bayesParams.estimatedVolume);

            //# maximum achievable pressure(mbar)
            //bayes['maxP'] = bayes['P'] * bayes['V'] / minV
            bayesParams.maxAchievablePressure = bayesParams.measuredPressure * bayesParams.estimatedVolume / minV;
            //# minimum achievable pressure(mbar)
            //bayes['minP'] = bayes['P'] * bayes['V'] / maxV
            bayesParams.minAchievablePressure = bayesParams.measuredPressure * bayesParams.estimatedVolume / maxV;

            //# largest positive pressure change(mbar)
            bayesParams.maxPositivePressureChangeAchievable = bayesParams.maxAchievablePressure - bayesParams.measuredPressure;//bayes['maxdP'] = bayes['maxP'] - bayes['P']

            //# largest negative pressure change(mbar)
            bayesParams.maxNegativePressureChangeAchievable = bayesParams.minAchievablePressure - bayesParams.measuredPressure;

            //# volume required to achieve nominalRange % increase in pressure from current pressure
            minV2 = bayesParams.estimatedVolume / (1.0f + bayesParams.minPressureAdjustmentRangeFactor);//minV2 = bayes['V'] / (1 + bayes['nominalRange'])

            //# volume required to achieve nominalRange % decrease in pressure from current pressure
            maxV2 = bayesParams.estimatedVolume / (1.0f - bayesParams.minPressureAdjustmentRangeFactor);//maxV2 = bayes['V'] / (1 - bayes['nominalRange'])

            //# number of steps required to achieve maxV2 target(negative definite)
            maxV2steps = round(max(0.0f, ((maxV2 - maxV) / screwParams.changeInVolumePerPulse)));

            //# number of steps required to achieve minV2 target(positive definite)
            minV2steps = round(min(0.0f, (minV2 - minV) / screwParams.changeInVolumePerPulse));

            if (false == (floatEqual(minV2steps * maxV2steps, 0.0f)))
            {
                //printf(error: nominalTarget position is not achievable', minV2steps, maxV2steps)
                //# nominal home position to achieve nominalRange target(steps)
                
            }
            //bayes['nominalHome'] = min(max(PID['position'] + maxV2steps + minV2steps,screw['minPosition']), screw['maxPosition'])
            bayesParams.nominalHomePosition = (int32_t)(min(max(pidParams.pistonPosition + maxV2steps + minV2steps, screwParams.minPosition), screwParams.maxPosition));
            //# limit to allowed range of position
            //# expected pressure at center piston position
            // bayes['centerP'] = bayes['P'] * bayes['V'] / (bayes['V'] + (PID['position'] - screw['centerPosition']) * screw['dV']) //#expected pressure at center piston position
            bayesParams.expectedPressureAtCenterPosition = bayesParams.measuredPressure * bayesParams.estimatedVolume / (bayesParams.estimatedVolume + (pidParams.pistonPosition - screwParams.centerPositionCount) * screwParams.changeInVolumePerPulse);
        }
    }
}

/**
* @brief	To set pressure for set point
* @param	void
* @retval	void
*/
float DController::pressureAsPerSetPointType(void)
{
    float pressureValue = 0.0f;
    if ((eSetPointType_t)eGauge == setPointType)
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
* @brief	Read the optical sensor counts
* @param	void
* @retval	void
*/
uint32_t DController::generateSensorCalTable(void)
{
    sensorPoints[0] = (uint32_t)(OPT_SENS_PT_1);
    sensorPoints[1] = (uint32_t)(OPT_SENS_PT_2);
    sensorPoints[2] = (uint32_t)(OPT_SENS_PT_3);
    sensorPoints[3] = (uint32_t)(OPT_SENS_PT_4);
    sensorPoints[4] = (uint32_t)(OPT_SENS_PT_5);

    posPoints[0] = (uint32_t)(POSITION_PT_1);
    posPoints[1] = (uint32_t)(POSITION_PT_2);
    posPoints[2] = (uint32_t)(POSITION_PT_3);
    posPoints[3] = (uint32_t)(POSITION_PT_4);
    posPoints[4] = (uint32_t)(POSITION_PT_5);
    return 1u;
}

/**
* @brief	Read the optical sensor counts
* @param	void
* @retval	void
*/
uint32_t DController::getCalibratedPosition(uint32_t adcReading, int32_t *calPosition)
{
    uint32_t status = 0u;
    uint32_t output = 0u;
    uint32_t index = 0u;
    float m = 0.0f;
    float c = 0.0f;
    uint32_t value = adcReading;

    if (value < sensorPoints[0])
    {
        status = 0u;
    }
    else if(value > sensorPoints[4])
    {
        status = 0u;
    }
    else
    {
        status = 1u;
        for (index = 1u; index <= (uint32_t)(4); index++)
        {
            if (value < sensorPoints[index])
            {
                m = (posPoints[index] - posPoints[index - 1u]) / (sensorPoints[index] - sensorPoints[index - 1u]);
                c = m * sensorPoints[index];
                c = c - posPoints[index];
                break;
            }
        }
        output = (uint32_t)(((m * float(value)) - c));
        *calPosition = output;
    }
    return status;    
}

/*
 * @brief   Get piston position
 * @param   None
 * @retval  None
 */
uint32_t DController::getPistonPosition(uint32_t adcReading, int32_t *position)
{
    uint32_t status = 0u;

    status = getCalibratedPosition(adcReading, position);

    return status;
}


uint32_t DController::readOpticalSensorCounts(void)
{
    uint32_t value = 0u;
    
    PV624->powerManager->getValue(EVAL_INDEX_IR_SENSOR_ADC_COUNTS, &value);
                                  
    return value;
}

/*
 * @brief   Returns if two float values are almost equal
 * @param   None
 * @retval  None
 */
uint32_t DController::floatEqual(float a, float b)
{
    uint32_t status = false;
    float temp = 0.0f;

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

#pragma diag_default=Pm046 /* Disable MISRA C 2004 rule 13.3 */
#pragma diag_default=Pm137 /* Disable MISRA C 2004 rule 10.4*/
#pragma diag_default=Pm128 /* Disable MISRA C 2004 rule 10.1 */

#pragma diag_suppress=Pm128 /* Disable MISRA C 2004 rule 10.1 */
/**
* @brief	Fine control loop
* @param	void
* @retval	void
*/
void DController::fineControlLoop()
{
    eControllerError_t errorStatus = eErrorNone;
    int32_t completedCnt = (int32_t)0;
    uint32_t status = 0u;
    uint32_t timeStatus = 0u;
    uint32_t epochTime = 0u;

    timeStatus = getEpochTime(&epochTime);
    if(true == timeStatus)
    {
        coarseControlLogParams.startTime = epochTime;
        pidParams.elapsedTime = epochTime;
    }
    
    if (1u == controllerStatus.bit.fineControl)
    {
        //# read pressure with highest precision
        //pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
        //    PID['mode'] = mode
        
        //if PID['mode'] != 1 or PID['excessLeak'] == 1 or PID['excessVolume'] == 1 or \
        //    PID['overPressure'] == 1 or PID['rangeExceeded'] == 1:
        //if ((controllerStatus.bytes && 0X00FF) == 0u)
        if((myMode == E_CONTROLLER_MODE_CONTROL) &&
              (controllerStatus.bit.excessLeak != (uint32_t)1) &&
              (controllerStatus.bit.excessVolume != (uint32_t)1)&&
              (controllerStatus.bit.overPressure != (uint32_t)1)&&
              (controllerStatus.bit.rangeExceeded != (uint32_t)1))
                
        {
            //# adjust measured pressure by simulated leak rate effect
            //# control to pressure in the same units as setpoint
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType(); //[pressureG, pressure, atmPressure][spType] + testParams.fakeLeak;
            pidParams.pressureSetPoint = pressureSetPoint;

            //# store previous dP and dV values
            bayesParams.prevChangeInPressure = bayesParams.changeInPressure; //bayes['dP_'] = bayes['dP']  //# previous measured dP(mbar)
            bayesParams.prevChangeInVolume = bayesParams.changeInVolume; //bayes['dV_'] = bayes['dV']  //# previous applied dV(mL)

            //# change in pressure from last measurement(mbar), +ve == increasing pressure
            bayesParams.changeInPressure = absolutePressure + testParams.fakeLeak - bayesParams.measuredPressure;//bayes['dP'] = pressure + testing['fakeLeak'] - bayes['P']

            //# use absolute pressure for volume estimates using gas law
            bayesParams.measuredPressure = absolutePressure + testParams.fakeLeak;// bayes['P'] = pressure + testing['fakeLeak']

            //# store previous pressure error for prediction error calculation
            bayesParams.targetdP = pidParams.pressureError;    //bayes['targetdP'] = PID['E']
            /*
            # Note: Unlike PID['targetdP'], bayes['targetdP']
            # is not adjusted for leak estimate.
            # The leak estimate may be incorrectand the bayes prediction error
            # should therefore be non - zero even if the PID['targetdP'] was achieved.
            */

            //# pressure error error(mbar), -ve if pressure above setpoint
            pidParams.pressureError = pidParams.pressureSetPoint - pidParams.controlledPressure;//PID['E'] = PID['setpoint'] - PID['pressure']

            //# target pressure correction after anticipating leak effect(mbar)
            pidParams.pressureCorrectionTarget = pidParams.pressureError - bayesParams.estimatedLeakRate;//PID['targetdP'] = PID['E'] - bayes['leak']

            //# steps to take to achieve zero pressure error in one control iteration
            pidParams.stepSize = int(round(bayesParams.estimatedKp * pidParams.pressureCorrectionTarget)); //PID['stepSize'] = int(round(bayes['kP'] * PID['targetdP']))

            //# setpoint achievable status(True / False)
            //# TBD use this to abort setpoint if not achievable before reaching max / min piston positions
            //PID['inRange'] = (PID['E'] > bayes['mindP']) and (PID['E'] < bayes['maxdP'])
            pidParams.isSetpointInControllerRange = (pidParams.pressureError > bayesParams.maxNegativePressureChangeAchievable) && (pidParams.pressureError < bayesParams.maxPositivePressureChangeAchievable);

            //# abort correction if pressure error is within the measurement noise floor to save power,
            //# also reduces control noise when at setpoint without a leak
            float fabsPressCorrTarget = fabs(pidParams.pressureCorrectionTarget);
            float sqrtUncertaintyPressDiff = sqrt(bayesParams.uncertaintyPressureDiff);
            float fabsPressureError = fabs(pidParams.pressureError);
           
            if((fabsPressCorrTarget < sqrtUncertaintyPressDiff) &&
               (fabsPressureError < sqrtUncertaintyPressDiff))
            /* Commented for MISRA, replaced by above 
            if ((fabs(pidParams.pressureCorrectionTarget) < sqrt(bayesParams.uncertaintyPressureDiff)) &&
                (fabs(pidParams.pressureError) < sqrt(bayesParams.uncertaintyPressureDiff)))                
            */
            {
                pidParams.pressureCorrectionTarget = 0.0f;// PID['targetdP'] = 0.0f;
                pidParams.stepSize = 0u; //PID['stepSize'] = 0.0f;
                //print('*LP')
            }
            //# increase motor current with gauge pressure
            //# saves power for lower pressure setPoints
            //PID['current'] = screw['minCurrent'] + (screw['maxCurrent'] - screw['minCurrent']) * \
            //abs(pressureG) / screw['maxPressure']
            pidParams.requestedMeasuredMotorCurrent = motorParams.minMotorCurrrent + (motorParams.maxMotorCurrent - motorParams.minMotorCurrrent) * \
                fabs(gaugePressure) / screwParams.maxPressure;

#ifdef DIFFERENT_CURRENTS
            //motor->writeAcclCurrent(pidParams.requestedMeasuredMotorCurrent);// SetAcclCurrent(PID['current'])
            //motor->writeDecelCurrent(pidParams.requestedMeasuredMotorCurrent);//(PID['current'])
#endif

#ifndef DIFFERENT_CURRENTS
            //motor->writeCurrent(pidParams.requestedMeasuredMotorCurrent);
#endif
            /*
            # request new stepSize and read back previous stepCount
            # motor acceleration limits number of steps that can be taken per iteration
            # so count != stepSize except when controlling around setpoint with small stepSize
            */
            //pidParams.stepCount = (eControllerError_t)motor->move(pidParams.stepSize); //.MOTOR_MoveContinuous(pidParams.stepSize);
#ifdef ENABLE_MOTOR            
            errorStatus = (eControllerError_t)PV624->stepperMotor->move(pidParams.stepSize, &completedCnt);      //# stop the motor
#endif
            if ((eControllerError_t)(eErrorNone) == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }
            //# total number of steps taken
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;

            //# change in volume(mL)
            bayesParams.changeInVolume = -screwParams.changeInVolumePerPulse * (float)(pidParams.stepCount);

            //# read the optical sensor piston position
            pidParams.opticalSensorAdcReading = readOpticalSensorCounts();//pv624.readOpticalSensor();
            status = getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition);
            controllerStatus.bit.rangeExceeded = validatePistonPosition(pidParams.pistonPosition);
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            if(1u != controllerStatus.bit.rangeExceeded)
            {              
                /* Check if piston is centered */
                controllerStatus.bit.pistonCentered = isPistonCentered(pidParams.pistonPosition);
                calcStatus();
                //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));              
            }
#ifdef L6472
            float currentADC = 0.0f;
            //need to uncomment PID['speed'], currentADC = motor.GetSpeedAndCurrent();

            pidParams.measuredMotorCurrent = int(currentADC * screwParams.readingToCurrent);
#endif
            //# update system parameter estimates from latest measurement data
            estimate();

            //# report back status to GENII
            //# calcStatus() imported from dataStructures4.py
            //calcStatus(PID = PID)
            //pv624.setControllerStatus(PID['status'])
            //# and abort on error or mode change
            /*
            # Code below used for testing controller performance, not for product use :
            #------------------------------------------------------------------------
            # scale maxFakeLeakRate by measured gage pressureand test volume
            # Don't use volume estimate to scale leak rate
            # so that errors in the estimate do not affect applied leak rate
            #and the true leak rate can be back - calculated from pressure and nominal test volume.
            */
            testParams.fakeLeakRate = -fabs(testParams.maxFakeLeakRate) * \
                gaugePressure / screwParams.maxAllowedPressure * screwParams.nominalTotalVolume / testParams.volumeForLeakRateAdjustment;

            //# new cumulative pressure error from the simulated leak(mbar)
            //testing['fakeLeak'] = testing['fakeLeak'] + testing['fakeLeakRate']
            testParams.fakeLeak = testParams.fakeLeak + testParams.fakeLeakRate;
            //# Code below to be replaced with logging of data to PV624 non - volatile memory
            //# for diagnostic purposes.# -------------------------------------------------
            /*
            if logging['logData']:
            //# get timestamp
            PID['elapsedTime'] = round((datetime.now() - logging['startTime']).total_seconds(), 3)
            //# append dynamic data to dataFile
            csvFile = csv.writer(f, delimiter = ',')
            csvFile.writerow(list(PID.values()) + list(bayes.values()) + list(testing.values()))

            # write a small subset of global data structure content to screen
            printValues = [round(PID['pressure'], 2),
            round(PID['E'], 2),
            round(bayes['V'], 1),
            round(bayes['leak'], 4),
            round(testing['fakeLeakRate'], 4),
            round(PID['position']),
            round(PID['measCurrent'], 2)
            ]

            formatString = len(printValues) * '{: >8} '
            //print(formatString.format(*printValues))
                */
#ifdef CPP_ON_PC
            logPidBayesAndTestParamDataValues(eFineControl);
            printf_s("\n\r%f\t%f\t%f\t%f\t%d\t%f", 
                        pidParams.controlledPressure,
                        pidParams.pressureError,
                        bayesParams.estimatedVolume,
                        bayesParams.estimatedLeakRate,
                        testParams.fakeLeakRate,
                        pidParams.pistonPosition,
                        pidParams.measuredMotorCurrent);
#endif
        }
        else
        {
            //# Genii has changed operating mode to vent or measure
            //# or a control error has occurred
            //# abort fine control and return to coarse control loop
            controllerStatus.bit.fineControl = 0u;
            controllerStatus.bit.stable = 0u;
            controllerState = eCoarseControlLoop; 
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
        }
    }
    else
    {
        /* Exit fine control */
        //controllerState = eCoarseControlLoopEntry;   
    }
    
}
#pragma diag_default=Pm128 /* Disable MISRA C 2004 rule 10.1 */

/**
 * @brief   Runs once before fine control loop is entered
 * @param   None
 * @retval  None
 */
void DController::fineControlSmEntry(void)
{
    uint32_t epochTime = 0u;
    bool status = false;
    status = getEpochTime(&epochTime);
    if(true == status)
    {
      fineControlLogParams.startTime = epochTime;
    }

    resetBayesParameters();
#ifdef RUN_ON_MICRO
    sprintf_s(fineControlLogParams.fileName, 80u, "%d-%02d-%02d%02d:%02d:%02d_%3.7f_%d",
                                                    ltm->tm_year + 1900,
                                                    ltm->tm_mon + 1,
                                                    ltm->tm_mday,
                                                    ltm->tm_hour,
                                                    ltm->tm_min,
                                                    ltm->tm_sec,
                                                    pidParams.pressureSetPoint,
                                                    pidParams.setPointType);
#else
   // strcpy_s(fineControlLogParams.fileName, fineControlFileName);
#endif

#ifdef CPP_ON_PC
    logDataKeys((eControllerType_t)eFineControl);
    logPidBayesAndTestParamDataValues((eControllerType_t)eFineControl);
#endif
    controllerState = eFineControlLoop;    
}

/*
 * @brief   Calculates the status as required by the DPI
 * @param   None
 * @retval  None
 */
void DController::calcStatus(void)
{
    ctrlStatusDpi.bytes = 0u;

    if(1u == controllerStatus.bit.pumpUp) 
    {
        ctrlStatusDpi.bits.pumpUp = 1u; 
    }
    else if(1u == controllerStatus.bit.pumpDown)
    {
        ctrlStatusDpi.bits.pumpDown = 1u;     
    }
    else if(controllerStatus.bit.control == 1u)
    {
        ctrlStatusDpi.bits.control = 1u;        
    }          
    else if(controllerStatus.bit.venting == 1u)
    {
        ctrlStatusDpi.bits.venting = 1u;        
    }          
    else if(1u == controllerStatus.bit.stable)
    {
        ctrlStatusDpi.bits.stable = 1u;       
    }
    else if(1u == controllerStatus.bit.measure)
    {
        ctrlStatusDpi.bits.measuring = 1u;
    }
    else if(controllerStatus.bit.vented == 1u)
    {
        ctrlStatusDpi.bits.vented = 1u;        
    }        
    else if(controllerStatus.bit.excessLeak == 1u)
    {
        ctrlStatusDpi.bits.excessLeak = 1u;        
    }  
    else if(controllerStatus.bit.excessVolume == 1u)
    {
        ctrlStatusDpi.bits.excessVolume = 1u;        
    }  
    else if(controllerStatus.bit.overPressure == 1u)
    {
        ctrlStatusDpi.bits.overPressure = 1u;        
    }          
    else
    {
    }
    PV624->setControllerStatusPm((uint32_t)(controllerStatus.bytes));
    PV624->setControllerStatus((uint32_t)(ctrlStatusDpi.bytes));
}

/*
 * @brief   Measure mode in coarase control
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlMeasure()
{
    uint32_t status = (uint32_t)(0);
    controllerStatus.bytes = 0u;
    controllerStatus.bit.coarseControlError = eCoarseControlErrorReset;
    controllerStatus.bit.measure = (uint32_t)1;
    
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));

    return status;
}

/*
 * @brief   Get piston in range
 * @param   None
 * @retval  None
 */
ePistonRange_t DController::validatePistonPosition(int32_t position)
{
    ePistonRange_t isInRange = ePistonOutOfRange;

    if (screwParams.maxPosition > position)
    {
        if (screwParams.minPosition < position)
        {
            isInRange = ePistonInRange;
        }
    }

    return isInRange;
}

/*
 * @brief   Get piston in centre
 * @param   None
 * @retval  None
 */
ePistonCentreStatus_t DController::isPistonCentered(int32_t position)
{
    int32_t positionLeft = (int32_t)(0);
    int32_t positionRight = (int32_t)(0);
    ePistonCentreStatus_t isInCentre = ePistonOutOfCentre;

    positionLeft = screwParams.centerPositionCount - screwParams.centerTolerance;
    positionRight = screwParams.centerPositionCount + screwParams.centerTolerance;

    if (positionLeft < position)
    {
        if (positionRight > position)
        {
            isInCentre = ePistonCentered;
        }
    }

    return isInCentre;
}

/**
 * @brief   Sets the motor current
 * @param   None
 * @retval  None
 */
void DController::setMotorCurrent(void)
{
    float motorCurrRange = 0.0f;
    //# increase motor current with gauge pressure
    //# saves power for lower pressure setPoints
    motorCurrRange = motorParams.maxMotorCurrent - motorParams.minMotorCurrrent;
    pidParams.requestedMeasuredMotorCurrent = motorParams.minMotorCurrrent +
        ((motorCurrRange * absolutePressure) / screwParams.maxPressure);
    //motor->writeAcclCurrent(pidParams.requestedMeasuredMotorCurrent);
    //motor->writeDecelCurrent(pidParams.requestedMeasuredMotorCurrent);
}

/**
 * @brief   Runs once before entering coarse control loop
 * @param   None
 * @retval  None
 */
void DController::coarseControlSmEntry(void)
{    
    uint32_t epochTime = 0u;
    uint32_t status = false;
    
    uint32_t sensorType = 0u;   
    
    status = getEpochTime(&epochTime);
    if(true == status)
    {
        coarseControlLogParams.startTime = epochTime;
        pidParams.elapsedTime = epochTime;
    }
    float uncertaintyScaling = (float)0.0;

#ifdef RUN_ON_MICRO
    sprintf_s(coarseControlLogParams.fileName,80u, "% d-%02d-%02d%02d:%02d:%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
#else
    //strcpy_s(coarseControlLogParams.fileName, coarseControlFileName);
#endif
    //# reset control status flags
    //    # TBD action when control error flags != 0 ?
    controllerStatus.bit.stable = 0u;
    controllerStatus.bit.excessLeak = 0u;
    controllerStatus.bit.excessVolume = 0u;
    controllerStatus.bit.overPressure = 0u;
    controllerStatus.bit.rangeExceeded = 0u;
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));

    PV624->getPosFullscale(&sensorFsValue);
    PV624->getPM620Type(&sensorType);
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));    
    //# scale default measurement uncertainties to PM FS value
    if((sensorType & (uint32_t)(PM_ISTERPS)) == 1u)
    {
        /* scale 4 times for terps */
        uncertaintyScaling = 16.0f * (sensorFsValue / fsValue) * (sensorFsValue / fsValue);
    }
    else
    {
        uncertaintyScaling = 1.0f * (sensorFsValue / fsValue) * (sensorFsValue / fsValue);
    }
    fsValue = sensorFsValue;  //# PM full scale pressure(mbar)
    bayesParams.sensorUncertainity = uncertaintyScaling * bayesParams.sensorUncertainity; //bayes['varP']  //# uncertainty in pressure measurement(mbar)
    bayesParams.uncertaintyPressureDiff = uncertaintyScaling * bayesParams.uncertaintyPressureDiff;  //# uncertainty in measured pressure changes(mbar)

    controllerStatus.bit.fineControl = 0u;  //# disable fine pressure control
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
    //# detect position of the piston
    pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
    status = getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition); 
    controllerStatus.bit.pistonCentered = isPistonCentered(pidParams.pistonPosition);    //PID['pistonCentered'] = (
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
    //# assume optical position reading is accurate
    pidParams.totalStepCount = pidParams.pistonPosition;

#ifdef CPP_ON_PC
    logDataKeys((eControllerType_t)eCoarseControl);
    logPidBayesAndTestParamDataValues((eControllerType_t)eCoarseControl);
    logScrewAndMotorControlValues((eControllerType_t)eCoarseControl);
#endif
    controllerState = eCoarseControlLoop;
}

/**
 * @brief   Runs once after exiting from coarse control
 * @param   None
 * @retval  None
 */
void DController::coarseControlSmExit(void)
{
    //# coarse adjustment complete, last iteration of coarse control loop
    controllerStatus.bit.fineControl = (uint32_t)1;  //# exiting coarse control after this iteration
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
    pidParams.stepSize = (int32_t)(0);
    //# read pressure and position once more before moving to fine control
    pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
    getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition); 
    //pidParams.controlledPressure = pressureAsPerSetPointType();//# pressure in setpoint units
    controllerStatus.bit.rangeExceeded = ePistonInRange; //# reset rangeExceeded flag set by fineControl()
    calcStatus();
    //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
    //controllerState = eFineControlLoopEntry;
}

#pragma diag_suppress=Pm128 /* Disable MISRA C 2004 rule 10.1 */
/**
 * @brief   Coarse control main control loop
 * @param   None
 * @retval  None
 */
void DController::coarseControlLoop(void)
{
    uint32_t caseStatus = (uint32_t)(0);
    uint32_t epochTime = 0u;
    uint32_t timeStatus = 0u;
    uint32_t status = (uint32_t)(0);
    //eControllerMode_t mode = PV624->getMode(); /* read mode set by Genii */
    eControllerError_t errorStatus = eErrorNone;
    int32_t completedCnt = (int32_t)0;
    
    timeStatus = getEpochTime(&epochTime);
    if(true == timeStatus)
    {
        coarseControlLogParams.startTime = epochTime;
        pidParams.elapsedTime = epochTime;
    }
    
    if (eFineControlDisabled == controllerStatus.bit.fineControl) /* Run coarse control only if fine control is not enabled */
    {
        if ((E_CONTROLLER_MODE_MEASURE == myMode) && ((uint32_t)(1) == controllerStatus.bit.control))
        {
            /* Mode set by genii is measure but PID is in control mode */
            /* Change mode to measure */
            setMeasure();
        }
        else if ((E_CONTROLLER_MODE_MEASURE == myMode) && ((uint32_t)(1) == controllerStatus.bit.venting))
        {
            /* Mode set by genii is measure but PID is venting */
            /* Change mode to measure */
            setMeasure();
        }
        else if ((E_CONTROLLER_MODE_CONTROL == myMode) && ((uint32_t)(1) == controllerStatus.bit.measure))
        {
            /* Mode set by genii is control but PID is in measure */
            /* Change mode to measure */
            setControlIsolate();
        }
        else if ((E_CONTROLLER_MODE_CONTROL == myMode) && ((uint32_t)(1) == controllerStatus.bit.venting))
        {
            /* Mode set by genii is control but PID is in measure */
            /* Change mode to measure */
            setControlIsolate();
        }
        else if ((E_CONTROLLER_MODE_VENT == myMode) && ((uint32_t)(1) == controllerStatus.bit.measure))
        {
            /* Mode set by genii is control but PID is in measure */
            /* Change mode to measure */
            ventReadingNum = eControlVentGetFirstReading;
            setVent();
        }
        else if ((E_CONTROLLER_MODE_VENT == myMode) && ((uint32_t)(1) == controllerStatus.bit.control))
        {
            /* Mode set by genii is control but PID is in measure */
            /* Change mode to measure */
            ventReadingNum = eControlVentGetFirstReading;
            setVent();
        }
        else if (E_CONTROLLER_MODE_MEASURE == myMode)
        {
            /* Mode is correct, so reset coarse control error */
            controllerStatus.bit.coarseControlError = eCoarseControlErrorReset;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            pidParams.controlledPressure = pressureAsPerSetPointType(); //[pressureG, pressure, atmPressure][spType] + testParams.fakeLeak;
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.setPointType = setPointType;
            coarseControlMeasure();
        }
        else if (E_CONTROLLER_MODE_CONTROL == myMode)
        {
            /* Mode is correct, so reset coarse control error */
            controllerStatus.bit.coarseControlError = eCoarseControlErrorReset;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));

            /* Read gauge pressure also */
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
            setPointG = pidParams.pressureSetPoint - (atmosphericPressure * setPointType);

            /* Check screw position */
            status = getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition); 
            /* Check if piston is within range */
            controllerStatus.bit.rangeExceeded = validatePistonPosition(pidParams.pistonPosition);
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            if(1u != controllerStatus.bit.rangeExceeded)
            {              
                /* Check if piston is centered */
                controllerStatus.bit.pistonCentered = isPistonCentered(pidParams.pistonPosition);
                calcStatus();
                //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));              
            }

            /* The coarse contol algorithm runs in one of 8 cases as below

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
            if ((uint32_t)(0) == caseStatus)
            {
                caseStatus = coarseControlCase2();
                if ((uint32_t)(0) == caseStatus)
                {
                    caseStatus = coarseControlCase3();
                    if ((uint32_t)(0) == caseStatus)
                    {
                        caseStatus = coarseControlCase4();
                        if ((uint32_t)(0) == caseStatus)
                        {
                            caseStatus = coarseControlCase5();
                            if ((uint32_t)(0) == caseStatus)
                            {
                                caseStatus = coarseControlCase6();
                                if ((uint32_t)(0) == caseStatus)
                                {
                                    caseStatus = coarseControlCase7();
                                    if ((uint32_t)(0) == caseStatus)
                                    {
                                        caseStatus = coarseControlCase8();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if ((int32_t)(0) != pidParams.stepSize)
            {
                setMotorCurrent();                                               
            }
            //pidParams.stepCount = (eControllerError_t)motor->move(pidParams.stepSize);
#ifdef ENABLE_MOTOR            
            errorStatus = (eControllerError_t)PV624->stepperMotor->move((int32_t)pidParams.stepSize, &completedCnt);      //# stop the motor
#endif
            if ((eControllerError_t)(eErrorNone) == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }

            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;          

        }
        else if (E_CONTROLLER_MODE_VENT == myMode)
        {
            /* Mode is correct, so reset coarse control error */
            controllerStatus.bit.coarseControlError = eCoarseControlErrorReset;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
            setPointG = pidParams.pressureSetPoint - (atmosphericPressure * setPointType);
                        
            coarseControlVent();
        }
        else
        {
            /* Mode is neither of measure, control or vent */
            /* This is an invalid case, hence signal error */
            controllerStatus.bit.coarseControlError = eCoarseControlErrorSet;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
        }
#ifdef CPP_ON_PC
        logPidBayesAndTestParamDataValues(eCoarseControl);
#endif
    }
}

#pragma diag_suppress=Pm031

/**
 * @brief   Coarse control vent state
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlVent(void)
{
    //ePistonRange_t pistonRange = ePistonOutOfRange;
    uint32_t status = (uint32_t)(0);

    eControllerError_t errorStatus = eErrorNone;
    int32_t completedCnt = (int32_t)0;

    //# in vent mode
    //# vent to atmosphere while reading pressure slowly to decide if vent complete

    //pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()

    //# detect position of the piston
    if (eControlVentGetFirstReading == ventReadingNum)
    {
        status = (uint32_t)1;
        pidParams.opticalSensorAdcReading = readOpticalSensorCounts(); // PID['opticalADC'] = pv624.readOpticalSensor()
        status = getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition); // PID['position'] = int(screw['readingToPosition'](PID['opticalADC']))

        controllerStatus.bit.rangeExceeded = validatePistonPosition(pidParams.pistonPosition);//# decide if in allowed range of piston position
        controllerStatus.bit.pistonCentered = isPistonCentered(pidParams.pistonPosition);
        calcStatus();
        //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
        //# assume optical position reading is accurate
        pidParams.totalStepCount = pidParams.pistonPosition; //PID['total'] = PID['position']
        previousGaugePressure = gaugePressure;
        ventReadingNum = eControlVentGetSecondReading;
    }
    else if (eControlVentGetSecondReading == ventReadingNum)
    {
        status = 1u;
        //if abs(oldPressureG - pressureG) < bayes['vardP'] * *0.5 and PID['centeringVent'] == 0:
        //# pressure is stable and screw is done centering
        //PID['vented'] = 1
        if ((fabs(previousGaugePressure - gaugePressure) < sqrt(bayesParams.uncertaintyPressureDiff)) &&
            (controllerStatus.bit.centeringVent == (uint32_t)0))
        {
            controllerStatus.bit.venting = 0u;
            controllerStatus.bit.vented = 1u;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            if (fabs(gaugePressure) > gaugeSensorUncertainty)
            {
                /*
                # TBD pressure offset is too large when vented
                # will reduce calibration accuracy
                #and may adversely affect controlled venting
                #and pumpUp / pumpDown decisions
                print('Warning: PM offset at 0 bar G:', pressureG, sensor['gaugeUncertainty'])
                */
            }
        }

        //# center piston while venting
        //# do not calculate volume as not possible while ventingand centering
        if (pidParams.pistonPosition > (screwParams.centerPositionCount + (screwParams.centerTolerance >> 1)))
        {
            //# move towards center position
            pidParams.stepSize = -1 * (motorParams.maxStepSize);
            if (controllerStatus.bit.centeringVent == (uint32_t)0)
            {
                controllerStatus.bit.centeringVent = (uint32_t)1;
                calcStatus();
                //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            }
        }
        else if (pidParams.pistonPosition < (screwParams.centerPositionCount - (screwParams.centerTolerance >> 1)))
        {
            pidParams.stepSize = motorParams.maxStepSize;
            if (controllerStatus.bit.centeringVent == (uint32_t)0)
            {
                controllerStatus.bit.centeringVent = (uint32_t)1;
                calcStatus();
                //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            }
        }
        else
        {
            pidParams.stepSize = (int32_t)(0);
            controllerStatus.bit.centeringVent = 0u;
        }
        
        if (pidParams.stepSize != (int32_t)0)
        {
            setMotorCurrent();
            //motor->writeAcclCurrent(pidParams.measuredMotorCurrent);
            //motor->writeDecelCurrent(pidParams.measuredMotorCurrent);
        }
        //pidParams.stepCount = motor->writeMoveContinuous(pidParams.stepSize);
#ifdef ENABLE_MOTOR         
        errorStatus = (eControllerError_t)(PV624->stepperMotor->move((int32_t)pidParams.stepSize, &completedCnt));      //# stop the motor
#endif        
        if (eErrorNone == (eControllerError_t)errorStatus)
        {
            pidParams.stepCount = completedCnt;
        }
        else
        {
            pidParams.stepCount = 0;
        }
        pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
        ventReadingNum = eControlVentGetFirstReading;
    }
    else
    {
        /* For misra, does nothing */
    }
    return status;
}
#pragma diag_default=Pm031
#pragma diag_default=Pm128 /* Disable MISRA C 2004 rule 10.1 */

/*
 * @brief   Returns the absolute pressure value from 2 values
 * @param   None
 * @retval  None
 */
uint32_t DController::getAbsPressure(float p1, float p2, float *absVal)
{
    uint32_t status = (uint32_t)(1);

    if (p1 > p2)
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
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlCase1()
{
    uint32_t status = (uint32_t)(0);
    float pumpTolerance = 0.0f;
    int32_t completedCnt = 0;
    eControllerError_t errorStatus = eErrorNone;
    float absValue = 0.0f;
    getAbsPressure(setPointG, gaugePressure, &absValue);
    pumpTolerance =  (absValue/ absolutePressure);

    if ((pumpTolerance < pidParams.pumpTolerance) && (ePistonCentered == controllerStatus.bit.pistonCentered))
    {
        /* If this case is executed, make the caseStatus 1 */
        status = (uint32_t)(1);
        setControlIsolate();
        pidParams.stepSize = (int32_t)(0);
        
        //errorStatus = (eControllerError_t)PV624->stepperMotor->move((int32_t)0, &completedCnt);      //# stop the motor
        if ((eControllerError_t)(eErrorNone) == errorStatus)
        {
            pidParams.stepCount = completedCnt;
        }
        else
        {
            pidParams.stepCount = 0;
        }
        pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
        controllerStatus.bit.fineControl = 1u;
        controllerState = eFineControlLoop;
        //controllerState = eCoarseControlExit;
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 2
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlCase2()
{
    uint32_t status = (uint32_t)(0);
    int32_t pistonCentreLeft = (int32_t)(0);

    eControllerError_t errorStatus = eErrorNone;
    int32_t completedCnt = (int32_t)0;
    pistonCentreLeft = screwParams.centerPositionCount - (screwParams.centerTolerance / (int32_t)(2));

    if ((setPointG > gaugePressure) && (pidParams.pistonPosition < pistonCentreLeft))
    {
        /* Make the status 1 if this case is executed */
        status = (uint32_t)(1);
        pidParams.stepSize = motorParams.maxStepSize;

        if (ePistonNotCentering == controllerStatus.bit.centering)
        {
            setControlIsolate();
            controllerStatus.bit.centering = ePistonCentering;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            bayesParams.changeInVolume = (float)(0);
            setMotorCurrent();

            /* Write motor current to secondary uC */
            /* Write accl and decel currents TODO */
            //pidParams.stepCount = motorWrite(eMove, pidParams.stepSize);
            //pidParams.stepCount = (eControllerError_t)motor->move(pidParams.stepSize);
            //errorStatus = (eControllerError_t)PV624->stepperMotor->move(pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eControllerError_t)(eErrorNone) == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
            pidParams.stepCount = 0;
            bayesParams.measuredPressure = absolutePressure;
        }
        bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
        bayesParams.changeInVolume = bayesParams.changeInVolume -
            ((float)(pidParams.stepCount) * screwParams.changeInVolumePerPulse);

        estimate();
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 3
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlCase3()
{
    uint32_t status = (uint32_t)(0);
    int32_t pistonCentreRight = (int32_t)(0);

    eControllerError_t errorStatus = eErrorNone;
    int32_t completedCnt = (int32_t)0;

    pistonCentreRight = screwParams.centerPositionCount + (screwParams.centerTolerance / (int32_t)(2));

    if ((setPointG < gaugePressure) && (pidParams.pistonPosition > pistonCentreRight))
    {
        status = (uint32_t)(1);

        pidParams.stepSize = -1 * (motorParams.maxStepSize);
        if (ePistonNotCentering == controllerStatus.bit.centering)
        {
            setControlIsolate();
            controllerStatus.bit.centering = ePistonCentering;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            bayesParams.changeInVolume = (float)(0);
            setMotorCurrent();

            /* Write motor current to secondary uC */
            /* Write accl and decel currents TODO */
            //pidParams.stepCount = (eControllerError_t)(eControllerError_t)motor->move(pidParams.stepSize);// Write(eMove, pidParams.stepSize);
            //errorStatus = (eControllerError_t)PV624->stepperMotor->move(pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eControllerError_t)(eErrorNone) == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
            pidParams.stepCount = (int32_t)0;
            bayesParams.measuredPressure = absolutePressure;
        }
        bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
        bayesParams.changeInVolume = bayesParams.changeInVolume -
            ((float)(pidParams.stepCount) * screwParams.changeInVolumePerPulse);
        estimate();
    }
    return status;
}

/**
 * @brief   Control mode CC CASE 4
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlCase4()
{
    uint32_t status = (uint32_t)(0);
    float effPressureNeg = 0.0f;
    float effPressurePos = 0.0f;
    float effPressure = 0.0f;

    int32_t pistonCentreLeft = (int32_t)(0);
    int32_t pistonCentreRight = (int32_t)(0);

    effPressureNeg = gaugePressure - gaugeSensorUncertainty;
    effPressurePos = gaugePressure + gaugeSensorUncertainty;
    
    if (((-gaugeSensorUncertainty > setPointG) && (setPointG > gaugePressure)) ||
            ((gaugeSensorUncertainty < setPointG) && (setPointG < gaugePressure) )||
            ((effPressureNeg > 0.0f) && (0.0f > setPointG)) ||
            ((effPressurePos < 0.0f) && (0.0f < setPointG)))
    {
        status = (uint32_t)(1);
        pidParams.stepSize = (int32_t)(0);

        if (eControlledVentStopped == controllerStatus.bit.controlledVent)
        {
            setControlVent();

            effPressure = gaugePressure - gaugeSensorUncertainty;

            if (effPressure > 0.0f)
            {
                controllerStatus.bit.ventDirDown = eVentDirDown;
                controllerStatus.bit.ventDirUp = eVentDirUpNone;
                calcStatus();
                //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            }
            else
            {
                controllerStatus.bit.ventDirUp = eVentDirUp;
                controllerStatus.bit.ventDirDown = eVentDirDownNone;
                calcStatus();
                //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            }

            bayesParams.measuredPressure = absolutePressure;
        }

        bayesParams.changeInPressure = bayesParams.measuredPressure - absolutePressure;

        pistonCentreLeft = screwParams.centerPositionCount - (screwParams.centerTolerance / (int32_t)(2));
        pistonCentreRight = screwParams.centerPositionCount + (screwParams.centerTolerance / (int32_t)(2));

        if (pidParams.pistonPosition > pistonCentreRight)
        {
            pidParams.stepSize = -1 * (motorParams.maxStepSize);

            if (eCenteringVentStopped == controllerStatus.bit.centeringVent)
            {
                controllerStatus.bit.centeringVent = eCenteringVentRunning;
                calcStatus();
                //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            }
        }
        else if (pidParams.pistonPosition < pistonCentreLeft)
        {
            pidParams.stepSize = motorParams.maxStepSize;

            if (eCenteringVentStopped == controllerStatus.bit.centeringVent)
            {
                controllerStatus.bit.centeringVent = eCenteringVentRunning;
                calcStatus();
                //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            }
        }
        else
        {
            pidParams.stepSize = (int32_t)(0);
            controllerStatus.bit.centeringVent = eCenteringVentStopped;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));

        }
    }
    return status;
}

/**
 * @brief   Control mode CC CASE 5
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlCase5()
{
    uint32_t status = (uint32_t)(0);

    int32_t pistonCentreRight = (int32_t)(0);

    eControllerError_t errorStatus = eErrorNone;
    int32_t completedCnt = (int32_t)0;

    pistonCentreRight = screwParams.centerPositionCount - (screwParams.centerTolerance / (int32_t)(2));

    if (pidParams.pistonPosition < pistonCentreRight)
    {
        status = (uint32_t)(1);
        pidParams.stepSize = motorParams.maxStepSize;
        if (ePistonNotCentering == controllerStatus.bit.centering)
        {
            setControlIsolate();
            controllerStatus.bit.centering = ePistonCentering;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            bayesParams.changeInVolume = 0.0f;
            setMotorCurrent();

            /* Write motor current to secondary uC */
            /* Write accl and decel currents TODO */
            //pidParams.stepCount = (eControllerError_t)(eControllerError_t)motor->move(pidParams.stepSize); // Write(eMove, pidParams.stepSize);
            //errorStatus = (eControllerError_t)PV624->stepperMotor->move((int32_t)pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eControllerError_t)(eErrorNone) == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }

            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
            pidParams.stepCount = (int32_t)0;
            bayesParams.measuredPressure = absolutePressure;
        }
        bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
        bayesParams.changeInVolume = bayesParams.changeInVolume -
            ((float)(pidParams.stepCount) * screwParams.changeInVolumePerPulse);
        estimate();
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 6
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlCase6()
{
    uint32_t status = (uint32_t)(0);
    int32_t pistonCentreLeft = (int32_t)(0);
    eControllerError_t errorStatus = eErrorNone;
    int32_t completedCnt = (int32_t)0;
    pistonCentreLeft = screwParams.centerPositionCount + (screwParams.centerTolerance / (int32_t)(2));

    if (pidParams.pistonPosition > pistonCentreLeft)
    {
        status = (uint32_t)(1);
        pidParams.stepSize = -1 * (motorParams.maxStepSize);
        if (ePistonNotCentering == controllerStatus.bit.centering)
        {
            setControlIsolate();
            controllerStatus.bit.centering = ePistonCentering;
            calcStatus();
            //PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
            bayesParams.changeInVolume = 0.0f;
            setMotorCurrent();

            /* Write motor current to secondary uC */
            /* Write accl and decel currents TODO */
            //pidParams.stepCount = (eControllerError_t)(eControllerError_t)motor->move(pidParams.stepSize);// Write(eMove, pidParams.stepSize);
            //errorStatus = (eControllerError_t)PV624->stepperMotor->move((int32_t)pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eControllerError_t)(eErrorNone) == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
            pidParams.stepCount = (int32_t)0;
            bayesParams.measuredPressure = absolutePressure;
        }
        bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
        bayesParams.changeInVolume = bayesParams.changeInVolume -
            ((float)(pidParams.stepCount) * screwParams.changeInVolumePerPulse);
        estimate();
    }

    return status;
}

/**
 * @brief   Control mode CC CASE 7
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlCase7()
{
    uint32_t status = (uint32_t)(0);

    eControllerError_t errorStatus = eErrorNone;
    int32_t completedCnt = (int32_t)0;
    /*
    pump up
    In small volumes a previous controlled vent may have overshot setpoint
    which could trigger an endless loop
    of vent and pump.  Break this loop by detecting if the the pump condition
    was preceded by a vent in the opposite direction.
    */
    if (setPointG > gaugePressure)
    {
        status = 1u;
        pidParams.stepSize = (int32_t)(0);

        if (ePumpUpNotRequired == controllerStatus.bit.pumpUp)
        {

            if ((eVentDirDown != controllerStatus.bit.ventDirDown) || 
                  ((eVentDirDownNone == controllerStatus.bit.ventDirDown) && 
                   (eVentDirUpNone == controllerStatus.bit.ventDirUp)))
            {
                /* previous controlled vent was not in the opposite direction to pump action */
                setControlUp();
            }
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
                pidParams.stepSize = (int32_t)(0);
                //pidParams.stepCount = (eControllerError_t)(eControllerError_t)motor->move((int32_t)0);      //# stop the motor
                //errorStatus = (eControllerError_t)PV624->stepperMotor->move((int32_t)0, &completedCnt);      //# stop the motor
                if ((eControllerError_t)(eErrorNone) == errorStatus)
                {
                    pidParams.stepCount = completedCnt;
                }
                else
                {
                    pidParams.stepCount = 0;
                }
                pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
                                
                //controllerState = eCoarseControlExit;
            }
        }
    }
    return status;
}

/**
 * @brief   Control mode CC CASE 8
 * @param   None
 * @retval  None
 */
uint32_t DController::coarseControlCase8()
{
    uint32_t status = (uint32_t)(0);

    eControllerError_t errorStatus = eErrorNone;
    int32_t completedCnt = (int32_t)0;

    if (setPointG < gaugePressure)
    {
        status = 1u;
        pidParams.stepSize = (int32_t)(0);

        if (ePumpDownNotRequired == controllerStatus.bit.pumpDown)
        {
            if ((eVentDirUp != controllerStatus.bit.ventDirUp) || 
                    ((eVentDirDownNone == controllerStatus.bit.ventDirDown) && 
                     (eVentDirUpNone == controllerStatus.bit.ventDirUp)))
            {
                /* previous controlled vent was not in the opposite direction to pump action */
                setControlDown();
            }
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
                pidParams.stepSize = (int32_t)(0);
                //pidParams.stepCount = (eControllerError_t)motor->move((int32_t)0);      //# stop the motor
                //errorStatus = (eControllerError_t)PV624->stepperMotor->move((int32_t)0, & completedCnt);      //# stop the motor
                if ((eControllerError_t )eErrorNone == errorStatus)
                {
                    pidParams.stepCount = completedCnt;
                }
                else
                {
                    pidParams.stepCount = 0;
                }
                pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;

                //controllerState = eCoarseControlExit;
            }
        }
    }
    return status;
}

/**
* @brief	Copy Data
* @param	void
* @retval	void
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
* @brief	Dump Data
* @param	void
* @retval	void
*/
void DController::dumpData(void)
{
#if 1
    uint8_t buff[300];
    uint32_t length = 0u;
    uint32_t totalLength = 0u;
    uint32_t ms = 0u;
    sControllerParam_t param;
    param.uiValue = 0u;
    length = 4u;
    getMilliSeconds(&ms);
    /* Write header */
    param.uiValue = 0xFFFFFFFFu;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    param.uiValue = 0xFFFFFFFFu;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);    
    
#ifdef DUMP_PID_DATA   
    /* Write PID Params */
    param.uiValue = pidParams.elapsedTime;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);  
    
    param.uiValue = ms;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.uiValue = (uint32_t)(pidParams.setPointType);    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.iValue = pidParams.stepCount;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.pressureError;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.iValue = pidParams.totalStepCount;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.controlledPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.pressureAbs;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.pressureGauge;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.pressureBaro;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.iValue = pidParams.stepSize;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.pressureCorrectionTarget;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.requestedMeasuredMotorCurrent;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.measuredMotorCurrent;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.uiValue = pidParams.opticalSensorAdcReading;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.iValue = pidParams.pistonPosition;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.motorSpeed;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.iValue = pidParams.isSetpointInControllerRange;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = pidParams.pumpTolerance;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
#endif
    
#ifdef DUMP_BAYES_DATA
    param.floatValue = bayesParams.minSysVolumeEstimateValue;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.maxSysVolumeEstimateValue;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.minEstimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.maxEstimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.measuredPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.smoothedPresure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.changeInPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.prevChangeInPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.dP2;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.estimatedVolume;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.uiValue = (uint32_t)(bayesParams.algorithmType);
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.changeInVolume;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.prevChangeInVolume;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.dV2;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.measuredVolume;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.estimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.measuredLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.estimatedKp;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.measuredKp;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.sensorUncertainity;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.uncertaintyPressureDiff;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.uncertaintyVolumeEstimate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.uncertaintyMeasuredVolume;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.uncertaintyVolumeChange;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.uncertaintyEstimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.uncertaintyMeasuredLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.maxZScore;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.lambda;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.uncerInSmoothedMeasPresErr;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.targetdP;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.smoothedPressureErr;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.smoothedSqaredPressureErr;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.gamma;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.predictionError;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.iValue = bayesParams.predictionErrType;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.maxAchievablePressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.minAchievablePressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.maxPositivePressureChangeAchievable;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.maxNegativePressureChangeAchievable;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.minPressureAdjustmentRangeFactor;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.iValue = bayesParams.nominalHomePosition;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.expectedPressureAtCenterPosition;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.uiValue = bayesParams.maxIterationsForIIRfilter;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.uiValue = bayesParams.minIterationsForIIRfilter;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.changeToEstimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.alpha;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.smoothedPressureErrForPECorrection;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.log10epsilon;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.residualLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.floatValue = bayesParams.measuredLeakRate1;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    param.uiValue = bayesParams.numberOfControlIterations;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);    
#endif
    
    param.uiValue = 0xFFFFFFFFu;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    param.uiValue = 0xFFFFFFFFu;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    
    PV624->print((uint8_t *)(buff), totalLength);
#endif
}

/**
* @brief	Pressure control loop
* @param	void
* @retval	void
*/
void DController::pressureControlLoop(pressureInfo_t* ptrPressureInfo)
{
    if (NULL != ptrPressureInfo)
    {
        /* use values from the pressure sensor and barometer */
         
        absolutePressure = ptrPressureInfo->absolutePressure;
        gaugePressure = ptrPressureInfo->gaugePressure;
        atmosphericPressure = ptrPressureInfo->atmosphericPressure;
        pressureSetPoint = ptrPressureInfo->pressureSetPoint;
        myMode = ptrPressureInfo->mode;
        setPointType = ptrPressureInfo->setPointType;
        pidParams.controlledPressure = ptrPressureInfo->pressure;
        pidParams.pressureAbs = absolutePressure;
        pidParams.pressureGauge = gaugePressure;
        pidParams.pressureBaro = atmosphericPressure;
        pidParams.pressureOld = ptrPressureInfo->oldPressure;
        pidParams.elapsedTime = ptrPressureInfo->elapsedTime;
        
#if 0        
        switch (controllerState)
        {
        case eCoarseControlLoopEntry:
            coarseControlSmEntry();
            resetBayesParameters();
            break;

        case eCoarseControlLoop:
            //# run coarse control loop until close to setpoint
            coarseControlLoop();
            fineControlLoop();
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
            fineControlLoop(); //# run fine control loop until Genii mode changes from control, or control error occurs
            break;

        default:
            break;
        }
#endif
        switch (controllerState)
        {
        case eCoarseControlLoopEntry:
            coarseControlSmEntry();
            resetBayesParameters();
            break;

        case eCoarseControlLoop:
            //# run coarse control loop until close to setpoint
            coarseControlLoop();
            //fineControlLoop();
            resetBayesParameters();
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
            fineControlLoop(); //# run fine control loop until Genii mode changes from control, or control error occurs
            break;

        default:
            break;
        }     
        
        dumpData(); 
        
        
    }
}


