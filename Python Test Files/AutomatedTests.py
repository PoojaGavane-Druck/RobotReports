# -*- coding: utf-8 -*-
"""
Created on Thu Aug 12 14:44:33 2021

@author: 212596572
"""
import dpi620gLib as dpi
from datetime import datetime
import time
import csv

printIt = 1

def writeStr(file, value, printState):
    file.write(value + "\n")
    if(printState == 1):
        print(value)

def testDuciCommands(phase):
    fileName = 'DUCI_COMMANDS_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'
    file = open(fileName, "w")
    try:
        DPI620G = dpi.DPI620G()

        
        if((phase == "Phase 1") or (phase == "All")):
            writeStr(file, "Phase 1 commands------------------------\n", printIt)
            writeStr(file, "Get commands----------------------------\n", printIt)
            msg, km = DPI620G.getKM()
            writeStr(file, "KM = " + str(km) + "\n", printIt)
            msg = DPI620G.getRI()
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getDK("0,0")
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getDK("0,1")
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getDK("1,0")
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getDK("1,1")
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getPV()
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getPT()
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getCC()
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getSP()
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getCM()
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getIZ()
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg = DPI620G.getIS()
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg, voltage = DPI620G.getRB('0')
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)
            msg, current = DPI620G.getRB('1')
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)        
            msg, percentage = DPI620G.getRB('2')
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)        
            msg, mAh = DPI620G.getRB('3')
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)        
            msg, minsToEmpty = DPI620G.getRB('4')
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)        
            msg, dcStatus = DPI620G.getRB('5')
            writeStr(file, msg, printIt)
            msg = DPI620G.getRE()
            writeStr(file, msg, printIt)        
            msg = DPI620G.setKM('R')
            writeStr(file, msg, printIt)
            msg, km = DPI620G.getKM()
            writeStr(file, "KM = " + str(km) + "\n\r", printIt)
            writeStr(file, "Set commands----------------------------\n\r", printIt)
            msg = DPI620G.setSD("08/12/2021")
            writeStr(file, msg, printIt)
            msg = DPI620G.getSD()
            writeStr(file, msg, printIt)
            msg = DPI620G.setST("12:34:56")
            writeStr(file, msg, printIt)
            msg = DPI620G.getST()     
            writeStr(file, msg, printIt)
            msg = DPI620G.setSP("78.124")
            writeStr(file, msg, printIt)
            msg = DPI620G.getSP()     
            writeStr(file, msg, printIt)
            msg = DPI620G.setPT("0")
            writeStr(file, msg, printIt)
            msg = DPI620G.getPV()
            writeStr(file, msg, printIt)
            msg = DPI620G.setPT("1")
            writeStr(file, msg, printIt)
            msg = DPI620G.getPV()
            writeStr(file, msg, printIt) 
            msg = DPI620G.setPT("2")
            writeStr(file, msg, printIt)
            msg = DPI620G.getPV()
            writeStr(file, msg, printIt) 
            msg = DPI620G.setCM("0")
            writeStr(file, msg, printIt)            
            msg = DPI620G.getCM()
            writeStr(file, msg, printIt)    
            msg = DPI620G.setCM("1")
            writeStr(file, msg, printIt)            
            msg = DPI620G.getCM()
            writeStr(file, msg, printIt)
            msg = DPI620G.setCM("2")
            writeStr(file, msg, printIt)            
            msg = DPI620G.getCM()
            writeStr(file, msg, printIt)            
            writeStr(file, "Pin commands----------------------------\n\r", printIt)
            msg = DPI620G.setPP("800")
            writeStr(file, msg, printIt)
            msg = DPI620G.getPP()
            writeStr(file, msg, printIt) 
            msg = DPI620G.setSN("84956273")
            writeStr(file, msg, printIt)
            msg = DPI620G.getSN('0')
            writeStr(file, msg, printIt)  
            msg = DPI620G.getSN('1')
            writeStr(file, msg, printIt) 
            msg = DPI620G.setPP("000")
            writeStr(file, msg, printIt)
            msg = DPI620G.getPP()
            writeStr(file, msg, printIt)    
            msg = DPI620G.setKM('L')
            writeStr(file, msg, printIt)
            msg, km = DPI620G.getKM()
            writeStr(file, "KM = " + str(km) + "\n\r", printIt)        
            
        if((phase == "Phase 2") or (phase == "All")):
            print("No commands for phase 2")
            
        if((phase == "Phase 3") or (phase == "All")):
            print("No commands for phase 3")
    
    finally:
        file.close()
        DPI620G.closePort()

def batteryTest(samples, sampleTime):
    DPI620G = dpi.DPI620G()
    
    dataFile = 'BATTERY_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(dataFile,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        count = 1
        while count < samples:
            tic = datetime.now()
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
            elapsedTime = (datetime.now() - tic).total_seconds()
            if(elapsedTime < sampleTime):
                sleepTime = sampleTime - elapsedTime
                time.sleep(sleepTime)
            
    dataFile.close()
    DPI620G.closePort()
    
def main():    
    print("Select test: ")
    print("1. DUCI Commands Phase 1")
    print("2. DUCI Commands Phase 2")
    print("3. DUCI Commands Phase 3")
    print("4. DUCI Commands All")
    print("5. Battery Test")
    
    test = input("Enter your choice: ")
    
    if(test == "1"):
        print("Testing phase 1 DUCI commands")
        testDuciCommands("Phase 1")
    elif(test == "2"):
        print("Testing phase 2 DUCI commands")
        testDuciCommands("Phase 2")
    elif(test == "3"):
        print("Testing phase 3 DUCI commands")
        testDuciCommands("Phase 3")
    elif(test == "4"):
        print("Testing ALL DUCI commands")
        testDuciCommands("All")
    elif(test == "5"):
        print("Starting Battery test")
        sampleStr = input("Enter number of samples: ")
        sampleTimeStr = input("Enter sampling rate (s): ")
        
        samples = int(sampleStr)
        sampleTime = int(sampleTimeStr)
        
        batteryTest(samples, sampleTime)
        
main()
    