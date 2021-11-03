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
* @file		DController.c
* @version	1.00.00
* @author	Makarand Deshmukh / Nageswara Pydisetty
* @date		31-08-2021
*
* @brief	Source file for pressure control algorithm
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
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
static const float EPSILON = (float)1E-10;  //arbitrary 'epsilon' value
static const float gaugeSensorUncertainty = (float)20.0; //uncertainty gage sensor pressure
static const float piValue = (float)3.14159;

/* User Code --------------------------------------------------------------------------------------------------------*/

/**
* @brief	Controller class constructor
* @param	void
* @retval	void
*/
DController::DController()
{
    controllerState = eCoarseControlLoopEntry;
    msTimer = 0u;
    pressureSetPoint = 0.0f;
    setPointG = 0.0f;
    absolutePressure = 0.0f;
    gaugePressure = 0.0f;
    atmosphericPressure = 0.0f;
    controllerStatus.bytes = 0u;
    ctrlStatusDpi.bytes = 0u;
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
    // elapsed time for log file(s)
    pidParams.elapsedTime = 0u;                             
    // pressure setpoint(mbar)
    pidParams.pressureSetPoint = (float)0.0;                
    // setpoint type from GENII(0 = gauge, 1 = abs, 2 = baro)
    pidParams.setPointType = (eSetPointType_t)0;            
    // number of motor pulses delivered since last stepSize request
    pidParams.stepCount = (int32_t)0;                       
    // pressure error for PID(mbar), +ve == below pressure setpoint
    pidParams.pressureError = (float)0.0;                   
    // total step count since start
    pidParams.totalStepCount = (int32_t)0;                  
    // controlled pressure(mbar gage or mbar abs)
    pidParams.controlledPressure = (float)0.0;              
    // requested number of steps to turn motor
    pidParams.stepSize = (int32_t)0;                        
    // leak - adjusted pressure correction target(mbar)
    pidParams.pressureCorrectionTarget = (float)0.0;        
    // requested measured motor current(mA)
    pidParams.requestedMeasuredMotorCurrent = (float)0.0;   
    // measured motor current(mA)
    pidParams.measuredMotorCurrent = (float)0.0;            
    // optical sensor ADC reading(0 to 4096)
    pidParams.opticalSensorAdcReading = (uint32_t)0;        
    // optical piston position(steps), 0 == fully retracted / max volume
    pidParams.pistonPosition = (int32_t)0;                  
    // motor speed from motor controller(pps)
    pidParams.motorSpeed = (float)0;                        
    // setpoint target in controller range, based on bayes range estimate
    pidParams.isSetpointInControllerRange = (int32_t)(1);   
    // max relative distance from setpoint before pumping is required, e.g. 0.1 == 10 % of setpoint
    pidParams.pumpTolerance = (float)0.005;                 
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
}

/**
* @brief	Init motor controller parameters
* @param	void
* @retval	void
*/
void DController::initMotorParams(void)
{
    // full step angle of motor(deg)
    motorParams.motorStepSize = (float)1.8;         
    // number of microsteps(2 == halfstep)
    motorParams.microStepSize = (float)4.0;         
    // motor controller s - curve alpha acceleration value, from excel calculator, fast to full speed
    motorParams.accelerationAlpha = (float)0.98;    
    // motor controller s - curve beta acceleration value, from excel calculator, fast to full speed
    motorParams.accelerationBeta = (float)2.86;     
    // motor controller s - curve alpha decelleration value, from excel calculator
    motorParams.decellerationAlpha = (float)1.02;   
    // motor controller s - curve beta decelleration value, from excel calculator
    motorParams.decellerationBeta = (float)0.0;     
    // maximum motor current(A)
    motorParams.maxMotorCurrent = (float)2.0;       
    // minimum motor current(A), amount required to overcome friction around 0 bar g
    motorParams.minMotorCurrrent= (float)0.7;       
    // hold current(A), used when when not moving, minimum = 0.2 A to avoid motor oscillations when controlling
    motorParams.holdCurrent = (float)0.2;           
    // maximum number of steps that can be taken in one control iteration, used in coarse control loop
    motorParams.maxStepSize = (int32_t)3000;        
}

#pragma diag_suppress=Pm137 /* Disable MISRA C 2004 rule 18.4 */
/**
* @brief	Init screw parameters
* @param	void
* @retval	void
*/
void DController::initScrewParams(void)
{
    // gear ratio of motor
    screwParams.gearRatio = (float)1.0;             
    // piston diameter(mm), Helix0.6 press
    screwParams.pistonDiameter = (float)12.2;       
    // lead screw pitch(mm / rotation)
    screwParams.leadScrewPitch = (float)0.6;        
    // travel piston(mm)
    screwParams.leadScrewLength = (float)38.0;      
    // piston area(mm ^ 2)
    screwParams.pistonArea = piValue * (screwParams.pistonDiameter / 2.0f) * (screwParams.pistonDiameter / 2.0f);  
    // volume change per control pulse(mL / pulse)
    screwParams.changeInVolumePerPulse = (motorParams.motorStepSize * screwParams.leadScrewPitch * 
                                            screwParams.pistonArea * (float)1e-3) / (motorParams.microStepSize * 
                                            screwParams.gearRatio * (float)360.0); 
    // maximum system pressure(mbar) for accel / decel current scaling
    screwParams.maxPressure = (float)21000.0; 
    // pulse count for full compression of piston, with 2 % safety factor to avoid collisions
    screwParams.maxPosition = int(0.98f * screwParams.leadScrewLength / screwParams.leadScrewPitch * (float)360.0 / 
                                motorParams.motorStepSize * motorParams.microStepSize);
    // pulse count for full retraction of piston, with 2 % safety factor to avoid collisions
    screwParams.minPosition = int((float)(screwParams.maxPosition) / (float)0.98f * (float)0.02);
    // use measured values instead of calculated
    screwParams.maxPosition = (int32_t)48800;
    screwParams.minPosition = (int32_t)1600;
    // step count at nominal center position(home)
    screwParams.centerPositionCount = (int32_t)25000;
    // tolerance for finding center position(counts) 
    screwParams.centerTolerance = (int32_t)2000;
    // shunt resistance for current sensor, motor testing(ohm) 
    screwParams.shuntResistance = (float)0.05;  
    // V / V gain of current sensor, with 20k Rload sensor and 50k ADC input impedance
    screwParams.shuntGain = 0.2f * (20.0f * 50.0f / (20.0f + 50.0f));  
    // conversion factor from shunt ADC counts to current(mA)
    screwParams.readingToCurrent = 3.3f / (4096.0f * screwParams.shuntGain * screwParams.shuntResistance) * 1000.0f;  
    // maximum leak rate bound(mbar / interation) at 20 bar and 10 mA, PRD spec is < 0.2
    screwParams.maxLeakRate = (float)0.3f;  
    // maximum allowed pressure in screw press(PM independent) (mbar), for max leak rate adjustments
    screwParams.maxAllowedPressure = (float)21000.0;
    // nominal total volume(mL), for max leak rate adjustments  
    screwParams.nominalTotalVolume = (float)10.0;  

    //screw['orificeK'] = 3.5  # vent orifice flow constant, empirically determined (mL/s)
    screwParams.orificeK = 3.5f;
    //screw['ventSR'] = 0.075  # nominal control loop iteration time during controlled vent operation (s)
    screwParams.ventSr = 0.075f;
    //screw['ventUncertainty'] = 0.4  # uncertainty in volume estimate during controlled vent, 0.4 = +/- 40%
    screwParams.ventUncertainty = 0.4f;
    //screw['ventDelay'] = 2  # number of control iterations to wait after switching vent valves before estimating volume
    screwParams.ventDelay = 2u;
}
#pragma diag_default=Pm137 /* Disable MISRA C 2004 rule 18.4 */

