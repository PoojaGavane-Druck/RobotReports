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
* @file     Controller.h
* @version  1.00.00
* @author   Makarand Deshmukh / Nageswara Pydisetty
* @date     2021-07-22
*
* @brief    Controller class header file
*/
//*********************************************************************************************************************
#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include <stdint.h>
#include <ctime>
#include "DValve.h"
#include "DStepperMotor.h"

/* Defines and constants  -------------------------------------------------------------------------------------------*/
#define PM_ISTERPS 1u

#define MAX_MOTOR_STEPS_POS 3000
#define MAX_MOTOR_STEPS_NEG -3000

#define END_STOP_TIMEOUT 15000u // ms, time taken for the piston to move from one end stop to another is approx 12s
#define CENTER_TIMEOUT 7500u // ms, time taken for the piston to move from one end stop to another is approx 12s

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum : uint32_t
{
    eGauge = 0,
    eAbsolute,
    eBarometer,
    eSetPointTypeMax
} eSetPointType_t;

typedef enum : uint32_t
{
    eMethodNone = 0,
    eRegressionMethod,
    eGasLawMethod,
    ePredictionErrorMethod,
} eAlgorithmType_t;

typedef enum : uint32_t
{
    ePumpUpNotRequired = 0,
    ePumpUpRequired
} ePumpUp_t;

typedef enum : uint32_t
{
    ePumpDownNotRequired = 0,
    ePumpDownRequired
} ePumpDown_t;

typedef enum : uint32_t
{
    eCenteringVentStopped = 0,
    eCenteringVentRunning
} eCenteringVent_t;

typedef enum : uint32_t
{
    eVentDirDownNone = 0,
    eVentDirDown
} eVentDirDown_t;

typedef enum : uint32_t
{
    eVentDirUpNone = 0,
    eVentDirUp
} eVentDirUp_t;

typedef enum : uint32_t
{
    eControlledVentStopped = 0,
    eControlledVentRunning
} eControlledVent_t;


typedef enum : uint32_t
{
    ePistonNotCentering = 0,
    ePistonCentering
} ePistonCentering_t;

typedef enum : uint32_t
{
    ePistonOutOfCentre = 0,
    ePistonCentered
} ePistonCentreStatus_t;

typedef enum : uint32_t
{
    eFineControlDisabled = 0,
    eFineControlEnabled
} eFineControlStatus_t;

typedef enum : uint32_t
{
    ePressureNotStable = 0,
    ePressureStable
} ePressureStable_t;

typedef enum : uint32_t
{
    eNotVented = 0,
    eVented
} eVentStatus_t;

typedef enum : uint32_t
{
    eNoExcessLeak = 0,
    eExcessLeakPresent
} eExcessLeak_t;

typedef enum : uint32_t
{
    eNoExcessVolume = 0,
    eExcessVolumePresent
} eExcessVolume_t;

typedef enum : uint32_t
{
    eCoarseControlErrorReset = 0,
    eCoarseControlErrorSet
} eCoarseControlError_t;

typedef enum : uint32_t
{
    eFineControlErrorReset = 0,
    eFineControlErrorSet
} eFineControlError_t;

typedef enum : uint32_t
{
    ePistonInRange = 0,
    ePistonOutOfRange
} ePistonRange_t;

typedef enum : uint32_t
{
    eCenteringStateNone = 0u,
    eCenteringStateMin,
    eCenteringStateMax,
    eCenterMotor,
    eCenterFailed
} eCenteringStates_t;

typedef enum : uint32_t
{
    eCoarseControlLoopEntry = 0,    // In this state reset controller error flags, create log file
    eCoarseControlLoop,             //Do Coarse Control Loop
    eFineControlLoop                //Do Fine Control Loop
} eControllerSmState_t;

typedef enum
{
    eCoarseControl =  0,
    eFineControl
} eControllerType_t;

