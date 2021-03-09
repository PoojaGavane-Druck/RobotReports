/**
* BHGE Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DComms.h
* @version  1.00.00
* @author   Makarand Deshmukh
* @date     27-Feb-2021
*
* @brief    Binary Parser for Stepper Motor Source File
*/

/* Includes -----------------------------------------------------------------*/
#ifndef _DSTEPPER_MOTOR_H_
#define _DSTEPPER_MOTOR_H_

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "Types.h"
#include "DBinaryParser.h"
#include "DDeviceSerial.h"
/* Defines ----------------------------------------------------------------------------------------------------------*/

#define MAX_ACK_BYTES 4
#define MAX_NACK_BYTES 4


#define CH_CURRENT_SENSOR 0u
#define CH_OPTICAL_SENSOR 1u

#define MOTOR_DIRECTION_BACKWARDS 0x10000
#define MAX_STEP_COUNT 0xFFFF

#define VERSION_MAJOR 4
#define VERSION_MINOR 0
#define VERSION_SUBMINOR 2

/* Types ------------------------------------------------------------------------------------------------------------*/
// Inter board communication error
typedef enum : uint32_t
{
    E_IBC_ERROR_NONE = 0x00000000u,		//no error
    E_IBC_ERROR_TIMEOUT = 0x00000001u,		//function timed out
    E_IBC_ERROR_UNAVAILABLE = 0x00000002u,		//function not implemented/available
    E_IBC_ERROR_FAULT = 0x00000004u,		//function failed (unspecified cause)
    E_IBC_ERROR_COMMAND = 0x00000008u,		//error in execution of a command
    E_IBC_ERROR_COMMS = 0x00000010u,		//error in sensor comms tx/rx
    E_IBC_ERROR_HAL = 0x00000020u,		//error returned by lower-level HAL function
    E_IBC_ERROR_PARAMETER = 0x00000040u,		//paramter value error
    E_IBC_ERROR_OS = 0x00000080u,		//OS function returned error
    E_IBC_ERROR_NCK = 0x00000100u,		//Sensor Returns NCK
    
} eIbcError_t;

