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
#include "cBL652.h"
/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsBluetooth : public DComms
{
public:
    DCommsBluetooth(char *mediumName, OS_ERR *os_error);
    virtual ~DCommsBluetooth();
    void initialise(void);

    virtual void setTestMode(bool state);
    bool getDeviceId(char *buffer, int32_t size);
    bool setPairingStatus(eBL652PairingMode_t pairingStatus);
    bool checkBlModulePresence(void);
    bool getFWVersion(char *str);
    bool getAppVersion(uint8_t *appVer, uint32_t sizeOfAppVer);
    bool startAdverts(uint8_t *str, uint32_t strLen);
    bool stopAdverts(void);
    bool disconnect(void);
    bool startApplication(void);
    bool setDeviceMode(eBL652mode_t mode);
    bool eraseBL652FileSystem(void);
    bool openFileInBL652ToCopyApp(void);
    bool writeToTheBl652Module(uint8_t *bufferPtr, uint8_t count);
    bool closeFile(void);
    bool resetBL652(void);
    bool getFileListBl652(void);
    bool getChecksumBl652(uint16_t *receivedChecksum);


};

#endif /* __DCOMMS_BLUETOOTH_H */
