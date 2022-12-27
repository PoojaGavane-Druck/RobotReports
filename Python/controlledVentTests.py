import dpi620gLib as dpi
import dpiAttributes as dpiAttr
import time
from datetime import datetime
import csv

dpi620gSn = ['FTBTBC9KA']
connectionType = dpiAttr.connectionOwi
setPointWaitTime = 1

def cvTests(pt, startSp, spWaitTime):
    fileName = 'VENT_SP_TESTS_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    f = open(fileName,'w',newline='')
    csvFile = csv.writer(f,delimiter = ',')
    result = ['Set Point', 'Time Taken']
    csvFile.writerow(result) 
    DPI620G = dpi.DPI620G(dpi620gSn, connectionType)

    DPI620G.setKM('R')
    model = DPI620G.getRI()
    version = DPI620G.getRV(0)
    print(model, " ", version)
    mode = DPI620G.getKM()
    setPoint = startSp
    if mode == 1:
            DPI620G.setCM(0)
            cm = DPI620G.getCM()
            if cm == 0:
                DPI620G.setSP(setPoint)
                sp = DPI620G.getSP()
                if sp == setPoint:
                    DPI620G.setCM(1)
                    cm = DPI620G.getCM()
                    if cm == 1:
                        pressure, error, status, baro = DPI620G.getPV()
                        
                        if pt == 'P':
                            while ((status & 0x01) != 0x01):
                                pressure, error, status, baro = DPI620G.getPV()
                                print(pressure, error, status, baro)

                        if pt == 'V':
                            while ((status & 0x02) != 0x02):
                                pressure, error, status, baro = DPI620G.getPV()
                                print(pressure, error, status, baro)

                        print('pump')
                        counter = 0

                        status = 0
                        while ((status & 0x10) != 0x10):
                            pressure, error, status, baro = DPI620G.getPV()
                            print(pressure, error, status, baro, counter)
                            counter = counter + 1

                        print('stable')
                        time.sleep(4)
                        print("Starting controlled vent tests")

                        while ((pt == 'P') and (setPoint > 0)) or ((pt == 'V') and (setPoint < 0)):
                            print(setPoint)
                            DPI620G.setCM(0)
                            cm = DPI620G.getCM()
                            if cm == 0:
                                DPI620G.setSP(setPoint)
                                sp = DPI620G.getSP()
                                if sp == setPoint:
                                    DPI620G.setCM(1)
                                    cm = DPI620G.getCM()
                                    controlModeStartTime = time.time()
                                    if cm == 1:
                                        next = 0
                                        while next == 0:
                                            status = 0
                                            pressure, error, status, baro = DPI620G.getPV()
                                            while ((status & 0x10) != 0x10):
                                                pressure, error, status, baro = DPI620G.getPV()
                                            print("stable")
                                            controlModeEndTime = time.time()
                                            stabilityTime = controlModeEndTime - controlModeStartTime
                                            status = 0
                                            '''pressure, error, status, baro = DPI620G.getPV()
                                            while ((status & 0x1000) == 0x1000):
                                                pressure, error, status, baro = DPI620G.getPV()
                                            print("Out of Center")
                                            now = time.time()
                                            '''
                                            
                                            timeRemaining = spWaitTime
                                            while timeRemaining > 0:
                                                time.sleep(1)
                                                timeRemaining = timeRemaining - 1
                                                print(timeRemaining)
                                            next = 1

                                            result = [setPoint, stabilityTime]
                                            csvFile.writerow(result) 
                                            print(result)

                                            if pt == 'P':
                                                setPoint = setPoint - 1000
                                            else:
                                                setPoint = setPoint + 50


                        DPI620G.setCM(2)
                        print("Complete")




pt = input("Enter P for pressure, or V for Vacuum: ")
startSp = float(input("Enter starting set point: "))
spWaitTime = int(input("Enter wait time for every set point in seconds: "))

if pt == 'P':
    if startSp < 0:
        print("ERROR: Set point is lower than 0 bar g")
    else:
        cvTests(pt, startSp, spWaitTime)

elif pt == 'V':
    if startSp > 0:
        print("ERROR: Set point is higher than 0 bar g")
    else:
        cvTests(pt, startSp, spWaitTime) 
else:
    print("ERROR: Invalid pressure type")    




