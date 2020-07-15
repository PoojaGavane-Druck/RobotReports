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
* @file     DDevice.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The device driver base class header file
*/

#ifndef __DDEVICE_H
#define __DDEVICE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdbool.h>
#include <os.h>
MISRAC_ENABLE

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DDevice
{
protected:
    bool opened;
    OS_MUTEX myMutex;

    bool createMutex(char* name);

public:
    DDevice();
    ~DDevice();

    virtual void open(void);
    virtual void close(void);
    virtual bool isOpen(void);

    virtual bool selfTest(void);
};

#endif /* __DDEVICE_H */
