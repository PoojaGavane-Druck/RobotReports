#Andrew May
#Feb 7 2019
#functions to read PACE1000 sensors
#June 11 2019
#added readPACE1000RAW()
#Aug 14 2019
#fixed MAJOR bug in reading of DUT SN
#order of serial numbers now matches order of pressure/diode/frequency readings
#previously they may have been shuffled randomly based on internal mux settings and cable connections
#this does not affect correlation between pressure and frequency/diode but does make it impossible
#to accurately match logged SN with the ones printed on the DUTS, particularly in test DUTs (T2 and T1 PACE 1000).
#Error first detected when the random swap made a referene SN look like a barometer in the 70-bar chair testing Aug 2019
#all other device specific data remains suspect, although separation between test and ref devices is intact (different PACE1000's)
#12/09/2019
#modified PACE1000 IDN search string to include PACE5000
# Dec 3 2020
#Added readPACE1000_FAST() to read pressures with at maximum sample rate
#Dec 8 2020
#debugged readPACE1000_FAST() and added _main() if statement for debugging
#Dec 10 2020
#added reading parameter to readPACE1000_FAST() to optionally read frequency and diode quickly too
# May 5 2020
# updated support for PACE5000

import numpy as np
import serial as ser
import serial.tools.list_ports as prtlst
import re
import traceback
from datetime import datetime
import time

def findPACE1000():
    #checks all COM ports for PACE1000 instruments
    #returns COM port IDs and SN of connected instuments
    queryIDN = '\r*IDN?\r'.encode() #request instrument ID, valid response:
    # *IDN GE Druck,PACE1000,11240594,DK0406  v01.10.12
    # *IDN GE Druck,PACE5000,2765613,DK0367  v03.02.11

    IDNSearchPattern = r'PACE1000,(\d+)' #regex to extract SN information from ID query

    commandCLS = '\r*CLS\r'.encode() #clear all event and condition registers
    #find all available COM ports
    pts = prtlst.comports()
    PACE1000port = []
    PACE1000SN = []
    SN=[]
    port = None
    validCOMdescription = 'USB Serial Port'.lower() #valid description for a COM port

    for pt in pts:
        #check each COM port for a PACE1000 and read its serial number

        if validCOMdescription in pt.description.lower():
            try:
                port = ser.Serial(pt.device,9600,timeout = 0.5)
                print('Looking for PACE1000 instruments on ' + pt.device)
                port.write(commandCLS)
                port.flushInput() #discard read data buffer
                port.write(queryIDN)
                response = (port.readline()).decode('utf-8')
                #print(response)
                match = re.search(IDNSearchPattern,response)
                if match: #found a PACE
                    SN = match.group(1)
                    print('PACE1000 SN ' + SN + ' found on ' + port.port)
                    PACE1000port.append(pt.device)
                    PACE1000SN.append(int(SN))
            except:
                 print('\nCommunication error on ' + str(pt.device) + ':\n')
                 traceback.print_exc()
                 print('\nContinuing\n')

            finally:
                 if port:
                     ser.Serial.close(port)
    if not PACE1000port:
        print('\nWarning: no PACE1000 instruments found\n')

    return PACE1000port,PACE1000SN

def findPACE5000(SN = []):
    #checks all COM ports that match SN for PACE5000 or PACE6000 instruments
    #returns COM port IDs and SN of connected instuments
    queryIDN = '\r*IDN?\r'.encode() #request instrument ID, valid response:
    # *IDN GE Druck,PACE6000,3086601,DK0388  v02.02.05

    IDNSearchPattern = r'PACE[5,6]000,(\d+)' #regex to extract SN information from ID query

    commandCLS = '\r*CLS\r'.encode() #clear all event and condition registers
    #find all available COM ports
    pts = prtlst.comports()
    PACE5000port = []
    PACE5000SN = []
    port = None
    validCOMdescription = 'USB Serial Port'.lower() #valid description for a COM port

    for pt in pts:
        #check each COM port in SN for a PACE5000 and read its serial number
        print(pt.hwid)
        if SN in pt.hwid:
            try:
                port = ser.Serial(pt.device,9600,timeout = 0.5)
                print('Looking for PACE5000 on ' + pt.device)
                port.write(commandCLS)
                #port.flushInput() #discard read data buffer
                port.write(queryIDN)
                response = (port.readline()).decode('utf-8')
                print('response from IDN', response)
                match = re.search(IDNSearchPattern, response)
                if match: #found a PACE
                    PACE5000SN = match.group(1)
                    print('PACE5000 SN ' + PACE5000SN + ' found on ' + port.port)
                    PACE5000port = pt.device
                    return PACE5000port
            except:
                 print('\nCommunication error on ' + str(pt.device) + ':\n')
                 traceback.print_exc()
                 print('\nContinuing\n')
            finally:
                 if port:
                     ser.Serial.close(port)
    if not PACE5000port:
        print('\nWarning: no PACE5000 instruments found\n')

    return PACE5000port

