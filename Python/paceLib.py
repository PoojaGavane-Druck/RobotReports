
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

    def getControlledPressure(self):
        msg = ":SENS2:PRES?"
        self.sendMessage(msg)
        msg = self.getMessage()
        msg = msg.decode('utf-8')
        msg = msg.split(' ')
        pressure = float(msg[1])
        return pressure

    def setPressureMode(self):
        msg = ":SOUR2:RANG " + "2.00barg"
        msg = self.sendMessage(msg)

    def setOutputOn(self):
        msg = ":OUTP2:STAT 1"
        msg = self.sendMessage(msg)

    def setOutputOff(self):
        msg = ":OUTP2:STAT 0"
        msg = self.sendMessage(msg)

    def setPressurePoint(self, value):
        msg = ":SOUR2:PRES" + " " + str(value)
        msg = self.sendMessage(msg)
        return str(msg)

    def getPressurePoint(self):
        msg = ":SOUR2:PRES?"
        msg = msg.decode('utf-8')
        msg = msg.split(' ')
        pressure = float(msg[1])
        return pressure
        
    def sendMessage(self, msg):
        self.port.flushInput()
        arr = bytes(msg, 'UTF-8')
        #checkSum = self.getChecksum(arr, len(msg))
        msg = msg + '\r\n'
        arr = bytes(msg, 'UTF-8')
        self.port.write(arr)
        time.sleep(0.1)
        return arr

    def getMessage(self):
        msg = self.port.readline()
        return msg