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

#define OPT_SENS_PT_1 422u
#define OPT_SENS_PT_2 670u
#define OPT_SENS_PT_3 1132u
#define OPT_SENS_PT_4 2018u
#define OPT_SENS_PT_5 3253u

#define POSITION_PT_1 0u
#define POSITION_PT_2 12800u
#define POSITION_PT_3 25600u
#define POSITION_PT_4 38400u
#define POSITION_PT_5 50400u

#define MAX_OPT_SENS_CAL_POINTS 5u

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

/*
typedef enum:uint32_t
{
    eModeMeasure = 0,
    eModeControl,
    eModeVent
}eControllerMode_t;
*/

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
    eCoarseControlLoopEntry = 0,   // In this state reset controller error flags, create log file
    eCoarseControlLoop,            //Do Coarse Control Loop
    eCoarseControlExit,
    eFineControlLoopEntry,          // Initialize all fine control variables and create log file for fine control
    eFineControlLoop              //Do Fine Control Loop
} eControllerSmState_t;

typedef enum
{
    eCoarseControl =  0,
    eFineControl
} eControllerType_t;

typedef enum
{
    eControlVentGetFirstReading, // Controlled vent state1--- Read pressure
    eControlVentGetSecondReading // Controlled vent state2 --- Read pressure find the pressure difference
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

        uint32_t reserved12 : 1;
        uint32_t reserved11 : 1;
        uint32_t reserved10 : 1;
        uint32_t reserved9 : 1;

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
    float floatValue;
} sControllerParam_t;

typedef struct
{
    float pressure;
    float absolutePressure;
    float gaugePressure;
    float atmosphericPressure;
    float oldPressure;
    float pressureSetPoint;                // # pressure setpoint(mbar)
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
    float pressureSetPoint; // PID['setpoint'] = 0  # pressure setpoint(mbar)
    eSetPointType_t setPointType; // PID['spType'] = 0  # setpoint type from GENII(0 = gauge, 1 = abs, 2 = baro)
    int32_t stepCount; // PID['count'] = 0  # number of motor pulses delivered since last stepSize request
    float pressureError; // PID['E'] = 0  # pressure error for PID(mbar), +ve == below pressure setpoint
    int32_t totalStepCount; // PID['total'] = 0  # total step count since start
    float controlledPressure; // PID['pressure'] = 0  # controlled pressure(mbar gage or mbar abs)
    float pressureAbs;
    float pressureGauge;
    float pressureBaro;
    float pressureOld;
    int32_t stepSize; //PID['stepSize'] = 0  # requested number of steps to turn motor
    float pressureCorrectionTarget; //PID['targetdP'] = 0  # leak - adjusted pressure correction target(mbar)
    float requestedMeasuredMotorCurrent; //PID['current'] = 0  # requested measured motor current(mA)
    float measuredMotorCurrent; //PID['measCurrent'] = 0  # measured motor current(mA)
    uint32_t opticalSensorAdcReading;//PID['opticalADC'] = 0  # optical sensor ADC reading(0 to 4096)
    int32_t pistonPosition; //PID['position'] = 0  # optical piston position(steps), 0 == fully retracted / max volume
    float motorSpeed; //PID['speed'] = 0  # motor speed from motor controller(pps)
    //PID['inRange'] = True  # setpoint target in controller range, based on bayes range estimate
    int32_t isSetpointInControllerRange;
    // # max relative distance from setpoint before pumping is required, e.g. 0.1 == 10 % of setpoint
    float pumpTolerance;  // PID['pumpTolerance'] = 0.005;
    uint32_t modeMeasure;
    uint32_t modeControl;
    uint32_t modeVent;
    /*
    # TBD compute nominal pumpTolerance from volume estimate during centering to prevent overshoot on pump
    # and enable control into volumes >> max specified volume
    # pumpTolerance should get smaller as volume increases
    */
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
} pidParams_t;

