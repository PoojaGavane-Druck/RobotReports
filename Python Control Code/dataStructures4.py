# -------------------------------------------------------------------------------
# Name:        dataStructures
# Purpose:      global variable data structures used by control algorithm
#
# Author:      200008458
#
# Created:     26/10/2020
# Copyright:   (c) 200008458 2020
# -------------------------------------------------------------------------------
# Dec 4 2020
# removed vestigial components
# May 05 2021
# updated to be used with controller4.py
# triple USB UART with custom PCB V1

# Aug 13 2021
# updated with additional state variables for c-code debug
# added position calibration data for rebuilt controller #5
# updated shuntR value to 0.3 ohm

# Sept 6 2021
# updated value for maxN to 300 for residualL calc in bayes.py

# Sept 9 2021
# removed range estimate parameters (not used)

# Sept 10 2021
# added variables for estimating volume during controlled vent

# Sept 20 2021
# updated hold current value for hi-Z motor control operation (set to maxCurrent always)

# Nov 16 2021
# added testing['latchingValve'] flag
# and screw['maxVentTime'], screw['minVentTime'], screw['ventTimeIncrement'], screw['latchVentTime']
# bayes['ventTime'], bayes['totalVentTime'] = 0
# for controlled vent using non-latching valve in coarseControl.py

# Nov 17 2021
# added testing['dwellAtThreshold'] flag
# used to prevent transition to fine control before adiabatic has decayed to acceptable level
# so that rangeExceeded error not raised subsequently
# removed bayes['smoothE2'] variable, not used anymore

# Nov 23
# added
# testing['forceOvershoot'], PID['overshoot'], PID['overshootScaling']
# for forcing overshoot during pump action and preempt issues with adiabatic decay in fine control

# Mar 30 2022
# removed analog optical sensor calibration data
# added maxLim and minLim to PID for digital optical switch status
# reduced maxVentTime to 4000 to use same current limit resistor as latching valves
# maximum PWM current of vent valve will be 2/3 max current of latching valves
# removed PID['total'], redundant

# Apr 4
# updated to use TDM venting instead of PWM

# Apr 5
# removed old optical sensor parameters and current/speed sense parameters from PID, not used
# increased centerTolerance value to prevent when centering
# increase gaugeUncertainty to allow faster control around 0 barG
# decreased minV to address sustained oscillation in fine control at small volume/high pressure condition

# Apr 6
# added SN's for OWI and USB to avoid embedding into pv624Lib.py and dpi620gLib.py driver
# removed com port settings of sensor (supplanted by pv624 interface)
# added PID['ventRate'] parameter for controlled vent rate in coarse control

# Apr 12
# added minGaugeUncertainty, offset to sensor[]
# added parameters for dual TDM and PWM vent control

# Apr 14
#  maxVentDutyCycle set to 6ms for TDR to match with inloop control value

# Apr 26
# changed PID['pistonCentered'] to [0,1] to match other status variable types

# May26
# Added screw['REF'] for reference sensor auto-detection

# Jun 6
# added sensor['offsetSafetyFactor']
# updated calcStatus to new DUCI spec
# added minPumpTolerance/minPumpTolerance and increased overshootScaling to compensate to previous nominal overshoot
# nominal pump overshoot will now be a fix value of 1.5%
# Done to enable changes to pumpTolerance without affecting overshoot value so that pumpTolerance
# can be reduced in courseControl.py when volume is too large to control without hitting range limits

# June 10
# added PID['pumpAttempts'] as way to detect bouncing around 0 bar G in course control.

# Jun 22
# increase sensorUncertainty to 20 ppm

# Sept 05
# decrease holdVentDutyCycle to 125 from 200
# causes the vent valve to hold 25% rated current continuously when system is vented

# Sept 06
# removed sensor['offsetSafetyFactor'], not used
# added sensor['FSLimitLP'] parameter
# removed sensor['holdVentInterval'] sensor['ventResetThreshold']
# sensor['holdVentIterations'] parameters, and sensor['holdVentCount']
# no longer used to cycle vent open/closed

# Oct 27
#added SN for new PCB PC22S474, COM SN 2064325F5431

# Nov 16
# decreased sensorUncertainty back to 10 ppm FS now that vent hold is fixed

# Nov 23 added PID['overshootDisabled'] flag to enable/disable intentional overshoot
# added back screw['ventResetThreshold'], should not have been removed in Sept

import numpy as np


