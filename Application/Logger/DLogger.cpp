/**
* Baker Hughes Confidential
* Copyright 2021. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DErrorLogger.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi & Simon Smith
* @date     27 April 2021
*
* @brief    The error logger source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DLogger.h"
#include "DPersistent.h"
#include "DPV624.h"
#include "Utilities.h"

MISRAC_DISABLE
#include "main.h"
#include <assert.h>
MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define ER_TASK_STK_SIZE                512u    //this is not bytes (CPU_STK is 4 bytes, so multiply by 4 for stack size in bytes)
#define ER_TASK_TIMEOUT_MS              500u
#define MS_TO_S                         1000u
#define BYTES_PER_MEGABYTE              1048576u
#define MIN_STORAGE_SPACE               1u * BYTES_PER_MEGABYTE
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
OS_TCB *erTaskTCB;
CPU_STK erHandlerTaskStack[ER_TASK_STK_SIZE];
#define MAX_LINE_SIZE     170u     //max number of characters on each line written to log file
char errorLogFilePath[FILENAME_MAX_LENGTH + 1u];
char line[MAX_LINE_SIZE + 1u];

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DLogger class constructor
 * @param   comms is the reference to the comms instance for this sensor
 * @retval  void
 */
DLogger::DLogger(OS_ERR *os_error)
    : DTask()
{
    
    myName = "Logger";

    //set up task stack pointer
    myTaskStack = &erHandlerTaskStack[0];

    // Register task for health monitoring
#ifdef WATCH_DOG_ENABLED
    registerTask();
#endif
    activate(myName, (CPU_STK_SIZE)ER_TASK_STK_SIZE, (OS_PRIO)14u, (OS_MSG_QTY)80u, os_error);

    // There is only ever one instance of the error logger
    erTaskTCB = &myTaskTCB;
}

/**
 * @brief   DLogger initialisation function
 * @param   void
 * @retval  void
 */
void DLogger::initialise(void)
{
}

/**
* @brief    Error logger task run - the top level function responsible for processing messages and maintaining its health monitoring keepalive.
* @param    void
* @return   void
*/
void DLogger::runFunction(void)
{
    OS_ERR os_error = OS_ERR_NONE;

    OS_MSG_SIZE msg_size;
    CPU_TS ts;

    //task main loop
    while(DEF_TRUE)
    {
        //wait until timeout, blocking, for a message on the task queue
        //sLogDetails_t  recvMsg = static_cast<sLogDetails_t>(reinterpret_cast<sLogDetails_t*>(OSTaskQPend((OS_TICK)ER_TASK_TIMEOUT_MS, OS_OPT_PEND_BLOCKING, &msg_size, &ts, &os_error)));
        sLogDetails_t*  pRecvMsg = static_cast<sLogDetails_t*>(OSTaskQPend((OS_TICK)ER_TASK_TIMEOUT_MS, OS_OPT_PEND_BLOCKING, &msg_size, &ts, &os_error));
        sLogDetails_t  recvMsg;
        
        recvMsg.eventCode = pRecvMsg->eventCode;
        recvMsg.eventState = pRecvMsg->eventState;
        recvMsg.paramValue = pRecvMsg->paramValue;
        recvMsg.paramDataType = pRecvMsg->paramDataType;
        recvMsg.instance = pRecvMsg->instance;
        recvMsg.eventType = pRecvMsg->eventType;
#ifdef STACK_MONITOR
        lastTaskRunning = myLastTaskId;
#endif

#ifdef WATCH_DOG_ENABLED
        keepAlive();
#endif
        switch(os_error)
        {
            case OS_ERR_NONE:
                if(msg_size == (OS_MSG_SIZE)0u)    //message size = 0 means 'rxMsg' is the message itself (always the case)
                {
                    processMessage(&recvMsg);
                }

                break;

            case OS_ERR_TIMEOUT:
                break;

            default:
#ifdef SET_ERROR_IN_ERROR_HANDLER
                PV624->handleError(E_ERROR_OS, os_error, 0u);
#endif
                break;
        }
    }
}

