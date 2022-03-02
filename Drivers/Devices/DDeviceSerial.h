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
* @file     DDeviceSerial.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The serial comms driver base class header file
*/

#ifndef __DDEVICE_SERIAL_H
#define __DDEVICE_SERIAL_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
MISRAC_ENABLE

#include "DDevice.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
//#define SERIAL_RECEIVE_BUFFER_SIZE 256u //TODO: uart driver has its own buffer - do we want to pass this one instead?
#define TX_BUFFER_SIZE 64u

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DDeviceSerial : public DDevice
{
//protected:
//    char rxBuffer[SERIAL_RECEIVE_BUFFER_SIZE]; //TODO: uart driver has its own buffer - do we want to pass this one instead?
    char txBuffer[TX_BUFFER_SIZE];

public:
    DDeviceSerial();
    ~DDeviceSerial();
    char *getTxBuffer(void);
    uint32_t getTxBufferSize(void);

    virtual bool getRcvBufLength(uint16_t *length);
    virtual void clearRxBuffer(void); //Temporarily virtual - all comms has own buffer which base class could clear

    virtual bool sendString(char *str);  //TODO: Extend this to have more meaningful returned status
    virtual bool receiveString(char **pStr, uint32_t waitTime); //TODO: Extend this to have more meaningful returned status

    virtual bool read(uint8_t **pStr, uint32_t numOfBytesToRead, uint32_t *numOfBytesRead, uint32_t waitTime);
    virtual bool write(uint8_t *str, uint32_t numOfBytesToWrite);

    virtual bool query(char *str, char **pStr, uint32_t waitTime);  //This is a combined send and receive with a resource lock around it
    virtual bool query(uint8_t *str, uint32_t cmdLength, uint8_t **pStr, uint32_t responseLen, uint32_t waitTime);  //This is a combined send and receive with a resource lock around it

};

#endif /* __DDEVICE_SERIAL_H */