typedef enum
{
    eVentFirstReading, // Controlled vent state1--- Read pressure
    eVentSecondReading // Controlled vent state2 --- Read pressure find the pressure difference
} eControlVentReading_t;

typedef enum : uint32_t
{
    eCcCaseOneReadingOne = 0,
    eCcCaseOneReadingTwo
} eCcCaseOneReading_t;

typedef enum
{
    eErrorNone = 0,
    eError
} eControllerError_t;

typedef enum
{
    eCenteringNone = 0,
    eCenteringInProgress,
    eCenteringComplete,
    eCenteringFailed
} eStartupCentering_t;

typedef union
{
    struct
    {
        uint32_t pumpUp : 1;
        uint32_t pumpDown : 1;
        uint32_t control : 1;
        uint32_t venting : 1;

        uint32_t stable : 1;
        uint32_t measuring : 1;
        uint32_t vented : 1;
        uint32_t excessLeak : 1;

        uint32_t excessVolume : 1;
        uint32_t overPressure : 1;
        uint32_t reserved: 22;
    } bits;
    uint32_t bytes;
} sCtrlStatusDpi_t;

typedef union
{
    struct
    {
        uint32_t pumpUp : 1; //when pumping required to increase pressure
        uint32_t pumpDown : 1; //when pumping required to decrease pressure
        uint32_t control : 1; //when pump is isolated and system is actively controlling pressure
        uint32_t venting : 1;  //when pump is isolatedand system is venting to atmosphere

        uint32_t stable : 1; //when pressure is stable to fine control limits
        uint32_t vented : 1; //when pump is isolated and system is vented
        uint32_t excessLeak : 1; //when excess leak rate is detected, TBD
        uint32_t excessVolume : 1; //when excess volume is detected, TBD

        uint32_t overPressure : 1; //when excess pressure has been applied to the PM, TBD
        // additional status bits not yet reported to pv624
        uint32_t excessOffset : 1; //if PM pressure offset larger than sensor['gaugeUncertainty'] when vented
        uint32_t measure : 1; //if in measure mode
        uint32_t fineControl : 1; //when fine control loop running, == 0 otherwise

        uint32_t pistonCentered : 1; //1 when piston is centered
        uint32_t centering : 1; //when piston is centering in coarse control loop
        uint32_t controlledVent : 1; //when venting towards setpoint
        uint32_t centeringVent : 1; //when centering piston during controlled vent

        uint32_t rangeExceeded : 1; //when piston adjustment range has been exceeded
        uint32_t coarseControlError : 1; //if unexpected error in coarseControlLoop() detected
        uint32_t ventDirUp : 1; //venting in the upward direction
        uint32_t ventDirDown : 1; // venting in the downward direction

        uint32_t controlRate : 1;
        uint32_t maxLim : 1;
        uint32_t minLim : 1;
        uint32_t fastVent : 1;

        uint32_t reserved8 : 1;
        uint32_t reserved7 : 1;
        uint32_t reserved6 : 1;
        uint32_t reserved5 : 1;

        uint32_t reserved4 : 1;
        uint32_t reserved3 : 1;
        uint32_t reserved2 : 1;
        uint32_t reserved1 : 1;
    } bit;
    uint32_t bytes;
} controllerStatus_t;

typedef union
{
    uint8_t byteArray[4];
    uint8_t byteValue;
    int32_t iValue;
    uint32_t uiValue;
    float32_t floatValue;
} sControllerParam_t;

typedef struct
{
    float32_t pressure;
    float32_t absolutePressure;
    float32_t gaugePressure;
    float32_t atmosphericPressure;
    float32_t oldPressure;
    float32_t pressureSetPoint;                // # pressure setpoint(mbar)
    eSetPointType_t setPointType;
    eControllerMode_t mode;
    uint32_t opticalAdcReading;
#ifdef RUN_ON_STMICRO
    time_t elapsedTime;                     //PID['elapsedTime'] = 0  # elapsed time for log file(s)
#else
    uint32_t elapsedTime;
#endif
} pressureInfo_t;

