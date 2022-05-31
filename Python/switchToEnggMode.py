import dpi620gLib as dpi
import pv624Lib as pvComms
import dpiAttributes as dpiAttr
from datetime import datetime
import time

printIt = 1
writeLog = 1

pv624sn = ['205C324B5431']
dpi620gSn = ['FTBTBC9KA']

def switchToEnggMode():
    enggModeSet = 0

    DPI620G = dpi.DPI620G(dpi620gSn)
    # First switch genii to remote mode
    DPI620G.setKM('R')
    # Switch the PV624 USB port from MSC to VCP
    enggModeSet = DPI620G.switchUsbToEnggMode()

    if enggModeSet == 1:
        # Initiate a new class for PV624
        PV624 = pvComms.PV624(pv624sn)
        pressure, error, status, baro = PV624.readAllSlow()


switchToEnggMode()


