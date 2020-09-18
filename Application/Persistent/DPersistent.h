/**
* BHGE Confidential
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
* @author   Harvinder Bhuhi
* @date     15 June 2020
*
* @brief    The persistent data storage class header file
*/

#ifndef __DPERSISTENT_H
#define __DPERSISTENT_H

#include "misra.h"

MISRAC_DISABLE
#include <os.h>
#include <stdint.h>
#include <stdbool.h>
MISRAC_ENABLE

#include "PersistentConfig.h"
#include "PersistentSettings.h"
#include "PersistentCal.h"
#include "PersistentFunctions.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define DATA_REV_INFO_ELEMENTS      7u      //allocated size of housekeeping area

/* types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_PERSIST_MAP,
    E_PERSIST_CONFIG,
    E_PERSIST_SETTINGS,
    E_PERSIST_FUNCTION_SETTINGS,
    E_PERSIST_CAL_DATA

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
        uint32_t FuncSettingsCrcError   : 1;    //1 = function settings data CRC failed
        uint32_t calDataCrcError        : 1;    //1 = cal data data CRC failed

        uint32_t reserved               : 22;   //available for more bits as needed
    };

} sPersistentDataStatus_t;

//housekeeping info data structure
typedef struct
{
    uint32_t data[DATA_REV_INFO_ELEMENTS];     //use whole area for data minus one element for crc value
    uint32_t crc;	                           //cyclic redundancy check for this data structure

} sPersistentMap_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DPersistent
{
private:
    OS_MUTEX myMutex;
    sPersistentDataStatus_t myStatus;

    uint32_t getOffset(uint32_t location_offset, ePersistentMem_t elem, uint32_t no_of_bytes);

    sPersistentMap_t mapInfo;
    sPersistentConfig_t configuration;
    sPersistentSettings_t userSettings;
    sPersistentCal_t calibrationData;
    sPersistentFunctions_t functionSettings;

    void readConfiguration(void);
    bool validateConfigData(void);
    void setDefaultConfigData(void);
    bool saveConfigData(void *srcAddr, size_t numBytes);

    void readUserSettings(void);
    bool validateUserSettings(void);
    void setDefaultUserSettings(void);
    bool saveUserSettings(void *srcAddr, size_t numBytes);

    void readFunctionSettings(void);
    bool validateFunctionSettings(void);
    void setDefaultFunctionSettings(void);
    bool saveFunctionSettings(void *srcAddr, size_t numBytes);

    void readCalibrationData(void);
    bool validateCalibrationData(void);
    bool saveCalibrationData(void *srcAddr, size_t numBytes);

    uint32_t readMapRevision(void);
    void setMapRevision(void);

    void recall(void);

public:
    DPersistent(void);

    sPersistentDataStatus_t getStatus(void);

    bool read(void *dest_addr, uint32_t location_offset, uint32_t no_of_bytes, ePersistentMem_t elem);
    bool write(void *src_addr, uint32_t location_offset, uint32_t no_of_bytes, ePersistentMem_t elem);

    //access functions
#ifdef PERSISTENT_ENABLED
    void getChannelFunction(eChannel_t channel, sChannelSetting_t *setting);
    bool setChannelFunction(eChannel_t channel, eFunction_t function, eFunctionDir_t direction);
#endif

    sPersistentFunctions_t *getFunctionSettingsAddr(void);
    sCalData_t *getCalDataAddr(void);
    void getPersistentData(void * src, void *dest, size_t size);
};

#endif /* __DPERSISTENT_H */