/**
* @brief	Init bayes parameters
* @param	void
* @retval	void
*/
void DController::initBayesParams(void)
{
    // minimum system volume estimate value(mL)
    bayesParams.minSysVolumeEstimate = (float)5.0; 
    // maximum system volume estimate value(mL)
    bayesParams.maxSysVolumeEstimate = (float)100.0; 
    // minimum estimated leak rate(mbar)
    bayesParams.minEstimatedLeakRate = (float)0.0; 
    // maximum absolute value of estimated leak rate(+/ -mbar / iteration)
    bayesParams.maxEstimatedLeakRate  = (float)0.2;
    // measured pressure(mbar) 
    bayesParams.measuredPressure = (float)1000.0;
    // smoothed pressure, depends on controlled pressure stability not sensor uncertainty, (mbar) 
    bayesParams.smoothedPresure = (float)1000.0;
    // measured change in pressure from previous iteration(mbar) 
    bayesParams.changeInPressure = (float)0.0; 
    // previous dP value(mbar)
    bayesParams.prevChangeInPressure = (float)0.0; 

    bayesParams.dP2 = 0.0f;
    // etimate of volume(mL), set to minV to give largest range estimate on startup
    bayesParams.estimatedVolume = bayesParams.maxSysVolumeEstimate; 
    // algorithm used to calculate V
    bayesParams.algorithmType = eMethodNone; 
    // volume change from previous stepSize command(mL)
    bayesParams.changeInVolume = (float)0.0; 
    // previous dV value(mL), used in regression method
    bayesParams.prevChangeInVolume = (float)0.0; 

    bayesParams.dV2 = 0.0f;
    // volume estimate using Bayes regression(mL)
    bayesParams.measuredVolume = bayesParams.maxSysVolumeEstimate;
    // estimate in leak rate(mbar / iteration), from regression method 
    bayesParams.estimatedLeakRate = bayesParams.minEstimatedLeakRate;
    //measured leak rate using Bayes regression(mbar / iteration) 
    bayesParams.measuredLeakRate = bayesParams.minEstimatedLeakRate; 
    // estim. kP(steps/mbar) that will reduce pressure error to zero in one iteration, large for fast initial response
    bayesParams.estimatedKp = (float)500.0; 
    /*
    bayes['measkP'] = bayes['kP']  
    measured optimal kP(steps / mbar) that will reduce pressure error to zero in one iteration
    state value variances
    bayesParams.sensorUncertainity = (10e-6 * sensorFsValue) * (10e-6 * sensorFsValue)
    uncertainty in pressure measurement(mbar), sigma ~= 10 PPM of FS pressure @ 13 Hz read rate
    */
    bayesParams.sensorUncertainity = (10e-6f * fsValue) * (10e-6f * fsValue);
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
    bayesParams.maxZScore = (float)2.0; 
    // maximum variance spread between measuredand estimated values before estimated variance is increased
    bayesParams.lambda = (float)0.1; // forgetting factor for smoothE
    bayesParams.uncerInSmoothedMeasPresErr  = (float)0.0; //smoothed measured pressure error variance(mbar * *2)
    bayesParams.targetdP = (float)0.0; // target correction from previous control iteration(mbar)
    bayesParams.smoothedPressureErr  = (float)0.0; // smoothed pressure error(mbar)
    bayesParams.smoothedSquaredPressureErr = (float)0.0; // smoothed squared pressure error(mbar * *2)
    bayesParams.uncerInSmoothedMeasPresErr  = (float)0.0; // smoothed measured pressure error variance(mbar * *2)
    bayesParams.gamma = (float)0.98; 
    /*
    volume scaling factor for nudging estimated volume with predictionError, (0.90, 0.98), 
    larger = faster response but noisier estimate
    */
    // prediction error from previous control iteration(mbar)
    bayesParams.predictionError  = (float)0.0; 
    // prediction error type(+/ -1), for volume estimate adjustment near setpoint
    bayesParams.predictionErrType = (int32_t)0; 
#if 0
    // maximum achievable pressure, from bayes estimates(mbar)
    bayesParams.maxAchievablePressure  = (float)0.0; 
    // minimum achievable pressure, from bayes estimates(mbar)
    bayesParams.minAchievablePressure  = (float)0.0; 
    // maximum positive pressure change achievable, from bayes estimates(mbar)
    bayesParams.maxPositivePressureChangeAchievable  = (float)1e6; 
    // maximum negative pressure change achievable, from bayes estimates(mbar)
    bayesParams.maxNegativePressureChangeAchievable = (float)-1e6; 
    /*
    minimum pressure adjustment range factor when at nominalHome, 
    e.g. 0.1 = minimum + / -10 % adjustment range of P at nominalHome piston location
    */
    bayesParams.minPressureAdjustmentRangeFactor = pidParams.pumpTolerance; 
    // nomimal "home" position to achieve + / -10 % adjustability
    bayesParams.nominalHomePosition = screwParams.centerPositionCount; 
    // expected pressure at center piston position(mbar)
    bayesParams.expectedPressureAtCenterPosition = (float)0.0;
    #endif
    // maximum iterations for leak rate integration filter in PE correction method 
    //bayesParams.maxIterationsForIIRfilter  = (uint32_t)100;
    bayesParams.maxIterationsForIIRfilter  = (uint32_t)300; // Updated from latest python code
    // minimum iterations for leak rate integration filter in PE correction method 
    bayesParams.minIterationsForIIRfilter  = (uint32_t)10;
    // change to estimated leak rate for PE correction method(mbar / iteration) 
    bayesParams.changeToEstimatedLeakRate = (float)0.0; 
    // low - pass IIR filter memory factor for PE correction method(0.1 to 0.98)
    bayesParams.alpha = (float)0.1; 
    // smoothed pressure error for PE correction method(mbar)
    bayesParams.smoothedPressureErrForPECorrection = (float)0; 
    // acceptable residual fractional error in PE method leak rate estimate(-2 = +/ -1 %, -1 = 10 %, -0.7 = 20 %)
    bayesParams.log10epsilon = (float)-0.7; 
    bayesParams.residualLeakRate = 0.0f;
    bayesParams.measuredLeakRate1 = 0.0f;
    bayesParams.numberOfControlIterations = bayesParams.minIterationsForIIRfilter;

    bayesParams.ventIterations = 0u;
    bayesParams.ventInitialPressure = 0.0f;
    bayesParams.ventFinalPressure = 0.0f;
}

