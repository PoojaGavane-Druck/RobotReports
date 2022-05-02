import dpi620gLibDuci as dpi
import pv624Lib as pv624
from datetime import datetime
import time
import csv
import random

sampleTime = 0.07
totalTime = 5 # mins
totalTimeSec = totalTime * 60


def acquirePressure():
    try:
        count = 1
        runTime = 0
        DPI620G = dpi.DPI620G()
        dataFile = 'PressureData_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
        with open(dataFile,'w',newline='') as f:
            csvFile = csv.writer(f,delimiter = ',')
            while runTime < totalTimeSec:
                pressure, error, status, baro = DPI620G.getPV()
                result = [count, pressure, baro]
                csvFile.writerow(result) 
                count = count + 1
                time.sleep(sampleTime)
                runTime = runTime + sampleTime
                print(runTime, totalTimeSec)

    finally:
        f.close()
        DPI620G.closePort()

acquirePressure()