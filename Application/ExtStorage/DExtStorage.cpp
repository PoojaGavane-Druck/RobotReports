/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DExtStorage.cpp
* @version  1.00.00
* @author   Simon Smith
* @date     January 2021
*
* @brief    The external storage source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#define __STDC_WANT_LIB_EXT1__ 1
#include "DExtStorage.h"
MISRAC_DISABLE
#include <assert.h>
#include <math.h>
#include "main.h"
#include "usbd_msc.h"
#include "app_cfg.h"
MISRAC_ENABLE

#include "DPV624.h"
#include "Utilities.h"
#include "ospi_nor_mx25l25645.h"
#include "crc.h"
#include <stdlib.h>
#include "stm32l4xx_hal.h"
#include <string.h>
/* Error handler instance parameter starts from 3101 to 3200 */

/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define EXTSTORAGE_HANDLER_TASK_STK_SIZE        8192u    //not bytes (CPU_STK is 4 bytes, so multiply by 4 for stack size in bytes)
#define EXTSTORAGE_TASK_TIMEOUT_MS              500u
#define FILE_PREAMBLE_LENGTH                    289u
#define TERMINATOR_CR                           '\r'
#define TERMINATOR_LF                           '\n'
#define LINE_TERMINATION                        "\r\n"

const char *directories[] = { "\\LogFiles", "\\Upgrades", NULL};
char const mainAppDkNumber[FILENAME_SIZE] = {'D', 'K', '0', '4', '9', '9'};
char const secondaryAppDkNumber[FILENAME_SIZE] = {'D', 'K', '0', '5', '0', '9'};

#if !defined USE_FATFS && !defined USE_UCFS
#warning Missing file system middleware :(
#endif

/* Variables --------------------------------------------------------------------------------------------------------*/
CPU_STK extStorageTaskStack[EXTSTORAGE_HANDLER_TASK_STK_SIZE];
extern IWDG_HandleTypeDef hiwdg;
extern CRC_HandleTypeDef hcrc;                  // Required for bootloaderAPI
extern const unsigned char cAppVersion[4];      // This contains the current main uC application fw version

USBD_HandleTypeDef *pdevUsbMsc = NULL;
uint8_t epnumUsbMsc;
OS_FLAG_GRP myEventFlagsStorage;               //event flags to pend on
OS_FLAGS myWaitFlagsStorage;                   //events (flags) to which the function will respond

eUpgradeStatus_t upgradeStatus;         // Used for getting Error code and Status of FW Upgrade

/**
* @brief    Constructor
* @param    os_error is pointer to OS error
* @return   void
*/
DExtStorage::DExtStorage(OS_ERR *os_error)
    : DTask()
{
    myName = "Ext Storage";

    myTaskId = eExternalStorageTask;
    //set up task stack pointer
    myTaskStack = &extStorageTaskStack[0];
    mainUcFwUpgradeRequired = false;       // To check main fw upgarde is required or not false-> not required, true-> required
    secondaryUcFwUpgradeRequired = false;  // To check secondary fw upgarde is required or not false-> not required, true-> required

    secondaryFwFileSizeInt = 0u;           // Used store secondary uC fw size to do fw upgrade


#ifdef ENABLE_STACK_MONITORING
    stackArray.uiStack.addr = (void *)myTaskStack;
    stackArray.uiStack.size = (uint32_t)(APP_CFG_EXT_STORAGE_TASK_STK_SIZE * 4u);
    fillStack((char *)myTaskStack, 0xAA, (size_t)(APP_CFG_EXT_STORAGE_TASK_STK_SIZE * 4u));

#endif
    memset_s((void *)&myEventFlagsStorage, sizeof(OS_FLAG_GRP), 0, sizeof(OS_FLAG_GRP));
    RTOSFlagCreate(&myEventFlagsStorage, myName, (OS_FLAGS)0, os_error);

#ifdef USE_UCFS
    // Init uc/FS - including Open device "nor:0:" and Open volume "nor:0:".
    Mem_Init();
    CPU_BOOLEAN status = App_FS_Init();
    MISRAC_DISABLE
    assert(status);
    MISRAC_ENABLE

    // Unmount volume until needed
    FS_ERR err = FS_ERR_NONE;
    FSVol_Close("nor:0:", &err);
    MISRAC_DISABLE
    assert(err == FS_ERR_NONE);
    MISRAC_ENABLE

    f = NULL;
#endif

#ifdef USE_FATFS
    d.obj.fs = NULL;
    f.obj.fs = NULL;
#endif

    //specify the flags that this function must respond to
    myWaitFlagsStorage = EV_FLAG_USB_MSC_ACCESS;

    activate(myName, (CPU_STK_SIZE)APP_CFG_EXT_STORAGE_TASK_STK_SIZE, (OS_PRIO)14u, (OS_MSG_QTY)1u, os_error);
}

/**
* @brief    Destructor
* @param    void
* @return   void
*/
DExtStorage::~DExtStorage()
{
}

/**
 * @brief   Task initialise method
 * @param   none
 * @retval  none
 */
void DExtStorage::initialise(void)
{

    // Prerequisites
    MISRAC_DISABLE
    assert(PV624 != NULL);
    MISRAC_ENABLE


    bool ok = (OSPI_NOR_Init() == (tOSPINORStatus)(OSPI_NOR_SUCCESS));

    if(!ok)
    {
        PV624->updateDeviceStatus(E_ERROR_CODE_EXTERNAL_STORAGE,
                                  eSetError);
    }

    verifyingUpgrade = false;
    upgradeStatus = E_UPGRADE_IDLE;

}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as we are using snprintf which violates the rule.
 **********************************************************************************************************************/
_Pragma("diag_suppress=Pm128")

/**
* @brief    Task run - the top level function that handles external storage.
* @param    void
* @return   void
*/
void DExtStorage::runFunction(void)
{
    OS_ERR os_error = OS_ERR_NONE;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;

    createDirectories();

    //task main loop
    while(DEF_TRUE)
    {
        PV624->keepAlive(eExternalStorageTask);

        //pend until timeout, blocking, on the events
        actualEvents = RTOSFlagPend(&myEventFlagsStorage,
                                    myWaitFlagsStorage, (OS_TICK)EXTSTORAGE_TASK_TIMEOUT_MS,
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
#ifdef ENABLE_STACK_MONITORING
        lastTaskRunning = myTaskId;
#endif
        //check flags to determine what to execute
#ifdef USE_UCFS

        if((actualEvents & EV_FLAG_USB_MSC_ACCESS) == EV_FLAG_USB_MSC_ACCESS)  // posted by USB OTG interrupt
        {
            if(pdevUsbMsc != NULL)
            {
                MSC_BOT_DataOut(pdevUsbMsc, epnumUsbMsc);
            }
        }

#endif

        // Below condition is added to resolve warning of actualEvents
        if((actualEvents & EV_FLAG_USB_MSC_ACCESS) == EV_FLAG_USB_MSC_ACCESS)  // posted by USB OTG interrupt
        {
            // No operation Requied
        }

        bool ok = ((os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT)));

        if(!ok)
        {
            PV624->handleError(E_ERROR_OS,
                               eSetError,
                               (uint32_t)os_error,
                               3102u);
        }
    }
}

/**
* @brief    Post event to task message queue
* @param    uint32_t event, uint32_t param8, uint32_t param16
* @return   OS_ERR error
*/
OS_ERR DExtStorage::postEvent(uint32_t event, uint32_t param8, uint32_t param16)
{
    OS_ERR os_error = OS_ERR_NONE;

    sTaskMessage_t message;
    message.value = 0u;
    message.event = event;

    message.param8 = param8;
    message.param16 = param16;

    //Post message to External Storage Task
    RTOSTaskQPost(&myTaskTCB, (void *)message.value, (OS_MSG_SIZE)0, (OS_OPT) OS_OPT_POST_FIFO, &os_error);

    if(os_error != static_cast<OS_ERR>(OS_ERR_NONE))
    {

        PV624->handleError(E_ERROR_OS,
                           eSetError,
                           (uint32_t)os_error,
                           3103u);
    }

    return os_error;
}

