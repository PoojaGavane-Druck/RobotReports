# -*- coding: utf-8 -*-
"""
Created on Tue Mar 30 13:01:35 2021

@author: 212596572
"""
import serial as ser
import serial.tools.list_ports as prtlst
import struct
import pv624EnggCommandsList as commandsList
import pv624Attributes as pv624attr
import time

def findPV624():
    #checks all COM ports for PM
    SN = ['A13G41XMA','A13G42XTA','A13G3JU6A',
        'A13G44LMA','A13G1FESA','A13G9OO6A',
        'A13G3JIJA', 'FTG40ZMIA','A13G3V47A',
        'FTG40ZMIA', '205B38714253', '206B38704253'] #valid SN of FTDI chip in USB to UART board
    port = {}
    for pt in prtlst.comports():
        print(pt.hwid)

        if any(s in pt.hwid for s in SN):
            print('\nFound PV624 FTDI UART:\n'+ pt.description)

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

def SerialInit():
    port = []
    # Get a list of all available COM ports
    port_list = prtlst.comports()
    print(port_list)
    
    st_port_config = {'baudrate': 115200,
                      'parity': ser.PARITY_ODD,
                      'stopbits': ser.STOPBITS_ONE,
                      'bytesize': ser.EIGHTBITS,
                      'timeout': 2}

    # Check all available COM ports for STMicro Dev Kit identifier
    for port in port_list:
        port = ser.Serial(port=port.device,
                             baudrate=st_port_config['baudrate'],
                             parity=st_port_config['parity'],
                             stopbits=st_port_config['stopbits'],
                             bytesize=st_port_config['bytesize'],
                             timeout=st_port_config['timeout'])
        port.flushInput()        

    return port

