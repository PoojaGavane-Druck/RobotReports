import dpi620gLib as dpi
import dpiAttributes as dpiAttr
from datetime import datetime

printIt = 1
writeLog = 1

pv624sn = ['205C324B5431']
dpi620gSn = ['FTBTBC9KA']
fileName = 'GENII_CONNECTION_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.txt'

try:
    file = open(fileName, "w")
except:
    errorHere = 1
    print("Could not open log file")

def display(toDisplay):
    if printIt == 1:
        print(toDisplay)
    if writeLog == 1:
        # File writing enabled 
        file.write(toDisplay + "\n")

def main():
    display("Starting genii connection test")
    baseDk = ""
    baseVer = ""
    DPI620G = dpi.DPI620G(dpi620gSn)

    # Poll if DK0499 is available
    while baseDk != "DK0499":
        display("Searching for PV624")
        baseDk, baseVer = DPI620G.getRI()
    
    # Write to Log
    display("PV624 Found: FW Version: " + str(baseVer))
    display("Setting remote mode")
    DPI620G.setKM('R')
    mode = DPI620G.getKM()
    if mode == 1:
        display("Mode setting passed")
        # Base could be connecting with sensor here, check for errors using PV command
        pressure = 0.0
        error = 0xFFFFFFFF
        status = 0
        baro = 0.0
        #check if sensor connection error is present
        while dpiAttr.error['referenceSensorCommFail'] == (error & dpiAttr.error['referenceSensorCommFail']):
            # keep running the loop
            display("Sensor comms error on PV624")
            pressure, error, status, baro = DPI620G.getPV()

        # Sensor is connected, proceed with next routine
        display("Sensor connected on PV624")
        pvPrimaryBootDk = DPI620G.getDK(dpiAttr.versionInfo['pvBootloader'])
        if pvPrimaryBootDk == "DK0498":
            pvPrimaryBootVer = DPI620G.getRV(dpiAttr.versionInfo['pvBootloader'])
            if pvPrimaryBootVer != "":
                display("PV624 primary bootloader details: " + str(pvPrimaryBootDk) + " " + str(pvPrimaryBootVer))
                pvPrimaryAppDk = DPI620G.getDK(dpiAttr.versionInfo['pvApplication'])
                if pvPrimaryAppDk == "DK0499":
                    pvPrimaryAppVer = DPI620G.getRV(dpiAttr.versionInfo['pvApplication'])
                    if pvPrimaryAppVer != "":
                        display("PV624 primary application details: " + str(pvPrimaryAppDk) + " " + str(pvPrimaryAppVer))
                        pvSecondaryBootDk = DPI620G.getDK(dpiAttr.versionInfo['secondaryBootloader'])
                        if pvSecondaryBootDk == "DK0510":
                            pvSecondaryBootVer = DPI620G.getRV(dpiAttr.versionInfo['secondaryBootloader'])
                            if pvSecondaryBootVer != "":
                                display("PV624 secondary bootloader details: " + str(pvSecondaryBootDk) + " " + str(pvSecondaryBootVer))
                                pvSecondaryAppDk = DPI620G.getDK(dpiAttr.versionInfo['secondaryApplication'])
                                if pvSecondaryAppDk == "DK0509":
                                    pvSecondaryAppVer = DPI620G.getRV(dpiAttr.versionInfo['secondaryApplication'])
                                    if pvSecondaryAppVer != "":
                                        display("PV624 secondary application details: " + str(pvSecondaryAppDk) + " " + str(pvSecondaryAppVer))
                                        baseSn = DPI620G.getSN(dpiAttr.serialNo['pv624'])
                                        if baseSn != 0:
                                            display("PV624 serial number: " + str(baseSn))
                                            pvDate = "05/04/2022"
                                            pvTime = "01:21:33"
                                            DPI620G.setSD(pvDate)
                                            DPI620G.setST(pvTime)
                                            pvReadDate = DPI620G.getSD()
                                            pvReadTime = DPI620G.getST()
                                            # Compare only date as time may have changed
                                            if pvDate == pvReadDate:
                                                # Assume time setting is successful too
                                                display("PV624 Date: " + str(pvReadDate))
                                                display("PV624 Time: " + str(pvReadTime))
                                                # Read the baselined sensor version, this is required if the terps requires an upgrade
                                                sensorBaseVer = DPI620G.getQV(dpiAttr.sensorBase['TERPS'])
                                                if sensorBaseVer == "02.00.00":
                                                    display("TERPS Sensor baselined firmware version: " + str(sensorBaseVer))
                                                    # Read sensor information
                                                    minP, maxP, senType = DPI620G.getIS(dpiAttr.pvSensor['barometer'])
                                                    if (minP == 800) and (maxP == 1100) and (senType == 5):
                                                        display("Barometer info:")
                                                        display("Minimum range: " + str(minP))
                                                        display("Maximum range: " + str(maxP))
                                                        display("Sensor Type: " + str(senType))
                                                        baroCalDate = DPI620G.getCD(dpiAttr.pvSensor['barometer'])
                                                        if baroCalDate != "":
                                                            display("Barometer calibration date: " + str(baroCalDate))
                                                            # Read barometer calibration interval
                                                            baroCalInterval = DPI620G.getCI(dpiAttr.pvSensor['barometer'])
                                                            if baroCalInterval != 0:
                                                                display("Barometer calibration interval: " + str(baroCalInterval) + " days")
                                                                # Read barometer next calibration date
                                                                baroNextCalDate = DPI620G.getND(dpiAttr.pvSensor['barometer'])
                                                                if baroNextCalDate != "":
                                                                    display("Barometer next cal date: " + str(baroNextCalDate))
                                                                    # Read number of barometer calibration points
                                                                    minPoints, maxPoints = DPI620G.getCN(dpiAttr.pvSensor['barometer'])
                                                                    if (minPoints != 0) and (maxPoints != 0):
                                                                        display("Barometer calibration minimum points: " + str(minPoints))
                                                                        display("Barometer calibration maximum points: " + str(maxPoints))
                                                                        # Read percentage battery level of the PV624
                                                                        battPercentage = DPI620G.getRB(dpiAttr.pvBattery['percentage'])
                                                                        # Do not verify if battery percentage could be 0, as there could be no battery connected
                                                                        display("PV624 battery percentage: " + str(battPercentage))
                                                                        # Read PV one more time, dont bother what it says
                                                                        pressure, error, status, baro = DPI620G.getPV()
                                                                        display(str(pressure) + " " + str(error) + " " + str(status) + " " + str(baro))
                                                                        # Read sensor app DK number
                                                                        sensorAppDk = DPI620G.getDK(dpiAttr.versionInfo['sensorApplication'])
                                                                        if sensorAppDk != "":
                                                                            # Read sensor app version
                                                                            sensorAppVer = DPI620G.getRV(dpiAttr.versionInfo['sensorApplication'])
                                                                            if sensorAppVer != "":
                                                                                display("Sensor Application Details: " + str(sensorAppDk) + " " + str(sensorAppVer))
                                                                                # Read sensor bootloader DK
                                                                                sensorBootDk = DPI620G.getDK(dpiAttr.versionInfo['sensorBootloader'])
                                                                                if sensorBootDk != "":
                                                                                    sensorBootVer = DPI620G.getRV(dpiAttr.versionInfo['sensorBootloader'])
                                                                                    if sensorBootVer != "":
                                                                                        display("Sensor Bootloader Details: " + str(sensorBootDk) + " " + str(sensorBootVer))
                                                                                        # Read sensor serial number
                                                                                        sensorSerial = DPI620G.getSN(dpiAttr.serialNo['sensor'])
                                                                                        if sensorSerial != "":
                                                                                            display("Sensor serial number " + str(sensorSerial))
                                                                                            # Read PM sensor calibration date
                                                                                            pmCalDate = DPI620G.getCD(dpiAttr.pvSensor['reference'])
                                                                                            if pmCalDate != "":
                                                                                                display("PM sensor cal date: " + str(pmCalDate))
                                                                                                # Read PM sensor information
                                                                                                pmMinP, pmMaxP, pmType= DPI620G.getIS(dpiAttr.pvSensor['reference'])
                                                                                                if (pmMinP != "") and (pmMaxP != "") and (pmType != ""):
                                                                                                    display("PM Sensor info:")
                                                                                                    display("Minimum range: " + str(pmMinP))
                                                                                                    display("Maximum range: " + str(pmMaxP))
                                                                                                    display("Sensor Type: " + str(pmType))
                                                                                                    # Read PM Sensor zero value
                                                                                                    zeroValue = DPI620G.getIZ(dpiAttr.pvSensor['reference'])
                                                                                                    display("PM Zero value: " + str(zeroValue))
                                                                                                    # Read set point
                                                                                                    sp = DPI620G.getSP()
                                                                                                    display("Set point value: " + str(sp))
                                                                                                    # Read control mode
                                                                                                    cm = DPI620G.getCM()
                                                                                                    display("Control mode: " + str(cm))
                                                                                                    # Read pressure type
                                                                                                    pt = DPI620G.getPT()
                                                                                                    display("Pressure type: " + str(pt))
                                                                                                    display("PV624 connected, test passed")
                                                                                                    print("Tests passed")
                                                                                                    file.close()
                                                                                                else: # PM Sensor Info
                                                                                                    display("ERROR:  PM Sensor info not available, check connection")
                                                                                                    print("Error, check connection")
                                                                                            else: # PM Sensor Cal date
                                                                                                display("ERROR:  PM Sensor cal date not available, check connection")   
                                                                                                print("Error, check connection")                                                                 
                                                                                        else: # PM Sensor serial number
                                                                                            display("ERROR:  Sensor serial number not available, check connection")
                                                                                            print("Error, check connection")
                                                                                    else: # PM Sensor Bootloader version
                                                                                        display("ERROR:  Sensor bootloader version not available, check connection")
                                                                                        print("Error, check connection")
                                                                                else: # PM Sensor bootloader dk number
                                                                                    display("ERROR:  Sensor bootloader DK not available, check connection")
                                                                                    print("Error, check connection")
                                                                            else: # PM Sensor app version number
                                                                                display("ERROR:  Sensor app version not available, check connection")
                                                                                print("Error, check connection")
                                                                        else: # PM Sensor app DK number
                                                                            display("ERROR:  Sensor app DK not available, check connection")
                                                                            print("Error, check connection")
                                                                    else: # Barometer cal points
                                                                        display("ERROR: Barometer cal points not available, check connection")
                                                                        print("Error, check connection")
                                                                else: # Barometer next cal date
                                                                    display("ERROR: Barometer next cal date not available, check connection")
                                                                    print("Error, check connection")
                                                            else: # Barometer cal interval
                                                                display("ERROR: Barometer cal interval not available, check connection")
                                                                print("Error, check connection")
                                                        else: # Barometer cal date
                                                            display("ERROR: Barometer cal date not available, check connection")
                                                            print("Error, check connection")
                                                    else: # Barometer sensor info
                                                        display("ERROR: Barometer info not acquired, check connection")
                                                        print("Error, check connection")
                                                else: # TERPS base version
                                                    display("ERROR: TERPS Sensor base version not available, check connection")
                                                    print("Error, check connection")
                                            else: # PV624 date and time
                                                display("ERROR: PV date / time not set, check connection")
                                                print("Error, check connection")
                                        else: # PV624 serial number
                                            display("ERROR: PV serial number not read, check connection")
                                            print("Error, check connection")
                                    else: # secondary app version
                                        display("ERROR: PV secondary app version not read, check connection")
                                        print("Error, check connection")
                                else: # secondary app dk
                                    display("ERROR: PV secondary app dk not read, check connection")
                                    print("Error, check connection")
                            else: # secondary boot ver
                                display("ERROR: PV secondary boot ver not read, check connection")
                                print("Error, check connection")
                        else: # secondary boot dk
                            display("ERROR: PV secondary boot dk not read, check connection")
                            print("Error, check connection")
                    else: # primary app version
                        display("ERROR: PV primary app version not read, check connection")
                        print("Error, check connection")
                else: # primary app dk
                        display("ERROR: PV primary app dk not read, check connection")
                        print("Error, check connection")
            else: #PV624 bootloader version number
                display("ERROR: PV bootloader version not read, check connection")
                print("Error, check connection")
        else: #PV624 bootloader DK number
            display("ERROR: PV bootloader DK not read, check connection")
            print("Error, check connection")
    else: # PV624 remote mode setting
        display("ERROR: Mode setting failed, check connection")
        print("Error, check connection")

# Run if not imported as a module
if __name__ == "__main__":
    main()