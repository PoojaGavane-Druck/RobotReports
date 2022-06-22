import dpi620gLib as dpi
import dpiAttributes as dpiAttr
from datetime import datetime
import time

printIt = 1
writeLog = 1
pv624sn = ['205C324B5431']
fileName = 'PRODUCTION_TESTS_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'

try:
    file = open(fileName, "w")
except:
    errorHere = 1
    print("Could not open log file")

def display(toDisplay):
    if printIt == 1:
        print(toDisplay)
    if writeLog == 1:
        # File writing enabled 
        file.write(toDisplay + "\n")

def runProductionTests():
    timeout = 0
    usbComms = dpi.DPI620G(pv624sn)

    display("Setting service mode")
    usbComms.setKM('S')
    mode = usbComms.getKM()

    if mode == 4:
        display("Service mode set")
        usbComms.setTP('3')

        result = usbComms.getTP('3')
        while result == 0:
            result = usbComms.getTP('3')
            timeout = timeout + 1
            time.sleep(1)
            if timeout > 10:
                timeout = 0
                failed = 1
                display("TEST 1: EEPROM - FAILED")
                break
        if failed == 0:
            display("TEST 1: EEPROM - PASSED")
    else:
        display("ERROR: Service mode not set")


runProductionTests()