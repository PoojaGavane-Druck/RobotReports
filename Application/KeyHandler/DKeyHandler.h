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

#include "gpio.h"
#include "DTask.h"

/* Defines  ---------------------------------------------------------------------------------------------------------*/
//Key message component masks
#define MSG_KEY_TYPE_MASK			0xFFFF0000
#define MSG_KEY_CODE_MASK			UI_KEY_MASK

#define MSG_KEY_TYPE_LOCAL			0x00010000
#define MSG_KEY_TYPE_REMOTE			0x00020000

#define KEY_DEBOUNCE_TIME_MS        50u
#define KEY_LONG_PRESS_TIME_MS		2000u

/*Key codes*/
#define UI_KEY_MASK             0x0000003Fu		/*Mask for all keys*/
#define UI_KEY_LONG_PRESS       0x00000020u		/*Mask/bit for long press*/

#define UI_KEY_TOP_LEFT         0x00000001u		/*LEFT button*/
#define UI_KEY_TOP_MIDDLE       0x00000002u		/*MIDDLE button*/
#define UI_KEY_TOP_RIGHT	    0x00000004u		/*RIGHT button*/
#define UI_KEY_BOTTOM_LEFT      0x00000008u		/*BACKLIGHT button*/
#define UI_KEY_BOTTOM_MIDDLE    0x00000010u		/*LEAK button*/
#define UI_KEY_BOTTOM_RIGHT     0x00000020u		/*on/off button*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_PRESS_SHORT = 0,
    E_PRESS_LONG

} ePress_t;

typedef enum
{
    E_BUTTON_NONE,
    E_BUTTON_1,
    E_BUTTON_2,
    E_BUTTON_3,
    E_BUTTON_5,
    E_BUTTON_4,
    E_BUTTON_6

} eButton_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DKeyHandler : public DTask
{
public:
    DKeyHandler(OS_ERR *osErr);
    ~DKeyHandler();

    virtual void runFunction(void);

    void sendKey(gpioButtons_t keyCode);
};

#endif /* __DKEYHANDLER_H */
