/**
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DPersistent.h
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     15 June 2020
*
* @brief    The persistent data storage class header file
*/
//*********************************************************************************************************************
#ifndef __DPERSISTENT_H
#define __DPERSISTENT_H

#include "misra.h"

MISRAC_DISABLE
#include <rtos.h>
#include <stdint.h>
#include <stdbool.h>
MISRAC_ENABLE

#include "PersistentConfig.h"
#include "PersistentSettings.h"
#include "PersistentCal.h"
#include "PersistentMaintenanceData.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define DATA_REV_INFO_ELEMENTS      7u      //allocated size of housekeeping area

/* types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_PERSIST_MAP,
    E_PERSIST_CONFIG,
    E_PERSIST_SETTINGS,
    E_PERSIST_MAINTENANCE_DATA,
    E_PERSIST_CAL_DATA,
    E_PERSIST_FACTORY_CAL_DATA,
    E_PERSIST_BACKUP_CAL_DATA,
    E_PERSIST_ERROR_LOG,
    E_PERSIST_EVENT_LOG

} ePersistentMem_t;

//persistent data status
typedef union
{
    uint32_t value;

    struct
    {
        uint32_t addrError              : 1;    //1 = address out of range
        uint32_t osError                : 1;    //1 = OS operation failed
        uint32_t readError              : 1;    //1 = failed to read persistent storage
        uint32_t writeError             : 1;    //1 = failed to write to persistent storage
        uint32_t mapCrcError            : 1;    //1 = map info data CRC failed
        uint32_t invalidMapRevision     : 1;    //1 = map version is not valid
        uint32_t configCrcError         : 1;    //1 = configuration data CRC failed
        uint32_t userSettingsCrcError   : 1;    //1 = user settings data CRC failed
        uint32_t maintenanceDataCrcError   : 1;    //1 = function settings data CRC failed
        uint32_t calDataCrcError        : 1;    //1 = cal data data CRC failed
        uint32_t errorLogCrcError       : 1;    //1 = error log CRC failed
        uint32_t eventLogCrcError       : 1;    //1 = event log CRC failed

        uint32_t selfTestResult         : 2;    //value: 0 = in progress; 1 = pass; -1 = fail
        uint32_t invalidateCalOperationResult : 2;    //value: 0 = in progress; 1 = pass; -1 = fail
        uint32_t reserved               : 20;   //available for more bits as needed
    };

} sPersistentDataStatus_t;

//housekeeping info data structure
typedef struct
{
    uint32_t data[DATA_REV_INFO_ELEMENTS];     //use whole area for data minus one element for crc value
    uint32_t crc;                              //cyclic redundancy check for this data structure

} sPersistentMap_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DPersistent
{
private:
    OS_MUTEX myMutex;
    sPersistentDataStatus_t myStatus;


    sPersistentMap_t mapInfo;
    sPersistentConfig_t configuration;
    sPersistentSettings_t userSettings;
    sPersistentCal_t calibrationData;
    sPersistentMaintenanceData_t maintenanceData;

    uint32_t getOffset(uint32_t location_offset, ePersistentMem_t elem, uint32_t no_of_bytes);

    // bool readConfiguration(void);
    bool validateConfigData(void);
    void setDefaultConfigData(void);

    void readUserSettings(void);
    bool validateUserSettings(void);
    void setDefaultUserSettings(void);

    void readMaintenanceData(void);
    bool validateMaintenanceData(void);
    void setDefaultMaintenanceData(void);


    bool readCalibrationData(ePersistentMem_t persistentMemArea = E_PERSIST_CAL_DATA);
    bool validateCalibrationData(void);


    uint32_t readMapRevision(void);
    void setMapRevision(void);

    void recall(void);

public:
    DPersistent(void);

    //general functions
    bool read(void *dest_addr, uint32_t location_offset, uint32_t no_of_bytes, ePersistentMem_t elem);
    bool write(void *src_addr, uint32_t location_offset, uint32_t no_of_bytes, ePersistentMem_t elem);

    void copyPersistentData(void *src, void *dest, size_t size);

    void selfTest(void);

    sPersistentDataStatus_t getStatus(void);

    //cal data access functions
    sCalData_t *getCalDataAddr(void);

    bool getCalibrationDate(sDate_t *calDate);
    bool setCalibrationDate(sDate_t *calDate);

    uint32_t getCalInterval(void);
    bool setCalInterval(uint32_t newCalInterval);

    bool loadCalibrationData(void *srcAddr, size_t numBytes);
    bool loadFactoryCalibration(void);
    bool loadBackupCalibration(void);

    bool invalidateCalibrationData(void);
    bool saveCalibrationData(void);
    bool saveCalibrationData(void *srcAddr, size_t numBytes, ePersistentMem_t persistentMemArea = E_PERSIST_CAL_DATA);

    bool saveAsFactoryCalibration(void);
    bool saveAsBackupCalibration(void);

    //Maintainence data access functions

    sMaintenanceData_t *getMaintenanceDatasAddr(void);
    bool setMaintenanceData(sMaintenanceData_t *mainData);
    bool saveMaintenanceData(void *srcAddr, size_t numBytes);
    bool saveMaintenanceData(void);

    sConfig_t *getConfigDataAddr(void);
    bool readConfiguration(void);
    //config access functions
    uint32_t getSerialNumber(void);
    bool setSerialNumber(uint32_t newSerialNumber);

    bool saveConfigData(void);
    bool saveConfigData(void *srcAddr, size_t numBytes);

    uint32_t getAutoPowerdown(void);
    bool setAutoPowerdown(uint32_t autoPowerdown);

    bool saveUserSettings(void);
    bool saveUserSettings(void *srcAddr, size_t numBytes);

    bool incrementSetPointCount(uint32_t *pNewSetPointCount);
    uint32_t getSetPointCount(void);

    bool updateDistanceTravelled(float32_t distanceTravelled);
    float32_t getDistanceTravelled(void);

    bool clearMaintainceData(void);

    bool getManufacturingDate(sDate_t *manufDate);
    bool setManufacturingDate(sDate_t *manufDate);

    bool getNextCalDate(sDate_t *nextCalDate);
    bool setNextCalDate(sDate_t *nextCalDate);
    bool getCalOffsets(float *pCalOffsets);

};

#endif /* __DPERSISTENT_H */