typedef struct
{
    float minSysVolumeEstimate; //bayes['minV'] = 5 #minimum system volume estimate value(mL)
    float maxSysVolumeEstimate; //bayes['maxV'] = 100 #maximum system volume estimate value(mL)
    float minEstimatedLeakRate; //bayes['minLeak'] = 0 #minimum estimated leak rate(mbar)
    //bayes['maxLeak'] = 0.2 #maximum absolute value of estimated leak rate(+/ -mbar / iteration)
    float maxEstimatedLeakRate;
    float measuredPressure; //bayes['P'] = 1000 #measured pressure(mbar)
    // bayes['smoothP'] = 1000 #smoothed pressure; depends on controlled pressure stability not sensor uncertainty;
    float smoothedPresure;
    float changeInPressure; //bayes['dP'] = 0 #measured change in pressure from previous iteration(mbar)
    float prevChangeInPressure;// bayes['dP_'] = 0 #previous dP value(mbar)
    float dP2;
    // bayes['V'] = bayes['maxV'] #estimate of volume(mL); set to minV to give largest range estimate on startup
    float estimatedVolume;
    eAlgorithmType_t algorithmType; //bayes['algoV'] = 0 #algorithm used to calculate V
    float changeInVolume; //bayes['dV'] = 0 #volume change from previous stepSize command(mL)
    float prevChangeInVolume; // bayes['dV_'] = 0 #previous dV value(mL); used in regression method
    float dV2;
    float measuredVolume; //bayes['measV'] = bayes['maxV'] #volume estimate using Bayes regression(mL)
    //bayes['leak'] = bayes['minLeak'] #estimate in leak rate(mbar / iteration); from regression method
    float estimatedLeakRate;
    //bayes['measLeak'] = bayes['minLeak'] #measured leak rate using Bayes regression(mbar / iteration)
    float measuredLeakRate;
    /*
    bayes['kP'] = 500 #estimated kP(steps / mbar) that will reduce pressure error to zero in one iteration
    large for fast initial response
    */
    float estimatedKp;
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
    float sensorUncertainity;
    //bayes['vardP'] = 2 * bayes['varP'] #uncertainty in measured pressure differences(mbar)
    float uncertaintyPressureDiff;
    /*
    bayes['varV'] = bayes['maxV'] * 1e6 #uncertainty in volume estimate(mL);
    large because initial volume is unknown; from regression method
    */
    float uncertaintyVolumeEstimate;
    /*
    bayes['varMeasV'] = (screw['dV'] * 10) * *2
    uncertainty in volume estimate from latest measurement using bayes regression(mL)
    */
    float uncertaintyMeasuredVolume;
    /*
    bayes['vardV'] = (screw['dV'] * 10)** 2 # uncertainty in volume change;
    depends mostly on backlash ~= +/ -10 half - steps; constant; mL
    */
    float uncertaintyVolumeChange;

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
    float uncertaintyEstimatedLeakRate;
    /*
    bayes['varMeasLeak'] = bayes['vardP'] measured leak rate uncertainty from bayes regresssion estimate
    (mbar / iteration)
    */
    float uncertaintyMeasuredLeakRate;
    /*
    bayes['maxZScore'] = 2 maximum variance spread between measured and estimated values
    before estimated variance is increased
    */
    float maxZScore;
    // bayes['lambda'] = 0.1 #forgetting factor for smoothE
    float lambda;
    // bayes['varE'] = 0 #smoothed measured pressure error variance(mbar * *2)
    float uncerInSmoothedMeasPresErr;
    // bayes['targetdP'] = 0 #target correction from previous control iteration(mbar)
    float targetdP;
    // bayes['smoothE'] = 0 #smoothed pressure error(mbar)
    float smoothedPressureErr;
    //bayes['smoothE2'] = 0 #smoothed squared pressure error(mbar * *2)
    float smoothedSquaredPressureErr;
    /*
    bayes['varE'] = 0 #smoothed measured pressure error variance(mbar * *2)
    bayes['gamma'] = 0.98 #volume scaling factor for nudging estimated volume with predictionError;
    (0.90; 0.98); larger = faster response but noisier estimate
    */
    float gamma;
    //bayes['predictionError'] = 0 #prediction error from previous control iteration(mbar)
    float predictionError;
    //bayes['predictionErrorType'] = 0 #prediction error type(+/ -1); for volume estimate adjustment near setpoint
    int32_t predictionErrType;
    //bayes['maxP'] = 0 #maximum achievable pressure; from bayes estimates(mbar)
    //float maxAchievablePressure;
    //bayes['minP'] = 0 #minimum achievable pressure; from bayes estimates(mbar)
    //float minAchievablePressure;
    //bayes['maxdP'] = 1e6 #maximum positive pressure change achievable; from bayes estimates(mbar)
    //float maxPositivePressureChangeAchievable;
    //bayes['mindP'] = -1e6 #maximum negative pressure change achievable; from bayes estimates(mbar)
    //float maxNegativePressureChangeAchievable;
    /*
    bayes['nominalRange'] = PID['pumpTolerance'] #minimum pressure adjustment range factor when at nominalHome;
    e.g. 0.1 = minimum + / -10 % adjustment range of P at nominalHome piston location
    */
    //float minPressureAdjustmentRangeFactor;
    //bayes['nominalHome'] = screw['centerPosition'] #nomimal "home" position to achieve + / -10 % adjustability
    //int32_t nominalHomePosition;
    //bayes['centerP'] = 0 #expected pressure at center piston position(mbar)
    //float expectedPressureAtCenterPosition;
    //bayes['maxN'] = 100 #maximum iterations for leak rate integration filter in PE correction method
    uint32_t maxIterationsForIIRfilter;
    //bayes['minN'] = 10 #minimum iterations for leak rate integration filter in PE correction method
    uint32_t minIterationsForIIRfilter;
    //bayes['dL'] = 0 #change to estimated leak rate for PE correction method(mbar / iteration)
    float changeToEstimatedLeakRate;
    //bayes['alpha'] = 0.1 #low - pass IIR filter memory factor for PE correction method(0.1 to 0.98)
    float alpha;
    //bayes['smoothE_PE'] = 0 #smoothed pressure error for PE correction method(mbar)
    float smoothedPressureErrForPECorrection;
    /*
    bayes['log10epsilon'] = -0.7 #acceptable residual fractional error in PE method leak rate estimate
    (-2 = +/ -1 %; -1 = 10 %; -0.7 = 20 %)
    */
    float log10epsilon;
    // measured residual leak rate from steady-state pressure error (mbar / iteration)
    float residualLeakRate;
    float measuredLeakRate1;
    // number of control iterations to average over for leak measurement in PE method
    uint32_t numberOfControlIterations;
    // control vent volume calculation algorithm parameters
    // number of control iterations since start of controlled vent
    uint32_t ventIterations;
    //initial pressure at start of controlled vent (mbar G)
    float ventInitialPressure;
    //final pressure at end of controlled vent (mbar G)
    float ventFinalPressure;

} bayesParams_t;