#ifdef PRODUCTION_TEST_BUILD
/**
 *  @brief Post shutdown notification to task
 *  @param  void
 *  @return void
 */
void DExtStorage::postShutdownNotification(void)
{
    //ignore shutdown request - because always needed, to allow upgrade
}
#endif

/**
 * @brief   Configure External Flash Memory
 * @param   void
 * @return   bool - true if ok, false if not ok
 */
bool DExtStorage::configure(void)
{
    bool ok = true;

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;

    // Mount volume
    if(ok)
    {
        FSVol_Open("nor:0:", "nor:0:", 0u, &err);
        ok &= ((err == FS_ERR_NONE) || (err == FS_ERR_VOL_ALREADY_OPEN));
    }

    // Create FAT volume with default cluster size
    if(ok)
    {
        FSVol_Fmt("nor:0:", (void *)0, &err);
        ok &= (err == FS_ERR_NONE);
    }

    // Set drive label
    if(ok)
    {
        char name[11];
        PV624->getInstrumentName(name);
        FSVol_LabelSet("nor:0:", name, &err);
        ok &= (err == FS_ERR_NONE);
    }

    // Create directories
    if(ok)
    {
        for(int i = 0; directories[i] != NULL; i++)
        {
            FSEntry_Create(directories[i], FS_ENTRY_TYPE_DIR, DEF_YES, &err);
            ok &= ((err == FS_ERR_NONE) || (err == FS_ERR_ENTRY_EXISTS));
        }
    }

    // Unmount volume
    FSVol_Close("nor:0:", &err);
    ok &= (err == FS_ERR_NONE);
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    // Create FAT volume with default cluster size
    // OK to save memory usage and reuse blockBuffer for buffer, as BLOCK_BUFFER_SIZE >= _MAX_SS
    err = f_mkfs(_T(""), (BYTE)FM_ANY, 0u, blockBuffer, (UINT)_MAX_SS);
    ok &= (err == (int)FR_OK);

    // Mount logical drive
    if(ok)
    {
        err = f_mount(&fs, _T(""), 0u);
        ok &= (err == (int)FR_OK);
    }

    // Set drive label
    if(ok)
    {
        char name[11];
        PV624->getInstrumentName(name);
        err = f_setlabel(name);
        ok &= (err == (int)FR_OK);
    }

    // Create directories
    if(ok)
    {
        for(int i = 0; directories[i] != NULL; i++)
        {
            err = f_mkdir(_T(directories[i]));
            ok &= (err == (int)FR_OK);
        }
    }

    // Unmount logical drive
    if(ok)
    {
        err = f_mount(0u, _T(""), 0u);
        ok &= (err == (int)FR_OK);
    }

#endif

    return ok;
}

/**
 * @brief   Get status of External Flash Memory
 * @param   uint32_t *bytesUsed, uint32_t *bytesTotal
 * @return  bool - true if ok, false if not ok
 */
bool DExtStorage::getStatus(uint32_t *bytesUsed, uint32_t *bytesTotal)
{
    bool ok = true;
    *bytesUsed = 0u;
    *bytesTotal = 0u;

#ifdef USE_FATFS
    DWORD fre_clust;
    FRESULT err = FR_OK;
    FILINFO fno;
    FATFS *pFs;
    pFs = &fs;

    if(ok)
    {
        // Mount logical drive
        err = f_mount(&fs, "", 0u);
        ok &= (err == (int)FR_OK);
    }

    // Check for existence of file system, err will be FR_NO_FILESYSTEM if missing
    if(ok)
    {
        for(int i = 0; directories[i] != NULL; i++)
        {
            err = f_stat(_T(directories[i]), &fno);
            ok &= (err == (int)FR_OK);
        }
    }

    // Get volume information and free clusters of drive
    err = f_getfree(_T(""), &fre_clust, &pFs);
    ok &= (err == (int)FR_OK);

    // Unmount logical drive
    if(ok)
    {
        err = f_mount(0u, _T(""), 0u);
        ok &= (err == (int)FR_OK);
    }

    if(ok)
    {
        // Get total sectors and free sectors
        uint32_t totalSectors = (pFs->n_fatent - 2u) * pFs->csize;
        uint32_t remainingSectors = fre_clust * pFs->csize;
        uint32_t usedSectors = totalSectors - remainingSectors;

        *bytesUsed = usedSectors * (UINT)_MAX_SS;
        *bytesTotal = totalSectors * (UINT)_MAX_SS;
    }

#endif

    return ok;
}

/**
* @brief    Open volume and file
* @param    char* filePath, bool writable
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::openFile(char *filePath, bool writable)
{
    bool ok = true;

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;

    // Mount volume
    if(ok)
    {
        FSVol_Open("nor:0:", "nor:0:", 0u, &err);
        ok &= ((err == FS_ERR_NONE) || (err == FS_ERR_VOL_ALREADY_OPEN));
    }

    // Open file
    if(ok)
    {
        FS_FLAGS mode = writable ? FS_FILE_ACCESS_MODE_WR | FS_FILE_ACCESS_MODE_CREATE | FS_FILE_ACCESS_MODE_TRUNCATE : FS_FILE_ACCESS_MODE_RD;
        f = FSFile_Open(filePath, mode, &err);
        ok &= (f != NULL);
        ok &= (err == FS_ERR_NONE);
    }

#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if(ok)
    {
        // Mount logical drive
        err = f_mount(&fs, "", 0u);
        ok &= (err == (int)FR_OK);
    }

    if(ok)
    {
        BYTE mode = writable ? (BYTE)FA_WRITE | (BYTE)FA_OPEN_APPEND : ((BYTE)FA_READ | (BYTE)FA_OPEN_EXISTING);
        err = f_open(&f, filePath, mode);
        strncpy_s(path, sizeof(path), filePath, (size_t)FILENAME_MAX_LENGTH);
        ok &= (err == (int)FR_OK);
    }

#endif

    return ok;
}

/**
* @brief    Close volume and file
* @param    void
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::close(void)
{
    bool ok = true;

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;

    // Close file - regardless of errors
    FSFile_Close(f, &err);
    f = NULL;
    ok &= (err == FS_ERR_NONE);

    // Unmount volume
    FSVol_Close("nor:0:", &err);
    ok &= (err == FS_ERR_NONE);
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    // Close file - regardless of errors
    err = f_close(&f);
    ok &= (err == (int)FR_OK);

    // Unmount logical drive
    if(ok)
    {
        err = f_mount(0u, _T(""), 0u);
        ok &= (err == (int)FR_OK);
    }

#endif

    return ok;
}

/**
* @brief    Read open readable file
* @param    char* buf, uint32_t length
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::read(char *buf, uint32_t length)
{
    bool ok = true;
    buf[0] = '\0';

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;

    ok &= (f != NULL);

    if(ok)
    {
        FSFile_Rd(f, (void *)buf, (CPU_SIZE_T)length, &err);
        ok &= (err == FS_ERR_NONE);
    }

#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if(ok)
    {
        err = f_read(&f, buf, (UINT)length, NULL);
        ok &= (err == (int)FR_OK);
    }

#endif

    return ok;
}

/**
* @brief    Write open writeable file
* @param    char* buf
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::write(char *buf, uint32_t bufSize)
{
    bool ok = true;
    uint32_t length = (uint32_t)strnlen_s(buf, bufSize);

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;

    ok &= (f != NULL);

    if(ok)
    {
        FSFile_Wr(f, (void *)buf, (CPU_SIZE_T)length, &err);
        ok &= (err == FS_ERR_NONE);
    }

#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if(ok)
    {
        err = f_write(&f, buf, (UINT)length, NULL);
        ok &= (err == (int)FR_OK);
    }

#endif

    if(!ok)
    {
        PV624->updateDeviceStatus(E_ERROR_CODE_EXTERNAL_STORAGE,
                                  eSetError);
    }

    else
    {
        PV624->updateDeviceStatus(E_ERROR_CODE_EXTERNAL_STORAGE,
                                  eClearError);
    }

    return ok;
}

/**
* @brief    Write open writeable file
* @param    char* buf
* @param    unit32_t length
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::write(char *buf, uint32_t bufSize, uint32_t length)
{
    bool ok = true;

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;

    ok &= (f != NULL);

    if(ok)
    {
        FSFile_Wr(f, (void *)buf, (CPU_SIZE_T)length, &err);
        ok &= (err == FS_ERR_NONE);
    }

#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if(ok)
    {
        err = f_write(&f, buf, (UINT)length, NULL);
        ok &= (err == (int)FR_OK);
    }

#endif

    if(!ok)
    {
        PV624->updateDeviceStatus(E_ERROR_CODE_EXTERNAL_STORAGE,
                                  eSetError);
    }

    else
    {
        PV624->updateDeviceStatus(E_ERROR_CODE_EXTERNAL_STORAGE,
                                  eClearError);
    }

    return ok;
}

/**
* @brief    Query size of open readable file in bytes and number of lines
* @param    uint32_t *size
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::query(uint32_t *size, uint32_t *numLines)
{
    bool ok = true;

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;
    FS_ENTRY_INFO info;

    ok &= (f != NULL);

    if(ok)
    {
        FSFile_Query(f, &info, &err);
        *size = info.Size;
        ok &= (err == FS_ERR_NONE);
    }

    *numLines = 0; // TBD

#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;
    FILINFO fno;

    if(ok)
    {
        err = f_stat(path, &fno);
        *size = fno.fsize;
        ok &= (err == (int)FR_OK);
    }

    if(ok)
    {
        char buf;
        *numLines = 0u;
        MISRAC_DISABLE

        while(!f_eof(&f))
        {
            read(&buf, 1);

            if(buf == TERMINATOR_CR)
            {
                (*numLines)++;
            }
        }

        MISRAC_ENABLE
    }

#endif

    return ok;
}

/**
* @brief    Check if file exists
* @param    char* filePath
* @return   bool - true if file exists, false if not ok
*/
bool DExtStorage::exists(char *filePath)
{
    bool ok = true;

    // Close any existing open file
    close();

    ok = openFile(filePath, false);
    ok &= close();

    return ok;
}

