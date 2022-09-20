import dpi620gLib as dpi
import paceLib as pace
import time
from datetime import datetime

paceSn = ['AC0128TUA']
dpi620gSn = ['FTBTBC9KA']
fileName = 'BARO_CALIBARTION_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'
printIt = 1
writeLog = 1

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

def calBaro():
    PACE6000 = pace.PACE(paceSn)
    DPI620G = dpi.DPI620G(dpi620gSn)
    # Set KM = R

    expInputPressurePos = 2000.0
    expInputPressureNeg = 500.0

    calPoint1 = 800.0
    calPoint2 = 1100.0

    PACE6000.setPressureMode(2, 'A')
    inpPos = PACE6000.getInputPressurePos(2)
    inpNeg = PACE6000.getInputPressureNeg(2)

    if inpPos > expInputPressurePos:
        display("Input positive pressure passed")
        if inpNeg < expInputPressureNeg:
            display("Input negative pressure passed")
            inputPressureTest = 1
        else:
            myStr = "PACE negative input pressure not sufficient"
            display(myStr)
    else:
        myStr = "PACE positive input pressure not sufficient"
        display(myStr)

    if inputPressureTest == 1:
        display("Setting remote mode")
        msg = DPI620G.setKM('R')
        km = DPI620G.getKM()

        if km == 1:
            display("Remote mode set")
            # Set PT = 2
            msg = DPI620G.setPT("2")
            display("Setting pressure type - Barometer")
            pType = DPI620G.getPT()
            if pType == 2:
                display("Pressure Type set")
                # Set PP = 123
                display("Setting cal pin - 123")
                DPI620G.setPP("123")
                setPin = DPI620G.getPP()
                if setPin == 123:
                    # Set CT = 0,0
                    display("Setting user cal mode")
                    DPI620G.setCT("0,0")
                    # Get CN, cn should be 2,2
                    display("Reading number of cal points")
                    minPoints, maxPoints = DPI620G.getCN("1")
                    if (minPoints == 2) and (maxPoints == 2):
                        myStr = "Min points = " + str(minPoints) + ", Max points = " + str(maxPoints)
                        display(myStr)
                        # Set CS
                        calPoints = 0
                        while calPoints < maxPoints:
                            myStr = "Calibration point no. " + str(calPoints + 1)
                            display(myStr)
                            display("Start sampling for cal point")

                            pacePressure = 0
                            if calPoints == 0:
                                pacePressure = calPoint1
                            elif calPoints == 1:
                                pacePressure = calPoint2

                            PACE6000.setPressurePoint(pacePressure, 2)
                            readPressure = round(PACE6000.getPressurePoint(2), 2)

                            if pacePressure == readPressure:
                                myStr = "Pace set point set at " + str(pacePressure)
                                display(myStr)

                                # Start the output
                                PACE6000.setOutputOn(2)
                                # start reading pressure until pressure becomes stable
                                counter = 0
                                limLow = pacePressure * 0.9995
                                limHigh = pacePressure * 1.0005
                                readPressure = PACE6000.getControlledPressure(2)

                                while ((limLow >= readPressure) or (limHigh <= readPressure)) and (counter < 50):
                                    counter = counter + 1
                                    readPressure = PACE6000.getControlledPressure(2)
                                    time.sleep(0.5)

                                if counter < 50:
                                    counter = 0
                                    stable = 0
                                    # wait until pressure becomes stable or times out
                                    while (counter < 50) and (stable < 10):
                                        counter = counter + 1
                                        readPressure = PACE6000.getControlledPressure(2)
                                        if limLow <= readPressure <= limHigh:
                                            stable = stable + 1
                                        else:
                                            stable = 0
                                        time.sleep(0.5)

                                    if stable >= 10:
                                        stable = 0
                                        DPI620G.setCS()
                                        # Get CS, read until CS = 0
                                        csVal = 100
                                        while csVal != 0:
                                            csVal = DPI620G.getCS()
                                            display(str(csVal))
                                        # Set CP1 = Applied value 
                                        calPoint = calPoints + 1
                                        appVal = PACE6000.getControlledPressure(2)
                                        myStr = "Cal point number = " + str(calPoint) + ", Pressure reading = " + str(appVal)
                                        display(myStr)
                                        DPI620G.setCP(calPoint, appVal)
                                        time.sleep(0.3)
                                        if calPoints == 1:
                                            settingPassed = 1
                                            PACE6000.vent(2)
                                        else:
                                            PACE6000.setOutputOff(2)

                                        calPoints = calPoints + 1
                                        
                                    else:
                                        myStr = "Pressure unstable on PACE failed at cal point " + str(calPoints + 1) + " at pressure " + str(pacePressure)
                                        PACE6000.vent(2)
                                        display(myStr)
                                else:
                                    myStr = "Pressure control on PACE failed at cal point " + str(calPoints + 1) + " at pressure " + str(pacePressure)
                                    PACE6000.vent(2)
                                    display(myStr)

                        if settingPassed == 1:
                            display("Accept calibration - CA")
                            DPI620G.setCA()
                            time.sleep(0.5)
                            off1, off2, off3, off4 = DPI620G.getCA()
                            print(off1, off2, off3, off4)

calBaro()