def calcStatus_old(PID):
    # calculate the status value from the status bit
    # to report back to pv624

    PID['status'] = PID['pumpUp'] + 2 * PID['pumpDown'] + 4 * PID['control'] + 8 * PID[
        'venting'] + 16 * PID['stable'] + 32 * PID['vented'] + 64 * PID[
                        'excessLeak'] + 128 * PID['excessVolume'] + 256 * PID['overPressure']


def calcStatus(PID):
    # calculate the status value from the status bit
    # to report back to pv624
    # updated to correspond with DUCI interface spec Sept 2021
    # updated to DUCI interface Jun 2022
    if PID['ventDir'] == 1:
        PID['ventDirUp'] = 1
        PID['ventDirDown'] = 0
    elif PID['ventDir'] == -1:
        PID['ventDirUp'] = 0
        PID['ventDirDown'] = 1
    else:
        PID['ventDirUp'] = 0
        PID['ventDirDown'] = 0

    PID['status'] = (PID['pumpUp'] + 2 * PID['pumpDown'] + 2**2 * PID['control'] + 2**3 * PID['venting'] +
                     2**4 * PID['stable'] + 2**5 * PID['vented'] + 2**6 * PID['excessLeak'] + 2**7 * PID['excessVolume'] +
                     2**8 * PID['overPressure'] + 2**9 * PID['excessOffset'] + 2**10 * PID['measure'] +
                     2**11 * PID['fineControl'] + 2**12 * PID['pistonCentered'] + 2**13 * PID['centering'] +
                     2**14 * PID['controlledVent'] + 2**15 * PID['centeringVent'] +
                     2**16 * PID['rangeExceeded'] + 2**17 * PID['ccError'] + 2**18 * PID['ventDirUp'] +
                     2**19 * PID['ventDirDown'] + 2**20 * PID['controlRate'] + 2**21 * PID['maxLim'] +
                     2**22 * PID['minLim'] + 2**23 * PID['fastVent'])


# lead screw and motor parameters
screw = {}
screw['motorStepSize'] = 1.8  # full step angle of motor (deg)
screw['microStep'] = 4  # number of microsteps (2 == halfstep)
screw['gearRatio'] = 1  # gear ratio of motor
screw['pistonDiameter'] = 12.2  # piston diameter (mm), Helix0.6 press
screw['lead'] = 0.6  # lead screw pitch (mm/rotation)
screw['length'] = 40  # stroke length of piston (mm)

screw['pistonArea'] = np.pi * (screw['pistonDiameter'] / 2) ** 2  # piston area (mm^2)
screw['dV'] = (screw['motorStepSize'] * screw['lead'] * screw['pistonArea'] * 1e-3) / (
            screw['microStep'] * screw['gearRatio'] * 360)  # volume change per control pulse (mL/pulse)

# pulse counts at various piston positions
screw['maxPosition'] = round(screw['length'] / screw['lead'] * 360 / screw['motorStepSize'] * screw['microStep'])
screw['minPosition'] = 0
screw['centerPosition'] = round(screw['maxPosition']/2)  # step count at nominal center position (home)
screw['centerTolerance'] = 3000  # tolerance for finding center position (counts)
screw['maxStepSize'] = 3000
# maximum number of steps that can be taken in one control iteration, used in coarse control loop

screw['rShunt'] = 0.03  # shunt resistance for current sensor (ohm)
screw['shuntGain'] = 50  # V/V gain of current sensor
screw['readingToCurrent'] = 3.3 / 4096 / screw['shuntGain'] / screw['rShunt'] * 1000  # mA / ADC count
# conversion factor from shunt ADC counts to 24V supply source current (mA)

screw['maxLeak'] = 0.3  # maximum leak rate bound (mbar / interation) at 20 bar G
# and 10 mA, PRD spec is < 0.2 (100 mbar / min in 10mL and 20 barG)
screw['maxP'] = 21000  # maximum allowed pressure in screw press (PM independent) (mbar), for max leak rate adjustments
screw['nominalV'] = 10  # nominal external volume (mL), for max leak rate adjustments


# Apr 6 USB serial numbers for pv624 and OWI
screw['USB'] = ['205C324B5431', '205A32355431', '2064324B5431', '2051325E5431', '2066325C5431','2064325F5431']  # SNs of PV624 board USB interfaces
screw['DPI'] = ['A13G41XMA', 'A13G42XTA', 'A13G3JU6A', 'A13G44LMA', 'A13G1FESA', 'A13G9OO6A',
                   'A13G3JIJA', 'A13G3V47A', 'FTBTAAIEA', 'A14MSA2RA']  # SNs of UB232 DPI OWI interfaces
