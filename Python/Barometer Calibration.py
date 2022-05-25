import dpi620gLib as dpi
import time
from datetime import datetime

dpi620gSn = ['FTBTA7ISA']
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
    DPI620G = dpi.DPI620G(dpi620gSn)
    # Set KM = R
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
            msg = DPI620G.setPP("123")
            # Set CT = 0,0
            display("Setting user cal mode")
            msg = DPI620G.setCT("0,0")
            # Get CN, cn should be 2,2
            display("Reading number of cal points")
            minPoints, maxPoints = DPI620G.getCN("1")
            myStr = "Min points = " + str(minPoints) + ", Max points = " + str(maxPoints)
            display(myStr)
            # Set CS
            calPoints = 0
            while calPoints < maxPoints:
                myStr = "Calibration point no. " + str(calPoints + 1)
                display(myStr)
                display("Start sampling for cal point")
                msg = DPI620G.setCS()
                # Get CS, read until CS = 0
                csVal = 100
                while csVal != 0:
                    csVal = DPI620G.getCS()
                    display(str(csVal))
                # Set CP1 = Applied value 
                calPoint = input("Enter cal point: ")
                appVal = input("Enter barometer reading: ")
                myStr = "Cal point number = " + str(calPoint) + ", Pressure reading = " + str(appVal)
                display(myStr)
                DPI620G.setCP(calPoint, appVal)
                time.sleep(0.3)
                calPoints = calPoints + 1
                if calPoints == maxPoints:
                    acceptCal = input("Accept calibration Y/N?: ")
                else:
                    a = input("Change set point")
                    print(a)
            # Set CA - accept calibration
            display("Accept calibration - CA")
            DPI620G.setCA()

calBaro()