# communicate directly with TERPS PM over one-wire interface
# to get diode, ref counts, and TERPS counts "ADC" values
# with minimal lag
# This code tests special command to get ref and terps counts as seperate values and bypasses
# readout lag on temperature value (no missed data)
# pressure filtering can be done subsequently by integrating separate count values prior to ratio calculation
# fastest response is 6Hz
# note: serial port setting for PM is 115200 8 O 1
# PM firmware update has not be released, written by RG
# see TN0835 and DK0249 for protocol and data structures
# 21/09/2020
# added cubic spline interpolation of presssure using scipy.interpolate.interp2d()
# 02 Feb 2021
# added readPM() from checkStepper6.py to remove dependency on checkStepper6.py
# Jun 10 2021
# corrected bit shifting operations for PM620 calcs for when bridge voltage is negative
# Sept 2021
# cleaned up and updated to use 2.0 FW on PM620T
# added stats summary at end of log
# May 26 2022
#  Added switch to use code for logging reference PM at high speed


import serial as ser
import serial.tools.list_ports as prtlst
import csv
import time
from datetime import datetime
import traceback
import struct
import binascii
from scipy.interpolate import Rbf


def findPM(SN=[]):
    # checks all COM ports for PM
    # return device ID
    if not SN:
        SN = ['FTBTA7P2A']  # SN of FTDI chip in USB to UART board
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
            return None, None

        COM.write(queryDK)
        DK = COM.read(DKmessageLength)
        time.sleep(0.01)

        while COM.inWaiting():  # something fishy with RG firmware ID, responding with too many bytes.
            # Discard any remaining read buffer here to avoid problems later.
            COM.read(COM.inWaiting())
            time.sleep(0.01)

        if DK[0:len(queryDK)] != queryDK or len(DK) < DKmessageLength:
            print('invalid application firmware', binascii.hexlify(DK[0:len(queryDK)]), len(DK))
            return None, None

        # first byte is echo of queryDK command, remaining is ASCII, terminated with NULL
        DK = DK[1:].decode()
        print('PM Firmware:', DK)

        # discard query echo and convert header back to binary representation from ascii hex (see DC0249)
        return binascii.unhexlify(header[1:]), DK

    except:
        print('error reading PM header:\n',header)
        traceback.print_exc()
        return None, None