/**
* @brief    Erase specified file
* @param    char* filePath
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::erase(char *filePath)
{
    bool ok = true;

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;
    FSEntry_Del(filePath, FS_ENTRY_TYPE_FILE, &err);
    ok &= (err == FS_ERR_NONE);
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if(ok)
    {
        // Mount logical drive
        err = f_mount(&fs, "", 0u);
        ok &= (err == (int)FR_OK);
    }

    if(ok)
    {
        // Erase file
        err = f_unlink(filePath);
        ok &= (err == (int)FR_OK);
        path[0] = '\0';
    }

    // Unmount logical drive
    if(ok)
    {
        err = f_mount(0u, _T(""), 0u);
        ok &= (err == (int)FR_OK);
    }

    if(!ok)
    {
        PV624->updateDeviceStatus(E_ERROR_CODE_EXTERNAL_STORAGE,
                                  eSetError);
    }

    else
    {
        PV624->updateDeviceStatus(E_ERROR_CODE_EXTERNAL_STORAGE,
                                  eClearError);
    }

#endif

    return ok;
}

/**
* @brief    Directory information
* @param    char* path, fileInfo_t* fileInfo (NULL fileInfo.filename indicates no more files)
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::dir(char *path, fileInfo_t *fileInfo)
{
    bool ok = true;

#ifdef USE_UCFS
    // TODO
    /*FS_ERR err = FS_ERR_NONE;
    FS_DIR *p_dir;
    struct fs_dirent dirent;
    struct fs_dirent *p_dirent;
    char str[50];
    //char *p_cwd_path;
    fs_time_t ts;

    p_dir = fs_opendir(path); // Open dir.
    if (p_dir != (FS_DIR *)0)
    {
        (void)fs_readdir_r(pdir, &dirent, &p_dirent); // Rd first dir entry.
        if (p_dirent == (FS_DIRENT *)0)
        {
            strcpy(filename, "empty");
            *size = 0u;
        }
        else
        {
            while (p_dirent != (struct dirent *)0)
            {
                (void)fs_readdir_r(pdir, &dirent, &p_dirent);
            }
            fs_closedir(p_dir);
        }
    }

    ok &= (err == FS_ERR_NONE);*/
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if(d.obj.fs == NULL)
    {
        if(ok)
        {
            // Mount logical drive
            err = f_mount(&fs, "", 0u);
            ok &= (err == (int)FR_OK);
        }

        if(ok)
        {
            // Open the directory
            err = f_opendir(&d, path);
            ok &= (err == (int)FR_OK);
        }
    }

    if(ok)
    {
        // Read a directory item
        FILINFO fno;
        err = f_readdir(&d, &fno);
        ok &= (err == (int)FR_OK);

        if((ok) && (fno.fname[0] != 0u))
        {
            strcpy_s(fileInfo->filename, sizeof(fileInfo->filename), fno.fname);
            fileInfo->size = fno.fsize;
            fileInfo->date.day = ((uint32_t)(fno.fdate)) & 0x1fu;
            fileInfo->date.month = ((uint32_t)fno.fdate & 0x1e0u) >> 5;
            fileInfo->date.year = (((uint32_t)fno.fdate & 0xfe00u) >> 9) + 1980u;
            fileInfo->time.hours = ((uint32_t)fno.fdate & 0xf80u) >> 11;
            fileInfo->time.minutes = ((uint32_t)fno.fdate & 0x7e0u) >> 5;
            fileInfo->time.seconds = ((uint32_t)fno.ftime & 0x1fu) * 2u;
            fileInfo->time.milliseconds = 0u;
            fileInfo->attribInfo.archive = (bool)((uint32_t)fno.fattrib & (uint32_t)AM_ARC);
            fileInfo->attribInfo.directory = (bool)((uint32_t)fno.fattrib & (uint32_t)AM_DIR);
            fileInfo->attribInfo.hidden = (bool)((uint32_t)fno.fattrib & (uint32_t)AM_HID);
            fileInfo->attribInfo.readOnly = (bool)((uint32_t)fno.fattrib & (uint32_t)AM_RDO);
            fileInfo->attribInfo.system = (bool)((uint32_t)fno.fattrib & (uint32_t)AM_SYS);
        }

        else
        {
            f_closedir(&d);
            d.obj.fs = NULL;

            // Unmount logical drive
            if(ok)
            {
                err = f_mount(0u, _T(""), 0u);
                ok &= (err == (int)FR_OK);
            }

            fileInfo->filename[0] = '\0';
            ok = false;
        }
    }

#endif

    return ok;
}

