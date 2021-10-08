# -*- coding: utf-8 -*-
"""
Created on Thu Oct  7 09:44:42 2021

@author: 212596572
"""
import dpi620gLib as dpi
from datetime import datetime
import time
import csv
import random

printIt = 1

dateNow = "05/05/2021"
timeNow = "14:15:16"
sn = "65816548"
sp = "15.224"

def writeStr(file, value, printState):
    file.writerow(value + "\n")
    if(printState == 1):
        print(value)    

def parseStatus(status):
    pumpUp = status & 0x01     
    count = 1     
    pumpDown = (status & 0x00000002) >> count
    count = count + 1
    control  = (status & 0x00000004) >> count
    count = count + 1
    venting = (status & 0x00000008) >> count
    count = count + 1
    stable  = (status & 0x00000010) >> count
    count = count + 1
    vented = (status & 0x00000020) >> count
    count = count + 1
    excessLeak = (status & 0x00000040) >> count
    count = count + 1
    excessVolume = (status & 0x00000080) >> count
    count = count + 1
    overPressure = (status & 0x00000100) >> count
    count = count + 1
    excessOffset = (status & 0x00000200) >> count
    count = count + 1
    measure = (status & 0x00000400) >> count
    count = count + 1
    fineControl = (status & 0x00000800) >> count
    count = count + 1
    pistonCentered = (status & 0x00001000) >> count
    count = count + 1
    centering = (status & 0x00002000) >> count
    count = count + 1
    controlledVent = (status & 0x00004000) >> count
    count = count + 1
    centeringVent = (status & 0x00008000) >> count
    count = count + 1
    rangeExceeded = (status & 0x00010000) >> count
    count = count + 1
    ccError = (status & 0x00020000) >> count
    count = count + 1
    ventDirectionUp = (status & 0x00040000) >> count
    count = count + 1
    ventDirectionDown = (status & 0x00080000) >> count
    result = [pumpUp, pumpDown, control, venting, stable, vented, excessLeak, \
              excessVolume, overPressure, excessOffset, measure, fineControl, \
            pistonCentered, centering, controlledVent, centeringVent, \
                rangeExceeded, ccError, ventDirectionUp, ventDirectionDown]
        
    return result    

def controllerTest(samples, sampleTime):
    DPI620G = dpi.DPI620G()
    
    DPI620G.setKM('R')
    DPI620G.getKM()
    time.sleep(1)
    
    mainCounter = 0
    
    controlMode = 0
    setPressureType = 0
    setPoint = 0    

    result = ['Counts', 'Set Mode', 'Set Pressure Type', 'Set set Point', \
              'Set Point', 'Pressure', 'Error', 'Pump Up', \
              'Pump Down', 'Control', 'Venting', 'Stable', 'Vented', \
                 'Excess Leak', 'Excess Volume', 'Over Pressure', 'Excess Offset', 'Measure', \
                      'Fine Control', 'Piston Centered', 'Centering', 'Controlled Vent', \
                          'Centering', 'Controlled Vent', 'Centering Vent', 'Range Exceeded', \
                              'CC Error', 'Vent Dir Up', 'Vent Dir Down', 'Pressure Type', 'mode']
    
    dataFile = 'CONTROLLER_TEST__' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(dataFile,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        count = 0
        csvFile.writerow(result)
        print(result)
        while True:
            while count < samples:
                pressure, error, status = DPI620G.getPVInfo()
                pressureType = DPI620G.getPressureType()
                mode = DPI620G.getControlMode()
                sp = DPI620G.getSetPoint()
                time.sleep(0.1)
                statusResult = parseStatus(status)
                result = [mainCounter, controlMode, setPressureType, setPoint, sp, pressure, error, statusResult, pressureType, mode]
                count = count + 1
                mainCounter = mainCounter + 1
                csvFile.writerow(result)
                print(result)
            
            count = 0
            controlMode = random.randint(0, 2)
            DPI620G.setControlMode(controlMode)
            setPressureType = random.randint(0, 1)
            DPI620G.setPressureType(setPressureType)
            
            if pressureType == 0:
                setPoint = round(random.uniform(-900, 5000), 3)
            else:
                setPoint = round(random.uniform(0, 6000))
            DPI620G.setSetPoint(setPoint)
            
    dataFile.close()
    DPI620G.closePort()
    
controllerTest(50, 1000)    