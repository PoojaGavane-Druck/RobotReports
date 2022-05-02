import dpi620gLib as dpi
import pv624Lib as pv624
from datetime import datetime
import time
import csv
import random
    
pv624sn = ['205C324B5431']
dpi620gSn = ['FTBTA7ISA']

def operateV1V2(PV624):
    PV624.OpenValve1()
    time.sleep(0.25)
    PV624.CloseValve1()
    time.sleep(0.25)
    PV624.OpenValve1()
    time.sleep(0.25)
    PV624.CloseValve1()
    time.sleep(0.25)

    PV624.OpenValve2()
    time.sleep(0.25)
    PV624.CloseValve2()
    time.sleep(0.25)
    PV624.OpenValve2()
    time.sleep(0.25)
    PV624.CloseValve2()
    time.sleep(0.25)

def main():   
    DPI620G = dpi.DPI620G(dpi620gSn)
    PV624 = pv624.PV624(pv624sn)
    sm = 0
    steps = -15
    reverse = -1
    counter = 0
    microSec = 6000

    DPI620G.setKM('R')

    mode = 0
    DPI620G.setControlMode(mode)
    dpiMode = DPI620G.getControlMode()
    pvMode = PV624.readMode()
    pressure, pressureG, baro, sp, pt, newMode = PV624.readAllSlow()
    print(mode, dpiMode, pvMode, newMode)

    mode = 1
    DPI620G.setControlMode(mode)
    dpiMode = DPI620G.getControlMode()
    pvMode = PV624.readMode()
    pressure, pressureG, baro, sp, pt, newMode = PV624.readAllSlow()
    print(mode, dpiMode, pvMode, newMode)

    mode = 2
    DPI620G.setControlMode(mode)
    dpiMode = DPI620G.getControlMode()
    pvMode = PV624.readMode()
    pressure, pressureG, baro, sp, pt, newMode = PV624.readAllSlow()
    print(mode, dpiMode, pvMode, newMode)

    mode = 3
    DPI620G.setControlMode(mode)
    dpiMode = DPI620G.getControlMode()
    pvMode = PV624.readMode()
    pressure, pressureG, baro, sp, pt, newMode = PV624.readAllSlow()
    print(mode, dpiMode, pvMode, newMode)

    DPI620G.setVR(2000)
    DPI620G.getVR()

    operateV1V2(PV624)

    rate = PV624.ReadVentRate()
    print(rate)

    DPI620G.setVR(1000)
    rate = PV624.ReadVentRate()
    print(rate)

    DPI620G.setVR(5000)
    rate = PV624.ReadVentRate()
    print(rate)

    DPI620G.setVR(0.2)
    rate = PV624.ReadVentRate()
    print(rate)

    #Configure vent valve in PWM mode
    PV624.ConfigValve(3, 1)
    PV624.OpenValve3()
    time.sleep(2)
    PV624.CloseValve3()
    time.sleep(2)

    # Test if v1 v2 are still working
    operateV1V2(PV624)

    #Configure vent valve in Time modulation
    PV624.ConfigValve(3, 0)
    while microSec > 500:
        counter = counter + 1
        steps = steps * reverse

        PV624.SetValveTime(microSec)
        PV624.OpenValve3()
        if microSec <= 500:
            microSec = 6000
        else:
            microSec = microSec - 250
        time.sleep(0.1)

    # Test if v1 v2 are still working
    operateV1V2(PV624)

    #Configure vent valve in PWM mode again
    PV624.ConfigValve(3, 1)
    print(1)
    PV624.OpenValve3()
    time.sleep(2)
    print(2)
    PV624.OpenValve3()
    time.sleep(2)
    print(3)
    PV624.OpenValve3()
    time.sleep(2)
    PV624.CloseValve3()
    time.sleep(2)

    # Test if v1 v2 are still working
    operateV1V2(PV624)

    #Configure vent valve in time modulation mode
    PV624.ConfigValve(3, 0)
    microSec = 500
    while microSec < 6000:
        counter = counter + 1
        steps = steps * reverse

        PV624.SetValveTime(microSec)
        PV624.OpenValve3()
        if microSec >= 6000:
            microSec = 500
        else:
            microSec = microSec + 250
        time.sleep(0.1)

    # Test if v1 v2 are still working
    operateV1V2(PV624)

    # Configure vent valve in PWM mode
    PV624.ConfigValve(3, 1)
    PV624.OpenValve3()
    time.sleep(0.07)
    PV624.ConfigValve(3, 1)
    PV624.OpenValve3()
    time.sleep(0.07)
    PV624.ConfigValve(3, 1)
    PV624.OpenValve3()
    time.sleep(0.07)
    PV624.CloseValve3()

    # Test if v1 v2 are still working
    operateV1V2(PV624)
             
main()

