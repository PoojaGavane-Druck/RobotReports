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
* @file     DComms.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The comms class header file
*/

#ifndef __DCOMMS_H
#define __DCOMMS_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DTask.h"
#include "DCommsFsm.h"
#include "DSensor.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DComms : public DTask
{
private:
    //nothing yet

protected:
    DDeviceSerial *commsMedium;
    DCommsFsm *myCommsFsm;

public:
    DComms();
    virtual ~DComms();
    void start(char *mediumName, OS_ERR *os_error);

    DDeviceSerial *getMedium(void);

    virtual void initialise(void);
    virtual void runFunction(void);

    void getConnectedDeviceInfo(sExternalDevice_t  *device);
    bool waitForEvent(OS_FLAGS waitFlags, uint32_t waitTime);

    virtual bool grab(DSensor *sensor);
    virtual bool release(DSensor *sensor);

    virtual void setTestMode(bool state);

};

#endif /* __DCOMMS_H */
