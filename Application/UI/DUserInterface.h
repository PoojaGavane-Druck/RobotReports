/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DUserInterface.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     30 March 2020
*
* @brief    The user interface class header file
*/

#ifndef __DUSER_INTERFACE_H
#define __DUSER_INTERFACE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <rtos.h>
#include <cpu.h>
#include <stdint.h>
//#include <stm32l4xx_hal.h>
//#include <bsp_os.h>
//#include <bsp_int.h>
//#include <stdbool.h>
MISRAC_ENABLE

#include "Types.h"
#include "DTask.h"
#include "leds.h"

#define UI_TASK_TIMEOUT_MS         250u    // UI task timeout
#define UI_DEFAULT_BLINKING_RATE   1u      // Default Blink rate is task timeout Time
#define BATTERY_LEDS_DISPLAY_TIME  (5000u)
#define BATTERY_LED_UPDATE_RATE    1u     // Battery Leds Update rate

#define DISP_TIME_CONTINUOUS    65535u
/* Types ------------------------------------------------------------------------------------------------------------*/
typedef struct
{
    eLedColour_t  colour;
    eLedOperation_t operation;
    eLedState_t stateAfterOperationCompleted;
    uint16_t displayTime;
    uint32_t blinkingRate;
} sLed_t;
/* Prototypes -------------------------------------------------------------------------------------------------------*/
class DUserInterface : public DTask
{
private:
    sLed_t statusLed;
    sLed_t blueToothLed;
    sLed_t batteryLed;

    sLed_t statusSavedLed;
    sLed_t bluetoothSavedLed;

    LEDS   myLeds;
    uint32_t statusLedBlinkRateCounter;
    uint32_t bluettothLedBlinkRateCounter;
    uint32_t batteryLedUpdateRateCounter;

    OS_ERR postEvent(uint32_t event);
    void processMessage(uint32_t rxMsgValue);
    void handleTimeout(void);

public:
    DUserInterface(OS_ERR *osErr);
    ~DUserInterface();

    virtual void initialise(void);
    virtual void runFunction(void);
    virtual void cleanUp(void);

    void statusLedControl(eStatusLed_t status,
                          eLedOperation_t operation,
                          uint16_t displayTime,
                          eLedState_t stateAfterTimeout,
                          uint32_t blinkingRate);

    void bluetoothLedControl(eBlueToothLed_t status,
                             eLedOperation_t operation,
                             uint16_t displayTime,
                             eLedState_t stateAfterTimeout,
                             uint32_t blinkingRate);
    void updateBatteryStatus(uint16_t displayTime,
                             uint32_t updateRate);

    void saveStatusLedState(void);
    void saveBluetoothLedState(void);
    void restoreStatusLedState(void);
    void restoreBluetoothLedState(void);



};

#endif /* __DUSER_INTERFACE_H */
