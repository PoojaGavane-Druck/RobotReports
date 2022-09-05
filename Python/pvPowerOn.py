import serial as ser
import serial.tools.list_ports as prtlst
import time

arduinoSn = "8593731333735120E011"
powerDown = "B"
powerUp = "A"

def findArduino(SN=[]):
    port = {}
    for pt in prtlst.comports():
        print(pt.hwid)

        id = pt.hwid.split(" ")
        serialNo = id[2].split("=")

        if serialNo[1] == SN:
            print('\nFound ARDUINO UART:\n'+ pt.description)

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
            break
    return port 
def sendMessage(port, msg):
    port.flushInput()
    arr = bytes(msg, 'UTF-8')
    msg = msg + '\r\n'
    arr = bytes(msg, 'UTF-8')
    port.write(arr)
    time.sleep(0.1)
    return arr

def powerOn(port):
    message = powerUp
    sendMessage(port, message)
    time.sleep(2)

arduinoCom = findArduino(arduinoSn)
powerOn(arduinoCom)






