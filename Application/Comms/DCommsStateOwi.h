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

#ifndef __DCOMMS_STATE_OWI_H
#define __DCOMMS_STATE_OWI_H
/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdbool.h>
MISRAC_ENABLE
#include "DCommsState.h"
#include "DDeviceSerial.h"
#include "DOwiParse.h"
#include "Types.h"

/* Defines and constants  -------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_STATE_OWI_READ = 0,
    E_STATE_OWI_WRITE,
    E_STATE_OWI_PRODUCTION,
    E_STATE_OWI_SIZE

} eStateOwi_t;




typedef enum : uint8_t
{
    E_DPI620G_CMD_NONE = 0,
    E_DPI620G_CMD_GET_VERSION_INFO = 0xD8,
    E_DPI620G_CMD_GET_PM620_SENSOR_INFO = 0xD9,
    E_DPI620G_CMD_SET_OPERATING_MODE = 0xDA,
    E_DPI620G_CMD_GET_OPERATING_MODE_ACCESS_LEVEL = 0xDC,
    E_DPI620G_CMD_GET_DEVICE_STATUS = 0xDE,
    E_DPI620G_CMD_GET_DEVICE_CONFIGURATION = 0xE0,
    E_DPI620G_CMD_GET_BATTERY_PARAM_INFO = 0xE2,
    E_DPI620G_CMD_GET_MEASUREMENT_AND_STATUS = 0xE3,
    E_DPI620G_CMD_GET_DATE_AND_TIME = 0xE6,
    E_DPI620G_CMD_GET_INPUT_PROCESSING = 0xE9,
    E_DPI620G_CMD_GET_PRESSURE_INFO = 0xEB,
    E_DPI620G_CMD_GET_TARE_VALUE = 0xEC,
    E_DPI620G_CMD_GET_SERVICE_LOG = 0xF1,
    E_DPI620G_CMD_GET_BAROMETER_INFO = 0xF3,  
    E_DPI620G_CMD_GET_FIRMWARE_UPGRADE_STATUS = 0xF7,
    E_DPI620G_CMD_GET_ERROR_NUMBER = 0xF8
    
   
}eDpi620gCommand_t;

typedef enum : uint8_t
{
    E_DPI620G_CMD_LEN_GET_VERSION_INFO = 0,
    E_DPI620G_CMD_LEN_GET_PM620_SENSOR_INFO = 0,
    E_DPI620G_CMD_LEN_SET_OPERATING_MODE = 1,
    E_DPI620G_CMD_LEN_GET_OPERATING_MODE_ACCESS_LEVEL = 0,
    E_DPI620G_CMD_LEN_GET_DEVICE_STATUS = 0,
    E_DPI620G_CMD_LEN_GET_DEVICE_CONFIGURATION = 0,
    E_DPI620G_CMD_LEN_GET_BATTERY_PARAM_INFO = 0,
    E_DPI620G_CMD_LEN_GET_MEASUREMENT_AND_STATUS = 0,
    E_DPI620G_CMD_LEN_GET_DATE_AND_TIME = 0,
    E_DPI620G_CMD_LEN_GET_SERVICE_LOG = 0,
    E_DPI620G_CMD_LEN_GET_BAROMETER_INFO = 0,
    E_DPI620G_CMD_LEN_GET_PRESSURE_INFO = 0,
    E_DPI620G_CMD_LEN_GET_FIRMWARE_UPGRADE_STATUS = 0,
    E_DPI620G_CMD_LEN_GET_ERROR_NUMBER = 0,
    E_DPI620G_CMD_LEN_GET_TARE_VALUE = 0,
    E_DPI620G_CMD_LEN_GET_INPUT_PROCESSING = 0,
}eDpi620gCmdLength_t;


typedef enum : uint8_t
{
    E_DPI620G_RESP_LEN_GET_VERSION_INFO = 10,
    E_DPI620G_RESP_LEN_GET_PM620_SENSOR_INFO = 24,
    E_DPI620G_RESP_LEN_SET_OPERATING_MODE = 2,
    E_DPI620G_RESP_LEN_GET_OPERATING_MODE_ACCESS_LEVEL = 2,
    E_DPI620G_RESP_LEN_GET_DEVICE_STATUS = 4,
    E_DPI620G_RESP_LEN_GET_DEVICE_CONFIGURATION = 8,
    E_DPI620G_RESP_LEN_GET_BATTERY_PARAM_INFO = 8,
    E_DPI620G_RESP_LEN_GET_MEASUREMENT_AND_STATUS = 8,
    E_DPI620G_RESP_LEN_GET_DATE_AND_TIME = 7,
    E_DPI620G_RESP_LEN_GET_SERVICE_LOG = 255,
    E_DPI620G_RESP_LEN_GET_BAROMETER_INFO = 64,
    E_DPI620G_RESP_LEN_GET_PRESSURE_INFO = 17,
    E_DPI620G_RESP_LEN_GET_FIRMWARE_UPGRADE_STATUS = 4,
    E_DPI620G_RESP_LEN_GET_ERROR_NUMBER = 1,
    E_DPI620G_RESP_LEN_GET_TARE_VALUE = 4,
    E_DPI620G_RESP_LEN_GET_INPUT_PROCESSING = 1,

}eDpi620gRespLength_t;

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateOwi :public DCommsState
{
private:
    //common commands


    static sOwiError_t fnGetVersionInfo(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetPM620SensorInfo(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnSetOperatingMode(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetOperatingModeAndAccessLevel(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetDeviceStatus(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetDeviceConfiguration(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetBatteryParamInfo(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetMeasurementAndStatus(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetDateAndTime(void *instance,uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetServiceLog(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetBarometerInfo(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetPressureInfo(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetFirmwareUpgradeStatus(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetErrorNumber(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetTareValue(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    static sOwiError_t fnGetInputProcessing(void *instance, uint8_t *paramBuf, uint32_t* paramBufSize);
    eOwiCommandType_t  getCommandType(uint8_t cmd);
    void sendAck(uint8_t cmd);
    void sendNck(void);

protected:
   
    
    DOwiParse *myParser;    

    eStateOwi_t nextState;
    
    sOwiError_t errorStatusRegister;

    

    virtual void createCommands(void);

    
   
    bool write(uint8_t *buf,uint8_t bufLen); 
    
    bool waitForCommand(uint8_t **pBuf); //TODO: Extend this to have more meaningful returned status

   
    bool query(uint8_t *cmdBuf,uint8_t cmdLen, uint8_t **pRecvBuf, uint8_t recvLen);
    
    

public:
    DCommsStateOwi(DDeviceSerial *commsMedium);

   
    
    virtual void cleanup(void);
    virtual eCommOperationMode_t run(void);
    //command handlers for this instance
    virtual sOwiError_t fnGetVersionInfo(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetPM620SensorInfo(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetOperatingMode(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetOperatingModeAndAccessLevel(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetDeviceStatus(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetDeviceConfiguration(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetBatteryParamInfo(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetMeasurementAndStatus(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetDateAndTime(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetServiceLog(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetBarometerInfo(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetPressureInfo(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetFirmwareUpgradeStatus(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetErrorNumber(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetTareValue(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnGetInputProcessing(uint8_t *paramBuf, uint32_t* paramBufSize);
  
     
};

#endif /* __DCOMMS_STATE_H */