/**
* @brief    processMessage - processes messages received.
* @param    uint32_t rxMsgValue
* @return   void
*/
void DLogger::processMessage(sLogDetails_t *plogDetails)
{
    bool ok = true;

    sDate_t date;
    ok &= PV624->getDate(&date);

    sTime_t instTime;
    ok &= PV624->getTime(&instTime);

    if (ok)
    {
        uint32_t timeSinceEpoch;
        int32_t byteCount = (int32_t)0;
        uint32_t remainingBufSize = (uint32_t)MAX_LINE_SIZE;
        
        convertLocalDateTimeToTimeSinceEpoch(&date, &instTime, &timeSinceEpoch);
        byteCount = snprintf(line, remainingBufSize,"%d,",timeSinceEpoch);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        
        byteCount = snprintf(line, remainingBufSize,"%d,",plogDetails->eventCode);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        
        byteCount = snprintf(line, remainingBufSize,"%d,",plogDetails->eventState);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        
        if((eDataType_t)eDataTypeUnsignedLong == plogDetails->paramDataType)
        {
          byteCount = snprintf(line, remainingBufSize,"%d,",plogDetails->paramValue);
          remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        }
        else if((eDataType_t)eDataTypeFloat == plogDetails->paramDataType)
        {
          byteCount = snprintf(line, remainingBufSize,"%f,",plogDetails->paramValue);
          remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        }
        else
        {
          /*Do Nothing*/
        }
          
        byteCount = snprintf(line, remainingBufSize,"%d,",plogDetails->instance);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        
        byteCount = snprintf(line, remainingBufSize,"%d,",plogDetails->eventType);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        
        writeLine();
          
        
    }
}

/**
 *  @brief Post specified event message to this task instance
 *  @param  event enum
 *  @param  event specific parameter
 *  @param  event specific parameter
 *  @return void
 */
OS_ERR DLogger::postEvent(eErrorCode_t errorCode, 
                           uint32_t errStatus,
                           uint32_t paramValue,
                           uint16_t errInstance, 
                           bool isFatal)
{
    OS_ERR os_error = OS_ERR_NONE;
    sLogDetails_t  logDetails;
    
    
    logDetails.eventCode = errorCode;
    logDetails.eventState = errStatus;
    logDetails.paramValue.uintValue = paramValue;
    logDetails.paramDataType = eDataTypeUnsignedLong;
    logDetails.instance = errInstance;
    logDetails.eventType = isFatal;

    //Post message to Error Logger Task
    OSTaskQPost(&myTaskTCB, 
                (void *)&logDetails, 
                (OS_MSG_SIZE)0,
                (OS_OPT) OS_OPT_POST_FIFO,
                &os_error);

    MISRAC_DISABLE
    assert(os_error == static_cast<OS_ERR>(OS_ERR_NONE));
    MISRAC_ENABLE

    // Don't handle errors, as this will create a message storm and fill the log!
    return OS_ERR_NONE;
}

/**
 *  @brief Post specified event message to this task instance
 *  @param  event enum
 *  @param  event specific parameter
 *  @param  event specific parameter
 *  @return void
 */
OS_ERR DLogger::postEvent(eErrorCode_t errorCode, 
                           uint32_t errStatus,
                           float paramValue,
                           uint16_t errInstance, 
                           bool isFatal)
{
    OS_ERR os_error = OS_ERR_NONE;
    sLogDetails_t  logDetails;
    
    
    logDetails.eventCode = errorCode;
    logDetails.eventState = errStatus;
    logDetails.paramValue.floatValue = paramValue;
    logDetails.paramDataType = eDataTypeFloat;
    logDetails.instance = errInstance;
    logDetails.eventType = isFatal;

    //Post message to Error Logger Task
    OSTaskQPost(&myTaskTCB,
                (void *)&logDetails, 
                (OS_MSG_SIZE)0, 
                (OS_OPT) OS_OPT_POST_FIFO, 
                &os_error);

    MISRAC_DISABLE
    assert(os_error == static_cast<OS_ERR>(OS_ERR_NONE));
    MISRAC_ENABLE

    // Don't handle errors, as this will create a message storm and fill the log!
    return OS_ERR_NONE;
}
                
/**
 * @brief   Log error from application
 * @param   error code
 * @param   error-specific parameter, e.g. OS ERROR code
 * @param   error instance, should be unique to help debugging
 * @return  flag: true if success, else fail
 */
bool DLogger::logError(eErrorCode_t errorCode, 
                        uint32_t errStatus,
                        uint32_t paramValue,
                        uint16_t errInstance, 
                        bool isFatal)
{
    return postEvent(errorCode, errStatus, paramValue, errInstance, isFatal) == (OS_ERR)OS_ERR_NONE ? true : false;
}

/**
 * @brief   Log error from application
 * @param   error code
 * @param   error-specific parameter, e.g. OS ERROR code
 * @param   error instance, should be unique to help debugging
 * @return  flag: true if success, else fail
 */
bool DLogger::logError(eErrorCode_t errorCode, 
                        uint32_t errStatus,
                        float paramValue,
                        uint16_t errInstance, 
                        bool isFatal)
{
    return postEvent(errorCode, errStatus, paramValue, errInstance, isFatal) == (OS_ERR)OS_ERR_NONE ? true : false;
}                


