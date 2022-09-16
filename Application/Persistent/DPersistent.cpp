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
* @file     DPersistent.cpp
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     15 June 2020
*
* @brief    The persistent data storage class source file
*
**********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DPersistent.h"

MISRAC_DISABLE
#include <string.h>
MISRAC_ENABLE

#include "EEPROM.h"
#include "DLock.h"
#include "crc.h"
#include "Utilities.h"

/* Error handler instance parameter starts from 3901 to 4000 */

#define DEFAULT_DAY     1u
#define DEFAULT_MONTH   1u
#define DEFAULT_YEAR    2018u
#define DEFAULT_CAL_INTERVAL 365u
/* Constants and Defines --------------------------------------------------------------------------------------------*/
/* *********** 8k byte persistent storage memory map ***************/
/*
 *   +---------------------------------------------+ offset = 0x1FFF
 *   |  Unused/free storage area (3908 bytes)      |
 *   +---------------------------------------------+
 *   |  Calibration data storage area (2k bytes)   |
 *   +---------------------------------------------+
 *   | Functions settings storage area (2k bytes)  |
 *   +---------------------------------------------+
 *   | User settings storage area (128 bytes)      |
 *   +---------------------------------------------+
 *   | Configuration storage area (128 bytes)      |
 *   +---------------------------------------------+
 *   | Housekeeping info storage area (32 bytes)   |
 *   +---------------------------------------------+ offset = 0x0000
 */
#define DEFAULT_UNIT_SERIAL_NUMBER 11110001u
#define DEFAULT_POWER_ON_INFO_VALUE 0u
//NOTE: Persistent storage has a maximum capacity of 8k bytes, so the sum of all blocks can't be greater than that
//WARNING: ALL SIZES MUST BE KEPT AS MULTIPLES OF 4
#define DATA_REV_INFO_SIZE      ((DATA_REV_INFO_ELEMENTS + 1u) * 4u) //allocated size of housekeeping area (add one element for crc)
#define DATA_CONFIG_SIZE        128u                                 //allocated size of configuration
#define DATA_SETTINGS_SIZE      128u                                 //allocated size of user settings
#define DATA_MAINTENANCE_DATA_SIZE      128u                                //allocated for function settings
#define DATA_CAL_SIZE           2048u                                //allocated size of calibration data

//Partitions follow on from each other (WARNING: DO NOT CHANGE THE ORDER OF THESE)
#define DATA_REV_INFO_START     0u                                         //start of house keeping area
#define DATA_CONFIG_START       (DATA_REV_INFO_START + DATA_REV_INFO_SIZE) //start of configuration
#define DATA_SETTINGS_START     (DATA_CONFIG_START + DATA_CONFIG_SIZE)     //start of user settings
#define DATA_MAINTENANCE_DATA_START     (DATA_SETTINGS_START + DATA_SETTINGS_SIZE) //start of function settings
#define DATA_CAL_START          (DATA_MAINTENANCE_DATA_START + DATA_MAINTENANCE_DATA_SIZE) //start of cal data
#define DATA_LIMIT              (DATA_CAL_START + DATA_CAL_SIZE)           //upper limit of used persistent storage

//free for other use
#define SPACE_AVAILABLE         (8192u - DATA_CONFIG_SIZE - DATA_SETTINGS_SIZE - DATA_FUNCTION_SIZE - DATA_CAL_SIZE - DATA_REV_INFO_SIZE)

static const uint32_t PERSIST_DATA_CONFIG_START = DATA_CONFIG_START;
static const uint32_t PERSIST_DATA_SETTINGS_START = DATA_SETTINGS_START;
static const uint32_t PERSIST_DATA_MAINTENANCE_DATA_START = DATA_MAINTENANCE_DATA_START;
static const uint32_t PERSIST_DATA_CAL_START = DATA_CAL_START;
static const uint32_t PERSIST_DATA_LIMIT = DATA_LIMIT;

#define MEMORY_MAP_REV          1u     //these are created in case data structures change in future versions
#define CONFIG_DATA_REV         1u     //these are created in case data structures change in future versions
#define USER_SETTINGS_DATA_REV  1u     //these are created in case data structures change in future versions
#define MAINTENANCE_DATA_REV  1u     //these are created in case data structures change in future versions
#define CAL_DATA_REV            1u     //these are created in case data structures change in future versions

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DPersistent class constructor
 * @param   void
 * @retval  void
 */
DPersistent::DPersistent(void)
{
    OS_ERR os_error = OS_ERR_NONE;

    myStatus.value = 0u;

    //create mutex for resource locking
    char *name = "Persistent";
    RTOSMutexCreate(&myMutex, (CPU_CHAR *)name, &os_error);

    if(os_error != (OS_ERR)OS_ERR_NONE)
    {
        myStatus.osError = 1u;
    }

    //update data structures from persistent storage
    recall();
}

/**
 * @brief   Calculate the memory offset in persistent storage for different blocks
 * @param   location_offset within the partition block
 * @param   partition block (i.e. config, settings, error log or , event log)
 * @param   no_of_bytes size of memory to be written
 * @return  Persistent memory offset location
 */