def queryPM(COM, type='PM620T'):
    # query either a PM620T or PM620
    if type == 'PM620T':
        # query PM connected to serial port on COM to get TERPS
        # the COM port must already be open and input buffer flushed

        #query = bytearray([0xC9,0x2F,0x1F,0x00,0x00])
        # request ref count, and TERPS count, and temperature ADC, at 6Hz nominal read rate
        # not used by PV624

        query = bytearray([0xC9,0x29,0x1F,0x00,0x00])
        # request ref count, and TERPS count, and temperature ADC, at 13Hz nominal read rate
        # slow read command for PV624, 12 PPM, 80 ms on PC

        #query = bytearray([0xC9, 0x28, 0x1F, 0x00, 0x00])
        # request ref count, and TERPS count, and temperature ADC, at 26Hz nominal read rate,
        # not used in PV624, 26 PPM, 46 ms on PC

        #query = bytearray([0xC9, 0x27, 0x1F, 0x00, 0x00])
        # request ref count, and TERPS count, and temperature ADC, at 55Hz nominal read rate, 50 PPM
        # used by readRefPM.py and fast read in PV624, 45 PPM, 25 ms on PC

        #query = bytearray([0xC9, 0x26, 0x1F, 0x00, 0x00])
        #  very fast read used for filter investigations

        messageLength = 15 # expected response is 15-byte value
        # first 5 byte are local echo of query command
        # next 4 bytes are reference count (unit32), requires additional bit-wise operations to extract value
        # next 2 bytes are TERPS count (uint16), requires additional bit-wise operations to extract value
        # next 4 bytes are temperature ADC value (1/.0798 uint32)
        # requires additonal bit-wise operations to extract value
        data = []
        try:
            COM.write(query)
            data = COM.read(messageLength)
            #print(binascii.hexlify(data))

            if data[0:len(query)] != query:
                print('invalid data from PM:\n',binascii.hexlify(data))
                return None
            return data
        except:
            print('error reading PM:\n',binascii.hexlify(data))
            traceback.print_exc()
            return None

    elif type == 'PM620':
        #query PM620 connected to serial port on COM
        #the COM port must already be open and input buffer flushed
        #query = bytearray([0xc9,0x0F,0x1F,0x20,0x30]) #from genii, query bridge voltage and diode voltage at 6 Hz nominal read rate, (nominally 330 ms for 2 ch data), actual = 575 - 591 ms
        #query = bytearray([0xc9,0x07,0x17,0x20,0x30]) #from RG, query bridge voltage and diode voltage at 55 Hz nominal read rate, (nominally 36 ms for 2 ch data)

        # query bridge voltage and diode voltage

        #query = bytearray([0xc9, 0x04, 0x16, 0x20, 0x30])  # 440 Hz nominal rate, 22 ms
        # fast read PV624

        #query = bytearray([0xc9, 0x05, 0x16, 0x20, 0x30])  # 220 Hz nominal rate, 27 ms
        # ref read PC, not used on PV624

        query = bytearray([0xc9, 0x07, 0x16, 0x20, 0x30])  # 55 Hz nominal rate, 54 ms
        # slow read PV624

        messageLength = 13  # expected response is 13-byte value, PM620
        # first 5 byte are local echo of query command
        # next 4 bytes are bridge voltage (uint32), requires additional bit-wise operations to extract value
        # next 4 bytes are temperature ADC value (uint32), requires additional bit-wise operations to extract value

        data = []
        try:
            COM.write(query)
            data = COM.read(messageLength)
            # print(binascii.hexlify(data))

            if data[0:len(query)] != query:
                print('invalid data from PM:\n', binascii.hexlify(data))
                return None
            return data
        except:
            print('error reading PM:\n', binascii.hexlify(data))
            traceback.print_exc()
            return None

    else:  # this command not used in PV624, too slow
        # query PM620 connected to serial port on COM using 2 separate commands for diode and bridge
        # the COM port must already be open and input buffer flushed
        queryBridge = bytearray([0xC9, 0x06, 0x10, 0x20, 0x30])  # query bridge voltage at 110 Hz nominal read rate
        queryDiode = bytearray([0xC9, 0x00, 0x16, 0x20, 0x30])  # query diode voltage at 110 Hz nominal read rate

        messageLength = 9  # expected response is 9-byte value
        # first 5 byte are local echo of query command
        # Next 4 bytes are bridge voltage (uint32) or temperature ADC value (uint32
        # Requires additional bit-wise operations to extract value

        try:
            COM.write(queryBridge)
            dataBridge = COM.read(messageLength)

            if dataBridge[0:len(queryBridge)] != queryBridge:
                print('invalid data from PM bridge reading:\n', binascii.hexlify(dataBridge))
                return None

            COM.write(queryDiode)
            dataDiode = COM.read(messageLength)

            if dataDiode[0:len(queryDiode)] != queryDiode:
                print('invalid data from PM diode reading:\n', binascii.hexlify(dataDiode))
                return None

            data = dataBridge + dataDiode  # append byte arrays for processing
            return data

        except:
            print('error reading PM:\n')
            traceback.print_exc()
            return None


def convertHeader(header = []):
    # extract relevant parameters from PM header data
    # data locations in header [offset,length] (bytes), from DK0249:

    SNloc = [0x2, 4]  # serial number, int32
    STloc = [0x8A, 2]  # sensor type (1 = abs, 2 = gauge), int16
    HPloc = [0x8C, 4]  # +ve FS, float32
    LPloc = [0x90, 4]  # -ve FS, float32
    PtypeLoc = [0x8A, 2]  # pressure type (2 = gauge, 1 = abs)

    nPressureLoc = [0xD0,2]  # number of pressure points at each temperature, int16
    nTemperatureLoc = [0xCE,2]  # number of temperature points at each pressure, int16
    coefLoc = [0x200,3584]  # coefficient data, float32, 3584 is entire space reserved for coefficients

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
        # nP = n, nT = m, coeffcients in coef[] are:
        # T1 P11 Vb11 Vd11 P12 Vb12 Vd12...P1n Vb1n Vd1n
        # T2 P21 Vb21 Vd21 P22 Vb22 Vd22...P2n Vb21 Vd2n
        # ...
        # Tm Pm1 Vbm1 Vdm1 .. Pmn Vbmn Vdmn

        T = coef[:, 0]  # applied temperatures, unused for pressure calculations, 1 x nT
        if np.any(np.diff(T) < 0):  # check that T values are monotonically increasing
            print('Error: temperature array values not monotonically increasing:', T)

        P = coef[:, np.arange(1,nP*3+1,3)]  # applied pressures at each temperature, nT x nP
        # Note: set of applied pressures P are nominally repeated at each temperature, but are not *exactly* the same
        if np.any(np.diff(P, axis=1) < 0):  # check that applied pressures are monotonically increasing
            print('Error: applied pressure values are not monotonically increasing:', P)

        F = coef[:, np.arange(2, nP*3+1,3)]  # measured frequency at each temperature and pressure, nT x nP
        V = coef[:, np.arange(3, nP*3+1,3)]  # measured diode voltage at each temperature and pressure nT x nP

        return T, P, F, V, SN, Ptype, LP, HP, nT, nP

    except:
        print('error parsing header')
        traceback.print_exc()
        return None,None,None,None,None,None,None,None,None,None


