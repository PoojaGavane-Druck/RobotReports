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

/* Error handler instance parameter starts from 5201 to 5300 */
/* Defines and constants ----------------------------------------------------*/

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
    // Motor comms object
    commsMotor = new DCommsMotor(&hspi2);

    stepperErrors = eStepperErrorNone;
}

/**
* @brief    DStepperMotor class destructor
* @param    void
* @retval   void
*/
DStepperMotor::~DStepperMotor()
{

}

#pragma diag_suppress=Pm137

#pragma diag_suppress=Pm128
#pragma diag_suppress=Pm136
/**
* @brief    Moves the motor by the set number of steps, also reads the previously executed steps from the motor
            controller
* @param    int32_t steps - Steps to be executed
            int32_t *completedCount - Steps completed on the previous command
* @retval   eMotorError_t - None if passed, Error if failed
*/
eMotorError_t DStepperMotor::move(int32_t steps, int32_t *completedCount)
{
    eMotorError_t error = eMotorError;
    uint8_t command = (uint8_t)(eCmdMoveContinuous);

    sError_t commError;
    sParameter_t paramWrite;
    sParameter_t paramRead;

    commError.value = 0u;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    paramWrite.iValue = steps;

    deviceStatus_t errors;
    errors.bytes = 0u;

    // Check if there is an optical board error, if yes, do not run motor
    errors = PV624->errorHandler->getDeviceStatus();

    if(0u == errors.bit.opticalBoardFail)
    {
        commError = commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray, (uint32_t *)(&stepperErrors));

        if(0u == commError.value)
        {
            *completedCount = (int32_t)((uint32_t)(paramRead.byteArray[0]) << 24u |
                                        (uint32_t)(paramRead.byteArray[1]) << 16u |
                                        (uint32_t)(paramRead.byteArray[2]) << 8u |
                                        (uint32_t)(paramRead.byteArray[3]));
            error = eMotorErrorNone;
        }
    }

    return error;
}
#pragma diag_default=Pm128
#pragma diag_default=Pm136

/**
* @brief    Reads the application and bootloader DK numbers from the stepper micro controller
* @param    uint32_t *appDk - pointer holding the value of the application DK number
            uint32_t *bootDk - pointer holding the value of the bootloader DK number
* @retval   eMotorError_t - None if passed, Error if failed
*/
eMotorError_t DStepperMotor::readDkNumbers(uint32_t *appDk, uint32_t *bootDk)
{
    eMotorError_t error = eMotorError;
    uint8_t command = (uint8_t)(eCmdGetDkApp);

    sError_t commError;
    sParameter_t paramWrite;
    sParameter_t paramRead;

    commError.value = 0u;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    if((appDk != NULL) && (bootDk != NULL))
    {
        if(0u == commError.value)
        {
            commError = commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray, (uint32_t *)(&stepperErrors));

            *appDk = paramRead.uiValue;

            paramWrite.uiValue = 0u;
            paramRead.uiValue = 0u;
            command = (uint8_t)(eCmdGetDkBoot);

            commError = commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray, (uint32_t *)(&stepperErrors));

            if(0u == commError.value)
            {
                *bootDk = paramRead.uiValue;
                error = eMotorErrorNone;
            }
        }
    }

    return error;
}

/**
* @brief    Reads the application and bootloader version numbers from the stepper micro controller
* @param    sVersion_t *appVer - pointer to the application version of the stepper micro controller
            sVersion_t *bootVer - pointer to the bootloader version of the stepper micro controller
* @retval   eMotorError_t - None if passed, Error if failed
*/
eMotorError_t DStepperMotor::readVersionInfo(sVersion_t *appVer, sVersion_t *bootVer)
{
    eMotorError_t error = eMotorError;
    uint8_t command = (uint8_t)(eCmdGetVersionApp);

    sError_t commError;
    sParameter_t paramWrite;
    sParameter_t paramRead;

    commError.value = 0u;
    paramWrite.uiValue = 0u;
    paramRead.uiValue = 0u;

    if((appVer != NULL) && (bootVer != NULL))
    {
        commError = commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray, (uint32_t *)(&stepperErrors));

        if(0u == commError.value)
        {
            appVer->major = (uint32_t) paramRead.byteArray[0];
            appVer->minor = (uint32_t) paramRead.byteArray[1];
            appVer->build = (uint32_t) paramRead.byteArray[2] << 8;
            appVer->build = (uint32_t) paramRead.byteArray[3];

            command = (uint8_t)(eCmdGetVersionBoot);

            paramWrite.uiValue = 0u;
            paramRead.uiValue = 0u;

            commError = commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray, (uint32_t *)(&stepperErrors));

            if(0u == commError.value)
            {
                bootVer->major = (uint32_t) paramRead.byteArray[0];
                bootVer->minor = (uint32_t) paramRead.byteArray[1];
                bootVer->build = (uint32_t) paramRead.byteArray[2] << 8;
                bootVer->build = (uint32_t) paramRead.byteArray[3];
                error = eMotorErrorNone;
            }
        }
    }

    return error;
}