def readPACE1000(PACE1000port = []):
    #reads pressure data from installed sensors in the PACE1000
    #returns pressure reading (mbar) and SN of each sensor

    commandCLS = '\r*CLS\r'.encode() #clear all event and condition registers

    searchPatternSN = r'SN (\d+)' #regex patten to extract sensor SN information
    #example response :INST:SENS2:SN 11368336

    searchPatternPressure = r'([-+]?\d+\.\d+)"' # regex pattern to extract instantaneous pressure reading
    #example response: INST:SENS:READ "-0.89635547638125 -0.885281823804576"
    #SCPI pressure reading is always in mbar, independent of displayed units on front of unit

    pressure = []
    SN = []
    for COMport in PACE1000port:
        print('Reading installed sensors in PACE1000 on ' + COMport)
        try:
            port = ser.Serial(COMport,9600,timeout = 0.5)
            port.write(commandCLS) #clear status register
            port.flushInput() #discard read data buffer


            for n in range(1,4): #check the 3 sensor positions on the PACE1000

                #getSensorID = ('\r:SENS:PRES:RANG' + str(n) + ':SN?\r').encode() #SCPI command to request sensor information - **WRONG COMMAND DO NOT USE!!!**
                getSensorID = ('\r:INST:SENS' + str(n) + ':SN?\r').encode() #SCPI command to request sensor information
                getPressure = ('\r:INST:SENS' + str(n) + ':READ?\r').encode() #SCPI command to request pressure reading

                try:
                    port.write(getSensorID)
                    response = (port.readline()).decode('utf-8')
                    #print(getSensorID.decode())
                    #print(response)
                    match = re.search(searchPatternSN,response)

                    if match:
                        SN.append(int(match.group(1)))
                        #print(match.group(1))

                except:
                    print('Error reading SN from sensor ' + str(n) + ' on' + COMport)
                    traceback.print_exc()
                    print('\n\nContinuing...')
                    pass


                try:
                    port.write(getPressure)
                    response = (port.readline()).decode('utf-8')
                    #print(getPressure.decode())
                    #print(response)
                    match = re.search(searchPatternPressure,response)

                    if match:
                        pressure.append(float(match.group(1)))
                        #print(match.group(1))

                except:
                    print('Error reading pressure from sensor ' + str(n) + ' on' + COMport)
                    traceback.print_exc()
                    print('\n\nContinuing...')
                    pass

        except:
            print('Communication error on ' + str(PACE1000port) + '\n')
            traceback.print_exc()
            print('\n\n\Continuing...')
        finally:
            if port:
                ser.Serial.close(port)

    return SN,pressure

def readPACE5000(PACE5000port = []):
    #reads pressure data from control sensors in the PACE5000
    #returns pressure reading (mbar) and SN

    commandCLS = '\r*CLS\r'.encode() #clear all event and condition registers

    searchPatternSN = r'SN (\d+)' #regex patten to extract sensor SN information
    #example response :INST:SENS1:SN 11368336
    searchPatternPressure = r':SENS:PRES:CONT ([-+]?\d+\.\d+)' # regex pattern to extract instantaneous pressure reading
    #example response: :SENS:PRES:CONT 1.8061859
    #SCPI pressure reading is always in mbar, independent of displayed units on front of unit

    pressure = []
    SN = []
    for COMport in PACE5000port:
        print('Reading installed sensors in PACE5000 on ' + COMport)
        try:
            port = ser.Serial(COMport,9600,timeout = 0.5)
            port.write(commandCLS) #clear status register
            port.flushInput() #discard read data buffer

            getSensorID = ('\r:INST:SENS5:SN?\r').encode() #SCPI command to request control sensor information on PACE5000
            #note: control module is id #1 and #0
            getPressure = ('\rSENS:PRES:CONT?\r').encode() #SCPI command to request pressure reading from control module in PACE5000

            try:
                port.write(getSensorID)
                response = (port.readline()).decode('utf-8')
                #print(getSensorID.decode())
                #print(response)
                match = re.search(searchPatternSN,response)

                if match:
                    SN.append(int(match.group(1)))
                    #print(match.group(1))

            except:
                print('Error reading SN from PACE5000 control sensor on ' + COMport)
                traceback.print_exc()
                print('\n\nContinuing...')
                pass


            try:
                port.write(getPressure)
                response = (port.readline()).decode('utf-8')
                #print(getPressure.decode())
                #print(response)
                match = re.search(searchPatternPressure,response)

                if match:
                    pressure.append(float(match.group(1)))
                    #print(match.group(1))

            except:
                print('Error reading pressure from PACE5000 control sensor on ' + COMport)
                traceback.print_exc()
                print('\n\nContinuing...')
                pass

        except:
            print('Communication error on ' + str(PACE5000port) + '\n')
            traceback.print_exc()
            print('\n\n\Continuing...')
        finally:
            if port:
                ser.Serial.close(port)

    return SN,pressure

