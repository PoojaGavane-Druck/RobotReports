import pv624Lib as pvComms
import dpi620gLib as dpi
import time

pv624sn = ['2047325E5431']
dpi620gSn = ['FTBTBC9KA']

def main():
    DPI620G = dpi.DPI620G(dpi620gSn)
    pv624 = pvComms.PV624(pv624sn)

    DPI620G.setKM('R')
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

    steps = pv624.MOTOR_MoveContinuous(200)
    print(steps)
    time.sleep(0.1)
    steps = pv624.MOTOR_MoveContinuous(-200)
    print(steps)
    time.sleep(0.1)
    steps = pv624.MOTOR_MoveContinuous(0)
    print(steps)

    pv624.OpenValve1()
    time.sleep(0.5)
    pv624.CloseValve1()
    time.sleep(0.5)
    pv624.OpenValve2()
    time.sleep(0.5)
    pv624.CloseValve2()
    time.sleep(0.5)
    pv624.OpenValve3()
    time.sleep(0.5)
    pv624.CloseValve3()

main()