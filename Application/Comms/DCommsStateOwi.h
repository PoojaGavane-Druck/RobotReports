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
    E_FUNC_NONE = 0,
    E_FUNC_GAUGE_PRESSURE,
    E_FUNC_ABSOLUTE_PRESSURE,
    E_FUNC_ATMOSPHERIC_PRESSURE
} eMeasureFunction_t;

typedef enum : uint8_t
{
    /*
    E_DPI620G_CMD_NONE = 0,
    E_DPI620G_CMD_GET_VERSION_INFO = 0x18,
    E_DPI620G_CMD_SET_FUNCTION = 0x39,
    E_DPI620G_CMD_GET_PM620_SENSOR_INFO = 0x19,
    E_DPI620G_CMD_SET_OPERATING_MODE = 0x1A,
    E_DPI620G_CMD_GET_OPERATING_MODE_ACCESS_LEVEL = 0x1C,
    E_DPI620G_CMD_GET_DEVICE_STATUS = 0x1E,
    E_DPI620G_CMD_GET_DEVICE_CONFIGURATION = 0x20,
    E_DPI620G_CMD_GET_BATTERY_PARAM_INFO = 0x22,
    E_DPI620G_CMD_GET_MEASUREMENT_AND_STATUS = 0x23,
    E_DPI620G_CMD_GET_DATE_AND_TIME = 0x26,
    E_DPI620G_CMD_GET_INPUT_PROCESSING = 0x29,
    E_DPI620G_CMD_GET_PRESSURE_INFO = 0x2B,
    E_DPI620G_CMD_GET_TARE_VALUE = 0x2C,
    E_DPI620G_CMD_GET_SERVICE_LOG = 0x31,
    E_DPI620G_CMD_GET_BAROMETER_INFO = 0x33,  
    E_DPI620G_CMD_GET_FIRMWARE_UPGRADE_STATUS = 0x37,
    E_DPI620G_CMD_GET_ERROR_NUMBER = 0x38       
    */
    E_DPI620G_CMD_NONE = 0,
    E_DPI620G_CMD_GET_VERSION_INFO = 0x18,
    E_DPI620G_CMD_GET_PM620_SENSOR_INFO = 0x19,
    E_DPI620G_CMD_SET_OPERATING_MODE = 0x1A,
    E_DPI620G_CMD_SET_AUTHENTICATE = 0x1B,
    E_DPI620G_CMD_GET_OPERATING_MODE_ACCESS_LEVEL = 0x1C,
    E_DPI620G_CMD_SET_ERASE_CONFIG_CALIB_DATA = 0x1D,
    E_DPI620G_CMD_GET_INSTRUMENT_ERROR = 0x1E,
    E_DPI620G_CMD_SET_INSTRUMENT_CONFIGURATION = 0x1F,
    E_DPI620G_CMD_GET_INSTRUMENT_CONFIGURATION = 0x20,
    E_DPI620G_CMD_SET_OPTIONAL_FEATURES = 0x21,
    E_DPI620G_CMD_GET_BATTERY_PARAM_INFO = 0x22,
    E_DPI620G_CMD_GET_MEASUREMENT_AND_STATUS = 0x23,
    E_DPI620G_CMD_GET_BAROMETER_READING = 0x24,
    E_DPI620G_CMD_SET_DATE_AND_TIME = 0x25,
    E_DPI620G_CMD_GET_DATE_AND_TIME = 0x26,
    E_DPI620G_CMD_SET_BARO_CALIB_PARAMS = 0x27,
    E_DPI620G_CMD_SET_INPUT_PROCESSING = 0x28,
    E_DPI620G_CMD_GET_INPUT_PROCESSING = 0x29,
    E_DPI620G_CMD_SET_VENT_PRESSURE = 0x2A,
    E_DPI620G_CMD_SET_TARE_VALUE = 0x2B,
    E_DPI620G_CMD_GET_TARE_VALUE = 0x2C,
    E_DPI620G_CMD_GET_CALIBRATION_PARAMS = 0x2D,
    E_DPI620G_CMD_SET_CALIBRATION_STATE = 0x2E,
    E_DPI620G_CMD_SET_CALIBRATION_PARAMS = 0x2F,
    E_DPI620G_CMD_SET_CALIBRATION_POINT = 0x30,
    E_DPI620G_CMD_GET_SERVICE_LOG = 0x31,
    E_DPI620G_CMD_SET_CLEAR_LOG = 0x32,
    E_DPI620G_CMD_GET_BAROMETER_INFO = 0x33,
    E_DPI620G_CMD_SET_SELECT_DEVICE_FOR_CAL = 0x34,
    E_DPI620G_CMD_GET_LATEST_FIRMWARE_VERSION = 0x35,
    E_DPI620G_CMD_SET_START_FIRMWARE_UPGRADE = 0x36,
    E_DPI620G_CMD_GET_FIRMWARE_UPGRADE_STATUS = 0x37,
    E_DPI620G_CMD_GET_ERROR_NUMBER = 0x38,
    E_DPI620G_CMD_SET_FUNCTION = 0x39
  
}eDpi620gCommand_t;

