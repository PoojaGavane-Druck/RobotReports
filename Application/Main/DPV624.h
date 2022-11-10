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
* @file     DPV624.h
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     12 April 2020
*
* @brief    The PV624 instrument class header file
*/
//*********************************************************************************************************************
#ifndef __DPV624_H
#define __DPV624_H

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DInstrument.h"
#include "DKeyHandler.h"
#include "DUserInterface.h"
#include "DErrorHandler.h"
#include "DCommsUSB.h"
#include "DCommsBluetooth.h"
#include "DCommsOwi.h"
#include "DCommsMotor.h"
#include "DExtStorage.h"
#include "DPersistent.h"
#include "DPowerManager.h"
#include "DStepperMotor.h"
#include "DValve.h"
#include "leds.h"
#include "DLogger.h"
#include "cBL652.h"

/* Defines-----------------------------------------------------------------------------------------------------------*/
#define SECONDARY

#define RESET_SECONDARY_MICRO     {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);} \
                                    {HAL_Delay(10u);} \
                                    {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);}


#define RESET_PIN 0x04000000u
#define RESET_POR 0x08000000u
#define RESET_SW 0x10000000u
#define RESET_IWDG 0x20000000u
#define RESET_WWDG 0x40000000u
#define RESET_LP 0x80000000u
#define RESET_REM_MASK (RESET_PIN | RESET_POR | RESET_SW | RESET_IWDG | RESET_WWDG | RESET_LP)

/* Types ------------------------------------------------------------------------------------------------------------*/
#ifdef ENABLE_STACK_MONITORING
typedef struct
{
    void *addr;
    uint32_t size;

} sStackInfo_t;

typedef struct
{
    sStackInfo_t keyStack;
    sStackInfo_t coulombStack;
    sStackInfo_t funcStack;
    sStackInfo_t slotStack;
    sStackInfo_t commsStack;
    sStackInfo_t touchStack;
    sStackInfo_t gfxStack;
    sStackInfo_t uiStack;

} sStackMonitor_t;

typedef enum
{
    BL_STATE_DISABLE = 0,
    BL_STATE_DEV_AT_CMDS = 1,
    BL_STATE_DTM = 2,
    BL_STATE_RUN_DTM = 3,
    BL_STATE_RUN_STAND_BY = 4,
    BL_STATE_RUN_ADV_IN_PROGRESS = 5,
    BL_STATE_RUN_CONNECTION_ESTABLISHED = 6,
    BL_STATE_RUN_DEEP_SLEEP = 7,
    BL_STATE_MAX = 7

} eBL652State_t;
extern int gfxLock;
extern int gfxLock2;
extern int lastTaskRunning;
extern sStackMonitor_t stackArray;
void fillStack(char *addr, short value, size_t bytes);
#endif

#define PM620_TERPS_APP_DK_NUMBER 472u

class DPV624
{

private:
    ePinMode_t myPinMode;
    uint32_t keepAliveCount[eNumberOfTasks];
    uint32_t keepAlivePreviousCount[eNumberOfTasks];
    uint32_t keepAliveIsStuckCount[eNumberOfTasks];


public:

    DPV624(); //constructor
    ~DPV624(void);  //destructor
    //devices
    DPersistent *persistentStorage;
    eSysMode_t myMode;
    //application objects

    DInstrument *instrument;
    DErrorHandler *errorHandler;
    DKeyHandler *keyHandler;
    DUserInterface *userInterface;

    DCommsUSB *commsUSB;
    DCommsOwi *commsOwi;
    DCommsBluetooth *commsBluetooth;
    DExtStorage *extStorage;
    //DCommsMotor *commsMotor;

    DValve *valve1;
    DValve *valve2;
    DValve *valve3;

    LEDS *leds;

    DPowerManager *powerManager;
    DStepperMotor *stepperMotor;
    //DSensorTemperature *temperatureSensor;

    float myTareValue;
    sDate_t manufactureDate;
    float zeroVal;
    uint32_t pmUpgradePercent;
    uint32_t pmUpgradeStatus;

    DLogger *logger;
    bool isPrintEnable;

    bool isEngModeEnable;
    eSysMode_t resetToPowerUp;

    eBL652State_t blState;
    uint32_t optBoardStatus;

    sInstrumentMode_t instrumentMode;