typedef struct
{
#ifdef RUN_ON_STMICRO
    time_t elapsedTime;                     //PID['elapsedTime'] = 0  # elapsed time for log file(s)
#else
    uint32_t elapsedTime;
#endif
    float32_t pressureSetPoint;     // PID['setpoint'] = 0  # pressure setpoint(mbar)
    eSetPointType_t setPointType;   // PID['spType'] = 0  # setpoint type from GENII(0 = gauge, 1 = abs, 2 = baro)
    int32_t stepCount;              // PID['count'] = 0  # number of motor pulses delivered since last stepSize request
    float32_t pressureError;        // PID['E'] = 0  # pressure error for PID(mbar), +ve == below pressure setpoint
    int32_t totalStepCount;         // PID['total'] = 0  # total step count since start
    float32_t controlledPressure;   // PID['pressure'] = 0  # controlled pressure(mbar gage or mbar abs)

    float32_t pressureAbs;          // Contains value of absolute pressure
    float32_t pressureGauge;        // Contains value of gauge pressure
    float32_t pressureBaro;         // Contains value of atmospheric pressure
    float32_t pressureOld;          // Contains value of an old measured pressure in previous iteration

    int32_t stepSize;                   //PID['stepSize'] = 0  - requested number of steps to turn motor
    float32_t pressureCorrectionTarget; //PID['targetdP'] = 0  leak - adjusted pressure correction target(mbar)
    int32_t pistonPosition; //PID['position'] = 0  # optical piston position(steps), 0 == fully retracted / max volume

    float32_t pumpTolerance;  // PID['pumpTolerance'] = 0.005;
    float32_t minPumpTolerance;
    float32_t maxPumpTolerance;
    uint32_t pumpAttempts;

    float32_t overshoot;            // Overshoot value in mbar
    float32_t overshootScaling;     // Overshoot scaling required in pump up and down modes

    // Control rate parameters
    float32_t ventRate;             // Rate of venting in mbar / iteration
    uint32_t holdVentCount;         // Number of counts to hold vent valve partially open

    uint32_t modeMeasure;           // Indicates if mode is measure
    uint32_t modeControl;           // Indicates if mode is control
    uint32_t modeVent;              // Indicates if mode is vent
    /*
    TBD compute nominal pumpTolerance from volume estimate during centering to prevent overshoot on pump
    and enable control into volumes >> max specified volume
    pumpTolerance should get smaller as volume increases
    */
    // Main status bits
    uint32_t mode;
    uint32_t status;
    uint32_t pumpUp;
    uint32_t pumpDown;
    uint32_t control;
    uint32_t venting;
    uint32_t stable;
    uint32_t vented;
    uint32_t excessLeak;
    uint32_t excessVolume;
    uint32_t overPressure;
    uint32_t controlRate;

    // Additional status bits
    uint32_t excessOffset;
    uint32_t measure;
    uint32_t fineControl;
    uint32_t pistonCentered;
    uint32_t centering;
    uint32_t controlledVent;
    uint32_t centeringVent;
    uint32_t rangeExceeded;
    uint32_t ccError;
    uint32_t ventDirUp;
    uint32_t ventDirDown;
    uint32_t maxLim;
    uint32_t minLim;
    uint32_t fastVent;
} pidParams_t;

