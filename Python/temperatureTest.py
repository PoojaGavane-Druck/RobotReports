import dpi620gLib as dpi
import pv624Lib as pvComms
import dpiAttributes as dpiAttr
from datetime import datetime
import time
import csv
import random

dpi620gSn = ['205C324B5431']

def temperatureTest():

    pistonCentered = 0
    count = 0
    noSteps = 0
    noStepsMax = 0
    noStepsMin = 0
    noStepsChange = 0
    pumpTimeDelay = 0
    exitTest = 0

    print("Starting temperature tests for PV624")
    try:
        pumpTimeDelay = int(input("Enter pump time: "))
    except:
        print("Invalid input")
        exitTest = 1

    if (pumpTimeDelay < 1) or (pumpTimeDelay > 50):
        print("Pump time delay is out of range, exiting test")
        exitTest = 1
    
    if exitTest == 0:
        print("Enter motor steps pattern for set point operation")
        print("1. Fixed")
        print("2. Incremental")
        print("3. Decremental")
        print("4. Random")
        try:
            pattern = int(input("Enter your choice: "))
        except:
            print("Invalid input")
            exitTest = 1    

        if 1 == pattern:
            try:
                noSteps = int(input("Enter fixed number of steps: "))
                if (noSteps > 500) or (noSteps < -500):
                    print("Entered no of steps is out of range. Exiting test")
                    exitTest = 1
            except:
                print("Invalid input")
                exitTest = 1

        elif 2 == pattern:
            try:
                noStepsMax = int(input("Enter max number of steps until increment: "))
            except:
                print("Invalid input")
                exitTest = 1
            try:
                noStepsMin = int(input("Enter min number of steps to start operation: "))
            except:
                print("Invalid input")
                exitTest = 1
            try:
                noStepsChange = int(input("Enter no of steps to change every cycle: "))
            except:
                print("Invalid input")
                exitTest = 1
            if exitTest == 0:
                if (noStepsMax > 500):
                    print("Entered no of max steps is out of range. Exiting test")
                    exitTest = 1
                if (noStepsMin < -500):
                    print("Entered no of min steps is out of range. Exiting test")
                    exitTest = 1
                if (noStepsChange < 1) or (noStepsChange > 50):
                    print("Entered no of changed steps is out of range. Exiting test")
                    exitTest = 1
        elif 3 == pattern:
            try:
                noStepsMax = int(input("Enter max number of steps until increment: "))
            except:
                print("Invalid input")
                exitTest = 1
            try:
                noStepsMin = int(input("Enter min number of steps to start operation: "))
            except:
                print("Invalid input")
                exitTest = 1
            try:
                noStepsChange = int(input("Enter no of steps to change every cycle: "))
            except:
                print("Invalid input")
                exitTest = 1
            if exitTest == 0:
                if (noStepsMax > 500):
                    print("Entered no of max steps is out of range. Exiting test")
                    exitTest = 1
                if (noStepsMin < -500):
                    print("Entered no of min steps is out of range. Exiting test")
                    exitTest = 1
                if (noStepsChange < 1) or (noStepsChange > 50):
                    print("Entered no of changed steps is out of range. Exiting test")
                    exitTest = 1  
        elif 4 == pattern:
            try:
                noStepsMax = int(input("Enter max number of steps until increment: "))
            except:
                print("Invalid input")
                exitTest = 1
            try:
                noStepsMin = int(input("Enter min number of steps to start operation: "))
            except:
                print("Invalid input")
                exitTest = 1
            if exitTest == 0:
                if (noStepsMax > 500):
                    print("Entered no of max steps is out of range. Exiting test")
                    exitTest = 1
                if (noStepsMin < -500):
                    print("Entered no of min steps is out of range. Exiting test")
                    exitTest = 1
                if noStepsMax <= noStepsMin:
                    print("Entered no of min steps is out of range. Exiting test")
                    exitTest = 1

    if exitTest == 0:
        DPI620G = dpi.DPI620G(dpi620gSn, "NO ECHO")
        # Remote mode set
        print("Resetting PV624...")
        DPI620G.getPT()
        print("Test will start in 20 seconds...")
        timeDelay = 20
        while timeDelay > 0:
            time.sleep(1)
            print(str(timeDelay) + " ", end = "")
            timeDelay = timeDelay - 1

        DPI620G = dpi.DPI620G(dpi620gSn, "NO ECHO")
        DPI620G.setKM('R')  
        keyMode = DPI620G.getKM()
        count = 1
        dataFile = 'TEMPERATURE_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
        print("Starting PV624 temperature tests")
        print("While running the test press CTRL+C to close the test at any time")
        with open(dataFile,'w',newline='') as f:
            csvFile = csv.writer(f,delimiter = ',')
            result = ['Count', 'Elapsed Time', 'Steps', 'Voltage', 'Current', 'Percentage', 'Remaining Capacity', 'Time To Empty', 'Charging State', 'Battery Temperature']
            csvFile.writerow(result) 
            
            if pattern == 1:
                steps = noSteps
            elif pattern == 2:
                steps = noStepsMin
                if steps >= noStepsMax:
                    steps = noStepsMax
            elif pattern == 3:
                steps = noStepsMax
                if steps <= noStepsMin:
                    steps = noStepsMin
            elif pattern == 4:
                steps = random.randint(noStepsMin, noStepsMax)
            try: 
                while True:
                    startTime = time.time()
                    DPI620G.setTS(steps)
                    DPI620G.setCM('0')
                    #print("Mode = Measure")
                    print("Pumping, press CTRL+C to end test ", end = "")
                    timeDelay = pumpTimeDelay
                    while timeDelay > 0:
                        time.sleep(1)
                        print(str(timeDelay) + " ", end = "")
                        timeDelay = timeDelay - 1
                    print("Running ", end = "")
                    DPI620G.setCM('1')
                    #print("Mode = Control")
                    pressure, error, status, baro = DPI620G.getPV()
                    pistonCentered = status & 4096
                    #print("1", hex(status), pistonCentered)

                    while pistonCentered == 4096:
                        pressure, error, status, baro = DPI620G.getPV()
                        pistonCentered = status & 4096
                        #print("2", hex(status), pistonCentered)

                    while pistonCentered != 4096:
                        pressure, error, status, baro = DPI620G.getPV()
                        pistonCentered = status & 4096
                        #print("3", hex(status), pistonCentered)

                    voltage = DPI620G.getRB('0')
                    current = DPI620G.getRB('1')
                    percentage = DPI620G.getRB('2')
                    remCap = DPI620G.getRB('3')
                    timeToEmpty = DPI620G.getRB('4')
                    dcPresent = DPI620G.getRB('5')
                    battTemperature = DPI620G.getRB('6')
                    endTime = time.time()
                    elapsedTime = endTime - startTime
                    result = [count, elapsedTime, steps, voltage, current, percentage, remCap, timeToEmpty, dcPresent, battTemperature]
                    csvFile.writerow(result)
                    print(result)
                    DPI620G.setCM('0')
                    count = count + 1
                    if pattern == 2:
                        steps = steps + noStepsChange
                        if steps >= noStepsMax:
                            steps = noStepsMax
                    elif pattern == 3:
                        steps = steps - noStepsChange
                        if steps <= noStepsMin:
                            steps = noStepsMin
                    elif pattern == 4:
                        steps = random.randint(noStepsMin, noStepsMax)
            except KeyboardInterrupt:
                print("Closing files and ending test")

    if exitTest == 0:
        DPI620G.setCM('0')
        DPI620G.ClosePort()
        f.close()
    print("Test ended")
        
      
temperatureTest()