/**
* @brief    Create directory (parent directory must already exist and directory does not already exist)
* @param    char* absolute path
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::mkdir(char *path)
{
    bool ok = true;

#ifdef USE_UCFS
    // TODO
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if(ok)
    {
        // Mount logical drive
        err = f_mount(&fs, "", 0u);
        ok &= (err == (int)FR_OK);
    }

    if(ok)
    {
        // Create the directory
        err = f_mkdir(path);
        ok &= (err == (int)FR_OK);
    }

    // Unmount logical drive
    if(ok)
    {
        err = f_mount(0u, _T(""), 0u);
        ok &= (err == (int)FR_OK);
    }

#endif

    return ok;
}

/**
* @brief Read a line from the open readable file
* @param char buf[FILE_MAX_LINE+1], uint32_t lineLength (NULL buf indicates end of file)
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::readLine(char *buf, uint32_t bufSize, uint32_t lineLength)
{
    bool ok = true;
    bool foundTerminator = false;
    const size_t termLength = sizeof(LINE_TERMINATION) / sizeof(char);
    buf[0] = '\0';

    // Get file pointer before read()
#ifdef USE_UCFS
    ok &= (f != NULL);

    FS_ERR err = FS_ERR_NONE;
    FS_FILE_SIZE fpStart = FSFile_PosGet(f, &err);
    ok &= (err == FS_ERR_NONE);
#endif

#ifdef USE_FATFS
    FSIZE_t fpStart = f_tell(&f); // no possible error code to verify
#endif

    ok &= (lineLength <= (uint32_t)FILE_MAX_LINE_LENGTH);

    if(ok)
    {
        ok &= read(buf, lineLength);
        lineLength = (uint32_t)strnlen_s(buf, bufSize);
    }

    if(ok)
    {
        for(uint32_t i = 0u; i < lineLength; i++)
        {
            if((buf[i] == TERMINATOR_CR) || (buf[i] == TERMINATOR_LF))
            {
                buf[i] = '\0';

                // Rewind file pointer relative to terminator ready for next readLine() call
#ifdef USE_UCFS
                FSFile_PosSet(f, (FS_FILE_OFFSET)fpStart + (FS_FILE_OFFSET)i + (FS_FILE_OFFSET)(termLength - (size_t)1), FS_FILE_ORIGIN_START, &err);
                ok &= (err == FS_ERR_NONE);
#endif

#ifdef USE_FATFS
                FRESULT err = FR_OK;
                err = f_lseek(&f, fpStart + (FSIZE_t)i + (FSIZE_t)(termLength - (size_t)1));
                ok &= (err == (int)FR_OK);
#endif

                foundTerminator = true;
                break;
            }
        }
    }

    ok &= foundTerminator;

    if(!ok)
    {
        buf[0] = '\0';
    }

    return ok;
}

/**
* @brief Write a line to the open writable file with terminator
* @param char* buf
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::writeLine(char *buf, uint32_t bufSize)
{
    bool ok = true;
    const size_t termLength = sizeof(LINE_TERMINATION) / sizeof(char);
    const size_t lineLength = strnlen_s(buf, bufSize);
    char lineBuf[(size_t)FILE_MAX_LINE_LENGTH + termLength];
    ok &= lineLength <= (size_t)FILE_MAX_LINE_LENGTH;

    if(ok)
    {
        snprintf_s(lineBuf, lineLength + termLength, "%s%s", buf, LINE_TERMINATION);
        ok &= write(lineBuf, (uint32_t)((uint32_t)FILE_MAX_LINE_LENGTH + (uint32_t)termLength));
    }

    return ok;
}


/**
* @brief Check if the direction is exist
* @param char* path
* @return bool - true if yes, else false
*/
bool DExtStorage::isDirectoryExist(const char *path)
{
    //return dir(path, &fileInfo);
    // Trim any trailing slashes on path that are incompatible with f_opendir()
    bool ok = false;
    FRESULT err = FR_OK;
    char trimmedPath[FILENAME_MAX_LENGTH] = {'\0'};
    strncpy_s(trimmedPath, sizeof(trimmedPath), path, FILENAME_MAX_LENGTH);

    while(true)
    {
        uint32_t lastChar = (uint32_t)strnlen_s(trimmedPath, sizeof(trimmedPath)) - 1u;

        if(lastChar >= FILENAME_MAX_LENGTH)
        {
            lastChar = FILENAME_MAX_LENGTH - 1u;
        }

        if(trimmedPath[lastChar] == '\\')
        {
            trimmedPath[lastChar] = '\0';
        }

        else
        {
            break;
        }
    }

    close(); // Close any existing open file

    err = f_mount(&fs, "", 0u);
    ok = (err == (int)FR_OK);

    if(ok)
    {
        // Open the directory
        err = f_opendir(&d, trimmedPath);
        ok = (err == (int)FR_OK);
    }

    return ok;

}


/**
* @brief Create the PV624 directory structure needed
* @param void
* @return bool - true if ok, false if not ok
*/
bool DExtStorage::createDirectories(void)
{
    bool ok = true;

#ifdef USE_UCFS

    if(ok)
    {
        for(int i = 0; directories[i] != NULL; i++)
        {
            FSEntry_Create(directories[i], FS_ENTRY_TYPE_DIR, DEF_YES, &err);
            ok &= ((err == FS_ERR_NONE) || (err == FS_ERR_ENTRY_EXISTS));
        }
    }

#endif

#ifdef USE_FATFS

    if(ok)
    {
        for(int i = 0; directories[i] != NULL; i++)
        {
            ok = isDirectoryExist(directories[i]);

            if(false == ok)
            {
                FRESULT err = f_mkdir(_T(directories[i]));
                ok = (err == (int)FR_OK);
            }
        }
    }

#endif

    return ok;
}
/**
* @brief get the directory path from predefined directories
* @param uint16_t index , char* path, uint16_t len
* @return   bool - true if yes, else false
*/
void DExtStorage::getDirectoryPath(uint16_t index, char *path, uint16_t len)
{
    memset_s(path, (rsize_t)len, 0x00, (rsize_t)len);
    snprintf_s(path, (uint32_t)len, directories[index]);
}

bool DExtStorage::deleteDirectory(char *path)
{
    bool ok = false;

#ifdef USE_UCFS
    // TODO
#endif

#ifdef USE_FATFS
    uint32_t i = 0u, j = 0u;
    FRESULT fr;
    DIR dir;
    FILINFO *fno = NULL;

    fr = f_opendir(&dir, path); /* Open the sub-directory to make it empty */
    ok = (fr == (FRESULT)FR_OK);

    if(ok)
    {
        /* Get current path length */
        for(i = 0u; path[i]; i++)
        {}              // do nothing

        path[i++] = _T('/');

        for(;;)
        {
            fr = f_readdir(&dir, fno);  /* Get a directory item */

            if((fr != (FRESULT)FR_OK) || (!fno->fname[0]) || (ok == false))
            {
                break;          // End of directory? exit
            }

            else
            {
                j = 0u;

                do
                {
                    /* Make a path name */
                    if((i + j) >= FILENAME_MAX_LENGTH)
                    {
                        /* Buffer over flow? */
                        fr = (FRESULT)100u;
                        break;    /* Fails with 100 when buffer overflow */
                    }

                    path[i + j] = fno->fname[j];
                }
                while(fno->fname[j++]);

                if(fno->fattrib & (BYTE)AM_DIR)
                {
                    /* Item is a sub-directory */
                    ok &= deleteDirectory(path);
                }

                else
                {
                    /* Item is a file */
                    if(f_unlink(path) != (FRESULT)FR_OK)
                    {
                        ok = false;
                    }
                }
            }
        }

        path[--i] = 0u;  /* Restore the path name */
        f_closedir(&dir);

        if(fr == (FRESULT)FR_OK)
        {
            fr = f_unlink(path);  /* Delete the empty sub-directory */
        }

        ok = (fr == (FRESULT)FR_OK);
    }

#endif

    return ok;
}

