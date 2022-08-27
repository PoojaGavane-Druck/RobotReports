import time
import paceLib as pace
import dpi620gLib as dpi
from datetime import datetime
import dpiAttributes as dpiAttr
import pressureTestAttributes as testAttr

printIt = 1
writeLog = 1

paceSn = ['AC0128TUA']
dpi620gSn = ['FTBTA7ISA']
fileName = 'AUTO_PRESSURE_TESTS_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'

try:
    file = open(fileName, "w")
except:
    errorHere = 1
    print("Could not open log file")

def display(toDisplay):
    if printIt == 1:
        print(toDisplay)
    if writeLog == 1:
        # File writing enabled 
        file.write(toDisplay + "\n")

expInputPressurePos = 2000
expInputPressureNeg = -500

def runPrerequisiteTest():
    DPI620G = dpi.DPI620G(dpi620gSn)
    PACE6000 = pace.PACE(paceSn)

    testPassed = 0
    reason = "None"

    PACE6000.setPressureMode(2)
    paceSetPoint = 200
    PACE6000.setPressurePoint(paceSetPoint, 2)
    readSetPoint = PACE6000.getPressurePoint(2)
    if readSetPoint == paceSetPoint:
        display("Pace available - connection passed")

        inpPos = PACE6000.getInputPressurePos(2)
        inpNeg = PACE6000.getInputPressureNeg(2)

        if inpPos > expInputPressurePos:
            display("Input positive pressure passed")
            if inpNeg < expInputPressureNeg:
                display("Input negative pressure passed")

                # test if connection between PV624 and DPI620G is established
                dk, version = DPI620G.getRI()
                if dk == "DK0499" and version != "":
                    display("PV624 connected: " + dk + version)

                    mode = DPI620G.getCM()
                    if mode == 0:
                        display("PV624 in measure mode")    
                        passed = 1
                        reason = ""
                    else:
                        display("PV624 not in measure mode, setting measure mode")
                        DPI620G.setKM('R')
                        km = DPI620G.getKM()

                        if km == 1:
                            DPI620G.setCM(0)
                            mode = DPI620G.getCM()
                            if mode == 0:
                                display("PV624 in measure mode") 
                                passed = 1
                                reason = ""  
                        else:
                            reason = "Remote mode not set on PV624"
                            display(reason)
                            testPassed = 0   

                    if passed == 1:
                        # Generate pressure from PACE
                        paceSetPoint = 30
                        PACE6000.setPressurePoint(paceSetPoint, 2)
                        readSetPoint = PACE6000.getPressurePoint(2)

                        if readSetPoint == paceSetPoint:
                            PACE6000.setOutputOn(2)
                            time.sleep(3)
                            limHigh = paceSetPoint + (paceSetPoint * 0.01)
                            limLow = paceSetPoint - (paceSetPoint * 0.01)
                            controlledPressure = PACE6000.getControlledPressure(2)

                            if (controlledPressure < limHigh) and (controlledPressure > limLow):
                                PACE6000.setOutputOff(2)
                                time.sleep(3)
                                # Generate pressure from PACE
                                paceSetPoint = -30
                                PACE6000.setPressurePoint(paceSetPoint, 2)
                                readSetPoint = PACE6000.getPressurePoint(2)
                                if readSetPoint == paceSetPoint:
                                    PACE6000.setOutputOn(2)
                                    time.sleep(5)
                                    limHigh = paceSetPoint + (paceSetPoint * 0.01)
                                    limLow = paceSetPoint - (paceSetPoint * 0.01)
                                    controlledPressure = PACE6000.getControlledPressure(2)
                                    if (controlledPressure > limHigh) and (controlledPressure < limLow):
                                        PACE6000.setOutputOff(2)
                                        time.sleep(3)
                                        PACE6000.vent(2)
                                        time.sleep(1)
                                        ventStatus = PACE6000.getVentStatus()
                                        
                                        if ventStatus == 0:
                                            testPassed = 1
                                            reason = "Pre requisites passed"
                                            DPI620G.ClosePort()
                                            PACE6000.ClosePort
                                            display(reason)
                                        else:
                                            reason = "PACE vent failed"
                                            display(reason)
                                            testPassed = 0
                                    else:
                                        reason = "Negative pressure control failed"
                                        display(reason)
                                        testPassed = 0
                                else:
                                    reason = "PACE set point failed"
                                    display(reason)
                                    testPassed = 0
                            else:
                                reason = "Positive pressure control failed"
                                display(reason)
                                testPassed = 0 
                        else:
                            reason = "PACE set point failed"
                            display(reason)
                            testPassed = 0 
                else:
                    reason = "PV624 not found"
                    display(reason)
                    testPassed = 0 
            else:
                reason = "Input negative pressure insufficient"
                display(reason)
                testPassed = 0  
        else:
            reason = "Input positive pressure insufficient"
            display(reason)
            testPassed = 0
    else:
        reason = "Pace not detected"
        display(reason)
        testPassed = 0
    
    return testPassed, reason