/**
* @brief	Init Test parameters
* @param	void
* @retval	void
*/
void DController::initTestParams(void)
{
    // simulated leak rate(mbar / iteration) at 10mL and 20 bar
    testParams.maxFakeLeakRate = screwParams.maxLeakRate * 0.5f;
    testParams.maxFakeLeakRate = (float)0.0f;
    // simulated leak rate, adjusted by current pressure and volume(mbar / iteration)
    testParams.fakeLeakRate = testParams.maxFakeLeakRate;
    // calibrate optical sensor if True
    testParams.isOpticalSensorCalibrationRequired = true;
    testParams.fakeLeak = (float)0.0f;  //# simulated cumulative leak effect(mbar)
    testParams.volumeForLeakRateAdjustment = (float)10.0f;  // fixed volume value used for leak rate adjustment(mL)

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
    bayesParams.smoothedSquaredPressureErr = 0.0f;
    bayesParams.uncerInSmoothedMeasPresErr = 0.0f;
    bayesParams.smoothedPressureErrForPECorrection = 0.0f;
}

/**
* @brief	Returns the sign of a float
* @param	void
* @retval	float
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
    // isolate pump for measure
#ifdef ENABLE_VALVES    
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
    PV624->valve3->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
#endif
    // With new status variables
    pidParams.control = 0u;
    pidParams.measure = 1u;
    pidParams.venting = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
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
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
    PV624->valve3->valveTest(E_VALVE_FUNCTION_REVERSE); // isolate pump outlet
#endif
    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 1u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
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
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_REVERSE); // isolate pump outlet
    PV624->valve3->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
#endif 
    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 1u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
}

/**
* @brief	Set controller state and valves when pump isolation is required
* @param	void
* @retval	void
*/
void DController::setControlIsolate(void)
{
    // isolate pump for screw control
#ifdef ENABLE_VALVES
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
    PV624->valve3->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
#endif
    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
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
    PV624->valve1->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump inlet
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump outlet
    PV624->valve3->valveTest(E_VALVE_FUNCTION_FORWARD); //isolate pump outlet
#endif
    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 1u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 1u;
    pidParams.controlledVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
}

/**
* @brief	Run controller in vent mode
* @param	void
* @retval	void
*/
void DController::setVent(void)
{
    // isolate pumpand vent
#ifdef ENABLE_VALVES    
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
    PV624->valve3->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
    PV624->valve1->valveTest(E_VALVE_FUNCTION_REVERSE); // isolate pump inlet 
#endif
    // With new status variables
    pidParams.control = 0u;
    pidParams.measure = 0u;
    pidParams.venting = 1u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 0u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 0u;
    pidParams.centeringVent = 0u;
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;
}

/**
* @brief	Run controller in when   controlled vent mode
* @param	void
* @retval	void
*/
void DController::setControlVent(void)
{
#ifdef ENABLE_VALVES
    // isolate pump for controlled vent to setpoint    
    PV624->valve2->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
    PV624->valve3->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
    PV624->valve1->valveTest(E_VALVE_FUNCTION_REVERSE); // isolate pump inlet    
#endif
    // With new status variables
    pidParams.control = 1u;
    pidParams.measure = 0u;
    pidParams.venting = 0u;
    pidParams.vented = 0u;
    pidParams.pumpUp = 1u;
    pidParams.pumpDown = 0u;
    pidParams.centering = 0u;
    pidParams.controlledVent = 1u;
    pidParams.centeringVent = 0u;   
    pidParams.ventDirUp = 0u;
    pidParams.ventDirDown = 0u;  
}

