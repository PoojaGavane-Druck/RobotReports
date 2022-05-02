# -*- coding: utf-8 -*-
"""
Created on Tue Mar 30 13:16:45 2021

@author: 212596572
"""

import pv624Lib as pvComms
import dpi620gLib as dpi
import dpiAttributes as dpiAttr
import time
from datetime import datetime
import random

pv624sn = ['205C324B5431']
dpi620gSn = ['FTBTA7ISA']

printStatus = 1
runMotor = 0
def DataAcqTest(dpi, pv, spType, controlMode, setPoint, iterations, logFile):
    
    setPointTest = 0
    modeTest = 0
    pressureTypeTest = 0
    slowTest = 0
    fastTest = 0
    
    top = 'Pressure Type: '    
    if spType == dpiAttr.pressureType['Gauge']:
        top = top + 'Gauge'
    elif spType == dpiAttr.pressureType['Absolute']:
        top = top + 'Absolute'
    elif spType == dpiAttr.pressureType['Barometer']:
        top = top + 'Barometer'    
    top = top + ', Set Point: ' + str(setPoint) + '-------------------------'
    
    fileWrite(logFile, '\n' + top, 0)

    dpi.setPressureType(spType)
    presType = dpi.getPressureType()
    if spType == presType:
        fileString = '\nPressure Type: '+ str(spType) + str(presType) + ':PASSED'
        fileWrite(logFile, fileString, 0)
        pressureTypeTest = 1
    else:
        fileString = '\nPressure Type: '+ str(spType) + str(presType) + ':FAILED'
        fileWrite(logFile, fileString, 0)
        
    pressure, error, status = dpi.getPVInfo()
    fileString = "\nPressure: " + str(pressure) + " Error: " + str(error) + " Status: " + str(status)
    fileWrite(logFile, fileString, 0)
    
    dpi.setSetPoint(setPoint)        
    sp =  dpi.getSetPoint()
    if sp == setPoint:
        fileString = '\nSet point value: ' + str(sp) + ':PASSED'
        fileWrite(logFile, fileString, 0) 
        setPointTest = 1
    else:
        fileString = '\nSet point value: ' + str(sp) + ':FAILED'
        fileWrite(logFile, fileString, 0)        
        
    dpi.setControlMode(controlMode)
    mode = dpi.getControlMode()       
    
    if mode == controlMode:
        fileString = '\nMode Setting: ' + str(controlMode) + str(mode) + ':PASSED'
        fileWrite(logFile, fileString, 0) 
        modeTest = 1
    else:
        fileString = '\nMode Setting: ' + str(controlMode) + str(mode) + ':FAILED'
        fileWrite(logFile, fileString, 0)  
        
    fileWrite(logFile, '\nTest slow reads for controller data ---------------------', 0)
    count = 0
    passCounts = 0
    
    while count < iterations:
        startTime = time.time()
        pressure, pressureG, baroPressure, sp, setPointType, mode = pv.readAllSlow()
        endTime = time.time()
        elapsedTime = endTime - startTime
        count = count + 1
        result = [count, round(pressure, 3), round(pressureG, 3), round(baroPressure, 3), round(sp, 3), setPointType, mode, round(elapsedTime, 3)]
        result = str(result)        
        if setPointType == spType:
            if mode == controlMode:
                if round(sp, 3) == setPoint:
                    result = result + ' :PASSED'
                    passCounts = passCounts + 1
                else:
                    result = result + ' :FAILED - SETPOINT'
            else:
                result = result + ' :FAILED - CONTROL MODE'
        else:
            result = result + ' :FAILED - SET POINT TYPE'        
        fileString = '\n' + result
        fileWrite(logFile, fileString, 0)  
        
    if passCounts == iterations:
        slowTest = 1

    fileWrite(logFile, '\nTest fast reads for controller data ---------------------', 0)
    count = 0
    passCounts = 0
    while count < iterations:
        startTime = time.time()
        pressure, pressureG, baroPressure, sp, setPointType, mode = pv.readAllFast()
        endTime = time.time()
        elapsedTime = endTime - startTime
        count = count + 1
        result = [count, round(pressure, 3), round(pressureG, 3), round(baroPressure, 3), round(sp, 3), setPointType, mode, round(elapsedTime, 3)]
        result = str(result)
        if setPointType == spType:
            if mode == controlMode:
                if round(sp, 3) == setPoint:
                    result = result + ' :PASSED'
                    passCounts = passCounts + 1
                else:
                    result = result + ' :FAILED - SETPOINT'
            else:
                result = result + ' :FAILED - CONTROL MODE'
        else:
            result = result + ' :FAILED - SET POINT TYPE' 
            
        fileString = '\n' + result
        fileWrite(logFile, fileString, 0)  
    if passCounts == iterations:
        fastTest = 1
     
    testStatus = 0
    if setPointTest == 1:
        if modeTest == 1:
            if pressureTypeTest == 1:
                if slowTest == 1:
                    if fastTest == 1:
                        testStatus = 1
                    else:
                        fileWrite(logFile, '\nFast Read Test: FAILED', 0)
                else:
                    fileWrite(logFile, '\nSlow Read Test: FAILED', 0) 
            else:
                fileWrite(logFile, '\nPressure Type Test: FAILED', 0)
        else:
            fileWrite(logFile, '\nMode Test: FAILED', 0)
    else:
        fileWrite(logFile, '\nSet Point Test: FAILED', 0)      
    
    return testStatus
                        