/**
* @brief    Write line to data log file, only opened for as long as necessary
* @param    void
* @return   log error status
*/
eLogError_t DLogger::writeLine()
{
    bool ok = PV624->extStorage->open(errorLogFilePath, true);

    if (ok)
    {
        ok &= PV624->extStorage->writeLine(line);
    }

    if (ok)
    {
        ok &= PV624->extStorage->close();
    }

    return ok ? E_DATALOG_ERROR_NONE : E_DATALOG_ERROR_WRITE;
}
/**
 * @brief   Clear error log
 * @param   void
 * @return  void
 */
void DLogger::clear(void)
{

}


/**
* @brief    Create log file
* @note     Filename is always:
*                (i) saved in DataLog folder in the root directory of the file system
*                (ii) given the '.csv' extension
* @param    filename is a char array representing the filename
*           if value is NULL (default parameter) then the name is auto-generated
* @return   log error status
*/
eLogError_t DLogger::createFile(char *filename)
{
    bool ok = true;

    //before starting check if there is sufficient space in file system
    eLogError_t logError = checkStorageSpace(MIN_STORAGE_SPACE);

    if (logError == (eLogError_t)E_DATALOG_ERROR_NONE)
    {
        if ((filename == NULL) || (filename[0] == '\0'))
        {
            //autogenerate filename
            sDate_t d;
            sTime_t t;
            ok &= PV624->getDate(&d);
            ok &= PV624->getTime(&t);
            if (ok)
            {
                snprintf(errorLogFilePath, (size_t)FILENAME_MAX_LENGTH, "\\DataLog\\%04d-%s-%02d_%02d-%02d-%02d.csv", d.year, convertMonthToAbbreviatedString(d.month), d.day, t.hours, t.minutes, t.seconds);
            }
            else
            {
                logError = E_DATALOG_ERROR_PATH;
            }
        }
        else
        {
            snprintf(errorLogFilePath, (size_t)FILENAME_MAX_LENGTH, "\\DataLog\\%s.csv", filename);
        }
    }

    if (logError == (eLogError_t)E_DATALOG_ERROR_NONE)
    {
        //continue with whatever file name is to be used
        //create file
        ok &= PV624->extStorage->open(errorLogFilePath, true);
        ok &= PV624->extStorage->close();
        logError = ok ? E_DATALOG_ERROR_NONE : E_DATALOG_ERROR_PATH;
    }

    return logError;
}

/**
 * @brief   Delete all stored files.
 * @param   void
 * @return  true if success; else false
 */
bool DLogger::deleteAllStoredFiles(void)
{
    fileInfo_t fileInfo = {0};
    char fn[2u * DATALOGGING_FILENAME_MAX_LENGTH];
    fn[0] = '\0';
    bool ok = false;

    PV624->extStorage->close();       // close any opened file

    do
    {
        ok = PV624->extStorage->dir("\\DataLog", &fileInfo);
        if (!ok)    // double check
        {
            ok = PV624->extStorage->dir("\\DataLog", &fileInfo);
        }
        if (ok)
        {
            snprintf(fn, 2u * DATALOGGING_FILENAME_MAX_LENGTH, "\\DataLog\\%s", fileInfo.filename);
            ok = PV624->extStorage->erase(fn);
            PV624->extStorage->dir("\\DataLog", &fileInfo);       // dummy read file
        }
    }
    while (ok);

    return ok;
}

/**
 * @brief   Delete a specific file.
 * @param   filename in plain ASCII
 * @return  true if success; else false
 */
bool DLogger::deleteFilename(char* filename)
{
    char fn[2u * DATALOGGING_FILENAME_MAX_LENGTH];
    fn[0] = '\0';

    PV624->extStorage->close();       // close any opened file

    snprintf(fn, 2u * DATALOGGING_FILENAME_MAX_LENGTH, "\\DataLog\\%s.csv", filename);
    bool ok = PV624->extStorage->erase(fn);

    return ok;
}

/**
* @brief    Check storage space exceeds minimum allowed
* @param    uint32_t minSpace in bytes
* @return   log error status
*/
eLogError_t DLogger::checkStorageSpace(uint32_t minSpace)
{
    uint32_t bytesUsed;
    uint32_t bytesTotal;
    uint32_t bytesFree;
    bool ok = PV624->extStorage->getStatus(&bytesUsed, &bytesTotal);

    if (ok)
    {
        bytesFree = bytesTotal - bytesUsed;
        ok &= bytesFree > minSpace;
    }

    return ok ? E_DATALOG_ERROR_NONE : E_DATALOG_ERROR_SPACE;
}