/**
* @brief Validate received file
* @param none
* @return   bool - true if yes, else false
*/
bool DExtStorage::validateMainFwFile(void)
{
    uint8_t fileHeaderData[HEADER_SIZE] = {0u};
    uint32_t mainImageSize = 0u;
    bool validImage = false;    // Used to check valid image and USB File open
    sVersion_t mainAppVersion;
    uint8_t currentVersionStr[10u] = {0};     // For Reading version number from getVersion function

    upgradeStatus = E_UPGRADE_VALIDATING_MAIN_APP;

    mainAppVersion.major = (uint8_t)cAppVersion[1];
    mainAppVersion.minor = (uint8_t)cAppVersion[2];
    mainAppVersion.build = (uint8_t)cAppVersion[3];

    // Check min Bootloader version for setting upgradeStatus flag
    PV624->getVersion(E_ITEM_PV624, E_COMPONENENT_BOOTLOADER, (char *)currentVersionStr);


    if(false == validateBootloaderVersionNumber(&currentVersionStr[0], (uint32_t)100))
    {
        upgradeStatus = E_UPGRADE_ERROR_INVALID_MAIN_BOOTLOADER;
    }

    mainUcFwUpgradeRequired = false;
    generateTableCrc8ExternalStorage(CRC8_POLYNOMIAL);

    // Open upgrade file for reading (prioritise release builds but also allow development builds)
    validImage = openFile("\\DK0514.raw", false);

    if(!validImage)
    {
        upgradeStatus = E_UPGRADE_ERROR_FILE_NOT_FOUND;
    }

    if(validImage)
    {
        MISRAC_DISABLE
        // Fill read buffer 0 to avoid data interruption
        memset_s(fileHeaderData, sizeof(fileHeaderData), 0, sizeof(fileHeaderData)); // clear entire buffer for final block which might not be fully filled with frames

        // Read 40 bytes Header data of Main uC FW
        read((char *)&fileHeaderData[0], (uint32_t)HEADER_SIZE);

        validImage = false;

        // validate main uC header crc
        if(true == validateHeaderCrc(&fileHeaderData[0]))
        {
            // Read and validate Image Size from main uC DK0514.raw file header, Max image size as (3 x size of .raw file)
            if(true == validateImageSize(&fileHeaderData[0], &mainImageSize, (uint32_t)MAX_ALLOWED_MAIN_APP_FW))
            {
                // validate Image crc of main uC from DK0514.raw file
                if(true == validateImageCrc(&fileHeaderData[0], mainImageSize))
                {
                    // Validate Main uC File name, Compare received file name with expected file name, string compare:
                    //Validate (FileName)DK number, Max version Number
                    if(true == validateHeaderInfo(&fileHeaderData[FILENAME_START_POSITION], mainAppVersion, (uint8_t *)mainAppDkNumber))
                    {
                        validImage = true;

                        // Validate Main uC FW version, Compare old with new version number, Minor version V 00.MM.00, sub Version V 00.00.MM
                        if(true == validateVersionNumber(&fileHeaderData[MAJOR_VERSION_NUMBER_START_POSITION], mainAppVersion))
                        {
                            mainUcFwUpgradeRequired = true;
                            // calculate block size and number of frames for writing to bank2   // we don't nee to read again the file size
                            numberOfFramesLeft = mainImageSize / BYTES_PER_FRAME;
                            numberOfBlocks = ((numberOfFramesLeft - 1u) / NUM_FRAMES_PER_BLOCK) + 1u; // rounded up to next block
                            upgradeStatus = E_UPGRADE_VALIDATED_MAIN_APP;
                        }

                        else
                        {
                            mainUcFwUpgradeRequired = false;
                            upgradeStatus = E_UPGRADE_ERROR_MAIN_APP_VERSION_INVALID;
                        }

                    }

                    else
                    {
                        upgradeStatus = E_UPGRADE_ERROR_MAIN_FILE_HEADER_INVALID;
                    }

                }

                else
                {
                    upgradeStatus = E_UPGRADE_ERROR_MAIN_APP_IMAGE_CRC_INVALID;
                }

            }

            else
            {
                upgradeStatus = E_UPGRADE_ERROR_MAIN_APP_FILE_SIZE_INVALID;
            }
        }

        else
        {
            upgradeStatus = E_UPGRADE_ERROR_MAIN_FILE_HEADER_CRC_INVALID;
        }

        MISRAC_ENABLE
    }

    return(validImage);
}
/**
* @brief Validate secondary uC received file
* @param none
* @return bool - true if yes, else false
*/
bool DExtStorage::validateSecondaryFwFile(void)
{
    bool ok = false;
    uint8_t fileHeaderData[HEADER_SIZE] = {0u};
    sVersion_t secondaryAppVersion;
    sVersion_t secondaryBlVersion;
    bool validImage = false;    // Used to check valid image

    secondaryUcFwUpgradeRequired = false;

    // Read Version number of Secondary uC to compare current version
    PV624->stepperMotor->readVersionInfo();
    PV624->stepperMotor->getAppVersion(&secondaryAppVersion);
    PV624->stepperMotor->getBootVersion(&secondaryBlVersion);

    const uint32_t version = (10000u * (secondaryBlVersion.major % 100u)) + (100u * (secondaryBlVersion.minor % 100u)) + (secondaryBlVersion.build % 100u);
    const uint32_t minVersionBL = 100u; // DK0510 00.01.00 onwards
    ok = (version > minVersionBL);

    if(!ok)
    {
        upgradeStatus = E_UPGRADE_ERROR_INVALID_SEC_BOOTLOADER;
    }

    if(ok)
    {
        generateTableCrc8ExternalStorage(CRC8_POLYNOMIAL);      // For crc8 calculation

        // read '\n'
        read((char *)&fileHeaderData, (uint32_t)ONE_BYTE);
        // Fill read buffer 0xFF to avoid data interruption
        memset_s(fileHeaderData, sizeof(fileHeaderData), 0, sizeof(fileHeaderData)); // clear entire buffer for final block which might not be fully filled with frames

        // Read 40 bytes Header data of secondary uC FW
        read((char *)&fileHeaderData[0], (uint32_t)HEADER_SIZE);

        MISRAC_DISABLE

        // validate secondary uC header crc
        if(true == validateHeaderCrc(&fileHeaderData[0]))
        {
            // Read and validate Image Size from secondary uC DK0514.raw file header, Max image size as (3 x size of .raw file)
            if(true == validateImageSize(&fileHeaderData[0], &secondaryFwFileSizeInt, (uint32_t)MAX_ALLOWED_SECONDARY_APP_FW))
            {
                // validate Image crc of secondary uC from DK0514.raw file
                if(true == validateImageCrc(&fileHeaderData[0], secondaryFwFileSizeInt))
                {
                    // Validate secondary uC File name, Compare received file name with expected file name, string compare:
                    //Validate (FileName)DK number, Max version Number
                    if(true == validateHeaderInfo(&fileHeaderData[FILENAME_START_POSITION], secondaryAppVersion, (uint8_t *)secondaryAppDkNumber))
                    {
                        validImage = true;

                        // Validate secondary uC FW version, Compare old with new version number,  Minor version V 00.MM.00, sub Version V 00.00.MM
                        if(true == validateVersionNumber(&fileHeaderData[MAJOR_VERSION_NUMBER_START_POSITION], secondaryAppVersion))
                        {
                            secondaryUcFwUpgradeRequired = true;
                            upgradeStatus = E_UPGRADE_VALIDATED_SEC_APP;
                        }

                        else
                        {
                            secondaryUcFwUpgradeRequired = false;
                            upgradeStatus = E_UPGRADE_ERROR_SEC_APP_VERSION_INVALID;
                        }
                    }

                    else
                    {
                        upgradeStatus = E_UPGRADE_ERROR_SEC_FILE_HEADER_INVALID;
                    }
                }

                else
                {
                    upgradeStatus = E_UPGRADE_ERROR_SEC_APP_IMAGE_CRC_INVALID;
                }
            }

            else
            {
                upgradeStatus = E_UPGRADE_ERROR_SEC_APP_FILE_SIZE_INVALID;
            }
        }

        else
        {
            upgradeStatus = E_UPGRADE_ERROR_SEC_FILE_HEADER_CRC_INVALID;
        }

        MISRAC_ENABLE
    }

    return(validImage);
}
/**
* @brief    updade Firmware - perform application firmware upgrade for main uC
* @param    void
* @return bool - true if yes, else false
*/
bool DExtStorage::updateMainUcFirmware(void)
{
    bool ok = false;
    uint32_t apiCommand = 999u;
    char tempBuf[HEADER_SIZE];      // Used for temporary reading of header information to seek cursor to start address of main uC FW
    uint32_t blockCounter = 0u;         // used as counter for reading and writing to bank 2
    uint32_t frame = 0u;                // used as counter for reading data from DK0514.raw file
    uint32_t numberOfFrames = 0u;

    // bootloaderAPI Test (returns 3 on success)
    bootLoaderError = bootloaderApi(BL_API_TEST3, NULL, 0u, 0u, &hcrc);
    ok = (bootLoaderError == BL_API_TEST3);

    if(!ok)
    {
        upgradeStatus = E_UPGRADE_ERROR_MAIN_APP_API_FAIL;
    }

    if(ok)
    {
        // Bootloader API call - Mass erase flash bank 2 (returns 0 on success)
        apiCommand = BL_API_BANK2MASSERASE;
        bootLoaderError = bootloaderApi(apiCommand, &dummy, 0u, 0u, &hcrc);
        ok &= (bootLoaderError == 0u);
        HAL_Delay(100u);

        if(!ok)
        {
            upgradeStatus = E_UPGRADE_ERROR_MAIN_APP_ERASE_FAIL;
            PV624->handleError(E_ERROR_ON_BOARD_FLASH,
                               eSetError,
                               0u,
                               3102u);
        }

    }


    // Open upgrade file for reading (prioritise release builds but also allow development builds)
    ok = openFile("\\DK0514.raw", false);

    if(!ok)
    {
        PV624->handleError(E_ERROR_CODE_FIRMWARE_UPGRADE_FAILED,
                           eSetError,
                           0u,
                           3105u);
        upgradeStatus = E_UPGRADE_ERROR_FILE_NOT_FOUND;
    }

    else
    {
        PV624->handleError(E_ERROR_CODE_FIRMWARE_UPGRADE_FAILED,
                           eClearError,
                           0u,
                           3106u);

    }

    MISRAC_DISABLE
    read(tempBuf, HEADER_SIZE);
    MISRAC_ENABLE

    if(ok)
    {
        reset = 1u; // only for first frame

        for(blockCounter = 0u; blockCounter < numberOfBlocks; blockCounter++)
        {
            if(ok)
            {
                memset_s(blockBuffer, BLOCK_BUFFER_SIZE, 0xFF, BLOCK_BUFFER_SIZE); // clear entire buffer for final block which might not be fully filled with frames
                numberOfFrames = (numberOfFramesLeft < NUM_FRAMES_PER_BLOCK) ? numberOfFramesLeft : NUM_FRAMES_PER_BLOCK; // i.e. never more than NUM_FRAMES_PER_BLOCK per block

                for(frame = 0u; frame < numberOfFrames; frame++)
                {
                    if(ok)
                    {
                        ok &= read((char *)&blockBuffer[frame * BYTES_PER_FRAME], (uint32_t)BYTES_PER_FRAME);
                    }
                }
            }

            if(!ok)
            {
                upgradeStatus = E_UPGRADE_ERROR_MAIN_APP_IMAGE_READ_FAIL;
            }

            // Bootloader API call - write to FLASH bank 2 RAM buffer (returns 0 on success)
            // reset - 1 on first frame, 0 on subsequent
            // numberOfFrames - always 15 until last block - otherwise error is returned
            if(ok)
            {
                apiCommand = BL_API_BANK2WRITE;
                bootLoaderError = bootloaderApi(apiCommand, blockBuffer, numberOfFrames, reset, &hcrc);
                ok &= (bootLoaderError == 0u);
                reset = 0u; // for subsequent frames
                numberOfFramesLeft = (numberOfFramesLeft >= NUM_FRAMES_PER_BLOCK) ? numberOfFramesLeft - NUM_FRAMES_PER_BLOCK : 0u; // i.e. positive or zero for unsigned type

                if(!ok)
                {
                    PV624->handleError(E_ERROR_ON_BOARD_FLASH,
                                       eSetError,
                                       0u,
                                       3103u);
                }
            }

            if(!ok)
            {
                upgradeStatus = E_UPGRADE_ERROR_MAIN_APP_DATA_WRITE_FAIL;
                ok = false;
                break;
            }
        }
    }

    // Close upgrade file - regardless of errors
    ok &= close();
    HAL_Delay(100u);

    if(ok)
    {
        upgradeStatus = E_UPGRADE_UPGRADING_MAIN_APP;
//        HAL_IWDG_Refresh(&hiwdg);
//        HAL_Delay(1000u);
        // Disable interrupts and scheduler to avoid any interruption whilst overwriting flash bank 1 inc. interrupt vector table
        __disable_irq();

        // Complete the upgrade process
        apiCommand = BL_API_BANK1WRITE;
        bootLoaderError = bootloaderApi(apiCommand, &dummy, 0u, 0u, &hcrc);
        ok = (bootLoaderError == 0u);

        if(!ok)
        {
            PV624->handleError(E_ERROR_ON_BOARD_FLASH,
                               eSetError,
                               0u,
                               3104u);
        }

        // Bootloader should cause a system reset regardless of success. There is nothing to do or can be done if the application continues to execute.
    }

    else
    {
        ok = false;
    }

    return(ok);
}
/**
* @brief    update Firmware - perform application firmware upgrade for secondary uC
* @param    void
* @return bool - true if yes, else false
*/
bool DExtStorage::updateSecondaryUcFirmware(void)
{
    bool ok = true;
    uint8_t tempBuf[HEADER_SIZE];      // Used for temporary reading of header information to seek cursor to start address of main uC FW
    uint32_t blockCounter = 0u;         // used as counter for reading and writing to bank 2
    uint32_t frame = 0u;
    uint8_t acknowledgement = 0u;
    uint32_t secondaryUcNumberOfBlocks;
    uint32_t secondaryUcNumberOfBytesLeft;
    uint32_t mainUcFileSize = 0u;
    uint8_t receivedDataBuffer[RECEIVED_DATA_BLOCK_SIZE] = {0u};        // Used to read data
    uint8_t secondaryUcBlockBuffer[SECONDARY_UC_BYTES_PER_FRAME + RECORD_NUMBER] = {0u}; // block buffer consists of NUM_FRAMES_PER_BLOCK frames of BYTES_PER_FRAME bytes

    union
    {
        uint16_t recordNumber = 0u;
        uint8_t recordNumberArray[2];
    } fwRecordNumber;

//    HAL_IWDG_Refresh(&hiwdg);
//    HAL_Delay(1000u);

    secondaryUcNumberOfBlocks = secondaryFwFileSizeInt / SECONDARY_UC_BYTES_PER_FRAME;
    secondaryUcNumberOfBytesLeft = secondaryFwFileSizeInt % SECONDARY_UC_BYTES_PER_FRAME;

    // Open upgrade file for reading (prioritise release builds but also allow development builds)
    ok = openFile("\\DK0514.raw", false);

    if(!ok)
    {
        PV624->handleError(E_ERROR_CODE_FIRMWARE_UPGRADE_FAILED,
                           eSetError,
                           0u,
                           3107u);
        upgradeStatus = E_UPGRADE_ERROR_FILE_NOT_FOUND;
    }

    else
    {
        PV624->handleError(E_ERROR_CODE_FIRMWARE_UPGRADE_FAILED,
                           eClearError,
                           0u,
                           3108u);
    }

    MISRAC_DISABLE
    ok &= read((char *)tempBuf, ((uint32_t)HEADER_SIZE));       // Read Main Header

    validateImageSize(tempBuf, &mainUcFileSize, (uint32_t)MAX_ALLOWED_MAIN_APP_FW);
    secondaryUcNumberOfBytesLeft = mainUcFileSize % RECEIVED_DATA_BLOCK_SIZE;           // Remaining data in file
    secondaryUcNumberOfBlocks = mainUcFileSize / RECEIVED_DATA_BLOCK_SIZE;              // No of Blocks required in multiple of 256 bytes

    // Read main Uc Data to move cursor to next position
    for(blockCounter = 0u; blockCounter < (secondaryUcNumberOfBlocks + 1u); blockCounter++)
    {
        frame = (blockCounter < secondaryUcNumberOfBlocks) ? RECEIVED_DATA_BLOCK_SIZE : secondaryUcNumberOfBytesLeft;

        memset_s(receivedDataBuffer, RECEIVED_DATA_BLOCK_SIZE, 0xFF, RECEIVED_DATA_BLOCK_SIZE); // clear entire buffer for final block which might not be fully filled with frames

        read((char *)&receivedDataBuffer[0], (uint32_t)frame);
    }

    ok &= read((char *)tempBuf, ((uint32_t)ONE_BYTE));  // Read '\n'
    ok &= read((char *)tempBuf, ((uint32_t)HEADER_SIZE));       // Read Secondary Header data

    secondaryUcNumberOfBlocks = secondaryFwFileSizeInt / SECONDARY_UC_BYTES_PER_FRAME;


    PV624->secondaryUcFwUpgradeCmd(secondaryFwFileSizeInt, &acknowledgement);           // This command is to switch the state machine of secondary uC Application

    ok = false;

    if(ACK_FW_UPGRADE == acknowledgement)
    {
        ok = true;
        acknowledgement = NACK_FW_UPGRADE;       // Made this zero to reuse the variable
    }

    else
    {
        upgradeStatus = E_UPGRADE_ERROR_SEC_APP_CMD_FAIL;
    }

    if(ok)
    {
        for(blockCounter = 0u; blockCounter < secondaryUcNumberOfBlocks; blockCounter++)
        {
            fwRecordNumber.recordNumber = blockCounter + 1u;

            memset_s(secondaryUcBlockBuffer, sizeof(secondaryUcBlockBuffer), 0xFF, SECONDARY_UC_BYTES_PER_FRAME + RECORD_NUMBER); // clear entire buffer for final block which might not be fully filled with frames

            secondaryUcBlockBuffer[0] = fwRecordNumber.recordNumberArray[0];
            secondaryUcBlockBuffer[1] = fwRecordNumber.recordNumberArray[1];

            ok &= read((char *)&secondaryUcBlockBuffer[2], SECONDARY_UC_BYTES_PER_FRAME);

            if(!ok)
            {
                upgradeStatus = E_UPGRADE_ERROR_SEC_APP_IMAGE_READ_FAIL;
            }

            else
            {
                PV624->secondaryUcFwUpgrade((uint8_t *)&secondaryUcBlockBuffer[0], (SECONDARY_UC_BYTES_PER_FRAME + RECORD_NUMBER), &acknowledgement);

                if(acknowledgement == ACK_FW_UPGRADE)
                {
                    upgradeStatus = E_UPGRADE_UPGRADING_SEC_APP;
                    ok = true;
                }

                else
                {
                    upgradeStatus = E_UPGRADE_ERROR_SEC_APP_DATA_WRITE_FAIL;
                    ok = false;
                    break;
                }
            }
        }
    }

    else
    {
        ok = false;
    }

    // Close upgrade file - regardless of errors
    close();
    HAL_Delay(100u);
    MISRAC_ENABLE
    return(ok);
}

