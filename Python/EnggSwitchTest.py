import pv624Lib as pvComms
import dpi620gLib as dpi
import dpiAttributes as dpiAttr
import time
from datetime import datetime
import random

pv624sn = ['2047325E5431']

def main():

    pv624 = pvComms.PV624(pv624sn)

    pv624.SwitchDuciToEngg()
    # allow time to switch
    time.sleep(1)

    pressure, pressureG, baroPressure, sp, setPointType, mode = pv624.readAllSlow()
    print(pressure, pressureG, baroPressure, sp, setPointType, mode)
    pressure, pressureG, baroPressure, sp, setPointType, mode = pv624.readAllSlow()
    print(pressure, pressureG, baroPressure, sp, setPointType, mode)
    pressure, pressureG, baroPressure, sp, setPointType, mode = pv624.readAllSlow()
    print(pressure, pressureG, baroPressure, sp, setPointType, mode)
    pressure, pressureG, baroPressure, sp, setPointType, mode = pv624.readAllSlow()
    print(pressure, pressureG, baroPressure, sp, setPointType, mode)
    pressure, pressureG, baroPressure, sp, setPointType, mode = pv624.readAllSlow()
    print(pressure, pressureG, baroPressure, sp, setPointType, mode)
    pressure, pressureG, baroPressure, sp, setPointType, mode = pv624.readAllSlow()
    print(pressure, pressureG, baroPressure, sp, setPointType, mode)

    pv624.ConfigValve(3, 1)
    pv624.SetValveTime(200)
main()