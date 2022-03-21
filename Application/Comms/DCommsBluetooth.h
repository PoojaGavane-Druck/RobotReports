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
* @file     DCommsBluetooth.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     13 November 2020
*
* @brief    The Bluetooth comms class header file
*/

#ifndef __DCOMMS_BLUETOOTH_H
#define __DCOMMS_BLUETOOTH_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DComms.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsBluetooth : public DComms
{
public:
    DCommsBluetooth(char *mediumName, OS_ERR *os_error);
    void initialise(void);

    virtual void setTestMode(bool state);
    bool getDeviceId(char *buffer, int32_t size);
};

#endif /* __DCOMMS_BLUETOOTH_H */