#pragma diag_suppress=Pm128 /* Disable MISRA C 2004 rule 10.1 */
#pragma diag_suppress=Pm136 /* Disable MISRA C 2004 rule 10.4 */
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
    Estimate volumeand leak rate using Bayesian conditional probability to merge new measurement
    data with previous parameter estimates(aka extended Kalman filter).
    Added empirical predictionError adjustment to volumeand leak estimates for smaller / slower corrections where gas
    law estimation methods are noisy.
    Jun 10 2021
    updated to use with fineControl.py and coarseControl.py modules
    improved comment clarity and code flow

    defaults when measV cannot be calculated
    */
    float residualL = 0.0f;
    float tempPressure = 0.0f;
    float tempVariable = 0.0f;
    float logValue = 0.0f;

    bayesParams.measuredVolume = 0.0f;
    bayesParams.uncertaintyMeasuredVolume = bayesParams.maxSysVolumeEstimate;
    bayesParams.algorithmType = eMethodNone;
    
    /*
    if PID['controlledVent'] == 1 and bayes['ventIterations'] > screw['ventDelay'] and \
    bayes['initialP'] / bayes['finalP'] > 1:
    */
    tempPressure = bayesParams.ventInitialPressure / bayesParams.ventFinalPressure;
    if((1u == pidParams.controlledVent) 
            && (bayesParams.ventIterations > screwParams.ventDelay) 
            && (tempPressure > 1.0f))
    {
        // calculate volume from vent rate during controlled vent and catch invalid pressure values
        bayesParams.algorithmType = (eAlgorithmType_t)(4);
        tempVariable = (float)(bayesParams.ventIterations - screwParams.ventDelay);
        tempVariable = tempVariable * screwParams.ventSr;
        tempVariable = tempVariable * screwParams.orificeK;
        logValue = log(tempPressure);
        tempVariable = tempVariable / logValue;
        bayesParams.measuredVolume = tempVariable;
        bayesParams.measuredVolume = max(min(bayesParams.measuredVolume, bayesParams.maxSysVolumeEstimate), 
                                                    bayesParams.minSysVolumeEstimate);
        tempVariable = screwParams.ventUncertainty * bayesParams.measuredVolume;                                
        tempVariable = tempVariable * tempVariable;
        bayesParams.uncertaintyVolumeEstimate = tempVariable;
    }

    if (1u == pidParams.fineControl)
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
        float signTargetDp = 0.0f;
        float signPredictionError = 0.0f;

        signTargetDp = getSign(bayesParams.targetdP);
        signPredictionError = getSign(bayesParams.predictionError);
        bayesParams.predictionErrType = (int32_t)(signTargetDp * signPredictionError);
        // low pass filtered pressure error(mbar)
        bayesParams.smoothedPressureErr = pidParams.pressureError * bayesParams.lambda + 
                                              bayesParams.smoothedPressureErr * (1.0f - bayesParams.lambda);
        // low pass filtered squared error(mbar * *2)
        bayesParams.smoothedSquaredPressureErr = (pidParams.pressureError * pidParams.pressureError) * 
                                              bayesParams.lambda + bayesParams.smoothedSquaredPressureErr * 
                                              (1.0f - bayesParams.lambda);
        /*
        dynamic estimate of pressure error variance
        see https ://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        Note : varE estimate not valid when smoothE2 >> varE
        bayes['varE'] = bayes['smoothE2'] - bayes['smoothE'] * *2
        */
        bayesParams.uncerInSmoothedMeasPresErr = bayesParams.smoothedSquaredPressureErr - 
                                                  (bayesParams.smoothedPressureErr * bayesParams.smoothedPressureErr);

        /*
        decide if pressure is stable, based on smoothed variance of pressure readings
        if bayes['varE'] * *0.5 < 5 * bayes['varP'] * *0.5 and bayes['smoothE'] < 5 * bayes['varP'] * *0.5:
        */
        float sqrtUncertaintySmoothedPresErr = 0.0f;
        float sqrtSensorUncertainty = 0.0f;
        float tempSensorUncertainty = 0.0f;
        
        sqrtUncertaintySmoothedPresErr = sqrt(bayesParams.uncerInSmoothedMeasPresErr);
        sqrtSensorUncertainty = sqrt(bayesParams.sensorUncertainity);
        tempSensorUncertainty = 5.0f * sqrtSensorUncertainty;
        
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
        float dP2, dV2;
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
            bayesParams.measuredVolume = (float)(fmax((-1.0f) * bayesParams.measuredPressure * 
                                            (bayesParams.changeInVolume / bayesParams.changeInPressure), 0.0));
            // uncertainty in measured volume(mL)
            // bayes['varMeasV'] = bayes['varP'] * (bayes['dV'] / bayes['dP']) * *2 + \
            // bayes['vardP'] * (bayes['P'] * bayes['dV'] / bayes['dP'] * *2) * *2 + \
            // bayes['vardV'] * (bayes['P'] / bayes['dP']) * *2
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
        else if (1u == pidParams.fineControl)
        {
            // use Prediction Error method to slowly adjust estimates of volume and leak rate
            // during fine control loop when close to setpoint
            bayesParams.algorithmType = ePredictionErrorMethod; //bayes['algoV'] = 3

            // measured residual leak rate from steady - state pressure error(mbar / iteration)
            float measL = 0.0f;
            //residualL = ((-1.0f) * bayesParams.smoothedPressureErrForPECorrection) / bayesParams.estimatedKp;
            // bayes['residualL'] = -bayes['smoothE_PE'] / bayes['n']  # fixed error in calc
            residualL = ((-1.0f) * bayesParams.smoothedPressureErrForPECorrection) / 
                                        bayesParams.numberOfControlIterations;
            bayesParams.residualLeakRate = residualL;

            // estimate of true leak rate magnitude(mbar / iteration)
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
                    //bayes['measV'] = bayes['V'] * bayes['gamma']
                    bayesParams.measuredVolume = bayesParams.estimatedVolume * bayesParams.gamma;
                    //bayes['varMeasV'] = bayes['varV']
                    bayesParams.uncertaintyMeasuredVolume = bayesParams.uncertaintyVolumeEstimate;
                    //bayes['leak'] = bayes['leak'] / bayes['gamma']
                    bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate / bayesParams.gamma; 
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
                    //bayes['measV'] = bayes['V'] / bayes['gamma']
                    bayesParams.measuredVolume = bayesParams.estimatedVolume / bayesParams.gamma; 
                    //bayes['varMeasV'] = bayes['varV']
                    bayesParams.uncertaintyMeasuredVolume = bayesParams.uncertaintyVolumeEstimate; 
                    //bayes['leak'] = bayes['leak'] * bayes['gamma']
                    bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate * bayesParams.gamma; 
                }
            }
        }
        else
        {
            /* Required for MISRA */
        }
        
        // combine prior volume estimate with latest measured volume, if available
        if (bayesParams.measuredVolume != 0.0f)
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
            float du = 0.0f;
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
        if (ePredictionErrorMethod == bayesParams.algorithmType)
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
            float minError = 0.0f;
            float maxError = 0.0f;
            uint32_t numOfIterations = 0u;
            // minError = bayes['vardP'] * *0.5 / bayes['maxN']
            minError = sqrt(bayesParams.uncertaintyPressureDiff) / ((float)bayesParams.maxIterationsForIIRfilter); 

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
                                (float)(bayesParams.minIterationsForIIRfilter))); 
            bayesParams.numberOfControlIterations = numOfIterations;
            /*
            Average E over more iterations when leak rate estimate is small and averaged pressure error is small
            so that the expected value of the pressure error from a leak
            is at least as big as pressure measurement noise.
            IIR filter memory factor to achieve epsilon * initial condition residual after n iterations
            bayes['alpha'] = 10 * *(bayes['log10epsilon'] / n)
            */
            bayesParams.alpha = (float)(pow(10, (bayesParams.log10epsilon / (float)(numOfIterations))));
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
            bayesParams.changeToEstimatedLeakRate = (residualL / (float)numOfIterations);

            // Special cases:
            if (residualL * bayesParams.estimatedLeakRate < 0.0f)
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
                                                        (float)(numOfIterations));
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
                                                                (float)(numOfIterations); 

                /*
                increase leak uncertainty if measLeak is not within maxZScore standard deviations of leak
                increases convergence to true value in the event of a step change in leak rate
                du = fabs(bayes['leak'] - bayes['measLeak'])
                
                bayes['varLeak'] = max(bayes['varLeak'], du / bayes['maxZScore'] - bayes['varMeasLeak'])
                */
                float du = fabs(bayesParams.estimatedLeakRate - bayesParams.measuredLeakRate);
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