# May 26 reference sensor ID
screw['REF'] = ['A14MQ095A', 'A14MSA2RA']  # SN of UB232 OWI interface to reference PM on test rig

# Apr 6 controlled rate parameters
#  PWM of vent valve, [1,999] == 0.1% to 99.9% on time at 50kHz PRF
#  current limit of valve driver is 50% higher than what is specified for non-latching valve
#  --> limit DutyCycle to < 500 to prevent over-current condition of valve
screw['minVentDutyCycle'] = 500  # minimum vent valve duty cycle during TDM (us on time)
screw['maxVentDutyCycle'] = 6000  # maximum vent valve duty cycle during TDM (us on time)
screw['ventDutyCycleIncrement'] = 10  # duty cycle increment per iteration during TDM (us)
screw['holdVentDutyCycle'] = 150  # vent valve duty cycle when holding vent (ppt), 200 == 20% full PWM current
screw['maxVentDutyCyclePWM'] = 500  # vent valve duty cycle when holding vent (ppt), 500 == 50% full PWM current
# minimum amount of time to ensure vent is opened in all cases
#screw['holdVentInterval'] = 50  # number of control iterations between applications of holdVentDutyCycle pulse
# when vented.  Used to maintain vented status without continuously energizing vent valve.
screw['ventResetThreshold'] = 0.5  # reset threshold for setting bayes['ventDutyCycle'] to minDutyCycle
# controlled vent will reset to minVentDutyCycle value once remaining pressure error is
# less than ventResetThreshold * previous vent effect
# Used to avoid overshoot during controlled vent towards setpoint.
screw['maxVentRate'] = 1000  # maximum controlled vent rate (mbar / iteration)
screw['minVentRate'] = 1  # minimum controlled vent rate (mbar / iteration)
screw['ventModePWM'] = 1  # value to set valve3 to for PWM control in pv624Lib.py
screw['ventModeTDM'] = 0  # valve to set valve3 to for TDM control in pv624Lip.py
#screw['holdVentIterations'] = round(screw['holdVentInterval'] * 0.2)  # number of iterations to hold vent


# PM sensor settings
sensor = {}
sensor['FS'] = 7000  # PM full scale pressure (mbar), updated after read of PM header
sensor['Ptype'] = 1  # pressure type, see pv624Attributes, 1 = absolute, 0 = gauge
sensor['Stype'] = 1  # pressure sensor type, see pv624Attributes, 1 = PM620 T, 0 = PM620
sensor['TERPSPenalty'] = 1  # scaling factor for TERPS PM uncertainty, 4 = TERPS uncertainty 4x worse than a piezo PM620
sensor['maxOffset'] = 60  # maximum measured offset value before excessOffset status raised
sensor['minGaugeUncertainty'] = 5  # minimum uncertainty of gauge sensor pressure reading vs barometer (mbar)
sensor['gaugeUncertainty'] = sensor['minGaugeUncertainty']
sensor['FSLimitLP'] = 1000  # sensors at or belo2064325F5431w this FS value will treated as low pressure differential sensors
# uncertainty gage sensor pressure reading vs barometer (mbar)

# use to avoid rangeError with very large volumes
sensor['offset'] = 0  # measured offset of PM (mbar)
#sensor['offsetSafetyFactor'] = 1.5
# safety factor for calculation pumpTolerance and gaugeUncertainty from measured sensor offset error
# should be > 1 but not so large that reasonable offset errors trigger excessiveOffset flag

# PID control algorithm parameters
PID = {}
PID['elapsedTime'] = 0  # elapsed time for log file (s)
PID['pressureSetPoint'] = 0  # pressure setpoint (mbar)
PID['setPointType'] = 0  # setpoint type from GENII (0 = gauge, 1 = abs, 2 = baro)
PID['stepCount'] = 0  # number of motor pulses delivered since last stepSize request
PID['pressureError'] = 0  # pressure error for PID (mbar), +ve == below pressure setpoint
PID['controlledPressure'] = 0  # controlled pressure (mbar gage or mbar abs)

# additional state variables for c-code verification
PID['pressureAbs'] = 0  # absolute pressure (mbar)
PID['pressureGauge'] = 0  # gauge pressure (mbar)
PID['pressureBaro'] = 0  # barometric pressure (mbar)
PID['pressureOld'] = 0  # previous gauge pressure (mbar), used to detect when vented