uint32_t DPersistent::getOffset(uint32_t location_offset,
                                ePersistentMem_t partition, uint32_t no_of_bytes)
{
    uint32_t offset = location_offset;
    bool overflow = false;

    switch(partition)
    {
    case E_PERSIST_MAP:    //address starts at base of persistent storage space
        offset += DATA_REV_INFO_START;

        if((offset + no_of_bytes) > PERSIST_DATA_CONFIG_START)
        {
            overflow = true;
        }

        break;

    case E_PERSIST_CONFIG: //address follows on from housekeeping data partition
        offset += PERSIST_DATA_CONFIG_START;

        if((offset + no_of_bytes) > PERSIST_DATA_SETTINGS_START)
        {
            overflow = true;
        }

        break;

    case E_PERSIST_SETTINGS:  //address follows on from configuration partition
        offset += PERSIST_DATA_SETTINGS_START;

        if((offset + no_of_bytes) > PERSIST_DATA_MAINTENANCE_DATA_START)
        {
            overflow = true;
        }

        break;

    case E_PERSIST_MAINTENANCE_DATA://address follows on from settings partition
        offset += PERSIST_DATA_MAINTENANCE_DATA_START;

        if((offset + no_of_bytes) > PERSIST_DATA_CAL_START)
        {
            overflow = true;
        }

        break;

    case E_PERSIST_CAL_DATA:  //address follows on from settings partition
        offset += PERSIST_DATA_CAL_START;

        if((offset + no_of_bytes) > PERSIST_DATA_LIMIT)
        {
            overflow = true;
        }

        break;

    default:
        /*should not ever get this*/
        break;
    }

    if(overflow == true)
    {
        offset = UINT32_MAX;
        myStatus.addrError = 1u;
    }

    return offset;
}

/**
 * @brief   Read data from persistent storage
 * @param   dest_addr address of memory location to store data read from persistent storage
 * @param   location_offset offset from start of partition
 * @param   no_of_bytes number of bytes to read
 * @param   partition block (i.e. config, settings, error log or , event log)
 * @return  success or failure
 */
bool DPersistent::read(void *dest_addr,
                       uint32_t location_offset,
                       uint32_t no_of_bytes,
                       ePersistentMem_t partition)
{
    bool successFlag = true;

    location_offset = getOffset(location_offset, partition, no_of_bytes);

    if(location_offset != UINT32_MAX)
    {
        memset_s(dest_addr, no_of_bytes, 0, no_of_bytes);
        DLock is_on(&myMutex);
        successFlag = eepromRead((uint8_t *)dest_addr,
                                 (uint16_t)location_offset,
                                 (uint16_t)no_of_bytes);
    }

    else
    {
        successFlag = false;
    }

    return successFlag;
}

/**
 * @brief   Write data to persistent storage
 * @param   src_addr address of memory location of data to be written to persistent storage
 * @param   location_offset offset from start of partition
 * @param   no_of_bytes number of bytes to write
 * @param   partition block (i.e. config, settings, error log or , event log)
 * @return  success or failure
 */
bool DPersistent::write(void *src_addr,
                        uint32_t location_offset,
                        uint32_t no_of_bytes,
                        ePersistentMem_t partition)
{
    bool successFlag = true;

    location_offset = getOffset(location_offset, partition, no_of_bytes);

    if(location_offset != UINT32_MAX)
    {
        DLock is_on(&myMutex);

        successFlag = eepromWrite((uint8_t *)src_addr,
                                  (uint16_t)location_offset,
                                  no_of_bytes);

    }

    else
    {
        successFlag = false;
    }

    return successFlag;
}

/**
 * @brief   Perform self-test
 * @param   void
 * @return  void
 */
bool DPersistent::selfTest(void)
{
    bool successFlag = false;
    myStatus.selfTestResult = 1u;       //mark self-test in progress

    successFlag = eepromTest();

    if(successFlag)
    {
        myStatus.selfTestResult = 2u;   //mark self-test passed
    }

    else
    {
        myStatus.selfTestResult = 3u;   //mark self-test failed
    }

    return successFlag;
}

/**
 * @brief   Read status of persistent storage operations (cleared on read)
 * @param   void
 * @return  status
 */
sPersistentDataStatus_t DPersistent::getStatus(void)
{
    sPersistentDataStatus_t status = myStatus;

    myStatus.value = 0u;

    return status;
}

/**
 * @brief   Read data from persistent storage data
 * @param   void
 * @return  void
 */
void DPersistent::recall(void)
{
    //check revision of memory map
    if(readMapRevision() == 0xFFFFu)
    {
        //TODO: Take whatever actions that may be required if revision is not right.
        //In initial release, it can be just set to the current (first) value
        setMapRevision();
    }

    //read persistent data
    readConfiguration();
    readUserSettings();
    readMaintenanceData();
    readCalibrationData();
}

/**
 * @brief   Read revision of persistent storage memory map.
 * @param   void
 * @retval  revision (0xFFFF signifies invalid revision)
 */
