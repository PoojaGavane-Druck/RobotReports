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
* @file     DProductionTest.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     18 September 2020
*
* @brief    The production test class header file
*/

#ifndef __DPRODUCTION_TEST_H
#define __DPRODUCTION_TEST_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <rtos.h>
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_def.h>
MISRAC_ENABLE

#include "Types.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
//Production test task flags
#define EV_FLAG_SELF_TEST_EEPROM        0x00000001u //EEPROM self-test
#define EV_FLAG_TASK_SELF_TEST_FLASH    0x00000002u //flash memory self-test
#define EV_FLAG_TASK_SELF_TEST_USB      0x00000004u //USB self-test
#define EV_FLAG_TASK_INVALIDATE_CAL_DATA 0x00000008u //It invalidates barometer calibration data


/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
typedef enum : uint8_t
{
    LED_NONE = 0,
    LED_1,
    LED_2,
    LED_3,
    LED_4,
    LED_5,
    LED_6,
    LED_7,
    LED_8,
    LED_9,
    LED_10,
    LED_11
} eLED_Num_t;

typedef enum : uint8_t
{
    LED_OFF = 0,
    LED_ON
} eLED_OnOffState_t;

class DProductionTest
{
private:
    static DProductionTest *myInstance;
    char *myName;                           //task name

    int32_t eepromSelfTestStatus;
    int32_t norFlashSelfTestStatus;
    int32_t usbSelfTestStatus;
    int32_t invalidateCalOperationStatus;
    int32_t stepCount;

    DProductionTest(void);                  //private constructor (singleton pattern)

    OS_TCB myTaskTCB;                       //Task Control Block for this task
    bool myReadyState;                      //flag to indicate task is up and running

    OS_FLAG_GRP myEventFlags;               //event flags to pend on
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond

    OS_MUTEX myMutex;                       //mutex for variable resource locking

    static void runFunction(void *p_arg);   //task function
    void postEvent(OS_FLAGS flags);

    void setReadyState(void);
    void ledsTest(eLED_Num_t LED_Number, eLED_OnOffState_t onOffState);
public:
    //public methods
    static DProductionTest *getInstance(void) //singleton pattern
    {
        if(myInstance == NULL)
        {
            myInstance = new DProductionTest();
        }

        return myInstance;
    }

    //methods
    void start(void);
    bool isRunning(void);
    void initialise(void);


    void eepromSelfTest(void);
    void performEepromSelfTest(void);
    int32_t queryEepromSelfTest(void);

    void spiFlashSelfTest(void);
    void performSpiFlashSelfTest(void);
    int32_t querySpiFlashSelfTest(void);

    int32_t getBarometerDeviceId(void);
    void softwareShutdown(int32_t subTestIndex);
    int32_t powerButtonMonitor(void);

    int32_t getTemperatureSensorDeviceId(void);

    int32_t get24VoltSupplyStatus(void);
    int32_t get6VoltSupplyStatus(void);
    int32_t get5VoltSupplyStatus(void);
    int32_t get5VoltPm620SupplyStatus(void);

    float32_t get24VoltSupplyValue(void);
    float32_t get6VoltSupplyValue(void);
    float32_t get5VoltSupplyValue(void);

    int32_t testValve1(int32_t subTestIndex);
    int32_t testValve2(int32_t subTestIndex);
    int32_t testValve3(int32_t subTestIndex);

    int32_t getBluetoothDeviceId(void);
    void switchOnLed(int32_t ledIndex);
    void switchOffLed(int32_t ledIndex);
    void displayBatteryStatus(float *pPercentCapacity,
                              uint32_t *pChargingStatus);
    int32_t getPM620DeviceId(void);
    int32_t getBatteryId(void);
    int32_t getBatteryChargerId(void);

    uint32_t getKeys(void);
    void setKeys(uint32_t keys, uint32_t duration);

    int32_t getBarometerReading(float32_t *measValue);
    int32_t getPM620Reading(float32_t *measValue);
    void setStepperMotorParam(int32_t param);
    int32_t controlChargerEnablePin(int32_t param);
    void invalidateCalibrationData(void);
    void performCalDataInvalidateOperation(void);
    int32_t queryInvalidateCalOpeResult(void);
    bool moveMotorTillForwardEnd(void);
    bool moveMotorTillReverseEnd(void);
    bool moveMotorTillForwardEndThenHome(void);
    bool moveMotorTillReverseEndThenHome(void);
    bool getMotorStatus(void);
    bool moveMotor(int32_t stepCnt);
    bool queryMotorStepCount(int32_t *stepCnt);
    int32_t querySecondMicroDKnumber(void);



};

#endif //__DPRODUCTION_TEST_H