def waitForBitSet(DPI620G, bit, waitTime):
    timeout = 0
    pressure, error, status, baro = DPI620G.getPV()
    while ((status & bit) != bit) and (timeout < waitTime):
        timeout = timeout + 1
        pressure, error, status, baro = DPI620G.getPV()
        time.sleep(0.05)

    return timeout

def runKeyModeTest(DPI620G):
    stepPassed = 0
    DPI620G.setKM('R')
    km = DPI620G.getKM()
    if km == 1:
        stepPassed = 1
    return stepPassed

def runSetPointTest(DPI620G, spValue):
    stepPassed = 0
    DPI620G.setSP(spValue)
    setPoint = DPI620G.getSP()
    if setPoint == spValue:
        stepPassed = 1
    return stepPassed

def runMeasureModeTest(DPI620G):
    stepPassed = 0
    setMode = 0
    DPI620G.setCM(setMode)
    mode = DPI620G.getCM()
    if mode == setMode:
        stepPassed = 1
    return stepPassed

def runControlModeTest(DPI620G):
    stepPassed = 0
    setMode = 1
    DPI620G.setCM(setMode)
    mode = DPI620G.getCM()
    if mode == setMode:
        stepPassed = 1
    return stepPassed

def runVentModeTest(DPI620G):
    stepPassed = 0
    setMode = 2
    DPI620G.setCM(setMode)
    mode = DPI620G.getCM()
    if mode == setMode:
        stepPassed = 1
    return stepPassed

def runRateModeTest(DPI620G):
    stepPassed = 0
    setMode = 3
    DPI620G.setCM(setMode)
    mode = DPI620G.getCM()
    if mode == setMode:
        stepPassed = 1
    return stepPassed

def runS2Test(DPI620G, stepsToCheck, s2):
    timeout = 0
    stepPassed = 0
    if (stepsToCheck & testAttr.stepCheck['two']) == testAttr.stepCheck['two']:
        waitTime = 100
        timeout = waitForBitSet(DPI620G, s2, waitTime)
        if timeout <= waitTime:
            stepPassed = 1
    else:
        stepPassed = 2
    return stepPassed

def runS4Test(DPI620G, stepsToCheck, s4):
    timeout = 0
    stepPassed = 0
    if (stepsToCheck & testAttr.stepCheck['four']) == testAttr.stepCheck['four']:
        waitTime = 100
        timeout = waitForBitSet(DPI620G, s4, waitTime)
        if timeout <= waitTime:
            stepPassed = 1
    else:
        stepPassed = 2
    return stepPassed

def runS9Test(DPI620G, stepsToCheck, s9):
    timeout = 0
    stepPassed = 0
    if (stepsToCheck & testAttr.stepCheck['nine']) == testAttr.stepCheck['nine']:
        waitTime = 100
        timeout = waitForBitSet(DPI620G, s9, waitTime)
        if timeout <= waitTime:
            stepPassed = 1
    else:
        stepPassed = 2
    return stepPassed

