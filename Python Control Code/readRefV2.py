# May 26 2022
# read PM620 as quickly as possible
# and log to file


import serial as ser
import serial.tools.list_ports as prtlst
import csv
import time
from datetime import datetime
import traceback
import struct
import binascii
from scipy.interpolate import Rbf
from dataStructures4 import *
import sys

SN = screw['REF']  # reference sensor SN's


def findPM(SN=[]):
    # checks all COM ports for PM620
    # return device ID
    port = []
    for pt in prtlst.comports():
        print(pt.hwid)
        if any(s in pt.hwid for s in SN):
            port = pt.device
            print('\nFound PM UART:\n' + pt.description)
            return port
    return port


def readHeader(COM):
    # query PM connected to serial port on COM to read header and characterization data
    query = bytearray([0xC3])  # command to request header and characterization data
    queryDK = bytearray([0xC8])  # query for application version, expect DK0472 or DK0351 for PM620T or PM620
    # data is returned as ASCII coded HEX
    messageLength = 8193  # expected response is 8193 bytes long
    DKmessageLength = 17  # expected response length for DK query, see TN0835 5.2.10
    # first byte is echo of query command
    # remaining 8192 bytes are ASCII coded HEX format (4096 bytes of binary data)
    # with data structure in DK0249
    header = []

    try:
        COM.write(query)
        header = COM.read(messageLength)

        if header[0:len(query)] != query or len(header) < messageLength:
            print('invalid header data:\n', binascii.hexlify(header[0:len(query)]),  len(header))
            sys.exit()

        COM.write(queryDK)
        DK = COM.read(DKmessageLength)
        time.sleep(0.01)

        while COM.inWaiting():  # something fishy with RG firmware ID, responding with too many bytes.
            # Discard any remaining read buffer here to avoid problems later.
            COM.read(COM.inWaiting())
            time.sleep(0.01)

        if DK[0:len(queryDK)] != queryDK or len(DK) < DKmessageLength:
            print('invalid application firmware', binascii.hexlify(DK[0:len(queryDK)]), len(DK))
            sys.exit()

        # first byte is echo of queryDK command, remaining is ASCII, terminated with NULL
        DK = DK[1:].decode()
        print('PM Firmware:', DK)

        # discard query echo and convert header back to binary representation from ascii hex (see DC0249)
        return binascii.unhexlify(header[1:]), DK

    except:
        print('error reading PM header:\n', header)
        traceback.print_exc()
        sys.exit()


def queryPM(COM):
    # query PM620 connected to serial port on COM
    query = bytearray([0xc9, 0x04, 0x16, 0x20, 0x30])  # 880 Hz nominal rate
    # fastest read, 0x03 = +/-0.04 mbar, 16 - 30 ms, 20ppm fs
    # slower read, 0x04 = +/= 0.014 mbar, 16 - 30 ms, 7ppm fs
    # --> use 0x04, no decrease in time with faster read but decrease in SNR

    messageLength = 13  # expected response is 13-byte value, PM620

    try:
        COM.write(query)
        data = COM.read(messageLength)
        if data[0:len(query)] != query:
            print('invalid data from PM:\n', binascii.hexlify(data))
            sys.exit()
        return data

    except:
        print('error reading PM')
        traceback.print_exc()
        sys.exit()


def convertHeader(header = []):
    # extract relevant parameters from PM header data
    # data locations in header [offset,length] (bytes), from DK0249:

    SNloc = [0x2, 4]  # serial number, int32
    HPloc = [0x8C, 4]  # +ve FS, float32
    LPloc = [0x90, 4]  # -ve FS, float32
    PtypeLoc = [0x8A, 2]  # pressure type (2 = gauge, 1 = abs)

    nPressureLoc = [0xD0, 2]  # number of pressure points at each temperature, int16
    nTemperatureLoc = [0xCE, 2]  # number of temperature points at each pressure, int16
    coefLoc = [0x200, 3584]  # coefficient data, float32, 3584 is entire space reserved for coefficients

    try:
        SN = struct.unpack('>l', header[SNloc[0]:SNloc[0] + SNloc[1]])[0]
        Ptype = struct.unpack('>h', header[PtypeLoc[0]:PtypeLoc[0] + PtypeLoc[1]])[0]
        LP = struct.unpack('>f', header[LPloc[0]:LPloc[0] + LPloc[1]])[0]
        HP = struct.unpack('>f', header[HPloc[0]:HPloc[0] + HPloc[1]])[0]
        nP = struct.unpack('>h', header[nPressureLoc[0]:nPressureLoc[0] + nPressureLoc[1]])[0]
        nT = struct.unpack('>h', header[nTemperatureLoc[0]:nTemperatureLoc[0] + nTemperatureLoc[1]])[0]
        coefLoc[1] = (nP*nT*3 + nT)*4  # expected number of coefficient value bytes in header
        coef = []

        for value in struct.iter_unpack('>f',header[coefLoc[0]:coefLoc[0] + coefLoc[1]]): #unpack as float32, big endian
            coef.append(value[0])
        coef = np.array(coef).reshape((nT, nP*3+1))  # convert to a 2D numpy array for more logical indexing

        T = coef[:, 0]  # applied temperatures, unused for pressure calculations, 1 x nT
        P = coef[:, np.arange(1,nP*3+1,3)]  # applied pressures at each temperature, nT x nP
        BV = coef[:, np.arange(2, nP*3+1,3)]  # measured bridge voltage at each temperature and pressure, nT x nP
        DV = coef[:, np.arange(3, nP*3+1,3)]  # measured diode voltage at each temperature and pressure nT x nP

        return T, P, BV, DV, SN, Ptype, LP, HP, nT, nP

    except:
        print('error parsing header')
        traceback.print_exc()
        sys.exit()