PID['stepSize'] = 0  # requested number of steps to turn motor
PID['pressureCorrectionTarget'] = 0  # leak-adjusted pressure correction target (mbar)
PID['pistonPosition'] = screw['centerPosition']  # optical piston position (steps), assume centered on startup

# PID['inRange'] = True  # setpoint target in controller range, based on bayes range estimate
PID['pumpTolerance'] = 0.005  # max relative distance from setpoint before pumping is required,
# e.g. 0.1 == 10% of setpoint, setpoint in mbar
# Jun 8 update to minPumpTolerance and overshootScaling
PID['minPumpTolerance'] = 0.001  # minimum pump tolerance for largest volumes
PID['maxPumpTolerance'] = 0.01  # maximum pump tolerance for smallest volumes
PID['pumpAttempts'] = 0  # number of pump up or down attempts for current setpoint value

PID['overshoot'] = 0  # amount to overshoot setpoint when pumping
PID['overshootScaling'] = 3  # maxPumpTolerance scaling factor when setting overshoot
PID['overshootDisabled'] = 1 # enable/disable intentional overshoot (1 or 0)

# control rate parameters
PID['ventRate'] = screw['minVentRate']  # target vent rate, mbar / iteration
#PID['holdVentCount'] = screw['holdVentInterval']  # iteration count when holding vent open after vent complete

# Status bits for reporting pv624 control status to GENII
PID['mode'] = 2  # control mode from GENII (0 = measure, 1 = control, 2 = vent, 3 = controlRate)
PID['status'] = 0  # 32 bit integer value of coded status values below
PID['pumpUp'] = 0  # == 1 when pumping required to increase pressure
PID['pumpDown'] = 0  # == 1 when pumping required to decrease pressure
PID['control'] = 0  # == 1 when system is actively controlling pressure setpoint
PID['venting'] = 0  # == 1 when pump is isolated and system is venting to atmosphere
PID['stable'] = 0  # == 1 when pressure is stable to fine control limits
PID['vented'] = 0  # == 1 when pump is isolated and system is vented and vent valve is open
PID['excessLeak'] = 0  # == 1 when excess leak rate is detected, TBD
PID['excessVolume'] = 0  # == 1 when excess volume is detected, TBD
PID['overPressure'] = 0  # == 1 when excess pressure has been applied to the PM, TBD
PID['controlRate'] = 0  # == 1 when pump is isolated and system is controlling pressure rate
PID['excessOffset'] = 0  # == 1 if PM pressure offset larger than sensor['gaugeUncertainty'] when vented
PID['measure'] = 0  # == 1 when pump is isolated and in measure mode
PID['fineControl'] = 0  # == 1 when fine control loop running, == 0 otherwise
PID['pistonCentered'] = 1  # == 1 when piston is centered
PID['centering'] = 0  # == 1 when piston is centering in coarse control loop
PID['controlledVent'] = 0  # == 1 when venting towards setpoint
PID['centeringVent'] = 0  # == 1 when centering piston during controlled vent
PID['rangeExceeded'] = 0  # == 1 when piston adjustment range has been exceeded
PID['coarseControlError'] = 0  # == 1 if unexpected error in coarseControlLoop() detected
PID['ventDir'] = 0  # vent direction during controlled vent, == 1 if venting up, == -1 if venting down, == 0 otherwise
PID['maxLim'] = 1  # maximum pressure position limit switch value, inverted logic, normally == 1
PID['minLim'] = 1  # minimum pressure position limit switch value, inverted logic, normally == 1
PID['fastVent'] = 0  # == 1 when venting towards 0 barG as quickly as possible
PID['ccError'] = 0  # course control error flag
PID['ventDirDown'] = 0  # used only by genii sim
PID['ventDirUp'] = 0    # used only by genii sim

# Bayes Law parameters for estimating of unknown or time varying system parameters
# Used primarily for online volume, range, and leak estimation and tuning of optimal kP
# varX is estimate of variance in parameter X, (variance = stdev**2)
# dX is change in parameter X from its previously measured value
# dX_ is the value of dX from previous control iteration
bayes = {}
bayes['minSysVolumeEstimate'] = 2  # minimum system volume estimate value (mL)
bayes['maxSysVolumeEstimate'] = 100  # maximum system volume estimate value (mL)
bayes['minEstimatedLeakRate'] = 0  # minimum estimated leak rate (mbar)
bayes['maxEstimatedLeakRate'] = 0.2  # maximum absolute value of estimated leak rate (+/- mbar / iteration)
bayes['measuredPressure'] = 1000  # measured pressure (mbar)
bayes['smoothedPressure'] = 1000  # smoothed pressure, depends on controlled pressure stability not sensor uncertainty, (mbar)
bayes['changeInPressure'] = 0  # measured change in pressure from previous iteration (mbar)
bayes['prevChangeInPressure'] = 0  # previous dP value (mbar)
bayes['dP2'] = 0  # change in dP from previous iteration (mbar), for c-code verification