typedef struct
{
    //screw['gearRatio'] = 1 #gear ratio of motor
    float gearRatio;
    //screw['pistonDiameter'] = 12.2 #piston diameter(mm); Helix0.6 press
    float pistonDiameter;
    //screw['lead'] = 0.6 #lead screw pitch(mm / rotation)
    float leadScrewPitch;
    //screw['length'] = 38 # travel piston(mm)
    float leadScrewLength;
    //screw['pistonArea'] = np.pi * (screw['pistonDiameter'] / 2) * *2 #piston area(mm ^ 2)
    float pistonArea;
    /*
    screw['dV'] = (screw['motorStepSize'] * screw['lead'] * screw['pistonArea'] * 1e-3) / (screw['microStep'] *
    screw['gearRatio'] * 360)
    #volume change per control pulse(mL / pulse)
    */
    float changeInVolumePerPulse;
    //screw['maxPressure'] = 21000 #maximum system pressure(mbar) for accel / decel current scaling
    float maxPressure;
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
    //screw['rShunt'] = 0.05  # shunt resistance for current sensor; motor testing(ohm)
    float shuntResistance;
    /*
    screw['shuntGain'] = 0.2 * (20 * 50 / (20 + 50))  # V / V gain of current sensor;
    with 20k Rload sensor and 50k ADC input impedance
    */
    float shuntGain;
    /*
    screw['readingToCurrent'] = 3.3 / (4096 * screw['shuntGain'] * screw['rShunt']) * 1000
    conversion factor from shunt ADC counts to current(mA)
    */
    float readingToCurrent;
    //screw['maxLeak'] = 2  # maximum leak rate bound(mbar / interation) at 20 bar and 10 mA; PRD spec is < 0.2
    float maxLeakRate;
    //screw['maxP'] = 21000  # maximum allowed pressure in screw press(PM independent) (mbar);
    float maxAllowedPressure;
    //screw['nominalV'] = 10  # nominal total volume(mL); for max leak rate adjustments
    float nominalTotalVolume;
    //screw['orificeK'] = 3.5  # vent orifice flow constant, empirically determined (mL/s)
    float orificeK;
    //screw['ventSR'] = 0.075  # nominal control loop iteration time during controlled vent operation (s)
    float ventSr;
    //screw['ventUncertainty'] = 0.4  # uncertainty in volume estimate during controlled vent, 0.4 = +/- 40%
    float ventUncertainty;
    //screw['ventDelay'] = 2  # number of control iterations to wait after switching vent valves before estimating volume
    uint32_t ventDelay;
} screwParams_t;

typedef struct
{
    //screw['motorStepSize'] = 1.8 #full step angle of motor(deg)
    float motorStepSize;
    //screw['microStep'] = 4 #number of microsteps(2 == halfstep)
    float microStepSize;
    /*
    screw['alphaA'] = 0.98 #motor controller s - curve alpha acceleration value; from excel calculator;
    fast to full speed
    */
    float accelerationAlpha;
    /*
    screw['betaA'] = 2.86 #motor controller s - curve beta acceleration value; from excel calculator;
    fast to full speed
    */
    float accelerationBeta;
    //screw['alphaD'] = 1.02 #motor controller s - curve alpha decelleration value; from excel calculator
    float decellerationAlpha;
    //  screw['betaD'] = 0 #motor controller s - curve beta decelleration value; from excel calculator
    float decellerationBeta;
    //screw['maxCurrent'] = 2 #maximum motor current(A)
    float maxMotorCurrent;
    //screw['minCurrent'] = 0.7 #minimum motor current(A); amount required to overcome friction around 0 bar g
    float minMotorCurrrent;
    /*
    screw['holdCurrent'] = 0.2 #hold current(A); used when when not moving;
    minimum = 0.2 A to avoid motor oscillations when controlling
    */
    float holdCurrent;
    /*
    screw['maxStepSize'] = 3000 # maximum number of steps that can be taken in one control iteration;
    used in coarse control loop
    */
    int32_t maxStepSize;
} motorParams_t;