def fileWrite(logFile, fileString, toprint):
    if toprint == 1:
        print(fileString, end = " ")
    
    logFile.write(fileString)
    
def runTest():
    fileName = 'DPI620G_PV624_Test_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'
    logFile = open(fileName, "w")

    DPI620G = dpi.DPI620G(dpi620gSn)
    pv624 = pvComms.PV624(pv624sn)
    
    print("Connected")
    iterations = 20

    # Test variables
    setPointTest = 0
    modeTest = 0
    ptaTest = 0
    ptgTest = 0
    ptbTest = 0
    statusTest = 0
    
    fileWrite(logFile, '\n*********************************************************************', 1)
    fileWrite(logFile, '\nStarting DPI620G and PV624 TEST', 1)
    fileWrite(logFile, '\n*********************************************************************', 1)
    
    fileWrite(logFile, '\nTesting sensor commands -----------------------------------', 0)
    sensorType = pv624.readSensorType()
    fileString = '\nSensor Type: ' + str(sensorType)
    fileWrite(logFile, fileString, 0)
    pressure = pv624.readFullScalePressure()
    fileString = '\nFull scale pressure: ' + str(pressure)
    fileWrite(logFile, fileString, 0)    
    sleep = 0.25
    
    fileWrite(logFile, '\nTesting valves -------------------------------------------', 1)
    fileWrite(logFile, '\nTesting valve 1 - Open', 0)
    pv624.OpenValve1()    
    time.sleep(sleep)

    fileWrite(logFile, '\nTesting valve 2 - Open', 0)
    pv624.OpenValve2()    
    time.sleep(sleep)
    
    fileWrite(logFile, '\nTesting valve 3 - Open', 0)
    pv624.OpenValve3()    
    time.sleep(sleep)
    
    fileWrite(logFile, '\nTesting valve 1 - Close', 0)
    pv624.CloseValve1()    
    time.sleep(sleep)
    
    fileWrite(logFile, '\nTesting valve 2 - Close', 0)
    pv624.CloseValve2()    
    time.sleep(sleep)
     
    fileWrite(logFile, '\nTesting valve 3 - Close', 0)
    pv624.CloseValve3()    
    time.sleep(sleep) 
    
    fileWrite(logFile, '\nSetting remote mode on DPI620G ---------------------------', 1)
    DPI620G.setKM('R')
    DPI620G.getKM()
    
    fileWrite(logFile, '\nTesting DPI620G non remote commands ----------------------', 1)
    pressure, error, status = DPI620G.getPVInfo()
    fileString = "\nPressure: " + str(pressure) + " Error: " + str(error) + " Status: " + str(status) + '\n'
    fileWrite(logFile, fileString, 0)
    mode = DPI620G.getControlMode()
    fileString = "\nMode: " + str(mode) + '\n'
    fileWrite(logFile, fileString, 0)
    setPoint = DPI620G.getSetPoint()
    fileString = "\nSet point: " + str(setPoint) + '\n'
    fileWrite(logFile, fileString, 0)
    presType = DPI620G.getPressureType()
    fileString = "\nPressure type: " + str(presType) + '\n'
    fileWrite(logFile, fileString, 0)
    status = DPI620G.getControllerStatus()
    fileString = "\nController status: " + str(status) + '\n'
    fileWrite(logFile, fileString, 0) 
    
    fileWrite(logFile, '\n*********************************************************************', 1)
    fileWrite(logFile, '\nRunning DPI620G and PV624 comms test', 1)
    fileWrite(logFile, '\n*********************************************************************', 1)
    
    fileWrite(logFile, '\nTest set point from DPI620 and read from PV624 ----------', 0)
    
    numSetPoints = 20
    passCounts = 0
    failCounts = 0
    for i in range(0, numSetPoints, 1): 
        sp = random.uniform(0, 20)
        sp = round(sp, 3)
        DPI620G.setSetPoint(sp)        
        setPoint = DPI620G.getSetPoint()
        time.sleep(0.1)   
        if sp == setPoint:
            fileString = '\nSet point value: ' + str(sp) + ':PASSED'
            fileWrite(logFile, fileString, 0) 
            passCounts = passCounts + 1
        else:
            fileString = '\nSet point value: ' + str(sp) + ':FAILED'
            fileWrite(logFile, fileString, 0) 
            failCounts = failCounts + 1
    if passCounts == numSetPoints:
        fileWrite(logFile, '\nSet Points Test: PASSED', 1)
        setPointTest = 1
    else:
        fileWrite(logFile, '\nSet Points Test: FAILED', 1)     
    
    fileWrite(logFile, '\nTest mode from DPI620 and read from PV624 ---------------', 0)
    numModes = 3
    passCounts = 0
    failCounts = 0
    for i in range(0, numModes, 1):
        DPI620G.setControlMode(i)
        dpiMode = DPI620G.getControlMode()
        pvMode = pv624.readMode()      
        if i == dpiMode == pvMode:
            fileString = '\nMode Setting: ' + str(i) + str(dpiMode) + str(pvMode) + ':PASSED'
            fileWrite(logFile, fileString, 0) 
            passCounts = passCounts + 1
        else:
            fileString = '\nMode Setting: ' + str(i) + str(dpiMode) + str(pvMode) + ':FAILED'
            fileWrite(logFile, fileString, 0) 
            failCounts = failCounts + 1
        time.sleep(0.1)
    if passCounts == numModes:
        fileWrite(logFile, '\nMode Test: PASSED', 1)
        modeTest = 1
    else:
        fileWrite(logFile, '\nMode Test: FAILED', 1)          
    
    fileWrite(logFile, '\nTest function from DPI620 and read pressure -------------', 0)
    fileWrite(logFile, '\nBarometric pressure -------------------------------------', 0)  
    setPresType = dpiAttr.pressureType['Barometer']
    DPI620G.setPressureType(setPresType)
    presType = DPI620G.getPressureType()
    if setPresType == presType:
        fileString = '\nPressure Type: ' + str(setPresType) + str(presType) + ':PASSED'
        fileWrite(logFile, fileString, 0) 
        passed = 1
    else:
        fileString = '\nPressure Type: ' + str(setPresType) + str(presType) + ':FAILED'
        fileWrite(logFile, fileString, 0) 
        passed = 0
    if passed == 1:
        fileWrite(logFile, '\nPressure Type: Barometer: PASSED', 1)
        ptbTest = 1
    else:
        fileWrite(logFile, '\nPressure Type: Barometer: FAILED', 1)
        
    pressure, error, status = DPI620G.getPVInfo()
    fileString = "\nPressure: " + str(pressure) + " Error: " + str(error) + " Status: " + str(status)
    fileWrite(logFile, fileString, 0)
    
    fileWrite(logFile, '\nAbsolute pressure ---------------------------------------', 0)
    setPresType = dpiAttr.pressureType['Absolute']
    DPI620G.setPressureType(setPresType)
    presType = DPI620G.getPressureType()
    if setPresType == presType:
        fileString = '\nPressure Type: ' + str(setPresType) + str(presType) + ':PASSED'
        fileWrite(logFile, fileString, 0) 
        passed = 1
    else:
        fileString = '\nPressure Type: ' + str(setPresType) + str(presType) + ':FAILED'
        fileWrite(logFile, fileString, 0) 
        passed = 0
    if passed == 1:
        fileWrite(logFile, '\nPressure Type: Absolute: PASSED', 1)
        ptaTest = 1
    else:
        fileWrite(logFile, '\nPressure Type: Absolute: FAILED', 1)      
        
    pressure, error, status = DPI620G.getPVInfo()
    fileString = "\nPressure: " + str(pressure) + " Error: " + str(error) + " Status: " + str(status)
    fileWrite(logFile, fileString, 0)
    
    fileWrite(logFile, '\nGauge pressure ------------------------------------------', 0)
    setPresType = dpiAttr.pressureType['Gauge']
    DPI620G.setPressureType(setPresType)
    presType = DPI620G.getPressureType()
    if setPresType == presType:
        fileString = '\nPressure Type: ' + str(setPresType) + str(presType) + ':PASSED'
        fileWrite(logFile, fileString, 0) 
        passed = 1
    else:
        fileString = '\nPressure Type: ' + str(setPresType) + str(presType) + ':FAILED'
        fileWrite(logFile, fileString, 0) 
        passed = 0
    if passed == 1:
        fileWrite(logFile, '\nPressure Type: Gauge: PASSED', 1)
        ptgTest = 1
    else:
        fileWrite(logFile, '\nPressure Type: Gauge: FAILED', 1)  
    
    pressure, error, status = DPI620G.getPVInfo()
    fileString = "\nPressure: " + str(pressure) + " Error: " + str(error) + " Status: " + str(status)
    fileWrite(logFile, fileString, 0)
    
    fileWrite(logFile, '\nTest controller status from PV624 and read on DPI -------', 0)
    setStatus = random.randint(10, 65535)
    pv624.setControllerStatus(setStatus)
    status = DPI620G.getControllerStatus()
    if setStatus == status:
        fileString = '\nSetting controller status: ' + str(setStatus) + str(status) + ':PASSED'
        fileWrite(logFile, fileString, 0)
        passed = 1
    else:
        fileString = '\nSetting controller status: ' + str(setStatus) + str(status) + ':FAILED'
        fileWrite(logFile, fileString, 0)
        passed = 0
    if passed == 1:
        fileWrite(logFile, '\nSet Controller Status: PASSED', 1)
        statusTest = 1
    else:
        fileWrite(logFile, '\nSet Controller Status: FAILED', 1)  
        
    fileWrite(logFile, '\nTest pressure type and set point from genii -------------', 0)
    setPoint = round(random.uniform(0, 20), 3)
    testStatus = DataAcqTest(DPI620G, pv624, dpiAttr.pressureType['Gauge'], dpiAttr.controlMode['Measure'], setPoint, iterations, logFile)  
    if testStatus == 1:
        fileWrite(logFile, '\nPressure: Gauge, Mode: Measure: PASSED', 0)
        setPoint = round(random.uniform(0, 20), 3)
        testStatus = DataAcqTest(DPI620G, pv624, dpiAttr.pressureType['Gauge'], dpiAttr.controlMode['Control'], setPoint, iterations, logFile)   
        if testStatus == 1:
            fileWrite(logFile, '\nPressure: Gauge, Mode: Control: PASSED', 0)
            setPoint = round(random.uniform(0, 20), 3)
            testStatus = DataAcqTest(DPI620G, pv624, dpiAttr.pressureType['Gauge'], dpiAttr.controlMode['Vent'], setPoint, iterations, logFile)   
            if testStatus == 1:
                fileWrite(logFile, '\nPressure: Gauge, Mode: Vent: PASSED', 0)
                setPoint = round(random.uniform(0, 20), 3)
                testStatus = DataAcqTest(DPI620G, pv624, dpiAttr.pressureType['Absolute'], dpiAttr.controlMode['Measure'], setPoint, iterations, logFile)   
                if testStatus == 1:
                    fileWrite(logFile, '\nPressure: Absolute, Mode: Measure: PASSED', 0)
                    setPoint = round(random.uniform(0, 20), 3)
                    testStatus = DataAcqTest(DPI620G, pv624, dpiAttr.pressureType['Absolute'], dpiAttr.controlMode['Control'], setPoint, iterations, logFile)   
                    if testStatus == 1:
                        fileWrite(logFile, '\nPressure: Absolute, Mode: Control: PASSED', 0)
                        setPoint = round(random.uniform(0, 20), 3)
                        testStatus = DataAcqTest(DPI620G, pv624, dpiAttr.pressureType['Absolute'], dpiAttr.controlMode['Vent'], setPoint, iterations, logFile)     
                        if testStatus == 1:
                            fileWrite(logFile, '\nPressure: Absolute, Mode: Vent: PASSED', 0)
                        else:
                            fileWrite(logFile, '\nPressure: Absolute, Mode: Vent: FAILED', 0)
                            testStatus = 0    
                    else:
                        fileWrite(logFile, '\nPressure: Absolute, Mode: Control: FAILED', 0)
                        testStatus = 0
                else:
                    fileWrite(logFile, '\nPressure: Absolute, Mode: Measure: FAILED', 0)
                    testStatus = 0                        
            else:
                fileWrite(logFile, '\nPressure: Gauge, Mode: Vent: FAILED', 0)
                testStatus = 0        
        else:
            fileWrite(logFile, '\nPressure: Gauge, Mode: Control: FAILED', 0)
            testStatus = 0         
    else:
        fileWrite(logFile, '\nPressure: Gauge, Mode: Measure: FAILED', 0)
        testStatus = 0             
  
    
    if setPointTest == 1:
        if modeTest == 1:
            if ptaTest == 1:
                if ptgTest == 1:
                    if ptbTest == 1:
                        if statusTest == 1:
                            if testStatus == 1:
                                fileWrite(logFile, '\n*********************************************************************', 1)
                                fileWrite(logFile, '\nALL TESTS PASSED', 1)                                
                            else:
                                fileWrite(logFile, '\n*********************************************************************', 1)
                                fileWrite(logFile, '\nDATA ACQUISITION TEST FAILED', 1) 
                        else:
                            fileWrite(logFile, '\n*********************************************************************', 1)
                            fileWrite(logFile, '\nSTATUS TEST FAILED', 1)
                    else:
                        fileWrite(logFile, '\n*********************************************************************', 1)
                        fileWrite(logFile, '\nPRESSURE TYPE BAROMETER TEST FAILED', 1)
                else:
                    fileWrite(logFile, '\n*********************************************************************', 1)
                    fileWrite(logFile, '\nPRESSURE TYPE GAUGE TEST FAILED', 1)
            else:
                fileWrite(logFile, '\n*********************************************************************', 1)
                fileWrite(logFile, '\nPRESSURE TYPE ABSOLUTETEST FAILED', 1)
        else:
            fileWrite(logFile, '\n*********************************************************************', 1)
            fileWrite(logFile, '\nMODE TEST FAILED', 1)
    else:
        fileWrite(logFile, '\n*********************************************************************', 1)
        fileWrite(logFile, '\nSET POINT TEST FAILED', 1)
    
    fileWrite(logFile, '\nOptical Sensor Test', 0)
    count = 0
    while count < 20:
        sensorVal1, sensorVal2 = pv624.ReadOptSensors()
        fileWrite(logFile, '\nOptical Sensor Value:' + str(sensorVal1) + str(sensorVal2), 0)
        count = count + 1
    
    print('\nResults saved to file - ' + str(fileName), end = " ")
    fileWrite(logFile, '\n*********************************************************************', 1)
    
    logFile.close()
    DPI620G.ClosePort()
    pv624.ClosePort()
    