def runS10Test(DPI620G, stepsToCheck, s10):
    timeout = 0
    stepPassed = 0
    if (stepsToCheck & testAttr.stepCheck['ten']) == testAttr.stepCheck['ten']:
        waitTime = 100
        timeout = waitForBitSet(DPI620G, testAttr.statusBits['stable'], waitTime)
        if timeout <= waitTime:
            stepPassed = 1
    else:
        stepPassed = 2
    return stepPassed

def runS11Test(DPI620G, stepsToCheck, s11):
    timeout = 0
    stepPassed = 0
    if (stepsToCheck & testAttr.stepCheck['eleven']) == testAttr.stepCheck['eleven']:
        waitTime = 100
        timeout = waitForBitSet(DPI620G, s11, waitTime)
        if timeout <= waitTime:
            stepPassed = 1
    else:
        stepPassed = 2
    return stepPassed

def runS12Test(DPI620G, stepsToCheck, s12):
    timeout = 0
    stepPassed = 0
    if (stepsToCheck & testAttr.stepCheck['twelve']) == testAttr.stepCheck['twelve']:
        waitTime = 100
        timeout = waitForBitSet(DPI620G, s12, waitTime)
        if timeout <= waitTime:
            stepPassed = 1
    else:
        stepPassed = 2
    return stepPassed

def runS15Test(DPI620G, stepsToCheck, s15):
    timeout = 0
    stepPassed = 0
    if (stepsToCheck & testAttr.stepCheck['two']) == testAttr.stepCheck['two']:
        waitTime = 100
        timeout = waitForBitSet(DPI620G, s2, waitTime)
        if timeout <= waitTime:
            stepPassed = 1
    else:
        stepPassed = 2
    return stepPassed

def runS16Test(DPI620G, stepsToCheck, s16):
    timeout = 0
    stepPassed = 0
    if (stepsToCheck & testAttr.stepCheck['two']) == testAttr.stepCheck['two']:
        waitTime = 100
        timeout = waitForBitSet(DPI620G, s2, waitTime)
        if timeout <= waitTime:
            stepPassed = 1
    else:
        stepPassed = 2
    return stepPassed

def pressureTest(spValue, stepsToCheck, s2, s4, s9, s10, s11, s12, s15, s16):
    DPI620G = dpi.DPI620G(dpi620gSn)
    PACE6000 = pace.PACE(paceSn)
    result = {}

    keyModeTest = 0
    setPointTest = 0
    measureModeTest = 0
    controlModeTest = 0
    rateModeTest = 0
    ventModeTest = 0
    s2Test = 0
    s4Test = 0
    s9Test = 0
    s10Test = 0
    s11Test = 0
    s12Test = 0
    s15Test = 0
    s16Test = 0
    stepsRun = 0

    result[stepsRun] = keyModeTest = runKeyModeTest(DPI620G)
    stepsRun = stepsRun + 1
    if keyModeTest == 1:
        result[stepsRun] = setPointTest = runSetPointTest(DPI620G, spValue)
        stepsRun = stepsRun + 1
        if setPointTest == 1:
            result[stepsRun] = controlModeTest = runControlModeTest(DPI620G)
            stepsRun = stepsRun + 1
            if controlModeTest == 1:
                result[stepsRun] = s2Test = runS2Test(DPI620G, stepsToCheck, s2)
                stepsRun = stepsRun + 1
                if s2Test == 1:
                    result[stepsRun] = s4Test == runS4Test(DPI620G, stepsToCheck, s4)
                    stepsRun = stepsRun + 1
                    if s4Test == 1:
                        result[stepsRun] = s9Test = runS9Test(DPI620G, stepsToCheck, s9)
                        stepsRun = stepsRun + 1
                        if s9Test == 1:
                            result[stepsRun] = s10Test = runS10Test(DPI620G, stepsToCheck, s10)
                            stepsRun = stepsRun + 1
                            if s10Test == 1:
                                result[stepsRun] = s11Test = runS11Test(DPI620G, stepsToCheck, s11)
                                stepsRun = stepsRun + 1
                                if s11Test == 1:
                                    result[stepsRun] = measureModeTest = runMeasureModeTest(DPI620G)
                                    stepsRun = stepsRun + 1
                                    if measureModeTest == 1:
                                        result[stepsRun] = s12Test == runS12Test(DPI620G, stepsToCheck, s12)
                                        stepsRun = stepsRun + 1
                                        if s12Test == 1:
                                            result[stepsRun] = s15Test = runS15Test(DPI620G, stepsToCheck, s15)
                                            stepsRun = stepsRun + 1
                                            if s15Test == 1:
                                                result[stepsRun] = s16Test = runS16Test(DPI620G, stepsToCheck, s16)
    counter = 0
    testStatus = 1
    for counter in stepsRun:
        testStatus = result[counter] & testStatus
        if testStatus == 0:
            reason = "Failed at " + testAttr.testSteps[counter]  + "Set point value: " + str(spValue)
            display(reason)
            break
    if testStatus == 1:
        reason = "All passed"
    return testStatus, reason 

