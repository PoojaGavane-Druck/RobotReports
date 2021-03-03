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
#include <os.h>
#include <cpu.h>
#include <stdint.h>
//#include <stm32l4xx_hal.h>
//#include <bsp_os.h>
//#include <bsp_int.h>
//#include <stdbool.h>
MISRAC_ENABLE

#include "Types.h"
#include "DTask.h"

/* Types ------------------------------------------------------------------------------------------------------------*/
//Instrument mode - all bit s0 means local mode, else remote or test as indicated by individual bits
typedef union
{
    uint32_t value;

    struct
    {
        uint32_t remoteOwi   : 1;
        uint32_t remoteUsb      : 1;
        uint32_t remoteBluetooth : 1;
        uint32_t test           : 1;
    };

} sInstrumentMode_t;

/* Prototypes -------------------------------------------------------------------------------------------------------*/
class DUserInterface : public DTask
{
private:

    sInstrumentMode_t myInstrumentMode;

    OS_ERR postEvent(uint32_t event, uint32_t param8, uint32_t param16);
    void processKey(uint32_t keyPressed, uint32_t pressType);
    
public:
    DUserInterface(OS_ERR *osErr);
    ~DUserInterface();

    virtual void initialise(void);
    virtual void runFunction(void);
    virtual void cleanUp(void);

    OS_ERR handleKey(uint32_t keyPressed, uint32_t pressType);
    OS_ERR handleMessage(eUiMessage_t uiMessage);

    sInstrumentMode_t getMode();
    void setMode(sInstrumentMode_t mask);
    void clearMode(sInstrumentMode_t mask);

   
    void notify(eUiMessage_t event, uint32_t channel, uint32_t index = 0u);
    void updateReading(uint32_t channel);
    void functionShutdown(uint32_t channel);
    void sensorConnected(uint32_t channel);
    void sensorDisconnected(uint32_t channel);
    void sensorPaused(uint32_t channel);

   
};

#endif /* __DUSER_INTERFACE_H */