typedef struct
{
    float32_t minSysVolumeEstimate; //bayes['minV'] = 5 #minimum system volume estimate value(mL)
    float32_t maxSysVolumeEstimate; //bayes['maxV'] = 100 #maximum system volume estimate value(mL)
    float32_t minEstimatedLeakRate; //bayes['minLeak'] = 0 #minimum estimated leak rate(mbar)
    //bayes['maxLeak'] = 0.2 #maximum absolute value of estimated leak rate(+/ -mbar / iteration)
    float32_t maxEstimatedLeakRate;
    float32_t measuredPressure; //bayes['P'] = 1000 #measured pressure(mbar)
    // bayes['smoothP'] = 1000 #smoothed pressure; depends on controlled pressure stability not sensor uncertainty;
    float32_t smoothedPresure;
    float32_t changeInPressure; //bayes['dP'] = 0 #measured change in pressure from previous iteration(mbar)
    float32_t prevChangeInPressure;// bayes['dP_'] = 0 #previous dP value(mbar)
    float32_t dP2;

    // bayes['V'] = bayes['maxV'] #estimate of volume(mL); set to minV to give largest range estimate on startup
    float32_t estimatedVolume;
    eAlgorithmType_t algorithmType; //bayes['algoV'] = 0 #algorithm used to calculate V
    float32_t changeInVolume; //bayes['dV'] = 0 #volume change from previous stepSize command(mL)
    float32_t prevChangeInVolume; // bayes['dV_'] = 0 #previous dV value(mL); used in regression method
    float32_t dV2;
    float32_t measuredVolume; //bayes['measV'] = bayes['maxV'] #volume estimate using Bayes regression(mL)
    //bayes['leak'] = bayes['minLeak'] #estimate in leak rate(mbar / iteration); from regression method
    float32_t estimatedLeakRate;
    //bayes['measLeak'] = bayes['minLeak'] #measured leak rate using Bayes regression(mbar / iteration)
    float32_t measuredLeakRate;
    /*
    bayes['kP'] = 500 #estimated kP(steps / mbar) that will reduce pressure error to zero in one iteration
    large for fast initial response
    */
    float32_t estimatedKp;
    /*
    bayes['measkP'] = bayes['kP'] #measured optimal kP(steps / mbar) that will
    reduce pressure error to zero in one iteration
    */
    //float measuredKp;
    /*
    state value variances
    bayes['varP'] = (10e-6 * sensor['FS']) * *2 #uncertainty in pressure measurement(mbar);
    sigma ~= 10 PPM of FS pressure @ 13 Hz read rate
    */
    float32_t sensorUncertainity;

    //bayes['vardP'] = 2 * bayes['varP'] #uncertainty in measured pressure differences(mbar)
    float32_t uncertaintyPressureDiff;
    /*
    bayes['varV'] = bayes['maxV'] * 1e6 #uncertainty in volume estimate(mL);
    large because initial volume is unknown; from regression method
    */
    float32_t uncertaintyVolumeEstimate;
    /*
    bayes['varMeasV'] = (screw['dV'] * 10) * *2
    uncertainty in volume estimate from latest measurement using bayes regression(mL)
    */
    float32_t uncertaintyMeasuredVolume;
    /*
    bayes['vardV'] = (screw['dV'] * 10)** 2 # uncertainty in volume change;
    depends mostly on backlash ~= +/ -10 half - steps; constant; mL
    */
    float32_t uncertaintyVolumeChange;

    /* vardV is an important apriori parameter; if too large it will prevent measurement of system volume with gas law
    regression and volume estimate correction will be only via prediction error nudge; slow to respond to changes and
    settle to steady state value. If too small it will become overly sensitive to backlashand adiabatic effectsand
    increase controller steady state noise / stability vardP is also important : if too large it will prevent nudge
    orrection to volume estimateand cause sustained oscillation if the estimate is not otherwise corrected by gas las
    measurement.  if too small volume estimate correction will be only via prediction error nudge;
    slow to respond to changesand settle to steady state value

    //bayes['varLeak'] = bayes['vardP'] uncertainty in leak rate from bayes estimate and gas law(mbar / iteration);
    from regression method
    */
    float32_t uncertaintyEstimatedLeakRate;
    /*
    bayes['varMeasLeak'] = bayes['vardP'] measured leak rate uncertainty from bayes regresssion estimate
    (mbar / iteration)
    */
    float32_t uncertaintyMeasuredLeakRate;
    /*
    bayes['maxZScore'] = 2 maximum variance spread between measured and estimated values
    before estimated variance is increased
    */
    float32_t maxZScore;
    // bayes['lambda'] = 0.1 #forgetting factor for smoothE
    float32_t lambda;
    // bayes['varE'] = 0 #smoothed measured pressure error variance(mbar * *2)
    float32_t uncerInSmoothedMeasPresErr;
    // bayes['targetdP'] = 0 #target correction from previous control iteration(mbar)
    float32_t targetdP;
    // bayes['smoothE'] = 0 #smoothed pressure error(mbar)
    float32_t smoothedPressureErr;
    //bayes['smoothE2'] = 0 #smoothed squared pressure error(mbar * *2)
    float32_t smoothedSquaredPressureErr;
    /*
    bayes['varE'] = 0 #smoothed measured pressure error variance(mbar * *2)
    bayes['gamma'] = 0.98 #volume scaling factor for nudging estimated volume with predictionError;
    (0.90; 0.98); larger = faster response but noisier estimate
    */
    float32_t gamma;
    //bayes['predictionError'] = 0 #prediction error from previous control iteration(mbar)
    float32_t predictionError;
    //bayes['predictionErrorType'] = 0 #prediction error type(+/ -1); for volume estimate adjustment near setpoint
    int32_t predictionErrType;
    //bayes['maxN'] = 100 #maximum iterations for leak rate integration filter in PE correction method
    uint32_t maxIterationsForIIRfilter;
    //bayes['minN'] = 10 #minimum iterations for leak rate integration filter in PE correction method
    uint32_t minIterationsForIIRfilter;
    //bayes['dL'] = 0 #change to estimated leak rate for PE correction method(mbar / iteration)
    float32_t changeToEstimatedLeakRate;
    //bayes['alpha'] = 0.1 #low - pass IIR filter memory factor for PE correction method(0.1 to 0.98)
    float32_t alpha;
    //bayes['smoothE_PE'] = 0 #smoothed pressure error for PE correction method(mbar)
    float32_t smoothedPressureErrForPECorrection;
    /*
    bayes['log10epsilon'] = -0.7 #acceptable residual fractional error in PE method leak rate estimate
    (-2 = +/ -1 %; -1 = 10 %; -0.7 = 20 %)
    */
    float32_t log10epsilon;
    // measured residual leak rate from steady-state pressure error (mbar / iteration)
    float32_t residualLeakRate;
    float32_t measuredLeakRate1;
    // number of control iterations to average over for leak measurement in PE method
    uint32_t numberOfControlIterations;
    // control vent volume calculation algorithm parameters
    // number of control iterations since start of controlled vent
    uint32_t ventIterations;
    //initial pressure at start of controlled vent (mbar G)
    float32_t ventInitialPressure;
    //final pressure at end of controlled vent (mbar G)
    float32_t ventFinalPressure;

    uint32_t ventDutyCycle;     // Time for which valve is energized
    float32_t totalVentTime;    // total time non-latching vent valve has been energized during controlled vent
    uint32_t dwellCount;    // number of control iterations into adiabatic dwell at end of coarse control

} bayesParams_t;

