#test file for PV624 coarse pressure control
#May 26 2021
#for use with GENIIsim2.py

# Aug 23 2021
# commented out state variable update on transition to fine control
# for easier c-code validation

# Sept 3 2021
#  updated threshold to force overshoot during pump up/down adjustment
#  to reduce probability of insufficient piston range from adiabatic recovery after pumping
#  high-pressure and low-vacuum setpoints.
#  TBD add an option to force zero-overshoot for critical applications

# fixed bug causing premature abort of pump up or down after controlled vent towards setpoint
# search for: if pressureG - sensor['gaugeUncertainty'] > 0

# Sept 9 2021
# Fixed switch bounce between pump and controlled vent modes when sitting at P = +/- gaugeUncertainty
# search for: elif (-sensor['gaugeUncertainty'] > setPointG > pressureG

# Sept 10 2021
# Added volume measurement during controlled vent

# Sept 17, 2021
# added support for one-USB FW version using testing['oneUSB'] flag enabled

# Sept 20 2021
# removed current modulation with pressure, causes lag in PC link with oneUSB

# Nov 16 2021
# added non-latching vent valve method
# look for: testing['latchingValve']
# TBD use vent rate data to infer volume prior to moving to fine control,
# unclear what algorithm to use

# Nov 19 2021, TBD
# added dwell at threshold method
# look for testing['dwellAtThreshold']
# forces coarse control to continue acting after setpoint pressure tolerance condition is met
# so that adiabatic effect can decay to manageable level before entering fine control loop.
# Takes about same amount of time as recovering from rangeExceeded error condition in fine control loop
# but uses less power and and gives a more intuitive feel to the control loop actions because piston does not recenter
# more than once for a new setpoint value.

# Nov 19 2021
# added OpenVentFull() and CloseVentFull() to non-latching vent valve method
# look for: testing['latchingValve']
# disabled this vent method until HW mod made to reduce vent valve current to 333 mA
# Using repeated pulsed vent instead, for now (annoying sound but effective)

# added PID['excessOffset'] flag when vented pressure is greater than gaugeUncertainty

# Nov 23
# added testing['forceOvershoot'] method to adjust setpoint during pump
# and force an overshoot of 5x pumpTolerance (2.5% of absolute pressure)
#  requires testing at hard vacuum and 2 bar before release

# Mar 30 2022
# updated for V2 of pv624 board
# and exclusively non-latching vent
# removed dwellAtThreshold method

# Apr 5
# increased centering tolerance

# Apr 6
# added rate control

# Apr 7
# added hold vent to hold vent open without using a lot of power
# removed unobservable updates to count and position for better verification data in log file

# Apr 11
# added fastVent() to controlledVent and vent modes
# added PID['fastVent'] status variable

# Apr 12
# updated initialization to measure PM offset and adjust gaugeUncertainty accordingly
# TBD merge setVent() and setFastVent() to simplify?

# Apr 14
# simplified pump up/down decision by removing dependence on gaugeUncertainty
# TBD add overPressure flag trigger and put into measure mode if pressure exceeds FS limits + 10%
# will need to check flag status to go to pump up mode

# Apr 26
# changed pistonCentered flag to 1,0

# May 9
# corrected bug in fastVent method that was preventing fastVent when abs(setpoint) < gaugeUncertainty

# May 31
# corrected bug in coarseControlLoop() that was centering piston even when new setpoint was
# enroute to center position and within pumpTolerance of current pressure
# removed CloseValve3() commands from all set*() methods, was pulsing the valve open when meant to close it
# unclear if this is required in FW to transition between TDM and PWM modes, but does not appear to have broken anything
# Set centerTolerance to the same value for all centering operations (removed / 2  factor for non-vent centering)
# Changed vented status to be active only when vent valve is open and pressure tolerance met, look for PID['vented']

# June 1
# corrected annoying behaviour that was toggling between pump up/down and fastVent modes
# when near gauge uncertainty.  Now only switches to fastVent mode if not pumping.

# Jun 8
# replaced magic number 1.5 with offsetSafetyFactor in dataStructures4
# corrected error in raising of excessOffset flag
# removed excessOffset flag clear on initialization, excessOffset should remain until reboot
# updated fastVent to enable fastVent iff previous state was not a pump in same direction
# added excessVolume flag set when initializing with rangeExceeded
# added handling of excessVolume flag to avoid repeated rangeExceeded error around +/- gaugeUncertainty

# Jun 10
# zeroed intentional overshoot for setpointG between +/- gaugeUncertainty
# added pumpAttempts limit to stop bouncing around 0 bar G in small volumes with large sensor offset
# TBD add sensor offset bias to gaugeUncertainty and made gaugeUncertainty a fixed value
# to minimize pressure range where venting towards setpoint is not reliable.

# Jun 13
# centered gaugeUncertainty comparisons by sensor offset to minimize operating region where reliably
# venting towards 0 barG is not feasible because of non-zero gaugeUncertainty
# search for instances of quoted text below:
# "sensor['offset'] + sensor['gaugeUncertainty']"
# and "sensor['offset'] - sensor['gaugeUncertainty']"

import csv
from datetime import datetime
import traceback
import time
import sys