/*
 * @brief   Read the ADC counts from the optical Sensor
 * @param   None
 * @retval  None
 */
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
    uint32_t timeStatus = 0u;
    uint32_t epochTime = 0u;

    timeStatus = getEpochTime(&epochTime);
    if(true == timeStatus)
    {
        coarseControlLogParams.startTime = epochTime;
        pidParams.elapsedTime = epochTime;
    }
    
    if (1u == pidParams.fineControl)
    {
        /*
        read pressure with highest precision
        pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
        PID['mode'] = mode
        
        if PID['mode'] != 1 or PID['excessLeak'] == 1 or PID['excessVolume'] == 1 or \
        PID['overPressure'] == 1 or PID['rangeExceeded'] == 1:
        */
        if((myMode == E_CONTROLLER_MODE_CONTROL) &&
              (pidParams.excessLeak != 1u) &&
              (pidParams.excessVolume != 1u)&&
              (pidParams.overPressure != 1u)&&
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
            float fabsPressCorrTarget = fabs(pidParams.pressureCorrectionTarget);
            float sqrtUncertaintyPressDiff = sqrt(bayesParams.uncertaintyPressureDiff);
            float fabsPressureError = fabs(pidParams.pressureError);
           
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
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;  
#endif
            // change in volume(mL)
            bayesParams.changeInVolume = -screwParams.changeInVolumePerPulse * (float)(pidParams.stepCount);
            /*
            read the optical sensor piston position
            */
            pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
            getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition);
            pidParams.rangeExceeded = validatePistonPosition(pidParams.pistonPosition);
            if(1u != pidParams.rangeExceeded)
            {              
                /* Check if piston is centered */
                pidParams.pistonCentered = isPistonCentered(pidParams.pistonPosition);          
            }
            float currentADC = 0.0f;
            float motorSpeed = 0.0f;            
            PV624->stepperMotor->readSpeedAndCurrent((uint32_t *)(&motorSpeed), &currentADC);            
            pidParams.motorSpeed = motorSpeed;
            pidParams.measuredMotorCurrent = currentADC * screwParams.readingToCurrent;
            // update system parameter estimates from latest measurement data
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
            controllerState = eFineControlLoopEntry;   
        }
        else
        {
            /*
            Genii has changed operating mode to vent or measure
            or a control error has occurred
            abort fine control and return to coarse control loop
            */
            pidParams.fineControl = 0u;
            pidParams.stable = 0u;
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
 * @brief   Runs once before fine control loop is entered
 * @param   None
 * @retval  None
 */
void DController::fineControlSmEntry(void)
{
    controllerState = eFineControlLoop;    
}

/*
 * @brief   Calculates the status as required by the DPI
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

    controllerStatus.bytes = tempStatus;

    PV624->setControllerStatusPm((uint32_t)(controllerStatus.bytes));
    PV624->setControllerStatus((uint32_t)(controllerStatus.bytes));
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
    pidParams.ccError = eCoarseControlErrorReset;
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
    /*
    increase motor current with gauge pressure
    saves power for lower pressure setPoints

    Will not be used for L6472 as we are using fixed current config
    TODO
    */
    motorCurrRange = motorParams.maxMotorCurrent - motorParams.minMotorCurrrent;
    pidParams.requestedMeasuredMotorCurrent = motorParams.minMotorCurrrent +
        ((motorCurrRange * absolutePressure) / screwParams.maxPressure);
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
    sprintf_s(coarseControlLogParams.fileName,80u, "% d-%02d-%02d%02d:%02d:%02d", 
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
    
    PV624->getPosFullscale(&sensorFsValue);
    PV624->getPM620Type(&sensorType);
    // scale default measurement uncertainties to PM FS value
    if((sensorType & (uint32_t)(PM_ISTERPS)) == 1u)
    {
        /* scale 4 times for terps */
        uncertaintyScaling = 16.0f * (sensorFsValue / fsValue) * (sensorFsValue / fsValue);
    }
    else
    {
        uncertaintyScaling = 1.0f * (sensorFsValue / fsValue) * (sensorFsValue / fsValue);
    }
    // PM full scale pressure(mbar)
    fsValue = sensorFsValue;  
    /*
    bayes['varP']  
    uncertainty in pressure measurement(mbar)
    */
    bayesParams.sensorUncertainity = uncertaintyScaling * bayesParams.sensorUncertainity; 
    // uncertainty in measured pressure changes(mbar)
    bayesParams.uncertaintyPressureDiff = uncertaintyScaling * bayesParams.uncertaintyPressureDiff;  

    pidParams.fineControl = 0u;  //# disable fine pressure control
    // detect position of the piston
    pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
    status = getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition); 
    pidParams.pistonCentered = isPistonCentered(pidParams.pistonPosition);  
    setVent();
    // assume optical position reading is accurate
    pidParams.totalStepCount = pidParams.pistonPosition;
    calcStatus();
    HAL_TIM_Base_Start(&htim2);
    controllerState = eCoarseControlLoop;
}

/**
 * @brief   Runs once after exiting from coarse control
 * @param   None
 * @retval  None
 */
