import dpi620gLibDuci as dpi
import pv624Lib as pv624
from datetime import datetime
import time
import csv
import random
    
def main():   
    PV624 = pv624.PV624()
    sm = 0
    steps = 20
    counter = 0
    while True:
        counter = counter + 1
        steps = steps * -1
        PV624.MOTOR_MoveContinuous(steps)
        pressure, pressureG, atmPressure, setPoint, spType, mode = PV624.readAllSlow()
        print(counter, pressure, pressureG, atmPressure, setPoint, spType, mode)
        time.sleep(0.01)
        '''
        # Valve state machine
        if sm == 0:
            PV624.OpenValve1()
            sm = 1
        elif sm == 1:
            PV624.CloseValve1()
            sm = 2
        elif sm == 2:
             PV624.OpenValve2()
             sm = 3
        elif sm == 3:
            PV624.CloseValve2()
            sm = 4
        elif sm == 4:
            PV624.OpenValve3()
            sm = 5
        elif sm == 5:
             PV624.CloseValve3()
             sm = 0
             '''

main()