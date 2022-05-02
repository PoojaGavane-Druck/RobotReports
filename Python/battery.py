from tracemalloc import start
import pv624Lib as pvComms
import dpi620gLib as dpi
import dpiAttributes as dpiAttr
import time
from datetime import datetime
import random
import csv

dpi620gSn = ['FTBTA7ISA']

def batteryTest():
    DPI620G = dpi.DPI620G(dpi620gSn)
    voltage = 0
    current = 0
    percentage = 0
    remCap = 0
    timeToEmpty = 0
    dcPresent = 0
    count = 0
    startTime = 0
    endTime = 0
    elapsedTime = 0

    fileName = 'BATTERY_CHARGING_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(fileName,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        result = ['Count', 'Voltage', 'Current', 'Percentage', 'Remaining Capacity', 'Time To Empty', 'Charging State']
        csvFile.writerow(result) 
        startTime = time.time()
        while percentage < 99.9:
            ring, voltage = DPI620G.getRB('0')
            ring, current = DPI620G.getRB('1')
            ring, percentage = DPI620G.getRB('2')
            ring, remCap = DPI620G.getRB('3')
            ring, timeToEmpty = DPI620G.getRB('4')
            ring, dcPresent = DPI620G.getRB('5')
            count = count + 1
            endTime = time.time()
            elapsedTime = endTime - startTime
            result = [count, round(elapsedTime, 2), round(voltage, 2), round(current, 2), round(percentage, 2), remCap, timeToEmpty, dcPresent]
            csvFile.writerow(result) 
            print(result)
            time.sleep(4.35)

batteryTest()