typedef enum : uint8_t
{
    /*
    E_DPI620G_CMD_LEN_GET_VERSION_INFO = 0,
    E_DPI620G_CMD_LEN_SET_FUNCTION = 1,
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
    */
    E_DPI620G_CMD_LEN_GET_VERSION_INFO = 0,
    E_DPI620G_CMD_LEN_GET_PM620_SENSOR_INFO = 0,
    E_DPI620G_CMD_LEN_SET_OPERATING_MODE = 1,
    E_DPI620G_CMD_LEN_SET_AUTHENTICATE = 1,
    E_DPI620G_CMD_LEN_GET_OPERATING_MODE_ACCESS_LEVEL = 1,
    E_DPI620G_CMD_LEN_SET_ERASE_CONFIG_CALIB_DATA = 1,
    E_DPI620G_CMD_LEN_GET_INSTRUMENT_ERROR = 0,
    E_DPI620G_CMD_LEN_SET_INSTRUMENT_CONFIGURATION = 5,
    E_DPI620G_CMD_LEN_GET_INSTRUMENT_CONFIGURATION = 0,
    E_DPI620G_CMD_LEN_SET_OPTIONAL_FEATURES = 5,
    E_DPI620G_CMD_LEN_GET_BATTERY_PARAM_INFO = 0,
    E_DPI620G_CMD_LEN_GET_MEASUREMENT_AND_STATUS = 0,
    E_DPI620G_CMD_LEN_GET_BAROMETER_READING = 0,
    E_DPI620G_CMD_LEN_SET_DATE_AND_TIME = 7,
    E_DPI620G_CMD_LEN_GET_DATE_AND_TIME = 0,
    E_DPI620G_CMD_LEN_SET_BARO_CALIB_PARAMS = 4,
    E_DPI620G_CMD_LEN_SET_INPUT_PROCESSING = 2,
    E_DPI620G_CMD_LEN_GET_INPUT_PROCESSING = 0,
    E_DPI620G_CMD_LEN_SET_VENT_PRESSURE = 0,
    E_DPI620G_CMD_LEN_SET_TARE_VALUE = 0,
    E_DPI620G_CMD_LEN_GET_TARE_VALUE = 0,
    E_DPI620G_CMD_LEN_GET_CALIBRATION_PARAMS = 0,
    E_DPI620G_CMD_LEN_SET_CALIBRATION_STATE = 1,
    E_DPI620G_CMD_LEN_SET_CALIBRATION_PARAMS = 1,
    E_DPI620G_CMD_LEN_SET_CALIBRATION_POINT = 1,
    E_DPI620G_CMD_LEN_GET_SERVICE_LOG = 0,
    E_DPI620G_CMD_LEN_SET_CLEAR_LOG = 1,
    E_DPI620G_CMD_LEN_GET_BAROMETER_INFO = 0,
    E_DPI620G_CMD_LEN_SET_SELECT_DEVICE_FOR_CAL = 1,
    E_DPI620G_CMD_LEN_GET_LATEST_FIRMWARE_VERSION = 0,
    E_DPI620G_CMD_LEN_SET_START_FIRMWARE_UPGRADE = 0,
    E_DPI620G_CMD_LEN_GET_FIRMWARE_UPGRADE_STATUS = 0,
    E_DPI620G_CMD_LEN_GET_ERROR_NUMBER = 0,
    E_DPI620G_CMD_LEN_SET_FUNCTION = 1

    
}eDpi620gCmdLength_t;


