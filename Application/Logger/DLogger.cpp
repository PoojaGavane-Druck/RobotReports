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
#include "app_cfg.h"
MISRAC_ENABLE

/* Error handler instance parameter starts from 3701 to 3800 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define ER_TASK_STK_SIZE                2048u    //this is not bytes (CPU_STK is 4 bytes, so multiply by 4 for stack size in bytes)
#define ER_TASK_TIMEOUT_MS              500u
#define MS_TO_S                         1000u
#define BYTES_PER_MEGABYTE              1048576u
#define MIN_STORAGE_SPACE               1u * BYTES_PER_MEGABYTE
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
OS_TCB *erTaskTCB;
CPU_STK loggerHandlerTaskStack[APP_CFG_DATALOGGER_TASK_STK_SIZE];
#define MAX_LINE_SIZE     170u     //max number of characters on each line written to log file
char errorLogFilePath[FILENAME_MAX_LENGTH + 1u] = "\\LogFiles\\ServiceErrorLog.csv";
char serviceLogFilePath[FILENAME_MAX_LENGTH + 1u] = "\\LogFiles\\ServiceLog.csv";
char line[MAX_LINE_SIZE + 1u];

static  char errorLogFileColumnHeader[MAX_LINE_SIZE + 1u] = "Date and Time (epoch), Event Code, Event State, Value, Instance, Critical/Non Critical";
static  char serviceLogFileColumnHeader[MAX_LINE_SIZE + 1u] = "Date and Time (epoch), Set Point Value, Set Point Count, Distnace Travelled";
sLogDetails_t  gLogDetails;
sServiceLogDetails_t  gSericeLogDetails;

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

    myTaskId = eLoggerTask;
    //set up task stack pointer
    myTaskStack = &loggerHandlerTaskStack[0];

#ifdef ENABLE_STACK_MONITORING
    stackArray.uiStack.addr = (void *)myTaskStack;
    stackArray.uiStack.size = (uint32_t)(APP_CFG_DATALOGGER_TASK_STK_SIZE * 4u);
    fillStack((char *)myTaskStack, 0xDD, (size_t)(APP_CFG_DATALOGGER_TASK_STK_SIZE * 4u));
#endif

    activate(myName, (CPU_STK_SIZE)APP_CFG_DATALOGGER_TASK_STK_SIZE, (OS_PRIO)15u, (OS_MSG_QTY)80u, os_error);

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
    //createFile(errorLogFilePath);
    //createFile(serviceLogFilePath);
    createServiceLogFile();
    createErrorLogFile();

    while(DEF_TRUE)
    {

#if 0
        //wait until timeout, blocking, for a message on the task queue
        sLogDetails_t *pRecvMsg = (sLogDetails_t *)(RTOSTaskQPend((OS_TICK)0, /* Wait for 100 OS Ticks maximum. */
                                  OS_OPT_PEND_BLOCKING, /* Task will block. */
                                  &msg_size, /* Will contain size of message in bytes. */
                                  &ts, /* Timestamp is not used. */
                                  &os_error));


        sLogDetails_t  recvMsg;

        recvMsg.eventCode = pRecvMsg->eventCode;
        recvMsg.eventState = pRecvMsg->eventState;
        recvMsg.paramValue = pRecvMsg->paramValue;
        recvMsg.paramDataType = pRecvMsg->paramDataType;
        recvMsg.instance = pRecvMsg->instance;
        recvMsg.eventType = pRecvMsg->eventType;

#endif

        uLogDetails_t *pRecvMsg = (uLogDetails_t *)(RTOSTaskQPend((OS_TICK)0, /* Wait for 100 OS Ticks maximum. */
                                  OS_OPT_PEND_BLOCKING, /* Task will block. */
                                  &msg_size, /* Will contain size of message in bytes. */
                                  &ts, /* Timestamp is not used. */
                                  &os_error));


#ifdef ENABLE_STACK_MONITORING
        lastTaskRunning = myTaskId;
#endif

#ifdef TASK_HEALTH_MONITORING_IMPLEMENTED
        PV624->keepAlive(myTaskId);
