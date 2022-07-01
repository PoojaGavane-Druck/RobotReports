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
* @brief    Stepper Motor Class Source File
*/

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <rtos.h>
MISRAC_ENABLE
#include "DStepperMotor.h"
#include "DBinaryParser.h"
#include "DStepperMotor.h"
#include "DPV624.h"

/* Defines and constants ----------------------------------------------------*/
#define MAX_CURRENT 2.0f // 2 Amps
#define RUN_CURRENT 2.0f // 2 Amps
#define HOLD_CURRENT 2.0f // 2 Amps
#define ACCL_CURRENT 2.0f // 2 Amps
#define DECEL_CURRENT 2.0f // 2 Amps

/* Types --------------------------------------------------------------------*/

/* Global Variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef hspi2;
/* File Statics -------------------------------------------------------------*/

/* User Code ----------------------------------------------------------------*/

/**
* @brief    DStepperMotor class constructor
* @param    void
* @retval   void
*/
DStepperMotor::DStepperMotor()
{
    /* Acceleration and deceleration parameters */
    commsMotor = new DCommsMotor(&hspi2);

    acclAlpha = (float32_t)(DEFAULT_ACCL_ALPHA);
    acclBeta = (float32_t)(DEFAULT_ACCL_BETA);
    decelAlpha = (float32_t)(DEFAULT_DECEL_ALPHA);
    decelBeta = (float32_t)(DEFAULT_DECEL_BETA);

    /* Step size */
    motorStepSize = 0x0Au; // This is from the

    /* Motor parameters */
    totalStepCount = 0;
    homePosition = 0;
    stepCount = 0;
    minimumSpeed = (uint32_t)(DEFAULT_MINIMUM_SPEED);

#ifdef DIFFERENT_CURRENTS
    float32_t runCurrent;
    float32_t holdCurrent;
    float32_t acclCurrent;
    float32_t decelCurrent;
#endif

#ifndef DIFFERENT_CURRENTS
    /* Motor current
    Only one current is now used for run, acclereration and deceleration */
    motorCurrent = (float32_t)(DEFAULT_CURRENT);
#endif
    /* Set the required motor currents and operation constants */
    //etOperationConstants();
    //setCurrents();
    //setStepSize();



}

/**
* @brief    DStepperMotor class destructor
* @param    void
* @retval   void
*/
DStepperMotor::~DStepperMotor()
{

}

/**
* @brief    Set the steps size for the motor
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::setStepSize(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdSetParameter);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    paramWrite.byteArray[0] = (uint8_t)(motorStepSize);
    paramWrite.byteArray[3] = 0x16u; // this is the register on the L6472

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

    if(0x3Cu == paramRead.byteArray[1])
    {

    }

    return error;
}

/**
* @brief    Sets the operating constants for accl and decel
* @param    void
* @retval   void
*/
void DStepperMotor::setOperationConstants(void)
{
    //writeAcclAlpha();
    //writeAcclBeta();
    //writeDecelAlpha();
    //writeDecelBeta();
    readAcclAlpha();
    readAcclBeta();
    readDecclAlpha();
    readDecclBeta();
}

/**
* @brief    Sets the motor current values
* @param    void
* @retval   void
*/
void DStepperMotor::setCurrents(void)
{
#ifdef DIFFERENT_CURRENTS

#endif

#ifndef DIFFERENT_CURRENTS
    writeCurrent((float32_t)(DEFAULT_CURRENT));
#endif
}

#pragma diag_suppress=Pm137

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::writeAcclAlpha(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdWriteAcclAlpha);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    paramWrite.uiValue = (uint32_t)(acclAlpha * 1000.0f);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

    if(0x3Cu == paramRead.byteArray[1])
    {

    }

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::writeAcclBeta(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdWriteAcclBeta);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    paramWrite.uiValue = (uint32_t)(acclBeta * 1000.0f);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}


/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::writeDecelAlpha(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdWriteDecelAlpha);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    paramWrite.uiValue = (uint32_t)(decelAlpha * 1000.0f);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::writeDecelBeta(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdWriteDecelBeta);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    paramWrite.uiValue = (uint32_t)(decelBeta * 1000.0f);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

#pragma diag_default=Pm137
/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readAcclAlpha(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdReadAcclAlpha);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readAcclBeta(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdReadAcclBeta);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readDecclAlpha(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdReadDecelAlpha);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readDecclBeta(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdReadDecelBeta);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

#pragma diag_suppress=Pm128
#pragma diag_suppress=Pm136
/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::move(int32_t ptrParam, int32_t *completedCount)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdMoveContinuous);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    paramWrite.iValue = ptrParam;

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

    //*completedCount = paramRead.iValue;
    *completedCount = (int32_t)((uint32_t)(paramRead.byteArray[0]) << 24u |
                                (uint32_t)(paramRead.byteArray[1]) << 16u |
                                (uint32_t)(paramRead.byteArray[2]) << 8u |
                                (uint32_t)(paramRead.byteArray[3]));

    return error;
}
#pragma diag_default=Pm128
#pragma diag_default=Pm136
/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readStepCount(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdReadStepCount);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readDkNumbers(uint32_t *appDk, uint32_t *bootDk)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdGetDkApp);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    if((appDk != NULL) && (bootDk != NULL))
    {
        commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

        *appDk = paramRead.uiValue;

        paramWrite.uiValue = 0u;
        paramRead.uiValue = 0u;
        command = (uint8_t)(eCmdGetDkBoot);

        commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

        *bootDk = paramRead.uiValue;
    }

    else
    {
        error = eMotorError;
    }

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readVersionInfo(sVersion_t *appVer, sVersion_t *bootVer)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdGetVersionApp);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    if((appVer != NULL) && (bootVer != NULL))
    {
        commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

        appVer->major = (uint32_t) paramRead.byteArray[0];
        appVer->minor = (uint32_t) paramRead.byteArray[1];
        appVer->build = (uint32_t) paramRead.byteArray[2] << 8;
        appVer->build = (uint32_t) paramRead.byteArray[3];

        command = (uint8_t)(eCmdGetVersionBoot);

        paramWrite.uiValue = 0u;
        paramRead.uiValue = 0u;

        commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

        bootVer->major = (uint32_t) paramRead.byteArray[0];
        bootVer->minor = (uint32_t) paramRead.byteArray[1];
        bootVer->build = (uint32_t) paramRead.byteArray[2] << 8;
        bootVer->build = (uint32_t) paramRead.byteArray[3];
    }

    else
    {
        error = eMotorError;
    }

    return error;
}