typedef enum : uint8_t
{
    /*
    E_DPI620G_RESP_LEN_GET_VERSION_INFO = 10,
    E_DPI620G_RESP_LEN_SET_FUNCTION = 1,
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
    */
    E_DPI620G_RESP_LEN_GET_VERSION_INFO = 10,
    E_DPI620G_RESP_LEN_GET_PM620_SENSOR_INFO = 24,
    E_DPI620G_RESP_LEN_SET_OPERATING_MODE = 1,
    E_DPI620G_RESP_LEN_SET_AUTHENTICATE = 1,
    E_DPI620G_RESP_LEN_GET_OPERATING_MODE_ACCESS_LEVEL = 2,
    E_DPI620G_RESP_LEN_SET_ERASE_CONFIG_CALIB_DATA = 1,
    E_DPI620G_RESP_LEN_GET_INSTRUMENT_ERROR = 4,
    E_DPI620G_RESP_LEN_SET_INSTRUMENT_CONFIGURATION = 1,
    E_DPI620G_RESP_LEN_GET_INSTRUMENT_CONFIGURATION = 20,
    E_DPI620G_RESP_LEN_SET_OPTIONAL_FEATURES = 1,
    E_DPI620G_RESP_LEN_GET_BATTERY_PARAM_INFO = 8,
    E_DPI620G_RESP_LEN_GET_MEASUREMENT_AND_STATUS = 8,
    E_DPI620G_RESP_LEN_GET_BAROMETER_READING = 4,
    E_DPI620G_RESP_LEN_SET_DATE_AND_TIME = 1,
    E_DPI620G_RESP_LEN_GET_DATE_AND_TIME = 7,
    E_DPI620G_RESP_LEN_SET_BARO_CALIB_PARAMS = 1,
    E_DPI620G_RESP_LEN_SET_INPUT_PROCESSING = 1,
    E_DPI620G_RESP_LEN_GET_INPUT_PROCESSING = 1,
    E_DPI620G_RESP_LEN_SET_VENT_PRESSURE = 1,
    E_DPI620G_RESP_LEN_SET_TARE_VALUE = 1,
    E_DPI620G_RESP_LEN_GET_TARE_VALUE = 4,
    E_DPI620G_RESP_LEN_GET_CALIBRATION_PARAMS = 86,
    E_DPI620G_RESP_LEN_SET_CALIBRATION_STATE = 1,
    E_DPI620G_RESP_LEN_SET_CALIBRATION_PARAMS = 5,
    E_DPI620G_RESP_LEN_SET_CALIBRATION_POINT = 5,
    E_DPI620G_RESP_LEN_GET_SERVICE_LOG = 100,
    E_DPI620G_RESP_LEN_SET_CLEAR_LOG = 1,
    E_DPI620G_RESP_LEN_GET_BAROMETER_INFO = 2,
    E_DPI620G_RESP_LEN_SET_SELECT_DEVICE_FOR_CAL = 1,
    E_DPI620G_RESP_LEN_GET_LATEST_FIRMWARE_VERSION = 5,
    E_DPI620G_RESP_LEN_SET_START_FIRMWARE_UPGRADE = 1,
    E_DPI620G_RESP_LEN_GET_FIRMWARE_UPGRADE_STATUS = 1,
    E_DPI620G_RESP_LEN_GET_ERROR_NUMBER = 4,
    E_DPI620G_RESP_LEN_SET_FUNCTION = 1,



}eDpi620gRespLength_t;

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateOwi :public DCommsState
{
private:
    //common commands

    /* Get type commands */
    static sOwiError_t fnGetVersionInfo(void *instance, 
                                        uint8_t *paramBuf, 
                                        uint32_t* paramBufSize);    
    
    static sOwiError_t fnGetPM620SensorInfo(void *instance, 
                                            uint8_t *paramBuf, 
                                            uint32_t* paramBufSize);    
    
    static sOwiError_t fnGetOperatingModeAndAccessLevel(void *instance, 
                                                        uint8_t *paramBuf, 
                                                        uint32_t* paramBufSize);
    
    static sOwiError_t fnGetDeviceStatus(void *instance, 
                                         uint8_t *paramBuf, 
                                         uint32_t* paramBufSize);
    
    static sOwiError_t fnGetDeviceConfiguration(void *instance, 
                                                uint8_t *paramBuf, 
                                                uint32_t* paramBufSize);
    
    static sOwiError_t fnGetBatteryParamInfo(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize);
    
    static sOwiError_t fnGetMeasurementAndStatus(void *instance, 
                                                 uint8_t *paramBuf, 
                                                 uint32_t* paramBufSize);
    
    static sOwiError_t fnGetDateAndTime(void *instance,
                                        uint8_t *paramBuf, 
                                        uint32_t* paramBufSize);
    
    static sOwiError_t fnGetServiceLog(void *instance, 
                                       uint8_t *paramBuf, 
                                       uint32_t* paramBufSize);
    
    static sOwiError_t fnGetBarometerInfo(void *instance, 
                                          uint8_t *paramBuf, 
                                          uint32_t* paramBufSize);
    
    static sOwiError_t fnGetPressureInfo(void *instance, 
                                         uint8_t *paramBuf, 
                                         uint32_t* paramBufSize);
    
    static sOwiError_t fnGetFirmwareUpgradeStatus(void *instance, 
                                                  uint8_t *paramBuf, 
                                                  uint32_t* paramBufSize);
    
    static sOwiError_t fnGetErrorNumber(void *instance, 
                                        uint8_t *paramBuf, 
                                        uint32_t* paramBufSize);
    
    static sOwiError_t fnGetTareValue(void *instance, 
                                      uint8_t *paramBuf, 
                                      uint32_t* paramBufSize);
    
    static sOwiError_t fnGetInputProcessing(void *instance, 
                                            uint8_t *paramBuf, 
                                            uint32_t* paramBufSize);
    
    static sOwiError_t fnGetCalibrationParams(void *instance, 
                                              uint8_t *paramBuf, 
                                              uint32_t* paramBufSize); 
    
    static sOwiError_t fnGetLatestFirmwareVersion(void *instance, 
                                                  uint8_t *paramBuf, 
                                                  uint32_t* paramBufSize); 
    
    /* Set type commands */
    static sOwiError_t fnSetFunction(void *instance, 
                                     uint8_t *paramBuf, 
                                     uint32_t* paramBufSize);
    
    static sOwiError_t fnSetOperatingMode(void *instance, 
                                          uint8_t *paramBuf, 
                                          uint32_t* paramBufSize);
    
    static sOwiError_t fnSetAuthenticate(void *instance, 
                                         uint8_t *paramBuf, 
                                         uint32_t* paramBufSize);
    
    static sOwiError_t fnSetEraseConfigAndCalibData(void *instance, 
                                                    uint8_t *paramBuf, 
                                                    uint32_t* paramBufSize);
    
    static sOwiError_t fnSetOptionalFeatures(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize);
    
    static sOwiError_t fnSetDateAndTime(void *instance, 
                                        uint8_t *paramBuf, 
                                        uint32_t* paramBufSize);
    
    static sOwiError_t fnSetBarometerCalibrationParams(void *instance, 
                                                       uint8_t *paramBuf, 
                                                       uint32_t* paramBufSize);
    
    static sOwiError_t fnSetInputProcessing(void *instance, 
                                            uint8_t *paramBuf, 
                                            uint32_t* paramBufSize);
    
    static sOwiError_t fnSetVentPressure(void *instance, 
                                         uint8_t *paramBuf, 
                                         uint32_t* paramBufSize);
    
    static sOwiError_t fnSetTareValue(void *instance, 
                                      uint8_t *paramBuf, 
                                      uint32_t* paramBufSize);
    
    static sOwiError_t fnSetCalibrationState(void *instance, 
                                             uint8_t *paramBuf, 
                                             uint32_t* paramBufSize);
    
    static sOwiError_t fnSetCalibrationParams(void *instance, 
                                              uint8_t *paramBuf, 
                                              uint32_t* paramBufSize);
    
    static sOwiError_t fnSetCalibrationPoints(void *instance, 
                                              uint8_t *paramBuf, 
                                              uint32_t* paramBufSize);
    
    static sOwiError_t fnSetClearServiceLog(void *instance, 
                                            uint8_t *paramBuf, 
                                            uint32_t* paramBufSize);
    
    static sOwiError_t fnSetSelectDeviceForCalibration(void *instance, 
                                                       uint8_t *paramBuf, 
                                                       uint32_t* paramBufSize);
    
    static sOwiError_t fnSetStartFirmwareUpgrade(void *instance, 
                                                 uint8_t *paramBuf, 
                                                 uint32_t* paramBufSize);    
    
    eOwiCommandType_t  getCommandType(uint8_t cmd);
    void sendAck(uint8_t cmd);
    void sendNck(void);

protected:
   
    
    DOwiParse *myParser;    

    eStateOwi_t nextState;
    
    sOwiError_t errorStatusRegister;
    
    virtual void createCommands(void);        
    bool waitForCommand(uint8_t **pBuf); //TODO: Extend this to have more meaningful returned status   
    bool query(uint8_t *cmdBuf,uint8_t cmdLen, uint8_t **pRecvBuf, uint8_t recvLen);
        
public:
    DCommsStateOwi(DDeviceSerial *commsMedium);
      
    virtual void cleanup(void);
    virtual eCommOperationMode_t run(void);
    //command handlers for this instance
    // Get type functions 
    virtual sOwiError_t fnGetVersionInfo(uint8_t *paramBuf, uint32_t* paramBufSize);    
    virtual sOwiError_t fnGetPM620SensorInfo(uint8_t *paramBuf, uint32_t* paramBufSize);    
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
    virtual sOwiError_t fnGetCalibrationParams(uint8_t *paramBuf, uint32_t* paramBufSize); 
    virtual sOwiError_t fnGetLatestFirmwareVersion(uint8_t *paramBuf, uint32_t* paramBufSize); 

    // Set type functions
    virtual sOwiError_t fnSetOperatingMode(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetFunction(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetAuthenticate(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetEraseConfigAndCalibData(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetOptionalFeatures(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetDateAndTime(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetBarometerCalibrationParams(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetInputProcessing(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetVentPressure(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetTareValue(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetCalibrationState(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetCalibrationParams(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetCalibrationPoints(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetClearServiceLog(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetSelectDeviceForCalibration(uint8_t *paramBuf, uint32_t* paramBufSize);
    virtual sOwiError_t fnSetStartFirmwareUpgrade(uint8_t *paramBuf, uint32_t* paramBufSize);             
};

#endif /* __DCOMMS_STATE_H */