/**
* @brief    Validate Header CRC
* @param    uint8_t *HeaderData: This pointer contains the Header array
* @return   bool ok = 1 for successful execution of function else ok = 0
*/
bool DExtStorage::validateHeaderCrc(uint8_t *HeaderData)
{
    bool ok = true;
    uint8_t tempCounter = 0u;
    uint8_t receivedHeaderCrc = 0u;       // used to pack received header Crc
    uint8_t calculatedHeaderCrc = 0u;     // used to compare with received header Crc
    uint8_t ucHeaderCrc[HEADER_CRC_BUFFER + 1u] = {0u}; // 1u for atoi end of character
    uint8_t Counter = 0u;

    for(tempCounter = (HEADER_SIZE - HEADER_CRC_BUFFER);
            ((tempCounter < HEADER_SIZE) && (Counter < sizeof(ucHeaderCrc)));
            tempCounter++)
    {
        ucHeaderCrc[Counter] = HeaderData[tempCounter];               //ucHeaderCrc used to convert atoi

        if((HeaderData[tempCounter] < '0') && (HeaderData[tempCounter] > '9'))
        {
            ok = false;
        }

        Counter++;
    }

    ucHeaderCrc[HEADER_CRC_BUFFER] = '\0';      // added for atoi end of character

    if(true == ok)
    {
        MISRAC_DISABLE
        receivedHeaderCrc = (uint32_t)(atoi((char const *)ucHeaderCrc));          // receivedHeaderCrc will have received crc from usb/vcp file
        // Calculate Header crc and compare with received header crc
        calculatedHeaderCrc = 0u;
        crc8((uint8_t *)HeaderData, (uint8_t)(HEADER_SIZE - HEADER_CRC_BUFFER), (uint8_t *)(&calculatedHeaderCrc));

        // If calculated Header crc is matched with received header crc then Go for validation of Image CRC
        if(calculatedHeaderCrc == receivedHeaderCrc)
        {
            ok = true;
        }

        else
        {
            ok = false;
        }
    }

    MISRAC_ENABLE
    return(ok);
}

