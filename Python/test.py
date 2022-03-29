# -*- coding: utf-8 -*-
"""
Created on Fri Aug 27 12:53:34 2021

@author: 212596572
"""


import dpi620gLibDuci as dpi
import pv624Lib as pv624
from datetime import datetime
import time
import csv
import random

def testTPCommands():
    ''' this function tests the TP commands'''
    myTester = dpi.DPI620G()
    #myTester.setTP('20')
    #time.sleep(2)
    #myTester.setTP('3')
    
    count = 0
    while count < 10:
        count = count + 1
        myTester.setTP('109=3')
        time.sleep(1)
        myTester.setTP('109=2')
        time.sleep(1)
    
def testDirection():
    count = 0
    pos = 0
    samples = 500
    steps = 3000

    PV624 = pv624.PV624()
    fileName = 'MOTOR_TEST_' + str(1) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(fileName,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        count = 0
        while count < samples:
            pos = PV624.MOTOR_MoveContinuous(steps)
            opt1, opt2 = PV624.readDigitalOptSensors()
            result = [count, steps, pos, opt1, opt2]
            print(result)
            csvFile.writerow(result) 
            count = count + 1
            time.sleep(0.1)
            if opt1 == 0:
                steps = -3000
            elif opt2 == 0:
                steps = 3000

def main():
    count = 0
    pos = 0
    samples = 500
    testNo = 1
    
    PV624 = pv624.PV624()

    fileName = 'MOTOR_TEST_' + str(testNo) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(fileName,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        count = 0
        while count < samples:
            steps = 3000
            pos = PV624.MOTOR_MoveContinuous(steps)
            opt1, opt2 = PV624.readDigitalOptSensors()
            result = [count, steps, pos, opt1, opt2]
            print(result)
            csvFile.writerow(result) 
            count = count + 1
            time.sleep(0.1)

    testNo = 2
    fileName = 'MOTOR_TEST_' + str(testNo) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(fileName,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        count = 0
        while count < samples:
            steps = -3000
            pos = PV624.MOTOR_MoveContinuous(steps)
            opt1, opt2 = PV624.readDigitalOptSensors()
            result = [count, steps, pos, opt1, opt2]
            print(result)
            csvFile.writerow(result) 
            count = count + 1
            time.sleep(0.1)

    testNo = 3
    fileName = 'MOTOR_TEST_' + str(testNo) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(fileName,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        count = 0
        while count < samples:
            steps = random.randint(-3000, 0)
            pos = PV624.MOTOR_MoveContinuous(steps)
            opt1, opt2 = PV624.readDigitalOptSensors()
            result = [count, steps, pos, opt1, opt2]
            print(result)
            csvFile.writerow(result) 
            count = count + 1
            time.sleep(0.1)

    testNo = 4
    fileName = 'MOTOR_TEST_' + str(testNo) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(fileName,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        count = 0
        while count < samples:
            steps = random.randint(0, 3000)
            pos = PV624.MOTOR_MoveContinuous(steps)
            opt1, opt2 = PV624.readDigitalOptSensors()
            result = [count, steps, pos, opt1, opt2]
            print(result)
            csvFile.writerow(result) 
            count = count + 1
            time.sleep(0.1)

    testNo = 5
    fileName = 'MOTOR_TEST_' + str(testNo) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(fileName,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        count = 0
        while count < samples:
            steps = random.randint(-3000, 3000)
            pos = PV624.MOTOR_MoveContinuous(steps)
            opt1, opt2 = PV624.readDigitalOptSensors()
            result = [count, steps, pos, opt1, opt2]
            print(result)
            csvFile.writerow(result) 
            count = count + 1
            time.sleep(0.1)

    PV624.closePort()

#main()
#testDirection()
testTPCommands()