#ifdef DIFFERENT_CURRENTS
/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readHoldCurrent(float32_t *holdCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readRunCurrent(float32_t *runCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readAcclCurrent(float32_t *acclCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readDecelCurrent(float32_t *decelCur)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::writeHoldCurrent(float32_t holdCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::writeRunCurrent(float32_t runCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::writeAcclCurrent(float32_t acclCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::writeDecelCurrent(float32_t deccelCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}
#endif

#ifndef DIFFERENT_CURRENTS
/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readCurrent(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdReadHoldCurrent);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::writeCurrent(float current)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCmdWriteHoldCurrent);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.floatValue = current;
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}
#endif

#pragma diag_suppress=Pm136
/**
* @brief    Sets the motor current
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::readSpeedAndCurrent(uint32_t *speed, float32_t *current)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = 0x31u;
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

    *speed = (uint32_t)((uint32_t)(paramRead.byteArray[0]) << 8u |
                        (uint32_t)(paramRead.byteArray[1]));
    *current = (float32_t)((uint32_t)(paramRead.byteArray[2]) << 8u |
                           (uint32_t)(paramRead.byteArray[3]));

    return error;
}

/**
* @brief    Sends a command and gets response from motor, used in engg mode
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::sendCommand(uint8_t cmd, uint8_t *txData, uint8_t *rxData)
{
    eMotorError_t error = eMotorErrorNone;

    commsMotor->query(cmd, txData, rxData);

    return error;
}

/**
* @brief    Gives secondary application DK number
* @param    sVersion_t * pointer variable to return application version information
* @retval   void
*/
void DStepperMotor::getAppDk(uint32_t *dk)
{
    *dk = appDkNum;
}
/**
* @brief    Gives secondary bootloader DK number
* @param    sVersion_t * pointer variable to return bootlaoder version information
* @retval   void
*/
void DStepperMotor::getBootDk(uint32_t *dk)
{
    *dk = bootDkNum;
}

/**
* @brief    gives application version information
* @param    sVersion_t * pointer variable to return application version information
* @retval   void
*/
void DStepperMotor::getAppVersion(sVersion_t *ver)
{
    if(ver != NULL)
    {
        ver->all =  appVersion.all;
    }
}
/**
* @brief    gives bootloader version information
* @param    sVersion_t * pointer variable to return bootlaoder version information
* @retval   void
*/
void DStepperMotor::getBootVersion(sVersion_t *ver)
{
    if(ver != NULL)
    {
        ver->all =  bootVersion.all;
    }
}

/**
* @brief    read version information from secondary uC
* @param    sVersion_t * pointer variable to return bootlaoder version information
* @retval   void
*/
eMotorError_t DStepperMotor::readVersionInfo(void)
{
    eMotorError_t error = eMotorErrorNone;
    readDkNumbers(&appDkNum, &bootDkNum);
    readVersionInfo(&appVersion, &bootVersion);
    return(error);
}
/**
* @brief    Sends a command and Fw Upgrade to Secondary uC
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::secondaryUcFwUpgrade(uint8_t *txData, uint8_t dataLength, uint8_t *response)
{
    eMotorError_t error = eMotorErrorNone;
    sParameter_t paramRead;
    paramRead.uiValue = (uint32_t)(0);
    uint8_t CommandFwUpgrade = eCmdFwUpgrade;

    //TODO: Add check of all pointers for NULL value
    if(((uint8_t)NULL != response) && ((uint8_t)NULL != txData) && ((uint8_t)NULL != dataLength))
    {
        commsMotor->query(CommandFwUpgrade, txData, dataLength, paramRead.byteArray, RX_LENGTH_FW_UPGRADE, SPI_TIMEOUT_FW_UPGRADE);

        if((uint8_t)(ACK_FW_UPGRADE) == paramRead.byteArray[1])
        {
            *response = (uint8_t)ACK_FW_UPGRADE;
        }

        else
        {
            *response = (uint8_t)NACK_FW_UPGRADE;
        }
    }

    else
    {
        error = eMotorError;
    }


    return error;
}

/**
* @brief    Sends a Fw Upgrade command to Secondary uC to switch the state of Secondary uC
* @param    void
* @retval   void
*/
eMotorError_t DStepperMotor::secondaryUcFwUpgradeCmd(uint32_t fileSize, uint8_t *responseAck)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = eCmdFwUpgradeStateChange; // This changes the state of Secondary uC application code to fw upgrade
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = fileSize;
    paramRead.uiValue = (uint32_t)(0);

    if(((uint8_t)NULL != responseAck) && ((uint32_t)NULL != fileSize))
    {
        commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

        if((uint8_t)(ACK_FW_UPGRADE) == paramRead.byteArray[1])
        {
            *responseAck = (uint8_t)ACK_FW_UPGRADE;
        }

        else
        {
            *responseAck = (uint8_t)NACK_FW_UPGRADE;
        }
    }

    else
    {
        error = eMotorError;
    }

    return error;
}
#pragma diag_default=Pm136