typedef struct
{
    float32_t motorStepSize;
    float32_t microStep;
    //screw['gearRatio'] = 1 #gear ratio of motor
    float32_t gearRatio;
    //screw['pistonDiameter'] = 12.2 #piston diameter(mm); Helix0.6 press
    float32_t pistonDiameter;
    //screw['lead'] = 0.6 #lead screw pitch(mm / rotation)
    float32_t leadScrewPitch;
    //screw['length'] = 38 # travel piston(mm)
    float32_t leadScrewLength;
    //screw['pistonArea'] = np.pi * (screw['pistonDiameter'] / 2) * *2 #piston area(mm ^ 2)
    float32_t pistonArea;
    /*
    screw['dV'] = (screw['motorStepSize'] * screw['lead'] * screw['pistonArea'] * 1e-3) / (screw['microStep'] *
    screw['gearRatio'] * 360)
    #volume change per control pulse(mL / pulse)
    */
    float32_t changeInVolumePerPulse;
    //screw['maxPressure'] = 21000 #maximum system pressure(mbar) for accel / decel current scaling
    float32_t maxPressure;
    /*
    pulse count for full compression of piston; with 2 % safety factor to avoid collisions
    screw['maxPosition'] = int(0.98 * screw['length'] / screw['lead'] * 360 / screw['motorStepSize'] *
    screw['microStep'])
    */
    int32_t maxPosition;
    /*
    pulse count for full retraction of piston; with 2 % safety factor to avoid collisions
    screw['minPosition'] = int(screw['maxPosition'] / 0.98 * 0.02)
    */
    int32_t minPosition;
    /*
    use measured values instead of calculated
    screw['maxPostion'] = 48800
    screw['minPosition'] = 1600
    */
    //screw['centerPosition'] = 25000 # step count at nominal center position(home)
    int32_t centerPositionCount;
    //screw['centerTolerance'] = 2000 # tolerance for finding center position(counts)
    int32_t centerTolerance;

    int32_t maxStepSize;
    //screw['rShunt'] = 0.05  # shunt resistance for current sensor; motor testing(ohm)
    float32_t shuntResistance;
    /*
    screw['shuntGain'] = 0.2 * (20 * 50 / (20 + 50))  # V / V gain of current sensor;
    with 20k Rload sensor and 50k ADC input impedance
    */
    float32_t shuntGain;
    /*
    screw['readingToCurrent'] = 3.3 / (4096 * screw['shuntGain'] * screw['rShunt']) * 1000
    conversion factor from shunt ADC counts to current(mA)
    */
    float32_t readingToCurrent;
    //screw['maxLeak'] = 2  # maximum leak rate bound(mbar / interation) at 20 bar and 10 mA; PRD spec is < 0.2
    float32_t maxLeakRate;
    //screw['maxP'] = 21000  # maximum allowed pressure in screw press(PM independent) (mbar);
    float32_t maxAllowedPressure;
    //screw['nominalV'] = 10  # nominal total volume(mL); for max leak rate adjustments
    float32_t nominalTotalVolume;
    //screw['orificeK'] = 3.5  # vent orifice flow constant, empirically determined (mL/s)
    float32_t orificeK;
    //screw['ventSR'] = 0.075  # nominal control loop iteration time during controlled vent operation (s)
    float32_t ventSr;
    //screw['ventUncertainty'] = 0.4  # uncertainty in volume estimate during controlled vent, 0.4 = +/- 40%
    float32_t ventUncertainty;
    //screw['ventDelay'] = 2  # number of control iterations to wait after switching vent valves before estimating volume
    uint32_t ventDelay;

    // Added for v6 control algorithm
    uint32_t minVentDutyCycle;  // Min vent duty cycle during vent, in us
    uint32_t maxVentDutyCycle;  // Max vent duty cycle during vent in us
    uint32_t ventDutyCycleIncrement;    // Duty cycle increment per iteration
    uint32_t holdVentDutyCycle; // vent duty cycle when holding vent at 20% pwm
    uint32_t maxVentDutyCyclePwm;   // Maximum vent duty while venting
    uint32_t holdVentInterval;  //number of control iterations between applying vent pulse
    float32_t ventResetThreshold; // reset threshold for setting bayes ventDutyCycle to reset
    float32_t maxVentRate;   // Maximum controlled vent rate
    float32_t minVentRate;   // Minimum controlled vent rate
    uint32_t ventModePwm;   // for setting valve 3 to be pwm
    uint32_t ventModeTdm;   // for setting valve 3 to be tdm
    uint32_t holdVentIterations;    // number of iterations to hold vent

    // Only in C Code
    float32_t distancePerStep;
    float32_t distanceTravelled;

} screwParams_t;

