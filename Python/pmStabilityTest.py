import dpi620gLib as dpi
import paceLib as pace
import csv
from datetime import datetime
import time
import dpiAttributes as dpiAttr

dpi620gSn = ['FTBTA7ISA']
paceSn = ['AC0128TUA']
samples = 200
samplingTime = 0.05

pressureStable = 0.5

def acquire():
    PACE = pace.PACE(paceSn)
    DPI620G = dpi.DPI620G(dpi620gSn)

    pressureUpperLimit = 0.0
    pressureLowerLimit = 0.0


    setPressurePace = input("Hi Dyno, enter pressure in mbar G: ")
    DPI620G.setKM('R')
    DPI620G.setPressureType(dpiAttr.pressureType['Gauge'])
    time.sleep(0.1)
    DPI620G.setControlMode(0)
    time.sleep(0.1)

    minP, maxP, Type = DPI620G.getSensorInfo()
    sensorSN = DPI620G.getSN('1')
    sensorDk = DPI620G.getDK('1,0')

    if(sensorDk == 'DK0472'):
        sensorName = "PM620T"
    else:
        sensorName = "PM620"

    PACE.setPressureMode()
    PACE.setPressurePoint(setPressurePace)
    setPressurePace = float(setPressurePace)
    # wait for pressure to be set
    pressureUpperLimit = setPressurePace + (setPressurePace * 0.5 / 100.0)
    pressureLowerLimit = setPressurePace - (setPressurePace * 0.5 / 100.0)
    print(pressureUpperLimit, pressureLowerLimit)

    PACE.setOutputOn()
    paceReadPressure = PACE.getControlledPressure()
    print(paceReadPressure)

    while (paceReadPressure < pressureLowerLimit) or (paceReadPressure > pressureUpperLimit):
        print(paceReadPressure, pressureUpperLimit, pressureLowerLimit)
        paceReadPressure = PACE.getControlledPressure()
        time.sleep(0.05)

    dataFile = sensorName + "_" + sensorSN + '_' + str(maxP) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(dataFile,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        result = ['Count', 'Pace pressure', 'PM Pressure', 'Pressure Diff', 'Baro pressure']
        csvFile.writerow(result) 
        count = 1
        while count < samples:
            pressurePace = PACE.getControlledPressure()
            pressurePm, error, status, baro = DPI620G.getPVInfo()

            result = [count, pressurePace, pressurePm, (pressurePace - pressurePm), baro]
            print(result)
            csvFile.writerow(result) 
            time.sleep(samplingTime)
            count = count + 1

        f.close()
        PACE.setOutputOff()
        print("Done dyno")

acquire()