    eErrorStatus_t osErrStatusDuringObectsCreation;
    void createApplicationObjects(void);
    void validateApplicationObject(OS_ERR os_error);
    eErrorStatus_t getOsErrStatusDuringObectsCreation(void);

    void handleError(eErrorCode_t errorCode,
                     eErrorStatus_t errStatus,
                     uint32_t paramValue,
                     uint16_t errInstance,
                     bool isFatal = false);

    void handleError(eErrorCode_t errorCode,
                     eErrorStatus_t errStatus,
                     float paramValue,
                     uint16_t errInstance,
                     bool isFatal = false);

    uint32_t getSerialNumber(uint32_t snType);
    bool setSerialNumber(uint32_t newSerialNumber);
    eRegionOfUse_t getRegion(void);
    bool setRegion(eRegionOfUse_t region);
    bool getDate(sDate_t *date);
    bool setDate(sDate_t *date);
    bool getTime(sTime_t *timeNow);
    bool setTime(sTime_t *timeNow);

    uint32_t getTestPoint(uint32_t index);
    uint32_t setTestPoint(uint32_t index, uint32_t parameter);

    int32_t queryEEPROMTest(void);
    void performEEPROMTest(void);
    bool getVersion(uint32_t item, char itemverStr[10]);
    bool getPosFullscale(float32_t  *fs);
    bool getNegFullscale(float32_t  *fs);
    bool getSensorType(eSensorType_t *sensorType);
    bool getControllerMode(eControllerMode_t *controllerMode);
    bool setControllerMode(eControllerMode_t newCcontrollerMode);
    bool getDK(uint32_t item, char dkStr[7]);
    void getInstrumentName(char nameStr[13]);
    bool getCalInterval(uint32_t sensor, uint32_t *interval);
    bool setCalInterval(uint32_t sensor, uint32_t interval);
    bool getFunction(eFunction_t *func);
    void takeNewReading(uint32_t rate);
    bool setControllerStatus(uint32_t newStatus);
    bool getControllerStatus(uint32_t *newStatus);
    bool getPressureSetPoint(float *setPoint);
    bool setPressureSetPoint(float newSetPointValue);
    void resetStepperMicro(void);
    bool getPM620Type(uint32_t *sensorType);
    ePinMode_t getPinMode(void);
    bool setPinMode(ePinMode_t mode);
    bool setManufactureDate(sDate_t *date);
    bool getManufactureDate(sDate_t *date);
    int32_t getUsbInstrumentPortConfiguration();
    void setUsbInstrumentPortConfiguration(int32_t mode);
    bool backupCalDataRestore(void);
    bool backupCalDataSave(void);
    bool setZero(uint32_t sensor, float32_t value);
    bool performPM620tUpgrade(void);
    bool performUpgrade(void);
    bool setRequiredNumCalPoints(uint32_t numCalPoints);
    bool abortCalibration(void);
    bool acceptCalibration(void);
    bool setCalPoint(uint32_t calPoint, float32_t value);
    bool startCalSampling(void);
    bool setCalibrationType(int32_t calType, uint32_t range);
    bool setCalDate(sDate_t *date);
    int32_t queryInvalidateCalOpeResult(void);
    bool getZero(float32_t *value);
    bool getRequiredNumCalPoints(eSensor_t sensorType, uint32_t *numCalPoints);
    bool getCalSamplesRemaining(uint32_t *samples);
    bool getCalDate(sDate_t *date);
    bool invalidateCalibrationData(void);
    DStepperMotor *getStepperMotorInstance(void);

    bool getVentRate(float *rate);
    bool setVentRate(float rate);

    bool setFilterCoeff(float32_t filterCoeff);
    bool getFilterCoeff(float32_t *filterCoeff);
    bool resetDisplayFilter(void);

