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

def findDPI(SN=[]):
    port = {}
    for pt in prtlst.comports():
        print(pt.hwid)

        if any(s in pt.hwid for s in SN):
            print('\nFound DPI FTDI UART:\n'+ pt.description)

            port = ser.Serial(port = pt.device,
                                baudrate = 115200,
                                bytesize = ser.EIGHTBITS,
                                parity =ser.PARITY_ODD,
                                stopbits = ser.STOPBITS_ONE,
                                xonxoff = False,
                                rtscts = False,
                                dsrdtr = False,
                                timeout = 2) #note unusual parity setting for the PM COM
            time.sleep(1) #give windows time to open the com port before flushing
            port.flushInput()

    return port 

class DPI620G:
    def __init__(self, deviceSN):
        self.port = {}
    
        self.port = findDPI(deviceSN)
            
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
        
        # print(pType)
        return pType
    
    def getPVInfo(self):
        msg = "#PV?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        pressure, error, status, baro = self.parse(msg, 'PV', 4)
        return pressure, error, status, baro
        
    def setKM(self, mode):        
        if mode == 'R': 
            msg = "#KM=R:"
        elif mode == 'r':
           msg = "#KM=R:"
        elif mode == 'L':
            msg = "#KM=L:"
        elif mode == 'l':
            msg = "#KM=L:"
        self.sendMessage(msg)
              
    def getKM(self):
        msg = "#KM?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        km = self.parse(msg, 'a', 1)
        
        if km == 'R' or km == 'r':
            # print('Current Mode: Remote')
            currentMode = 1
        elif km == 'L' or km == 'l':
            # print('Current Mode: Local')
            currentMode = 0     
        elif km == 'S' or km == 's':
            # print('Current Mode: Service')
            currentMode = 2
            
        return currentMode
        
    def getControllerStatus(self):
        msg = "#CC?:"
        self.sendMessage(msg)
        msg = self.getMessage()   
        status = self.parse(msg, 'x', 1)
        return status
        
    def setPressureTypeO(self, pressureType):        
        if pressureType == 'A':
            msg = "#SF0=13,0:"
        elif pressureType == 'G':
            msg = "#SF0=13,1:"
        elif pressureType == 'B':
            msg = "#SF0=7,0:"
        else:
            msg = "#SF0=0,0:"
            
        self.sendMessage(msg)
            
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
        return str(msg), val

    def getPressureTypeO(self):
        msg = "#SF0?:"
        self.sendMessage(msg)
        msg = self.getMessage() 

    def setVR(self, rate):
        # print("Set Point: ", round(setPoint, 3))
        value = round(rate, 3)
        valueStr = str(value)
        msg = "#VR=" + valueStr + ":"
        self.sendMessage(msg)

    def getVR(self)        :
        msg = "#VR?:"
        self.sendMessage(msg)
        msg = self.getMessage()
        ventRate = self.parse(msg, 'f', 1)
        return ventRate

    def sendMessage(self, msg):
        self.port.flushInput()
        arr = bytes(msg, 'UTF-8')
        checkSum = self.getChecksum(arr, len(msg))
        msg = msg + checkSum + '\r\n'
        arr = bytes(msg, 'UTF-8')
        self.port.write(arr)
        time.sleep(0.1)
        return arr

    def getMessage(self):
        msg = self.port.readline()
        msg = self.port.readline()
        # print(msg)
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
                # print(value)
                return value
            elif retType == 'A' or retType == 'a':
                # print(value)
                return value
            elif retType == 'X' or retType == 'x':
                value = int(value, 16)
                # print(value)
                return value
            elif retType == 'I' or retType == 'i':
                value = int(value)
                return value
        
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
            
    def ClosePort(self):
        self.port.close() 
        
        