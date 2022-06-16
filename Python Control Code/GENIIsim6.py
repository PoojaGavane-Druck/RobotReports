#GENII simulator for algorithm testing and demo
#A May
#23 May 2021
#triggers reference PM logging with readRefPM.py
#at last 30s of each setpoint
#readRefPM.py must be running concurrently in another python process
#TBD, start holdTime count only after pump up/down complete
#TBD, log PV624 data to file (maybe)?
# 17 Jun
# removed bugs

# Sept 21
# updated status code definitions to meet DUCI spec

# Oct 28
# updated status code definitions to meet updated spec for in-loop controller

#  17 Nov 2021
# replaced calls to DPI620G.getPressure() with DPI620G.getPVInfo()
# to make compatible with latest DUCI command spec

# 5 Apr 2022
# fixed status interpretation

# 6 Apr
# update to pass DPI interface SN
# update to add controlled vent rate mode

# Apr 12
# initial mode == measure

# Jun 8 2022
# updated status parsing and action display to meet DUCI spec
# this code is used only with eng mode fw
# to be made obsolete once eng mode toggle enabled in released 624 fw build

import PySimpleGUIQt as sg
import dpi620gLib_oldPV as dpi
import time
import random
from datetime import datetime
import csv
from dataStructures4 import *

DPI620G = dpi.DPI620G(screw['DPI'])  # connect to PV624 OWI GENII port
controlModes = {0: 'Measure', 1: 'Control', 2: 'Vent', 3: 'Rate'}
setpointTypes = {0: 'Gauge', 1: 'Abs'}

# define GUI
layout = [
    [sg.Frame(layout=[
        [sg.Text('Hold Time (s)', size=(8, 1)), sg.InputText('60', size=(5, 1)),
         sg.Button(button_text='START', size=(17, 1)),
         sg.Button(button_text='EXIT', size=(17, 1))],
        [sg.Listbox(values=('Random Pressure',
                            'Random Vacuum'),
                    size=(20, 2),
                    default_values='Random Pressure'),
         sg.Text('Auto Timer', size=(10, 1)), sg.Text('---', size=(10, 1), key='Auto Timer')]],
        title='Automatic Setpoint Control', title_color='white', relief=sg.RELIEF_SUNKEN)],
    [sg.Frame(layout=[
        [sg.Text('Setpoint', size=(6, 1)), sg.InputText('0', size=(10, 1)),
         sg.InputCombo(('mbar G', 'mbar'), size=(10, 1)),
         sg.Button(button_text='UPDATE SETPOINT', size=(17, 1))],
        [sg.Button(button_text='MEASURE', size=(17, 1)),
         sg.Button(button_text='CONTROL', size=(17, 1)),
         sg.Button(button_text='VENT', size=(17, 1))]],
        title='Pressure Control', title_color='white', relief=sg.RELIEF_SUNKEN)],
[sg.Frame(layout=[
        [sg.Text('Rate', size=(6, 1)), sg.InputText('10', size=(10, 1)),
         sg.Button(button_text='UPDATE RATE', size=(15, 1)),
         sg.Button(button_text='CONTROL RATE', size=(15, 1))]],
        title='Rate Control', title_color='white', relief=sg.RELIEF_SUNKEN)],
    [sg.Frame(layout=[
        [sg.Text('PM FS Pressure', size=(15, 1)), sg.Text('---', size=(10, 1), key='PM FS Pressure'),
         sg.Text('PM Type', size=(15, 1)), sg.Text('---', size=(10, 1),key='PM Type')],
        [sg.Text('Pressure Reading', size=(15, 1)), sg.Text('---', size=(10, 1), key='Pressure Reading'),
         sg.Text('Setpoint Type', size=(15, 1)), sg.Text('---', size=(10, 1), key='Setpoint Type')],
        [sg.Text('Control Mode', size=(15, 1)), sg.Text('---', size=(10, 1), key='Control Mode'),
         sg.Text('Status Code', size=(15, 1)), sg.Text('---', size=(10, 1), key='Status Code')],
        [sg.Text('Status', size=(15, 1)), sg.Text('---', size=(10, 1), key='Action'),
         sg.Text('Setpoint Mode', size=(15, 1)), sg.Text('---', size=(10, 1), key='Setpoint Mode')],
        [sg.Text('Setpoint', size=(15, 1)), sg.Text('---', size=(10, 1), key='Setpoint'),
         sg.Text('Rate', size=(15, 1)), sg.Text('---', size=(10, 1), key='Rate')]
        ],
        title='Outputs', title_color='white', relief=sg.RELIEF_SUNKEN)]]