uint32_t DPersistent::readMapRevision(void)
{
    uint32_t revision = 0xFFFFu;


    memset_s(&mapInfo, sizeof(sPersistentMap_t), 0xFF, sizeof(sPersistentMap_t));


    //read instrument housekeeping area
    if(read((void *)&mapInfo, 0u, sizeof(sPersistentMap_t), E_PERSIST_MAP) == false)
    {
        myStatus.readError = 1u;
    }

    //do validation whether or not the read (above) was successful; validation will catch bad data anyway
    uint32_t crc = crc32((uint8_t *)&mapInfo.data[0], (size_t)DATA_REV_INFO_ELEMENTS);

    if(crc != mapInfo.crc)
    {
        myStatus.mapCrcError = 1u;
    }

    else
    {
        //crc is good so can use the data
        revision = mapInfo.data[0];

        if(revision > MEMORY_MAP_REV)
        {
            //invalidate it
            revision = 0xFFFFu;
            myStatus.invalidMapRevision = 1u;
        }
    }

    return revision;
}

/**
 * @brief   Set revision of persistent storage memory map to current
 * @param   void
 * @retval  void
 */
void DPersistent::setMapRevision(void)
{
    memset_s(&mapInfo, sizeof(sPersistentMap_t), 0x00, sizeof(sPersistentMap_t));
    mapInfo.data[0] = MEMORY_MAP_REV;

    //calculate new CRC
    mapInfo.crc = crc32((uint8_t *)&mapInfo.data[0],
                        (size_t)DATA_REV_INFO_ELEMENTS);

    //update the data in persistent storage
    if(write((void *)&mapInfo, 0u,
             sizeof(sPersistentMap_t),
             E_PERSIST_MAP) == false)
    {
        myStatus.writeError = 1u;
    }
}

/**
 * @brief   Read configuration data from persistent storage data
 * @param   void
 * @return  void
 */
bool DPersistent::readConfiguration(void)
{
#ifdef DEBUG
    memset_s(&configuration.data, sizeof(sConfig_t), 0xFF, sizeof(sConfig_t));
#endif
    bool successFlag = true;

    //read instrument configuration
    if(read((void *)&configuration,
            0u,
            sizeof(sPersistentConfig_t),
            E_PERSIST_CONFIG) == false)
    {
        myStatus.readError = 1u;
        successFlag = false;
    }

    //do validation whether or not the read (above) was successful; validation will catch bad data anyway
    if(validateConfigData() == false)
    {
        myStatus.configCrcError = 1u;

        //set defaults
        setDefaultConfigData();

        //calculate new CRC
        configuration.crc = crc32((uint8_t *)&configuration.data,
                                  sizeof(sConfig_t));

        //update the data in persistent storage

        if(write((void *)&configuration,
                 0u,
                 sizeof(sPersistentConfig_t),
                 E_PERSIST_CONFIG) == false)
        {
            myStatus.writeError = 1u;
        }

        successFlag = false;
        //TODO: if region has been reset then update dependent settings too
    }



    return successFlag;
}

/**
 * @brief   Perform CRC validation on configuration data in persistent storage
 * @param   void
 * @return  flag: true if crc check passes, else false
 */