typedef struct
{
    //screw['motorStepSize'] = 1.8 #full step angle of motor(deg)
    float32_t motorStepSize;
    //screw['microStep'] = 4 #number of microsteps(2 == halfstep)
    float32_t microStepSize;
    /*
    screw['alphaA'] = 0.98 #motor controller s - curve alpha acceleration value; from excel calculator;
    fast to full speed
    */
    float32_t accelerationAlpha;
    /*
    screw['betaA'] = 2.86 #motor controller s - curve beta acceleration value; from excel calculator;
    fast to full speed
    */
    float32_t accelerationBeta;
    //screw['alphaD'] = 1.02 #motor controller s - curve alpha decelleration value; from excel calculator
    float32_t decellerationAlpha;
    //  screw['betaD'] = 0 #motor controller s - curve beta decelleration value; from excel calculator
    float32_t decellerationBeta;
    //screw['maxCurrent'] = 2 #maximum motor current(A)
    float32_t maxMotorCurrent;
    //screw['minCurrent'] = 0.7 #minimum motor current(A); amount required to overcome friction around 0 bar g
    float32_t minMotorCurrrent;
    /*
    screw['holdCurrent'] = 0.2 #hold current(A); used when when not moving;
    minimum = 0.2 A to avoid motor oscillations when controlling
    */
    float32_t holdCurrent;
    /*
    screw['maxStepSize'] = 3000 # maximum number of steps that can be taken in one control iteration;
    used in coarse control loop
    */
    int32_t maxStepSize;
} motorParams_t;