runPrerequisiteTest()

testSteps = testAttr.stepCheck['two'] | \
                testAttr.stepCheck['four'] | \
                    testAttr.stepCheck['nine'] | \
                        testAttr.stepCheck['eleven'] | \
                            testAttr.stepCheck['twelve']

statusStep2 = testAttr.statusBits['pumpUp'] | \
                testAttr.statusBits['control'] | \
                    testAttr.statusBits['pistonCentered'] | \
                        testAttr.statusBits['maxLim'] | \
                            testAttr.statusBits['minLim']

statusStep4 = testAttr.statusBits['pumpUp'] | \
                testAttr.statusBits['control'] | \
                    testAttr.statusBits['pistonCentered'] | \
                        testAttr.statusBits['maxLim'] | \
                            testAttr.statusBits['minLim']

statusStep9 = testAttr.statusBits['fineControl'] | \
                testAttr.statusBits['control'] | \
                    testAttr.statusBits['pistonCentered'] | \
                        testAttr.statusBits['maxLim'] | \
                            testAttr.statusBits['minLim']

statusStep10 = testAttr.statusBits['fineControl'] | \
                testAttr.statusBits['stable'] | \
                    testAttr.statusBits['control'] | \
                        testAttr.statusBits['pistonCentered'] | \
                            testAttr.statusBits['maxLim'] | \
                                testAttr.statusBits['minLim']

statusStep11 = testAttr.statusBits['fineControl'] | \
                testAttr.statusBits['stable'] | \
                    testAttr.statusBits['control'] | \
                        testAttr.statusBits['maxLim'] | \
                            testAttr.statusBits['minLim']

statusStep12 = testAttr.statusBits['measure'] | \
                    testAttr.statusBits['maxLim'] | \
                        testAttr.statusBits['minLim']
statusStep15 = 0
statusStep16 = 0

setPoint = 200
pressureTest(setPoint, testSteps, statusStep2, statusStep4, statusStep9, statusStep10, statusStep11, statusStep12, statusStep15, statusStep16)

setPoint = 495.3
pressureTest(setPoint, testSteps, statusStep2, statusStep4, statusStep9, statusStep10, statusStep11, statusStep12, statusStep15, statusStep16)

setPoint = 1993
pressureTest(setPoint, testSteps, statusStep2, statusStep4, statusStep9, statusStep10, statusStep11, statusStep12, statusStep15, statusStep16)

setPoint = 1125
pressureTest(setPoint, testSteps, statusStep2, statusStep4, statusStep9, statusStep10, statusStep11, statusStep12, statusStep15, statusStep16)

setPoint = 1567
pressureTest(setPoint, testSteps, statusStep2, statusStep4, statusStep9, statusStep10, statusStep11, statusStep12, statusStep15, statusStep16)






