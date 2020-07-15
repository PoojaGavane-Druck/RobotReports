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
* @file     DSensorExternal.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     08 June 2020
*
* @brief    The external sensor base class header file
*/

#ifndef _DSENSOR_EXTERNAL_H
#define _DSENSOR_EXTERNAL_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
MISRAC_ENABLE

#include "DSensor.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DSensorExternal : public DSensor
{
public:
    DSensorExternal();

    virtual eSensorError_t readIdentity(void);
    virtual eSensorError_t readSerialNumber(void);
    virtual eSensorError_t readFullscaleAndType(void);
    virtual eSensorError_t readNegativeFullscale(void);
    virtual eSensorError_t readCalInterval(void);
    virtual eSensorError_t readCalDate(void);
    virtual eSensorError_t readManufactureDate(void);
};

#endif /* _DSENSOR_EXTERNAL_H */