typedef enum
{
    STEPPER_MOTOR_CMD_None = 0,                     // 0 - 0x00
    STEPPER_MOTOR_CMD_SetParameter,                 // 1 - 0x01
    STEPPER_MOTOR_CMD_GetParameter,                 // 2 - 0x02
    STEPPER_MOTOR_CMD_Run,                          // 3 - 0x03
    STEPPER_MOTOR_CMD_StepClock,                    // 4 - 0x04
    STEPPER_MOTOR_CMD_Move,                         // 5 - 0x05
    STEPPER_MOTOR_CMD_GoTo,                         // 6 - 0x06
    STEPPER_MOTOR_CMD_GoToDir,                      // 7 - 0x07
    STEPPER_MOTOR_CMD_GoUntil,                      // 8 - 0x08
    STEPPER_MOTOR_CMD_ReleaseSW,                    // 9 - 0x09
    STEPPER_MOTOR_CMD_GoHome,                       // 10 - 0x0A
    STEPPER_MOTOR_CMD_GoMark,                       // 11 - 0x0B
    STEPPER_MOTOR_CMD_ResetPos,                     // 12 - 0x0C
    STEPPER_MOTOR_CMD_ResetDevice,                  // 13 - 0x0D
    STEPPER_MOTOR_CMD_SoftStop,                     // 14 - 0x0E
    STEPPER_MOTOR_CMD_HardStop,                     // 15 - 0x0F
    STEPPER_MOTOR_CMD_SoftHiZ,                      // 16 - 0x10
    STEPPER_MOTOR_CMD_HardHiZ,                      // 17 - 0x11
    STEPPER_MOTOR_CMD_GetStatus,                    // 18 - 0x12
    STEPPER_MOTOR_CMD_MoveContinuous,               // 19 - 0x13
    STEPPER_MOTOR_CMD_ReadStepCount,                // 20 - 0x14
    STEPPER_MOTOR_CMD_WriteRegister,                // 21 - 0x15
    STEPPER_MOTOR_CMD_ReadRegister,                 // 22 - 0x16
    STEPPER_MOTOR_CMD_WriteAcclAlpha,               // 23 - 0x17
    STEPPER_MOTOR_CMD_WriteAcclBeta,                // 24 - 0x18
    STEPPER_MOTOR_CMD_WriteDecclAlpha,              // 25 - 0x19
    STEPPER_MOTOR_CMD_WriteDecclBeta,               // 26 - 0x1A
    STEPPER_MOTOR_CMD_ReadAcclAlpha,                // 27 - 0x1B
    STEPPER_MOTOR_CMD_ReadAcclBeta,                 // 28 - 0x1C
    STEPPER_MOTOR_CMD_ReadDecclAlpha,               // 29 - 0x1D
    STEPPER_MOTOR_CMD_ReadDecclBeta,                // 30 - 0x1E
    STEPPER_MOTOR_CMD_MinimumSpeed,                 // 31 - 0x1F
    STEPPER_MOTOR_CMD_MaximumSpeed,                 // 32 - 0x20
    STEPPER_MOTOR_CMD_WatchdogTime,                 // 33 - 0x21
    STEPPER_MOTOR_CMD_WatchdogEnable,               // 34 - 0x22
    STEPPER_MOTOR_CMD_AccelerationTime,             // 35 - 0x23
    STEPPER_MOTOR_CMD_DecelerationTime,             // 36 - 0x24
    STEPPER_MOTOR_CMD_SetAbsPosition,               // 37 - 0x25
    STEPPER_MOTOR_CMD_GetAbsPosition,               // 38 - 0x26
    STEPPER_MOTOR_CMD_GetVersionInfo,               // 39 - 0x27
    STEPPER_MOTOR_CMD_ResetController,              // 40 - 0x28
    STEPPER_MOTOR_CMD_WriteHoldCurrent,             // 41 - 0x29
    STEPPER_MOTOR_CMD_WriteRunCurrent,              // 42 - 0x2A
    STEPPER_MOTOR_CMD_WriteAcclCurrent,             // 43 - 0x2B
    STEPPER_MOTOR_CMD_WriteDecelCurrent,            // 44 - 0x2C
    STEPPER_MOTOR_CMD_ReadHoldCurrent,              // 45 - 0x2D
    STEPPER_MOTOR_CMD_ReadRunCurrent,               // 46 - 0x2E
    STEPPER_MOTOR_CMD_ReadAcclCurrent,              // 47 - 0x2F
    STEPPER_MOTOR_CMD_ReadDecelCurrent,             // 48 - 0x30
    STEPPER_MOTOR_CMD_ReadSpeedAndCurrent,          // 49 - 0x31
    STEPPER_MOTOR_CMD_ReadOpticalSensor,            // 50 - 0x32
    STEPPER_MOTOR_CMD_OpenValveOne,                 // 51 - 0x33
    STEPPER_MOTOR_CMD_CloseValveOne,                // 52 - 0x34
    STEPPER_MOTOR_CMD_OpenValveTwo,                 // 53 - 0x35
    STEPPER_MOTOR_CMD_CloseValveTwo,                // 54 - 0x36
    STEPPER_MOTOR_CMD_OpenValveThree,               // 55 - 0x37
    STEPPER_MOTOR_CMD_CloseValveThree               // 56 - 0x38
}STEPPER_MOTOR_CMD_Command_t;