def readPACE1000RAW(PACE1000port = []):
    #reads raw data from installed sensors in the PACE1000
    #returns frequency (Hz) and diode (mV)

    commandCLS = '\r*CLS\r'.encode() #clear all event and condition registers

    #searchPatternSN = r'(\d+)"' #regex patten to extract sensor SN information - Not correct
    #example response: :SENS:PRES:RANG:SN "20.00barg 11162883" - Not correct
    searchPatternSN = r'SN (\d+)' #regex patten to extract sensor SN information
    #example response :INST:SENS2:SN 11368336
    searchPatternFrequency = r'(\d+\.\d+)' #regex to extract current frequency reading
    #example response: :DIAG:RPT:FREQ 29459.5239210
    searchPatternDiode = r'(\d+\.\d+)' #regex to extract current frequency reading
    #example response: :DIAG:RPT:DIOD 536.6359651


    frequency = []
    diode = []
    SN = []
    for COMport in PACE1000port:
        print('Reading installed sensors in PACE1000 on ' + COMport)
        try:
            port = ser.Serial(COMport,9600,timeout = 0.5)
            port.write(commandCLS) #clear status register
            port.flushInput() #discard read data buffer


            for n in range(1,4): #check the 3 sensor positions on the PACE1000

                #getSensorID = ('\r:SENS:PRES:RANG' + str(n) + ':SN?\r').encode() #SCPI command to request sensor information **WRONG COMMAND DON'T USE*
                getSensorID = ('\r:INST:SENS' + str(n) + ':SN?\r').encode() #SCPI command to request sensor information
                getFrequency = ('\r:DIAG:RPT' + str(n) + ':FREQ?\r').encode() #SCPI command to request FREQUENCY reading
                getDiode = ('\r:DIAG:RPT' + str(n) + ':DIOD?\r').encode() #SCPI command to request DIODE reading

                try:
                    port.write(getSensorID)
                    response = (port.readline()).decode('utf-8')
                    #print(response)
                    match = re.search(searchPatternSN,response)

                    if match:
                        SN.append(int(match.group(1)))
                        #print(match.group(1))

                except:
                    print('Error reading SN from sensor ' + str(n) + ' on' + COMport)
                    traceback.print_exc()
                    print('\n\nContinuing...')
                    pass


                try:
                    port.write(getFrequency)
                    response = (port.readline()).decode('utf-8')
                    #print(response)
                    match = re.search(searchPatternFrequency,response)

                    if match:
                        frequency.append(float(match.group(1)))
                        #print(match.group(1))

                except:
                    print('Error reading frequency from sensor ' + str(n) + ' on' + COMport)
                    traceback.print_exc()
                    print('\n\nContinuing...')
                    pass

                try:
                    port.write(getDiode)
                    response = (port.readline()).decode('utf-8')
                    #print(response)

                    match = re.search(searchPatternDiode,response)

                    if match:
                        diode.append(float(match.group(1)))
                        #print(match.group(1))

                except:
                    print('Error reading diode from sensor ' + str(n) + ' on' + COMport)
                    traceback.print_exc()
                    print('\n\nContinuing...')
                    pass

        except:
            print('Communication error on ' + str(PACE1000port) + '\n')
            traceback.print_exc()
            print('\n\n\Continuing...')
        finally:
            if port:
                ser.Serial.close(port)

    return SN,frequency,diode


