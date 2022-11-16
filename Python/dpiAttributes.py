# -*- coding: utf-8 -*-
"""
Created on Mon May 24 10:07:42 2021

@author: 212596572
"""

pressureType = {}
pressureType['Gauge'] = 0x00
pressureType['Absolute'] = 0x01
pressureType['Barometer'] = 0x02

controlMode = {}
controlMode['Measure'] = 0x00
controlMode['Control'] = 0x01
controlMode['Vent'] = 0x02

error = {}
error['lowReferenceSensorVoltage'] = 1 << 0
error['referenceSensorCommFail'] = 1 << 1
error['barometerSensorFail'] = 1 << 2
error['stepperControllerFail'] = 1 << 3
error['motorVoltageFail'] = 1 << 4
error['stepperDriverFail'] = 1 << 5
error['valveFail'] = 1 << 6
error['persistentMemoryFail'] = 1 << 7
error['batteryWarningLevel'] = 1 << 8
error['batteryCriticalLevel'] = 1 << 9
error['extFlashCorrupt'] = 1 << 10
error['extFlashWriteFailure'] = 1 << 11
error['onboardFlashFail'] = 1 << 12
error['overTemperature'] = 1 << 13
error['opticalSensorFail'] = 1 << 14
error['barometerSensorMode'] = 1 << 15
error['barometerSensorCalStatus'] = 1 << 16
error['smBusBatteryComFailed'] = 1 << 17
error['smBusBatChargerComFailed'] = 1 << 18
error['chargingStatus'] = 1 << 19
error['osError'] = 1 << 20

versionInfo = {}
versionInfo['pvApplication'] = "0,0"
versionInfo['pvBootloader'] = "0,1"
versionInfo['sensorApplication'] = "1,0"
versionInfo['sensorBootloader'] = "1,1"
versionInfo['secondaryApplication'] = "0,6"
versionInfo['secondaryBootloader'] = "0,7"

serialNo = {}
serialNo['pv624'] = "0"
serialNo['sensor'] = "1"

pvSensor = {}
pvSensor['reference'] = "0"
pvSensor['barometer'] = "1"

pvBattery = {}
pvBattery['voltage'] = "0"
pvBattery['current'] = "1"
pvBattery['percentage'] = "2"
pvBattery['charge'] = "3"
pvBattery['minsToEmpty'] = "4"

sensorBase = {}
sensorBase['TERPS'] = "0,1"

connectionOwi = 0
connectionUsb = 1