from tracemalloc import start
import pv624Lib as pvComms
import dpi620gLib as dpi
import dpiAttributes as dpiAttr
import time
from datetime import datetime
import random
import csv

dpi620gSn = ['FTBTBC9KA']
filterWindow = 7
pressureArr = [0,0,0,0,0]
pressureArrAvg = [0,0,0,0,0]
pressureIndex = 0

def arrangeAscending(arr, window):
    indexOne = 0
    indexTwo = 0

    for indexOne in range(0, window, 1):
        for indexTwo in range(indexOne + 1, window, 1):
            if arr[indexOne] > arr[indexTwo]:
                temp = arr[indexOne]
                arr[indexOne] = arr[indexTwo]
                arr[indexTwo] = temp

    return arr

def medianFilter(arr, window):
    arr = arrangeAscending(arr, window)
    median = arr[3]
    return median

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
            result = ['Count', 'Time', 'Actual Pressure', 'Median Actual', 'Average Pressure', 'Median Average']
            csvFile.writerow(result) 
            startTime = time.time()
            while count < samples:
                pressure, pressureAvg = DPI620G.getPA()
                pressureArr[pressureIndex] = pressure
                pressureArrAvg[pressureIndex] = pressure
                pressureIndex = pressureIndex + 1
                if pressureIndex >= (filterWindow - 1):
                    pressureIndex = 0
                    
                filteredPressure = medianFilter(pressureArr, filterWindow)
                filteredPressureAvg = medianFilter(pressureArrAvg, filterWindow)

                count = count + 1
                endTime = time.time()
                elapsedTime = endTime - startTime
                result = [count, round(elapsedTime, 2), round(pressure, 5), round(filteredPressure, 5), \
                                                round(pressureAvg, 5), round(filteredPressureAvg, 5)]
                csvFile.writerow(result) 
                print(result)
    else:
        print("Why do you want 0 samples, Ashwini? Why not enter a larger number :)")

pressureTest()