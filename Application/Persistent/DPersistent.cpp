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
* @file     DPersistent.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
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

//NOTE: Persistent storage has a maximum capacity of 8k bytes, so the sum of all blocks can't be greater than that
//WARNING: ALL SIZES MUST BE KEPT AS MULTIPLES OF 4
#define DATA_REV_INFO_SIZE      ((DATA_REV_INFO_ELEMENTS + 1u) * 4u) //allocated size of housekeeping area (add one element for crc)
#define DATA_CONFIG_SIZE        128u                                 //allocated size of configuration
#define DATA_SETTINGS_SIZE      128u                                 //allocated size of user settings
#define DATA_FUNCTION_SIZE      2048u                                //allocated for function settings
#define DATA_CAL_SIZE           2048u                                //allocated size of calibration data

//Partitions follow on from each other (WARNING: DO NOT CHANGE THE ORDER OF THESE)
#define DATA_REV_INFO_START     0u                                         //start of house keeping area
#define DATA_CONFIG_START       (DATA_REV_INFO_START + DATA_REV_INFO_SIZE) //start of configuration
#define DATA_SETTINGS_START     (DATA_CONFIG_START + DATA_CONFIG_SIZE)     //start of user settings
#define DATA_FUNCTION_START     (DATA_SETTINGS_START + DATA_SETTINGS_SIZE) //start of function settings
#define DATA_CAL_START          (DATA_FUNCTION_START + DATA_FUNCTION_SIZE) //start of cal data
#define DATA_LIMIT              (DATA_CAL_START + DATA_CAL_SIZE)           //upper limit of used persistent storage

//free for other use
#define SPACE_AVAILABLE         (8192u - DATA_CONFIG_SIZE - DATA_SETTINGS_SIZE - DATA_FUNCTION_SIZE - DATA_CAL_SIZE - DATA_REV_INFO_SIZE)

static const uint32_t PERSIST_DATA_CONFIG_START = DATA_CONFIG_START;
static const uint32_t PERSIST_DATA_SETTINGS_START = DATA_SETTINGS_START;
static const uint32_t PERSIST_DATA_FUNCTION_START = DATA_FUNCTION_START;
static const uint32_t PERSIST_DATA_CAL_START = DATA_CAL_START;
static const uint32_t PERSIST_DATA_LIMIT = DATA_LIMIT;

