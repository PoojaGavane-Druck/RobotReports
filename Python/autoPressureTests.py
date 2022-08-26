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


def runPressureTest(spValue):
    DPI620G = dpi.DPI620G(dpi620gSn)
    PACE6000 = pace.PACE(paceSn)

    DPI620G.setKM('R')
    km = DPI620G.getKM()

    if km == 'R':
        DPI620G.setSP(spValue)
        readSp = DPI620G.getSP()

        if readSp == spValue:
            DPI620G.setCM(1)
            mode = DPI620G.getCM()
            if mode == 1:
                print("")
            else:
                reason = "Mode could not be set on PV624"
                display("reason")
        else:
            reason = "Set point could not be set on PV624: " + str(spValue)
            display(reason)
    else:
        reason = "Remote mode not set on PV624"
        display(reason)
        testPassed = 0       

runPrerequisiteTest()

runPressureTest(200, stepCheck, statusStep2, statusStep4, statusStep9, statusStep10, statusStep11, statusStep12, statusStep15, statusStep16)






