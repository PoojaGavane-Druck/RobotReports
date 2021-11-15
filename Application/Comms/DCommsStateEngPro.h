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
* @file     DCommsState.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The comms class header file
*/

#ifndef __DCOMMS_STATE_ENG_PROTOCL_H
#define __DCOMMS_STATE_ENG_PROTOCL_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdbool.h>
MISRAC_ENABLE

#include "DCommsState.h"
#include "DDeviceSerial.h"
#include "DEngProtocolParser.h"
#include "Types.h"
#include "DCommsMotor.h"

/* Defines and constants  -------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/




/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateEngPro : public DCommsState
{
private:
    //common commands 
    static sEngProError_t fnOpenValve1(void *instance, sEngProtocolParameter_t* parameterArray); //Control Valve
    static sEngProError_t fnOpenValve2(void* instance, sEngProtocolParameter_t* parameterArray); //Control Valve
    static sEngProError_t fnOpenValve3(void* instance, sEngProtocolParameter_t* parameterArray); //Control Valve
    static sEngProError_t fnCloseValve1(void* instance, sEngProtocolParameter_t* parameterArray); //Control Valve
    static sEngProError_t fnCloseValve2(void* instance, sEngProtocolParameter_t* parameterArray); //Control Valve
    static sEngProError_t fnCloseValve3(void* instance, sEngProtocolParameter_t* parameterArray); //Control Valve

    static sEngProError_t fnGetRE(void *instance, sEngProtocolParameter_t* parameterArray);//Controller Status
    static sEngProError_t fnGetFS(void* instance, sEngProtocolParameter_t* parameterArray); //Get Full Scale
    static sEngProError_t fnGetBR(void* instance, sEngProtocolParameter_t* parameterArray); //Get Barometer Reading    
    static sEngProError_t fnGetIV(void *instance, sEngProtocolParameter_t* parameterArray);//Get PM620 Reading
    static sEngProError_t fnGetIS(void *instance, sEngProtocolParameter_t* parameterArray);//Get Sensor Type
    static sEngProError_t fnGetSP(void *instance, sEngProtocolParameter_t*parameterArray);//Get Set Point
    static sEngProError_t fnGetCM(void* instance, sEngProtocolParameter_t* parameterArray);//Get Controller Mode
    static sEngProError_t fnGetPR(void* instance, sEngProtocolParameter_t* parameterArray); // Get Position Sensor
    static sEngProError_t fnCheckSerial(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGetPmType(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnSwitchToDuci(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnSetValveTimer(void* instance, sEngProtocolParameter_t* parameterArray); 

    /* Motor control functions */
    static sEngProError_t fnSetParameter(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGetParameter(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnRun(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnStepClock(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGoTo(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGoToDir(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGoUntil(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReleaseSw(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGoHome(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGoMark(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnResetPos(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnResetDevice(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnSoftStop(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnHardStop(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnSoftHiZ(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnHardHiZ(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGetStatus(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnMove(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadSteps(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWriteRegister(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadRegister(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWriteAcclAlpha(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWriteAcclBeta(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWriteDecelAlpha(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWriteDecelBeta(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadAcclAlpha(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadAcclBeta(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadDecelAlpha(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadDecelBeta(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnMinSpeed(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnMaxSpeed(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWdTime(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWdEnable(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnAcclTime(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnDecelTime(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnSetAbsPosition(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGetAbsPosition(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnGetVersionInfo(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnResetController(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWriteHoldCurrent(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWriteRunCurrent(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWriteAcclCurrent(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnWriteDecelCurrent(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadHoldCurrent(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadRunCurrent(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadAcclCurrent(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadDecelCurrent(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadSpeedAndCurrent(void* instance, sEngProtocolParameter_t* parameterArray);
    static sEngProError_t fnReadOpticalSensor(void* instance, sEngProtocolParameter_t* parameterArray);
    

protected:
    DEngProtocolParser *myParser;
    
    eStateDuci_t nextState;
 
    sEngProError_t errorStatusRegister;

    virtual void createCommands(void);

    bool sendResponse(sEngProtocolParameter_t *params, uint8_t numOfParams);
    bool receiveCmd(char **pStr, uint32_t numbOfByteToRead, uint32_t* numOfBytesRead);
    bool query(char *str, char **pStr);
 
    
public:
    DCommsStateEngPro(DDeviceSerial *commsMedium, DTask *task);

    virtual void initialise(void);
    virtual eStateDuci_t run(void);
   
    
    
    /* Other functions */
    sEngProError_t fnOpenValve1(sEngProtocolParameter_t* parameterArray); //Control Valve
    sEngProError_t fnOpenValve2(sEngProtocolParameter_t* parameterArray); //Control Valve
    sEngProError_t fnOpenValve3(sEngProtocolParameter_t* parameterArray); //Control Valve
    sEngProError_t fnCloseValve1(sEngProtocolParameter_t* parameterArray); //Control Valve
    sEngProError_t fnCloseValve2(sEngProtocolParameter_t* parameterArray); //Control Valve
    sEngProError_t fnCloseValve3(sEngProtocolParameter_t* parameterArray); //Control Valve

    sEngProError_t fnGetRE(sEngProtocolParameter_t* parameterArray);//Controller Status
    sEngProError_t fnGetFS(sEngProtocolParameter_t* parameterArray); //Get Full Scale
    sEngProError_t fnGetBR(sEngProtocolParameter_t* parameterArray); //Get Barometer Reading    
    sEngProError_t fnGetIV(sEngProtocolParameter_t* parameterArray);//Get PM620 Reading
    sEngProError_t fnGetIS(sEngProtocolParameter_t* parameterArray);//Get Sensor Type
    sEngProError_t fnGetSP(sEngProtocolParameter_t* parameterArray);//Get Set Point
    sEngProError_t fnGetCM(sEngProtocolParameter_t* parameterArray);//Get Controller Mode
    sEngProError_t fnGetPR(sEngProtocolParameter_t* parameterArray); // Get Position Sensor
    sEngProError_t fnCheckSerial(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGetPmType(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnSwitchToDuci(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnSetValveTimer(sEngProtocolParameter_t* parameterArray);    

    /* Motor control functions */
    sEngProError_t fnSetParameter(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGetParameter(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnRun(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnStepClock(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGoTo(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGoToDir(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGoUntil(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReleaseSw(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGoHome(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGoMark(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnResetPos(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnResetDevice(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnSoftStop(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnHardStop(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnSoftHiZ(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnHardHiZ(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGetStatus(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnMove(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadSteps(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWriteRegister(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadRegister(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWriteAcclAlpha(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWriteAcclBeta(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWriteDecelAlpha(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWriteDecelBeta(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadAcclAlpha(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadAcclBeta(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadDecelAlpha(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadDecelBeta(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnMinSpeed(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnMaxSpeed(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWdTime(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWdEnable(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnAcclTime(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnDecelTime(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnSetAbsPosition(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGetAbsPosition(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnGetVersionInfo(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnResetController(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWriteHoldCurrent(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWriteRunCurrent(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWriteAcclCurrent(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnWriteDecelCurrent(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadHoldCurrent(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadRunCurrent(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadAcclCurrent(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadDecelCurrent(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadSpeedAndCurrent(sEngProtocolParameter_t* parameterArray);
    sEngProError_t fnReadOpticalSensor(sEngProtocolParameter_t* parameterArray);    
    
};

#endif /* __DCOMMS_STATE_H */