typedef struct
{
    float32_t acclAlpha;
    float32_t acclBeta;
    float32_t decclAlpha;
    float32_t decclBeta;
    uint32_t motorRunning;
    uint32_t steps;
    uint32_t overrunSteps;
    uint32_t decelSteps;
    float32_t decelFactor;
    uint32_t motorMoveCommand;
    uint32_t motorStopFlag;
    uint32_t motorMoveComplete;
    uint32_t motorAccelerate;
    uint32_t motorDecelerate;
    uint32_t minimumSpeed;
    uint32_t maximumSpeed;
    uint32_t decelComplete;
    uint32_t accelComplete;
    uint32_t currentDirection;
    uint32_t previousDirection;
    uint32_t directionChanged;
    int32_t absoluteStepCounter;
    int32_t previousSteps;
    int32_t stepsBeforeDecel;
    int32_t stepsAfterDecel;
    int32_t stepsCounter;

    uint32_t motorSpeed;
    float32_t motorCurrent;
    uint32_t opticalSensorVal;

    float32_t currHold;
    float32_t currRun;
    float32_t currAccl;
    float32_t currDecel;

}sMotorParms_t;
/* Variables --------------------------------------------------------------------------------------------------------*/
class DStepperMotor
{
    DDeviceSerial* myComms;
    DBinaryParser* myParser;

    uint8_t* myTxBuffer;
    uint32_t myTxBufferSize;
    sMotorParms_t motorParams;

    uint32_t commandTimeoutPeriod; //time in (ms) to wait for a response to a command
    uint32_t watchdogTimeout;
    sVersion_t version;
    void createCommands(void);
    eIbcError_t sendCommand(uint8_t cmd, uint8_t* cmdData, uint8_t cmdDataLength);


   
        sError_t fnSetAcclAlpha(void *parent, sParameter_t* ptrParam);
        sError_t fnSetAcclBeta(void *parent, sParameter_t* ptrParam);
        sError_t fnSetDecclAlpha(void *parent, sParameter_t* ptrParam);
        sError_t fnSetDecclBeta(void *parent, sParameter_t* ptrParam);
        sError_t fnGetAcclAlpha(void *parent, sParameter_t* ptrParam);
        sError_t fnGetAcclBeta(void *parent, sParameter_t* ptrParam);
        sError_t fnGetDecclAlpha(void *parent, sParameter_t* ptrParam);
        sError_t fnGetDecclBeta(void *parent, sParameter_t* ptrParam);
        sError_t fnSetMoveContinuous(void *parent, sParameter_t* ptrParam);
        sError_t fnGetStepCount(void *parent, sParameter_t* ptrParam);
        sError_t fnSetMinimumSpeed(void *parent, sParameter_t* ptrParam);
        sError_t fnSetMaximumSpeed(void *parent, sParameter_t* ptrParam);
        sError_t fnSetWatchdogTimeout(void *parent, sParameter_t* ptrParam);
        sError_t fnSetAccelerationTime(void *parent, sParameter_t* ptrParam);
        sError_t fnSetAbsolutePosition(void *parent, sParameter_t* ptrParam);
        sError_t fnGetAbsolutePosition(void *parent, sParameter_t* ptrParam);
        sError_t fnGetVersionInfo(void *parent, sParameter_t* ptrParam);
        sError_t fnGetHoldCurrent(void *parent, sParameter_t* ptrParam);
        sError_t fnGetRunCurrent(void *parent, sParameter_t* ptrParam);
        sError_t fnGetAcclCurrent(void *parent, sParameter_t* ptrParam);
        sError_t fnGetDecelCurrent(void *parent, sParameter_t* ptrParam);
        sError_t fnSetHoldCurrent(void *parent, sParameter_t* ptrParam);
        sError_t fnSetRunCurrent(void *parent, sParameter_t* ptrParam);
        sError_t fnSetAcclCurrent(void *parent, sParameter_t* ptrParam);
        sError_t fnSetDecelCurrent(void *parent, sParameter_t* ptrParam);
        sError_t fnGetSpeedAndCurrent(void *parent, sParameter_t* ptrParam);
        

    public:    
        DStepperMotor();
        ~DStepperMotor();

