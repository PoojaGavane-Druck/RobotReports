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
* @file     DDeviceSerialRS485.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The RS485 serial comms driver base class header file
*/

#ifndef __DDEVICE_SERIAL_INTERFACE_H
#define __DDEVICE_SERIAL_INTERFACE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DDeviceSerial.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DDeviceSerialInterface : public DDeviceSerial
{
public:
    DDeviceSerialInterface();

    void clearRxBuffer(void); //Temporarily overriden - all comms has own buffer which base class could clear
    bool sendString(char *str, uint32_t buffSize);  //TODO: Extend this to have more meaningful returned status
    bool receiveString(char **pStr, uint32_t waitTime); //TODO: Extend this to have more meaningful returned status

    virtual bool read(uint8_t **pStr, uint32_t numOfBytesToRead, uint32_t *numOfBytesRead, uint32_t waitTime);
    virtual bool write(uint8_t *Str, uint32_t numOfBytesToWrite);
    virtual bool query(uint8_t *str,
                       uint32_t cmdLength,
                       uint8_t **pStr,
                       uint32_t responseLen,
                       uint32_t waitTime);
};

#endif /* __DDEVICE_SERIAL_RS485_H */