def frequencySplines(F = [], P = [], V = []):
    # compute two 4-point sliding window spline coefficient arrays
    # relating frequency to applied pressure and frequency to diode voltage
    # at constant temperature
    # there are better ways to do this with scipy.interpolate,
    # but the explicit numpy polyfit coefficient solution
    # is more useful for demonstrating the interpolation method
    # currently used on DRUCK sensors

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
                                     0x3F, 0x33, 0xE8, 0x23, 0x3F, 0x33, 0x1E, 0x1E]), type='PM620T'):
    if type == 'PM620T':
        # convert TERPS PM data message to floats [temperature,refCount,terpsCount,pressureRatio]
        # where data acquired from one read request, custom firmware required
        # first 5 byte are local echo of query command from queryPM()
        # next 4 bytes are reference count (uint32), requires additional bit-wise operations to extract value
        # next 2 bytes are TERPS count (uint16), requires additional bit-wise operations to extract value
        # next 4 bytes are temperature ADC count (uint32), requires additonal bit-wise operations to extract value

        refData = data[5:9]  # ref clock count
        terpsData = data[9:11]  # TERPS edge count
        temperatureData = data[11:15]  # temperature ADC count

        # throw out first 5 bits of temperatureData
        temperatureCount = temperatureData[0] & 0x07  # bit-wise AND with '0000 0111'
        refCount = refData[0] & 0x07  # bit-wise AND with '0000 0111'

        # bit shift left 7 bits to convert to int16
        terpsCount = (terpsData[0] << 7) + terpsData[1]
        # bit shift left 21 bits to convert to uint32
        temperatureCount = ((temperatureCount << 21) + (temperatureData[1] << 14) +
                            (temperatureData[2] << 7) + temperatureData[3])

        refCount = (refCount << 21) + (refData[1] << 14) + (refData[2] << 7) + refData[3]
        # compute count ratio used in spline interpolation algorithm for PM
        # Note: for smoothing operations integrate numerator and denominator values separately before taking the ratio
        # rather than computing average of multiple ratio values.
        pressureRatio = int((terpsCount << 33) / (refCount * 9))  # ratio of terps counts to ref counts, uint32
        # Note: python represents int type with infinite precision and has no built-in uint type.
        # In strongly typed languages there could be rounding error from casting uint32 value as int here.

        return pressureRatio, temperatureCount, refCount, terpsCount

    elif type == 'PM620':
        # convert PM data message to floats [temperature,bridge voltage]
        # where both data requested by one read command
        # first 5 byte are local echo of query command
        # next 4 bytes are bridge voltage (uint32), requires additional bit-wise operations to extract value
        # next 4 bytes are temperature ADC value (uint32), requires additonal bit-wise operations to extract value

        bridgeData = data[5:9]  # bridge voltage data as bytes
        temperatureData = data[9:13]  # temperature ADC count as bytes

        # throw out first 4 bits of temperatureData
        temperatureCount = temperatureData[0] & 0x0F  # bit-wise AND with '0000 1111'
        # throw out first 4 bits of bridgeData
        bridgeVoltage = bridgeData[0] & 0x0F  # bit-wise AND with '0000 1111'

        # bit shift left 21 bits to convert to int32, and apply sign bit correction with a magic number
        # Note: sign bit from ADC is at bit 25 instead of bit 31,
        # hence magic number 0x1000000 subtraction required for negative voltages
        # this requires better documentation in the TS
        temperatureCount = ((temperatureCount << 21) + (temperatureData[1] << 14) +
                            (temperatureData[2] << 7) + temperatureData[3]) - 0x1000000

        # bit shift left 21 bits to convert to int32, and apply sign bit correction with a magic number
        # Note: sign bit from ADC is at bit 25 instead of bit 31,
        # hence magic number 0x1000000 subtraction required for negative bridge voltages
        # this requires better documentation in the TS
        bridgeVoltage = ((bridgeVoltage << 21) + (bridgeData[1] << 14) +
                         (bridgeData[2] << 7) + bridgeData[3]) - 0x1000000

        return bridgeVoltage, temperatureCount, 0, 0

    else:
        # convert PM data message to floats [temperature,bridge voltage]
        # where data has been acquired by 2 consecutive read commands
        # first 5 byte are local echo of bridge query command
        # next 4 bytes are bridge voltage (uint32), requires additional bit-wise operations to extract value
        # next 5 bytes are local echo of temperature query command
        # next 4 bytes are temperature ADC value (uint32), requires additonal bit-wise operations to extract value
        bridgeData = data[5:9]  # bridge voltage data as bytes
        temperatureData = data[14:18]  # temperature ADC count as bytes
        # throw out first 5 bits of temperatureData
        temperatureCount = temperatureData[0] & 0x07  # bit-wise AND with '0000 0111'
        # throw out first 5 bits of bridgeData
        bridgeVoltage = bridgeData[0] & 0x07  # bit-wise AND with '0000 0111'
        # extract sign bit from bridge Data
        bridgeVoltageSign = ((bridgeData[0] & 0x08) >> 2) - 1
        # mask 5th bit and convert to +/- 1 for sign: 'xxxx 1xxx' --> 1, 'xxxx 0xxx' --> -1
        # bit shift left 21 bits to convert to uint32
        temperatureCount = ((temperatureCount << 21) + (temperatureData[1] << 14) +
                            (temperatureData[2] << 7) + temperatureData[3])
        # bit shift left 7 bits to convert to uint16
        bridgeVoltage = (bridgeVoltageSign * ((bridgeVoltage << 21) + (bridgeData[1] << 14) +
                                              (bridgeData[2] << 7) + bridgeData[3]))
        return bridgeVoltage, temperatureCount, 0, 0


