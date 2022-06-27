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

/* Includes -----------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdbool.h>
MISRAC_ENABLE

#include "DCommsState.h"
#include "Types.h"
#include "DCommsMotor.h"

/* Defines and constants ----------------------------------------------------*/
#define DEFAULT_ACCL_ALPHA 0.98
#define DEFAULT_ACCL_BETA 2.86
#define DEFAULT_DECEL_ALPHA 1.02
#define DEFAULT_DECEL_BETA 0.0

#define DEFAULT_STEP_SIZE 4 // Quarter stepping 1/4 step size

#define DEFAULT_MINIMUM_SPEED 1000 // steps / second

#define DEFAULT_CURRENT 2.0

#define SPI_TIMEOUT_FW_UPGRADE ((uint32_t)2000)
#define RX_LENGTH_FW_UPGRADE   ((uint8_t)9)
/* Types --------------------------------------------------------------------*/
typedef enum
{
    eMotorErrorNone = 0,
    eMotorError
} eMotorError_t;

/* Variables ----------------------------------------------------------------*/

class DStepperMotor
{
private:
    /* Acceleration and deceleration parameters */
    float acclAlpha;
    float acclBeta;
    float decelAlpha;
    float decelBeta;

    /* Step size */
    uint32_t motorStepSize;


    /* Motor parameters */
    int32_t totalStepCount;
    int32_t homePosition;
    int32_t stepCount;
    uint32_t minimumSpeed;

    sVersion_t  appVersion;
    sVersion_t  bootVersion;

#ifdef DIFFERENT_CURRENTS
    float runCurrent;
    float holdCurrent;
    float acclCurrent;
    float decelCurrent;
#endif

#ifndef DIFFERENT_CURRENTS
    /* Motor current
    Only one current is now used for run, acclereration and deceleration */
    float motorCurrent;
#endif

    void setOperationConstants(void);
    void setCurrents(void);
    eMotorError_t setStepSize(void);

protected:

public:
    DStepperMotor();
    ~DStepperMotor();
    DCommsMotor *commsMotor;

    eMotorError_t writeAcclAlpha(void);
    eMotorError_t writeAcclBeta(void);
    eMotorError_t writeDecelAlpha(void);
    eMotorError_t writeDecelBeta(void);
    eMotorError_t readAcclAlpha(void);
    eMotorError_t readAcclBeta(void);
    eMotorError_t readDecclAlpha(void);
    eMotorError_t readDecclBeta(void);
    eMotorError_t move(int32_t ptrParam, int32_t *completedCount);
    eMotorError_t readStepCount(void);
    eMotorError_t writeMinimumSpeed(void);
    eMotorError_t writeMaximumSpeed(void);
    eMotorError_t readVersionInfo(void);
    eMotorError_t readVersionInfo(sVersion_t *appVer, sVersion_t *bootVer);

#ifdef DIFFERENT_CURRENTS
    eMotorError_t readHoldCurrent(float32_t *holdCurrent);
    eMotorError_t readRunCurrent(float32_t *runCurrent);
    eMotorError_t readAcclCurrent(float32_t *acclCurrent);
    eMotorError_t readDecelCurrent(float32_t *decelCur);
    eMotorError_t writeHoldCurrent(float32_t holdCurrent);
    eMotorError_t writeRunCurrent(float32_t runCurrent);
    eMotorError_t writeAcclCurrent(float32_t acclCurrent);
    eMotorError_t writeDecelCurrent(float32_t deccelCurrent);
#endif

#ifndef DIFFERENT_CURRENTS
    eMotorError_t readCurrent(void);
    eMotorError_t writeCurrent(float current);

#endif
    eMotorError_t readSpeedAndCurrent(uint32_t *speed, float32_t *current);
    eMotorError_t sendCommand(uint8_t cmd, uint8_t *txData, uint8_t *rxData);
    eMotorError_t secondaryUcFwUpgrade(uint8_t *txData, uint8_t dataLength, uint8_t *response);
    eMotorError_t secondaryUcFwUpgradeCmd(uint32_t fileSize, uint8_t *responseAck);
    void getAppVersion(sVersion_t *ver);
    void getBootVersion(sVersion_t *ver);

};

#endif /* DStepperMotor.h*/