bayes['estimatedVolume'] = bayes['maxSysVolumeEstimate']  # estimate of volume (mL)
bayes['algorithmType'] = 0  # algorithm used to calculate
bayes['changeInVolume'] = 0  # volume change from previous stepSize command (mL)
bayes['prevChangeInVolume'] = 0  # previous dV value (mL), used in regression method
bayes['dV2'] = 0  # change in dV values from previous iteration (mL), for c-code verification
bayes['measuredVolume'] = bayes['maxSysVolumeEstimate']  # volume estimate using Bayes regression (mL)
bayes['estimatedLeakRate'] = bayes['minEstimatedLeakRate']  # estimate in leak rate (mbar/iteration), from regression method
bayes['measuredLeakRate'] = bayes['minEstimatedLeakRate']  # measured leak rate using Bayes regression (mbar / iteration)
bayes['estimatedKp'] = 500  # estimated kP (steps / mbar) that will reduce pressure error
# to zero in one iteration, large for fast initial response
# bayes['measkP'] = bayes['estimatedKp'] #measured optimal kP (steps / mbar) that will reduce pressure error to zero in one iteration
# state value variances
# Jun 22 increase uncertainty to 20ppm from 10ppm
# Nov 16 decrease back to 10 ppm now that vent hold is fixed
bayes['sensorUncertainty'] = (10e-6 * sensor[
    'FS']) ** 2  # uncertainty in pressure measurement (mbar), sigma ~= 10 PPM of FS pressure @ 13 Hz read rate
bayes['uncertaintyPressureDiff'] = 2 * bayes['sensorUncertainty']  # uncertainty in measured pressure differences (mbar)
bayes['uncertaintyVolumeEstimate'] = bayes['maxSysVolumeEstimate'] * 1e6  # uncertainty in volume estimate (mL),
# large because initial volume is unknown, from regression method
bayes['uncertaintyMeasuredVolume'] = (screw['dV'] * 10) ** 2  # uncertainty in volume estimate
# from latest measurement using bayes regression (mL)
bayes['uncertaintyVolumeChange'] = (screw['dV'] * 10) ** 2  # uncertainty in volume change,
# depends mostly on backlash ~= +/-10 half-steps, constant, mL
# vardV is an important apriori parameter, if too large it will prevent
# measurement of system volume with gas law regression and
# volume estimate correction will be only via prediction error nudge,
# slow to respond to changes and settle to steady state value
# If too small it will become overly sensitive to backlash and
# adiabatic effects and increase controller steady state noise/stability
# vardP is also important: if too large it will prevent nudge
# correction to volume estimate and cause sustained oscillation if the
# estimate is not otherwise corrected by gas las measurement.
# If too small volume estimate correction will be only via prediction error nudge,
# slow to respond to changes and settle to steady state value

bayes['uncertaintyEstimatedLeakRate'] = bayes['uncertaintyPressureDiff']  # uncertainty in leak rate from bayes estimate
# and gas law (mbar/iteration), from regression method
bayes['uncertaintyMeasuredLeakRate'] = bayes['uncertaintyPressureDiff']  # measured leak rate uncertainty from bayes regression estimate (mbar/iteration)
bayes['maxZScore'] = 2  # maximum variance spread between measured and estimated values
# before estimated variance is increased
bayes['lambda'] = 0.1  # forgetting factor for smoothE
bayes['uncerInSmoothedMeasPresErr'] = 0  # smoothed measured pressure error variance (mbar**2)
bayes['targetdP'] = 0  # target correction from previous control iteration (mbar)
bayes['smoothedPressureErr'] = 0  # smoothed pressure error (mbar)
# bayes['smoothE2'] = 0  # smoothed squared pressure error (mbar**2), removed Nov 18 2021
bayes['uncerInSmoothedMeasPresErr'] = 0  # smoothed measured pressure error variance (mbar**2)
bayes['gamma'] = 0.98  # volume scaling factor for nudging estimated volume with predictionError
# (0.90,0.98), larger = faster response but noisier estimate
bayes['predictionError'] = 0  # prediction error from previous control iteration (mbar)
bayes['predictionErrType'] = 0  # prediction error type (+/-1), for volume estimate adjustment near setpoint
# bayes['maxP'] = 0  # maximum achievable pressure, from bayes estimates (mbar)
# bayes['minP'] = 0  # minimum achievable pressure, from bayes estimates (mbar)
# bayes['maxdP'] = 1e6  # maximum positive pressure change achievable, from bayes estimates (mbar)
# bayes['mindP'] = -1e6  # maximum negative pressure change achievable, from bayes estimates (mbar)
# bayes['nominalRange'] = PID[
#     'pumpTolerance']  # minimum pressure adjustment range factor when at nominalHome, e.g. 0.1 = minimum +/-10% adjustment range of P at nominalHome piston location
# bayes['nominalHome'] = screw['centerPosition']  # nomimal "home" position to achieve +/- 10% adjustability
# bayes['centerP'] = 0  # expected pressure at center piston position (mbar)
# bayes['maxIterationsForIIRfilter'] = 100  # maximum iterations for leak rate integration filter in PE correction method
bayes['maxIterationsForIIRfilter'] = 300  # maximum iterations for leak rate integration filter in PE correction method
# updated for corrected residualL calc, Sept 2021

