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
#define EV_FLAG_USB_MSC_ACCESS 0x00000001u
#define EV_FLAG_FW_VALIDATE    0x00000002u
#define EV_FLAG_FW_UPGRADE     0x00000004u
#define FILE_MAX_LINE_LENGTH   1520
#define FILENAME_MAX_LENGTH    60u

#define EXTSTORAGE_DIRECTORY_INDEX_LOCALDOC     3

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
    bool upgradeFirmware(OS_FLAGS flags);

    bool open(char *filePath, bool writable);
    bool close();
    bool read(char *buf, uint32_t length);
    bool write(char *buf);
    bool query(uint32_t *size, uint32_t *numLines);
    bool exists(char *filePath);
    bool erase(char *filePath);
    bool dir(char *path, fileInfo_t *fileInfo);
    bool mkdir(char *path);
    bool isDirectoryExist(const char *path);
    void getDirectoryPath(uint16_t index, char *path, uint16_t len);
    bool deleteDirectory(char *path);

    bool readLine(char *buf, uint32_t lineLength);
    bool writeLine(char *buf);
    bool createDirectories(void);
private:
    OS_ERR postEvent(uint32_t event, uint32_t param8, uint32_t param16);
    void handleEvents(OS_FLAGS actualEvents);

    uint32_t numberOfFrames;
    uint32_t reset;
    uint32_t numberOfBlocks;
    uint32_t numberOfFramesLeft;
    const uint8_t dummy = 42u;
    uint32_t bootLoaderError;

#ifdef USE_UCFS
    FS_FILE *f;
#endif

#ifdef USE_FATFS
    FATFS fs;      // Filesystem object
    FIL f;         // File object
    DIR d;
    char path[FILENAME_MAX_LENGTH + 1u];
#endif

    bool readyForUpgrade;
    bool verifyingUpgrade;
    char terminator[2];

#ifdef PRODUCTION_TEST_BUILD
    virtual void postShutdownNotification(void);
#endif
};

#endif /* __DEXTSTORAGE_H */