    bool print(uint8_t *buf, uint32_t bufSize);
    void setPrintEnable(bool newState);
    bool engModeStatus(void);
    bool setAquisationMode(eAquisationMode_t newAcqMode);
    void getPmUpgradePercentage(uint32_t *percentage, uint32_t *pUpgradeStatus);
    void setPmUpgradePercentage(uint32_t percentage, uint32_t pUpgradeStatus);
    bool incrementSetPointCount(uint32_t *pSetPointCount);
    uint32_t getSetPointCount(void);
    void getBatLevelAndChargingStatus(float *pPercentCapacity,
                                      uint32_t *pChargingStatus);
    void updateBatteryStatus(void);
    void managePower(void);
    void shutdown(void);
    void startup(void);
    void setCommModeStatus(eCommInterface_t comInterface, eCommModes_t commMode);
    void clearCommModeStatus(eCommInterface_t comInterface, eCommModes_t commMode);
    sInstrumentMode_t getCommModeStatus(void);
    uint32_t getBoardRevision(void);
    bool getVersion(uint32_t item, uint32_t *itemver);
    bool manageBlueToothConnection(eBL652mode_t newMode);
    bool clearErrorLog(void);
    bool clearServiceLog(void);
    void keepAlive(eTaskID_t taskNum);
    bool IsAllTasksAreAlive(void);
    void resetSystem(void);
    void holdStepperMotorReset(void);
    bool getBaroPosFullscale(float32_t  *fs);
    bool getBaroNegFullscale(float32_t  *fs);
    bool moveMotorTillForwardEnd(void);
    bool moveMotorTillReverseEnd(void);
    bool moveMotorTillForwardEndThenHome(void);
    bool moveMotorTillReverseEndThenHome(void);
    bool setNextCalDate(sDate_t *date);
    bool setInstrumentCalDate(sDate_t *date);
    bool getInstrumentCalDate(sDate_t *date);
    bool getNextCalDate(sDate_t *date);
    bool logSetPointInfo(uint32_t setPointCount,
                         float setPointValue,
                         float distanceTravelled);

    bool updateDistanceTravelled(float32_t distanceTravelled);
    float32_t getDistanceTravelled(void);
    void holdStepperMicroInReset(void);
    void releaseStepperMicroReset(void);
    void stopMotor(void);
    void openVent(void);
    void switchUsbPortConfiguration(void);
    bool getSensorBrandMin(char *brandMin, uint32_t bufLen);
    bool getSensorBrandMax(char *brandMax, uint32_t bufLen);
    bool getSensorBrandType(char *brandType, uint32_t bufLen);
    bool getSensorBrandUnits(char *brandUnits, uint32_t bufLen);
    bool getSensorZeroValue(uint32_t sensor, float32_t *value);
    eBL652State_t getBlState(void);
    void setBlState(eBL652State_t bl652State);
    void setBlStateBasedOnMode(eBL652mode_t bl652Mode);

    eBluetoothTaskState_t myBlTaskState;
    eBluetoothTaskState_t getBluetoothTaskState(void);
    void setBluetoothTaskState(eBluetoothTaskState_t blTaskState);

    float32_t controllerDistance;
    bool resetDistanceTravelledByController(void);
    bool setDistanceTravelledByController(float32_t distance);
    bool getDistanceTravelledByController(float32_t *distance);
    eMotorError_t secondaryUcFwUpgrade(uint8_t *txData, uint8_t dataLength, uint8_t *response);
    eMotorError_t secondaryUcFwUpgradeCmd(uint32_t fileSize, uint8_t *responseAck);
    bool isDeviceDueForService(void);
    bool clearMaintainceData(void);
    bool getCalOffsets(float32_t *pCalOffsets);

    bool readOpticalBoardStatus(void);
    void setOpticalBoardStatus(void);
    uint32_t getOpticalBoardStatus(void);
    bool isBarometerDueForCalibration(bool *calDueStatus);
    void setSysMode(eSysMode_t sysMode);
    eSysMode_t getSysMode(void);
    eSysMode_t getResetCause(void);

    void waitOnSecondaryStartup(void);
    void resetQspiFlash(void);

    deviceStatus_t getDeviceStatus(void);
    void updateDeviceStatus(eErrorCode_t errorCode, eErrorStatus_t errStatus);
    void ventSystem(void);
    bool getBarometerCalStatus(void);
    void initClassVariables(void);
    uint32_t runDiagnostics(void);
    bool testExternalFlash(void);
    bool checkBluetoothCommInterface(void);
    bool updateSetPointCount(uint32_t setPointCount);
    bool shutdownPeripherals(void);
    void getBatteryManufName(int8_t *batteryManuf,
                             uint32_t bufSize);

    bool performShutdown(eShutdown_t shutdownType, bool autoPowerDownForced = false);
    bool queryPowerdownAllowed(void);
    bool configureExternalFlashMemory(void);
    bool getExternalFlashStatus(uint32_t *bytesUsed, uint32_t *bytesTotal);


};

/* Variables -------------------------------------------------------------------------------------------------------*/
extern DPV624 *PV624;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

#endif /* __DPI610E_H */