# runTest()   

def runConstantsTests(setAccAlpha, setAccBeta, setDecAlpha, setDecBeta, file, pv624, noTests, noPassed):
    fileStr = '\nSetting new values of accelaration constants'
    fileWrite(file, fileStr, printStatus)        
    pv624.MOTOR_SetAccAlpha(setAccAlpha)
    noTests = noTests + 1
    pv624.MOTOR_SetAccBeta(setAccBeta)   
    noTests = noTests + 1
    pv624.MOTOR_SetDecAlpha(setDecAlpha)  
    noTests = noTests + 1
    pv624.MOTOR_SetDecBeta(setDecBeta)  
    noTests = noTests + 1
    fileStr = '\nReading values of accelaration constants'
    fileWrite(file, fileStr, printStatus) 
    accAlpha = pv624.MOTOR_GetAccAlpha()
    accBeta = pv624.MOTOR_GetAccBeta()
    decAlpha = pv624.MOTOR_GetDecAlpha()
    decBeta = pv624.MOTOR_GetDecBeta()
    
    result = [setAccAlpha, setAccBeta, setDecAlpha, setDecBeta]
    result = '\nSet values: ' + str(result)
    fileWrite(file, result, printStatus)
    result = [accAlpha, accBeta, decAlpha, decBeta]
    result = '\nGet values: ' + str(result)
    fileWrite(file, result, printStatus)

    if accAlpha == setAccAlpha:
        fileStr = '\nAcceleration alpha: PASSED'
        noPassed = noPassed + 1
        fileWrite(file, fileStr, printStatus)
    else:
        fileStr = '\nAcceleration alpha: FAILED'
        fileWrite(file, fileStr, printStatus)        
        
    if accBeta == setAccBeta:
        fileStr = '\nAcceleration beta: PASSED'
        noPassed = noPassed + 1
        fileWrite(file, fileStr, printStatus)
    else:
        fileStr = '\nAcceleration beta: FAILED'
        fileWrite(file, fileStr, printStatus) 

    if decAlpha == setDecAlpha:
        fileStr = '\nDeceleration alpha: PASSED'
        noPassed = noPassed + 1
        fileWrite(file, fileStr, printStatus)
    else:
        fileStr = '\nDeceleration alpha: FAILED'
        fileWrite(file, fileStr, printStatus) 
        
    if decBeta == setDecBeta:
        fileStr = '\nDeceleration beta: PASSED'
        noPassed = noPassed + 1
        fileWrite(file, fileStr, printStatus)
    else:
        fileStr = '\nDeceleration beta: FAILED'
        fileWrite(file, fileStr, printStatus)       

    return noTests, noPassed

