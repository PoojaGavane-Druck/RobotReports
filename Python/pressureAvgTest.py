from tracemalloc import start
import pv624Lib as pvComms
import dpi620gLib as dpi
import dpiAttributes as dpiAttr
import time
from datetime import datetime
import random
import csv

dpi620gSn = ['FTBTBC9KA']

def pressureTest():

    samplesStr = input("Hi Ashwini, how many samples: ")
    samples = 0
    count = 0
    try:
        samples = int(samplesStr)
    except:
        print("Ashwini, please enter a number. Samples cannot be non numerical :)")
    
    if samples > 0:
        DPI620G = dpi.DPI620G(dpi620gSn)

        DPI620G.setKM('R')
        DPI620G.setPT('1')
        fileName = 'PA_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
        with open(fileName,'w',newline='') as f:
            csvFile = csv.writer(f,delimiter = ',')
            result = ['Count', 'Time', 'Actual Pressure', 'Average Pressure']
            csvFile.writerow(result) 
            startTime = time.time()
            while count < samples:
                pressure, pressureAvg = DPI620G.getPA()
                count = count + 1
                endTime = time.time()
                elapsedTime = endTime - startTime
                result = [count, round(elapsedTime, 2), round(pressure, 5), round(pressureAvg, 5)]
                csvFile.writerow(result) 
                print(result)
    else:
        print("Why do you want 0 samples, Ashwini? Why not enter a larger number :)")

pressureTest()