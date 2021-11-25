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
* @file     DKeyHandler.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The key handler class header file
*/

#ifndef __DKEYHANDLER_H
#define __DKEYHANDLER_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <os.h>
#include <cpu.h>
#include <lib_mem.h>
#include <stm32l4xx_hal.h>
#include <bsp_os.h>
#include <bsp_int.h>
#include <stdbool.h>
MISRAC_ENABLE


#include "DTask.h"

/* Defines  ---------------------------------------------------------------------------------------------------------*/
//Key message component masks

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_PRESS_SHORT = 0,
    E_PRESS_POWER_ONOFF,
    E_PRESS_LONG

} ePress_t;

typedef enum
{
    E_BUTTON_NONE,
    E_BUTTON_1,
    E_BUTTON_2,
   

} eButton_t;

typedef union
{
    struct
    {
        uint32_t powerOnOff  : 1;
        uint32_t blueTooth   : 1;
        

               
        uint32_t remote    : 1;
        uint32_t reserved  : 29;

    } bit;

    uint32_t bytes;

} gpioButtons_t;

typedef union
{
    struct
    {
        uint32_t powerOnOff     : 1;
        uint32_t updateBattery  : 1;
        uint32_t blueTooth      : 1;
        

        uint32_t longPress : 1;        
        uint32_t remote    : 1;
        uint32_t reserved  : 27;

    } bit;

    uint32_t bytes;

} pressType_t;
/* Variables --------------------------------------------------------------------------------------------------------*/

class DKeyHandler : public DTask
{
    uint32_t timeoutCount;
    uint32_t powerState;
    bool triggered;
    gpioButtons_t keys;
    pressType_t pressType;  
    gpioButtons_t getKey(void);
    void processKey(bool timedOut);
    void sendKey(void);
public:
    DKeyHandler(OS_ERR *osErr);
    ~DKeyHandler();

    virtual void runFunction(void);

    void sendKey(gpioButtons_t keyCode, pressType_t keyPressType);
    void setKeys(uint32_t keyCodes, uint32_t duration);
    uint32_t getKeys(void);
    
};

#endif /* __DKEYHANDLER_H */
