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
#include "cBL652.h"
#include "Types.h"
/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DDeviceSerialBluetooth : public DDeviceSerial
{
    char blTxString[TX_BUFFER_SIZE];
public:
    DDeviceSerialBluetooth();

    void clearRxBuffer(void);           //Overridden - all comms has own buffer which base class could clear
    bool sendString(char *str, uint32_t buffSize);
    bool receiveString(char **pStr, uint32_t waitTime = 0u);
    bool getDeviceId(char *buffer, int32_t size);
    bool startAdverts(uint8_t *str, uint32_t strLen);
    bool startApplication(void);
    bool stopAdverts();
    bool setDeviceMode(eBL652mode_t mode);
    bool disconnect();
    bool checkBlModulePresence(void);
    bool getFWVersion(char *str);
    bool getAppVersion(char *str);
    bool setPairingStatus(eBL652PairingMode_t pairingState);
};

#endif /* __DDEVICE_SERIAL_BLUETOOTH_H */
