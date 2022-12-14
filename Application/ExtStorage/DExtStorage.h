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
* @file     DExtStorage.h
* @version  1.00.00
* @author   Simon Smith
* @date     January 2021
*
* @brief    The external storage class header file
*/

#ifndef __DEXTSTORAGE_H
#define __DEXTSTORAGE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DTask.h"
#include "misra.h"
#include "Types.h"
MISRAC_DISABLE
#ifdef USE_UCFS
extern "C"
{
#include "fs_app.h"
#include "fs_entry.h"
#include "fs_err.h"
#include "fs_file.h"
#include "fs_vol.h"
}
#endif
#ifdef USE_FATFS
#include "ff.h"
#endif
MISRAC_ENABLE

/* Defines  ---------------------------------------------------------------------------------------------------------*/
#define EV_FLAG_USB_MSC_ACCESS             0x00000001u
#define FILE_MAX_LINE_LENGTH               1520
#define FILENAME_MAX_LENGTH                60u

#define EXTSTORAGE_DIRECTORY_INDEX_LOCALDOC     3

#define ACK_FW_UPGRADE                  0x3C            // This macro is used for SPI response of Secondary to main uC 
#define NACK_FW_UPGRADE                 0x00            // This macro is used for SPI response of Secondary to main uC 

#define NUM_FRAMES_PER_BLOCK                    15u
#define BYTES_PER_FRAME                         528u                         // BYTES_PER_FRAME for fw upgrade of main uC
#define SECONDARY_UC_BYTES_PER_FRAME            132u                         // SPI fw data size 132
#define BLE652_APP_BYTES_PER_FRAME              50u                         // BLE UART Buffer size 50
#define BLE652_APP_EXT_ASCII_BYTES_PER_FRAME    (BLE652_APP_BYTES_PER_FRAME / 2u)        // BLE EXT ASCII Buffer size 50/2 = 25u
#define BLOCK_BUFFER_SIZE                       (NUM_FRAMES_PER_BLOCK * BYTES_PER_FRAME)

#define CRC8_POLYNOMIAL                         0x07u
// Following macros are used for validating fw data using received header of 40 bytes
#define HEADER_SIZE                             40u     // Used in validate file function
#define ONE_BYTE                                1u      // Used in validate file function
#define FILENAME_SIZE                           6u      // Used in validate file function
#define FILESIZE_BUFFER                         10u     // Used in validate file function
#define FILE_CRC_BUFFER                         10u     // Used in validate file function
#define IMAGE_CRC_BUFFER_SIZE                   10u     // Used in validate file function
#define HEADER_CRC_BUFFER                       3u      // Used in validate file function
#define RECEIVED_DATA_BLOCK_SIZE                512u    // Used to calculate crc32 
#define FILENAME_VERSION_SIZE                   9u      // Used in validate file function
#define FW_VERSION_SIZE                         9u      // Used in validate file function
#define FILENAME_START_POSITION                 0u      // Used in validate file function
#define RECORD_NUMBER                           2u      // Used in Fw Upgrade of secondary Fw
#define MAJOR_VERSION_NUMBER_START_POSITION     (FILENAME_SIZE + 3u)    //During file validation, Used for moving cursor of 40 bytes header array to required location
#define MINOR_VERSION_NUMBER_START_POSITION     (FILENAME_SIZE + 6u)    //During file validation, Used for moving cursor of 40 bytes header array to required location
#define SUB_VERSION_NUMBER_START_POSITION       (FILENAME_SIZE + 9u)    //During file validation, Used for moving cursor of 40 bytes header array to required location
#define IMAGE_CRC_START_POSITION                (FILENAME_VERSION_SIZE + FILENAME_SIZE + FILESIZE_BUFFER + 2u)  // Used in for loop for validating image CRC
#define IMAGE_CRC_END_POSITION                  (IMAGE_CRC_START_POSITION + FILE_CRC_BUFFER)    // Used in for loop for validating image size
#define IMAGE_SIZE_START_POSITION               (FILENAME_VERSION_SIZE + FILENAME_SIZE + 2u)  // Used in for loop for validating image size
#define IMAGE_SIZE_END_POSITION                  (IMAGE_SIZE_START_POSITION + FILESIZE_BUFFER)
#define MAX_VERSION_NUMBER_LIMIT                99u

#define MAX_ALLOWED_MAIN_APP_FW                 1081872u
#define MAX_ALLOWED_SECONDARY_APP_FW            101904u
#define MAX_ALLOWED_BLE_SMART_BASIC_APP_FW      30720u