def runCurrentTests(current, file, pv624, noTests, noPassed):
    fileStr = '\nTesting current values setting'
    fileWrite(file, fileStr, printStatus)
    
    fileStr = '\nReset values of current:-'
    fileWrite(file, fileStr, printStatus)
        
    holdI = pv624.GetHoldCurrent()
    fileStr = '\nHOld current: ' + str(round(holdI, 3))
    fileWrite(file, fileStr, printStatus)
    
    accI = pv624.GetAcclCurrent()
    fileStr = '\nAcc current: ' + str(round(accI, 3))
    fileWrite(file, fileStr, printStatus)
    
    decI = pv624.GetDecelCurrent()
    fileStr = '\nDec current: ' + str(round(decI, 3))
    fileWrite(file, fileStr, printStatus)
    
    fileStr = '\nSetting new values of currents'
    fileWrite(file, fileStr, printStatus)        
    pv624.SetHoldCurrent(current)
    noTests = noTests + 1
    pv624.SetAcclCurrent(current)   
    noTests = noTests + 1
    pv624.SetDecelCurrent(current)  
    noTests = noTests + 1
    fileStr = '\nReading values of currents'
    fileWrite(file, fileStr, printStatus) 
    holdI = pv624.GetHoldCurrent()
    acclI = pv624.GetAcclCurrent()
    decelI = pv624.GetDecelCurrent()
    
    result = [current, current, current]
    result = '\nSet values: ' + str(result)
    fileWrite(file, result, printStatus)
    result = [holdI, acclI, decelI]
    result = '\nGet values: ' + str(result)
    fileWrite(file, result, printStatus)

    if current == holdI:
        fileStr = '\nHold current: PASSED'
        noPassed = noPassed + 1
        fileWrite(file, fileStr, printStatus)
    else:
        fileStr = '\nHold current: FAILED'
        fileWrite(file, fileStr, printStatus)        
        
    if current == acclI:
        fileStr = '\nAcceleration current: PASSED'
        noPassed = noPassed + 1
        fileWrite(file, fileStr, printStatus)
    else:
        fileStr = '\nAcceleration current: FAILED'
        fileWrite(file, fileStr, printStatus) 

    if current == decelI:
        fileStr = '\nDeceleration current: PASSED'
        noPassed = noPassed + 1
        fileWrite(file, fileStr, printStatus)
    else:
        fileStr = '\nDeceleration current: FAILED'
        fileWrite(file, fileStr, printStatus) 
        
    return noTests, noPassed