/**
* @brief    Validate Image CRC
* @param    uint8_t *HeaderData: This pointer contains the Header array
* @param    uint32_t imageSize: In this variable, we will have Image Size from Header file
* @return   bool ok = 1 for successful execution of function else ok = 0
*/
bool DExtStorage::validateImageCrc(uint8_t *HeaderData, uint32_t imageSize)
{
    bool ok = true;
    uint32_t tempCounter = 0u;
    uint32_t receivedImageCrc = 0u;       // used to pack received Image Crc
    uint32_t calculatedImageCrc = 0u;     // used to compare with received Image Crc
    uint8_t ucImageCrc[IMAGE_CRC_BUFFER_SIZE + 1u] = {0u}; // Used to extract information from header,  // 1u for atoi end of character
    uint8_t Counter = 0u;               // Used as counter purpose
    uint32_t bufferSize = 0u;           // Used in read data for loop
    uint32_t leftBytes = 0u;            // Used for crc32 calculation
    uint32_t numBlocks = 0u;             // Used for crc32 calculation
    uint32_t maxBlocks = 0u;             // Used for crc32 calculation
    uint8_t receivedDataBuffer[RECEIVED_DATA_BLOCK_SIZE] = {0u};        // Used to read data

    if(0u != imageSize) // check if imageSize is non zero value
    {
        for(tempCounter = IMAGE_CRC_START_POSITION ;
                ((tempCounter < IMAGE_CRC_END_POSITION) && (Counter < sizeof(ucImageCrc)));
                tempCounter++)
        {
            ucImageCrc[Counter] = HeaderData[tempCounter];               //ucImageCrc used to convert atoi

            if((HeaderData[tempCounter] < '0') && (HeaderData[tempCounter] > '9'))
            {
                ok = false;
            }

            Counter++;
        }

        ucImageCrc[IMAGE_CRC_BUFFER_SIZE] = '\0'; // Added for atoi end of character

        if(true == ok)
        {
            MISRAC_DISABLE
            receivedImageCrc = (uint32_t)(atoi((char const *)ucImageCrc));      // receivedImageCrc will have received crc
            leftBytes = imageSize % RECEIVED_DATA_BLOCK_SIZE;           // Remaining data in file
            numBlocks = imageSize / RECEIVED_DATA_BLOCK_SIZE;              // No of Blocks required in multiple of 256 bytes

            // if remainder is non zero then take one more block
            if(leftBytes)
            {
                maxBlocks = numBlocks + 1u;
            }

            else
            {
                maxBlocks = numBlocks;
            }

            // Read main uC raw data and calculate crc
            calculatedImageCrc = 0u;      // Need this for crc32 function

            for(tempCounter = 0u; tempCounter < maxBlocks; tempCounter++)
            {
                bufferSize = (tempCounter < numBlocks) ? RECEIVED_DATA_BLOCK_SIZE : leftBytes;

                memset_s(receivedDataBuffer, sizeof(receivedDataBuffer), 0xFF, RECEIVED_DATA_BLOCK_SIZE); // clear entire buffer for final block which might not be fully filled with frames

                read((char *)&receivedDataBuffer[0], (uint32_t)bufferSize);
                calculatedImageCrc = crc32ExternalStorage((uint8_t *)&receivedDataBuffer, bufferSize, calculatedImageCrc);
            }

            // compare calculated crc with received Image Crc from DK0514.raw file
            if(calculatedImageCrc == receivedImageCrc)
            {
                ok = true;
            }

            else
            {
                ok = false;
            }
        }

        else
        {
            ok = false;
        }
    }

    else
    {
        ok = false;       // return 0, if imageSize is zero value
    }

    return(ok);
}

