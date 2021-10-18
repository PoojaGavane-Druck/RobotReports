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
#include "DExtStorage.h"
MISRAC_DISABLE
#include <assert.h>
#include <math.h>
#include "main.h"
#include "usbd_msc.h"
MISRAC_ENABLE

#include "DPV624.h"
#include "Utilities.h"

/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define EXTSTORAGE_HANDLER_TASK_STK_SIZE   8192u    //not bytes (CPU_STK is 4 bytes, so multiply by 4 for stack size in bytes)
#define EXTSTORAGE_TASK_TIMEOUT_MS         500u
#define FILE_PREAMBLE_LENGTH 289u
#define NUM_FRAMES 15u
#define BYTES_PER_FRAME 528u
#define BLOCK_BUFFER_SIZE (NUM_FRAMES * BYTES_PER_FRAME)
#define TERMINATOR_CR '\r'
#define TERMINATOR_LF '\n'
#define LINE_TERMINATION "\r\n"

const char *directories[] = { "\\Calibration", "\\DataLog", "\\DocData", "\\DocData\\Local", "\\DocData\\Remote", "\\ErrorLog", "\\EventLog", "\\Favourites", "\\HART", "\\LeakTest", "\\SwitchTest", NULL };

#if !defined USE_FATFS && !defined USE_UCFS
#warning Missing file system middleware :(
#endif

/* Variables --------------------------------------------------------------------------------------------------------*/
CPU_STK extStorageTaskStack[EXTSTORAGE_HANDLER_TASK_STK_SIZE];
extern CRC_HandleTypeDef hcrc;
uint8_t blockBuffer[BLOCK_BUFFER_SIZE] = {0u}; // block buffer consists of NUM_FRAMES frames of BYTES_PER_FRAME bytes
bool bootloaderIrq = false;
USBD_HandleTypeDef *pdevUsbMsc = NULL;
uint8_t epnumUsbMsc;
OS_FLAG_GRP myEventFlagsStorage;               //event flags to pend on
OS_FLAGS myWaitFlagsStorage;                   //events (flags) to which the function will respond

/**
* @brief    Constructor
* @param    os_error is pointer to OS error
* @return   void
*/
DExtStorage::DExtStorage(OS_ERR *os_error)
    : DTask()
{
    myName = "Ext Storage";

    //set up task stack pointer
    myTaskStack = &extStorageTaskStack[0];

    // Register task for health monitoring
#ifdef HEALTH_MONITORING
    registerTask();
#endif
    memset((void*)&myEventFlagsStorage, 0, sizeof(OS_FLAG_GRP));
    OSFlagCreate(&myEventFlagsStorage, myName, (OS_FLAGS)0, os_error);

    bool ok = (*os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (*os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT));

    if(!ok)
    {
        
        PV624->handleError(E_ERROR_OS, 
                           eSetError,
                           (uint32_t)*os_error,
                           (uint16_t)38);
    }

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
    myWaitFlagsStorage = EV_FLAG_USB_MSC_ACCESS | EV_FLAG_FW_UPGRADE | EV_FLAG_FW_VALIDATE;

    activate(myName, (CPU_STK_SIZE)EXTSTORAGE_HANDLER_TASK_STK_SIZE, (OS_PRIO)14u, (OS_MSG_QTY)1u, os_error);
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
#if 0
    // Prerequisites
    MISRAC_DISABLE
    assert(PV624 != NULL);
    MISRAC_ENABLE
#endif
    readyForUpgrade = false;
    verifyingUpgrade = false;
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as we are using snprintf which violates the rule.
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm128")

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

    //task main loop
    while(DEF_TRUE)
    {
        //pend until timeout, blocking, on the events
        actualEvents = OSFlagPend(&myEventFlagsStorage,
                                  myWaitFlagsStorage, (OS_TICK)EXTSTORAGE_TASK_TIMEOUT_MS,
                                  OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                  &cpu_ts,
                                  &os_error);

        //check flags to determine what to execute
#ifdef USE_UCFS
        if ((actualEvents & EV_FLAG_USB_MSC_ACCESS) == EV_FLAG_USB_MSC_ACCESS) // posted by USB OTG interrupt
        {
            if (pdevUsbMsc != NULL)
            {
                MSC_BOT_DataOut(pdevUsbMsc, epnumUsbMsc);
            }
        }
#endif

        if ((actualEvents & EV_FLAG_FW_VALIDATE) == EV_FLAG_FW_VALIDATE)
        {
            validateUpgrade();
        }

        if ((actualEvents & EV_FLAG_FW_UPGRADE) == EV_FLAG_FW_UPGRADE)
        {
            upgradeApplicationFirmware();
        }

#ifdef STACK_MONITOR
        lastTaskRunning = myLastTaskId;
#endif

#ifdef HEALTH_MONITORING
        keepAlive();
#endif
        bool ok = ((os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT)));

        if(!ok)
        {
            PV624->handleError(E_ERROR_OS, 
                               eSetError,
                               (uint32_t)os_error,
                               (uint16_t)43);
        }
    }
}

