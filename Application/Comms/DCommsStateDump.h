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
* @file     DCommsStateDump.h
* @version  1.00.00
* @author   Nageswara Rao
* @date     01 April 2020
*
* @brief    This comm state is for data dump
*/

#ifndef __DCOMMS_STATE_DUMP_H
#define __DCOMMS_STATE_DUMP_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDuci.h"
#include "DDeviceSerial.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateDump : public DCommsStateDuci
{
private:
   
protected:
    

public:
    DCommsStateDump(DDeviceSerial *commsMedium, DTask *task);
    
    virtual eStateDuci_t run(void);


};

#endif /* __DCOMMS_STATE_LOCAL_H */
