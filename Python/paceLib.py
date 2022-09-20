
import serial as ser
import serial.tools.list_ports as prtlst
import dpiAttributes as dpiAttr
import time
import struct

def findpace(SN=[]):
    port = {}
    for pt in prtlst.comports():
        print(pt.hwid)

        if any(s in pt.hwid for s in SN):
            print('\nFound PACE FTDI UART:\n'+ pt.description)

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

class PACE:
    def __init__(self, deviceSN):
        self.port = {}
        self.port = findpace(deviceSN)

    def getControlledPressure(self, module = 1):
        msg = ":SENS" + str(module) + ":PRES?"
        self.sendMessage(msg)
        msg = self.getMessage()
        msg = msg.decode('utf-8')
        msg = msg.split(' ')
        pressure = float(msg[1])
        return pressure

    def getInputPressurePos(self, module = 1):
        msg = ":SOUR" + str(module) + ":PRES:COMP1?"
        msg = self.sendMessage(msg)
        msg = self.getMessage()
        msg = msg.decode('utf-8')
        msg = msg.split(' ')
        pressure = float(msg[1])
        return pressure

    def getInputPressureNeg(self, module = 1):
        msg = ":SOUR" + str(module) + ":PRES:COMP2?"
        msg = self.sendMessage(msg)
        msg = self.getMessage()
        msg = msg.decode('utf-8')
        msg = msg.split(' ')
        pressure = float(msg[1])
        return pressure

    def setPressureMode(self, module = 1, ptype = 'A'):
        if ptype == 'G':
            msg = ":SOUR" + str(module) + ":RANG "  + '"' + "2.00barg" + '"'
        elif ptype == 'A':
            msg = ":SOUR" + str(module) + ":RANG " + '"' + "3.00bara" + '"'
        msg = self.sendMessage(msg)

    def setOutputOn(self, module=1):
        msg = ":OUTP" + str(module) + ":STAT 1"
        msg = self.sendMessage(msg)

    def setOutputOff(self, module=1):
        msg = ":OUTP" + str(module) + ":STAT 0"
        msg = self.sendMessage(msg)

    def setPressurePoint(self, value, module=1):
        msg = ":SOUR" + str(module) + ":PRES" + " " + str(value)
        msg = self.sendMessage(msg)
        return str(msg)

    def getPressurePoint(self, module=1):
        msg = ":SOUR" + str(module) + ":PRES?"
        self.sendMessage(msg)
        msg = self.getMessage()
        msg = msg.decode('utf-8')
        msg = msg.split(' ')
        pressure = float(msg[1])
        return pressure
        
    def vent(self, module=1):
        msg = ":SOUR" + str(module) + ":PRES:LEV:IMM:AMPL:VENT 1"
        msg = self.sendMessage(msg)
        return str(msg)

    def getVentStatus(self, module=1):
        msg = ":SOUR" + str(module) + ":PRES:LEV:IMM:AMPL:VENT?"
        self.sendMessage(msg)
        msg = self.getMessage()
        msg = msg.decode('utf-8')
        msg = msg.split(' ')
        ventStatus = int(msg[1])
        return ventStatus

    def sendMessage(self, msg):
        self.port.flushInput()
        arr = bytes(msg, 'UTF-8')
        msg = msg + '\r\n'
        arr = bytes(msg, 'UTF-8')
        self.port.write(arr)
        time.sleep(0.1)
        return arr

    def getMessage(self):
        msg = self.port.readline()
        return msg

    def ClosePort(self):
        self.port.close() 