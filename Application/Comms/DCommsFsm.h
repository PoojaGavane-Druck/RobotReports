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
* @file     DCommsFsm.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The comms finite state machine base class header file
*/

#ifndef __DCOMMS_FSM_H
#define __DCOMMS_FSM_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <stdlib.h>
MISRAC_ENABLE

#include "DCommsState.h"
#include "DDeviceSerial.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsFsm
{
protected:
    eStateDuci_t myInitialState;
    eStateDuci_t myCurrentState;
    DCommsState *myStateArray[E_STATE_DUCI_SIZE];

public:
    DCommsFsm(void);

    virtual void createStates(DDeviceSerial *commsMedium);

    virtual void run(void);
    void suspend(void);
    void resume(void);

    sExternalDevice_t *getConnectedDeviceInfo(void);
};

#endif /* __DCOMMS_FSM_H */

