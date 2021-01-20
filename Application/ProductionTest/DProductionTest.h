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
#include <os.h>
MISRAC_ENABLE

#include "Types.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
//Production test task flags
#define EV_FLAG_SELF_TEST_EEPROM        0x00000001u //EEPROM self-test
#define EV_FLAG_TASK_SELF_TEST_FLASH    0x00000002u //flash memory self-test
#define EV_FLAG_TASK_SELF_TEST_USB      0x00000004u //USB self-test


/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DProductionTest
{
private:
    static DProductionTest *myInstance;

    int32_t eepromSelfTestStatus;
    int32_t spiFlashSelfTestStatus;
    int32_t usbSelfTestStatus;

    DProductionTest(void);                  //private constructor (singleton pattern)

    OS_TCB myTaskTCB;                       //Task Control Block for this task
    bool myReadyState;                      //flag to indicate task is up and running

    OS_FLAG_GRP myEventFlags;               //event flags to pend on
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond

    OS_MUTEX myMutex;                       //mutex for variable resource locking

    static void runFunction(void *p_arg);   //task function
    void postEvent(OS_FLAGS flags);

    void setReadyState(void);

public:
    //public methods
    static DProductionTest *getInstance(void) //singleton pattern
    {
        if (myInstance == NULL)
        {
            myInstance = new DProductionTest();
        }

        return myInstance;
    }

    //methods
    void start(void);
    bool isRunning(void);
    void initialise(void);
    void pressureSensorTest(int32_t subTestIndex);
    void eepromSelfTest(void);
    void performEepromSelfTest(void);
    int32_t queryEepromSelfTest(void);
    int32_t getBarometerDeviceId(void);
    void softwareShutdown(int32_t subTestIndex);
    int32_t powerButtonMonitor(void);
    void spiFlashSelfTest(void);
    void performSpiFlashSelfTest(void);
    int32_t querySpiFlashSelfTest(void);
    
    void usb_LDO_IC19_Enable(int32_t subTestIndex);
    void configureUsb(void);
    void usbSelfTest(void);
    void performUsbSelfTest(void);
    int32_t queryUsbSelfTest(void);
    int32_t getBluetoothDeviceId(void);
    void bluetoothReset(int32_t subTestIndex);
    void setLed(int32_t ledIndex);
    void resetLed(int32_t ledIndex);
   
   
   
   
    int getKeys(void);
    void setKeys(int32_t keys, int32_t duration);
    void displayTestMessage(char *str);
   
};

#endif //__DPRODUCTION_TEST_H
