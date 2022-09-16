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
* @file     DCommsStateBluetoothIdle.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     13 November 2020
*
* @brief    The Bluetooth comms idle (local) state class header file
*/

#ifndef __DCOMMS_STATE_USB_BLUETOOTH_H
#define __DCOMMS_STATE_USB_BLUETOOTH_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDuci.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateBluetoothIdle : public DCommsStateDuci
{
    uint32_t remoteRequestTimeOut;
protected:
    virtual void createCommands(void);

public:
    DCommsStateBluetoothIdle(DDeviceSerial *commsMedium, DTask *task);
    virtual ~DCommsStateBluetoothIdle(void);

    virtual eStateDuci_t run(void);
    virtual void suspend(void);
    virtual void resume(void);

    virtual sDuciError_t fnGetKM(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnSetKM(sDuciParameter_t *parameterArray);
};

#endif /* __DCOMMS_STATE_USB_BLUETOOTH_H */
