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
import csv  # added for dataDump
from datetime import datetime  # added for dataDump file name

printIt = 0
ACK = 0x3C


def findPV624():
    # checks all COM ports for PM
    SN = ['206B38704253', '205A38714253', '206438714253', '2050386A4253']  # SNs of valid USB COM connections to pv624
    port = {}
    for pt in prtlst.comports():
        print(pt.hwid)

        if any(s in pt.hwid for s in SN):
            print('\nFound PV624 FTDI UART:\n' + pt.description)

            port = ser.Serial(port=pt.device,
                              baudrate=115200,
                              bytesize=ser.EIGHTBITS,
                              parity=ser.PARITY_ODD,
                              stopbits=ser.STOPBITS_ONE,
                              xonxoff=False,
                              rtscts=False,
                              dsrdtr=False,
                              timeout=2)  # note unusual parity setting for the PM COM
            time.sleep(1)  # give windows time to open the com port before flushing
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
        self.timeDelay = 0.001
        self.sensorType = pv624attr.sensorType['PM620']
        self.port = findPV624()
        self.params = {}

        # new parameters for reading dataDump from serial port
        # headers are little endian
        self.header = b'\xff\xff\xff\xff\xff\xff\xff\xff'  # header, unlikely to exist in message data
        self.footer = b'\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe'  # footer, slightly different from header

        self.parsedMessage = {}  # dictionary containing latest dataDump message after parsing
        self.allReceivedData = bytearray()  # raw data dataDump from serial port
        self.message = []  # parsed values from latest dataDump message

        # see excel DataDump.xls from MD for definitions of message format
        self.keys = ['elapsedTime', 'ms', 'pressureSetPoint', 'setPointType', 'stepCount', 'pressureError',
                     'totalStepCount', 'controlledPressure', 'pressureAbs', 'pressureGauge', 'pressureBaro',
                     'pressureOld', 'stepSize', 'pressureCorrectionTarget', 'requestedMotorCurrent',
                     'measuredMotorCurrent', 'opticalSensorReading', 'pistonPosition', 'motorSpeed', 'inRange',
                     'pumpTolerance', 'minSysVolumeEstimate', 'maxSysVolumeEstimate', 'minEstimatedLeakRate',
                     'maxEstimatedLeakRate', 'measuredPressure', 'smoothedPressure', 'changeInPressure',
                     'prevChangeInPressure', 'dP2', 'estimatedVolume', 'algorithmType', 'changeInVolume',
                     'prevChangeInVolume', 'dV2', 'measuredVolume', 'estimatedLeakRate', 'measuredLeakRate',
                     'estimatedKp', 'measuredKp', 'sensorUncertainty', 'uncertaintyPressureDiff',
                     'uncertaintyVolumeEstimate', 'uncertaintyMeasuredVolume', 'uncertaintyVolumeChange',
                     'uncertaintyEstimatedLeakRate', 'uncertaintyMeasuredLeakRate', 'maxZScore', 'lambda',
                     'uncertaintySmoothedPressureError', 'targetdP', 'smoothedPressureError',
                     'smoothedSquaredPressureError', 'gamma', 'predictionError', 'predictionErrorType',
                     'maxAchievablePressure', 'minAcheivablePressure', 'maxPositivePressureChangeAchievable',
                     'maxNegativePressureChangeAchievable', 'minPressureAdjustmentFactor', 'nominalHomePosition',
                     'expectedPressureCenterPosition', 'maxIterationsIIRFilter', 'minInterationsIIRFilter',
                     'changeToEstimatedleakRate', 'alpha', 'smoothedPressureErrorForCorrection', 'log10Epsilon',
                     'residualLeakRate', 'measuredLeakRate1', 'numberOfControlIterations', 'pumpUp', 'pumpDown',
                     'control', 'venting', 'stable', 'vented', 'excessLeak', 'excessVOlume', 'overPressure',
                     'excessOffset', 'measure', 'fineControl', 'pistonCentered', 'centering', 'controlledVent',
                     'centeringVent', 'rangeExceeded', 'coarseControlError', 'ventDirUp', 'ventDirDown']

        self.keys = ['elapsedTime', 'ms', 'pressureSetPoint', 'setPointType', 'stepCount', 'pressureError',
                     'totalStepCount', 'controlledPressure', 'pressureAbs', 'pressureGauge', 'pressureBaro',
                     'pressureOld', 'stepSize', 'pressureCorrectionTarget', 'requestedMotorCurrent',
                     'measuredMotorCurrent', 'opticalSensorReading', 'pistonPosition', 'motorSpeed', 'inRange',
                     'pumpTolerance', 'minSysVolumeEstimate', 'maxSysVolumeEstimate', 'minEstimatedLeakRate',
                     'maxEstimatedLeakRate', 'measuredPressure', 'smoothedPressure', 'changeInPressure',
                     'prevChangeInPressure', 'dP2', 'estimatedVolume', 'algorithmType', 'changeInVolume',
                     'prevChangeInVolume', 'dV2', 'measuredVolume', 'estimatedLeakRate', 'measuredLeakRate',
                     'estimatedKp', 'measuredKp', 'sensorUncertainty', 'uncertaintyPressureDiff',
                     'uncertaintyVolumeEstimate', 'uncertaintyMeasuredVolume', 'uncertaintyVolumeChange',
                     'uncertaintyEstimatedLeakRate', 'uncertaintyMeasuredLeakRate', 'maxZScore', 'lambda',
                     'uncertaintySmoothedPressureError', 'targetdP', 'smoothedPressureError',
                     'smoothedSquaredPressureError', 'gamma', 'predictionError', 'predictionErrorType',
                     'maxAchievablePressure', 'minAcheivablePressure', 'maxPositivePressureChangeAchievable',
                     'maxNegativePressureChangeAchievable', 'minPressureAdjustmentFactor', 'nominalHomePosition',
                     'expectedPressureCenterPosition', 'maxIterationsIIRFilter', 'minInterationsIIRFilter',
                     'changeToEstimatedleakRate', 'alpha', 'smoothedPressureErrorForCorrection', 'log10Epsilon',
                     'residualLeakRate', 'measuredLeakRate1', 'numberOfControlIterations', 'status']

        # names of bits encoded by status uint32 value
        self.statusKeys = ['pumpUp', 'pumpDown',
                           'control', 'venting', 'stable', 'vented', 'excessLeak', 'excessVOlume', 'overPressure',
                           'excessOffset', 'measure', 'fineControl', 'pistonCentered', 'centering', 'controlledVent',
                           'centeringVent', 'rangeExceeded', 'coarseControlError', 'ventDirUp', 'ventDirDown']

        # data types to be extracted from message body using stuct, mapped to self.keys
        self.dataTypes = '<IIfIififffffifffiifIfffffffffffIfffffffffffffffffffffffIfffffifIIIIIIIIII'

        self.messageLength = len(self.keys) * 4  # expected length of message in data dump (bytes)
        # not including header and footer

        self.maxSize = (self.messageLength + 8 + 8) * 2 - 1  # maximum size of serial data buffer (bytes)
        # make read buffer large enough to hold 2 complete messages minus one byte
        # The buffer is then guaranteed to hold one and only one complete message
        # with the latest message located somewhere random within the buffer.
        # Each message has a 8 byte header 0xFFFFFFFF0xFFFFFFFF
        # and 8 byte footer 0xFEFEFEFExFEFEFEFE
        # and each key value is 4 bytes

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
                pressType = 0x00

            elif receivedData[2] == pv624attr.sensorPressType['Absolute']:
                str2 = " Absolute"
                pressType = 0x01

            else:
                str2 = ""

        # print(str1 + str2)
        time.sleep(self.timeDelay)
        return self.sensorType, pressType

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

    def MOTOR_ResetControllerIC(self):
        txBuff = [0x28, 0x00, 0x00, 0x00, 0x00]
        return self.MOTOR_SendRecieveAck(txBuff)

    def MOTOR_GetVersionID(self):
        txBuff = [0x27, 0x00, 0x00, 0x00, 0x00]
        self.port.write(txBuff)
        rxBuff = self.port.read(4)
        self.printMessage(rxBuff, printIt)
        val = int.from_bytes(bytes=rxBuff, byteorder="big")
        version = str(rxBuff[0]) + "." + str(rxBuff[1]) + "." + str(val & 0xFFFF)
        time.sleep(self.timeDelay)
        return version

    def MOTOR_GetAccAlpha(self):
        txBuff = [0x1B, 0x00, 0x00, 0x00, 0x00]
        self.port.write(txBuff)
        rxBuff = self.port.read(4)
        self.printMessage(rxBuff, printIt)
        alpha = float(int.from_bytes(bytes=rxBuff, byteorder='big')) / 1000
        self.params['Acc_alpha'] = alpha
        time.sleep(self.timeDelay)
        return alpha

    def MOTOR_GetAccBeta(self):
        txBuff = [0x1C, 0x00, 0x00, 0x00, 0x00]
        self.port.write(txBuff)
        rxBuff = self.port.read(4)
        self.printMessage(rxBuff, printIt)
        beta = float(int.from_bytes(bytes=rxBuff, byteorder='big')) / 1000
        self.params['Acc_beta'] = beta
        time.sleep(self.timeDelay)
        return beta

    def MOTOR_GetDecAlpha(self):
        txBuff = [0x1D, 0x00, 0x00, 0x00, 0x00]
        self.port.write(txBuff)
        rxBuff = self.port.read(4)
        self.printMessage(rxBuff, printIt)
        alpha = float(int.from_bytes(bytes=rxBuff, byteorder='big')) / 1000
        self.params['Dec_alpha'] = alpha
        time.sleep(self.timeDelay)
        return alpha

    def MOTOR_GetDecBeta(self):
        txBuff = [0x1E, 0x00, 0x00, 0x00, 0x00]
        self.port.write(txBuff)
        rxBuff = self.port.read(4)
        self.printMessage(rxBuff, printIt)
        beta = float(int.from_bytes(bytes=rxBuff, byteorder='big')) / 1000
        self.params['Dec_beta'] = beta
        time.sleep(self.timeDelay)
        return beta

    def MOTOR_SetAccAlpha(self, alpha):
        self.params['Acc_alpha'] = alpha
        alpha = int(alpha * 1000)
        temp = alpha.to_bytes(2, 'big')
        txBuff = [0x17, 0x00, 0x00, temp[0], temp[1]]
        status = self.MOTOR_SendRecieveAck(txBuff)
        print(status)
        time.sleep(self.timeDelay)
        if status:
            return True
        else:
            return False

    def MOTOR_SetAccBeta(self, beta):
        self.params['Acc_beta'] = beta
        beta = int(beta * 1000)
        temp = beta.to_bytes(2, 'big')
        txBuff = [0x18, 0x00, 0x00, temp[0], temp[1]]
        status = self.MOTOR_SendRecieveAck(txBuff)
        print(status)
        time.sleep(self.timeDelay)
        if status:
            return True
        else:
            return False

    def MOTOR_SetDecAlpha(self, alpha):
        self.params['Dec_alpha'] = alpha
        alpha = int(alpha * 1000)
        temp = alpha.to_bytes(2, 'big')
        txBuff = [0x19, 0x00, 0x00, temp[0], temp[1]]
        status = self.MOTOR_SendRecieveAck(txBuff)
        print(status)
        time.sleep(self.timeDelay)
        if status:
            return True
        else:
            return False

    def MOTOR_SetDecBeta(self, beta):
        self.params['Dec_beta'] = beta
        beta = int(beta * 1000)
        temp = beta.to_bytes(2, 'big')
        txBuff = [0x1A, 0x00, 0x00, temp[0], temp[1]]
        status = self.MOTOR_SendRecieveAck(txBuff)
        time.sleep(self.timeDelay)
        if status:
            return True
        else:
            return False

    def MOTOR_SendRecieveAck(self, data):
        command = data[0]
        expectedResponse = [command, ACK, 0x00, 0x00]
        self.port.write(data)
        rxBuff = self.port.read(4)
        self.printMessage(rxBuff, printIt)
        response = [rxBuff[0], rxBuff[1], rxBuff[2], rxBuff[3]]
        time.sleep(self.timeDelay)
        if response == expectedResponse:
            return True
        else:
            return False

    def SetHoldCurrent(self, current):
        if current > 2.0:
            print("Cannot set current higher than 2.0A")
        elif current < 0.0:
            print("Cannot set current lower than 0.0A")
        else:
            alpha = int(current * 1000)
            temp = alpha.to_bytes(2, 'big')
            cmd = [0x29, 0x00, 0x00, temp[0], temp[1]]
            self.MOTOR_SendRecieveAck(cmd)
            time.sleep(self.timeDelay)

    def SetAcclCurrent(self, current):
        if current > 2.0:
            print("Cannot set current higher than 2.0A")
        elif current < 0.0:
            print("Cannot set current lower than 0.0A")
        else:
            alpha = int(current * 1000)
            temp = alpha.to_bytes(2, 'big')
            cmd = [0x2B, 0x00, 0x00, temp[0], temp[1]]
            self.MOTOR_SendRecieveAck(cmd)
            time.sleep(self.timeDelay)

    def SetDecelCurrent(self, current):
        if current > 2.0:
            print("Cannot set current higher than 2.0A")
        elif current < 0.0:
            print("Cannot set current lower than 0.0A")
        else:
            alpha = int(current * 1000)
            temp = alpha.to_bytes(2, 'big')
            cmd = [0x2C, 0x00, 0x00, temp[0], temp[1]]
            self.MOTOR_SendRecieveAck(cmd)
            time.sleep(self.timeDelay)

    def GetHoldCurrent(self):
        txBuff = [0x2D, 0x00, 0x00, 0x00, 0x00]
        self.port.write(txBuff)
        rxBuff = self.port.read(4)
        self.printMessage(rxBuff, printIt)
        val = float(int.from_bytes(bytes=rxBuff, byteorder='big'))
        val = float(val / 1000)
        time.sleep(self.timeDelay)
        return val

    def GetAcclCurrent(self):
        txBuff = [0x2F, 0x00, 0x00, 0x00, 0x00]
        self.port.write(txBuff)
        rxBuff = self.port.read(4)
        self.printMessage(rxBuff, printIt)
        val = float(int.from_bytes(bytes=rxBuff, byteorder='big'))
        val = float(val / 1000)
        time.sleep(self.timeDelay)
        return val

    def GetDecelCurrent(self):
        txBuff = [0x30, 0x00, 0x00, 0x00, 0x00]
        self.port.write(txBuff)
        rxBuff = self.port.read(4)
        self.printMessage(rxBuff, printIt)
        val = float(int.from_bytes(bytes=rxBuff, byteorder='big'))
        val = float(val / 1000)
        time.sleep(self.timeDelay)
        return val

    def MOTOR_MoveContinuous(self, steps):
        # Check if a change in direction has been set and inform the L6472 as such
        if steps > 0:
            self.dir = 'forwards'
        else:
            self.dir = 'backwards'

        if steps > 32767:
            steps = 32676
        elif steps < -32767:
            steps = -32767

        valueCmd = int.to_bytes(steps, 4, "big", signed=True)
        txBuff = [0x13, valueCmd[0], valueCmd[1], valueCmd[2], valueCmd[3]]
        self.port.write(txBuff)
        # Read the returned 4 bytes for steps taken on previous move_continuous command
        rxBuff = self.port.read(4)
        # Convert byte array to signed int
        pos = int.from_bytes(bytes=rxBuff, byteorder='big', signed=True)
        time.sleep(self.timeDelay)
        return pos

    def MOTOR_SetStepSize(self, stepSize):
        # Check that a correct step size has been passed
        if stepSize in {1, 2, 4, 8, 16}:
            if stepSize == 1:
                data = 0x00
            elif stepSize == 2:
                data = 0x01
            elif stepSize == 4:
                data = 0x02
            elif stepSize == 8:
                data = 0x03
            elif stepSize == 16:
                data = 0x04
            else:
                # Default to full step (should never be called)
                data = 0x00

            # This bit needs setting for the L6472 to acknowledge the new step size
            data = data | 0x08
            txBuff = [0x01, 0x16, 0x00, 0x00, data]
            status = self.MOTOR_SendRecieveAck(txBuff)
            print(status)
            time.sleep(self.timeDelay)
            if status:
                return True
            else:
                return False

        else:
            print("Step size note available\n"
                  "Please Enter one of the following:\n"
                  "\t1, 2, 4, 8 or 16")
            return

    def GetSpeedAndCurrent(self):
        txBuff = [0x31, 0x00, 0x00, 0x00, 0x00]
        self.port.write(txBuff)
        rxBuff = self.port.read(4)
        val = int.from_bytes(bytes=rxBuff, byteorder='big')
        curr = val & 0xFFFF
        speed = (val >> 16) & 0xFFFF
        time.sleep(self.timeDelay)
        return speed, curr

    def readOpticalSensor(self):
        command = commandsList.enggCommands['ReadPositionSensorValue']
        self.port.write(command)
        receivedData = self.port.read(4)
        [opticalSensor] = struct.unpack('i', receivedData)
        time.sleep(self.timeDelay)
        return opticalSensor

    def printMessage(self, buff, printState):
        if printState == 1:
            print(buff)

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

    def ClosePort(self):
        self.port.close()

    def closePort(self):
        self.port.close()

    # -------------------------------------------------
    # Added by A May for embedded control loop data dump
    # Oct 14 2021

    def readDataDump(self, testData = b''):
        #  read binary data from serial port
        #  parse the latest PV624 message
        #  return the parsed values as a dictionary
        #  write the dictionary values to csv log file for offline analysis

        if testData:
            # use provided testData
            print('Used testData from file')
            self.allReceivedData = testData

        else:
            #  use real-time data from serial port
            bytesWaiting = self.port.in_waiting

            if bytesWaiting > 0:
                receivedData = self.port.read(bytesWaiting)
                self.allReceivedData = self.allReceivedData + receivedData

            if len(self.allReceivedData) > self.maxSize:
                self.allReceivedData = self.allReceivedData[-self.maxSize:]  # truncate to last maxSize bytes
                lastFooter = self.allReceivedData.rfind(self.footer)  # index of start of last footer

                # TBD, reinstate this method when footer value updated to efefefefefefef
                firstHeader = self.allReceivedData.find(self.header) + len(self.header)  # index after end of first header
                #if lastFooter - firstHeader == self.messageLength:
                    #binaryMessage = self.allReceivedData[firstHeader:lastFooter]

                if lastFooter > self.messageLength:  # full message available for parsing
                    binaryMessage = self.allReceivedData[lastFooter - self.messageLength:lastFooter]
                    # parse binary message to numeric values
                    newMessage = struct.unpack(self.dataTypes, binaryMessage)
                    if newMessage != self.message:
                        self.message = newMessage
                        self.parsedMessage = dict(zip(self.keys, self.message))
                        # convert epoch format time stamp to total ms time for easier offline analysis
                        # msTime = (self.parsedMessage['elapsedTime'] * 1000 + self.parsedMessage['ms'])
                        # self.parsedMessage['elapsedTime'] = msTime  # overwrite epoch time stamp with total ms time
                        # self.parsedMessage['ms'] = 0  # overwrite ms time stamp, not used, make zero to avoid confusion

                        #  decode the status uint32 into string bit pattern
                        #  and reverse the bit order to match order in self.statusKeys
                        self.parsedMessage['status'] = ('{0:032b}'.format(self.parsedMessage['status']))[::-1]
                        #  assign the bit pattern values in statusKeys and append to parsedMessage
                        #  improves readability and filtering of status values in offline processing
                        for key, value in zip(self.statusKeys, self.parsedMessage['status']):
                            self.parsedMessage[key] = int(value)

                    else:
                        print('Warning: no update from dataDump')
                else:
                    print('Error: invalid dataDump message\n',
                          self.allReceivedData)
                    print(lastFooter, firstHeader, lastFooter - firstHeader, self.messageLength)
            else:
                # print('Warning: insufficient data in buffer')
                return {}  # return an empty dict instead of parsedMessage when nothing new to report
        return self.parsedMessage


