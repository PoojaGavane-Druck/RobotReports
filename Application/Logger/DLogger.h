/**
* Baker Hughes Confidential
* Copyright 2021.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DErrorLogger.h
* @version  1.00.00
* @author   Harvinder Bhuhi & Simon Smith
* @date     27 April 2021
*
* @brief    The error logger header file
*/

#ifndef __DERROR_LOGGER_H
#define __DERROR_LOGGER_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DTask.h"
#include "Types.h"
/* Defines ----------------------------------------------------------------------------------------------------------*/
//the log area is bytes (ie, 64 pages x 32 bytes)
//first page reserved for header info, 63 pages for entries
#define ER_VALID_LOG_SIGNATURE 0x45525253u                 //Hex pattern representing ASCII text "ERRS"
#define ER_LOG_MIN_INDEX       0u                          //index to first entry location (not same as the earliest)
#define ER_LOG_MAX_ENTRIES     100u                        //maximum number of entries
#define ER_LOG_MAX_INDEX       (ER_LOG_MAX_ENTRIES - 1u)   //index of location of last entry (not same as the most recent)
#define ER_LOG_ENTRY_SIZE      8u                          //size of each entry is 8 bytes

/* Types ------------------------------------------------------------------------------------------------------------*/
/* Defines ---------------------------------------------------------------------------------------------------------*/
#define DATALOGGING_FILENAME_MAX_LENGTH         20u     // excluding path and file extension
#define DATALOGGING_FILENAMEPATH_MAX_LENGTH     256u    // including path and file extension

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_DATALOG_ERROR_NONE,
    E_DATALOG_ERROR_MODE,
    E_DATALOG_ERROR_PATH,
    E_DATALOG_ERROR_FILENAME,
    E_DATALOG_ERROR_WRITE,
    E_DATALOG_ERROR_TIMER,
    E_DATALOG_ERROR_SPACE,
    E_DATALOG_ERROR_FILESIZE_WARNING,
    E_DATALOG_ERROR_FILESIZE_ERROR,
    E_DATALOG_ERROR_FILESIZE_LARGE
} eLogError_t;

#define DATALOGHEADER_FILENAME_LENGTH   30u
#define DATALOGHEADER_DATE_LENGTH       11u
#define DATALOGHEADER_TIME_LENGTH       13u
#define DATALOGHEADER_FUNCTION_LENGTH   25u
#define DATALOGHEADER_DATA_POINT        11u
typedef enum
{
    E_ERRORLOGGER_OTHER_ENTRY,
    E_ERRORLOGGER_EARLIEST_ENTRY,
    E_ERRORLOGGER_LATEST_ENTRY,
    E_ERRORLOGGER_NEXT_ENTRY
} eErrorLogEntryQuery_t;



/* structure of a logged error entry in persistent storage */
typedef struct
{
    uint32_t  eventCode;
    uint32_t  eventState;
    uParameter_t paramValue;
    eDataType_t  paramDataType;
    uint16_t     instance;
    uint8_t     eventType;

} sLogDetails_t;

typedef struct
{
    uint32_t  eventCode;
    uint32_t  eventState;
    uParameter_t paramValue;
    eDataType_t  paramDataType;
    uint16_t     instance;
    uint8_t     eventType;

} sErrorLogDetails_t;

typedef struct
{
    uint32_t  eventCode;
    uint32_t  setPointCount;
    float     setPointValue;
    float     distanceTravelled;
    uint16_t  shortReserved;
    uint8_t   ucharReserved;

} sServiceLogDetails_t;

typedef union
{
    sErrorLogDetails_t errorLogDetails;
    sServiceLogDetails_t serviceLogDetails;
} uLogDetails_t;

typedef struct
{
    uint32_t        timestamp;
    sLogDetails_t   logDetails;

} sLogEntry_t;



/* Variables -------------------------------------------------------------------------------------------------------*/
class DLogger : public DTask
{
private:
    void processMessage(sLogDetails_t *plogDetails);
    void processErrorMessage(sErrorLogDetails_t *plogDetails);
    void processSeviceMessage(sServiceLogDetails_t *plogDetails);

    OS_ERR postEvent(eErrorCode_t errorCode,
                     uint32_t errStatus,
                     uint32_t paramValue,
                     uint16_t errInstance,
                     bool isFatal);

    OS_ERR postEvent(eErrorCode_t errorCode,
                     uint32_t errStatus,
                     float paramValue,
                     uint16_t errInstance,
                     bool isFatal);

    eLogError_t writeLine();
    eLogError_t writeLineToSeviceErrorLog();
    eLogError_t writeLineToSeviceLog();

    OS_ERR postEvent(uint8_t event, uint16_t param16, uint8_t param8);

    OS_ERR postEvent(
        uint32_t setPointCount,
        float setPointValue,
        float distanceTravelled
    );


    eLogError_t checkStorageSpace(uint32_t minSpace);

protected:

public:
    DLogger(OS_ERR *os_error);

    virtual void initialise(void);
    virtual void runFunction(void);
    eLogError_t createFile(char *filename);


    bool logError(eErrorCode_t errorCode,
                  uint32_t errStatus,
                  uint32_t paramValue,
                  uint16_t errInstance,
                  bool isFatal);

    bool logError(eErrorCode_t errorCode,
                  uint32_t errStatus,
                  float paramValue,
                  uint16_t errInstance,
                  bool isFatal);

    bool logServiceInfo(uint32_t setPointCount,
                        float setPointValue,
                        float distanceTravelled);

    bool deleteFilename(char *filename);
    bool deleteAllStoredFiles(void);
    void clear(void);

    bool clearErrorLog(void);
    bool clearServiceLog(void);
    eLogError_t createErrorLogFile(void);
    eLogError_t createServiceLogFile(void);

};

#endif /* __DERROR_LOGGER_H */