def moveMotor(motor, steps, counts):
    count = 0
    while count < counts:
        pos = motor.MOTOR_MoveContinuous(steps)
        time.sleep(0.1)
        speed, current = motor.GetSpeedAndCurrent()
        time.sleep(0.1)
        count = count + 1
        print(steps, pos, speed, current)
        
def runMotorTest(): 
    try:
        totalTests = 0
        testsPassed = 0
        
        fileName = 'PV624_MOTOR_Test_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'
        file = open(fileName, "w")
        
        pv624 = pvComms.PV624()       
        # pv624.SwitchDuciToEngg()
        
        fileStr = '\nTesting acceleration and deceleration constants setting'
        fileWrite(file, fileStr, printStatus)
        
        fileStr = '\nReset values of constants:-'
        fileWrite(file, fileStr, printStatus)
        
        pv624.MOTOR_MoveContinuous(200)
        time.sleep(0.2)
        pv624.MOTOR_MoveContinuous(100)
        time.sleep(0.2)
        pv624.MOTOR_MoveContinuous(50)
        time.sleep(0.2)
        pv624.MOTOR_MoveContinuous(20)
        time.sleep(0.2)
        pv624.MOTOR_MoveContinuous(5)
        time.sleep(0.2)        
        
        accAlpha = pv624.MOTOR_GetAccAlpha()
        fileStr = '\nAcc Alpha: ' + str(round(accAlpha, 3))
        fileWrite(file, fileStr, printStatus)
        
        accBeta = pv624.MOTOR_GetAccBeta()
        fileStr = '\nAcc Beta: ' + str(round(accBeta, 3))
        fileWrite(file, fileStr, printStatus)
        
        decAlpha = pv624.MOTOR_GetDecAlpha()
        fileStr = '\nDec Alpha: ' + str(round(decAlpha, 3))
        fileWrite(file, fileStr, printStatus)
        
        decBeta = pv624.MOTOR_GetDecBeta()
        fileStr = '\nDec Beta: ' + str(round(decBeta, 3))
        fileWrite(file, fileStr, printStatus)
        
        pv624.MOTOR_MoveContinuous(-200)
        time.sleep(0.2)
        pv624.MOTOR_MoveContinuous(-100)
        time.sleep(0.2)
        pv624.MOTOR_MoveContinuous(-50)
        time.sleep(0.2)
        pv624.MOTOR_MoveContinuous(-20)
        time.sleep(0.2)
        pv624.MOTOR_MoveContinuous(-5)
        time.sleep(0.2)               
        
        if totalTests == testsPassed:
            fileWrite(file, '\n*********************************************************************', 1)
            fileWrite(file, '\nALL TESTS PASSED', 1)         
            fileWrite(file, '\n*********************************************************************', 1)
        else:
            fileWrite(file, '\n*********************************************************************', 1)
            fileWrite(file, '\nTESTS FAILED', 1)         
            fileWrite(file, '\n*********************************************************************', 1)
        print('\nResults saved to file - ' + str(fileName), end = " ")
                
    finally:
        file.close()
        pv624.closePort() 
        
def runValveTimeTest():
    try:
        fileName = 'PV624_VALVE_Test_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'
        file = open(fileName, "w")
        
        fileStr = '\nRunning Valve Test'
        fileWrite(file, fileStr, printStatus)
        
        pv624 = pvComms.PV624()
        valveTime = 1000
        
        while valveTime < 6050:
            fileStr = "\nSetting valve time to: " + str(valveTime) + "us"
            fileWrite(file, fileStr, printStatus)
            pv624.SetValveTime(valveTime)
            pv624.OpenValve1()
            valveTime = valveTime + 50
            time.sleep(0.08)
            
        valveTime = 6000
        while valveTime > 950:
            fileStr = "\nSetting valve time to: " + str(valveTime) + "us"
            fileWrite(file, fileStr, printStatus)
            pv624.SetValveTime(valveTime)
            pv624.OpenValve1()
            valveTime = valveTime - 50
            time.sleep(0.08)            
    finally:
        fileStr = '\nValve Test Complete'
        fileWrite(file, fileStr, printStatus)
        file.close()
        pv624.closePort()
    

runTest()
#runValveTimeTest()
#runMotorTest() 