#pragma diag_suppress=Pm136

/**
* @brief    Sends a command and gets response from motor, used in engg mode
* @param    uint8_t cmd - command to be sent to the stepper driver
            uint8_t *txData - pointer to the data to be sent to the stepper driver
            uint8_t *rxData - pointer to the data to be received
* @retval   eMotorError_t - None if passed, Error if failed
*/
eMotorError_t DStepperMotor::sendEnggCommand(uint8_t cmd, uint8_t *txData, uint8_t *rxData)
{
    eMotorError_t error = eMotorErrorNone;

    sError_t commError;
    commError.value = 0u;

    commError = commsMotor->query(cmd, txData, rxData, (uint32_t *)(&stepperErrors));

    if(0u != commError.value)
    {
        error = eMotorError;
    }

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
* @retval   eMotorError_t - None if passed, Error if failed
*/
eMotorError_t DStepperMotor::readVersionInfo(void)
{
    eMotorError_t error = eMotorErrorNone;

    readDkNumbers(&appDkNum, &bootDkNum);
    readVersionInfo(&appVersion, &bootVersion);

    return(error);
}
/**
* @brief    Sends a command for FW upgrade to the secondary micro controller
* @param    uint8_t *txData - pointer to the data to be sent to the stepper driver
            uint8_t dataLength - length of the data to be sent
            uint8_t *response - pointer to the data to be received
* @retval   eMotorError_t - None if passed, Error if failed
*/
eMotorError_t DStepperMotor::secondaryUcFwUpgrade(uint8_t *txData, uint8_t dataLength, uint8_t *response)
{
    eMotorError_t error = eMotorError;

    sError_t commError;
    sParameter_t paramRead;
    paramRead.uiValue = (uint32_t)(0);
    uint8_t CommandFwUpgrade = eCmdFwUpgrade;

    commError.value = 0u;

    //TODO: Add check of all pointers for NULL value
    if(((uint8_t)NULL != response) && ((uint8_t)NULL != txData) && ((uint8_t)NULL != dataLength))
    {
        commError = commsMotor->query(CommandFwUpgrade,
                                      txData,
                                      dataLength,
                                      paramRead.byteArray,
                                      RX_LENGTH_FW_UPGRADE,
                                      SPI_TIMEOUT_FW_UPGRADE,
                                      (uint32_t *)(&stepperErrors));

        if(0u == commError.value)
        {
            if((uint8_t)(ACK_FW_UPGRADE) == paramRead.byteArray[1])
            {
                *response = (uint8_t)ACK_FW_UPGRADE;
                error = eMotorErrorNone;
            }

            else
            {
                *response = (uint8_t)NACK_FW_UPGRADE;
            }
        }
    }

    return error;
}

/**
* @brief    Sends a Fw Upgrade command to Secondary uC to switch the state of Secondary uC
* @param    uint32_t fileSize - Size of the firmware file to be sent
            uint8_t *responseAck - acknowledgement response from the secondary micro
* @retval   eMotorError_t - None if passed, Error if failed
*/
eMotorError_t DStepperMotor::secondaryUcFwUpgradeCmd(uint32_t fileSize, uint8_t *responseAck)
{
    eMotorError_t error = eMotorError;
    uint8_t command = eCmdFwUpgradeStateChange; // This changes the state of Secondary uC application code to fw upgrade
    sError_t commError;
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = fileSize;
    paramRead.uiValue = (uint32_t)(0);

    commError.value = 0u;

    if(((uint8_t)NULL != responseAck) && ((uint32_t)NULL != fileSize))
    {
        commError = commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray, (uint32_t *)(&stepperErrors));

        if(0u == commError.value)
        {
            if((uint8_t)(ACK_FW_UPGRADE) == paramRead.byteArray[1])
            {
                *responseAck = (uint8_t)ACK_FW_UPGRADE;
                error = eMotorErrorNone;
            }

            else
            {
                *responseAck = (uint8_t)NACK_FW_UPGRADE;
            }
        }
    }

    return error;
}
#pragma diag_default=Pm136