#endif

        switch(os_error)
        {
        case OS_ERR_NONE:

            if(msg_size == (OS_MSG_SIZE)sizeof(uLogDetails_t))    //message size = 0 means 'rxMsg' is the message itself (always the case)
            {
                if(0u == pRecvMsg->errorLogDetails.eventCode)
                {
                    sServiceLogDetails_t  recvMsg;

                    recvMsg.eventCode = pRecvMsg->serviceLogDetails.eventCode;
                    recvMsg.setPointCount = pRecvMsg->serviceLogDetails.setPointCount;
                    recvMsg.setPointValue = pRecvMsg->serviceLogDetails.setPointValue;
                    recvMsg.distanceTravelled = pRecvMsg->serviceLogDetails.distanceTravelled;
                    recvMsg.shortReserved = 0u;
                    recvMsg.ucharReserved = 0u;
                    processSeviceMessage(&recvMsg);
                }

                else
                {
                    sErrorLogDetails_t  recvMsg;

                    recvMsg.eventCode = pRecvMsg->errorLogDetails.eventCode;
                    recvMsg.eventState = pRecvMsg->errorLogDetails.eventState;
                    recvMsg.paramValue = pRecvMsg->errorLogDetails.paramValue;
                    recvMsg.paramDataType = pRecvMsg->errorLogDetails.paramDataType;
                    recvMsg.instance = pRecvMsg->errorLogDetails.instance;
                    recvMsg.eventType = pRecvMsg->errorLogDetails.eventType;
                    processErrorMessage(&recvMsg);
                }

            }

            break;

        case OS_ERR_TIMEOUT:
            break;

        default:
#ifdef SET_ERROR_IN_ERROR_HANDLER
            PV624->handleError(E_ERROR_OS, os_error, 3701u);
#endif
            break;
        }
    }
}

/**
* @brief    processMessage - processes messages received.
* @param    plogDetails pointer log details structure
* @return   void
*/
void DLogger::processMessage(sLogDetails_t *plogDetails)
{
    bool ok = true;
    sDate_t date;
    ok &= PV624->getDate(&date);
    sTime_t instTime;
    ok &= PV624->getTime(&instTime);
    int32_t byteIndex = 0;

    if(ok)
    {
        uint32_t timeSinceEpoch;
        int32_t byteCount = 0;
        uint32_t remainingBufSize = (uint32_t)MAX_LINE_SIZE;

        convertLocalDateTimeToTimeSinceEpoch(&date, &instTime, &timeSinceEpoch);
        byteCount = snprintf(line, remainingBufSize, "%d,", timeSinceEpoch);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->eventCode);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteIndex + byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->eventState);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteIndex + byteCount;
        plogDetails->paramDataType = (eDataType_t)eDataTypeUnsignedLong;

        if((eDataType_t)eDataTypeUnsignedLong == plogDetails->paramDataType)
        {
            byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->paramValue.uintValue);
            remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        }

        else if((eDataType_t)eDataTypeFloat == plogDetails->paramDataType)
        {
            byteCount = snprintf(line + byteIndex, remainingBufSize, "%f,", plogDetails->paramValue.floatValue);
            remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        }

        else
        {
            /*Do Nothing*/
        }

        byteIndex = byteIndex + byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->instance);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteIndex + byteCount;
        snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->eventType);

        writeLine();
    }
}