typedef struct
{
    float32_t fullScalePressure;    // Sensor full scale pressure in mbar
    uint32_t pressureType;         // pressure type, gauge = 0, abs - 1
    uint32_t sensorType;           // sensor type 1 -terps, 0 - guage
    uint32_t terpsPenalty;          // scaling factor ,4 = TERPS uncertainty 4x worse than a piezo PM620
    float32_t gaugeUncertainty;     // maximum uncertainty gage sensor pressure reading vs barometer (mbar)
    float32_t minGaugeUncertainty;  // minimum uncertainty of gauge sensor pressure reading vs barometer (mbar)
    float32_t maxOffset;            // maximum measured offset value before excessOffset status raised
    float32_t offset;               // measured offset of PM (mbar)
    float32_t offsetSafetyFactor;   // safety factor for calculation pumpTolerance and GU from measured sensor offset
} sensorParams_t;


typedef struct
{
    uint32_t forceOvershoot;
    //data structure for testingand debugging algorithms
    //simulated leak rate(mbar / iteration) at 10mL and 20 bar
    float32_t maxFakeLeakRate; // testing['maxFakeLeakRate'] = screw['maxLeak'] * 0.5
    //simulated leak rate, adjusted by current pressure and volume(mbar / iteration)
    float32_t fakeLeakRate;
    //calibrate optical sensor if True
    bool isOpticalSensorCalibrationRequired; //testing['calibratePosition'] = True
    float32_t fakeLeak; //testing['fakeLeak'] = 0  # simulated cumulative leak effect(mbar)
    float32_t volumeForLeakRateAdjustment; //testing['V'] = 10  # fixed volume value used for leak rate adjustment(mL)
} testParams_t;

typedef struct
{
    bool logData;       //# log all data to dataFile if True
    char fileName[100];  //logging['dataFile'] = ''  //# file name of log file
    uint32_t startTime;  //# start time for log file
} loggingParams_t;

/* Variables --------------------------------------------------------------------------------------------------------*/
class DController
{

    float32_t pressureSetPoint;
    float32_t setPointG;
    float32_t absolutePressure;     //Absolute Pressure
    float32_t gaugePressure;        //Gauge Pressure
    float32_t atmosphericPressure;  //Atmospheric Pressure
    eSetPointType_t setPointType;
    float32_t sensorFsValue; //Sensor Full scale Value
    float32_t previousGaugePressure;
    float32_t fsValue ;  //Scaled sensor full scale value
    float32_t sensorOffset;

