
import dpi620gLib as dpi
import paceLib as pace
import time
from datetime import datetime
import csv

soCoeff = 0
dpi620gSn = ['FTBTBC9KA']
dataFile = 'INPUT_IIR_' + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
outputDataFile = 'FILTER_IIR_' + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'

def iirFilter(coeff, value, oldValue):
    outputValue = ((1 - coeff) * value) + (coeff  * oldValue)
    oldValue = outputValue
    return outputValue, oldValue

def getReadings(iterations):
    DPI620G = dpi.DPI620G(dpi620gSn)
    
    DPI620G.setKM('R')
    time.sleep(0.5)
    DPI620G.setSP(2000)
    DPI620G.setCM(1)
    time.sleep(0.5)
    myFile = open(dataFile, 'a', newline='')
    csvFile = csv.writer(myFile, delimiter=',')
    result = ['Pressure']
    csvFile.writerow(result)
    myFile.close()

    iterations = int(iterations)
    count = 0
    for n in range(0, iterations):
        startTime = time.time()
        myFile = open(dataFile, 'a', newline='')
        csvFile = csv.writer(myFile, delimiter=',')
        tic = datetime.now()
        pressure, error, status, baro = DPI620G.getPV()
        endTime = time.time()
        elapsedTime = endTime - startTime
        count = count + 1
        result = [pressure]
        print(result)
        csvFile.writerow(result)
        myFile.close()

def filterReadings(coeff):
    myFile = open(dataFile, 'r', newline='')
    csvFile = csv.reader(myFile)
    counter = 0
    oldPressure = 0
    outputDataFile = 'FILTER_IIR_' + str(round(coeff, 1)) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    for lines in csvFile:
        
        if counter == 0:
            myFile = open(outputDataFile, 'a', newline='')
            csvFile = csv.writer(myFile, delimiter=',')
            result = ['Pressure', 'Filtered Pressure']
            csvFile.writerow(result)
            myFile.close()

        if counter != 0:
            value = lines
            pressure = float(value[0])
            myFile = open(outputDataFile, 'a', newline='')
            csvFile = csv.writer(myFile, delimiter=',')
            filteredPressure, oldPressure = iirFilter(coeff, pressure, oldPressure)
            result = [pressure, filteredPressure]
            print(result)
            csvFile.writerow(result)
            myFile.close()
        
        counter = counter + 1

getReadings(500)
coeff = 0
for n in range (0, 10):
    filterReadings(coeff)
    coeff = coeff + 0.1



