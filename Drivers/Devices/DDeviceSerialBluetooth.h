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
* @file     DDeviceSerialBluetooth.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     13 November 2020
*
* @brief    The Bluetooth serial comms driver base class header file
*/

#ifndef __DDEVICE_SERIAL_BLUETOOTH_H
#define __DDEVICE_SERIAL_BLUETOOTH_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DDeviceSerial.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DDeviceSerialBluetooth : public DDeviceSerial
{
public:
    DDeviceSerialBluetooth();

    void clearRxBuffer(void); //Temporarily overridden - all comms has own buffer which base class could clear
    bool sendString(char *str);  //TODO: Extend this to have more meaningful returned status
    bool receiveString(char **pStr, uint32_t waitTime = 0u); //TODO: Extend this to have more meaningful returned status

    virtual bool query(char *str, char **pStr, uint32_t waitTime = 0u);
};

#endif /* __DDEVICE_SERIAL_BLUETOOTH_H */