window = sg.Window('PV624 GENII Simulator V6.0', layout, default_element_size=(200, 1), grab_anywhere=False)

event = []
values = []
setPointTypes = {'mbar G': 0, 'mbar': 1}
setpointMode = 'manual override'
burstTime = 10 #duration of reference PM recording, should match value in readRefPM.py (s)
elapsedTime = 0
burstEnabled = False #log reference PM with readRefPM.py for last burstTime of each auto setpoint
swapFile = 'swap.csv' #swap file for triggering reference PM log used by readRefPM.py
# also contains the filename to be used for the ref PM log
newAutoSetpoint = False  # automatically change setpoint at end of each hold time

minP = 100  # minimum absolute pressure allowed (mbar)
baroUncertainty = 50  # max error between baro and vented PM absolute pressure reading (mbar)
# baroUncertainty prevents automatic setpoints from switching between vacuum or pressure sourcing requirements
statusCodes = {}  # meaning of status codes from PV624 status byte

#  status codes
#  each key is bit location in uint32 value
#  ordered by increasing priority of display in GUI Action output
statusCodes['0x8'] = 'Venting'
statusCodes['0x20'] = 'Vented'
statusCodes['0x400'] = 'Measure'
statusCodes['0x4'] = 'Control'
statusCodes['0x80'] = 'ExcessVolume'
statusCodes['0x10'] = 'Stable'
statusCodes['0x1'] = 'Pump Up'
statusCodes['0x2'] = 'Pump Down'
statusCodes['0x200'] = 'ExcessOffset'

