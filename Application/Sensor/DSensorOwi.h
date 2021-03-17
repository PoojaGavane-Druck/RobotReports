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
* @file     DSensorAMC.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     14 april 2020
*
* @brief    The DUCI sensor class header file
*/

#ifndef __DSENSOR_OWI_H
#define __DSENSOR_OWI_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_def.h>
MISRAC_ENABLE

#include "DSensorExternal.h"
#include "DDeviceSerial.h"
#include "DOwiParse.h"
#include "DCommsState.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
// Masks for OWI protocol
#define OWI_SYNC_BIT 0x80u
#define OWI_TYPE_BIT 0x40u
//sensor error bit masks
#define DUCI_SENSOR_ERROR_PROGRAMMING       0x0001u
#define DUCI_SENSOR_ERROR_PARAMETER         0x0002u
#define DUCI_SENSOR_ERROR_CONFIGURATION		0x0004u
#define DUCI_SENSOR_ERROR_ADDRESS           0x0008u
#define DUCI_SENSOR_ERROR_CHECKSUM          0x0010u
#define DUCI_SENSOR_ERROR_ZERO      		0x0020u
#define DUCI_SENSOR_ERROR_CALIBRATION		0x0040u
#define DUCI_SENSOR_ERROR_SEQUENCE	        0x0080u
#define DUCI_SENSOR_ERROR_INVALID_FUNCTION  0x0100u

//only interested in the bits specified in this mask
#define DUCI_SENSOR_ERROR_MASK              (DUCI_SENSOR_ERROR_ZERO | DUCI_SENSOR_ERROR_CALIBRATION)

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum : uint32_t
{
  OWI_INTERFACE_1,
  OWI_INTERFACE_2
}OwiInterfaceNo_t;
/* Variables --------------------------------------------------------------------------------------------------------*/

class DSensorOwi : public DSensorExternal
{
private:
    //static functions required as 'function pointer' parameters
   

protected:
    DDeviceSerial* myComms;
    DOwiParse *myParser;


    uint8_t *myTxBuffer;
    uint32_t myTxBufferSize;

    uint32_t commandTimeoutPeriod; //time in (ms) to wait for a response to a command
    
    OwiInterfaceNo_t myInterfaceNumber;

    virtual void createOwiCommands(void);

    sExternalDevice_t connectedDevice;

    eSensorError_t sendQuery(uint8_t cmd);

    void updateStatus(uint32_t errorStatus);

public:
    DSensorOwi(OwiInterfaceNo_t interfaceNumber);

    virtual eSensorError_t initialise();
    virtual eSensorError_t close();

    virtual eSensorError_t measure();
    
    virtual eSensorError_t readAppIdentity(void);
    virtual eSensorError_t readBootLoaderIdentity(void);
    virtual eSensorError_t readSerialNumber(void);
    virtual eSensorError_t readCalDate(void);
    virtual eSensorError_t readManufactureDate(void);
    virtual eSensorError_t readFullscaleAndType(void);
    virtual eSensorError_t readNegativeFullscale(void);
    virtual eSensorError_t readStatus(void);
    virtual eSensorError_t readZero(float32_t *pZero);

    virtual eSensorError_t readCalInterval(void);
    virtual eSensorError_t writeCalInterval(uint32_t calInterval);

    virtual eSensorError_t writeCalModeExit(void);
    virtual eSensorError_t writeCalMode(void);
    virtual eSensorError_t writeCalDate(RTC_DateTypeDef eDateTime);

    virtual eSensorError_t performZero(void);

   
   
};

#endif /* __DSENSOR_DUCI_H */