/**
* @brief    processMessage - processes messages received.
* @param    plogDetails pointer log details structure
* @return   void
*/
void DLogger::processErrorMessage(sErrorLogDetails_t *plogDetails)
{
    bool ok = true;
    sDate_t date;
    ok &= PV624->getDate(&date);
    sTime_t instTime;
    ok &= PV624->getTime(&instTime);
    int32_t byteIndex = 0;

    if(ok)
    {
        uint32_t timeSinceEpoch;
        int32_t byteCount = 0;
        uint32_t remainingBufSize = (uint32_t)MAX_LINE_SIZE;

        convertLocalDateTimeToTimeSinceEpoch(&date, &instTime, &timeSinceEpoch);
        byteCount = snprintf(line, remainingBufSize, "%d,", timeSinceEpoch);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->eventCode);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteIndex + byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->eventState);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteIndex + byteCount;
        plogDetails->paramDataType = (eDataType_t)eDataTypeUnsignedLong;

        if((eDataType_t)eDataTypeUnsignedLong == plogDetails->paramDataType)
        {
            byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->paramValue.uintValue);
            remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        }

        else if((eDataType_t)eDataTypeFloat == plogDetails->paramDataType)
        {
            byteCount = snprintf(line + byteIndex, remainingBufSize, "%f,", plogDetails->paramValue.floatValue);
            remainingBufSize = remainingBufSize - (uint32_t)byteCount;
        }

        else
        {
            /*Do Nothing*/
        }

        byteIndex = byteIndex + byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->instance);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteIndex + byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->eventType);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        writeLineToSeviceErrorLog();
    }
}


/**
* @brief    processMessage - processes messages received.
* @param    plogDetails pointer log details structure
* @return   void
*/
void DLogger::processSeviceMessage(sServiceLogDetails_t *plogDetails)
{
    bool ok = true;
    sDate_t date;
    ok &= PV624->getDate(&date);
    sTime_t instTime;
    ok &= PV624->getTime(&instTime);
    int32_t byteIndex = 0;

    if(ok)
    {
        uint32_t timeSinceEpoch;
        int32_t byteCount = 0;
        uint32_t remainingBufSize = (uint32_t)MAX_LINE_SIZE;

        convertLocalDateTimeToTimeSinceEpoch(&date, &instTime, &timeSinceEpoch);
        byteCount = snprintf(line, remainingBufSize, "%d,", timeSinceEpoch);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%d,", plogDetails->setPointCount);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        byteIndex = byteIndex + byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%f,", plogDetails->setPointValue);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;


        byteIndex = byteIndex + byteCount;
        byteCount = snprintf(line + byteIndex, remainingBufSize, "%f,", plogDetails->distanceTravelled);
        remainingBufSize = remainingBufSize - (uint32_t)byteCount;

        writeLineToSeviceLog();
    }
}
/**
 *  @brief Post specified event message to this task instance
 *  @param  errorCode      specific error code
 *  @param  errorStatus    set error or clear error
 *  @param  paramValue     uint32_t type parameter value during error
 *  @param  errInstance    should be unique to help debugging
 *  @param  isFatal        0: non fatal error 1: Fatal Error
 *  @return OS_ERR
 */
