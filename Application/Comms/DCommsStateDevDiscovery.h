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
* @file     DCommsStateDevDiscovery.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     14 April 2020
*
* @brief    The comms external device discovery state class header file
*/

#ifndef __DCOMMS_STATE_DEV_DISCOVERY_H
#define __DCOMMS_STATE_DEV_DISCOVERY_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsState.h"
#include "DDeviceSerial.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateDevDiscovery : public DCommsState
{
private:
    bool isDeviceSupported(void);

public:
    DCommsStateDevDiscovery(DDeviceSerial *commsMedium);
    virtual eStateDuci_t run(void);

    virtual sDuciError_t fnSetSN(sDuciParameter_t * parameterArray);
};

#endif /* __DCOMMS_STATE_DEV_DISCOVERY_H */