if DPI620G:
    DPI620G.setKM('R')  # set PV624 GENII interface to remote mode
    time.sleep(0.1)
    DPI620G.setControlMode(0)  # measure
    DPI620G.setPressureType(2)  # measure barometer
    baroP, pv624Error, pv624Status = DPI620G.getPVInfo()
    DPI620G.setPressureType(0)  # read gage pressure
    DPI620G.setSetPoint(0)  # setpoint == atmospheric pressure
    minFS, FS, Ptype = DPI620G.getSensorInfo()
    print('Baro Read:', baroP)

    while event != 'EXIT':
        event, values = window.read(timeout=100)  # add GUI timeout force while loop execution

        # read parameters from PV624
        pressure, pv624Error, pv624Status = DPI620G.getPVInfo()
        controlMode = DPI620G.getControlMode()
        setpoint = DPI620G.getSetPoint()
        pressureType = DPI620G.getPressureType()
        rate = DPI620G.getVR()

        # update PV624 parameters on GUI display
        window['Pressure Reading'].update(round(pressure, 2))
        window['Setpoint Type'].update(setpointTypes[pressureType])
        window['Control Mode'].update(controlModes[controlMode])
        window['Status Code'].update(hex(pv624Status) + ' ' + hex(pv624Error))

        action = '----'
        for key in statusCodes:
            hexKey = int(key, base=16)  # convert key string to its hex-encoded value
            if (hexKey & pv624Status) != 0:  # mask pv624Status value with key
                action = statusCodes[key]

        window['Action'].update(action)
        window['Setpoint Mode'].update(setpointMode)
        window['PM FS Pressure'].update(FS)
        window['PM Type'].update(Ptype)
        window['Setpoint'].update(setpoint)
        window['Rate'].update(rate)
        window['Auto Timer'].update(round(elapsedTime, 1))

        # poll for GUI events
        if event:
            if event == 'VENT':
                DPI620G.setControlMode(2)
                setpointMode = 'manual override'
                elapsedTime = 0

            elif event == 'MEASURE':
                DPI620G.setControlMode(0)
                setpointMode = 'manual override'
                elapsedTime = 0

            elif event == 'CONTROL':
                DPI620G.setControlMode(1)
                setpointMode = 'manual override'
                elapsedTime = 0

            elif event == 'UPDATE SETPOINT':
                manualSetpoint = int(values[2])
                manualSetpointType = setPointTypes[values[3]]
                DPI620G.setControlMode(0)  # set to measure mode on any setpoint change
                DPI620G.setSetPoint(manualSetpoint)
                DPI620G.setPressureType(manualSetpointType)
                setpointMode = 'manual override'
                elapsedTime = 0

            elif event == 'CONTROL RATE':
                DPI620G.setControlMode(3)  # set to rate control mode
                setpointMode = 'manual rate'
                elapsedTime = 0

            elif event == 'UPDATE RATE':
                manualRate = abs(int(values[4]))
                DPI620G.setControlMode(0)  # set to measure mode on any setpoint change
                DPI620G.setVR(manualRate)
                setpointMode = 'manual rate'
                elapsedTime = 0

            elif event == 'START' or newAutoSetpoint:
                DPI620G.setControlMode(0)  # set measure mode
                time.sleep(0.2)  # give time for PV624 to see mode change
                DPI620G.setPressureType(2)  # measure barometer
                baroP, pv624Error, pv624Status = DPI620G.getPVInfo()
                print('Baro Read:', baroP)
                newAutoSetpoint = False
                burstEnabled = True
                startTime = datetime.now()
                # filename for the reference PM data log
                refDataFile = 'R_' + datetime.now().strftime('%Y-%m-%dT%H-%M-%S')
                setpointMode = values[1][0]
                holdTime = int(values[0])

                if setpointMode == 'Random Pressure':
                    autoSetpointType = random.randint(0, 1)  # randomly do gage (0) or abs (1) pressure
                    # define a random setpoint in PMs FS range (mbar or mbar G), nominally 50 to FS mbar G
                    autoSetpoint = random.randint(autoSetpointType * baroP + baroUncertainty, FS)
                    # 25/75 weighed average new setpoint with current one to limit how quickly it can change
                    # and make it achievable in < 30 s during automated testing
                    # e.g. for 20 barG FS old setpoint == 0 --> new setpoint <= 5 +/- 1
                    # and old setpoint == 20 --> new setpoint >= 15 +/- 1
                    # with uncertainty caused by potential mismatch in old vs. new setpoint type (gage or abs)
                    autoSetpoint = int(autoSetpoint * 0.25 + setpoint * 0.75)

                    if autoSetpoint < baroP + baroUncertainty and autoSetpointType == 1:
                        # weighted average has resulted in an invalid absolute pressure (too small),
                        # make this setpoint a gage pressure to fix the problem
                        autoSetpointType = 0

                    DPI620G.setPressureType(autoSetpointType)
                    DPI620G.setSetPoint(autoSetpoint)
                    DPI620G.setControlMode(1)  # set to control mode
                    setpoint = DPI620G.getSetPoint()

                elif setpointMode == 'Random Vacuum':
                    autoSetpointType = random.randint(0, 1)  # randomly do gage (0) or abs (1) pressure
                    DPI620G.setPressureType(autoSetpointType)

                    # define a random setpoint in PMs FS range (mbar or mbar G), nominally -900 to -50 mbar G
                    autoSetpoint = random.randint((autoSetpointType - 1) * 1000 + minP,
                                                  autoSetpointType * baroP - baroUncertainty)

                    DPI620G.setSetPoint(autoSetpoint)
                    DPI620G.setControlMode(1)  # control mode
                    setpoint = DPI620G.getSetPoint()

            if setpointMode == 'Random Pressure' or setpointMode == 'Random Vacuum':
                elapsedTime = (datetime.now() - startTime).total_seconds()

                if (holdTime - elapsedTime) < 0 and holdTime != 0:
                    print('end burst')
                    newAutoSetpoint = True

                elif (holdTime - elapsedTime) < burstTime and burstEnabled and holdTime != 0:
                    newAutoSetpoint = False
                    burstEnabled = False
                    refDataFile = str(autoSetpoint) + '_' + str(autoSetpointType) + '_' + refDataFile + '.csv'

                    try:
                        with open(swapFile, 'w', newline='') as f:
                            csvFile = csv.writer(f, delimiter=',')
                            csvFile.writerow([refDataFile])

                    except:
                        print('Error creating swap file for: ', refDataFile)

            else:
                enablePV624Read = True
                newAutoSetpoint = False

    if DPI620G:
        DPI620G.ClosePort()
    window.close()



