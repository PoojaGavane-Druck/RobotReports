# -*- coding: utf-8 -*-
"""
Created on Fri Jun 18 11:11:45 2021

@author: 212596572
"""


import pv624Lib as pvComms
import time
import traceback
pv624 = []

try:
    pv624 = pvComms.PV624()  # find STM32 main board
    # motor = mtr.Motor()  # find STM32 motor controller
except:
    print('error opening COM devices')
    traceback.print_exc()
    if pv624:
        pv624.ClosePort()
    exit()

try:
    count = 0
    while True:
        count = count + 1
        mode = pv624.readMode()
        # time.sleep(0.005)
        # pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
        # pv624.setControllerStatus(1)
        print(count)
        
except:
    traceback.print_exc()
    if pv624:
        pv624.ClosePort()
