# -*- coding: utf-8 -*-
"""
Created on Wed Jun 16 18:21:46 2021

@author: 212596572
"""

import dpi620gLib as dpi
from datetime import datetime
import time
import csv

def dpiTest():
    DPI620G = dpi.DPI620G()
    samples = 3600
        
    km = DPI620G.getKM()
    print(km)
    DPI620G.setKM('R')
    km = DPI620G.getKM()
    print(km)    
    DPI620G.getRI()
    DPI620G.getDK("0,0")
    DPI620G.getDK("0,1")
    DPI620G.getDK("1,0")
    DPI620G.getDK("1,1")
    DPI620G.getRE()
    DPI620G.getPV()
    DPI620G.getPT()
    DPI620G.getCC()
    DPI620G.getSP()
    DPI620G.getCM()
    DPI620G.getIZ()
    DPI620G.getIS()
    voltage = DPI620G.getRB('0')
    current = DPI620G.getRB('1')
    percentage = DPI620G.getRB('2')
    mAh = DPI620G.getRB('3')
    minsToEmpty = DPI620G.getRB('4')
    dcStatus = DPI620G.getRB('5')
    DPI620G.setSetPoint(12.345)
    sp = DPI620G.getSetPoint()
    print(sp)
    DPI620G.setKM('L')
    km = DPI620G.getKM()
    print(km)     

    dataFile = 'BATTERY_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(dataFile,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        count = 1
        while count < samples:
            hm = datetime.now().strftime("%H:%M")
            sms = datetime.now().strftime("%S.%f")
            voltage = DPI620G.getRB('0')
            current = DPI620G.getRB('1')
            percentage = DPI620G.getRB('2')
            mAh = DPI620G.getRB('3')
            minsToEmpty = DPI620G.getRB('4')
            dcStatus = DPI620G.getRB('5')    
            result = [hm, sms, round(voltage, 3), current, round(percentage, 3), mAh, minsToEmpty, dcStatus]
            print(result)
            csvFile.writerow(result) 
            count = count + 1
            time.sleep(0.5)
            
    
    
dpiTest()