if __name__ == '__main__':
    pv624 = []
    try:
        pv624 = PV624()  # find STM32 main board and open serial connection
        dataDumpFile = datetime.now().strftime('%Y-%m-%dT%H-%M-%S') + '.csv'  # parsed dataDump file

        # with open('DataDumpVentRaw72Parms', 'rb') as f:
        #     testData = f.read()  # read example binary data dump file

        with open(dataDumpFile, 'w', newline='') as f:
            # write header keys to dataFile
            # keep file open for subsequent value writes
            csvFile = csv.writer(f, delimiter=',')
            csvFile.writerow(pv624.keys + pv624.statusKeys)
            index = 0
            writeMode = 0

            while True:
                try:
                    data = pv624.readDataDump()
                    if data:
                        print(data['elapsedTime'],
                              data['pistonPosition'],
                              round(data['pressureAbs'], 2),
                              round(data['estimatedVolume'], 1),
                              round(data['measuredMotorCurrent'], 1),
                              round(data['estimatedLeakRate'], 1),
                              round(data['pressureError'], 1),
                              data['stepSize'],
                              data['control'],
                              data['venting'],
                              data['measure'])
                        index = index + 1
                            
                        if data['measure'] == '1':
                            writeMode = 0
                        elif data['control'] == '1':
                            writeMode = 1
                        elif data['venting'] == '1':
                            writeMdoe = 2
                        
                        csvFile.writerow(data.values())
                        # time.sleep(0.01)  # maximum read rate 10 ms
                except KeyboardInterrupt:
                    print('Stopped')
                    break
    finally:
        if pv624:
            f.close()
            pv624.closePort()