/**
* @brief    validateUpgrade - validate if ready for application firmware upgrade (assumes instrument is in a suitable state).
* @param    void
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::validateUpgrade(void)
{
    bool ok = true;
    uint32_t size;
    uint32_t numLines;

    // clear flag
    readyForUpgrade = false;

    // Open upgrade file for reading (prioritise release builds but also allow development builds)
    ok = open("\\DK0492.raw", false);
    if (!ok)
    {
        ok = open("\\DK0492_Dev.raw", false);
    }

    // Get file size (numLines not relevant)
    if (ok)
    {
        ok &= query(&size, &numLines);
    }

    // Test that file contains an exact multiple of blocks
    if (ok)
    {
        numberOfFramesLeft = size / BYTES_PER_FRAME;
        numberOfBlocks = ((numberOfFramesLeft - 1u) / NUM_FRAMES) + 1u; // rounded up to next block
        ok &= (size > 0u);
        ok &= ((size % BYTES_PER_FRAME) == 0u);
    }

    // Close upgrade file - regardless of errors
    ok &= close();

    // Test API (returns 3 on success)
    if (ok)
    {
        bootLoaderError = bootloaderApi(BL_API_TEST3, NULL, 0u, 0u, &hcrc);
        ok &= (bootLoaderError == BL_API_TEST3);
    }

    readyForUpgrade = ok;
    verifyingUpgrade = false;
    return readyForUpgrade;
}

/**
* @brief    upgradeApplicationFirmware - perform application firmware upgrade (assumes instrument is in a suitable state and IWDG is not yet running).
* @param    void
* @return   void - should never return
*/
void DExtStorage::upgradeApplicationFirmware(void)
{
    bool ok = true;
    //char dbg[40];
    uint32_t apiCommand = 999u;

    bootloaderIrq = true;

    if (ok)
    {
        // Bootloader API call - Mass erase flash bank 2 (returns 0 on success)
        apiCommand = BL_API_BANK2MASSERASE;
        bootLoaderError = bootloaderApi(apiCommand, &dummy, 0u, 0u, &hcrc);
        ok &= (bootLoaderError == 0u);
    }

    // Open upgrade file for reading (prioritise release builds but also allow development builds)
    ok = open("\\DK0492.raw", false);
    if (!ok)
    {
        ok = open("\\DK0492_Dev.raw", false);
    }

    if (ok)
    {
        reset = 1u; // for first frame
        for (uint32_t block = 0u; block < numberOfBlocks; block++)
        {
#ifdef HEALTH_MONITOR
            keepAlive();
#endif
            // Read encrypted file block from last read position sequentially (recording last position)
            // Must be 15 frames of 528 bytes - until last block - this is checked
            // Place read frames into a cleared RAM buffer (clear with 0xFF)
            // RAM buffer size of 15 frames of 528 bytes
            if (ok)
            {
                memset(blockBuffer, 0xFF, BLOCK_BUFFER_SIZE); // clear entire buffer for final block which might not be fully filled with frames

                numberOfFrames = (numberOfFramesLeft < NUM_FRAMES) ? numberOfFramesLeft : NUM_FRAMES; // i.e. never more than NUM_FRAMES per block
                for (uint32_t frame = 0u; frame < numberOfFrames; frame++)
                {
                    if (ok)
                    {
                        ok &= read((char*)&blockBuffer[frame * BYTES_PER_FRAME], (uint32_t)BYTES_PER_FRAME);
                    }
                }
            }

            // Bootloader API call - write to FLASH bank 2 RAM buffer (returns 0 on success)
            // reset - 1 on first frame, 0 on subsequent
            // numberOfFrames - always 15 until last block - otherwise error is returned
            if (ok)
            {
                apiCommand = BL_API_BANK2WRITE;
                bootLoaderError = bootloaderApi(apiCommand, blockBuffer, numberOfFrames, reset, &hcrc);
                ok &= (bootLoaderError == 0u);
                reset = 0u; // for subsequent frames
                numberOfFramesLeft = (numberOfFramesLeft >= NUM_FRAMES) ? numberOfFramesLeft - NUM_FRAMES : 0u; // i.e. positive or zero for unsigned type
            }
//TODo : Showing FIRMWARE Upgrade status on LED
/*
            SMLCD_ClearVRam();
            SMLCD_PutStr(0u, 0u, "DPI610E Upgrade", fnt7x10);
            int pct = (int)roundf((100.0f * ((float32_t)block + 1.0f) / (float32_t)numberOfBlocks));
            sprintf(dbg, "%03d%% complete", pct);
            SMLCD_PutStr(0u, 15u, dbg, fnt7x10);
            SMLCD_Flush(NULL);
*/
            if (!ok)
            {
                break;
            }
        }
    }

    // Close upgrade file - regardless of errors
    ok &= close();

    // Bootloader API call - write to FLASH bank 1 image from bank 2 (returns 0 on success)
    if (ok)
    {
      //ToDo:Showing  upgrade  status  on LED
      /*
        SMLCD_ClearVRam();
        SMLCD_PutStr(0u, 0u, "DPI610E Upgrade", fnt7x10);
        SMLCD_PutStr(0u, 15u, "Updating...", fnt7x10);
        SMLCD_Flush(NULL);
*/
        // Disable interrupts and scheduler to avoid any interruption whilst overwriting flash bank 1 inc. interrupt vector table
        __disable_irq();

        // Complete the upgrade process
        apiCommand = BL_API_BANK1WRITE;
        bootLoaderError = bootloaderApi(apiCommand, &dummy, 0u, 0u, &hcrc);

        // Bootloader should cause a system reset regardless of success. There is nothing to do or can be done if the application continues to execute.
    }

    PV624->handleError(E_ERROR_CODE_EXTERNAL_STORAGE, 
                               eSetError,
                               (uint32_t)bootLoaderError,
                               (uint16_t)42);
     //ToDo:Showing  upgrade  status  on LED
    /*
    SMLCD_ClearVRam();
    SMLCD_PutStr(0u, 0u, "DPI610E Upgrade", fnt7x10);
    sprintf(dbg, "Failed at API:%d, Err:%d", apiCommand, bootLoaderError);
    SMLCD_PutStr(0u, 15u, dbg, fnt7x10);
    SMLCD_Flush(NULL);
   */

    while(1)
    {
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
    OSTaskQPost(&myTaskTCB, (void *)message.value, (OS_MSG_SIZE)0, (OS_OPT) OS_OPT_POST_FIFO, &os_error);

    if(os_error != static_cast<OS_ERR>(OS_ERR_NONE))
    {
        
        PV624->handleError(E_ERROR_OS, 
                               eSetError,
                               (uint32_t)os_error,
                               (uint16_t)41);
    }

    return os_error;
}

/**
* @brief    upgradeFirmware - perform application firmware upgrade (assumes instrument is in a suitable state).
* @param    OS_FLAGS flags
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::upgradeFirmware(OS_FLAGS flags)
{
    verifyingUpgrade = (flags == EV_FLAG_FW_VALIDATE);

    OS_ERR os_error = OS_ERR_NONE;
    OSFlagPost(&myEventFlagsStorage, flags, OS_OPT_POST_FLAG_SET, &os_error);

    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE));

    if(!ok)
    {
        PV624->handleError(E_ERROR_OS, 
                           eSetError,
                           (uint32_t)os_error,
                           (uint16_t)40);
    }

    // Wait for completion
    while (verifyingUpgrade)
    {
        sleep(10u);
    }

    ok &= readyForUpgrade;

    return ok;
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
    if (ok)
    {
        FSVol_Open("nor:0:", "nor:0:", 0u, &err);
        ok &= ((err == FS_ERR_NONE) || (err == FS_ERR_VOL_ALREADY_OPEN));
    }

    // Create FAT volume with default cluster size
    if (ok)
    {
        FSVol_Fmt("nor:0:", (void *)0, &err);
        ok &= (err == FS_ERR_NONE);
    }

    // Set drive label
    if (ok)
    {
        char name[11];
        PV624->getInstrumentName(name);
        FSVol_LabelSet("nor:0:", name, &err);
        ok &= (err == FS_ERR_NONE);
    }

    // Create directories
    if (ok)
    {
        for (int i = 0; directories[i] != NULL; i++)
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
    if (ok)
    {
        err = f_mount(&fs, _T(""), 0u);
        ok &= (err == (int)FR_OK);
    }

    // Set drive label
    if (ok)
    {
        char name[11];
        PV624->getInstrumentName(name);
        err = f_setlabel(name);
        ok &= (err == (int)FR_OK);
    }

    // Create directories
    if (ok)
    {
        for (int i = 0; directories[i] != NULL; i++)
        {
            err = f_mkdir(_T(directories[i]));
            ok &= (err == (int)FR_OK);
        }
    }

    // Unmount logical drive
    if (ok)
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

    if (ok)
    {
        // Mount logical drive
        err = f_mount(&fs, "", 0u);
        ok &= (err == (int)FR_OK);
    }

    // Check for existence of file system, err will be FR_NO_FILESYSTEM if missing
    if (ok)
    {
        for (int i = 0; directories[i] != NULL; i++)
        {
            err = f_stat(_T(directories[i]), &fno);
            ok &= (err == (int)FR_OK);
        }
    }

    // Get volume information and free clusters of drive
    err = f_getfree(_T(""), &fre_clust, &pFs);
    ok &= (err == (int)FR_OK);

    // Unmount logical drive
    if (ok)
    {
        err = f_mount(0u, _T(""), 0u);
        ok &= (err == (int)FR_OK);
    }

    if (ok)
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
bool DExtStorage::open(char* filePath, bool writable)
{
    bool ok = true;

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;

    // Mount volume
    if (ok)
    {
        FSVol_Open("nor:0:", "nor:0:", 0u, &err);
        ok &= ((err == FS_ERR_NONE) || (err == FS_ERR_VOL_ALREADY_OPEN));
    }

    // Open file
    if (ok)
    {
        FS_FLAGS mode = writable ? FS_FILE_ACCESS_MODE_WR | FS_FILE_ACCESS_MODE_CREATE | FS_FILE_ACCESS_MODE_TRUNCATE : FS_FILE_ACCESS_MODE_RD;
        f = FSFile_Open(filePath, mode, &err);
        ok &= (f != NULL);
        ok &= (err == FS_ERR_NONE);
    }
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if (ok)
    {
        // Mount logical drive
        err = f_mount(&fs, "", 0u);
        ok &= (err == (int)FR_OK);
    }

    if (ok)
    {
        BYTE mode = writable ? (BYTE)FA_WRITE | (BYTE)FA_OPEN_APPEND : (BYTE)FA_READ;
        err = f_open(&f, filePath, mode);
        strncpy(path, filePath, (size_t)FILENAME_MAX_LENGTH);
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
    if (ok)
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
bool DExtStorage::read(char* buf, uint32_t length)
{
    bool ok = true;
    buf[0] = '\0';

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;

    ok &= (f != NULL);

    if (ok)
    {
        FSFile_Rd(f, (void *)buf, (CPU_SIZE_T)length, &err);
        ok &= (err == FS_ERR_NONE);
    }
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if (ok)
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
bool DExtStorage::write(char* buf)
{
    bool ok = true;
    uint32_t length = (uint32_t)strlen(buf);

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;

    ok &= (f != NULL);

    if (ok)
    {
        FSFile_Wr(f, (void *)buf, (CPU_SIZE_T)length, &err);
        ok &= (err == FS_ERR_NONE);
    }
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if (ok)
    {
        err = f_write(&f, buf, (UINT)length, NULL);
        ok &= (err == (int)FR_OK);
    }
#endif

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

    if (ok)
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

    if (ok)
    {
        err = f_stat(path, &fno);
        *size = fno.fsize;
        ok &= (err == (int)FR_OK);
    }

    if (ok)
    {
        char buf;
        *numLines = 0u;
        MISRAC_DISABLE
        while(!f_eof(&f))
        {
            read(&buf, 1);
            if (buf == TERMINATOR_CR)
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
bool DExtStorage::exists(char* filePath)
{
    bool ok = true;

    // Close any existing open file
    close();

    ok &= open(filePath, false);
    ok &= close();

    return ok;
}

/**
* @brief    Erase specified file
* @param    char* filePath
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::erase(char* filePath)
{
    bool ok = true;

#ifdef USE_UCFS
    FS_ERR err = FS_ERR_NONE;
    FSEntry_Del(filePath, FS_ENTRY_TYPE_FILE, &err);
    ok &= (err == FS_ERR_NONE);
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if (ok)
    {
        // Mount logical drive
        err = f_mount(&fs, "", 0u);
        ok &= (err == (int)FR_OK);
    }

    if (ok)
    {
        // Erase file
        err = f_unlink(filePath);
        ok &= (err == (int)FR_OK);
        path[0] = '\0';
    }

    // Unmount logical drive
    if (ok)
    {
        err = f_mount(0u, _T(""), 0u);
        ok &= (err == (int)FR_OK);
    }
#endif

    return ok;
}

/**
* @brief    Directory information
* @param    char* path, fileInfo_t* fileInfo (NULL fileInfo.filename indicates no more files)
* @return   bool - true if ok, false if not ok
*/
bool DExtStorage::dir(char* path, fileInfo_t* fileInfo)
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

    if (d.obj.fs == NULL)
    {
        if (ok)
        {
            // Mount logical drive
            err = f_mount(&fs, "", 0u);
            ok &= (err == (int)FR_OK);
        }

        if (ok)
        {
            // Open the directory
            err = f_opendir(&d, path);
            ok &= (err == (int)FR_OK);
        }
    }

    if (ok)
    {
        // Read a directory item
        FILINFO fno;
        err = f_readdir(&d, &fno);
        ok &= (err == (int)FR_OK);

        if ((ok) && (fno.fname[0] != 0u))
        {
            strcpy(fileInfo->filename, fno.fname);
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
            if (ok)
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
bool DExtStorage::mkdir(char* path)
{
    bool ok = true;

#ifdef USE_UCFS
    // TODO
#endif

#ifdef USE_FATFS
    FRESULT err = FR_OK;

    if (ok)
    {
        // Mount logical drive
        err = f_mount(&fs, "", 0u);
        ok &= (err == (int)FR_OK);
    }

    if (ok)
    {
        // Create the directory
        err = f_mkdir(path);
        ok &= (err == (int)FR_OK);
    }

    // Unmount logical drive
    if (ok)
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
bool DExtStorage::readLine(char* buf, uint32_t lineLength)
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

    if (ok)
    {
        ok &= read(buf, lineLength);
        lineLength = (uint32_t)strlen(buf);
    }

    if (ok)
    {
        for (uint32_t i = 0u; i < lineLength; i++)
        {
            if ((buf[i] == TERMINATOR_CR) || (buf[i] == TERMINATOR_LF))
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
                lineLength = i;
                break;
            }
        }
    }

    ok &= foundTerminator;

    if (!ok)
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
bool DExtStorage::writeLine(char* buf)
{
    bool ok = true;
    const size_t termLength = sizeof(LINE_TERMINATION) / sizeof(char);
    const size_t lineLength = strlen(buf);
    char lineBuf[(size_t)FILE_MAX_LINE_LENGTH + termLength];
    ok &= lineLength <= (size_t)FILE_MAX_LINE_LENGTH;

    if (ok)
    {
        snprintf(lineBuf, lineLength + termLength, "%s%s", buf, LINE_TERMINATION);
        ok &= write(lineBuf);
    }

    return ok;
}

/**
* @brief Check if the direction is exist
* @param char* path
* @return   bool - true if yes, else false
*/
bool DExtStorage::isDirectoryExist(char* path)
{
    fileInfo_t fileInfo = {0};

    close();    // Close any existing open file

    return dir(path, &fileInfo);
}

/**
* @brief get the directory path from predefined directories
* @param uint16_t index , char* path, uint16_t len
* @return   bool - true if yes, else false
*/
void DExtStorage::getDirectoryPath(uint16_t index, char * path, uint16_t len)
{
    memset(path, 0x00, (uint32_t)len);
    snprintf(path, (uint32_t)len, directories[index]);
}

bool DExtStorage::deleteDirectory(char * path)
{
    bool ok = false;

#ifdef USE_UCFS
    // TODO
#endif

#ifdef USE_FATFS
    uint32_t i = 0u, j = 0u;
    FRESULT fr;
    DIR dir;
    FILINFO* fno;

    fr = f_opendir(&dir, path); /* Open the sub-directory to make it empty */
    ok = (fr == (FRESULT)FR_OK);
    if (ok)
    {
        /* Get current path length */
        for (i = 0u; path[i]; i++)
        {}              // do nothing
        path[i++] = _T('/');

        for (;;)
        {
            fr = f_readdir(&dir, fno);  /* Get a directory item */
            if ((fr != (FRESULT)FR_OK) || (!fno->fname[0]) || (ok == false))
            {
                break;          // End of directory? exit
            }
            else
            {
                j = 0u;
                do
                {
                    /* Make a path name */
                    if ((i + j) >= FILENAME_MAX_LENGTH)
                    {
                        /* Buffer over flow? */
                        fr = (FRESULT)100u;
                        break;    /* Fails with 100 when buffer overflow */
                    }
                    path[i + j] = fno->fname[j];
                }
                while (fno->fname[j++]);

                if (fno->fattrib & (BYTE)AM_DIR)
                {
                    /* Item is a sub-directory */
                    ok &= deleteDirectory(path);
                }
                else
                {
                    /* Item is a file */
                    if (f_unlink(path)!= (FRESULT)FR_OK)
                    {
                        ok = false;
                    }
                }
            }
        }

        path[--i] = 0u;  /* Restore the path name */
        f_closedir(&dir);

        if (fr == (FRESULT)FR_OK)
        {
            fr = f_unlink(path);  /* Delete the empty sub-directory */
        }
        ok = (fr == (FRESULT)FR_OK);
    }
#endif

    return ok;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm128")