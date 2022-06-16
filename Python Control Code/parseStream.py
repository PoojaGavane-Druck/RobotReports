# A May, Apr 21 2022
# read in debug parameter definitions from xls
# open PV624 USB port in closed loop mode (binary debug data stream over USB)
# parse parameters from serial data stream and write to CSV

import pandas as pd
from dataStructures4 import *
from datetime import datetime
import csv
import time
import struct
import serial as ser
import serial.tools.list_ports as prtlst

#  global variables
pv624 = []
headerFile = 'Controller Debug Variables.xlsx'  # USB debug parameter definitions
dataDumpFile = datetime.now().strftime('%Y-%m-%dT%H-%M-%S') + '.csv'  # parsed dataDump filename


def findPV624(SN=[]):
    # checks all COM ports for PV624 in SN list
    # connects to first match in the list
    port = {}
    for pt in prtlst.comports():

        if any(s in pt.hwid for s in SN):
            print('\nFound PV624 UART:\n' + pt.description)

            port = ser.Serial(port = pt.device,
                                baudrate = 115200,
                                bytesize = ser.EIGHTBITS,
                                parity =ser.PARITY_ODD,
                                stopbits = ser.STOPBITS_ONE,
                                xonxoff = False,
                                rtscts = False,
                                dsrdtr = False,
                                timeout = 2) #note unusual parity setting for the USB coms
            time.sleep(1) #give windows time to open the com port before flushing
            port.flushInput()
    return port


# read parameter file from xls and parse header information
dfVars = pd.read_excel(headerFile, sheet_name='Vars', usecols='A,C,D,E')
keys = dfVars['Expression']
types = dict(zip(keys, dfVars['Type']))
lengths = dict(zip(keys, dfVars['Length']))
values = dict(zip(keys, dfVars['Value']))
header1 = bytes.fromhex(values['Header 1'][2:])  # header1, remove '0x' from string
footer1 = bytes.fromhex(values['Footer 1'][2:])  # footer1, remove '0x' from string
header2 = bytes.fromhex(values['Header 2'][2:])  # header2, remove '0x' from string
footer2 = bytes.fromhex(values['Footer 2'][2:])  # footer2, remove '0x' from string
dfStatus = pd.read_excel(headerFile, sheet_name='Status bits', usecols='A:B')
statusBits = dict(zip(dfStatus['Bit Name'], dfStatus['Bit No']))

messageLength = len(keys) * 4  # expected length of message in data dump (bytes)
# each parameter value is 4 bytes long of various data types

maxSize = messageLength * 2 - 1  # maximum size of serial data buffer (bytes)
# make read buffer large enough to hold 2 complete messages minus one byte
# The buffer is then guaranteed to hold one and only one complete message
# with the latest message located somewhere within the buffer.

# form the struct dataType pattern string for parsing binary messages
dataTypes = '<'  # little endian format for all parameters
for dataType in types.values():
    if dataType == 'uint32_t':
        dataTypes += 'I'
    elif dataType == 'float' or dataType == 'float32_t':
        dataTypes += 'f'
    elif dataType == 'int32_t':
        dataTypes += 'i'
    else:
        print('error unknown data type for struct conversion', dataType)

try:
    allReceivedData = bytearray()  # initialize byte array to hold binary data to be parsed
    pv624 = findPV624(SN=screw['USB'])  # find STM32 main board
    with open(dataDumpFile, 'w', newline='') as f:
        # write header keys to dataFile
        # keep file open for subsequent value writes
        csvFile = csv.writer(f, delimiter=',')
        csvFile.writerow(list(keys) + list(statusBits.keys()))
        while True:
            receivedData = pv624.read(messageLength)  # read latest data from serial buffer
            allReceivedData = allReceivedData + receivedData  # append to allReceivedData

            # discard old messages to minimize memory use
            if len(allReceivedData) > maxSize:
                allReceivedData = allReceivedData[-maxSize:]  # truncate allReceivedData to last maxSize bytes

            # find latest message in allReceivedData
            # index of end of last footer
            lastFooter = allReceivedData.rfind(footer1 + footer2) + len(footer1) + len(footer2)
            # latest binary message
            binaryMessage = allReceivedData[lastFooter - messageLength:lastFooter]
            # interpret message
            newMessage = struct.unpack(dataTypes, binaryMessage)
            #  decode the status parameter uint32 into string bit pattern
            #  and reverse the bit order to match order in statusBits.keys()
            newMessageDict = dict(zip(keys, newMessage))
            statusMessage = ('{0:032b}'.format(newMessageDict['status']))[::-1]
            # append to file
            csvFile.writerow(list(newMessage) + list(statusMessage))

finally:
    if pv624:
        pv624.close()















