import dpi620gLib as dpi
import dpiAttributes as dpiAttr
from datetime import datetime

printIt = 1
writeLog = 1

pv624sn = ['205C324B5431']
dpi620gSn = ['FTBTBC9KA']
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
    display("Set point test for the PV624")
    baseDk = ""
    baseVer = ""
    DPI620G = dpi.DPI620G(dpi620gSn)

    setPointValue = -100
    setPointIncrementValue = 0.05
    setPointNumber = 0
    setPointMaxNumber = 45001
    