def interpolateHeader(F , V, P):
    scaling = 4096/(5*2**24)
    # multiplier factor to convert uint32 (f,v) values
    # from PM to float values in characterization header data (== 4.8828125e-05)
    # value is vestigial from piezo-PM Genie source code and has not be documented in DK0249 or TN0835
    # presumably it was intended to scale PM ADC count data to mV level values
    # for debugging purposes, and left in the code thereafter

    # iP = interp2d(F/scaling,V/scaling,P,kind='cubic')
    # Note: DO NOT USE interp2d for interpolation, causes kinks in result.  Use Rbf() instead.
    # see: https://www.semicolonworld.com/question/57896/how-can-i-perform-two-dimensional-interpolation-using-scipy

    iP = Rbf(F/scaling, V/scaling, P, function='cubic', smooth=0)
    # radial basis function interpolator operator for pressure, going through all the (F,V,P) chacterization data points
    # pre-apply scaling factor to header data to avoid requirement during real-time calculations
    # Call this function once to initialize iP() interpolator object, then use it to estimated pressure
    # as Pest = iP(f,v), where (f,v) are measured frequency and diode voltage (scalar values) bounded by (F,V)
    # Note: iP() returns an array object even for a scalar input
    # convert to float() if appending / displaying / writing to file
    return iP

def readPM(COM, iP, DK):
    # wrapper function to return pressure as float from PM in one call
    # requires pre-computation of iP() interpolator object required before use
    # uses queryPM(), convertReading() from checkPM.py
    if 'DK0472, V01.01' in DK or 'DK0472, V02.00.0' in DK or 'DK0472, V03.04.0' in DK:
                PMtype = 'PM620T'
                # using RG commands
    elif 'DK0351' in DK:
        PMtype = 'PM620'
    else:
        print('PM DK not recognized', DK)
        exit()
    data = queryPM(COM, PMtype)
    (Fratio, Vdiode, Nref, Nterps) = convertReading(data, PMtype)  # parse reading from PM
    pressure = float(iP(Fratio, Vdiode))
    return pressure