void DController::coarseControlSmExit(void)
{
    // coarse adjustment complete, last iteration of coarse control loop
    pidParams.fineControl = (uint32_t)1;  //# exiting coarse control after this iteration
    calcStatus();
    pidParams.stepSize = (int32_t)(0);
    // read pressure and position once more before moving to fine control
    pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
    getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition); 
    pidParams.rangeExceeded = ePistonInRange; //# reset rangeExceeded flag set by fineControl()
    calcStatus();
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
    
    timeStatus = getEpochTime(&epochTime);
    if(true == timeStatus)
    {
        coarseControlLogParams.startTime = epochTime;
        pidParams.elapsedTime = epochTime;
    }
    // Run coarse control only if fine control is not enabled
    if (0u == pidParams.fineControl) 
    {
        if ((E_CONTROLLER_MODE_MEASURE == myMode) && (1u == pidParams.control))
        {
            /* 
            Mode set by genii is measure but PID is in control mode 
            Change mode to measure 
            */
            setMeasure();
        }
        else if ((E_CONTROLLER_MODE_MEASURE == myMode) && (1u == pidParams.venting))
        {
            /* 
            Mode set by genii is measure but PID is venting
            Change mode to measure 
            */
            setMeasure();
        }
        else if ((E_CONTROLLER_MODE_CONTROL == myMode) && (1u == pidParams.measure))
        {
            /* 
            Mode set by genii is control but PID is in measure
            Change mode to measure 
            */
            setControlIsolate();
        }
        else if ((E_CONTROLLER_MODE_CONTROL == myMode) && (1u == pidParams.venting))
        {
            /* 
            Mode set by genii is control but PID is in measure
            Change mode to measure 
            */
            setControlIsolate();
        }
        else if ((E_CONTROLLER_MODE_VENT == myMode) && (1u == pidParams.measure))
        {
            /* 
            Mode set by genii is control but PID is in measure
            Change mode to measure 
            */
            ventReadingNum = eControlVentGetFirstReading;
            setVent();
        }
        else if ((E_CONTROLLER_MODE_VENT == myMode) && (1u == pidParams.control))
        {
            /* 
            Mode set by genii is control but PID is in measure
            Change mode to measure 
            */
            ventReadingNum = eControlVentGetFirstReading;
            setVent();
        }      
        else if (E_CONTROLLER_MODE_MEASURE == myMode)
        {
            // Mode is correct, so reset coarse control error
            pidParams.ccError = eCoarseControlErrorReset;          
            // [pressureG, pressure, atmPressure][spType] + testParams.fakeLeak;
            pidParams.controlledPressure = pressureAsPerSetPointType(); 
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.setPointType = setPointType;
            coarseControlMeasure();
        }
        else if (E_CONTROLLER_MODE_CONTROL == myMode)
        {
            // Mode is correct, so reset coarse control error
            pidParams.ccError = eCoarseControlErrorReset;

            // Read gauge pressure also
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
            setPointG = pidParams.pressureSetPoint - (atmosphericPressure * setPointType);

            // Check screw position
            getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition); 
            // Check if piston is within range
            pidParams.rangeExceeded = validatePistonPosition(pidParams.pistonPosition);
            if(1u != pidParams.rangeExceeded)
            {              
                /* Check if piston is centered */
                pidParams.pistonCentered = isPistonCentered(pidParams.pistonPosition);         
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
#ifdef ENABLE_MOTOR_CC      
            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount); 
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;  
#endif
        }
        else if (E_CONTROLLER_MODE_VENT == myMode)
        {
            // Mode is correct, so reset coarse control error
            pidParams.ccError = 0u;
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.setPointType = setPointType;
            pidParams.mode = myMode;
            pidParams.controlledPressure = pressureAsPerSetPointType();                                  
            coarseControlVent();
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
    // ePistonRange_t pistonRange = ePistonOutOfRange;
    uint32_t status = (uint32_t)(0);
    /*
    in vent mode
    vent to atmosphere while reading pressure slowly to decide if vent complete
    pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
    */
    // detect position of the piston
    if (eControlVentGetFirstReading == ventReadingNum)
    {
        status = 1u;
        pidParams.pressureOld = gaugePressure;
        previousGaugePressure = gaugePressure;
        ventReadingNum = eControlVentGetSecondReading;
    }
    else if (eControlVentGetSecondReading == ventReadingNum)
    {
        status = 1u;
        /*
        if abs(oldPressureG - pressureG) < bayes['vardP'] * *0.5 and PID['centeringVent'] == 0:
        pressure is stable and screw is done centering
        PID['vented'] = 1
        */                   
        pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
        getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition);

        if((screwParams.minPosition < pidParams.pistonPosition) && 
                    (pidParams.pistonPosition < screwParams.maxPosition))
        {
            pidParams.rangeExceeded = 0u;
        }
        else
        {
            pidParams.rangeExceeded = 1u;
        }
        pidParams.pistonCentered = isPistonCentered(pidParams.pistonPosition);
        pidParams.totalStepCount = pidParams.pistonPosition;

        if ((fabs(previousGaugePressure - gaugePressure) < sqrt(bayesParams.uncertaintyPressureDiff)) &&
                        (0u == pidParams.centeringVent))
        {
            pidParams.vented = 1u;           
            if (fabs(gaugePressure) > gaugeSensorUncertainty)
            {
                /*
                TBD pressure offset is too large when vented
                will reduce calibration accuracy
                and may adversely affect controlled venting
                and pumpUp / pumpDown decisions
                print('Warning: PM offset at 0 bar G:', pressureG, sensor['gaugeUncertainty'])
                
                What do we need to do here, wait for input from AM, TODO
                */
            }
        }

        /*
        center piston while venting
        do not calculate volume as not possible while ventingand centering
        */
        if (pidParams.pistonPosition > (screwParams.centerPositionCount + (screwParams.centerTolerance >> 1)))
        {
            /*
            move towards center position
            pidParams.stepSize = -1 * (motorParams.maxStepSize);
            pidParams.stepSize = motorParams.maxStepSize;
            This is changed for the purpose of the motor direction issue
            revert when root cause is found TODO 
            */
            pidParams.stepSize = -1 * (motorParams.maxStepSize);                   
            if (0u == pidParams.centeringVent)
            {
                pidParams.centeringVent = 1u;
            }
        }
        else if (pidParams.pistonPosition < (screwParams.centerPositionCount - (screwParams.centerTolerance >> 1)))
        {
            /* This is changed for the purpose of the motor direction issue
            * revert when root cause is found TODO 
            */      
            pidParams.stepSize = motorParams.maxStepSize;             
            if (0u == pidParams.centeringVent)
            {
                pidParams.centeringVent = 1u;
            }
        }
        else
        {
            pidParams.stepSize = (int32_t)(0);
            pidParams.centeringVent = 0u;
        }
#ifdef ENABLE_MOTOR_CC      
        PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount); 
        pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;  