bool DPersistent::validateConfigData(void)
{
    bool successFlag = false;

    uint32_t crc = crc32((uint8_t *)&configuration.data, sizeof(sConfig_t));

    if(crc == configuration.crc)
    {
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Set default instrument calibration
 * @param   void
 * @return  void
 */
void DPersistent::setDefaultConfigData(void)
{
    //set config to defaults
    memset_s(&configuration.data, sizeof(sConfig_t), 0x00, sizeof(sConfig_t));

    sConfig_t *config = &configuration.data;

    config->revision = CONFIG_DATA_REV;                         //set revision to latest one
    config->serialNumber = 0XFFFFFFFFu;
    config->region = E_REGION_NOT_SET;
    config->instrumentType = E_INSTRUMENT_TYPE_STD;
}

/**
 * @brief   Save config data to persistent storage
 * @param   srcAddr: RAM address of data to be stored
 * @param   numBytes: number of bytes to be written
 * @return  successFlag: true is ok, else false
 */
bool DPersistent::saveConfigData(void *srcAddr, size_t numBytes)
{
    //calculate offset in persistent storage
    uint32_t startAddr = (uint32_t)&configuration;      //start (base address) of persistent storage structure
    uint32_t locationAddr = (uint32_t)srcAddr;          //location we are writing to

    //offset is the difference between them - assuming that startAddr <= locationAddr
    uint32_t offset = locationAddr - startAddr;

    bool successFlag = write(srcAddr, offset, numBytes, E_PERSIST_CONFIG);

    if(successFlag == true)
    {
        //if write was ok then need to calculate and save new CRC
        configuration.crc = crc32((uint8_t *)&configuration.data,
                                  sizeof(sConfig_t));

        locationAddr = (uint32_t)&configuration.crc;

        successFlag = write((void *)locationAddr,
                            (locationAddr - startAddr),
                            sizeof(uint32_t),
                            E_PERSIST_CONFIG);
    }

    return successFlag;
}

/**
 * @brief   Save all config data to persistent storage
 * @param   void
 * @return  flag: true is ok, else false
 */
bool DPersistent::saveConfigData(void)
{
    bool successFlag = false;
    //calculate and new CRC
    configuration.crc = crc32((uint8_t *)&configuration.data, sizeof(sConfig_t));

    //write back the whole data structure
    successFlag = write((void *)&configuration,
                        0u,
                        sizeof(sPersistentConfig_t),
                        E_PERSIST_CONFIG);

    if(true == successFlag)
    {
        successFlag = readConfiguration();
    }

    return successFlag;
}

/**
 * @brief   Get serial number
 * @note    Instrument "serial number" if actually a free format ASCII character string
 * @param   void
 * @retval  character string
 */
uint32_t DPersistent::getSerialNumber(void)
{
    uint32_t serailno = DEFAULT_UNIT_SERIAL_NUMBER;

    if(configuration.data.serialNumberSetStatus == E_PARAM_ALREADY_SET)
    {
        serailno = configuration.data.serialNumber;
    }

    return serailno;
}


/**
 * @brief   Set serial number
 * @note    Instrument "serial number" if actually a free format ASCII character string
 * @param   str - string
 * @retval  true = success, false = failed
 */
bool DPersistent::setSerialNumber(uint32_t newSerialNumber)
{
    bool successFlag  = false;

    configuration.data.serialNumber = newSerialNumber;
    configuration.data.serialNumberSetStatus = E_PARAM_ALREADY_SET;
    successFlag = saveConfigData();
    return successFlag;
}

/**
 * @brief   Read user settings data from persistent storage data
 * @param   void
 * @return  void
 */
void DPersistent::readUserSettings(void)
{


    //read instrument user settings
    if(read((void *)&userSettings, 0u, sizeof(sPersistentSettings_t),
            E_PERSIST_SETTINGS) == false)
    {
        myStatus.readError = 1u;
    }

    //do validation whether or not the read (above) was successful; validation will catch bad data anyway
    if(validateUserSettings() == false)
    {
        myStatus.userSettingsCrcError = 1u;

        //set defaults
        setDefaultUserSettings();

        //calculate new CRC
        userSettings.crc = crc32((uint8_t *)&userSettings.data, sizeof(sUserSettings_t));

        //update the data in persistent storage
        if(write((void *)&userSettings, 0u, sizeof(sPersistentSettings_t), E_PERSIST_SETTINGS) == false)
        {
            myStatus.writeError = 1u;
        }
    }
}

/**
 * @brief   Perform CRC validation on user settings data in persistent storage
 * @param   void
 * @return  successFlag: true if crc check passes, else false
 */
bool DPersistent::validateUserSettings(void)
{
    bool successFlag = false;

    uint32_t crc = crc32((uint8_t *)&userSettings.data, sizeof(sUserSettings_t));

    if(crc == userSettings.crc)
    {
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Set default user settings
 * @param   void
 * @return  void
 */
void DPersistent::setDefaultUserSettings(void)
{
    //set user settings to defaults
    memset_s(&userSettings.data, sizeof(sUserSettings_t), 0x00, sizeof(sUserSettings_t));

    sUserSettings_t *settings = &userSettings.data;

    settings->revision = USER_SETTINGS_DATA_REV;                        //revision set to latest one
    settings->calPin = 4321u;                                           //default user calibration PIN = 4321
    settings->autoPowerdown = 10u;                                      //default auto-powerdown time set to 10 minutes

}

/**
 * @brief   Read function settings data from persistent storage data
 * @param   void
 * @return  void
 */
void DPersistent::readMaintenanceData(void)
{

    memset_s(&maintenanceData.data, sizeof(sMaintenanceData_t), 0xFF, sizeof(sMaintenanceData_t));


    //read instrument function settings
    if(read((void *)&maintenanceData, (uint32_t)0u, sizeof(sPersistentMaintenanceData_t), E_PERSIST_MAINTENANCE_DATA) == false)
    {
        myStatus.readError = 1u;
    }

    //do validation whether or not the read (above) was successful; validation will catch bad data anyway
    if(validateMaintenanceData() == false)
    {
        myStatus.maintenanceDataCrcError = 1u;

        //set defaults
        setDefaultMaintenanceData();

        //calculate new CRC
        maintenanceData.crc = crc32((uint8_t *)&maintenanceData.data, sizeof(sMaintenanceData_t));

        //update the data in persistent storage
        if(write((void *)&maintenanceData, 0u, sizeof(sPersistentMaintenanceData_t), E_PERSIST_MAINTENANCE_DATA) == false)
        {
            myStatus.writeError = 1u;
        }
    }
}

/**
 * @brief   Perform CRC validation on function settings data in persistent storage
 * @param   void
 * @return  flag: true if crc check passes, else false
 */
bool DPersistent::validateMaintenanceData(void)
{
    bool successFlag = false;

    uint32_t crc = crc32((uint8_t *)&maintenanceData.data, sizeof(sMaintenanceData_t));

    if(crc == maintenanceData.crc)
    {
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Set default function settings
 * @param   void
 * @return  void
 */
void DPersistent::setDefaultMaintenanceData(void)
{
    //set function settings to defaults
    memset_s(&maintenanceData.data, sizeof(sMaintenanceData_t), 0x00, sizeof(sMaintenanceData_t));

    sMaintenanceData_t *settings = &maintenanceData.data;

    settings->revision = MAINTENANCE_DATA_REV;            //revision set to latest one

    //ToDo: set default values of  maintenanace data
    //units selected
}

bool DPersistent::saveMaintenanceData(void)
{
    //calculate and new CRC
    maintenanceData.crc = crc32((uint8_t *)&maintenanceData.data, sizeof(sMaintenanceData_t));

    //write back the whole data structure
    return write((void *)&maintenanceData, (uint32_t)0u, sizeof(sPersistentMaintenanceData_t), E_PERSIST_MAINTENANCE_DATA);
}

/**
 * @brief   Save function settings to persistent storage
 * @param   srcAddr: RAM address of data to be stored
 * @param   numBytes: number of bytes to be written
 * @return  successFlag: true is ok, else false
 */
bool DPersistent::saveMaintenanceData(void *srcAddr, size_t numBytes)
{
    //calculate offset in persistent storage
    uint32_t startAddr = (uint32_t)&maintenanceData;   //start (base address) of persistent storage structure
    uint32_t locationAddr = (uint32_t)srcAddr;          //location we are writing to

    //offset is the difference between them - assuming that startAddr <= locationAddr
    uint32_t offset = locationAddr - startAddr;

    bool successFlag = write(srcAddr, offset, numBytes, E_PERSIST_MAINTENANCE_DATA);

    if(successFlag == true)
    {
        //if write was ok then need to calculate and save new CRC
        maintenanceData.crc = crc32((uint8_t *)&maintenanceData.data, sizeof(sMaintenanceData_t));

        locationAddr = (uint32_t)&maintenanceData.crc;

        successFlag = write((void *)locationAddr, (locationAddr - startAddr), sizeof(uint32_t), E_PERSIST_MAINTENANCE_DATA);
    }

    return successFlag;
}

bool DPersistent::setMaintenanceData(sMaintenanceData_t *mainData)
{
    bool flag = false;

    return flag;
}

/**
 * @brief   Read calibration data from persistent storage data
 * @param   void
 * @return  void
 */
bool DPersistent::readCalibrationData(ePersistentMem_t persistentMemArea)
{

    memset_s(&calibrationData.data, sizeof(sCalData_t), 0xFF, sizeof(sCalData_t));


    //read instrument user settings
    if(read((void *)&calibrationData, (uint32_t)0u, sizeof(sPersistentCal_t), persistentMemArea) == false)
    {
        myStatus.readError = 1u;
    }

    //if read ok then do validation; validation will catch bad data anyway
    bool successFlag = validateCalibrationData();
    sCalData_t *calData = &calibrationData.data;

    if(successFlag == false)
    {
        //only modify and write back if current calibration block is being read
        if(persistentMemArea == (ePersistentMem_t)E_PERSIST_CAL_DATA)
        {
            myStatus.calDataCrcError = 1u;

            //NOTE: It is left for the sensor to validate its own cal data at the point of use. However, we need to update
            //the housekeeping data by setting it to sensible defaults and write it back to persistent storage

            calData->revision = CAL_DATA_REV;

            //set invalid creation or 'last modified' date
            calData->modifiedDate.day = DEFAULT_DAY;
            calData->modifiedDate.month = DEFAULT_MONTH;
            calData->modifiedDate.year = DEFAULT_YEAR;

            //set invalid instrument cal date
            calData->calDate.day = DEFAULT_DAY;
            calData->calDate.month = DEFAULT_MONTH;
            calData->calDate.year = DEFAULT_YEAR;

            //set invalid instrument calibration interval
            calData->calInterval = DEFAULT_CAL_INTERVAL;


            saveCalibrationData();
        }
    }


    return successFlag;

}

/**
 * @brief   Perform CRC validation on calibration data in persistent storage
 * @param   void
 * @return  successFlag: true if crc check passes, else false
 */
bool DPersistent::validateCalibrationData(void)
{
    bool successFlag = false;

    uint32_t crc = crc32((uint8_t *)&calibrationData.data, sizeof(sCalData_t));

    if(crc == calibrationData.crc)
    {
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Save all current calibration data to persistent storage
 * @param   none
 * @return  flag: true is ok, else false
 */
bool DPersistent::saveCalibrationData(void)
{
    //calculate new CRC value
    calibrationData.crc = crc32((uint8_t *)&calibrationData.data, sizeof(sCalData_t));

    //write back the whole data structure
    return write((void *)&calibrationData, (uint32_t)0u, sizeof(sPersistentCal_t), E_PERSIST_CAL_DATA);
}

/**
 * @brief   Save calibration data to persistent storage
 * @param   srcAddr: RAM address of data to be stored
 * @param   numBytes: number of bytes to be written
 * @param   persistentMemArea is the cal data area to write to (default parameter value is E_PERSIST_CAL_DATA)
 * @return  flag: true is ok, else false
 */
bool DPersistent::saveCalibrationData(void *srcAddr, size_t numBytes, ePersistentMem_t persistentMemArea)
{
    calibrationData.crc = crc32((uint8_t *)&calibrationData.data, sizeof(sCalData_t));
    return write((void *)&calibrationData, (uint32_t)0u, sizeof(sPersistentCal_t), persistentMemArea);
}

/**
 * @brief   Read specified number of bytes from from calibration data in persistent storage
 * @param   srcAddr: RAM address of data to be stored
 * @param   numBytes: number of bytes to be written
 * @return  flag: true is ok, else false
 */
bool DPersistent::loadCalibrationData(void *srcAddr, size_t numBytes)
{
    //calculate offset in persistent storage
    uint32_t startAddr = (uint32_t)&calibrationData;    //start (base address) of persistent storage structure
    uint32_t locationAddr = (uint32_t)srcAddr;          //location we are writing to

    //offset is the difference between them - assuming that startAddr <= locationAddr
    uint32_t offset = locationAddr - startAddr;

    return read(srcAddr, offset, numBytes, E_PERSIST_CAL_DATA);
}

/**
 * @brief   Load cal from previously saved factory cal data
 * @param   void
 * @return  true if successful, false if factory cal has not been saved or is invalid
 */
bool DPersistent::loadFactoryCalibration(void)
{
    bool success = readCalibrationData(E_PERSIST_FACTORY_CAL_DATA);

    if(success == false)
    {
        //if failed then load (or reload) last in-use calibration data
        readCalibrationData(E_PERSIST_CAL_DATA);
    }

    return success;
}

/**
 * @brief   Save current cal data as factory cal
 * @param   void
 * @return  true if successful, false if failed
 */
bool DPersistent::saveAsFactoryCalibration(void)
{
    //Update the 'last modifed' date and save it within the saved data
    getSystemDate(&calibrationData.data.modifiedDate);

    return saveCalibrationData((void *&)calibrationData, sizeof(sCalData_t), E_PERSIST_FACTORY_CAL_DATA);
}

/**
 * @brief   Load cal from previously backed up cal data
 * @param   void
 * @return  true if successful, false if backup does not exist or is invalid
 */
bool DPersistent::loadBackupCalibration(void)
{
    bool success = readCalibrationData(E_PERSIST_BACKUP_CAL_DATA);

    if(success == false)
    {
        //if failed then load (or reload) last in-use calibration data
        readCalibrationData(E_PERSIST_CAL_DATA);
    }

    return success;
}

/**
 * @brief   Saved current cal data as backup cal
 * @param   void
 * @return  true if successful, false if failed
 */
bool DPersistent::saveAsBackupCalibration(void)
{
    //Update the 'last modifed' date and save it within the saved data
    getSystemDate(&calibrationData.data.modifiedDate);

    return saveCalibrationData((void *&)calibrationData, sizeof(sCalData_t), E_PERSIST_BACKUP_CAL_DATA);
}



/**
 * @brief   Get address from function settings partition
 * @return  pointer to function settings data
 */
sMaintenanceData_t *DPersistent::getMaintenanceDatasAddr(void)
{
    return &maintenanceData.data;
}

/**
 * @brief   Copy data
 * @param   dest is the destination (start) address
 * @param   destSize size of the destination buffer
 * @param   src is the source (start) address
 * @param   size if the number of bytes to copy from source to destination
 * @return  pointer to function settings data
 */
void DPersistent::copyPersistentData(void *dest, size_t destSize, void *src, size_t size)
{
    DLock is_on(&myMutex);
    memcpy_s(dest, destSize, src, size);
}

/**
 * @brief   Get address from cal data block
 * @return  pointer to cal data
 */
sCalData_t *DPersistent::getCalDataAddr(void)
{
    return &calibrationData.data;
}

/**
 * @brief   Get address from configuration data partition
 * @return  pointer to config data
 */
sConfig_t *DPersistent::getConfigDataAddr(void)
{
    return &configuration.data;
}

/**
 * @brief   Get  calibration Interval
 * @return  cal interval data
 */
uint32_t DPersistent::getCalInterval(void)
{

    uint32_t interval = DEFAULT_CAL_INTERVAL;

    if(calibrationData.data.measureBarometer.data.calIntervalSetStatus == E_PARAM_ALREADY_SET)
    {
        interval = calibrationData.data.measureBarometer.data.calInterval;
    }

    return interval;
}

/**
 * @brief   Set serial number
 * @note    Instrument "serial number" if actually a free format ASCII character string
 * @param   str - string
 * @retval  true = success, false = failed
 */
bool DPersistent::setCalInterval(uint32_t newCalInterval)
{

    bool successFlag  = false;

    calibrationData.data.measureBarometer.data.calInterval = newCalInterval;
    calibrationData.data.measureBarometer.data.calIntervalSetStatus = E_PARAM_ALREADY_SET;
    successFlag = saveCalibrationData();
    return successFlag;
}



/**
 * @brief   Invalidate Calibration data serial number
 * @note    After calling this instrument treated as not calibrated.
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPersistent::invalidateCalibrationData(void)
{
    bool successFlag  = false;
    myStatus.invalidateCalOperationResult = 1u;       //mark self-test in progress

    calibrationData.data.measureBarometer.calStatus = SENSOR_NOT_CALIBRATED;

    successFlag = saveCalibrationData();

    if(true  == successFlag)
    {
        myStatus.invalidateCalOperationResult = 2u;   //mark self-test passed
    }

    else
    {
        myStatus.invalidateCalOperationResult = 3u;   //mark self-test failed
    }

    return successFlag;
}

/**
 * @brief   increments the setpoint count and saves into eeprom
 * @param   void
 * @retval  true if saved  sucessfully false if save fails
 */
bool DPersistent::incrementSetPointCount(uint32_t *pNewSetPointCount)
{
    bool successFlag  = false;

    maintenanceData.data.numOfSetPoints = maintenanceData.data.numOfSetPoints + 1u;
    successFlag = saveMaintenanceData();

    if(true  == successFlag)
    {
        *pNewSetPointCount = maintenanceData.data.numOfSetPoints;
        readMaintenanceData();

        if(*pNewSetPointCount == maintenanceData.data.numOfSetPoints)
        {
            successFlag = true;
        }

        else
        {
            successFlag = false;
        }
    }

    return successFlag;
}

/**
 * @brief   retruns set point count
 * @param   void
 * @retval  setPointCount
 */
uint32_t DPersistent::getSetPointCount(void)
{
    return maintenanceData.data.numOfSetPoints;
}
/**
 * @brief   update distance travelled by piston into eeprom
 * @param   float32_t distance travelled
 * @retval  true if saved  sucessfully false if save fails
 */
bool DPersistent::updateDistanceTravelled(float32_t distanceTravelled)
{
    bool successFlag  = false;

    maintenanceData.data.distanceTravelled = distanceTravelled;
    successFlag = saveMaintenanceData();

    return successFlag;
}

/**
 * @brief   gives distance travelled by the piston
 * @param   void
 * @retval  returns distance travelled by the piston
 */
float32_t DPersistent::getDistanceTravelled(void)
{
    return maintenanceData.data.distanceTravelled;
}

/**
 * @brief   gives the manufacturing date
 * @param   sDate_t *  pointer to date structure to return namufacturing date
 * @retval  true or false  status
 */
bool DPersistent::getManufacturingDate(sDate_t *manufDate)
{
    bool successFlag = false;

    if(manufDate != NULL)
    {
        successFlag = true;
        manufDate->day = DEFAULT_DAY;
        manufDate->month = DEFAULT_MONTH;
        manufDate->year = DEFAULT_YEAR;

        if(configuration.data.manfDateSetStatus == E_PARAM_ALREADY_SET)
        {
            manufDate->day = configuration.data.manfacturingDate.day;
            manufDate->month = configuration.data.manfacturingDate.month;
            manufDate->year = configuration.data.manfacturingDate.year;

        }
    }

    return successFlag;
}

/**
 * @brief   Set PV624 manufacturing date and saves the date into EEPROM
 * @param   sDate_t *   manufacturing date
 * @retval  true if  manufacturing date successfully saved otherwise returns false
 */
bool DPersistent::setManufacturingDate(sDate_t *manufDate)
{
    bool successFlag = false;

    if(manufDate != NULL)
    {
        configuration.data.manfDateSetStatus = E_PARAM_ALREADY_SET;
        configuration.data.manfacturingDate.day = manufDate->day;
        configuration.data.manfacturingDate.month = manufDate->month;
        configuration.data.manfacturingDate.year = manufDate->year;
        successFlag = saveConfigData();
    }

    return successFlag;
}

/**
 * @brief   gives the next calibration date
 * @param   sDate_t *  pointer to date structure to return next calibration date
 * @retval  true or false  status
 */
bool DPersistent::getNextCalDate(sDate_t *nextCalDate)
{
    bool successFlag = false;

    if(nextCalDate != NULL)
    {
        successFlag = true;
        nextCalDate->day = DEFAULT_DAY;
        nextCalDate->month = DEFAULT_MONTH;
        nextCalDate->year = DEFAULT_YEAR;

        if((calibrationData.data.measureBarometer.data.nextCalDateSetStatus == (uint32_t)E_PARAM_ALREADY_SET) &&
                (calibrationData.data.measureBarometer.calStatus  == (uint32_t)SENSOR_CALIBRATED))
        {
            nextCalDate->day = calibrationData.data.measureBarometer.data.nextCalDate.day;
            nextCalDate->month = calibrationData.data.measureBarometer.data.nextCalDate.month;
            nextCalDate->year = calibrationData.data.measureBarometer.data.nextCalDate.year;

        }
    }

    return successFlag;
}

/**
 * @brief   Set PV624 barometer next calibration and saves the date into EEPROM
 * @param   sDate_t *   next alibration date
 * @retval  true if  next calibration date successfully saved otherwise returns false
 */
bool DPersistent::setNextCalDate(sDate_t *nextCalDate)
{
    bool successFlag = false;

    if(nextCalDate != NULL)
    {
        calibrationData.data.measureBarometer.data.nextCalDateSetStatus = (uint32_t)E_PARAM_ALREADY_SET;

        calibrationData.data.measureBarometer.data.nextCalDate.day = nextCalDate->day;
        calibrationData.data.measureBarometer.data.nextCalDate.month = nextCalDate->month ;
        calibrationData.data.measureBarometer.data.nextCalDate.year = nextCalDate->year;
        successFlag = saveCalibrationData();
    }

    return successFlag;
}

/**
 * @brief   gives the calibration date
 * @param   sDate_t *  pointer to date structure to return calibration date
 * @retval  true or false  status
 */
bool DPersistent::getCalibrationDate(sDate_t *calDate)
{
    bool successFlag = false;

    if(calDate != NULL)
    {
        successFlag = true;
        calDate->day = DEFAULT_DAY;
        calDate->month = DEFAULT_MONTH;
        calDate->year = DEFAULT_YEAR;

        if((calibrationData.data.measureBarometer.data.calDateSetStatus == (uint32_t)E_PARAM_ALREADY_SET) &&
                (calibrationData.data.measureBarometer.calStatus  == (uint32_t)SENSOR_CALIBRATED))
        {
            calDate->day = calibrationData.data.measureBarometer.data.calDate.day;
            calDate->month = calibrationData.data.measureBarometer.data.calDate.month;
            calDate->year = calibrationData.data.measureBarometer.data.calDate.year;

        }
    }

    return successFlag;
}
/**
 * @brief   Set PV624 barometer calibrationdate and saves the date into EEPROM
 * @param   sDate_t *  calibration date
 * @retval  true if calibration date successfully saved otherwise returns false
 */

bool DPersistent::setCalibrationDate(sDate_t *calDate)
{
    bool successFlag = false;

    if(calDate != NULL)
    {

        calibrationData.data.measureBarometer.data.calDateSetStatus = (uint32_t)E_PARAM_ALREADY_SET;

        calibrationData.data.measureBarometer.data.calDate.day = calDate->day;
        calibrationData.data.measureBarometer.data.calDate.month = calDate->month ;
        calibrationData.data.measureBarometer.data.calDate.year = calDate->year;
        successFlag = saveCalibrationData();

    }

    return successFlag;
}

/**
 * @brief   clears set point count and distance travelled
 * @param   void
 * @retval  true if cleared sucessfully otherwise returns false
 */
bool DPersistent::clearMaintainceData(void)
{
    bool successFlag  = false;

    maintenanceData.data.numOfSetPoints = 0u;
    maintenanceData.data.distanceTravelled = 0.0f;
    successFlag = saveMaintenanceData();

    if(successFlag)
    {
        readMaintenanceData();

        if(0u == maintenanceData.data.numOfSetPoints)
        {
            successFlag = true;
        }

        else
        {
            successFlag = false;
        }
    }

    return successFlag;
}
/**
 * @brief   get the barometer calibration data
 * @param   float* pointer to float array to return calibration offsets
 * @retval  true if cleared sucessfully otherwise returns false
 */
bool DPersistent::getCalOffsets(float32_t *pCalOffsets)
{
    bool successFlag  = false;



    if(NULL  != pCalOffsets)
    {
        successFlag = true;

        if(SENSOR_CALIBRATED == calibrationData.data.measureBarometer.calStatus)
        {
            pCalOffsets[0] = calibrationData.data.measureBarometer.data.calPoints[0].x;
            pCalOffsets[1] = calibrationData.data.measureBarometer.data.calPoints[0].y;
            pCalOffsets[2] = calibrationData.data.measureBarometer.data.calPoints[1].x;
            pCalOffsets[3] = calibrationData.data.measureBarometer.data.calPoints[1].y;
        }

        else
        {
            pCalOffsets[0] = 0.0f;
            pCalOffsets[1] = 0.0f;
            pCalOffsets[2] = 0.0f;
            pCalOffsets[3] = 0.0f;
        }
    }

    return successFlag;
}

/**
 * @brief   returns barometer calibration status
 * @param   void
 * @retval  returns true if it si calibrated otherwise returns false
 */
bool DPersistent::getCalibrationStatus(void)
{
    bool calStatusFlag = false;

    if(SENSOR_CALIBRATED == calibrationData.data.measureBarometer.calStatus)
    {
        calStatusFlag = true;
    }

    return calStatusFlag;
}


/**
 * @brief   update set point count into eeprom
 * @param   uint32_t new set point count
 * @retval  true if saved  sucessfully false if save fails
 */
bool DPersistent::updateSetPointCount(uint32_t setPointCount)
{
    bool successFlag  = false;

    maintenanceData.data.numOfSetPoints = setPointCount;
    successFlag = saveMaintenanceData();


    return successFlag;
}