def readPACE1000_FAST(PACE1000port = [],nReadings = 1, parameter = 0):
    #Reads parameter data from installed sensors in one or more PACE1000
    #connected to the serial ports list in PACE1000port
    #parameter = 0 --> read SN and pressure (mbar)
    #parameter = 1 --> read SN and frequency (Hz)
    #parameter = 2 --> read SN and diode voltage (mV)
    #Read is triggered nReadings times with minimum lag between
    #SCPI presure read commands and non-blocking response reads.
    #returns a 2D data array with pressure readings in columns by serial number
    #and first colume as time in s.
    #Typical SR for in 4x PACE1000 (12 sensors) is 0.25 s

    tic = datetime.now() #start time
    commandCLS = '\r*CLS\r'.encode() #clear all event and condition registers
    searchPatternSN = r'SN (\d+)' #regex patten to extract sensor SN information
    #example response :INST:SENS2:SN 11368336
    searchPatternPressure = r'([-+]?\d+\.\d+)"' # regex pattern to extract instantaneous pressure reading
    #example response: INST:SENS:READ "-0.89635547638125 -0.885281823804576"
    #SCPI pressure reading is always in mbar, independent of displayed units on front of unit
    searchPatternFrequency = r'(\d+\.\d+)' #regex to extract current frequency reading
    #example response: :DIAG:RPT:FREQ 29459.5239210
    searchPatternDiode = r'(\d+\.\d+)' #regex to extract current frequency reading
    #example response: :DIAG:RPT:DIOD 536.6359651
    searchPatterns = [searchPatternPressure,searchPatternFrequency,searchPatternDiode]
    parameterNames = ['pressure','frequency','diode']

    readings = []
    readingArray = []
    SN = ['time (s)'] #SN will header in data array, first col is time
    port = []


    try:
        for COMport in PACE1000port:
            #print('Opening ',COMport)
            print('Requesting',parameterNames[parameter],'on',COMport)
            port.append(ser.Serial(COMport,9600,timeout = 0.5))
            port[-1].write(commandCLS) #clear status register
            port[-1].flushInput() #discard read data buffer

        for n in range(0,nReadings): #take nReadings of pressure as quickly as possible
            reading = [] #clear pressure list
            elapsedTime = (datetime.now() - tic).total_seconds() #time since start of test (s)
            reading.append(elapsedTime) #first element of each pressure list is elapsed time
            for COMport in port:
                #request readings from all DUTS connected to the PACE1000 on each port
                #don't wait for a response before moving to the next request (non-blocking)
                for n in range(1,4): #query the 3 sensor positions on the PACE1000 sequentially
                    getPressure = ('\r:INST:SENS' + str(n) + ':READ?\r').encode() #SCPI command to request pressure reading
                    getFrequency = ('\r:DIAG:RPT' + str(n) + ':FREQ?\r').encode() #SCPI command to request FREQUENCY reading
                    getDiode = ('\r:DIAG:RPT' + str(n) + ':DIOD?\r').encode() #SCPI command to request DIODE reading
                    command = [getPressure,getFrequency,getDiode]
                    COMport.write(command[parameter])

            for COMport in port:
                #read back all the responses from the read buffer
                #and append to 1-D list of length len(SN) * nReadings
                for n in range(1,4): #read response from the 3 sensor positions on the PACE1000 sequentially
                    response = (COMport.readline()).decode('utf-8')
                    match = re.search(searchPatterns[parameter],response)
                    if match:
                        reading.append(float(match.group(1)))

            readingArray.append(reading)

        for COMport in port:
            #request serial numbers from all DUTS connected to the PACE1000 on each port
            for n in range(1,4): #check the 3 sensor positions on the PACE1000
                getSensorID = ('\r:INST:SENS' + str(n) + ':SN?\r').encode() #SCPI command to request sensor information
                COMport.write(getSensorID)
                response = (COMport.readline()).decode('utf-8') #wait for a response
                match = re.search(searchPatternSN,response)
                if match:
                        SN.append(int(match.group(1)))

    except:
        print('Communication error on ' + COMport + '\n')
        traceback.print_exc()
        print('\n\n\Continuing...')
    finally:
        for COMport in port:
            ser.Serial.close(COMport)

    return SN, readingArray


