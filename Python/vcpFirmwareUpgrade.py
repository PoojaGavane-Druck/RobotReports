from compileall import compile_path
import dpi620gLib as dpi
import dpiAttributes as dpiAttr
from datetime import datetime

printIt = 1
writeLog = 1

pv624sn = ['205C324B5431']
dpi620gSn = ['FTBTBC9KA']
fileName = 'PV624_VCP_FW_UPGRADE_LOG_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'
upgradeFileName = "DK0499.dpi"

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

def upgradeFirmware():
    display("Upgrading PV624 firmware via VCP...")

    try:
        upgradeFile = open(upgradeFileName, 'rb')
    except:
        display("Could not open firmware upgrade file")

    upgradeFileBuff = upgradeFile.read()

    #print(upgradeFileBuff)

    # 1. Send RE command to check for errors
    # 2. Send KM command to set device into remote mode

upgradeFirmware()


