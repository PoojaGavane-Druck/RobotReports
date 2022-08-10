# -*- coding: utf-8 -*-
"""
Created on Fri Apr 30 14:47:38 2021

@author: 212596572
"""

import serial as ser
import serial.tools.list_ports as prtlst
import dpiAttributes as dpiAttr
import time
import struct

sleepTime = 0.05

def findDPI(SN=[]):
    port = {}
    for pt in prtlst.comports():
        print(pt.hwid)

        if any(s in pt.hwid for s in SN):
            print('\nFound DPI FTDI UART:\n'+ pt.description)

            port = ser.Serial(port = pt.device,
                                baudrate = 115200,
                                bytesize = ser.EIGHTBITS,
                                parity =ser.PARITY_NONE,
                                stopbits = ser.STOPBITS_ONE,
                                xonxoff = False,
                                rtscts = False,
                                dsrdtr = False,
                                timeout = 2) #note unusual parity setting for the PM COM
            time.sleep(1) #give windows time to open the com port before flushing
            port.flushInput()

    return port 

class BleDPI:
    def __init__(self, deviceSN):
        self.port = {}
        self.port = findDPI(deviceSN)

    def receiveOk(self, value):
        message = self.getMessageLine()
        expectedMessage = "OK=" + value + "\r\n"
        msg = message.decode('utf-8')

        if msg == expectedMessage:
            ok = 1
        else:
            ok = 0

        return ok

    def setFilter(self, value):
        message = "filter " + value
        self.sendMessage(message)

    def scan(self):
        message = "scan\r\n"
        self.sendMessage(message)
        
    def setSetPoint(self, setPoint):
        # print("Set Point: ", round(setPoint, 3))
        value = round(setPoint, 3)
        valueStr = str(value)
        msg = "#SP=" + valueStr + ":"
        self.sendMessage(msg)

    def getSetPoint(self)        :
        msg = "#SP?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        setPoint = self.parse(msg, 'f', 1)
        return setPoint
        
    def getPressure(self):
        msg = "#IV0?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        pressure = self.parse(msg, 'f', 1)
        return pressure
        
    def getSensorInfo(self):
        msg = "#IS?:"
        self.sendMessage(msg)
        msg = self.getMessage()   
        minP, maxP, senType = self.parse(msg, 'IS', 3)
        print(minP, ', ', maxP, ', ', senType)
        return minP, maxP, senType
        
    def setControlMode(self, mode):
        if mode == 0x00:
            # print('Mode: Measure')
            mode = '0'
        elif mode == 0x01:
            # print('Mode: Control')
            mode = '1'
        elif mode == 0x02:
            # print('Mode: Vent')
            mode = '2'
        elif mode == 0x03:
            # print('Mode: Vent')
            mode = '3'
        else:
            mode = '0'
            print('Invalid mode')
        msg = "#CM=" + mode + ":"
        self.sendMessage(msg)
        
    def getControlMode(self):
        msg = "#CM?:"
        self.sendMessage(msg)
        msg = self.getMessage()           
        mode = self.parse(msg, 'i', 1)
        return mode
         
    def setPressureType(self, pressureType):
        if pressureType == dpiAttr.pressureType['Absolute']:
            msg = "#PT=1:"
        elif pressureType == dpiAttr.pressureType['Gauge']:
            msg = "#PT=0:"
        elif pressureType == dpiAttr.pressureType['Barometer']:
            msg = "#PT=2:"
        else:
            msg = "#PT=1:"
        self.sendMessage(msg)

    def getPressureType(self):        
        msg = "#PT?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        pType = self.parse(msg, 'i', 1)
        return pType
    
    def getPVInfo(self):
        msg = "#PV?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        pressure, error, status, baro = self.parse(msg, 'PV', 4)
        return pressure, error, status, baro
        
    def getControllerStatus(self):
        msg = "#CC?:"
        self.sendMessage(msg)
        msg = self.getMessage()   
        status = self.parse(msg, 'x', 1)
        return status

    def getBU(self, value):
        msg = "#BU" + str(value) + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()

        if value == '1':
            brandUnits = self.parse(msg, 'BU', 1)
            return brandUnits
        else:
            brandMin, brandMax, brandType, brandUnits = self.parse(msg, 'BU', 4)
            return brandMin, brandMax, brandType, brandUnits

    def setCA(self):
        msg = "#CA:"
        self.sendMessage(msg)
        msg = self.getMessage() 
        print(msg)
        return str(msg)
        
    def getCD(self, value):
        msg = "#CD" + str(value) + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        calDate = self.parse(msg, 'A', 1)
        return calDate

    def getCI(self, value):
        msg = "#CI" + str(value) + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        calInt = self.parse(msg, 'I', 1)
        return calInt

    def setCM(self, value):
        msg = "#CM=" + value + ":"
        self.sendMessage(msg)

    def getCM(self):
        msg = "#CM?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        mode = self.parse(msg, 'i', 1)
        return mode

    def getCN(self, value):
        msg = "#CN" + str(value) + "?:"
        self.sendMessage(msg)
        msg = self.getMessage() 
        min, max = self.parse(msg, 'CN', 2)
        return int(min), int(max)  

    def setCP(self, calPoint, appVal):
        msg = "#CP" + str(calPoint) + "=" + str(appVal) + ":"
        self.sendMessage(msg)

    def getCS(self):
        msg = "#CS?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        value = self.parse(msg, 'I', 1)
        return value

    def setCS(self):
        msg = "#CS:"
        self.sendMessage(msg)

    def setCT(self, value):
        msg = "#CT=" + value + ":"
        self.sendMessage(msg)
    
    def getDK(self, parm):
        msg = "#DK" + parm + "?:"
        self.sendMessage(msg)
        msg = self.getMessage() 
        dk = self.parse(msg, 'A', 1)
        return dk

    def getIS(self, type):
        msg = "#IS" + str(type) + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        minP, maxP, senType = self.parse(msg, 'IS', 3)
        return minP, maxP, senType

    def getIZ(self, value):
        msg = "#IZ" + str(value) + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        zeroValue = self.parse(msg, 'f', 1)
        return zeroValue
    
    def setIZ(self, value):
        msg = "#IZ:"
        self.sendMessage(msg)    

    def getKM(self):
        msg = "#KM?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        km = self.parse(msg, 'a', 1)
        if km == 'R' or km == 'r':
            currentMode = 1
        elif km == 'L' or km == 'l':
            currentMode = 0     
        elif km == 'S' or km == 's':
            currentMode = 2
        elif km == 'E' or km == 'e':
            currentMode = 3
        elif km == 'S' or km == 's':
            currentMode = 4
        return currentMode

    def setKM(self, mode):        
        if mode == 'R': 
            msg = "#KM=R:"
        elif mode == 'r':
           msg = "#KM=R:"
        elif mode == 'L':
            msg = "#KM=L:"
        elif mode == 'l':
            msg = "#KM=L:"
        elif mode == 'E':
            msg = "#KM=E:"
        elif mode == 'e':
            msg = "#KM=E:"
        elif mode == 'S':
            msg = "#KM=S"
        elif mode == 's':
            msg = "#KM=S"
        self.sendMessage(msg)

    def getND(self, value):
        msg = "#ND" + str(value) + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        nextCalDate = self.parse(msg, 'A', 1)
        return nextCalDate

    def getPP(self):
        msg = "#PP?:"
        self.sendMessage(msg)
        msg = self.getMessage() 
        pinValue = self.parse(msg, 'I', 1)
        return pinValue

    def setPP(self, value):
        msg = "#PP=" + value + ":"
        self.sendMessage(msg)
    
    def getPT(self):
        msg = "#PT?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        pType = self.parse(msg, 'i', 1)
        return pType
    
    def setPT(self, value):
        msg = "#PT=" + value + ":"
        self.sendMessage(msg)
        
    def getPV(self):
        msg = "#PV?:"
        self.sendMessage(msg)
        msg = self.getMessage() 
        pressure, error, status, baro = self.parse(msg, 'PV', 4)
        return pressure, error, status, baro

    def getPA(self):
        msg = "#PA?:"
        self.sendMessage(msg)
        msg = self.getMessage() 
        pressure,pressureAvg = self.parse(msg, 'PA', 2)
        return pressure, pressureAvg

    def getQV(self, value):
        msg = "#QV" + str(value) + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        baseVer = self.parse(msg, 'A', 1)
        return baseVer

    def getRB(self, parm):
        msg = "#RB" + parm + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        if parm == '0':
            val = self.parse(msg, 'f', 1)
        if parm == '1':
            val = self.parse(msg, 'i', 1)
        if parm == '2':
            val = self.parse(msg, 'f', 1)
        if parm == '3':
            val = self.parse(msg, 'i', 1)
        if parm == '4':
            val = self.parse(msg, 'i', 1)
        if parm == '5':
            val = self.parse(msg, 'i', 1)            
        return val
    
    def getRD(self):
        msg = "#RD?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        manDate = self.parse(msg, 'a', 1)
        return manDate

    def getRE(self):
        msg = "#RE?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        error = self.parse(msg, 'x', 1)
        return error
    
    def getRF(self):
        msg = "#RF?:"
        self.sendMessage(msg)
        msg = self.getMessage() 
        fs = self.parse(msg, 'f', 1)
        return fs
        
    def getRI(self):
        msg = "#RI?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        dk, ver = self.parse(msg, 'RI', 2)
        return dk, ver
    
    def getRV(self, type):
        msg = "#RV" + str(type) + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()  
        ver = self.parse(msg, 'a', 1)
        return ver

    def getSC(self):
        msg = "#SC?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        sc = self.parse(msg, 'I', 1)
        return sc
    
    def setSC(self, value):
        msg = "#SC=" + value + ":"
        self.sendMessage(msg)
    
    def getSD(self):
        msg = "#SD?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        sysDate = self.parse(msg, 'A', 1)
        return sysDate

    def setSD(self, value):
        msg = "#SD=" + value + ":"
        self.sendMessage(msg)
        
    def getSN(self, value):
        msg = "#SN" + value + "?:"
        self.sendMessage(msg)
        msg = self.getMessage() 
        sn = self.parse(msg, 'I', 1)
        return sn
    
    def setSN(self, value):
        msg = "#SN0=" + value + ":"
        self.sendMessage(msg) 
    
    def getSP(self):
        msg = "#SP?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        sp = self.parse(msg, 'f', 1)
        return sp
    
    def setSP(self, value):
        msg = "#SP=" + value + ":"
        self.sendMessage(msg)    
        
    def getST(self):
        msg = "#ST?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        sysTime = self.parse(msg, 'ST', 1)
        return sysTime
    
    def getSZ(self):
        msg = "#SZ?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        setPoints = self.parse(msg, 'f', 1)
        return setPoints

    def setST(self, value):
        msg = "#ST=" + value + ":"
        self.sendMessage(msg)
    
    def getTP(self, value):
        msg = "#TP" + value + "?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        tp = self.parse(msg, 'I', 1)
        return tp

    def setTP(self, value):
        msg = "#TP" + value + ":"
        self.sendMessage(msg)

    def getVR(self)        :
        msg = "#VR?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        ventRate = self.parse(msg, 'f', 1)
        return ventRate

    def setVR(self, rate):
        value = round(rate, 3)
        valueStr = str(value)
        msg = "#VR=" + valueStr + ":"
        self.sendMessage(msg)

    def getUF(self)        :
        msg = "#UF?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        value = self.parse(msg, 'i', 1)
        return value

    def setUF(self, value):
        msg = "#UF" + str(value) + ":"
        self.sendMessage(msg)

    def sendMessage(self, message):
        self.port.flushInput()
        message = message + '\r\n'
        arr = bytes(message, 'UTF-8')
        self.port.write(arr)

    def sendMessageWithChecksum(self, msg):
        self.port.flushInput()
        arr = bytes(msg, 'UTF-8')
        checkSum = self.getChecksum(arr, len(msg))
        msg = msg + checkSum + '\r\n'
        arr = bytes(msg, 'UTF-8')
        self.port.write(arr)
        time.sleep(sleepTime)
        return arr

    def getMessage(self, length):
        getFailed = 0
        try:
            msg = self.port.read(length)
        except:
            getFailed = 1

        if getFailed == 1:
            print("No response")
            msg = ""
        
        return msg

    def getMessageLine(self):
        getFailed = 0
        try:
            msg = self.port.readline()
        except:
            getFailed = 1
            
        if getFailed == 1:
            print("No response")
            msg = ""
        
        return msg
           
    def getChecksum(self, arr, length):
        checksum = 0
        for i in range(0, length, 1):
            checksum = checksum + arr[i]
        checksum = checksum % 100
        if checksum < 10:
            checksum = '0' + str(checksum)
        else:
            checksum = str(checksum)
        return checksum
        
    def parse(self, msg, retType, retArgs):
        msg = msg.decode('utf-8')
        msg = msg.split('=')
        msg = msg[1].split(':')

        if retArgs == 1:
            if ' ' in msg:
                msg = msg[0].split(' ')
                value = msg[1]
            else:
                value = msg[0]
            
            if retType == 'F' or retType == 'f':
                value = float(value)
                return value
            elif retType == 'A' or retType == 'a':
                return value
            elif retType == 'X' or retType == 'x':
                value = int(value, 16)
                return value
            elif retType == 'I' or retType == 'i':
                value = int(value)
                return value
            elif retType == "BU":
                brandUnits = str(msg[0])
                return brandUnits
            elif retType == "ST":
                timeVal = str(msg[0]) + ":" + str(msg[1]) + ":" + str(msg[2])
                return timeVal
                
        if retArgs == 2:
            if retType == 'PA':
                if ' ' in msg:
                    msg = msg[0].split(' ')
                    msg = msg[1]
                else:
                    msg = msg[0]

                msg = msg.split(',')
                pressure = float(msg[0])
                pressureAvg = float(msg[1])   
                return pressure, pressureAvg

            if retType == 'RI':
                if ' ' in msg:
                    msg = msg[0].split(' ')
                    msg = msg[1]
                else:
                    msg = msg[0]
                
                msg = msg.split(',')
                dk = msg[0]
                ver = msg[1]

                return dk, ver

            if retType == 'CN':
                if ' ' in msg:
                    msg = msg[0].split(' ')
                    msg = msg[1]
                else:
                    msg = msg[0]
                
                msg = msg.split(',')
                minP = msg[0]
                maxP = msg[1]

                return minP, maxP                
                
        if retArgs == 3:
            if retType == 'IS':
                if ' ' in msg:
                    msg = msg[0].split(' ')
                    msg = msg[1]
                else:
                    msg = msg[0]
                
                msg = msg.split(',')
                minP = float(msg[0])
                maxP = float(msg[1])
                senType = int(msg[2])

                return minP, maxP, senType  

        if retArgs == 4:
            if retType == 'PV':
                if ' ' in msg:
                    msg = msg[0].split(' ')
                    msg = msg[1]
                else:
                    msg = msg[0]
                    
                msg = msg.split(',')
                pressure = float(msg[0])
                error = int(msg[1], 16)
                status = int(msg[2], 16)
                baro = float(msg[3])
                return pressure, error, status, baro     
            if retType == 'BU':
                if ' ' in msg:
                    msg = msg[0].split(' ')
                    msg = msg[1]
                else:
                    msg = msg[0]
                    
                msg = msg.split(',')
                brandMin = str(msg[0])
                brandMax = str(msg[1], 16)
                brandType = str(msg[2], 16)
                brandUnits = str(msg[3])
                return brandMin, brandMax, brandType, brandUnits          
            
    def ClosePort(self):
        self.port.close() 
        
        