def findDPI515():
    #checks all COM ports for DPI515 controller
    #returns COM port IDs and SN of connected instuments
    queryIDN = '\r*IDN?\r'.encode() #request instrument ID, valid response:
    # *IDN Druck,DPI515C,51500581,02.01.00
    IDNSearchPattern = r'DPI515C,(\d+)' #regex patten to extract SN information from ID query
    commandCLS = '\r*CLS\r'.encode() #clear all event and condition registers
    #find all available COM ports
    pts = prtlst.comports()
    DPI515port = []
    DPI515SN = []
    SN=[]
    port = None
    validCOMdescription = 'USB Serial Port'.lower() #valid description for a COM port

    for pt in pts:
        #check each COM port for a PACE1000 and read its serial number

        if validCOMdescription in pt.description.lower():
            try:
                port = ser.Serial(pt.device,9600,timeout = 0.5)
                print('Looking for DPI515 pressure controller on ' + pt.device)
                port.write(commandCLS)
                port.flushInput() #discard read data buffer
                port.write(queryIDN)
                response = (port.readline()).decode('utf-8')
                print(response)
                match = re.search(IDNSearchPattern,response)
                if match: #found a DPI515
                    SN = match.group(1)
                    print('DPI515 SN ' + SN + ' found on ' + port.port)
                    DPI515port.append(pt.device)
                    DPI515SN.append(int(SN))
            except:
                 print('\nCommunication error on ' + str(pt.device) + ':\n')
                 traceback.print_exc()
                 print('\nContinuing\n')

            finally:
                 if port:
                     ser.Serial.close(port)
    if not DPI515port:
        print('\nWarning: no DPI515 controller found\n')

    return DPI515port,DPI515SN


def controlPACE5000(PACE5000Port=[], setpoint=0):
    # set output pressure setpoint to pressure
    # assumes pressure units, limits and pressure type have already been setup correctly
    commandCLS = '\r*CLS\r'.encode()  # clear all event and condition registers
    P = str(round(setpoint, 2))  # pressure as a string to max two decimal places
    commandSetP = ('\r:SOUR ' + P + '\r').encode()
    querySetpoint = '\r:SOUR?\r'.encode()
    commandEnableControl = '\rOUTP:STAT ON\r'.encode()
    if PACE5000Port:
        try:
            port = ser.Serial(PACE5000Port, 9600, timeout=0.5)
            #time.sleep(0.5)
            port.write(commandCLS)
            port.write(commandSetP)
            port.write(commandEnableControl)

        finally:
            if port:
                ser.Serial.close(port)


def measurePACE5000(PACE5000Port=[]):
    # set PACE5000 to measure mode
    commandCLS = '\r*CLS\r'.encode()  # clear all event and condition registers
    commandSetMeasure = '\rOUTP:STAT OFF\r'.encode()
    if PACE5000Port:
        try:
            port = ser.Serial(PACE5000Port, 9600, timeout=0.5)
            #time.sleep(0.5)
            port.write(commandCLS)
            port.write(commandSetMeasure)
        finally:
            if port:
                ser.Serial.close(port)

if __name__ == '__main__':
    PACE1000port = []
    PACE1000SN = []
    PACE5000port =[]
    PACE5000SN = []
    DPI515port = []
    DPI515SN = []
    SN1 = []
    SN2 = []
    SN3 = []
    SN4 = []
    SN5 = []
    SN6 = []
    allSN = []
    pressureArray = []
    frequencyArray = []
    diodeArray = []
    allPressure = []
    pressure1 = []
    pressure3 = []
    frequency = []
    diode = []

    #PACE1000port, PACE1000SN = findPACE1000()
    #PACE1000port, PACE1000SN = findPACE1000()
    PACE5000port = findPACE5000('FT1J3XMTA')
    #PACE5000port, PACE5000SN = findPACE5000()
    #DPI515port, DPI515SN = findDPI515()
    #DPI515port, DPI515SN = findDPI515()
    #SN1, pressure1 = readPACE1000(PACE1000port = PACE1000port)
    #SN2, frequency, diode = readPACE1000RAW(PACE1000port = PACE1000port)
    #SN3, pressure3 = readPACE5000(PACE5000port = PACE5000port)
    #allSN = SN1 + SN3
    #allPressure = pressure1 + pressure3

    #SN4, pressureArray = readPACE1000_FAST(PACE1000port = PACE1000port, nReadings = 5,parameter = 0)
    #SN5, frequencyArray = readPACE1000_FAST(PACE1000port = PACE1000port, nReadings = 5,parameter = 1)
    #SN6, diodeArray = readPACE1000_FAST(PACE1000port = PACE1000port, nReadings = 5,parameter = 2)

    controlPACE5000(PACE5000Port=PACE5000port, setpoint=0.5)
    time.sleep(30)
    measurePACE5000(PACE5000Port=PACE5000port)