bayes['minIterationsForIIRfilter'] = 10  # minimum iterations for leak rate integration filter in PE correction method
bayes['changeToEstimatedLeakRate'] = 0  # change to estimated leak rate for PE correction method (mbar / iteration)
bayes['alpha'] = 0.1  # low-pass IIR filter memory factor for PE correction method (0.1 to 0.98)
bayes['smoothedPressureErrForPECorrection'] = 0  # smoothed pressure error for PE correction method (mbar)
bayes['log10epsilon'] = -0.7  # acceptable residual fractional error in PE method leak rate estimate
# (-2 = +/-1%, -1 = 10%, -0.7 = 20%)
# based on IIR LPF filter state equation: x(k+1) = (1-alpha)*x(k) + alpha * x(k-1), 0 < alpha < 1
# --> alpha**n = epsilon --> alpha = 10**(log10(epsilon) / n)
# n = iteration number for decay of initial condition x(0): x(k) < epsilon * x(0) for k > n
# larger acceptable error speeds convergence at expense of final steady state pressure error
# New state variables for c-code verification
bayes['residualLeakRate'] = 0  # measured residual leak rate from steady-state pressure error (mbar / iteration)
bayes['measuredLeakRate1'] = 0  # estimate of true leak rate magnitude (mbar/iteration)
bayes['numberOfControlIterations'] = bayes['minIterationsForIIRfilter']  # number of control iterations to average over for leak measurement in PE method
# control vent volume calculation algorithm parameters
bayes['ventIterations'] = 0  # number of control iterations since start of controlled vent
bayes['ventInitialPressure'] = 0  # initial pressure at start of controlled vent (mbar G)
bayes['ventFinalPressure'] = 0  # final pressure at end of controlled vent (mbar G)
bayes['ventDutyCycle'] = screw['minVentDutyCycle']  # energized time of vent valve solenoid (us)
bayes['totalVentTime'] = 0  # total time non-latching vent valve has been energized during controlled vent (us)
bayes['dwellCount'] = 0  # number of control iterations into adiabatic dwell at end of coarse control

# data structure for testing and debugging algorithms
testing = {}
testing['forceOvershoot'] = True  # if True force 10x tolerance overshoot when pumping hard vacuum or high pressure
# to compensate for trapped air release into control volume when switching to isolate valve position

# simulated leak rate (mbar / iteration) at 10mL and 20 bar
testing['maxFakeLeakRate'] = screw['maxLeak']

# simulated leak rate, adjusted by current pressure and volume (mbar / iteration)
testing['fakeLeakRate'] = testing['maxFakeLeakRate']
# calibrate optical sensor if True
testing['calibratePosition'] = True
testing['fakeLeak'] = 0  # simulated cumulative leak effect (mbar)

testing['V'] = 5  # external volume value used for fake leak rate adjustment (mL): 5, 25, or 60
testing['maxFakeLeakRate'] = 0  # set to zero to disable fake leak simulation or comment out otherwise

logging = {}  # data logging structure
logging['logData'] = True  # log all data to dataFile if True
logging['dataFile'] = ''  # file name of log file
logging['startTime'] = 0  # start time for log file

if __name__ == '__main__':
    print(screw, sensor, PID, bayes, testing, logging)