# ===============================================================================
if __name__ == '__main__':
    import numpy as np

    # PMtype = 'PM620D' #use two sequential standard commands for non-TERPS
    # PMtype = 'PM620' #use standard commands for non-TERPS
    PMtype = 'PM620T' # use RG firmware commands for TERPS
    PM = findPM()

    dataFile = dataFile = 'PM_' + datetime.now().strftime('%Y-%m-%dT%H-%M-%S') + '.csv'
    iterations = int(input('How many iterations (100)? ') or '100')
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
            time.sleep(1)  # give time for windows to open the serial port
            PMCOM.flush()
            time.sleep(0.1)

            (header,DK) = readHeader(PMCOM)  # read the header and DK version info
            (T, P, F, V, SN, Ptype, LP, HP, nT, nP) = convertHeader(header)  # extract calibration data
            print('SN, Ptype, LP, HP\n', SN, Ptype, LP, HP)
            iP = interpolateHeader(F, V, P)
            # create spline interpolator object to estimate pressure given frequency and diode voltage

            if 'DK0472' in DK:
                PMtype = 'PM620T'
                print('using RG commands')
            else:
                PMtype = 'PM620'

            # check that interpolated values agree exactly at characterization data points
            for f, v, p in np.nditer([F,V,P]):  # calculate interpolated p for each measured (f,v) value
                scaling = 4096/(5*2**24)
                # multiplier factor to convert uint32 (f,v) values from PM to float values
                # in characterizaton header data (== 4.8828125e-05)
                # value is vistigial from piezo-PM Genie source code and has not be documented in DK0249 or TN0835
                # presumably it was intended to scale PM ADC count data to mV level values for debugging purposes
                # and left in the code thereafter
                residual = iP(f/scaling,v/scaling) - p
                if np.abs(residual / p) > 0.0001:  # values should agree to machine precision at data points
                    print('Interpolation Error (p,%): ', p, residual*100)  # interpolation error detected

            pressure = readPM(PMCOM, iP, DK)
            print(pressure)
            counter = 0
            P = np.zeros(iterations)
            T = np.zeros(iterations)
            for n in range(0, iterations):
                myFile = open(dataFile, 'a', newline='')
                csvFile = csv.writer(myFile, delimiter=',')
                tic = datetime.now()
                data = queryPM(PMCOM, type=PMtype)
                elapsedTime = (datetime.now() - tic).total_seconds()
                # PM measurement clock is asynchronous to request for data
                # nominal sampling rate is 13 Hz
                # to avoid missed measurements it must be queried at least every 0.07 s
                # Querying at a faster rate than 13 Hz will not increase the sampling rate
                # because the queryPM() is a blocking operation gated by the PM measurement time
                # print(binascii.hexlify(data))  # convert asci-coded hex from PM back to normal hex values
                (Fratio, Vdiode, Nref, Nterps) = convertReading(data, type=PMtype)
                # parse reading from PM, last two elements zero if not a TERPS
                Pest = float(iP(Fratio,Vdiode))
                P[n] = Pest
                T[n] = elapsedTime
                # elapsedTime = (datetime.now() - tic).total_seconds()
                print(counter, round(Fratio, 3), round(Vdiode, 3), round(Nref, 0),
                        round(Nterps, 0), round(elapsedTime, 3), round(Pest, 2),
                        sep='\t')
                result = [counter, Fratio, Vdiode, Nref, Nterps, Pest, elapsedTime]
                csvFile.writerow(result)
                counter = counter + 1
                myFile.close()

            # save characterization header data to csv files by variable name
            np.savetxt("F.csv", F/scaling, delimiter=",")
            np.savetxt("V.csv", V/scaling, delimiter=",")
            np.savetxt("P.csv", P, delimiter=",")

        except:
            traceback.print_exc()
            if PMCOM:
                PMCOM.close()
            exit()

        finally:
            if PMCOM:
                PMCOM.close()
            print('\ntest completed')






