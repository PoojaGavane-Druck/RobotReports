import dpi620gLibDuci as dpi
import pv624Lib as pv624
from datetime import datetime
import time
import csv
import random

walkSteps = 400
def runTest():
    iterations = 0
    totalSteps = 0
    setSteps = 0
    readSteps = 0
    opt1 = 1
    opt2 = 1
    count = 0
    state = 0
    rate = 0.05
    todayDate = 0
    todayTime = 0
    PV624 = pv624.PV624()
    # Assume motor is at centre
    fileName = 'LIFE_TEST_' + str(1) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(fileName,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        result = ['Count', 'State', 'Date', 'Time', 'Iterations', 'Set Steps', 'Read Steps', 'Total Steps', 'Optical Sensor 1', 'Optical Sensor 2']
        csvFile.writerow(result) 
        # Random walk to extreme end, not sensing optical sensor now as there is no setup
        opt1, opt2 = PV624.readDigitalOptSensors()
        while opt1 == 1:
            setSteps = 3000
            readSteps = PV624.MOTOR_MoveContinuous(setSteps)
            opt1, opt2 = PV624.readDigitalOptSensors()

        time.sleep(2)

        while count < 100:
            count = count + 1
            while totalSteps > -25000:
                state = 1
                iterations = iterations + 1
                setSteps = -3000
                readSteps = PV624.MOTOR_MoveContinuous(setSteps)
                totalSteps = totalSteps + readSteps
                opt1, opt2 = PV624.readDigitalOptSensors()
                time.sleep(rate)

            todayDate = datetime.now().strftime('%Y-%m-%d')
            todayTime = datetime.now().strftime('%H-%M-%S')
            result = [count, state, todayDate, todayTime, iterations, setSteps, readSteps, totalSteps, opt1, opt2]
            print(result)
            csvFile.writerow(result) 
            # Random walk to other end, not sensing optical sensor now as there is no setup
            while opt2 == 1:
                state = 2
                iterations = iterations + 1    
                setSteps = random.randint(-walkSteps, -1)
                readSteps = PV624.MOTOR_MoveContinuous(setSteps)
                totalSteps = totalSteps + readSteps
                # Read optical sensors too, at this moment this is of no use 04-03-2022
                opt1, opt2 = PV624.readDigitalOptSensors()
                time.sleep(rate)
            todayDate = datetime.now().strftime('%Y-%m-%d')
            todayTime = datetime.now().strftime('%H-%M-%S')
            result = [count, state, todayDate, todayTime, iterations, setSteps, readSteps, totalSteps, opt1, opt2]
            print(result)
            csvFile.writerow(result) 

            time.sleep(2)

            # Centre the motor position again
            while totalSteps < -25000:
                state = 3
                iterations = iterations + 1
                setSteps = 3000
                readSteps = PV624.MOTOR_MoveContinuous(setSteps)
                totalSteps = totalSteps + readSteps
                opt1, opt2 = PV624.readDigitalOptSensors()
                time.sleep(rate)
            todayDate = datetime.now().strftime('%Y-%m-%d')
            todayTime = datetime.now().strftime('%H-%M-%S')
            result = [count, state, todayDate, todayTime, iterations, setSteps, readSteps, totalSteps, opt1, opt2]
            print(result)
            csvFile.writerow(result) 

            while opt1 == 1:
                state = 4
                iterations = iterations + 1    
                setSteps = random.randint(1, walkSteps)
                readSteps = PV624.MOTOR_MoveContinuous(setSteps)
                totalSteps = totalSteps + readSteps
                opt1, opt2 = PV624.readDigitalOptSensors()
                time.sleep(rate)

            todayDate = datetime.now().strftime('%Y-%m-%d')
            todayTime = datetime.now().strftime('%H-%M-%S')
            result = [count, state, todayDate, todayTime, iterations, setSteps, readSteps, totalSteps, opt1, opt2]
            print(result)
            csvFile.writerow(result) 

            time.sleep(2)
            
def centre():
    PV624 = pv624.PV624()
    opt1 = 1
    opt2 = 1
    rate = 0.05
    totalSteps = 0

    opt1, opt2 = PV624.readDigitalOptSensors()
    while opt1 == 1:
        setSteps = 3000
        readSteps = PV624.MOTOR_MoveContinuous(setSteps)
        opt1, opt2 = PV624.readDigitalOptSensors()
        time.sleep(rate)

    while opt2 == 1:
        setSteps = -200
        print(setSteps)
        readSteps = PV624.MOTOR_MoveContinuous(setSteps)
        opt1, opt2 = PV624.readDigitalOptSensors()
        totalSteps = totalSteps + readSteps
        time.sleep(rate)

    print(totalSteps)
    

def main():
    count = 0
    pos = 0
    samples = 500
    testNo = 1
    
    PV624 = pv624.PV624()

    opt1, opt2 = PV624.readDigitalOptSensors()
    print(opt1, opt2)
    steps = PV624.MOTOR_MoveContinuous(200)
    print(steps)

#main()

#centre()

runTest()

def timePrint():
    print(datetime.now().strftime('%Y-%m-%d'))
    print(datetime.now().strftime('%H-%M-%S'))

#timePrint()