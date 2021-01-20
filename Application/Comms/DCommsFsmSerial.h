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
* @file     DCommsFsmSerial.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The serial comms finite state machine class header file
*/

#ifndef __DCOMMS_FSM_SERIAL_H
#define __DCOMMS_FSM_SERIAL_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <stdlib.h>
MISRAC_ENABLE

#include "DCommsFsm.h"
#include "DDeviceSerial.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsFsmSerial : public DCommsFsm
{
public:
    DCommsFsmSerial(void);

    virtual void createStates(DDeviceSerial *commsMedium, DTask *task);
};

#endif /* __DCOMMS_FSM_SERIAL_H */

