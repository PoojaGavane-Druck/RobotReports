import time
import paceLib as pace
import dpi620gLib as dpi
from datetime import datetime
import dpiAttributes as dpiAttr
import pressureTestAttributes as testAttr

printIt = 1
writeLog = 1

paceSn = ['AC0128TUA']
dpi620gSn = ['FTBTBC9KA']
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

                        if km == 'R':
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
                            time.sleep(5)
                            limHigh = paceSetPoint + (paceSetPoint * 0.01)
                            limLow = paceSetPoint - (paceSetPoint * 0.01)
                            controlledPressure = PACE6000.getControlledPressure(2)

                            if (controlledPressure < limHigh) and (controlledPressure > limLow):
                                PACE6000.setOutputOff(2)
                                time.sleep(5)
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
                                        time.sleep(5)
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


def runPressureTest(spValue, stepsToCheck, s2, s4, s9, s10, s11, s12, s15, s16):
    DPI620G = dpi.DPI620G(dpi620gSn)
    PACE6000 = pace.PACE(paceSn)

    timeout = 0

    DPI620G.setKM('R')
    km = DPI620G.getKM()

    if km == 'R':
        DPI620G.setSP(spValue)
        readSp = DPI620G.getSP()

        if readSp == spValue:
            DPI620G.setCM(1)
            mode = DPI620G.getCM()
            if mode == 1:
                if stepsToCheck & testAttr.stepCheck['two'] == testAttr.stepCheck['two']:      
                    status = 0
                    while ((status & s2) != s2) or (timeout > testAttr.testFailedTimeout):
                        pressure, error, status, baro = DPI620G.getPV()
                        timeout = timeout + 1

                    if timeout > testAttr.testFailedTimeout:
                        reason = "Failed at step S2"
                        display("reason")
                        testPassed = 0
                    else:
                        # Continue test
                        timeout = 0
                        status = 0
                        if stepsToCheck & testAttr.stepCheck['four'] == testAttr.stepCheck['four']:      
                            while ((status & s4) != s4) or (timeout > testAttr.testFailedTimeout):
                                pressure, error, status, baro = DPI620G.getPV()
                                timeout = timeout + 1

                            if timeout > testAttr.testFailedTimeout:
                                reason = "Failed at step S4"
                                display("reason")
                                testPassed = 0
            else:
                reason = "Mode could not be set on PV624"
                display("reason")
                testPassed = 0
        else:
            reason = "Set point could not be set on PV624: " + str(spValue)
            display(reason)
            testPassed = 0
    else:
        reason = "Remote mode not set on PV624"
        display(reason)
        testPassed = 0       

#runPrerequisiteTest()

setPoint = 200
testSteps = testAttr.stepCheck['two'] | \
                testAttr.stepCheck['four'] | \
                    testAttr.stepCheck['nine'] | \
                        testAttr.stepCheck['ten'] | \
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

statusStep12 = 0
statusStep15 = 0
statusStep16 = 0

runPressureTest(setPoint, testSteps, statusStep2, statusStep4, statusStep9, statusStep10, statusStep11, statusStep12, statusStep15, statusStep16)