#endif  
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
    uint32_t status = 1u;

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
uint32_t DController::coarseControlCase1(void)
{
    uint32_t status = (uint32_t)(0);
    float pressurePumpTolerance = 0.0f;
    float absValue = 0.0f;
    getAbsPressure(setPointG, gaugePressure, &absValue);
    pressurePumpTolerance =  (absValue/ absolutePressure);

    if(eCcCaseOneReadingOne == ccCaseOneIteration)
    {
        if((pressurePumpTolerance < pidParams.pumpTolerance) &&
                (1u == pidParams.pistonCentered) &&
                (((gaugePressure < setPointG) && (setPointG < gaugeSensorUncertainty)) ||
                ((gaugePressure > setPointG) && (setPointG > (-gaugeSensorUncertainty)))))
        {
            status = 1u;
            setControlIsolate();
            // After this need one more fast read to ensure entry to fine control
            ccCaseOneIteration = eCcCaseOneReadingTwo;
        }
    }
    else
    {
        ccCaseOneIteration = eCcCaseOneReadingOne;
        // Second read of pressure now check whether
        //abort exit if isolation valve switch has made pressure change too much
        //isolation valve switching can release trapped air into the control volume
        //and change the pressure significantly, particularly when in hard vacuum
        //TBD, make this visible to c-code implementation if it helps fix the problem
        if(pressurePumpTolerance < pidParams.pumpTolerance)
        {
            status = 1u;
            pidParams.fineControl = 1u;
            pidParams.stepSize = 0;
#ifdef ENABLE_MOTOR         
            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);            
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
#endif              
            pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
            pidParams.rangeExceeded = 0u;   
            controllerState = eFineControlLoop;        
        }
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

    pistonCentreLeft = screwParams.centerPositionCount - (screwParams.centerTolerance / (int32_t)(2));

    if ((setPointG > gaugePressure) && (pidParams.pistonPosition < pistonCentreLeft))
    {
        // Make the status 1 if this case is executed
        status = 1u;      
        pidParams.stepSize = motorParams.maxStepSize;         

        if (0u == pidParams.centering)
        {
            setControlIsolate();
            pidParams.centering = 1u;
            bayesParams.changeInVolume = (float)(0);
#ifdef ENABLE_MOTOR         
            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount); 
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
#endif              
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

    pistonCentreRight = screwParams.centerPositionCount + (screwParams.centerTolerance / (int32_t)(2));

    if ((setPointG < gaugePressure) && (pidParams.pistonPosition > pistonCentreRight))
    {
        status = 1u;
        pidParams.stepSize = -1 * (motorParams.maxStepSize);        
        if (0u == pidParams.centering)
        {
            setControlIsolate();
            pidParams.centering = 1u;
            bayesParams.changeInVolume = (float)(0);
#ifdef ENABLE_MOTOR         
            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);          
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
#endif  
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

    int32_t pistonCentreLeft = (int32_t)(0);
    int32_t pistonCentreRight = (int32_t)(0);

    effPressureNeg = gaugePressure - gaugeSensorUncertainty;
    effPressurePos = gaugePressure + gaugeSensorUncertainty;            
    pistonCentreLeft = screwParams.centerPositionCount - (screwParams.centerTolerance / (int32_t)(2));
    pistonCentreRight = screwParams.centerPositionCount + (screwParams.centerTolerance / (int32_t)(2));  
    /*
    elif (-sensor['gaugeUncertainty'] > setPointG > pressureG or
    sensor['gaugeUncertainty'] < setPointG < pressureG or
    (pressureG - sensor['gaugeUncertainty']) > 0 > setPointG or
    (pressureG + sensor['gaugeUncertainty']) < 0 < setPointG) and \
    (pressureG > sensor['gaugeUncertainty'] and PID['pumpDown'] == 0
    or pressureG < -sensor['gaugeUncertainty'] and PID['pumpUp'] == 0):
    */
    if(((((-gaugeSensorUncertainty) > setPointG) && (setPointG > gaugePressure)) ||
        ((gaugeSensorUncertainty < setPointG) && (setPointG < gaugePressure)) ||
        ((effPressureNeg > 0.0f) && (0.0f >= setPointG)) ||
        ((effPressurePos < 0.0f) && (0.0f <= setPointG))) &&
        (((gaugePressure > gaugeSensorUncertainty) && (0u == pidParams.pumpDown)) ||
        ((gaugePressure < (-gaugeSensorUncertainty)) && (0u == pidParams.pumpUp))))
    {
        pidParams.stepSize = 0;
        status = 1u;
        if(0u == pidParams.controlledVent)
        {
            /*
            store vent direction for overshoot detection on completion
            estimate system volume during controlled vent
            */
            setControlVent();
            if(effPressureNeg > 0.0f)
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

            if(bayesParams.ventIterations < screwParams.ventDelay)
            {
                bayesParams.ventInitialPressure = gaugePressure;
            }
            else
            {
                bayesParams.ventFinalPressure = gaugePressure;
                estimate();
            }
        }      
        else if (pidParams.pistonPosition > pistonCentreRight)
        {
            pidParams.stepSize = -1 * (motorParams.maxStepSize);                 
            if(0u == pidParams.centeringVent)
            {
                pidParams.centeringVent = 1u;
            }
        }
        else if (pidParams.pistonPosition < pistonCentreLeft)
        {       
            pidParams.stepSize = motorParams.maxStepSize;            
            if(0u == pidParams.centeringVent)
            {
                pidParams.centeringVent = 1u;
            }
        }
        else
        {
            pidParams.stepSize = (int32_t)(0);
            pidParams.centeringVent = 0u;
            bayesParams.ventIterations = bayesParams.ventIterations + 1u;
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

    pistonCentreRight = screwParams.centerPositionCount - (screwParams.centerTolerance / (int32_t)(2));

    if (pidParams.pistonPosition < pistonCentreRight)
    {
        status = 1u;     
        pidParams.stepSize = motorParams.maxStepSize;
        
        if (0u == pidParams.centering)
        {
            setControlIsolate();
            pidParams.centering = 1u;
            bayesParams.changeInVolume = 0.0f;
#ifdef ENABLE_MOTOR         
            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount);            
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
#endif              
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

    pistonCentreLeft = screwParams.centerPositionCount + (screwParams.centerTolerance / (int32_t)(2));

    if (pidParams.pistonPosition > pistonCentreLeft)
    {
        status = 1u;
        pidParams.stepSize = -1 * (motorParams.maxStepSize);
        if (0u == pidParams.centering)
        {
            setControlIsolate();
            pidParams.centering = 1u;
            bayesParams.changeInVolume = 0.0f;
#ifdef ENABLE_MOTOR         
            PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount); 
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
#endif            
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

        if (0u == pidParams.pumpUp)
        {
            if((1u != pidParams.ventDirDown) ||
                ((0u == pidParams.ventDirDown) && 
                    (0u == pidParams.ventDirUp)))
            {
                // previous controlled vent was not in the opposite direction to pump action
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
                pidParams.fineControl = 1u;
                pidParams.stepSize = 0;
#ifdef ENABLE_MOTOR         
                PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount); 
                pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount; 
#endif     
                // PID['position'] = int(screw['readingToPosition'](PID['opticalADC']))  
                pidParams.opticalSensorAdcReading = readOpticalSensorCounts();                
                getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition); 
                controllerState = eFineControlLoop;
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

    if (setPointG < gaugePressure)
    {
        status = 1u;
        pidParams.stepSize = (int32_t)(0);

        if (0u == pidParams.pumpDown)
        {
            if((1u != pidParams.ventDirUp) ||
                ((0u == pidParams.ventDirDown) && 
                    (0u == pidParams.ventDirUp)))
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
                pidParams.fineControl = 1u;
                pidParams.stepSize = 0;
#ifdef ENABLE_MOTOR         
                PV624->stepperMotor->move((int32_t)pidParams.stepSize, &pidParams.stepCount); 
#endif
                pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;      
                // PID['position'] = int(screw['readingToPosition'](PID['opticalADC']))  
                pidParams.opticalSensorAdcReading = readOpticalSensorCounts();                
                getPistonPosition(pidParams.opticalSensorAdcReading, &pidParams.pistonPosition); 
                controllerState = eFineControlLoop;
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
    param.uiValue = myMode;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 3
    param.floatValue = pidParams.pressureSetPoint;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 4
    param.uiValue = (uint32_t)(pidParams.setPointType);
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 5
    param.iValue = pidParams.stepCount;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 6
    param.floatValue = pidParams.pressureError;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 7
    param.iValue = pidParams.totalStepCount;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 8
    param.floatValue = pidParams.controlledPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 9
    param.floatValue = pidParams.pressureAbs;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 10
    param.floatValue = pidParams.pressureGauge;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 11
    param.floatValue = pidParams.pressureBaro;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 12
    param.floatValue = pidParams.pressureOld;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 13
    param.iValue = pidParams.stepSize;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 14
    param.floatValue = pidParams.pressureCorrectionTarget;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 15
    param.floatValue = pidParams.requestedMeasuredMotorCurrent;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 16
    param.floatValue = pidParams.measuredMotorCurrent;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 17
    param.uiValue = pidParams.opticalSensorAdcReading;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 18
    param.iValue = pidParams.pistonPosition;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 19
    param.floatValue = pidParams.motorSpeed;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 20
    param.iValue = pidParams.isSetpointInControllerRange;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 21
    param.floatValue = pidParams.pumpTolerance;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
#endif
    
#ifdef DUMP_BAYES_DATA
    // 22
    param.floatValue = bayesParams.minSysVolumeEstimate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 23
    param.floatValue = bayesParams.maxSysVolumeEstimate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 24
    param.floatValue = bayesParams.minEstimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 25
    param.floatValue = bayesParams.maxEstimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 26
    param.floatValue = bayesParams.measuredPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 27
    param.floatValue = bayesParams.smoothedPresure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 28
    param.floatValue = bayesParams.changeInPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 29
    param.floatValue = bayesParams.prevChangeInPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 30
    param.floatValue = bayesParams.dP2;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 31
    param.floatValue = bayesParams.estimatedVolume;
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 32
    param.uiValue = (uint32_t)(bayesParams.algorithmType);
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 33
    param.floatValue = bayesParams.changeInVolume;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 34
    param.floatValue = bayesParams.prevChangeInVolume;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 35
    param.floatValue = bayesParams.dV2;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 36
    param.floatValue = bayesParams.measuredVolume;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 37
    param.floatValue = bayesParams.estimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 38
    param.floatValue = bayesParams.measuredLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 39
    param.floatValue = bayesParams.estimatedKp;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 40
    param.floatValue = bayesParams.sensorUncertainity;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 41
    param.floatValue = bayesParams.uncertaintyPressureDiff;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 42
    param.floatValue = bayesParams.uncertaintyVolumeEstimate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 43
    param.floatValue = bayesParams.uncertaintyMeasuredVolume;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 44
    param.floatValue = bayesParams.uncertaintyVolumeChange;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 45
    param.floatValue = bayesParams.uncertaintyEstimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 46
    param.floatValue = bayesParams.uncertaintyMeasuredLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 47
    param.floatValue = bayesParams.maxZScore;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 48
    param.floatValue = bayesParams.lambda;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 49
    param.floatValue = bayesParams.uncerInSmoothedMeasPresErr;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 50
    param.floatValue = bayesParams.targetdP;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 51
    param.floatValue = bayesParams.smoothedPressureErr;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 52
    param.floatValue = bayesParams.smoothedSquaredPressureErr;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 53
    param.floatValue = bayesParams.gamma;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 54
    param.floatValue = bayesParams.predictionError;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 55
    param.iValue = bayesParams.predictionErrType;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 56
    param.uiValue = bayesParams.maxIterationsForIIRfilter;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 57
    param.uiValue = bayesParams.minIterationsForIIRfilter;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 58
    param.floatValue = bayesParams.changeToEstimatedLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 59
    param.floatValue = bayesParams.alpha;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 60
    param.floatValue = bayesParams.smoothedPressureErrForPECorrection;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 61
    param.floatValue = bayesParams.log10epsilon;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 62
    param.floatValue = bayesParams.residualLeakRate;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 63
    param.floatValue = bayesParams.measuredLeakRate1;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    // 64
    param.uiValue = bayesParams.numberOfControlIterations;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);    
    // 65
    param.uiValue = bayesParams.ventIterations;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);    
    // 66
    param.floatValue = bayesParams.ventInitialPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length); 
    // 67
    param.floatValue = bayesParams.ventFinalPressure;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);     
#endif
    // 68
    param.uiValue = (uint32_t)(controllerStatus.bytes);   
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);    
    param.uiValue = 0xFEFEFEFEu;    
    totalLength = totalLength + copyData(&buff[totalLength], param.byteArray, length);
    param.uiValue = 0xFEFEFEFEu;    
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
        pidParams.pressureAbs = absolutePressure;
        pidParams.pressureGauge = gaugePressure;
        pidParams.pressureBaro = atmosphericPressure;
        //pidParams.pressureOld = ptrPressureInfo->oldPressure;
        pidParams.elapsedTime = ptrPressureInfo->elapsedTime;
        switch (controllerState)
        {
        case eCoarseControlLoopEntry:
            coarseControlSmEntry();
            resetBayesParameters();
            break;

        case eCoarseControlLoop:
            // run coarse control loop until close to setpoint
            coarseControlLoop();
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
            // run fine control loop until Genii mode changes from control, or control error occurs
            fineControlLoop(); 
            break;

        default:
            break;
        }     
        dumpData(); 
    }
}


