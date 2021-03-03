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
* @file     DCommsStateLocal.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The comms local state class header file
*/

#ifndef __DCOMMS_STATE_LOCAL_H
#define __DCOMMS_STATE_LOCAL_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDuci.h"
#include "DDeviceSerial.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateLocal : public DCommsStateDuci
{
private:
    static sDuciError_t fnSetRI(void *instance, sDuciParameter_t * parameterArray);

protected:
    virtual void createCommands(void);

public:
    DCommsStateLocal(DDeviceSerial *commsMedium, DTask *task);
    
    virtual eStateDuci_t run(void);

    virtual sDuciError_t fnGetKM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnSetKM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnSetRI(sDuciParameter_t * parameterArray);
};

#endif /* __DCOMMS_STATE_LOCAL_H */