OS_ERR DLogger::postEvent(eErrorCode_t errorCode,
                          uint32_t errStatus,
                          uint32_t paramValue,
                          uint16_t errInstance,
                          bool isFatal)
{
    OS_ERR os_error = OS_ERR_NONE;



    gLogDetails.eventCode = errorCode;
    gLogDetails.eventState = errStatus;
    gLogDetails.paramValue.uintValue = paramValue;
    gLogDetails.paramDataType = eDataTypeUnsignedLong;
    gLogDetails.instance = errInstance;
    gLogDetails.eventType = isFatal;

    //Post message to Error Logger Task
    RTOSTaskQPost(&myTaskTCB,
                  (void *)&gLogDetails,
                  (OS_MSG_SIZE)sizeof(sLogDetails_t),
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
 *  @param  errorCode      specific error code
 *  @param  errorStatus    set error or clear error
 *  @param  paramValue     float type parameter value during error
 *  @param  errInstance    should be unique to help debugging
 *  @param  isFatal        0: non fatal error 1: Fatal Error
 *  @return OS_ERR
 */
OS_ERR DLogger::postEvent(eErrorCode_t errorCode,
                          uint32_t errStatus,
                          float paramValue,
                          uint16_t errInstance,
                          bool isFatal)
{
    OS_ERR os_error = OS_ERR_NONE;


    gLogDetails.eventCode = errorCode;
    gLogDetails.eventState = errStatus;
    gLogDetails.paramValue.floatValue = paramValue;
    gLogDetails.paramDataType = eDataTypeFloat;
    gLogDetails.instance = errInstance;
    gLogDetails.eventType = isFatal;

    //Post message to Error Logger Task
    RTOSTaskQPost(&myTaskTCB,
                  (void *)&gLogDetails,
                  (OS_MSG_SIZE)sizeof(sLogDetails_t),
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
 *  @param  errorCode      specific error code
 *  @param  errorStatus    set error or clear error
 *  @param  paramValue     float type parameter value during error

 *  @return OS_ERR
 */
OS_ERR DLogger::postEvent(
    uint32_t setPointCount,
    float setPointValue,
    float distanceTravelled
)
{
    OS_ERR os_error = OS_ERR_NONE;
    OS_MSG_SIZE msgSize = (OS_MSG_SIZE)(0);


    msgSize = (OS_MSG_SIZE)sizeof(sServiceLogDetails_t);
    gSericeLogDetails.eventCode = 0u;
    gSericeLogDetails.setPointCount = setPointCount;
    gSericeLogDetails.setPointValue = setPointValue;
    gSericeLogDetails.distanceTravelled = distanceTravelled;

    //Post message to Error Logger Task
    RTOSTaskQPost(&myTaskTCB,
                  (void *)&gSericeLogDetails,
                  (OS_MSG_SIZE)sizeof(sServiceLogDetails_t),
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
 * @param   error code : Specific Error code
 * @param   error status  : set specific error or clear error
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
 * @param   error status  : set specific error or clear error
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
 * @brief   Log service info from application
 * @param   setPointCount  : Number set points completed
 * @param   setPointValue  : Current set point value
 * @param   distanceTravelled : distance Travelled till now
*/

bool DLogger::logServiceInfo(uint32_t setPointCount,
                             float setPointValue,
                             float distanceTravelled)
{
    return postEvent(setPointCount, setPointValue, distanceTravelled) == (OS_ERR)OS_ERR_NONE ? true : false;
}

/**
* @brief    Write line to file, only opened for as long as necessary
* @param    void
* @return   log error status
*/
eLogError_t DLogger::writeLineToSeviceErrorLog()
{
    bool ok = PV624->extStorage->open(errorLogFilePath, true);

    if(ok)
    {
        ok &= PV624->extStorage->writeLine(line);
    }

    if(ok)
    {
        ok &= PV624->extStorage->close();
    }

    return ok ? E_DATALOG_ERROR_NONE : E_DATALOG_ERROR_WRITE;
}

/**
* @brief    Write line to file, only opened for as long as necessary
* @param    void
* @return   log error status
*/
eLogError_t DLogger::writeLineToSeviceLog()
{

    bool ok = PV624->extStorage->open(serviceLogFilePath, true);

    if(ok)
    {
        ok &= PV624->extStorage->writeLine(line);
    }

    if(ok)
    {
        ok &= PV624->extStorage->close();
    }

    return ok ? E_DATALOG_ERROR_NONE : E_DATALOG_ERROR_WRITE;
}
/**
* @brief    Write line to file, only opened for as long as necessary
* @param    void
* @return   log error status
*/
eLogError_t DLogger::writeLine()
{
    createFile(NULL);

    bool ok = PV624->extStorage->open(errorLogFilePath, true);

    if(ok)
    {
        ok &= PV624->extStorage->writeLine(line);
    }

    if(ok)
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
    //eLogError_t logError = checkStorageSpace(MIN_STORAGE_SPACE);
    eLogError_t logError = E_DATALOG_ERROR_NONE;

    if(logError == (eLogError_t)E_DATALOG_ERROR_NONE)
    {
        if((filename == NULL) || (filename[0] == '\0'))
        {
            //autogenerate filename
            sDate_t d;
            sTime_t t;
            ok &= PV624->getDate(&d);
            ok &= PV624->getTime(&t);

            if(ok)
            {
                snprintf(errorLogFilePath, (size_t)FILENAME_MAX_LENGTH, "\\LogFiles\\%04d-%s.csv", d.year, convertMonthToAbbreviatedString(d.month));
            }

            else
            {
                logError = E_DATALOG_ERROR_PATH;
            }
        }

    }

    if(logError == (eLogError_t)E_DATALOG_ERROR_NONE)
    {
        //continue with whatever file name is to be used
        //create file
        ok = PV624->extStorage->open(filename, true);
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

        if(!ok)     // double check
        {
            ok = PV624->extStorage->dir("\\DataLog", &fileInfo);
        }

        if(ok)
        {
            snprintf(fn, 2u * DATALOGGING_FILENAME_MAX_LENGTH, "\\DataLog\\%s", fileInfo.filename);
            ok = PV624->extStorage->erase(fn);
            PV624->extStorage->dir("\\DataLog", &fileInfo);       // dummy read file
        }
    }
    while(ok);

    return ok;
}

/**
 * @brief   Delete a specific file.
 * @param   filename in plain ASCII
 * @return  true if success; else false
 */
bool DLogger::deleteFilename(char *filename)
{
    char fn[2u * DATALOGGING_FILENAME_MAX_LENGTH];
    fn[0] = '\0';

    PV624->extStorage->close();       // close any opened file

    snprintf(fn, 2u * DATALOGGING_FILENAME_MAX_LENGTH, "\\LogFiles\\%s.csv", filename);
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

    if(ok)
    {
        bytesFree = bytesTotal - bytesUsed;
        ok &= bytesFree > minSpace;
    }

    return ok ? E_DATALOG_ERROR_NONE : E_DATALOG_ERROR_SPACE;
}

/**
 * @brief   Delete Error Log file
 * @param   void
 * @retval  returns true if suucceeded and false if it fails
 */

bool DLogger::clearErrorLog(void)
{
    PV624->extStorage->close();       // close any opened file
    bool ok = PV624->extStorage->erase(errorLogFilePath);
    //createFile(errorLogFilePath);
    createErrorLogFile();
    return ok;

}
/**
 * @brief   Delete Service Log File
 * @param   void
 * @retval  returns true if suucceeded and false if it fails
 */

bool DLogger::clearServiceLog(void)
{
    PV624->extStorage->close();       // close any opened file
    bool ok = PV624->extStorage->erase(serviceLogFilePath);
    //createFile(serviceLogFilePath);
    createServiceLogFile();
    return ok;
}

/**
* @brief    Create Error log file if not present. ALso add column headers
* @note     Filename is always:
*                (i) saved in LogFiles folder in the root directory of the file system
*                (ii) given the '.csv' extension
* @param    filename is a char array representing the filename
*           if value is NULL (default parameter) then the name is auto-generated
* @return   log error status
*/
eLogError_t DLogger::createErrorLogFile(void)
{
    bool ok = true;


    eLogError_t logError = E_DATALOG_ERROR_NONE;

    //continue with whatever file name is to be used
    //create file
    ok = PV624->extStorage->open(errorLogFilePath, false);

    if(!ok)
    {
        PV624->extStorage->close();
        ok = PV624->extStorage->open(errorLogFilePath, true);

        if(ok)
        {
            ok = PV624->extStorage->writeLine(errorLogFileColumnHeader);
        }
    }

    ok &= PV624->extStorage->close();
    logError = ok ? E_DATALOG_ERROR_NONE : E_DATALOG_ERROR_PATH;

    return logError;
}

/**
* @brief    Create service log file if not present. ALso add column headers
* @note     Filename is always:
*                (i) saved in LogFiles folder in the root directory of the file system
*                (ii) given the '.csv' extension
* @param    filename is a char array representing the filename
*           if value is NULL (default parameter) then the name is auto-generated
* @return   log error status
*/
eLogError_t DLogger::createServiceLogFile(void)
{
    bool ok = true;

    eLogError_t logError = E_DATALOG_ERROR_NONE;

    //continue with whatever file name is to be used
    //create file
    ok = PV624->extStorage->open(serviceLogFilePath, false);

    if(!ok)
    {
        PV624->extStorage->close();
        ok = PV624->extStorage->open(serviceLogFilePath, true);

        if(ok)
        {
            ok = PV624->extStorage->writeLine(serviceLogFileColumnHeader);
        }
    }

    ok &= PV624->extStorage->close();
    logError = ok ? E_DATALOG_ERROR_NONE : E_DATALOG_ERROR_PATH;


    return logError;
}