def frequencySplines(F=[], P=[], V=[]):
    order = 3  # fit order, 3 = cubic
    points = order + 1  # number of points per spline fit
    (nT, nP) = F.shape  # number of unique temperature and pressure points characterization data
    S = np.zeros((nT, nP-order, points))  # initialize sliding window coefficient array
    Q = np.zeros((nT, nP-order, points))  # initialize sliding window coefficient array
    for n in range(0, nT):
        for m in range(0, nP-order):
            # fit cubic polynomials through each set of points
            # alternate method could be used, e.g. Newton Interpolating Polynomial
            # to reduce computation time on embedded platform
            S[n, m] = np.polyfit(F[n, m:m+points], P[n, m:m+points], order)  # S(F,T) = P
            Q[n, m] = np.polyfit(F[n, m:m+points], V[n, m:m+points], order)  # Q(F,T) = V
    return S, Q


def convertReading(data = bytearray([0xC9, 0x2F, 0x1F, 0x00, 0x00, 0xE8, 0x23,
                                     0x3F, 0x33, 0xE8, 0x23, 0x3F, 0x33, 0x1E, 0x1E])):

    bridgeData = data[5:9]  # bridge voltage data as bytes
    temperatureData = data[9:13]  # temperature ADC count as bytes
    temperatureCount = temperatureData[0] & 0x0F  # bit-wise AND with '0000 1111'
    bridgeVoltage = bridgeData[0] & 0x0F  # bit-wise AND with '0000 1111'
    temperatureCount = ((temperatureCount << 21) + (temperatureData[1] << 14) +
                        (temperatureData[2] << 7) + temperatureData[3]) - 0x1000000
    bridgeVoltage = ((bridgeVoltage << 21) + (bridgeData[1] << 14) +
                     (bridgeData[2] << 7) + bridgeData[3]) - 0x1000000
    return bridgeVoltage, temperatureCount


def interpolateHeader(F , V, P):
    scaling = 4096/(5*2**24)
    iP = Rbf(F/scaling, V/scaling, P, function='cubic', smooth=0)
    return iP


def readPM(COM, iP, DK):
    # wrapper function to return pressure as float from PM in one call
    # requires pre-computation of iP() interpolator object required before use
    # uses queryPM(), convertReading() from checkPM.py
    if 'DK0351' in DK:
        PMtype = 'PM620'
    else:
        print('PM DK not recognized', DK)
        sys.exit()
    data = queryPM(COM)
    (Vbridge, Vdiode) = convertReading(data)  # parse reading from data
    pressure = float(iP(Vbridge, Vdiode))
    return pressure


# ===============================================================================
if __name__ == '__main__':
    import numpy as np
    PM = findPM(SN=SN)

    dataFile = datetime.now().strftime('%Y-%m-%dT%H-%M-%S') + '_Ref' + '.csv'
    print('Reading PM header')

    if PM:
        try:
            PMCOM = ser.Serial(
                port=PM,
                baudrate=115200,
                bytesize=ser.EIGHTBITS,
                parity=ser.PARITY_ODD,
                stopbits=ser.STOPBITS_ONE,
                xonxoff=False,
                rtscts=False,
                dsrdtr=False,
                timeout=6)  # note unusual parity setting for the PM COM
            PMCOM.set_buffer_size(rx_size=16384)
            # increase input buffer size to 16kB to prevent overflow when reading 8kB header from PM
            # timeout must be > 5s for header data transmit to complete
            time.sleep(0.5)  # give time for windows to open the serial port

            (header, DK) = readHeader(PMCOM)  # read the header and DK version info
            (T, P, F, V, SN, Ptype, LP, HP, nT, nP) = convertHeader(header)  # extract calibration data
            # create spline interpolator object to estimate pressure given frequency and diode voltage
            iP = interpolateHeader(F, V, P)

            with open(dataFile, 'w', newline='') as f:
                csvFile = csv.writer(f, delimiter=',')
                print('logging ref PM to: ', dataFile)
                while True:
                    tic = datetime.now()
                    pressure = readPM(PMCOM, iP, DK)
                    elapsedTime = round((datetime.now() - tic).total_seconds() * 1000)
                    csvFile.writerow([elapsedTime, pressure])

        except:
            traceback.print_exc()

        finally:
            if PMCOM:
                PMCOM.close()
            print('\ntest completed')