/**
* @brief    Read Image Size
* @param    uint8_t *HeaderData: This pointer contains the Header array
* @param    uint32_t imageSize: In this variable, we will have Image Size from Header file
* @return   bool ok = 1 for successful execution of function else ok = 0
*/
bool DExtStorage::validateImageSize(uint8_t *HeaderData, uint32_t *imageSize, uint32_t maxAllowedImageSize)
{
    bool ok = true;
    uint8_t tempCounter = 0u;
    uint8_t ucImageSizeBuffer[FILESIZE_BUFFER + 1u] = {0u};      // 1u for atoi end of character
    uint8_t Counter = 0u;
    uint32_t receivedImageSize = 0u;

    for(tempCounter = IMAGE_SIZE_START_POSITION;
            ((tempCounter < IMAGE_SIZE_END_POSITION) && (Counter < sizeof(ucImageSizeBuffer)));
            tempCounter++)
    {
        ucImageSizeBuffer[Counter] = HeaderData[tempCounter];               //ucHeaderCrc used to convert atoi

        if((HeaderData[tempCounter] < '0') && (HeaderData[tempCounter] > '9'))
        {
            ok = false;
        }

        Counter++;
    }

    ucImageSizeBuffer[FILESIZE_BUFFER] = '\0';      // added for atoi end of character

    if(ok)
    {
        receivedImageSize = (uint32_t)(atoi((char const *)ucImageSizeBuffer));          // receivedHeaderCrc will have received crc from usb/vcp file

        if(receivedImageSize > maxAllowedImageSize)
        {
            ok = false;
        }

        if(ok)
        {
            MISRAC_DISABLE
            *imageSize = receivedImageSize;
            MISRAC_ENABLE
        }
    }

    return(ok);
}
/**
* @brief    Validate Version Number
* @param    uint8_t *HeaderData: This pointer contains the Header array
* @param    uint8_t currentVersionNumber: In this variable, we will have currentVersion which needs to check with new version received
* @return   bool ok = 1 for successful execution of function else ok = 0
*/
bool DExtStorage::validateVersionNumber(uint8_t *HeaderData, sVersion_t currentAppVersion)
{
    bool ok = true;
    uint8_t versionNum[3u] = {0u};       // Used for atoi conversion, //2u for Version Number and 1u for atoi end of character
    uint8_t receivedVersionNumber = 0u;         // used to get version number in integer
    sVersion_t receivedAppVersion;

    MISRAC_DISABLE
    //received Major version
    versionNum[0] = HeaderData[0];
    versionNum[1] = HeaderData[1];
    versionNum[2] = '\0';             // added for atoi end of character
    receivedVersionNumber = (uint8_t)(atoi((char const *)versionNum));
    receivedAppVersion.major = receivedVersionNumber;

    //received minor version
    versionNum[0] = HeaderData[3];
    versionNum[1] = HeaderData[4];
    versionNum[2] = '\0';             // added for atoi end of character
    receivedVersionNumber = (uint8_t)(atoi((char const *)versionNum));
    receivedAppVersion.minor = receivedVersionNumber;

    //received Build Number
    versionNum[0] = HeaderData[6];
    versionNum[1] = HeaderData[7];
    versionNum[2] = '\0';             // added for atoi end of character
    receivedVersionNumber = (uint8_t)(atoi((char const *)versionNum));
    receivedAppVersion.build = receivedVersionNumber;

    if((currentAppVersion.major != receivedAppVersion.major)
            || (currentAppVersion.minor != receivedAppVersion.minor)
            || (currentAppVersion.build != receivedAppVersion.build))
    {
        ok = true;
    }

    else
    {
        ok = false;
    }

    MISRAC_ENABLE

    return(ok);
}

/**
* @brief    Validate File Name
* @param    uint8_t *HeaderData: This pointer contains the Header array
* @param    uint8_t currentFileName: In this variable, we will have expected file name which needs to check with received file name
* @return   bool ok = 1 for successful execution of function else ok = 0
*/
bool DExtStorage::validateHeaderInfo(uint8_t *HeaderData, sVersion_t receivedAppVersion, const uint8_t *currentDkNumber)
{
    bool ok = true;
    uint8_t tempCounter = 0u;

    // Validate Main uC File name, Compare received file name with expected file name, string compare:
    for(tempCounter = 0u; tempCounter < FILENAME_SIZE; tempCounter++)
    {
        if(currentDkNumber[tempCounter] != HeaderData[tempCounter])
        {
            ok = false;
        }
    }

    // Validate Max 99 version number for Major, minor and build
    if(ok)
    {
        if((receivedAppVersion.major > MAX_VERSION_NUMBER_LIMIT)
                || (receivedAppVersion.minor > MAX_VERSION_NUMBER_LIMIT)
                || (receivedAppVersion.build > MAX_VERSION_NUMBER_LIMIT))
        {
            ok = false;
        }
    }

    // Validate Received Image size is valid or not

    return(ok);
}

/**
* @brief    Get status of upgrade for programming tool
* @param    void
* @return   upgrade status
*/
eUpgradeStatus_t DExtStorage::getUpgradeStatus(void)
{
    return upgradeStatus;
}

/**
* @brief    Validate bootLoaderVersion Number
* @param    uint8_t *HeaderData: This pointer contains the Header array
* @param    uint8_t currentVersionNumber: In this variable, we will have currentVersion which needs to check with new version received
* @return   bool ok = 1 for successful execution of function else ok = 0
*/
bool DExtStorage::validateBootloaderVersionNumber(uint8_t *HeaderData, uint32_t minVersionBL)
{
    bool ok = false;
    uint8_t versionNum[3u] = {0u};       // Used for atoi conversion, //2u for Version Number and 1u for atoi end of character
    sVersion_t currentBlVersion;

    MISRAC_DISABLE
    //received Major version
    versionNum[0] = HeaderData[0];
    versionNum[1] = HeaderData[1];
    versionNum[2] = '\0';             // added for atoi end of character
    currentBlVersion.major = (uint8_t)(atoi((char const *)versionNum));

    //received minor version
    versionNum[0] = HeaderData[3];
    versionNum[1] = HeaderData[4];
    versionNum[2] = '\0';             // added for atoi end of character
    currentBlVersion.minor  = (uint8_t)(atoi((char const *)versionNum));

    //received Build Number
    versionNum[0] = HeaderData[6];
    versionNum[1] = HeaderData[7];
    versionNum[2] = '\0';             // added for atoi end of character
    currentBlVersion.build = (uint8_t)(atoi((char const *)versionNum));

    const uint32_t version = (10000u * (currentBlVersion.major % 100u)) + (100u * (currentBlVersion.minor % 100u)) + (currentBlVersion.build % 100u);
    ok = (version > minVersionBL);

    MISRAC_ENABLE

    return(ok);
}

/**
* @brief    Validate and Upgrade Main and Secondary uC Fw
* @param    uint8_t *HeaderData: This pointer contains the Header array
* @param    uint8_t currentVersionNumber: In this variable, we will have currentVersion which needs to check with new version received
* @return   bool ok = 1 for successful execution of function else return ok = 0
*/
bool DExtStorage::validateAndUpgradeFw(void)
{
    bool fwUpgradeStatus = false;      // For Fw Upgrade
    bool successFlag = true;          // For Fw Upgrade

    // Start LED Blinking once Fw Upgrades Start
    PV624->userInterface->statusLedControl(eStatusProcessing,
                                           E_LED_OPERATION_TOGGLE,
                                           LED_30_SECONDS,
                                           E_LED_STATE_SWITCH_OFF,
                                           0u);

    if(true == validateMainFwFile())
    {
        if(true == validateSecondaryFwFile())
        {
            close();
            HAL_Delay(100u);

            if(true == secondaryUcFwUpgradeRequired)
            {
                successFlag = updateSecondaryUcFirmware();

                if(false == successFlag)
                {
                    fwUpgradeStatus = false;
                }
            }

            else
            {
//                HAL_IWDG_Refresh(&hiwdg);
//                HAL_Delay(1000u);
            }

            if((true == mainUcFwUpgradeRequired) && (true == successFlag))
            {
                successFlag = updateMainUcFirmware();

                if(false == successFlag)
                {
                    fwUpgradeStatus = false;
                }
            }
        }

        else
        {
            fwUpgradeStatus = false;
        }
    }

    else
    {
        fwUpgradeStatus = false;
    }

    if(false == fwUpgradeStatus)
    {
        // Turn Red LED on for 5 Seconds if FW Upgrade is failed
        PV624->userInterface->statusLedControl(eStatusError,
                                               E_LED_OPERATION_SWITCH_ON,
                                               LED_5_SECONDS,
                                               E_LED_STATE_SWITCH_OFF,
                                               1u);

        PV624->handleError(E_ERROR_CODE_FIRMWARE_UPGRADE_FAILED,
                           eSetError,
                           (uint32_t)upgradeStatus,
                           3109u);
    }

    else
    {
        PV624->handleError(E_ERROR_CODE_FIRMWARE_UPGRADE_FAILED,
                           eClearError,
                           0u,
                           3110u);
    }

    return(fwUpgradeStatus);
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma("diag_default=Pm128")