#define MEMORY_MAP_REV          1u     //these are created in case data structures change in future versions
#define CONFIG_DATA_REV         1u     //these are created in case data structures change in future versions
#define USER_SETTINGS_DATA_REV  1u     //these are created in case data structures change in future versions
#define FUNC_SETTINGS_DATA_REV  1u     //these are created in case data structures change in future versions
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
    OS_ERR os_error;

    myStatus.value = 0u;

    //create mutex for resource locking
    char *name = "Persistent";
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &os_error);

    if (os_error != (OS_ERR)OS_ERR_NONE)
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
uint32_t DPersistent::getOffset(uint32_t location_offset, ePersistentMem_t partition, uint32_t no_of_bytes)
{
    uint32_t offset = location_offset;
    bool overflow = false;

    switch(partition)
    {
        case E_PERSIST_MAP:         //address starts at base of persistent storage space
            offset += DATA_REV_INFO_START;

            if ((offset + no_of_bytes) > PERSIST_DATA_CONFIG_START)
            {
                overflow = true;
            }
            break;

        case E_PERSIST_CONFIG:     //address follows on from housekeeping data partition
            offset += PERSIST_DATA_CONFIG_START;

            if ((offset + no_of_bytes) > PERSIST_DATA_SETTINGS_START)
            {
                overflow = true;
            }
            break;

        case E_PERSIST_SETTINGS:          //address follows on from configuration partition
            offset += PERSIST_DATA_SETTINGS_START;

            if ((offset + no_of_bytes) > PERSIST_DATA_FUNCTION_START)
            {
                overflow = true;
            }
            break;

        case E_PERSIST_FUNCTION_SETTINGS: //address follows on from settings partition
            offset += PERSIST_DATA_FUNCTION_START;

            if ((offset + no_of_bytes) > PERSIST_DATA_CAL_START)
            {
                overflow = true;
            }
            break;

        case E_PERSIST_CAL_DATA:          //address follows on from settings partition
            offset += PERSIST_DATA_CAL_START;

            if ((offset + no_of_bytes) > PERSIST_DATA_LIMIT)
            {
                overflow = true;
            }
            break;

        default:
            /*should not ever get this*/
            break;
    }

    if (overflow == true)
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
bool DPersistent::read(void *dest_addr, uint32_t location_offset, uint32_t no_of_bytes, ePersistentMem_t partition)
{
    bool bSuccess = true;

    location_offset = getOffset(location_offset, partition, no_of_bytes);

    if (location_offset != UINT32_MAX)
    {
        memset(dest_addr, 0, no_of_bytes);
        DLock is_on(&myMutex);
        bSuccess = eepromRead((uint8_t *)dest_addr, (uint16_t)location_offset, (uint16_t)no_of_bytes);
    }
    else
    {
        bSuccess = false;
    }

    return bSuccess;
}

/**
 * @brief   Write data to persistent storage
 * @param   src_addr address of memory location of data to be written to persistent storage
 * @param   location_offset offset from start of partition
 * @param   no_of_bytes number of bytes to write
 * @param   partition block (i.e. config, settings, error log or , event log)
 * @return  success or failure
 */
bool DPersistent::write(void *src_addr, uint32_t location_offset, uint32_t no_of_bytes, ePersistentMem_t partition)
{
    bool bSuccess = true;

    location_offset = getOffset(location_offset, partition, no_of_bytes);

    if (location_offset != UINT32_MAX)
    {
        DLock is_on(&myMutex);
        bSuccess = eepromWrite((uint8_t *)src_addr, (uint16_t)location_offset, no_of_bytes);
    }
    else
    {
        bSuccess = false;
    }

    return bSuccess;
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
    if (readMapRevision() == 0xFFFFu)
    {
        //TODO: Take whatever actions that may be required if revision is not right.
        //In initial release, it can be just set to the current (first) value
        setMapRevision();
    }

    //read persistent data
    readConfiguration();
    readUserSettings();
    readFunctionSettings();
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

#ifdef DEBUG
    memset(&mapInfo, 0xFF, sizeof(sPersistentMap_t));
#endif

    //read instrument housekeeping area
    if (read((void *)&mapInfo, 0u, sizeof(sPersistentMap_t), E_PERSIST_MAP) == false)
    {
        myStatus.readError = 1u;
    }

    //do validation whether or not the read (above) was successful; validation will catch bad data anyway
    uint32_t crc = crc32((uint8_t *)&mapInfo.data[0], (size_t)DATA_REV_INFO_ELEMENTS);

    if (crc != mapInfo.crc)
    {
        myStatus.mapCrcError = 1u;
    }
    else
    {
        //crc is good so can use the data
        revision = mapInfo.data[0];

        if (revision > MEMORY_MAP_REV)
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
    memset(&mapInfo, 0x00, sizeof(sPersistentMap_t));
    mapInfo.data[0] = MEMORY_MAP_REV;

    //calculate new CRC
    mapInfo.crc = crc32((uint8_t *)&mapInfo.data[0], (size_t)DATA_REV_INFO_ELEMENTS);

    //update the data in persistent storage
    if (write((void *)&mapInfo, 0u, sizeof(sPersistentMap_t), E_PERSIST_MAP) == false)
    {
        myStatus.writeError = 1u;
    }
}

/**
 * @brief   Read configuration data from persistent storage data
 * @param   void
 * @return  void
 */
void DPersistent::readConfiguration(void)
{
#ifdef DEBUG
    memset(&configuration.data, 0xFF, sizeof(sConfig_t));
#endif

    //read instrument configuration
    if (read((void *)&configuration, 0u, sizeof(sPersistentConfig_t), E_PERSIST_CONFIG) == false)
    {
        myStatus.readError = 1u;
    }

    //do validation whether or not the read (above) was successful; validation will catch bad data anyway
    if (validateConfigData() == false)
    {
        myStatus.configCrcError = 1u;

        //set defaults
        setDefaultConfigData();

        //calculate new CRC
        configuration.crc = crc32((uint8_t *)&configuration.data, sizeof(sConfig_t));

        //update the data in persistent storage

        if (write((void *)&configuration, 0u, sizeof(sPersistentConfig_t), E_PERSIST_CONFIG) == false)
        {
            myStatus.writeError = 1u;
        }

        //TODO: if region has been reset then update dependent settings too
    }
}

/**
 * @brief   Perform CRC validation on configuration data in persistent storage
 * @param   void
 * @return  flag: true if crc check passes, else false
 */
bool DPersistent::validateConfigData(void)
{
    bool flag = false;

    uint32_t crc = crc32((uint8_t *)&configuration.data, sizeof(sConfig_t));

    if (crc == configuration.crc)
    {
        flag = true;
    }

    return flag;
}

/**
 * @brief   Set default instrument calibration
 * @param   void
 * @return  void
 */
void DPersistent::setDefaultConfigData(void)
{
    //set config to defaults
    memset(&configuration.data, 0x00, sizeof(sConfig_t));

    sConfig_t *config = &configuration.data;

    config->revision = CONFIG_DATA_REV;                         //set revision to latest one
    snprintf(&config->serialNumber[0], 8u, "%s", "NOT SET");    //null terminated text string
    config->region = E_REGION_NOT_SET;
    config->instrumentType = E_INSTRUMENT_TYPE_STD;
}

/**
 * @brief   Save config data to persistent storage
 * @param   srcAddr: RAM address of data to be stored
 * @param   numBytes: number of bytes to be written
 * @return  flag: true is ok, else false
 */
bool DPersistent::saveConfigData(void *srcAddr, size_t numBytes)
{
    //calculate offset in persistent storage
    uint32_t startAddr = (uint32_t)&configuration;      //start (base address) of persistent storage structure
    uint32_t locationAddr = (uint32_t)srcAddr;			//location we are writing to

    //offset is the difference between them - assuming that startAddr <= locationAddr
    uint32_t offset = locationAddr - startAddr;

    bool flag = write(srcAddr, offset, numBytes, E_PERSIST_CONFIG);

    if (flag == true)
    {
        //if write was ok then need to calculate and save new CRC
        configuration.crc = crc32((uint8_t *)&configuration.data, sizeof(sConfig_t));

        locationAddr = (uint32_t)&configuration.crc;

        flag = write((void *)locationAddr, (locationAddr - startAddr), sizeof(uint32_t), E_PERSIST_CONFIG);
    }

    return flag;
}

/**
 * @brief   Read user settings data from persistent storage data
 * @param   void
 * @return  void
 */
void DPersistent::readUserSettings(void)
{
#ifdef DEBUG
    memset(&userSettings.data, 0xFF, sizeof(sUserSettings_t));
#endif

    //read instrument user settings
    if (read((void *)&userSettings, (uint32_t)0u, sizeof(sPersistentSettings_t), E_PERSIST_SETTINGS) == false)
    {
        myStatus.readError = 1u;
    }

    //do validation whether or not the read (above) was successful; validation will catch bad data anyway
    if (validateUserSettings() == false)
    {
        myStatus.userSettingsCrcError = 1u;

        //set defaults
        setDefaultUserSettings();

        //calculate new CRC
        userSettings.crc = crc32((uint8_t *)&userSettings.data, sizeof(sUserSettings_t));

        //update the data in persistent storage
        if (write((void *)&userSettings, 0u, sizeof(sPersistentSettings_t), E_PERSIST_SETTINGS) == false)
        {
            myStatus.writeError = 1u;
        }
    }
}

/**
 * @brief   Perform CRC validation on user settings data in persistent storage
 * @param   void
 * @return  flag: true if crc check passes, else false
 */
bool DPersistent::validateUserSettings(void)
{
    bool flag = false;

    uint32_t crc = crc32((uint8_t *)&userSettings.data, sizeof(sUserSettings_t));

    if (crc == userSettings.crc)
    {
        flag = true;
    }

    return flag;
}

/**
 * @brief   Set default user settings
 * @param   void
 * @return  void
 */
void DPersistent::setDefaultUserSettings(void)
{
  /*
    //set user settings to defaults
    memset(&userSettings.data, 0x00, sizeof(sUserSettings_t));

    sUserSettings_t *settings = &userSettings.data;

    settings->revision = USER_SETTINGS_DATA_REV;                         //revision set to latest one
    settings->calPin = 4321u;						                    //default user calibration PIN = 4321
    settings->autoPowerdown = 10u;				                        //default auto-powerdown time set to 10 minutes
  
    settings->language = E_LANGUAGE_NOT_SET;                            //default is no language set, offer user choice

    sChannelSetting_t *defChannel =  &settings->channel[E_CHANNEL_1];   //default CH1 is mA measure
    defChannel->function = E_FUNCTION_MA;
    defChannel->direction = E_FUNCTION_DIR_MEASURE;

    defChannel = &settings->channel[E_CHANNEL_2];                       //default CH2 is no output
    defChannel->function = E_FUNCTION_NONE;
    defChannel->direction = E_FUNCTION_DIR_MEASURE;

    defChannel = &settings->channel[E_CHANNEL_3];                       //default CH3 is internal pressure measure
    defChannel->function = E_FUNCTION_INT_PRESSURE;
    defChannel->direction = E_FUNCTION_DIR_MEASURE;
*/
}

/**
 * @brief   Save user settings to persistent storage
 * @param   srcAddr: RAM address of data to be stored
 * @param   numBytes: number of bytes to be written
 * @return  flag: true is ok, else false
 */
bool DPersistent::saveUserSettings(void *srcAddr, size_t numBytes)
{
    //calculate offset in persistent storage
    uint32_t startAddr = (uint32_t)&userSettings;      //start (base address) of persistent storage structure
    uint32_t locationAddr = (uint32_t)srcAddr;			//location we are writing to

    //offset is the difference between them - assuming that startAddr <= locationAddr
    uint32_t offset = locationAddr - startAddr;

    bool flag = write(srcAddr, offset, numBytes, E_PERSIST_SETTINGS);

    if (flag == true)
    {
        //if write was ok then need to calculate and save new CRC
        userSettings.crc = crc32((uint8_t *)&userSettings.data, sizeof(sUserSettings_t));

        locationAddr = (uint32_t)&userSettings.crc;

        flag = write((void *)locationAddr, (locationAddr - startAddr), sizeof(uint32_t), E_PERSIST_SETTINGS);
    }

    return flag;
}

/**
 * @brief   Read function settings data from persistent storage data
 * @param   void
 * @return  void
 */
void DPersistent::readFunctionSettings(void)
{
#ifdef DEBUG
    memset(&functionSettings.data, 0xFF, sizeof(sFunctionsData_t));
#endif

    //read instrument function settings
    if (read((void *)&functionSettings, (uint32_t)0u, sizeof(sPersistentFunctions_t), E_PERSIST_FUNCTION_SETTINGS) == false)
    {
        myStatus.readError = 1u;
    }

    //do validation whether or not the read (above) was successful; validation will catch bad data anyway
    if (validateFunctionSettings() == false)
    {
        myStatus.FuncSettingsCrcError = 1u;

        //set defaults
        setDefaultFunctionSettings();

        //calculate new CRC
        functionSettings.crc = crc32((uint8_t *)&functionSettings.data, sizeof(sFunctionsData_t));

        //update the data in persistent storage
        if (write((void *)&functionSettings, 0u, sizeof(sPersistentFunctions_t), E_PERSIST_FUNCTION_SETTINGS) == false)
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
bool DPersistent::validateFunctionSettings(void)
{
    bool flag = false;

    uint32_t crc = crc32((uint8_t *)&functionSettings.data, sizeof(sFunctionsData_t));

    if (crc == functionSettings.crc)
    {
        flag = true;
    }

    return flag;
}

/**
 * @brief   Set default function settings
 * @param   void
 * @return  void
 */
void DPersistent::setDefaultFunctionSettings(void)
{
    //set function settings to defaults
    memset(&functionSettings.data, 0x00, sizeof(sFunctionsData_t));

    sFunctionsData_t *settings = &functionSettings.data;

    settings->revision = FUNC_SETTINGS_DATA_REV;            //revision set to latest one
/*
    //measure mA function settings
    sFunctionSetting_t *func = &settings->measureMA;
    func->function = E_FUNCTION_MA;                         //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_MA;                               //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure mV function func
    func = &settings->measureMV;
    func->function = E_FUNCTION_MV;                         //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_MV;                               //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure Volts function func
    func = &settings->measureVolts;
    func->function = E_FUNCTION_VOLTS;                      //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_VOLTS;                            //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure internal pressure function func
    func = &settings->measureIntP;
    func->function = E_FUNCTION_INT_PRESSURE;               //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure external pressure function func
    func = &settings->measureExtP;
    func->function = E_FUNCTION_EXT_PRESSURE;               //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure barometer function func
    func = &settings->measureBarometer;
    func->function = E_FUNCTION_BAROMETER;                  //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure P1 - P2 function func
    func = &settings->measureDiffIntExtP;
    func->function = E_FUNCTION_DIFF_EXT_INT_P;             //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure internal pressure - barometer (pseudo guage) function func
    func = &settings->measureDiffIntBaro;
    func->function = E_FUNCTION_DIFF_INT_BARO;              //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure external pressure - barometer (pseudo guage) function func
    func = &settings->measureDiffExtBaro;
    func->function = E_FUNCTION_DIFF_EXT_BARO;              //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure P1 + P2 function func
    func = &settings->measureSumIntExtP;
    func->function = E_FUNCTION_ADD_EXT_INT_P;              //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure internal pressure + barometer (pseudo absolute) function func
    func = &settings->measureSumIntBaro;
    func->function = E_FUNCTION_ADD_INT_BARO;               //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure external pressure + barometer (pseudo absolute) function func
    func = &settings->measureSumExtBaro;
    func->function = E_FUNCTION_ADD_EXT_BARO;               //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting

    //measure RTD function func
    func = &settings->measureRTD;
    func->function = E_FUNCTION_RTD;                        //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_CENTIGRADE;                       //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting
    func->mode = (int32_t)E_RTD_SENSOR_MODE_TEMP;           //for RTD sensor, mode means temperature or resistance

    //measure HART function func
    func = &settings->measureHART;
    func->function = E_FUNCTION_HART;                       //my function identification
    func->direction = E_FUNCTION_DIR_MEASURE;               //my measure/source direction
    func->units = E_UNITS_BAR;                              //units selected
    func->filter.state = E_INIT_STATE_NOT_SET;              //filter setting
    func->alarm.state = E_INIT_STATE_NOT_SET;               //alarm setting
    func->scaling.state = E_INIT_STATE_NOT_SET;             //scaling setting
    func->mode = (int32_t)true;                             //for HART, mode means HART resistor mode (true = on by default)

//    //source mA function func
//    sSourceFunctionSetting_t *srcFunc = &settings->sourceMA;
//    srcFunc->function = E_FUNCTION_MA;                      //my function identification
//    srcFunc->direction = E_FUNCTION_DIR_SOURCE;             //my measure/source direction
//    srcFunc->units = E_UNITS_MA;                            //units selected
//
//    //source CH2 volts function func
//    srcFunc = &settings->sourceCh2Volts;
//    srcFunc->function = E_FUNCTION_VOLTS;                   //my function identification
//    srcFunc->direction = E_FUNCTION_DIR_SOURCE;             //my measure/source direction
//    srcFunc->units = E_UNITS_VOLTS;                         //units selected
*/
}

/**
 * @brief   Save function settings to persistent storage
 * @param   srcAddr: RAM address of data to be stored
 * @param   numBytes: number of bytes to be written
 * @return  flag: true is ok, else false
 */
bool DPersistent::saveFunctionSettings(void *srcAddr, size_t numBytes)
{
    //calculate offset in persistent storage
    uint32_t startAddr = (uint32_t)&functionSettings;   //start (base address) of persistent storage structure
    uint32_t locationAddr = (uint32_t)srcAddr;			//location we are writing to

    //offset is the difference between them - assuming that startAddr <= locationAddr
    uint32_t offset = locationAddr - startAddr;

    bool flag = write(srcAddr, offset, numBytes, E_PERSIST_FUNCTION_SETTINGS);

    if (flag == true)
    {
        //if write was ok then need to calculate and save new CRC
        functionSettings.crc = crc32((uint8_t *)&functionSettings.data, sizeof(sFunctionsData_t));

        locationAddr = (uint32_t)&functionSettings.crc;

        flag = write((void *)locationAddr, (locationAddr - startAddr), sizeof(uint32_t), E_PERSIST_FUNCTION_SETTINGS);
    }

    return flag;
}

/**
 * @brief   Read calibration data from persistent storage data
 * @param   void
 * @return  void
 */
void DPersistent::readCalibrationData(void)
{
#ifdef DEBUG
    memset(&calibrationData.data, 0xFF, sizeof(sCalData_t));
#endif

    //read instrument user settings
    if (read((void *)&calibrationData, (uint32_t)0u, sizeof(sPersistentCal_t), E_PERSIST_CAL_DATA) == false)
    {
        myStatus.readError = 1u;
    }

    //do validation whether or not the read (above) was successful; validation will catch bad data anyway
    if (validateCalibrationData() == false)
    {
        myStatus.calDataCrcError = 1u;

        //NOTE: if cal data block is not valid it is just cleared to zero. It is left for the sensor, at the point of use,
        //to validate its cal data and signal issues if not usable. If it is required to go through all ca data on startup
        //and notify that one or more sensor ranges are uncalibrated then it could be done here.
        memset(&calibrationData, 0x00, sizeof(sPersistentCal_t));
    }
}

/**
 * @brief   Perform CRC validation on calibration data in persistent storage
 * @param   void
 * @return  flag: true if crc check passes, else false
 */
bool DPersistent::validateCalibrationData(void)
{
    bool flag = false;

    uint32_t crc = crc32((uint8_t *)&calibrationData.data, sizeof(sCalData_t));

    if (crc == calibrationData.crc)
    {
        flag = true;
    }

    return flag;
}

/**
 * @brief   Save calibration data to persistent storage
 * @param   srcAddr: RAM address of data to be stored
 * @param   numBytes: number of bytes to be written
 * @return  flag: true is ok, else false
 */
bool DPersistent::saveCalibrationData(void *srcAddr, size_t numBytes)
{
    //calculate offset in persistent storage
    uint32_t startAddr = (uint32_t)&calibrationData;    //start (base address) of persistent storage structure
    uint32_t locationAddr = (uint32_t)srcAddr;			//location we are writing to

    //offset is the difference between them - assuming that startAddr <= locationAddr
    uint32_t offset = locationAddr - startAddr;

    bool flag = write(srcAddr, offset, numBytes, E_PERSIST_CAL_DATA);

    if (flag == true)
    {
        //if write was ok then need to calculate and save new CRC
        calibrationData.crc = crc32((uint8_t *)&calibrationData.data, sizeof(sCalData_t));

        locationAddr = (uint32_t)&calibrationData.crc;
        flag = write((void *)locationAddr, (locationAddr - startAddr), sizeof(uint32_t), E_PERSIST_CAL_DATA);
    }

    return flag;
}

/**
 * @brief   Get function setting on specified channel
 * @param   channel
 * @param   pointer to settings for return value
 * @return  void
 */
void DPersistent::getChannelFunction(eChannel_t channel, sChannelSetting_t *setting)
{
    DLock is_on(&myMutex);
    setting->function = userSettings.data.channel[(int)channel].function;
    setting->direction = userSettings.data.channel[(int)channel].direction;
}

/**
 * @brief   Set function setting on specified channel
 * @param   channel
 * @param   function enumeration
 * @param   direction (measure or source)
 * @return  true if set and saved succesfully, else false
 */
bool DPersistent::setChannelFunction(eChannel_t channel, eFunction_t function, eFunctionDir_t direction)
{
    DLock is_on(&myMutex);
    sChannelSetting_t *channelSetting = &userSettings.data.channel[(int)channel];
    channelSetting->function = function;
    channelSetting->direction = direction;

    //save to persistent storage
    return saveUserSettings((void *)channelSetting, sizeof(sChannelSetting_t));
}

/**
 * @brief   Get address from function settings partition
 * @return  void
 */
sPersistentFunctions_t *DPersistent::getFunctionSettingsAddr(void)
{
    return &functionSettings;
}

void DPersistent::getPersistentData(void * src, void *dest, size_t size)
{
    DLock is_on(&myMutex);
    memcpy(dest, src, size);
}

/**
 * @brief   Get address from cal data block
 * @return  void
 */
sCalData_t *DPersistent::getCalDataAddr(void)
{
    return &calibrationData.data;
}

//
///**
// * @brief   Set settings for specified function
// * @param   channel
// * @param   function enumeration
// * @param   direction (measure or source)
// * @return  true if set and saved succesfully, else false
// */
//bool DPersistent::setFunctionSettings(eChannel_t channel, eFunction_t function, eFunctionDir_t direction)
//{
//    DLock is_on(&myMutex);
//    sChannelSetting_t *channelSetting = &userSettings.data.channel[(int)channel];
//    channelSetting->function = function;
//    channelSetting->direction = direction;
//
//    //save to persistent storage
//    return saveUserSettings((void *)channelSetting, sizeof(sChannelSetting_t));
//}
//