def initialize(screw, sensor, PID, bayes, testing):
    # initialize pv624
    # set valves to vent
    # read sensor parameters and piston position
    # update status to GENII
    # Apr 12 measure PM offset pressure when vented
    # and adjust gaugeUncertainty to larger of minGaugeUncertainty or 1.5x offset
    import pv624Lib as pvComms
    from dataStructures4 import calcStatus as calcStatus

    pv624 = []
    try:
        pv624 = pvComms.PV624(screw['USB'])  # find STM32 main board
        # initialize motor controller
        pv624.MOTOR_MoveContinuous(0)  # flush motor controller stepCount value

    except:
        print('Error initializing PV624')
        traceback.print_exc()
        if pv624:
            pv624.ClosePort()
        sys.exit()

    setVent(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)
    checkPiston(pv624=pv624, screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

    (sensor['Stype'], sensor['Ptype']) = pv624.readSensorType()  # PM sensor type and pressure type
    # scale default measurement uncertainties to PM FS value
    uncertaintyScaling = (pv624.readFullScalePressure() / sensor['FS']) ** 2

    sensor['FS'] = pv624.readFullScalePressure()  # update PM full scale pressure (mbar)
    bayes['sensorUncertainty'] = uncertaintyScaling * bayes['sensorUncertainty']
    # uncertainty in pressure measurement (mbar)
    bayes['uncertaintyPressureDiff'] = uncertaintyScaling * bayes['uncertaintyPressureDiff']
    # uncertainty in measured pressure changes (mbar)

    PID['fineControl'] = 0  # disable fine pressure control
    PID['stable'] = 0  # update pressure stable flag

    # Jun 8 replaced 1.5 with offsetSafetyFactor
    # Jun 13 updated to use constant gaugeUncertainty = minGaugeUncertainty
    # to be centered around measured sensor offset when applied to pressures around 0 bar G
    print('waiting for system vent to complete')
    # initialize variables for first iteration of startupWatchdog
    pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
    initialPressureG = pressureG
    pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
    finalPressureG = pressureG
    bayes['changeInPressure'] = abs(finalPressureG - initialPressureG)
    print('pressure change: ', bayes['changeInPressure'])
    sensor['offset'] = finalPressureG
    sensor['gaugeUncertainty'] = sensor['minGaugeUncertainty']
    startupWatchdog = 0

    while bayes['changeInPressure'] > 2 * (bayes['uncertaintyPressureDiff']**0.5):
        # wait until pressure stops changing by more than 2 sigma per iteration
        startupWatchdog += 1
        initialPressureG = finalPressureG
        pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
        finalPressureG = pressureG
        bayes['changeInPressure'] = abs(finalPressureG - initialPressureG)
        print('pressure change: ', bayes['changeInPressure'])
        if startupWatchdog > 100:
            print('Pressure unstable, aborting offset measurement')
            sensor['offset'] = 0
            break
        else:
            sensor['offset'] = finalPressureG

    print('measured sensor offset (mbar)', sensor['offset'])

    # Jun 8 updated criteria for excessOffset flag
    if abs(sensor['offset']) * sensor['offsetSafetyFactor'] > sensor['maxOffset']:
        print('Excessive offset error on PM')
        PID['excessOffset'] = 1
        # TBD Genii should flag this error to user on startup to request zeroing of sensor before starting calibrations

    setMeasure(pv624=pv624, screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)
    calcStatus(PID=PID)
    pv624.setControllerStatus(PID['status'])
    return pv624


def setMeasure(pv624,screw,sensor,PID,bayes,testing):
    # isolate pump for measure
    #pv624.CloseValve3()  # isolate vent port
    pv624.ConfigValve(valveNum=3, config=screw['ventModeTDM'])  # set vent valve to TDM configuration
    pv624.CloseValve1()  # isolate pump inlet, should isolate external source too if vacuum
    pv624.CloseValve2()  # isolate pump outlet, should isolate external source too if pressure
    PID['control'] = 0
    PID['measure'] = 1
    PID['venting'] = 0
    PID['controlRate'] = 0
    PID['vented'] = 0
    PID['pumpUp'] = 0
    PID['pumpDown'] = 0
    PID['centering'] = 0
    PID['controlledVent'] = 0
    PID['fastVent'] = 0
    PID['centeringVent'] = 0
    PID['ventDir'] = 0
    PID['pumpAttempts'] = 0
    print('measure Mode set')


def setControlUp(pv624,screw,sensor,PID,bayes,testing):
    # set for pump up
   #pv624.CloseValve3()  # isolate vent port
    pv624.ConfigValve(valveNum=3, config=screw['ventModeTDM'])  # set vent valve to TDM configuration
    pv624.CloseValve1()  # isolate pump inlet
    pv624.OpenValve2()  # connect pump outlet
    PID['control'] = 1
    PID['measure'] = 0
    PID['venting'] = 0
    PID['controlRate'] = 0
    PID['vented'] = 0
    PID['pumpUp'] = 1
    PID['pumpDown'] = 0
    PID['centering'] = 0
    PID['controlledVent'] = 0
    PID['fastVent'] = 0
    PID['centeringVent'] = 0
    PID['ventDir'] = 0
    PID['pumpAttempts'] += 1
    print('pump up Mode set', PID['controlledPressure'])


def setControlDown(pv624,screw,sensor,PID,bayes,testing):
    # set for pump down, TBD safety check initial pressure vs atmosphere
    #pv624.CloseValve3()  # isolate vent port
    pv624.ConfigValve(valveNum=3, config=screw['ventModeTDM'])  # set vent valve to TDM configuration
    pv624.OpenValve1()  # connect pump inlet
    pv624.CloseValve2()  # isolate pump outlet
    PID['control'] = 1
    PID['measure'] = 0
    PID['venting'] = 0
    PID['controlRate'] = 0
    PID['vented'] = 0
    PID['pumpUp'] = 0
    PID['pumpDown'] = 1
    PID['centering'] = 0
    PID['controlledVent'] = 0
    PID['fastVent'] = 0
    PID['centeringVent'] = 0
    PID['ventDir'] = 0
    PID['pumpAttempts'] += 1
    print('pump down Mode set', PID['controlledPressure'])


def setControlIsolate(pv624,screw,sensor,PID,bayes,testing):
    # isolate pump for screw control
    #pv624.CloseValve3()  # isolate vent port
    pv624.ConfigValve(valveNum=3, config=screw['ventModeTDM'])  # set vent valve to TDM configuration
    pv624.CloseValve1()  # isolate pump inlet
    pv624.CloseValve2()  # isolate pump outlet
    PID['control'] = 1
    PID['measure'] = 0
    PID['venting'] = 0
    PID['controlRate'] = 0
    PID['vented'] = 0
    PID['pumpUp'] = 0
    PID['pumpDown'] = 0
    PID['centering'] = 0
    PID['controlledVent'] = 0
    PID['fastVent'] = 0
    PID['centeringVent'] = 0
    PID['ventDir'] = 0
    print('set Control Isolate')


def setControlCentering(pv624,screw,sensor,PID,bayes,testing):
    # isolate pump for centering piston
    #pv624.CloseValve3()  # isolate vent port
    pv624.ConfigValve(valveNum=3, config=screw['ventModeTDM'])  # set vent valve to TDM configuration
    pv624.CloseValve1()  # isolate pump inlet
    pv624.CloseValve2()  # isolate pump outlet
    PID['control'] = 1
    PID['measure'] = 0
    PID['venting'] = 0
    PID['controlRate'] = 0
    PID['vented'] = 0
    PID['pumpUp'] = 0
    PID['pumpDown'] = 0
    PID['centering'] = 1
    PID['controlledVent'] = 0
    PID['fastVent'] = 0
    PID['centeringVent'] = 0
    PID['ventDir'] = 0
    print('set Control Centering')


def setVent(pv624,screw,sensor,PID,bayes,testing):
    # isolate pump and vent
    pv624.CloseValve1()  # isolate pump inlet
    pv624.CloseValve2()  # isolate pump outlet
    #pv624.CloseValve3()  # isolate vent port
    bayes['ventDutyCycle'] = screw['maxVentDutyCyclePWM']
    pv624.SetValveTime(bayes['ventDutyCycle'])  # set time to energize vent valve
    pv624.ConfigValve(valveNum=3, config=screw['ventModePWM'])  # set vent valve to TDM configuration
    pv624.OpenValve3()  # vent to atmosphere

    PID['control'] = 0
    PID['measure'] = 0
    PID['venting'] = 1
    PID['controlRate'] = 0
    PID['vented'] = 0
    PID['pumpUp'] = 0
    PID['pumpDown'] = 0
    PID['centering'] = 0
    PID['controlledVent'] = 0
    PID['fastVent'] = 0
    PID['centeringVent'] = 0
    PID['ventDir'] = 0
    PID['pumpAttempts'] = 0
    print('vent Mode set')


def setControlVent(pv624, screw, sensor, PID, bayes, testing):
    # isolate pump for controlled vent to setpoint
    pv624.CloseValve1()  # isolate pump inlet
    pv624.CloseValve2()  # isolate pump outlet
    #pv624.CloseValve3()  # isolate vent port
    pv624.ConfigValve(valveNum=3, config=screw['ventModeTDM'])  # set vent valve to TDM configuration
    bayes['ventDutyCycle'] = screw['minVentDutyCycle']
    pv624.SetValveTime(bayes['ventDutyCycle'])  # set time to energize vent valve
    pv624.OpenValve3()  # partially open vent valve

    PID['control'] = 1
    PID['measure'] = 0
    PID['venting'] = 0  # set to 0 because vent was not requested by GENII
    PID['controlRate'] = 0
    PID['pumpUp'] = 0
    PID['pumpDown'] = 0
    PID['centering'] = 0
    PID['controlledVent'] = 1
    PID['fastVent'] = 0
    PID['centeringVent'] = 0
    PID['ventDir'] = 0
    print('controlled vent mode set', PID['controlledPressure'])

def setFastVent(pv624, screw, sensor, PID, bayes, testing):
    # isolate pump for controlled vent to setpoint
    pv624.CloseValve1()  # isolate pump inlet
    pv624.CloseValve2()  # isolate pump outlet
    #pv624.CloseValve3()  # isolate vent port
    pv624.ConfigValve(valveNum=3, config=screw['ventModePWM'])  # set vent valve to PWM configuration
    bayes['ventDutyCycle'] = screw['maxVentDutyCyclePWM']
    pv624.SetValveTime(bayes['ventDutyCycle'])  # set time to energize vent valve
    pv624.OpenValve3()  # fully open vent valve
    PID['pumpUp'] = 0
    PID['pumpDown'] = 0
    PID['controlledVent'] = 0
    PID['fastVent'] = 1
    PID['centeringVent'] = 0
    PID['ventDir'] = 0
    print('fast vent mode set')

def setControlRate(pv624, screw, sensor, PID, bayes, testing):
    # isolate pump for controlled vent to constant pressure rate
    pv624.CloseValve1()  # isolate pump inlet
    pv624.CloseValve2()  # isolate pump outlet
    #pv624.CloseValve3()  # isolate vent port
    pv624.ConfigValve(valveNum=3, config=screw['ventModeTDM'])  # set vent valve to TDM configuration
    bayes['ventDutyCycle'] = screw['minVentDutyCycle']
    pv624.SetValveTime(bayes['ventDutyCycle'])  # set time to energize vent valve
    pv624.OpenValve3()  # partially open vent valve

    PID['control'] = 0
    PID['measure'] = 0
    PID['venting'] = 0
    PID['controlRate'] = 1
    PID['pumpUp'] = 0
    PID['pumpDown'] = 0
    PID['centering'] = 0
    PID['controlledVent'] = 0
    PID['fastVent'] = 0
    PID['centeringVent'] = 0
    PID['ventDir'] = 0
    PID['pumpAttempts'] = 0
    print('controlled rate mode set', PID['controlledPressure'])


def pulseVent(pv624, screw, sensor, PID, bayes, testing):
    if bayes['ventDutyCycle'] > 0:
        pv624.SetValveTime(bayes['ventDutyCycle'])  # set time to energize vent valve
        pv624.OpenValve3()  # partially open vent valve
        #print(bayes['ventDutyCycle'], round(PID['controlledPressure'], 2), PID['vented'])


def checkPiston(pv624, screw, sensor, PID, bayes, testing):
    # check if in allowed range of piston travel
    PID['maxLim'], PID['minLim'] = pv624.ReadOptSensors()  # read the limit switch state
    if PID['maxLim'] == 0:
        #  maximum limit switch triggered
        PID['pistonPosition'] = screw['maxPosition']  # reset position counter
        PID['rangeExceeded'] = 1  # outside of allowed range
    elif PID['minLim'] == 0:
        PID['pistonPosition'] = screw['minPosition']  # reset position counter
        PID['rangeExceeded'] = 1  # outside of allowed range
    else:
        PID['rangeExceeded'] = 0  # within allowed range

    #  decide if piston is centered
    if (screw['centerPosition'] - screw['centerTolerance']
            < PID['pistonPosition'] < screw['centerPosition'] + screw['centerTolerance']):
        PID['pistonCentered'] = 1
    else:
        PID['pistonCentered'] = 0


def coarseControlLoop(pv624,  screw, sensor, PID, bayes, testing, logging):

    from bayes import estimate as estimate
    from dataStructures4 import calcStatus as calcStatus
    # start a new log file on each call to coarseControlLoop()
    # logfile on PC be replaced by logging data to non-volatile memory in PV624
    logging['dataFile'] = datetime.now().strftime('%Y-%m-%dT%H-%M-%S') + '.csv'
    logging['startTime'] = datetime.now()

    # always create a log file
    with open(logging['dataFile'], 'w', newline='') as f:
        csvFile = csv.writer(f, delimiter=',')
        # make file header from data structure keys
        csvFile.writerow(list(PID.keys()) + list(bayes.keys()) + list(testing.keys()) + list(sensor.keys()) +
                         list(screw.keys()))
        # write all values in first row, even static ones
        csvFile.writerow(list(PID.values()) + list(bayes.values()) + list(testing.values()) + list(sensor.values())
                         + list(screw.values()))

        # reset control status flags
        # TBD action when control error flags != 0 ?
        # Jun 8 update to flag excessVolume on rangeExceeded and removed extraneous checkPiston() call and
        # excessOffset/rangeExceeded flag clearing

        PID['stable'] = 0
        PID['excessLeak'] = 0
        PID['excessVolume'] = 0
        PID['overPressure'] = 0
        # Jun 10, clear pumpAttempts count on return from fine control
        PID['pumpAttempts'] = 0

        # Jun 8 calculate pumpTolerance based on previous setpoint volume estimate
        # larger volume = smaller pumpTolerance within max/min tolerance limits
        # On boot estimatedVolume is assumed to be large
        # so pumpTolerance will initialize to minPumpTolerance.
        # Worst case, the true volume will actually be small,
        # in which case the controlled vent after first pump attempt
        # may overshoot setpoint and initial setpoint between
        # +/- gaugeUncertainty will request one unnecessary pump assist.
        # Thereafter, accuracy of pumpTolerance depends accuracy of volume estimate.
        # If the volume estimate is too low then the pumpTolerance value could cause a rangeExceeded in fine control,
        # but this is no worse than using a static pumpTolerance value that is too high and at least now
        # the value will be adjusted to prevent repeat of fault.
        # if the volume estimate is too high, then this is similar to fresh boot situation.
        # Underestimation of volume could occur if the previous adiabatic is so large
        # that the volume estimate remains low until after rangeExceeded
        # (bayes gas law calc returns negative volume estimate)?  This would cause repeated
        # rangeExceeded error for large volumes unless pumpTolerance is coerced back to minimum value by coarse control
        # on rangeExceeded error (ignore volume estimate).
        if PID['rangeExceeded'] == 0:
            # volume estimate is probably ok, use it to adjust pumpTolerance
            # to maximize range of fully-automatic control
            PID['pumpTolerance'] = min(PID['minPumpTolerance'] *
                                       (bayes['maxSysVolumeEstimate'] / bayes['estimatedVolume']),
                                       PID['maxPumpTolerance'])
        else:
            # range exceeded in previous fine control attempt, assume volume estimate cannot be trusted
            # and set pumpTolerance for controlling into large volumes to prevent it from happening again.
            PID['pumpTolerance'] = PID['minPumpTolerance']

        while PID['fineControl'] == 0:
            # update valve config and status on change to mode by GENII
            mode = pv624.readMode()

            if mode == 0 and PID['control'] == 1:
                # mode changed from control to measure
                setMeasure(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 0 and PID['venting'] == 1:
                # mode changed from vent to measure
                setMeasure(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 0 and PID['controlRate'] == 1:
                # mode changed from controlRate to measure
                setMeasure(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 1 and PID['measure'] == 1:
                # mode changed from measure to control
                print('measure to control')
                setControlIsolate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 1 and PID['venting'] == 1:
                # mode change from vent to control
                print('vent to control')
                setControlIsolate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 1 and PID['controlRate'] == 1:
                # mode change from controlRate to control
                print('vent to control')
                setControlIsolate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 2 and PID['measure'] == 1:
                # mode changed from measure to vent
                print('measure to vent')
                setVent(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 2 and PID['control'] == 1:
                # mode changed from control to vent
                print('control to vent')
                setVent(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 2 and PID['controlRate'] == 1:
                # mode changed from controlRate to vent
                print('controlRate to control vent')
                setVent(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 3 and PID['measure'] == 1:
                # mode changed from measure to controlRate
                print('measure to control rate')
                setControlRate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 3 and PID['venting'] == 1:
                # mode change from vent to controlRate
                print('vent to control rate')
                setControlRate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            elif mode == 3 and PID['control'] == 1:
                # mode change from control to controlRate
                print('vent to control rate')
                setControlRate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

            # act on current control mode value

            elif mode == 0:
                # in measure mode
                # read the pressure slowly for less noise
                # update the setpoint value and pressure reading
                pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
                PID['controlledPressure'] = [pressureG, pressure, atmPressure][spType]  # pressure in setpoint units
                PID['pressureSetPoint'] = setPoint
                PID['setPointType'] = spType
                PID['mode'] = mode
                PID['pressureAbs'] = pressure  # additional state variable for c-code verification
                PID['pressureGauge'] = pressureG  # additional state variable for c-code verification
                PID['pressureBaro'] = atmPressure  # additional state variable for c-code verification

            elif mode == 1:
                # in control mode
                # read the pressure as quickly as possible to avoid overshoot during manual pumping or controlled vent
                pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllFast()
                setPointG = setPoint - atmPressure * spType  # setpoint in mbar G
                PID['controlledPressure'] = [pressureG, pressure, atmPressure][spType]  # pressure in setpoint units
                PID['pressureSetPoint'] = setPoint
                PID['setPointType'] = spType
                PID['pressureAbs'] = pressure  # additional state variable for c-code verification
                PID['pressureGauge'] = pressureG  # additional state variable for c-code verification
                PID['pressureBaro'] = atmPressure  # additional state variable for c-code verification
                checkPiston(pv624=pv624, screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)
                PID['mode'] = mode

                # calculate how much to overshoot setpoint when pumping
                # so that subsequent adiabatic decay does not require additional pumping
                # to meet pumpTolerance limits
                # Overshoot by a fixed percentage of absolute pressure.
                # Overshoot is enabled one iteration after pump up or pump down is triggered
                # and disabled one iteration after controlled vent is triggered.
                # Apr 04 2022 fixed bug that was using setpoint instead of setpointA in calc of overshoot
                # Jun 8 change to PID['overshoot'] calculation to remove magic numbers and set the overshoot
                # to the maximum value allowed for best chance of hitting setpoint in one pump attempt.
                #  TBD, get acceptable overshoot from user via GENII?
                # Jun 10 fix PID['overshoot'] to zero overshoot if setPointG is within +/-gaugeUncertainty
                # to reduce possibility of bouncing around 0 bar G in small volumes.
                # It is still possible for this to happen if, for example, setpointG ~= +/- gaugeUncertainty
                # in which case overshoot by pump action alone may occur even with zero intentional overshoot.
                setPointA = setPointG + atmPressure
                if PID['pumpUp'] == 1 and not (sensor['offset'] - sensor['gaugeUncertainty'] <= setPointG
                                               <= sensor['offset'] + sensor['gaugeUncertainty']):
                    PID['overshoot'] = setPointA * PID['overshootScaling'] * PID['maxPumpTolerance']
                elif PID['pumpDown'] == 1 and not (sensor['offset']-sensor['gaugeUncertainty'] <= setPointG
                                                   <= sensor['offset'] + sensor['gaugeUncertainty']):
                    PID['overshoot'] = -setPointA * PID['overshootScaling'] * PID['maxPumpTolerance']
                else:
                    PID['overshoot'] = 0

                # decide how to adjust the pressure
                # Apr 04 2022 added exit condition (4) for pressure and setpoint within gaugeTolerance of 0 bar G
                # to prevent pump/vent cycle within +/- gaugeTolerance with small volumes
                # May 31 2022 added exit condition (5) for when pressure is within pumpTolerance of
                # setpoint and piston motion will be towards center position and travel range is not exceeded
                # Jun 8 simplified and updated condition to always require pressure within pumpTolerance
                # to prevent possibility of endless bouncing between +/- gaugeUncertainty when volume is large.

                if ((PID['pistonCentered'] == 1
                     and abs(setPointG + PID['overshoot'] - pressureG) / pressure < PID['pumpTolerance']
                     and (pressureG <= setPointG + PID['overshoot'] <= sensor['offset'] + sensor['gaugeUncertainty']
                          or pressureG >= setPointG + PID['overshoot'] >= sensor['offset'] - sensor['gaugeUncertainty']))
                        or (PID['rangeExceeded'] == 0
                            and abs(setPointG + PID['overshoot'] - pressureG) / pressure < PID['pumpTolerance']
                            and (sensor['offset'] + sensor['gaugeUncertainty'] < setPointG < pressureG
                                 or pressureG < setPointG < sensor['offset'] - sensor['gaugeUncertainty']))):

                    # coarse adjustment complete, last iteration of coarse control loop
                    # conditions met: (1) piston is centered.
                    # (2) pressure is within pump tolerance of setpoint
                    # and (3) exactly one overshoot of the setpoint has occurred if a pump action has been required.
                    # or (4) pressure is not within pump tolerance of setpoint but within gaugeUncertainty of 0 barG
                    # or (5) piston is not centered but not at range limits either,
                    # and setPoint is towards center position and within pumpTolerance of pressure.
                    # Condition (3) helps ensure that adiabatic recovery after pump does not exceed adjustment range
                    # of piston.  Adiabatic recovery is generally in opposite direction to pump action:
                    # decreasing pressure when pumping up and increasing when pumping down.  This is equivalent
                    # to making the pumpTolerance range one-sided instead of symmetric around setpoint.
                    # Nov 23 2021: added PID['overshoot'] adjustment to setPointG to force a larger overshoot
                    # when pumping

                    print('final pressure', pressureG)

                    PID['stepSize'] = 0
                    PID['stepCount'] = pv624.MOTOR_MoveContinuous(0)  # stop the motor
                    PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']

                    setControlIsolate(pv624=pv624,  screw=screw,
                                      sensor=sensor, PID=PID, bayes=bayes, testing=testing)

                    # Jun 8 removed check of testing['forceOvershoot'].  Method is no longer experimental.
                    if PID['overshoot'] == 0:
                        # overshoot requirement has been met in previous iteration or was not required
                        PID['fineControl'] = 1  # exit coarse control after this iteration
                    else:
                        PID['fineControl'] = 0
                        # Stay in coarse control until pressure is within pumpTolerance of setpoint.
                        # Next iteration will reset overshoot to zero and trigger a controlled vent
                        # to remove overshoot while adiabatic decays.  The controlled vent
                        # must be slow compared to adiabatic decay or the adiabatic will still be substantial
                        # after transitioning to fine control, possibly causing range exceeded error.

                elif setPointG > pressureG and PID['pistonPosition'] < screw['centerPosition'] - screw['centerTolerance']:
                    # move towards center position to increase pressure towards setpoint
                    PID['stepSize'] = screw['maxStepSize']

                    if PID['centering'] == 0:
                        # first time through this case
                        print('center piston, increasing pressure')
                        setControlIsolate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes,
                                          testing=testing)
                        PID['centering'] = 1
                        bayes['changeInVolume'] = 0
                        # Apr 7 removed unobservable count and position change
                        # position and count values will be modified again before writing to file
                        # TBD avoid this to ensure data log is verifiable in simulation
                        # PID['stepCount'] = pv624.MOTOR_MoveContinuous(PID['stepSize'])  # start the motor
                        # PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']
                        bayes['measuredPressure'] = pressure  # initial pressure at start of centering operation, mbar
                        bayes['changeInPressure'] = 0
                        bayes['changeInVolume'] = 0
                    else:
                        bayes['changeInPressure'] = pressure - bayes['measuredPressure']  # change in pressure during centering, mbar
                        bayes['changeInVolume'] = bayes['changeInVolume'] - PID['stepCount'] * screw['dV']  # change in volume during centering, mL

                    estimate(PID=PID, bayes=bayes, screw=screw)  # estimate system volume with gas law

                elif setPointG < pressureG and PID['pistonPosition'] > screw['centerPosition'] + screw['centerTolerance']:
                    # move towards center position to decrease pressure towards setpoint
                    PID['stepSize'] = -screw['maxStepSize']
                    if PID['centering'] == 0:
                        # first time through this case
                        print('center piston, decreasing pressure')
                        setControlIsolate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes,
                                          testing=testing)
                        PID['centering'] = 1
                        bayes['changeInVolume'] = 0
                        # Apr 7 removed unobservable count and position change
                        # position and count values will be modified again before writing to file
                        # TBD avoid this to ensure data log is verifiable in simulation
                        #PID['stepCount'] = pv624.MOTOR_MoveContinuous(PID['stepSize'])  # start the motor
                        #PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']
                        bayes['measuredPressure'] = pressure  # initial pressure at start of centering operation, mbar
                        bayes['changeInPressure'] = 0

                    else:
                        bayes['changeInPressure'] = pressure - bayes['measuredPressure']  # change in pressure during centering, mbar
                        bayes['changeInVolume'] = bayes['changeInVolume'] - PID['stepCount'] * screw['dV']  # change in volume during centering, mL

                    estimate(PID=PID, bayes=bayes, screw=screw)  # estimate system volume from gas law

                #  Apr 11 added fastVent method to increase vent rate when 0 barG is between setPointG and pressureG
                #  May 9 fixed bug that was bypassing fastVent when abs(setPointG) < gaugeUncertainty
                #  now fastVent method increases vent rate when gaugeUncertainty is between setPointG and pressureG
                #  June 1 removed valve flicker between fastVent and pump up/down when hovering around gaugeUncertainty
                #  e.g. when adiabatic recovery after fastVent pushes pressure back outside of +/- gaugeUncertainty
                #  Jun 8 modified to allow fastVent after pump iff the pump was in the opposite direction to the vent
                #  Jun 13 updated comments below to clarify intent of method and subsequent actions.
                elif (((setPointG <= sensor['offset'] + sensor['gaugeUncertainty'] < pressureG)
                       and PID['pumpDown'] == 0)
                      or ((setPointG >= sensor['offset'] - sensor['gaugeUncertainty'] > pressureG)
                          and PID['pumpUp'] == 0)):
                    # Vent as quickly as possible towards +/-gaugeUncertainty around offset
                    # using PWM mode to keep valve open continuously.
                    # Done when setpoint is within +/-gaugeUncertainty of offset and pressureG
                    # is outside this range.  An uncontrolled (fast) vent will bring the pressure to within
                    # gaugeUncertainty of offsetG, which may then require a pump to bring it to within
                    # pumpTolerance of setpointG.
                    PID['stepSize'] = 0
                    if PID['fastVent'] == 0:
                        setFastVent(pv624=pv624, screw=screw, sensor=sensor,
                                    PID=PID, bayes=bayes, testing=testing)
                        # first time through this case
                        if pressureG > setPointG:
                            PID['ventDir'] = -1
                        else:
                            PID['ventDir'] = 1
                        pulseVent(pv624=pv624, screw=screw, sensor=sensor,
                                  PID=PID, bayes=bayes, testing=testing)

                # Apr 04 2022 fixed bug that was entering controlled vent when below setpoint at positive pressure
                # Apr 11 simplified case to remove venting through 0 bar G requirement that is handled by fastVent
                # Jun 13 updated to use sensor offset as bias point for gaugeUncertainty and simplified logic
                elif (pressureG < setPointG + PID['overshoot'] <= sensor['offset'] - sensor['gaugeUncertainty']
                      or sensor['offset'] + sensor['gaugeUncertainty'] <= setPointG + PID['overshoot'] < pressureG):
                    # vent towards setpoint, two possibilities where
                    # controlled vent towards sensor offsetG brings pressure closer to setpoint:
                    # [pressureG, setpointG + overshoot, offsetG - gaugeUncertainty]
                    # [offsetG + gaugeUncertainty, setpointG + overshoot, pressureG]
                    # Note: overshoot is set to zero on next iteration after the controlled vent starts
                    # (pumpUp and pumpDown == 0), and its sign depends on relative magnitude of setpointG and pressureG.
                    # First iteration of controlled vent will attempt vent to overshoot-adjusted setpoint
                    # rather than final setpoint.  Note: This is required because pumpUp and pumpDown are lower
                    # priority actions than controlled vent, so the pump action will be prematurely aborted
                    # unless the controlled vent setpoint is overshoot-adjusted while still pumping.

                    if PID['controlledVent'] == 0:
                        # first time through this case
                        print('controlled vent')
                        setControlVent(pv624=pv624,  screw=screw, sensor=sensor,
                                       PID=PID, bayes=bayes, testing=testing)
                        # store vent direction for overshoot detection on completion
                        # Apr 03 2022 fixed ventDir error that used gaugeUncertainty instead of setPointG for compare
                        if pressureG > setPointG:
                            PID['ventDir'] = -1
                        else:
                            PID['ventDir'] = 1
                        bayes['ventIterations'] = 0  # number of control iterations since start of controlled vent
                        bayes['ventInitialPressure'] = pressureG
                        bayes['ventFinalPressure'] = pressureG

                    # vent to setpoint without overshooting it
                    # pulse valve once per iteration with increasing on-time until
                    # previous effect is greater than half remaining pressure error
                    bayes['ventInitialPressure'] = bayes['ventFinalPressure']
                    bayes['ventFinalPressure'] = pressureG
                    bayes['changeInPressure'] = bayes['ventFinalPressure'] - bayes['ventInitialPressure']

                    if abs(bayes['changeInPressure']) < screw['ventResetThreshold'] * abs(setPointG - pressureG):
                        # increase vent on-time to maxVentTime until previous vent effect is within 50%
                        # of remaining pressure error
                        bayes['ventDutyCycle'] = min(screw['ventDutyCycleIncrement'] + bayes['ventDutyCycle'],
                                                     screw['maxVentDutyCycle'])
                    else:
                        # reset vent on-time back to minimum value and start ramp up again if still
                        # not close enough to setpoint to exit coarse control
                        bayes['ventDutyCycle'] = screw['minVentDutyCycle']

                    pulseVent(pv624=pv624,  screw=screw, sensor=sensor,
                              PID=PID, bayes=bayes, testing=testing)

                    # center piston while venting
                    if PID['pistonPosition'] > screw['centerPosition'] + screw['centerTolerance']:
                        # move towards center position
                        PID['stepSize'] = -screw['maxStepSize']
                        if PID['centeringVent'] == 0:
                            # first time through this case
                            print('center piston, controlled vent')
                            PID['centeringVent'] = 1

                    elif PID['pistonPosition'] < screw['centerPosition'] - screw['centerTolerance']:
                        # move towards center position
                        PID['stepSize'] = screw['maxStepSize']
                        if PID['centeringVent'] == 0:
                            # first time through this case
                            print('center piston, controlled vent')
                            PID['centeringVent'] = 1

                    else:
                        # centeringVent complete, controlledVent continuing
                        PID['stepSize'] = 0
                        PID['centeringVent'] = 0
                        bayes['ventIterations'] = bayes['ventIterations'] + 1  # update vent iteration count

                elif PID['pistonPosition'] < screw['centerPosition'] - screw['centerTolerance']:
                    # move towards center position away from setpoint
                    # will increase pressure error and require pump down to correct afterwards
                    PID['stepSize'] = screw['maxStepSize']

                    if PID['centering'] == 0:
                        # first time through this case
                        print('center piston, increasing pressure')
                        setControlIsolate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes,
                                          testing=testing)
                        PID['centering'] = 1
                        bayes['changeInVolume'] = 0
                        # Apr 7 removed unobservable count and position change
                        # position and count values will be modified again before writing to file
                        # TBD avoid this to ensure data log is verifiable in simulation
                        # PID['stepCount'] = pv624.MOTOR_MoveContinuous(PID['stepSize'])  # start the motor
                        # PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']
                        bayes['measuredPressure'] = pressure  # initial pressure at start of centering operation, mbar
                        bayes['changeInPressure'] = 0
                    else:
                        bayes['changeInPressure'] = pressure - bayes['measuredPressure']
                        # change in pressure during centering, mbar
                        bayes['changeInVolume'] = bayes['changeInVolume'] - PID['stepCount'] * screw['dV']
                        # change in volume during centering, mL
                        estimate(PID=PID, bayes=bayes, screw=screw)  # estimate system volume
                        # uses same algo as during fine control

                elif PID['pistonPosition'] > screw['centerPosition'] + screw['centerTolerance']:
                    # move towards center position away from setpoint
                    # will increase pressure error and required pump up to correct afterwards
                    PID['stepSize'] = -screw['maxStepSize']

                    if PID['centering'] == 0:
                        # first time through this case
                        print('center piston, decreasing pressure')
                        setControlIsolate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes,
                                          testing=testing)
                        PID['centering'] = 1
                        bayes['changeInVolume'] = 0
                        # Apr 7 removed unobservable count and position change
                        # position and count values will be modified again before writing to file
                        # TBD avoid this to ensure data log is verifiable in simulation
                        # PID['stepCount'] = pv624.MOTOR_MoveContinuous(PID['stepSize'])  # start the motor
                        # PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']
                        bayes['measuredPressure'] = pressure  # initial pressure at start of centering operation, mbar
                        bayes['changeInPressure'] = 0
                    else:
                        bayes['changeInPressure'] = pressure - bayes['measuredPressure']  # change in pressure during centering, mbar
                        bayes['changeInVolume'] = bayes['changeInVolume'] - PID['stepCount'] * screw['dV']  # change in volume during centering, mL

                    estimate(PID=PID, bayes=bayes, screw=screw)  # estimate system volume from gas law

                # Apr 04 2022 fixed bug that entered endless pump/vent loop for small volumes at 0 bar G
                elif setPointG + PID['overshoot'] > pressureG:
                    # pump up
                    # In small volumes a previous controlled vent may have overshot setpoint
                    # which could trigger an endless loop
                    # of vent and pump.  Break this loop by detecting if the the pump condition
                    # was preceded by a vent in the opposite direction.
                    PID['stepSize'] = 0
                    if PID['pumpUp'] == 0:
                        # first time through this case
                        if PID['ventDir'] != -1 and PID['pumpAttempts'] == 0:
                            # previous controlled vent was not in the opposite direction to pump action
                            # print('pump up', PID['ventDir'])
                            setControlUp(pv624=pv624,  screw=screw, sensor=sensor,
                                         PID=PID, bayes=bayes, testing=testing)
                        else:
                            # previous control vent overshot setpoint by a more than pumpTolerance
                            # This can happen if the volume is too small to catch the setpoint during one
                            # fast control iteration, in which case the control range is considerably larger than
                            # pumpTolerance anyway.  Assume that is the case and attempt to move to fine control even
                            # though pressure is not within pumpTolerance range.
                            # coarse adjustment complete, last iteration of coarse control loop
                            setControlIsolate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes,
                                              testing=testing)
                            print('aborting pump up', PID['ventDir'])
                            PID['fineControl'] = 1  # exiting coarse control after this iteration
                            PID['stepSize'] = 0
                            # Apr 7 removing unobservable changes to count and position
                            # PID['stepCount'] = pv624.MOTOR_MoveContinuous(0)  # stop the motor
                            # PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']
                            print('Volume estimate:', round(bayes['estimatedVolume'], 2),
                                  round(bayes['uncertaintyVolumeEstimate'], 2))

                # Apr 04 2022 fixed bug that entered endless pump/vent loop for small volumes at 0 bar G
                elif setPointG + PID['overshoot'] < pressureG:
                    # pump down
                    PID['stepSize'] = 0
                    if PID['pumpDown'] == 0:
                        # first time through this case

                        if PID['ventDir'] != 1 and PID['pumpAttempts'] == 0:
                            # previous controlled vent was not in the opposite direction to pump action
                            print('pump down', PID['ventDir'])
                            setControlDown(pv624=pv624,  screw=screw, sensor=sensor,
                                           PID=PID, bayes=bayes, testing=testing)
                        else:
                            # previous control vent overshot setpoint by a more than pumpTolerance
                            # This can happen if the volume is too small to catch the setpoint during one
                            # fast control iteration, in which case the control range is considerably larger than
                            # pumpTolerance anyway.  Assume that is the case and attempt to move to fine control even
                            # though pressure is not within pumpTolerance range.
                            # coarse adjustment complete, last iteration of coarse control loop
                            print('aborting pump down', PID['ventDir'])
                            setControlIsolate(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes,
                                              testing=testing)
                            PID['fineControl'] = 1  # exiting coarse control after this iteration
                            PID['stepSize'] = 0
                            # Apr 7 removed unobservable changes to count and position
                            # PID['stepCount'] = pv624.MOTOR_MoveContinuous(0)  # stop the motor
                            # PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']
                            print('Volume estimate:', round(bayes['estimatedVolume'], 2),
                                  round(bayes['uncertaintyVolumeEstimate'], 2))

                PID['stepCount'] = pv624.MOTOR_MoveContinuous(PID['stepSize'])
                PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']

            elif mode == 2:
                # in vent mode
                # vent to atmosphere while reading pressure slowly to decide if vent complete
                # update setpoint value and center piston in anticipation of a change to control mode
                checkPiston(pv624=pv624, screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)
                pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
                PID['controlledPressure'] = [pressureG, pressure, atmPressure][spType]  # pressure in setpoint units
                PID['pressureSetPoint'] = setPoint
                PID['setPointType'] = spType
                PID['pressureAbs'] = pressure  # additional state variable for c-code verification
                PID['pressureGauge'] = pressureG  # additional state variable for c-code verification
                PID['pressureBaro'] = atmPressure  # additional state variable for c-code verification
                PID['mode'] = mode
                PID['stepSize'] = 0
                # Apr 11 added fastVent method
                # May 31 updated to change PID['vented'] status to 0 when vent valve is closed
                if PID['fastVent'] == 0:
                    # first time through this case
                    setFastVent(pv624=pv624, screw=screw, sensor=sensor,
                                PID=PID, bayes=bayes, testing=testing)
                    bayes['ventIterations'] = 0  # number of control iterations since start of controlled vent
                    bayes['ventFinalPressure'] = pressureG

                # vent at ventRate mbar per iteration until vented
                # pulse valve once per iteration with increasing on-time until
                # vent rate is greater or equal to target rate
                bayes['ventInitialPressure'] = bayes['ventFinalPressure']
                bayes['ventFinalPressure'] = pressureG
                bayes['changeInPressure'] = bayes['ventFinalPressure'] - bayes['ventInitialPressure']
                # May 31 changed vented status to be only when vent valve is open
                # Jun 12 updated comparison to use sensor offset as bias
                if (sensor['offset'] - sensor['gaugeUncertainty'] <
                        pressureG < sensor['offset'] + sensor['gaugeUncertainty']
                        and abs(bayes['changeInPressure']) < 2 * bayes['uncertaintyPressureDiff']**0.5):
                    # system is vented, maintain the vented state
                    # while minimizing power use by pulsing it at holdVentDutyCycle PWM current
                    # for holdVentIterations every holdVentInterval

                    if PID['holdVentCount'] < screw['holdVentIterations']:
                        PID['holdVentCount'] += 1
                        bayes['ventDutyCycle'] = screw['holdVentDutyCycle']
                        pulseVent(pv624=pv624, screw=screw, sensor=sensor,
                                  PID=PID, bayes=bayes, testing=testing)
                        if PID['vented'] == 0:
                            print('Vented')  # print vented status to screen on transition only
                        PID['vented'] = 1

                    elif PID['holdVentCount'] > screw['holdVentInterval']:
                        PID['holdVentCount'] = 0
                        bayes['ventDutyCycle'] = screw['holdVentDutyCycle']
                        pulseVent(pv624=pv624, screw=screw, sensor=sensor,
                                  PID=PID, bayes=bayes, testing=testing)

                    else:
                        PID['holdVentCount'] += 1
                        bayes['ventDutyCycle'] = 0
                        pv624.CloseValve3()  # close the vent valve
                        if PID['vented'] == 1:
                            print('not Vented')  # print vented status to screen on transition only
                        PID['vented'] = 0
                else:
                    # system is not vented, vent as quickly as possible
                    bayes['ventDutyCycle'] = screw['maxVentDutyCyclePWM']
                    PID['holdVentCount'] = 0
                    if PID['vented'] == 1:
                        print('not vented', round(pressureG))  # print on transition from vented only
                    PID['vented'] = 0
                    pulseVent(pv624=pv624, screw=screw, sensor=sensor,
                              PID=PID, bayes=bayes, testing=testing)

                # center piston while venting
                # do not calculate volume as not possible while venting
                if PID['pistonPosition'] > screw['centerPosition'] + screw['centerTolerance']:
                    # move towards center position
                    PID['stepSize'] = -screw['maxStepSize']

                    if PID['centeringVent'] == 0:
                        # first time through this case
                        print('center piston, vent ', PID['pistonPosition'], PID['stepSize'])
                        PID['centeringVent'] = 1
                    print(PID['pistonPosition'], PID['stepSize'])

                elif PID['pistonPosition'] < screw['centerPosition'] - screw['centerTolerance']:
                    # move towards center position
                    PID['stepSize'] = screw['maxStepSize']

                    if PID['centeringVent'] == 0:
                        # first time through this case
                        print('center piston, vent ', PID['pistonPosition'], PID['stepSize'])
                        PID['centeringVent'] = 1
                    print(PID['pistonPosition'], PID['stepSize'])

                else:
                    PID['stepSize'] = 0
                    PID['centeringVent'] = 0

                PID['stepCount'] = pv624.MOTOR_MoveContinuous(PID['stepSize'])
                PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']

            elif mode == 3:
                # in controlled rate mode
                # adjust ventDutyCycle slowly by fixed amount until ventRate is achieved
                pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
                PID['ventRate'] = pv624.ReadVentRate()
                PID['controlledPressure'] = [pressureG, pressure, atmPressure][spType]  # pressure in setpoint units
                PID['pressureSetPoint'] = setPoint
                PID['setPointType'] = spType
                PID['pressureAbs'] = pressure  # additional state variable for c-code verification
                PID['pressureGauge'] = pressureG  # additional state variable for c-code verification
                PID['pressureBaro'] = atmPressure  # additional state variable for c-code verification
                PID['mode'] = mode
                PID['stepSize'] = 0

                if PID['controlledVent'] == 0:
                    # first time through this case
                    print('controlled pressure rate')
                    PID['controlledVent'] = 1
                    bayes['ventIterations'] = 0  # number of control iterations since start of controlled vent
                    bayes['ventFinalPressure'] = pressureG

                # vent at ventRate mbar per iteration until vented
                # pulse valve once per iteration with increasing on-time until
                # vent rate is greater or equal to target rate
                bayes['ventInitialPressure'] = bayes['ventFinalPressure']
                bayes['ventFinalPressure'] = pressureG
                bayes['changeInPressure'] = bayes['ventFinalPressure'] - bayes['ventInitialPressure']

                # May 31 changed vented flag to true only when vent valve is open
                # Jun 13 updated comparison to use sensor offset as bias
                if (sensor['offset'] - sensor['gaugeUncertainty'] <
                        pressureG < sensor['offset'] + sensor['gaugeUncertainty']):
                    # system is vented, maintain the vented state
                    # while minimizing power use by pulsing it at holdVentDutyCycle PWM current
                    # for holdVentIterations every holdVentInterval
                    # TBD needs a watchdog to detect excessOffset
                    # May 31, updated to set vented status when vent valve is open
                    pv624.ConfigValve(valveNum=3, config=screw['ventModePWM'])  # set vent valve to PWM configuration

                    if PID['holdVentCount'] < screw['holdVentIterations']:
                        PID['holdVentCount'] += 1
                        bayes['ventDutyCycle'] = screw['holdVentDutyCycle']
                        pulseVent(pv624=pv624, screw=screw, sensor=sensor,
                                  PID=PID, bayes=bayes, testing=testing)
                        PID['vented'] = 1

                    elif PID['holdVentCount'] > screw['holdVentInterval']:
                        PID['holdVentCount'] = 0
                        bayes['ventDutyCycle'] = screw['holdVentDutyCycle']
                        pulseVent(pv624=pv624, screw=screw, sensor=sensor,
                                  PID=PID, bayes=bayes, testing=testing)
                        PID['vented'] = 1

                    else:
                        PID['holdVentCount'] += 1
                        bayes['ventDutyCycle'] = 0
                        pv624.CloseValve3()  # close the vent valve
                        PID['vented'] = 0

                elif abs(bayes['changeInPressure']) < max(PID['ventRate'], 3*(bayes['uncertaintyPressureDiff']**2)):
                    # increase vent on-time to maxVentDutyCycle until previous vent effect is greater
                    # than the smaller of target ventRate or pressure uncertainty
                    pv624.ConfigValve(valveNum=3, config=screw['ventModeTDM'])  # set vent valve to TDM configuration
                    bayes['ventDutyCycle'] = min(bayes['ventDutyCycle'] + screw['ventDutyCycleIncrement'],
                                                 screw['maxVentDutyCycle'])
                    # print('increasing rate', round(bayes['changeInPressure'], 2), -PID['ventRate'], bayes['ventDutyCycle'])
                    PID['vented'] = 0
                    pulseVent(pv624=pv624, screw=screw, sensor=sensor,
                              PID=PID, bayes=bayes, testing=testing)

                else:
                    # decrease vent on-time to minVentDutyCycle until previous vent effect is less than or
                    # equal to target ventRate
                    pv624.ConfigValve(valveNum=3, config=screw['ventModeTDM'])  # set vent valve to TDM configuration
                    bayes['ventDutyCycle'] = max(bayes['ventDutyCycle'] - screw['ventDutyCycleIncrement'],
                                                 screw['minVentDutyCycle'])
                    # print('decreasing rate', round(bayes['changeInPressure'], 2), -PID['ventRate'], bayes['ventDutyCycle'])
                    PID['vented'] = 0
                    pulseVent(pv624=pv624, screw=screw, sensor=sensor,
                              PID=PID, bayes=bayes, testing=testing)

            else:
                # something unexpected has happened
                print('Error during coarse control loop')
                PID['coarseControlError'] = 1

            # report back status to GENII
            # calcStatus() imported from dataStructures4.py
            calcStatus(PID=PID)
            pv624.setControllerStatus(PID['status'])

            # Code below to be replaced with logging of data to PV624 non-volatile memory
            # for diagnostic purposes.
            # -------------------------------------------------
            if logging['logData']:
                PID['elapsedTime'] = round((datetime.now() - logging['startTime']).total_seconds(), 3)
                # append dynamic data to dataFile
                csvFile = csv.writer(f, delimiter=',')
                csvFile.writerow(list(PID.values()) +
                                 list(bayes.values()) + list(testing.values()))


if __name__ == '__main__':
    print('coarse control testing')
    from dataStructures4 import *

    # initialize controller
    pv624 = initialize(screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

    # run coarse control loop until close to setpoint
    # then wait for measure mode command and repeat
    try:

        while True:
            coarseControlLoop(pv624=pv624,  screw=screw,
                              sensor=sensor, PID=PID, bayes=bayes,
                              testing=testing, logging=logging)

            PID['fineControl'] = 0  # disable fine pressure control
            PID['stable'] = 0  # update pressure stable flag
            while pv624.readMode() != 0:
                # wait until GENII moves to next setpoint
                # by going into measure mode
                time.sleep(0.05)
            setMeasure(pv624=pv624,  screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

    finally:
        if pv624:
            pv624.ClosePort()