    // For entry state offset calculation
    float32_t entryInitPressureG;
    float32_t entryFinalPressureG;

    uint32_t msTimer;
    uint32_t previousError;
    uint32_t ledFineControl;
    // Main controller parameters
    sensorParams_t sensorParams;
    pidParams_t pidParams;
    bayesParams_t bayesParams;
    screwParams_t screwParams;
    motorParams_t motorParams;

    eControllerMode_t myMode;
    eControllerMode_t myPrevMode;
    controllerStatus_t controllerStatus;
    // Test parameters
    testParams_t testParams;

    loggingParams_t fineControlLogParams;
    loggingParams_t coarseControlLogParams;
    eControllerSmState_t controllerState;

    eCcCaseOneReading_t ccCaseOneIteration;
    eCenteringStates_t stateCentre;
    int32_t totalSteps;

    uint32_t entryState; // for coarse control entry state machine
    uint32_t entryIterations;

    uint32_t moveTimeout;

    uint32_t getPistonPosition(int32_t *position);
    uint32_t getCalibratedPosition(uint32_t adcReading, int32_t *calPosition);
    uint32_t getAbsPressure(float32_t p1, float32_t p2, float32_t *absValue);

    uint32_t coarseControlMeasure(void);
    uint32_t coarseControlCase1(void);
    uint32_t coarseControlCase2(void);
    uint32_t coarseControlCase3(void);
    uint32_t coarseControlCase4(void);
    uint32_t coarseControlCase5(void);
    uint32_t coarseControlCase6(void);
    uint32_t coarseControlCase7(void);
    uint32_t coarseControlCase8(void);
    uint32_t coarseControlCase9(void);
    uint32_t coarseControlVent(void);
    uint32_t coarseControlRate(void);

    void coarseControlLed(void);
    void fineControlLed(void);

    float32_t pressureAsPerSetPointType(void);
    void resetBayesParameters(void);
    uint32_t readOpticalSensorCounts(void);

    void logDataKeys(eControllerType_t controlType);
    void logPidBayesAndTestParamDataValues(eControllerType_t controlType);
    void logScrewAndMotorControlValues(eControllerType_t controlType);
    void coarseControlSmEntry(void);

    ePistonRange_t validatePistonPosition(int32_t position);
    ePistonCentreStatus_t isPistonCentered(int32_t position);

    void checkPiston(void);
    void initPidParams(void);
    void initSensorParams(void);
    void initScrewParams(void);
    void initBayesParams(void);
    void initMotorParams(void);
    void initTestParams(void);
    void initCenteringParams(void);
    void pulseVent(void);

    float32_t getSign(float32_t value);
    uint32_t floatEqual(float32_t f1, float32_t f2);
    float32_t max(float32_t f1, float32_t f2);
    void dumpData(void);
    void calcStatus(void);
    void initMainParams(void);
    void getOptSensors(uint32_t *opt1, uint32_t *opt2);

    uint32_t copyData(uint8_t *from, uint8_t *to, uint32_t length);

    void calcDistanceTravelled(int32_t stepsMoved);

public:
    DController();
    void initialize(void);
    void setMeasure(void);
    void setControlUp(void);
    void setControlDown(void);
    void setControlIsolate(void);
    void setControlCentering(void);
    void setVent(void);
    void setControlVent(void);
    void setFastVent(void);
    void setControlRate(void);
    void pressureControlLoop(pressureInfo_t *ptrPressureInfo);
    void coarseControlLoop();
    void fineControlLoop();
    void estimate(void);
    uint32_t centreMotor(void);
    uint32_t moveMotorMin(void);
    uint32_t moveMotorMax(void);
    uint32_t moveMotorCenter(int32_t setSteps);
    uint32_t moveMotorCenterFromMax(int32_t setSteps);

    ~DController();
};

#endif /* Controller.h*/
