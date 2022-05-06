import dpi620gLib as dpi
import dpiAttributes as dpiAttr
from datetime import datetime

printIt = 1
writeLog = 1

pv624sn = ['205C324B5431']
dpi620gSn = ['FTBTA7ISA']
fileName = 'GENII_CONNECTION_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'

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

def main():
    display("Starting baro connection test")
    baseDk = ""
    baseVer = ""
    DPI620G = dpi.DPI620G(dpi620gSn)

    DPI620G.setPT("2")
    pType = DPI620G.getPT()
    display("Pressure type: ", str(pType))
    display("Reading barometer calibration date")
    baroCalDate = DPI620G.getCD(dpiAttr.pvSensor['barometer'])
    display("Barometer calibration date: " + str(baroCalDate))
    # Read barometer calibration interval
    ''' 
    Possible issue here is that the function in the code is never set to barometric measurement, 
    evaluate tomorrow in the tests
    '''
    display("Reading barometer calibration date")
    baroCalInterval = DPI620G.getCI(dpiAttr.pvSensor['barometer'])
    display("Barometer calibration interval " + str(baroCalInterval) + " days")
    # Read barometer next calibration date
    display("Reading barometer next calibration date")
    baroNextCalDate = DPI620G.getND(dpiAttr.pvSensor['barometer'])
    display("Barometer calibration interval " + str(baroNextCalDate))
    # Read number of barometer calibration points
    minPoints, maxPoints = DPI620G.getCN(dpiAttr.pvSensor['barometer'])
    display("Barometer calibration minimum points: " + str(minPoints))
    display("Barometer calibration maximum points: " + str(maxPoints))