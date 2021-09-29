# -*- coding: utf-8 -*-
"""
Created on Fri Jun 18 11:09:59 2021

@author: 212596572
"""


import dpi620gLib as dpi
import time

DPI620G = dpi.DPI620G()  # connect to PV624 OWI GENII port
DPI620G.setKM('R')  # set PV624 GENII interface to remote mode
time.sleep(0.1)
DPI620G.setControlMode(2)  # vent
DPI620G.setPressureType(2)  # measure barometer
baroP = int(DPI620G.getPressure())  # baro pressure, mbar
DPI620G.setPressureType(0)  # read gage pressure
DPI620G.setSetPoint(0)  # setpoint == atmospheric pressure
minFS,FS,Ptype = DPI620G.getSensorInfo()
print('Baro Read:', baroP)

while True:
    pressure, pv624Error, pv624Status = DPI620G.getPVInfo()
    controlMode = DPI620G.getControlMode()
    setpoint = DPI620G.getSetPoint()
    pressureType = DPI620G.getPressureType()
    
    result = [pressure, pv624Error, pv624Status, controlMode, setpoint, pressureType]
    
    print(result)