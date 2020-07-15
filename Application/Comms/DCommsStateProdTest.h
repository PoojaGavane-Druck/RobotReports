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
* @file     DCommsStateProdTest.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The comms production test state class header file
*/

#ifndef __DCOMMS_STATE_PROD_TEST_H
#define __DCOMMS_STATE_PROD_TEST_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsState.h"
#include "DDeviceSerial.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateProdTest : public DCommsState
{
protected:
    virtual void createDuciCommands(void);

public:
    DCommsStateProdTest(DDeviceSerial *commsMedium);
    virtual eStateDuci_t run(void);
};

#endif /* __DCOMMS_STATE_PROD_TEST_H */
