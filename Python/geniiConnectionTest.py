import dpi620gLib as dpi
import dpiAttributes as dpiAttr
from datetime import datetime

printIt = 1
writeLog = 1

pv624sn = ['205C324B5431']
dpi620gSn = ['FTBTA7ISA']
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
    display("PV624 Found")
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
        while dpiAttr.error['referenceSensorCommFail'] == (error and dpiAttr.error['referenceSensorCommFail']):
            # keep running the loop
            display("Sensor comms error on PV624")
            pressure, error, status, baro = DPI620G.getPV()

        # Sensor is connected, proceed with next routine
        display("Sensor connected on PV624")
        display("Querying PV624 bootloader DK")
        pvBootDk = DPI620G.getDK(dpiAttr.versionInfo['pvBootloader'])
        if pvBootDk != "":
            display("PV bootloader DK: " + str(pvBootDk))
            display("Querying PV624 bootloader version")
            pvBootVer = DPI620G.getRV(dpiAttr.versionInfo['pvBootloader'])
            if pvBootVer != "":
                display("PV bootloader version: " + str(pvBootVer))
                display("Querying PV624 serial number")
                baseSn = DPI620G.getSN(dpiAttr.serialNo['pv624'])
                if baseSn != 0:
                    display("PV624 serial number: " + str(baseSn))
                    display("Setting PV624 date and time")
                    pvDate = "05/04/2022"
                    pvTime = "01:21:33"
                    DPI620G.setSD(pvDate)
                    DPI620G.setST(pvTime)
                    pvReadDate = DPI620G.getSD()
                    pvReadTime = DPI620G.getST()
                    # Compare only date as time may have changed
                    if pvDate == pvReadDate:
                        # Assume time setting is successful too
                        display("Date and time setting success")

                        # Read the baselined sensor version, this is required if the terps requires an upgrade
                        display("Reading baselined terps sensor firmware version")
                        sensorBaseVer = DPI620G.getQV(dpiAttr.sensorBase['TERPS'])
                        if sensorBaseVer != "":
                            display("TERPS Sensor baselined firmware version: " + str(sensorBaseVer))

                            # Read sensor information
                            display("Reading barometer information")
                            minP, maxP, senType = DPI620G.getIS(dpiAttr.pvSensor['barometer'])
                            if (minP != "") and (maxP != "") and (senType != ""):
                                display("Barometer info:")
                                display("Minimum range: " + str(minP))
                                display("Maximum range: " + str(maxP))
                                display("Sensor Type: " + str(senType))
                                
                                # Read barometer brand units, barometer units are only available as mbar
                                display("Reading barometer units")
                                baroUnits = DPI620G.getBU(dpiAttr.pvSensor['barometer'])
                                if baroUnits != "":
                                    display("Barometer units: " + str(baroUnits))
                                    # Read barometer calibration information
                                    display("Reading barometer calibration date")
                                    baroCalDate = DPI620G.getCD(dpiAttr.pvSensor['barometer'])
                                    if baroCalDate != "":
                                        display("Barometer calibration date: " + str(baroCalDate))
                                        # Read barometer calibration interval
                                        ''' 
                                        Possible issue here is that the function in the code is never set to barometric measurement, 
                                        evaluate tomorrow in the tests
                                        '''
                                        display("Reading barometer calibration date")
                                        baroCalInterval = DPI620G.getCI(dpiAttr.pvSensor['barometer'])
                                        if baroCalInterval != 0:
                                            display("Barometer calibration interval " + str(baroCalInterval) + " days")
                                            # Read barometer next calibration date
                                            display("Reading barometer next calibration date")
                                            baroNextCalDate = DPI620G.getND(dpiAttr.pvSensor['barometer'])
                                            if baroNextCalDate != "":
                                                display("Barometer calibration interval " + str(baroNextCalDate))
                                                # Read number of barometer calibration points
                                                minPoints, maxPoints = DPI620G.getCN(dpiAttr.pvSensor['barometer'])
                                                if (minPoints != 0) and (maxPoints != 0):
                                                    display("Barometer calibration minimum points: " + str(minPoints))
                                                    display("Barometer calibration maximum points: " + str(maxPoints))
                                                    # Read percentage battery level of the PV624
                                                    display("Reading PV624 battery percentage")
                                                    battPercentage = DPI620G.getRB(dpiAttr.pvBattery['percentage'])
                                                    # Do not verify if battery percentage could be 0, as there could be no battery connected
                                                    display("PV624 battery percentage: " + str(battPercentage))
                                                    # Read PV one more time, dont bother what it says
                                                    display("Reading pressure from PV624")
                                                    pressure, error, status, baro = DPI620G.getPV()
                                                    display(str(pressure) + " " + str(error) + " " + str(status) + " " + str(baro))
                                                    # Read sensor app DK number
                                                    display("Reading sensor application DK number")
                                                    sensorAppDk = DPI620G.getDK(dpiAttr.versionInfo['sensorApplication'])
                                                    if sensorAppDk != "":
                                                        display("Sensor Application DK " + str(sensorAppDk))
                                                        # Read sensor app version
                                                        display("Reading sensor application version number")
                                                        sensorAppVer = DPI620G.getRV(dpiAttr.versionInfo['sensorApplication'])
                                                        if sensorAppVer != "":
                                                            display("Sensor Application version " + str(sensorAppVer))
                                                            # Read sensor bootloader DK
                                                            display("Reading sensor bootloader DK number")
                                                            sensorBootDk = DPI620G.getDK(dpiAttr.versionInfo['sensorBootloader'])
                                                            if sensorBootDk != "":
                                                                display("Sensor bootloader DK " + str(sensorBootDk))
                                                                # Read sensor bootloader version
                                                                display("Reading sensor bootloader version number")
                                                                sensorBootVer = DPI620G.getRV(dpiAttr.versionInfo['sensorBootloader'])
                                                                if sensorBootVer != "":
                                                                    display("Sensor bootloader version " + str(sensorBootVer))
                                                                    # Read sensor serial number
                                                                    display("Reading sensor serial number")
                                                                    sensorSerial = DPI620G.getSN(dpiAttr.serialNo['sensor'])
                                                                    if sensorSerial != "":
                                                                        display("Sensor serial number " + str(sensorSerial))
                                                                        # Read PM sensor calibration date 
                                                                        display("Reading PM sensor calibration date")
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
                                                                                display("Reading PM Sensor zero value")
                                                                                zeroValue = DPI620G.getIZ(dpiAttr.pvSensor['reference'])
                                                                                display("PM Zero value: " + str(zeroValue))
                                                                                # Read set point
                                                                                display("Reading set point value")
                                                                                sp = DPI620G.getSP()
                                                                                display("Set point value: " + str(sp))
                                                                                # Read control mode
                                                                                display("Reading control mode")
                                                                                cm = DPI620G.getCM()
                                                                                display("Control mode: " + str(cm))
                                                                                # Read pressure type
                                                                                display("Reading pressure type")
                                                                                pt = DPI620G.getPT()
                                                                                display("Pressure type: " + str(pt))
                                                                                display("PV624 connected, test passed")
                                                                                print("Tests passed")
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
                                else:   # Baro units
                                    display("ERROR: Barometer units not available, check connection")
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