class PV624:
    def __init__(self):
        self.port = {}
        self.timeDelay = 0.000
        self.sensorType = pv624attr.sensorType['PM620']
        self.port = findPV624()
        
        
    def readPressureFast(self):
        if self.sensorType == pv624attr.sensorType['PM620TERPS']:    
             command = commandsList.enggCommands['ReadPressure_55Hz']
            
        elif self.sensorType == pv624attr.sensorType['PM620']: 
            command = commandsList.enggCommands['ReadPressure_440Hz']
            
        self.port.write(command)        
        receivedData = self.port.read(4)
        [pressure] = struct.unpack('f', receivedData)  
        time.sleep(self.timeDelay)
        
        return pressure
        
    def readPressureSlow(self):
        if self.sensorType == pv624attr.sensorType['PM620TERPS']:    
            command = commandsList.enggCommands['ReadPressure_13_75Hz']
            
        elif self.sensorType == pv624attr.sensorType['PM620']: 
            command = commandsList.enggCommands['ReadPressure_55Hz']   
            
        self.port.write(command)        
        receivedData = self.port.read(4)
        [pressure] = struct.unpack('f', receivedData)
        time.sleep(self.timeDelay)
        return pressure

    def readPressureData(self, receivedData):  
        presBuff = receivedData[0:4]
        baroBuff = receivedData[4:8]
        spBuff = receivedData[8:12]
        modeBuff = receivedData[12:16]    
        presGBuff = receivedData[16:20]
        spTypeBuff = receivedData[20:24]
        
        mode = 0
        spType = ''
        
        [pressure] = struct.unpack('f', presBuff)
        [atmPressure] = struct.unpack('f', baroBuff)
        [setPoint] = struct.unpack('f', spBuff) 
        [pressureG] = struct.unpack('f', presGBuff)
        
        if modeBuff[0] == pv624attr.deviceModes['Measure']:
            mode = pv624attr.deviceModes['Measure']
        
        elif modeBuff[0] == pv624attr.deviceModes['Control']:
            mode = pv624attr.deviceModes['Control']
            
        elif modeBuff[0] == pv624attr.deviceModes['Vent']:
            mode = pv624attr.deviceModes['Vent']
            
        if spTypeBuff[0] == pv624attr.spType['Barometer']:
            spType = pv624attr.spType['Barometer']
        elif spTypeBuff[0] == pv624attr.spType['Absolute']:
            spType = pv624attr.spType['Absolute']
        elif spTypeBuff[0] == pv624attr.spType['Gauge']:
            spType = pv624attr.spType['Gauge']
            
        time.sleep(self.timeDelay)
        return pressure, pressureG, atmPressure, setPoint, spType, mode 
    
    def readAllFast(self):
        if self.sensorType == pv624attr.sensorType['PM620TERPS']:    
             command = commandsList.enggCommands['ReadPressure_55Hz']
            
        elif self.sensorType == pv624attr.sensorType['PM620']: 
            command = commandsList.enggCommands['ReadPressure_440Hz']
            
        self.port.write(command)        
        receivedData = self.port.read(24)   
        
        time.sleep(self.timeDelay)
        pressure, pressureG, atmPressure, setPoint, spType, mode = self.readPressureData(receivedData)
        
        return pressure, pressureG, atmPressure, setPoint, spType, mode
        
    def readAllSlow(self):
        if self.sensorType == pv624attr.sensorType['PM620TERPS']:    
            command = commandsList.enggCommands['ReadPressure_13_75Hz']
            
        elif self.sensorType == pv624attr.sensorType['PM620']: 
            command = commandsList.enggCommands['ReadPressure_55Hz']   
            
        self.port.write(command)        
        receivedData = self.port.read(24)    
        
        pressure, pressureG, atmPressure, setPoint, spType, mode = self.readPressureData(receivedData)
        
        time.sleep(self.timeDelay)
        return pressure, pressureG, atmPressure, setPoint, spType, mode

    def readPressure(self, speed):
        
        if speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_CH_OFF']:
            print('Error: Sample rate set to 0Hz')
        
        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_6_875_HZ']:
            command = commandsList.enggCommands['ReadPressure_6_875Hz']
            
        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_13_75_HZ']:
            command = commandsList.enggCommands['ReadPressure_13_75Hz']

        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_27_5_HZ']:
            command = commandsList.enggCommands['ReadPressure_27_5Hz']
            
        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_55_HZ']:
            command = commandsList.enggCommands['ReadPressure_55Hz']
            
        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_110_HZ']:
            command = commandsList.enggCommands['ReadPressure_110Hz']

        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_220_HZ']:
            command = commandsList.enggCommands['ReadPressure_220Hz']   
            
        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_440_HZ']:
            command = commandsList.enggCommands['ReadPressure_440Hz']
            
        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_880_HZ']:
            command = commandsList.enggCommands['ReadPressure_880Hz']

        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_1760_HZ']:
            command = commandsList.enggCommands['ReadPressure_1760Hz']   
            
        elif speed == pv624attr.pmSpeed['E_ADC_SAMPLE_RATE_3520_HZ']:
            command = commandsList.enggCommands['ReadPressure_3520Hz']            
        
        else:
            print('Error: Sample rate not supported')
                  
        self.port.write(command)        
        receivedData = self.port.read(4)
        [pressure] = struct.unpack('f', receivedData)
        time.sleep(self.timeDelay)
        return pressure
    
    def readBarometer(self):
        command = commandsList.enggCommands['ReadAtmPressure']        
        self.port.write(command)        
        receivedData = self.port.read(4)
        [atmPressure] = struct.unpack('f', receivedData) 
        time.sleep(self.timeDelay)
        return atmPressure
        
    def readSensorType(self):
        command = commandsList.enggCommands['PmSensorType']        
        self.port.write(command)        
        receivedData = self.port.read(4)    
        
        str1 = ""
        str2 = ""
        
        if receivedData[0] == pv624attr.sensorType['PM620TERPS']:
            self.sensorType = pv624attr.sensorType['PM620TERPS']
            str1 = "Sensor Type: PM620 TERPS"
            
        elif receivedData[0] == pv624attr.sensorType['PM620']:
            self.sensorType = pv624attr.sensorType['PM620']
            str1 = "Sensor Type: PM620"     
        
        else:
            str1 = "No sensor found"

        if str1 != "No sensor found":
            if receivedData[2] == pv624attr.sensorPressType['Gauge']:
                str2 = " Gauge"            
            
            elif receivedData[2] == pv624attr.sensorPressType['Absolute']:
                str2 = " Absolute"
                
            else:
                str2 = ""
                
        # print(str1 + str2)
        time.sleep(self.timeDelay)
        return self.sensorType
        
    def setControllerStatus(self, status):
        command = [0x00, 0x00, 0x00, 0x00, 0x00]
        command[0] = 0x39
        statusArr = status.to_bytes(4, 'little')    
        command[1] = statusArr[0]
        command[2] = statusArr[1]
        command[3] = statusArr[2]
        command[4] = statusArr[3]
        self.port.write(command) 
        receivedData = self.port.read(4)
        time.sleep(self.timeDelay)
        
    def readFullScalePressure(self):
        command = commandsList.enggCommands['GetFullScaleValue']        
        self.port.write(command)        
        receivedData = self.port.read(4)
        [fsPressure] = struct.unpack('f', receivedData) 
        time.sleep(self.timeDelay)
        
        return fsPressure       
    
    def readSetPoint(self):
        command = commandsList.enggCommands['ReadSetPoint']        
        self.port.write(command)        
        receivedData = self.port.read(4)
        [setPoint] = struct.unpack('f', receivedData) 
        time.sleep(self.timeDelay)
        
        return setPoint
    
    def readMode(self):
        command = commandsList.enggCommands['ReadMode']        
        self.port.write(command)        
        receivedData = self.port.read(4)
        
        mode = 0
        str1 = "Mode: "
        str2 = ""
        
        if receivedData[0] == pv624attr.deviceModes['Measure']:
            str2 = "Measure"
            mode = 0
        
        elif receivedData[0] == pv624attr.deviceModes['Control']:
            str2 = "Control"
            mode = 1
            
        elif receivedData[0] == pv624attr.deviceModes['Vent']:
            str2 = "Vent"     
            mode = 2
                
        # print(str1 + str2) 
        time.sleep(self.timeDelay)
        return mode
        
    def readOpticalSensor(self):        
        command = commandsList.enggCommands['ReadPositionSensorValue']        
        self.port.write(command)        
        receivedData = self.port.read(4)        
        [opticalSensor] = struct.unpack('i', receivedData)   
        time.sleep(self.timeDelay)
        return opticalSensor
        
    def OpenValve1(self):
        command = commandsList.enggCommands['OpenValve1']        
        self.port.write(command) 
        receivedData = self.port.read(4)
        time.sleep(self.timeDelay)
        
    def OpenValve2(self):
        command = commandsList.enggCommands['OpenValve2']        
        self.port.write(command) 
        receivedData = self.port.read(4)
        time.sleep(self.timeDelay)
        
    def OpenValve3(self):
        command = commandsList.enggCommands['OpenValve3']        
        self.port.write(command)
        receivedData = self.port.read(4)
        time.sleep(self.timeDelay)
         
    def CloseValve1(self):
        command = commandsList.enggCommands['CloseValve1']        
        self.port.write(command) 
        receivedData = self.port.read(4)
        time.sleep(self.timeDelay)
        
    def CloseValve2(self):
        command = commandsList.enggCommands['CloseValve2']        
        self.port.write(command) 
        receivedData = self.port.read(4)
        time.sleep(self.timeDelay)
        
    def CloseValve3(self):    
        command = commandsList.enggCommands['CloseValve3']        
        self.port.write(command)
        receivedData = self.port.read(4)
        time.sleep(self.timeDelay)

    def closePort(self):
        self.port.close()         

        