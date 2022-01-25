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
* @file     DCommsStateRemoteUsb.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     04 June 2020
*
* @brief    The USB comms remote state class header file
*/

#ifndef __DCOMMS_STATE_REMOTE_USB_H
#define __DCOMMS_STATE_REMOTE_USB_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateRemote.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/
class DCommsStateRemoteUsb : public DCommsStateDuci
{
protected:
    DCommsStateRemote *myRemoteCommsState;

public:
    //public methods
    DCommsStateRemoteUsb(DDeviceSerial *commsMedium, DTask *task);

    virtual eStateDuci_t run(void);
};

#endif /* __DCOMMS_STATE_REMOTE_USB_H */