typedef struct
{
    //#data structure for testingand debugging algorithms
    //# simulated leak rate(mbar / iteration) at 10mL and 20 bar
    float maxFakeLeakRate; // testing['maxFakeLeakRate'] = screw['maxLeak'] * 0.5

    //# simulated leak rate, adjusted by current pressure and volume(mbar / iteration)
    float fakeLeakRate;

    //# calibrate optical sensor if True
    bool isOpticalSensorCalibrationRequired; //testing['calibratePosition'] = True

    float fakeLeak; //testing['fakeLeak'] = 0  # simulated cumulative leak effect(mbar)

    float volumeForLeakRateAdjustment; //testing['V'] = 10  # fixed volume value used for leak rate adjustment(mL)
} testParams_t;

typedef struct
{
    bool logData;       //# log all data to dataFile if True
    char fileName[100];  //logging['dataFile'] = ''  //# file name of log file
    uint32_t startTime;  //# start time for log file
} loggingParams_t;

/* Variables ----------------------------------------------------------------*/
class DController
{

    float pressureSetPoint;
    float setPointG;
    float absolutePressure;     //Absolute Pressure
    float gaugePressure;        //Gauge Pressure
    float atmosphericPressure;  //Atmospheric Pressure
    eSetPointType_t setPointType;
    float sensorFsValue; //Sensor Full scale Value
    float previousGaugePressure;
    float fsValue ;  //Scaled sensor full scale value

    uint32_t posPoints[MAX_OPT_SENS_CAL_POINTS];
    uint32_t sensorPoints[MAX_OPT_SENS_CAL_POINTS];
    uint32_t msTimer;
    pidParams_t pidParams;
    bayesParams_t bayesParams;
    screwParams_t screwParams;
    motorParams_t motorParams;
    eControllerMode_t myMode;
    controllerStatus_t controllerStatus;
    sCtrlStatusDpi_t ctrlStatusDpi;
    testParams_t testParams;
    loggingParams_t fineControlLogParams;
    loggingParams_t coarseControlLogParams;
    eControllerSmState_t controllerState;
    eControlVentReading_t ventReadingNum;
    eCcCaseOneReading_t ccCaseOneIteration;

    DValve *ventValve;
    DValve *valve1;
    DValve *valve2;
    DStepperMotor *motor;

    uint32_t getPistonPosition(uint32_t adcReading, int32_t *position);
    uint32_t getCalibratedPosition(uint32_t adcReading, int32_t *calPosition);
    uint32_t getAbsPressure(float p1, float p2, float *absValue);
    uint32_t generateSensorCalTable(void);

    uint32_t coarseControlMeasure(void);
    uint32_t coarseControlCase1(void);
    uint32_t coarseControlCase2(void);
    uint32_t coarseControlCase3(void);
    uint32_t coarseControlCase4(void);
    uint32_t coarseControlCase5(void);
    uint32_t coarseControlCase6(void);
    uint32_t coarseControlCase7(void);
    uint32_t coarseControlCase8(void);
    uint32_t coarseControlVent(void);
    float pressureAsPerSetPointType(void);
    void resetBayesParameters(void);
    uint32_t readOpticalSensorCounts(void);

    void logDataKeys(eControllerType_t controlType);
    void logPidBayesAndTestParamDataValues(eControllerType_t controlType);
    void logScrewAndMotorControlValues(eControllerType_t controlType);
    void fineControlSmEntry(void);
    void coarseControlSmEntry(void);
    void coarseControlSmExit(void);

    ePistonRange_t validatePistonPosition(int32_t position);
    ePistonCentreStatus_t isPistonCentered(int32_t position);
    void setMotorCurrent(void);
    void initPidParams(void);
    void initScrewParams(void);
    void initBayesParams(void);
    void initMotorParams(void);
    void initTestParams(void);

    float getSign(float value);
    uint32_t floatEqual(float f1, float f2);
    float max(float f1, float f2);
    void dumpData(void);
    void calcStatus(void);
    void initMainParams(void);
    uint32_t copyData(uint8_t *from, uint8_t *to, uint32_t length);

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
    void pressureControlLoop(pressureInfo_t *ptrPressureInfo);
    void coarseControlLoop();
    void fineControlLoop();
    void estimate(void);
    ~DController();
};

#endif /* Controller.h*/
