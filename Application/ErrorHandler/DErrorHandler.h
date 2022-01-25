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


/* Types ------------------------------------------------------------------------------------------------------------*/
/* this union used to maintain status of current errors */
#pragma diag_suppress=Pm093 /* Disable MISRA C 2004 rule 18.4 */
typedef union
{
    struct
    {

        uint32_t lowReferenceSensorVoltage          : 1;
        uint32_t referenceSensorCommFail            : 1;
        uint32_t barometerSensorFail                : 1;
        uint32_t stepperControllerFail              : 1;

        uint32_t motorVoltageFail                   : 1;
        uint32_t stepperDriverFail                  : 1;
        uint32_t vlaveFail                          : 1;
        uint32_t referenceSensorOutOfCal            : 1;

        uint32_t barometerOutOfCal                  : 1;
        uint32_t persistentMemoryFail               : 1;
        uint32_t batteryWarningLevel                : 1;
        uint32_t batteryCriticalLevel               : 1;

        uint32_t extFlashCorrupt                    : 1;
        uint32_t extFlashWriteFailure               : 1;
        uint32_t onboardFlashFail                   : 1;
        uint32_t overTemperature                    : 1;

        uint32_t OpticalSensorFail                  : 1;
        uint32_t barometerSensorMode                : 1;
        uint32_t barometerSensorCalStatus           : 1;
        uint32_t barometerNotEnabled                : 1;

        uint32_t smBusBatteryComFailed              : 1;
        uint32_t smBusBatChargerComFailed           : 1;
        uint32_t chargingStatus                     : 1;
        uint32_t osError                            : 1;

        uint32_t Reserved8                          : 1;
        uint32_t Reserved7                          : 1;
        uint32_t Reserved6                          : 1;
        uint32_t Reserved5                          : 1;

        uint32_t Reserved4                          : 1;
        uint32_t Reserved3                          : 1;
        uint32_t Reserved2                          : 1;
        uint32_t Reserved1                          : 1;
    } bit;
    uint32_t bytes;
} deviceStatus_t;

typedef enum
{
    eClearError = 0,
    eSetError
} eErrorStatus_t;


/* Variables -------------------------------------------------------------------------------------------------------*/

class DErrorHandler
{
private:
    deviceStatus_t deviceStatus;


protected:

public:
    DErrorHandler(OS_ERR *os_error);

    void handleError(eErrorCode_t errorCode,
                     eErrorStatus_t errStatus,
                     uint32_t paramValue,
                     uint16_t errInstance,
                     bool isFatal);

    void handleError(eErrorCode_t errorCode,
                     eErrorStatus_t errStatus,
                     float paramValue,
                     uint16_t errInstance,
                     bool isFatal);

    void updateDeviceStatus(eErrorCode_t errorCode,
                            eErrorStatus_t errStatus);
    void clearAllErrors(void);
    void clearErrorLog(void);
    deviceStatus_t getDeviceStatus(void);
};

#endif /* __DERROR_HANDLER_H */