#define LED_5_SECONDS                           5000u    // 5000 ms -> 5 sec
#define LED_30_SECONDS                          30000u   // 30000 ms -> 30 sec

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef struct
{
    bool readOnly;
    bool hidden;
    bool system;
    bool archive;
    bool directory;
} attribInfo_t;

typedef struct
{
    uint32_t size; // File size
    sDate_t date; // Modified date
    sTime_t time; // Modified time
    attribInfo_t attribInfo; // file attributes
    char filename[FILENAME_MAX_LENGTH + 1u]; // Primary file name
} fileInfo_t;

typedef enum
{
    E_UPGRADE_IDLE,
    E_UPGRADE_PREPARING,                // Unused
    E_UPGRADE_ERROR_DEVICE_BUSY,        // Used after queryPowerDownAllowed check if any of task/process is running
    E_UPGRADE_ERROR_BATTERY_TOO_LOW,    // Used if battery is low
    E_UPGRADE_ERROR_BATTERY_NOT_PRESENT,  //
    E_UPGRADE_ERROR_INVALID_OPTION_BYTES,
    E_UPGRADE_ERROR_FILE_NOT_FOUND,              // If DK0514.raw is not available, generate this error
    E_UPGRADE_ERROR_PERSISTENT_STORAGE_WRITE_FAIL,      // TODO

    E_UPGRADE_VALIDATING_MAIN_APP,
    E_UPGRADE_VALIDATED_MAIN_APP,
    E_UPGRADE_UPGRADING_MAIN_APP,
    E_UPGRADE_ERROR_INVALID_MAIN_BOOTLOADER, // Get bootloader version of main uC and add it in start of Validation Function
    E_UPGRADE_ERROR_MAIN_APP_FILE_SIZE_INVALID,
    E_UPGRADE_ERROR_MAIN_APP_API_FAIL,
    E_UPGRADE_ERROR_MAIN_APP_ERASE_FAIL,
    E_UPGRADE_ERROR_MAIN_APP_IMAGE_READ_FAIL,
    E_UPGRADE_ERROR_MAIN_APP_DATA_WRITE_FAIL,
    E_UPGRADE_ERROR_MAIN_APP_VERSION_INVALID,
    E_UPGRADE_ERROR_MAIN_FILE_HEADER_INVALID,
    E_UPGRADE_ERROR_MAIN_FILE_HEADER_CRC_INVALID,
    E_UPGRADE_ERROR_MAIN_APP_IMAGE_CRC_INVALID,

    E_UPGRADE_VALIDATING_SEC_APP,
    E_UPGRADE_VALIDATED_SEC_APP,
    E_UPGRADE_UPGRADING_SEC_APP,
    E_UPGRADE_ERROR_INVALID_SEC_BOOTLOADER, // Get bootloader version of sec uC and add it in start of Validation Function
    E_UPGRADE_ERROR_SEC_APP_FILE_SIZE_INVALID,
    E_UPGRADE_ERROR_SEC_APP_CMD_FAIL,   // Check for secondaryUcFwUpgradeCmd response
    E_UPGRADE_ERROR_SEC_APP_IMAGE_READ_FAIL,
    E_UPGRADE_ERROR_SEC_APP_DATA_WRITE_FAIL,    // Check when sending data with record number
    E_UPGRADE_ERROR_SEC_APP_VERSION_INVALID,
    E_UPGRADE_ERROR_SEC_FILE_HEADER_INVALID,
    E_UPGRADE_ERROR_SEC_FILE_HEADER_CRC_INVALID,
    E_UPGRADE_ERROR_SEC_APP_IMAGE_CRC_INVALID,

    E_UPGRADE_VALIDATING_BLE652_SMART_BASIC_APP,        // unused
    E_UPGRADE_VALIDATED_BLE652_SMART_BASIC_APP,
    E_UPGRADE_UPGRADING_BLE652_SMART_BASIC_APP,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_FILE_SIZE_INVALID,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_IMAGE_READ_FAIL,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_WRITE_FAIL,    // Check when sending ble data
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_BT_FS_DELETE_FAILED,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_BT_FILE_CREATION_FAILED,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_FILE_HEADER_INVALID,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_FILE_HEADER_CRC_INVALID,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_IMAGE_CRC_INVALID,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_FILE_CLOSE_FAILED,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_CMD_DIR_FAILED,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_CHECKSUM_FAILED,
    E_UPGRADE_ERROR_BLE652_SMART_BASIC_APP_CMD_ATI_C1C2_CHECKSUM_FAILED,

} eUpgradeStatus_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DExtStorage : public DTask
{
public:
    DExtStorage(OS_ERR *os_error);
    ~DExtStorage();

    virtual void initialise(void);
    virtual void runFunction(void);

    bool configure(void);
    bool getStatus(uint32_t *bytesUsed, uint32_t *bytesTotal);

    bool validateUpgrade(void);
    void upgradeApplicationFirmware(void);

    bool openFile(char *filePath, bool writable);
    bool close();
    bool read(char *buf, uint32_t length);
    bool write(char *buf, uint32_t bufSize);
    bool write(char *buf, uint32_t bufSize, uint32_t length);
    bool query(uint32_t *size, uint32_t *numLines);
    bool exists(char *filePath);
    bool erase(char *filePath);
    bool dir(char *path, fileInfo_t *fileInfo);
    bool mkdir(char *path);
    bool isDirectoryExist(const char *path);
    void getDirectoryPath(uint16_t index, char *path, uint16_t len);
    bool deleteDirectory(char *path);
    bool scanForUnexpectedFiles(char *filePath, uint32_t level, bool deleteFiles);
    bool readLine(char *buf, uint32_t bufSize, uint32_t lineLength);
    bool writeLine(char *buf, uint32_t bufSize);
    bool createDirectories(void);

    bool validateMainFwFile(void);
    bool validateSecondaryFwFile(void);
    bool validateBleSmartBasicAppFwFile(void);
    bool updateMainUcFirmware(void);
    bool updateSecondaryUcFirmware(void);
    bool updateBle652SmartBasicAppFirmware(void);
    bool validateHeaderCrc(uint8_t *HeaderData);
    bool validateImageCrc(uint8_t *HeaderData, uint32_t imageSize);
    bool validateImageSize(uint8_t *HeaderData, uint32_t *imageSize, uint32_t maxAllowedImageSize);
    bool validateVersionNumber(uint8_t *HeaderData, sVersion_t currentAppVersion);
    bool validateHeaderInfo(uint8_t *HeaderData, sVersion_t receivedAppVersion, const uint8_t *currentDkNumber);

    eUpgradeStatus_t getUpgradeStatus(void);    // used for UF1 Command
    bool validateBootloaderVersionNumber(uint8_t *HeaderData, uint32_t minVersionBL);
    bool validateAndUpgradeFw(void);
    uint32_t ByteChecksum(uint32_t usCrcVal, uint8_t ucChar);

private:
    OS_ERR postEvent(uint32_t event, uint32_t param8, uint32_t param16);
    void handleEvents(OS_FLAGS actualEvents);

    OS_FLAGS myWaitFlagsStorage;                   //events (flags) to which the function will respond
    OS_MUTEX myMutex;                           //mutex for resource locking


    uint32_t reset;
    uint32_t numberOfBlocks;            // Used in validate file and Upgrade fw function
    uint32_t numberOfFramesLeft;        // Used in validate file and Upgrade fw function
    uint32_t numberOfBytesLeft;         // Used in BLE652 validate and Upgrade fw function

    uint32_t bootLoaderError;           // Upgrade fw function
    const uint8_t dummy = 42u;
    uint32_t secondaryFwFileSizeInt;           // Used store secondary uC fw size to do fw upgrade
    uint32_t bleSmartBasicAppFwFileSizeInt;    // Used to store ble652 smart basic app fw size to do fw upgrade
    bool mainUcFwUpgradeRequired;       // To check main fw upgarde is required or not false-> not required, true-> required
    bool secondaryUcFwUpgradeRequired;       // To check secondary fw upgarde is required or not false-> not required, true-> required
    bool bleSmartBasicAppFwUpgradeRequired;       // To check ble Smart Basic App fw upgarde is required or not false-> not required, true-> required

#ifdef USE_UCFS
    FS_FILE *f;
#endif

#ifdef USE_FATFS
    FATFS fs;      // Filesystem object
    FIL f;         // File object
    DIR d;
    char path[FILENAME_MAX_LENGTH + 1u];
#endif

    bool verifyingUpgrade;
    char terminator[2];

#ifdef PRODUCTION_TEST_BUILD
    virtual void postShutdownNotification(void);
#endif
};

#endif /* __DEXTSTORAGE_H */