        eIbcError_t writeAcclAlpha(float32_t newAcclAlpha);
        eIbcError_t writeAcclBeta(float32_t newAcclBeta);
        eIbcError_t writeDecclAlpha(float32_t newDcclAlpha);
        eIbcError_t writeDecclBeta(float32_t newDcclBeta);
        eIbcError_t readAcclAlpha(float32_t* acclAlpha);
        eIbcError_t readAcclBeta(float32_t* acclBeta);
        eIbcError_t readDecclAlpha(float32_t* decclAlpha);
        eIbcError_t readDecclBeta(float32_t* decclBeta);
        eIbcError_t writeMoveContinuous(uint32_t ptrParam);
        eIbcError_t readStepCount(uint32_t* stepCount);
        eIbcError_t writeMinimumSpeed(uint32_t ptrParam);
        eIbcError_t writeMaximumSpeed(uint32_t newSpeed);
        eIbcError_t writeWatchdogTimeout(uint32_t newTimeOut);
        eIbcError_t writeAccelerationTime(uint32_t newTime);
        eIbcError_t writeAbsolutePosition(uint32_t newPosition);
        eIbcError_t readAbsolutePosition(uint32_t* absPosition);
        eIbcError_t readVersionInfo(sVersion_t *ver);
        eIbcError_t readHoldCurrent(float32_t* holdCurrent);
        eIbcError_t readRunCurrent(float32_t* runCurrent);
        eIbcError_t readAcclCurrent(float32_t* acclCurrent);
        eIbcError_t readDecelCurrent(float32_t* decelCur);
        eIbcError_t writeHoldCurrent(float32_t holdCurrent);
        eIbcError_t writeRunCurrent(float32_t runCurrent);
        eIbcError_t writeAcclCurrent(float32_t acclCurrent);
        eIbcError_t writeDecelCurrent(float32_t deccelCurrent);
        eIbcError_t readSpeedAndCurrent(uint32_t *speed, float32_t current);
        eIbcError_t readOpticalSensor(uint32_t optSensor);


        sError_t fnSetAcclAlpha(sParameter_t* ptrParam);
        sError_t fnSetAcclBeta(sParameter_t* ptrParam);
        sError_t fnSetDecclAlpha(sParameter_t* ptrParam);
        sError_t fnSetDecclBeta(sParameter_t* ptrParam);
        sError_t fnGetAcclAlpha(sParameter_t* ptrParam);
        sError_t fnGetAcclBeta(sParameter_t* ptrParam);
        sError_t fnGetDecclAlpha(sParameter_t* ptrParam);
        sError_t fnGetDecclBeta(sParameter_t* ptrParam);
        sError_t fnSetMoveContinuous(sParameter_t* ptrParam);
        sError_t fnGetStepCount(sParameter_t* ptrParam);
        sError_t fnSetMinimumSpeed(sParameter_t* ptrParam);
        sError_t fnSetMaximumSpeed(sParameter_t* ptrParam);
        sError_t fnSetWatchdogTimeout(sParameter_t* ptrParam);
        sError_t fnSetAccelerationTime(sParameter_t* ptrParam);
        sError_t fnSetAbsolutePosition(sParameter_t* ptrParam);
        sError_t fnGetAbsolutePosition(sParameter_t* ptrParam);
        sError_t fnGetVersionInfo(sParameter_t* ptrParam);
        sError_t fnGetHoldCurrent(sParameter_t* ptrParam);
        sError_t fnGetRunCurrent(sParameter_t* ptrParam);
        sError_t fnGetAcclCurrent(sParameter_t* ptrParam);
        sError_t fnGetDecelCurrent(sParameter_t* ptrParam);
        sError_t fnSetHoldCurrent(sParameter_t* ptrParam);
        sError_t fnSetRunCurrent(sParameter_t* ptrParam);
        sError_t fnSetAcclCurrent(sParameter_t* ptrParam);
        sError_t fnSetDecelCurrent(sParameter_t* ptrParam);
        sError_t fnGetSpeedAndCurrent(sParameter_t* ptrParam);
        
};
#endif