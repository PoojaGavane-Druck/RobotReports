# -*- coding: utf-8 -*-
"""
Created on Fri Aug 27 12:53:34 2021

@author: 212596572
"""


import dpi620gLibDuci as dpi
import pv624Lib as pv624
from datetime import datetime
import time



def main():
    
    DPI620G = dpi.DPI620G()
    
    DPI620G.getDK("1,0")
    DPI620G.getDK("1,1")
    DPI620G.getIS()
    DPI620G.getIZ()
    count = 0
    
    # while count < 10:
    while True:
        
        startTime = datetime.now()
        pressure, pv624Error, pv624Status = DPI620G.getPVInfo()
        controlMode = DPI620G.getControlMode()
        setpoint = DPI620G.getSetPoint()
        pressureType = DPI620G.getPressureType()
        endTime = datetime.now()
        elapsedTime = (endTime - startTime).total_seconds()
        print(count, controlMode, pressure, pv624Error, pv624Status, setpoint, pressureType, str(round(elapsedTime * 1000, 2)) + "ms")
        time.sleep(0.03)
        count = count + 1
    
    DPI620G.closePort()
    
main()