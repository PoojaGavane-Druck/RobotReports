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
* @file     DStepperMotor.c
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     20-09-2021
*
* @brief    Stepper Motor Class Header File
*/

#ifndef __DSTEPPERMOTOR_H__
#define __DSTEPPERMOTOR_H__

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdbool.h>
MISRAC_ENABLE

#include "DCommsState.h"
#include "Types.h"
#include "DCommsMotor.h"

/* Defines and constants --------------------------------------------------------------------------------------------*/
#define SPI_TIMEOUT_FW_UPGRADE ((uint32_t)2000)
#define RX_LENGTH_FW_UPGRADE   ((uint8_t)11)

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    eMotorErrorNone = 0,
    eMotorError
} eMotorError_t;

typedef enum
{
    eStepperErrorNone = 0,
    eStepperErrorOverCurrent,
    eStepperErrorThermalShutdown,
    eStepperErrorThermalWarning,
    eStepperErrorUnderVoltage
} eStepperError_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DStepperMotor
{
private:
    uint32_t appDkNum;
    uint32_t bootDkNum;
    sVersion_t  appVersion;
    sVersion_t  bootVersion;
protected:

public:
    DStepperMotor();        // Constructor
    ~DStepperMotor();       // Destructor

    DCommsMotor *commsMotor;    // Object to the motor comms manager

    eStepperError_t stepperErrors;

    eMotorError_t readVersionInfo(void);
    eMotorError_t move(int32_t steps, int32_t *completedCount);
    eMotorError_t readDkNumbers(uint32_t *appDk, uint32_t *bootDk);
    eMotorError_t readVersionInfo(sVersion_t *appVer, sVersion_t *bootVer);
    eMotorError_t sendEnggCommand(uint8_t cmd, uint8_t *txData, uint8_t *rxData);
    eMotorError_t secondaryUcFwUpgradeCmd(uint32_t fileSize, uint8_t *responseAck);
    eMotorError_t secondaryUcFwUpgrade(uint8_t *txData, uint8_t dataLength, uint8_t *response);

    void getAppDk(uint32_t *dk);
    void getBootDk(uint32_t *dk);
    void getAppVersion(sVersion_t *ver);
    void getBootVersion(sVersion_t *ver);
};

#endif /* DStepperMotor.h*/
