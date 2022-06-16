#test file for PV624 fine pressure control
#May 26 2021
#for use with GENIIsim2.py
#June 30 attept to restore deleted file from backup on git
# Aug 2021
# added additional state variables for c-code verification

# Sept 24:
# removed modulation of motor current with pressure

# Apr 5 2022:
# updated to use digital optical sensors for range limit error
# removed volume estimate during vent
# removed measurement of motor current and speed
# removed motor class, incorporated into pv624 class now
# removed PID['total'], incorporated into PID['pistonPosition']



def fineControlLoop(pv624, screw, sensor, PID, bayes, testing, logging):

    from bayes import estimate as estimate
    from dataStructures4 import calcStatus as calcStatus
    from coarseControl import checkPiston as checkPiston

    # reset bayes parameters on first iteration of fineControlLoop()
    # and set fake leak effect to zero
    # retain bayes volume and optimal kP estimates as these should have been
    # calculated before end of coarseControlLoop() during centering operations

    pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()

    bayes['changeInPressure'] = 0
    bayes['prevChangeInPressure'] = 0
    bayes['changeInVolume'] = 0
    bayes['prevChangeInVolume'] = 0
    bayes['targetdP'] = 0
    bayes['estimatedLeakRate'] = 0
    bayes['uncertaintyEstimatedLeakRate'] = bayes['uncertaintyPressureDiff']
    testing['fakeLeak'] = 0
    bayes['measuredPressure'] = pressure
    bayes['smoothedPressure'] = pressure
    bayes['smoothedPressureErr'] = 0
    bayes['smoothE2'] = 0
    bayes['uncerInSmoothedMeasPresErr'] = 0
    bayes['smoothedPressureErrForPECorrection'] = 0
    PID['mode'] = mode  # additional state variable for c-code verification
    PID['pressureAbs'] = pressure  # additional state variable for c-code verification
    PID['pressureGauge'] = pressureG  # additional state variable for c-code verification
    PID['pressureBaro'] = atmPressure  # additional state variable for c-code verification

    # use timestamp created by coarseControl loop at start of setpoint
    # to match-up coarse and fine control data files later
    # prepend setpoint and value to the filename to differentiate it from the coarse control file
    logging['dataFile'] = str(int(PID['pressureSetPoint'])) + '_' + str(PID['setPointType']) + '_' + logging['dataFile']
    # TBD reset the timestamp at start of fine control
    # logging['startTime'] = datetime.now()

    with open(logging['dataFile'], 'w', newline='') as f:
        csvFile = csv.writer(f, delimiter=',')
        # make file header from data structure keys
        csvFile.writerow(list(PID.keys()) + list(bayes.keys()) + list(testing.keys()) + list(sensor.keys()) +
                         list(screw.keys()))
        # write all values in first row, even static ones
        csvFile.writerow(list(PID.values()) + list(bayes.values()) + list(testing.values()) + list(sensor.values())
                         + list(screw.values()))

        while PID['fineControl'] == 1:
            # read pressure with highest precision
            pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
            # Note: in c-code this readAllSlow() is repeated two times and first read pressures discarded
            # Done to address self-oscillation problem at around 1-2 Hz from apparent pressure phase lag
            # of 3-5 iterations in closed loop response embedded hardware (Oct 2021).
            # C-code iteration loop time is increased to about 150ms
            # because of this "fix".  Unclear what the root cause is, unable to replicate with python version.
            # Apr 7 2002
            # oscillation issue has been replicated in python code for high pressure/low volume condition.
            # fixed by reducing minV volume estimate value limit in dataStructures to 2 mL
            # consider removing additional pressure read from c-code to verify that it remains fixed by
            # this change.

            setPointG = setPoint - atmPressure * spType  # setpoint in mbar G
            PID['mode'] = mode
            PID['pressureAbs'] = pressure  # additional state variable for c-code verification
            PID['pressureGauge'] = pressureG  # additional state variable for c-code verification
            PID['pressureBaro'] = atmPressure  # additional state variable for c-code verification

            if PID['mode'] != 1 or PID['excessLeak'] == 1 or PID['excessVolume'] == 1 or \
                    PID['overPressure'] == 1 or PID['rangeExceeded'] == 1:
                # Genii has changed operating mode to vent or measure
                # or a control error has occurred
                # abort fine control and return to coarse control loop
                PID['fineControl'] = 0
                return

            # adjust measured pressure by simulated leak rate effect
            # control to pressure in the same units as setpoint
            PID['controlledPressure'] = [pressureG, pressure, atmPressure][spType] + testing['fakeLeak']
            PID['pressureSetPoint'] = setPoint
            PID['setPointType'] = spType

            # store previous dP and dV values
            bayes['prevChangeInPressure'] = bayes['changeInPressure']  # previous measured dP (mbar)
            bayes['prevChangeInVolume'] = bayes['changeInVolume']  # previous applied dV (mL)

            # change in pressure from last measurement (mbar), +ve == increasing pressure
            bayes['changeInPressure'] = pressure + testing['fakeLeak'] - bayes['measuredPressure']

            # use absolute pressure for volume estimates using gas law
            bayes['measuredPressure'] = pressure + testing['fakeLeak']

            # store previous pressure error for prediction error calculation
            bayes['targetdP'] = PID['pressureError']
            # Note: Unlike PID['pressureCorrectionTarget'], bayes['targetdP']
            # is not adjusted for leak estimate.
            # The leak estimate may be incorrect and the bayes prediction error
            # should therefore be non-zero even if the PID['pressureCorrectionTarget'] was achieved.

            # pressure error error (mbar), -ve if pressure above setpoint
            PID['pressureError'] = PID['pressureSetPoint'] - PID['controlledPressure']

            # target pressure correction after anticipating leak effect (mbar)
            PID['pressureCorrectionTarget'] = PID['pressureError'] - bayes['estimatedLeakRate']

            # steps to take to achieve zero pressure error in one control iteration
            PID['stepSize'] = int(round(bayes['estimatedKp'] * PID['pressureCorrectionTarget']))

            # setpoint achievable status (True/False)
            # TBD use this to abort setpoint if not achievable before reaching max/min piston positions
            # PID['inRange'] = (PID['pressureError'] > bayes['mindP']) and (PID['pressureError'] < bayes['maxdP'])

            # abort correction if pressure error is within the measurement noise floor to save power,
            # also reduces control noise when at setpoint without a leak
            if abs(PID['pressureCorrectionTarget']) < bayes['uncertaintyPressureDiff'] ** 0.5 and \
                    abs(PID['pressureError']) < bayes['uncertaintyPressureDiff'] ** 0.5:
                PID['pressureCorrectionTarget'] = 0
                PID['stepSize'] = 0
                print('*LP')

            PID['stepCount'] = pv624.MOTOR_MoveContinuous(PID['stepSize'])

            # total number of steps taken
            PID['pistonPosition'] = PID['pistonPosition'] + PID['stepCount']

            # change in volume (mL)
            bayes['changeInVolume'] = -screw['dV'] * PID['stepCount']

            # read the optical sensor piston position
            # updated to digital sensor Apr 5 2022
            checkPiston(pv624=pv624, screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)
            if PID['rangeExceeded'] == 1:
                print('Control Error: rangeExceeded')

            # update system parameter estimates from latest measurement data
            estimate(bayes=bayes, screw=screw, PID=PID)

            # report back status to GENII
            # calcStatus() imported from dataStructures4.py
            calcStatus(PID=PID)
            # and abort on error or mode change

            pv624.setControllerStatus(PID['status'])

            # Code below used for testing controller performance, not for product use:
            #------------------------------------------------------------------------
            # scale maxFakeLeakRate by measured gage pressure and test volume
            # Don't use volume estimate to scale leak rate
            # so that errors in the estimate do not affect applied leak rate
            # and the true leak rate can be back-calculated from pressure and nominal test volume.
            testing['fakeLeakRate'] = -abs(testing['maxFakeLeakRate']) * \
                                    setPointG / screw['maxP'] * screw['nominalV'] / testing['V']

            # new cumulative pressure error from the simulated leak (mbar)
            testing['fakeLeak'] = testing['fakeLeak'] + testing['fakeLeakRate']

            # Code below to be replaced with logging of data to PV624 non-volatile memory
            # for diagnostic purposes.
            # -------------------------------------------------
            if logging['logData']:
                # get timestamp
                PID['elapsedTime'] = round((datetime.now() - logging['startTime']).total_seconds(), 3)
                #logging['startTime'] = datetime.now()  # log time intervals instead of total time, debug timing with oneUSB

                # append dynamic data to dataFile
                csvFile = csv.writer(f, delimiter=',')
                csvFile.writerow(list(PID.values()) + list(bayes.values()) + list(testing.values()))

            # write a small subset of global data structure content to screen
            printValues = [round(PID['controlledPressure'], 2),
                           round(PID['pressureError'], 2),
                           round(bayes['estimatedVolume'], 1),
                           round(bayes['estimatedLeakRate'], 4),
                           round(testing['fakeLeakRate'], 4),
                           round(PID['pistonPosition']),
                           round(PID['elapsedTime']*1000, 0)
                           ]
            formatString = len(printValues) * '{: >8} '
            print(formatString.format(*printValues))


if __name__ == '__main__':
    print('fine control testing')
    import csv
    from datetime import datetime
    import coarseControl as cc
    from dataStructures4 import *

    # initialize controller
    pv624 = cc.initialize(screw=screw, sensor=sensor, PID=PID, bayes=bayes, testing=testing)

    try:
        while True:
            # run coarse control loop until close to setpoint
            cc.coarseControlLoop(pv624=pv624, screw=screw,
                              sensor=sensor, PID=PID, bayes=bayes,
                              testing=testing, logging=logging)

            # run fine control loop until Genii mode changes from control, or control error occurs
            fineControlLoop(pv624=pv624, screw=screw,
                              sensor=sensor, PID=PID, bayes=bayes,
                              testing=testing, logging=logging)
    finally:
        if pv624:
            pv624.ClosePort()

