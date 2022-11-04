import dpi620gLib as dpi
import csv
import time
from datetime import datetime
#dpi620gSn = ['2047325E5431']
dpi620gSn = ['FTBTBC9KA']
filterCoeff = 0.8

def run():
    DPI620G = dpi.DPI620G(dpi620gSn)

    pressure, error, status, baro = DPI620G.getPV()

    setPoint = pressure * 0.9

    DPI620G.setKM('R')
    DPI620G.setSP(setPoint)
    sp = DPI620G.getSP()
    DPI620G.setCM(1)

    rangeExceeded = 0
    cycleComplete = 0

    while True:
        pressure, error, status, baro = DPI620G.getPV()

        if ((status & 0x10000) == 0x10000) and (cycleComplete == 0):
            rangeExceeded = rangeExceeded + 1
            print(pressure, error, status, baro, rangeExceeded)
            time.sleep(0.2)

run()


