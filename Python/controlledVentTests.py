import dpi620gLib as dpi
import dpiAttributes as dpiAttr
import time

dpi620gSn = ['FTBTBC9KA']
connectionType = dpiAttr.connectionOwi
setPointWaitTime = 4

def cvTests():
    DPI620G = dpi.DPI620G(dpi620gSn, connectionType)

    DPI620G.setKM('R')

    mode = DPI620G.getKM()
    setPoint = 16000
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
                        
                        while (status & 0x01) != 0x01:
                            pressure, error, status, baro = DPI620G.getPV()
                            print(pressure, error, status, baro)

                        print('pump')
                        counter = 0
                        while pressure < (setPoint - setPoint * 0.01):
                            pressure, error, status, baro = DPI620G.getPV()
                            print(pressure, error, status, baro, counter)
                            counter = counter + 1

                        print('stable')

                        time.sleep(4)
                        print("Starting controlled vent tests")

                        while setPoint > 0:
                            setPoint = setPoint - 1000
                            print(setPoint)
                            DPI620G.setCM(0)
                            cm = DPI620G.getCM()
                            if cm == 0:
                                DPI620G.setSP(setPoint)
                                sp = DPI620G.getSP()
                                if sp == setPoint:
                                    DPI620G.setCM(1)
                                    cm = DPI620G.getCM()
                                    if cm == 1:
                                        next = 0
                                        while next == 0:
                                            status = 0
                                            pressure, error, status, baro = DPI620G.getPV()
                                            while ((status & 0x10) != 0x10):
                                                pressure, error, status, baro = DPI620G.getPV()
                                            print("stable")

                                            status = 0
                                            pressure, error, status, baro = DPI620G.getPV()
                                            while ((status & 0x1000) == 0x1000):
                                                pressure, error, status, baro = DPI620G.getPV()
                                            print("Out of Center")
                                            timeRemaining = setPointWaitTime
                                            while timeRemaining > 0:
                                                time.sleep(1)
                                                timeRemaining = timeRemaining - 1
                                                print(timeRemaining)
                                            next = 1

                        DPI620G.setCM(2)
                        print("Complete")




cvTests()



