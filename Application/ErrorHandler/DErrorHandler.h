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
* @file     DErrorHandler.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 April 2020
*
* @brief    The error handler header file
*/

#ifndef __DERROR_HANDLER_H
#define __DERROR_HANDLER_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <os.h>
MISRAC_ENABLE

#include "Types.h"
#include "DTask.h"

/* Types ------------------------------------------------------------------------------------------------------------*/
/* this union used to maintain status of current errors */
#pragma diag_suppress=Pm093 /* Disable MISRA C 2004 rule 18.4 */
typedef union
{
    struct
    {
        uint32_t syntaxError                : 1;
        uint32_t checksumError              : 1;
        uint32_t zeroError                  : 1;
        uint32_t calibrationOutOfRange      : 1;

        uint32_t genericApplicationBug      : 1;
        uint32_t pinError                   : 1;
        uint32_t osError                    : 1;
        uint32_t internalPressureSensor     : 1;

        uint32_t externalPressureSensor     : 1;
        uint32_t rtdInterface               : 1;
        uint32_t barometer                  : 1;
        uint32_t hartComms                  : 1;

        uint32_t uiDisplayBacklightHardKeys : 1;
        uint32_t persistentStorage          : 1;
        uint32_t externalStorage            : 1;
        uint32_t batteryMonitor             : 1;

        uint32_t duciMaster                 : 1;
        uint32_t duciSlave                  : 1;
        uint32_t bluetoothNFC               : 1;
        uint32_t unik6000                   : 1;

        uint32_t wdog                       : 1;
        uint32_t dac                        : 1;
        uint32_t mux                        : 1;
        uint32_t pga                        : 1;

        uint32_t adc                        : 1;
        uint32_t usb                        : 1;
        uint32_t uart                       : 1;
        uint32_t eeprom                     : 1;

        uint32_t i2c                        : 1;
        uint32_t gpio                       : 1;
        uint32_t spi                        : 1;
        uint32_t reserved1                  : 1;
    } bit;
    uint32_t bytes;
} error_code_t;


/* Variables -------------------------------------------------------------------------------------------------------*/

class DErrorHandler : public DTask
{
private:
    error_code_t currentError;

protected:

public:
    DErrorHandler(OS_ERR *os_error);

    void handleError(error_code_t errorCode, int32_t param = -1, bool blocking = false);
    void clearError(error_code_t errorCode);
    void clearAllErrors(void);
    void clearErrorLog(void);
    error_code_t getErrors(void);
};

#endif /* __DERROR_HANDLER_H */
