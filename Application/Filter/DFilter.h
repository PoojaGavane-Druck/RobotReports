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
* @file     DFilter.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     17 July 2020
*
* @brief    The sensor sample filter base class header file
*/

#ifndef _DFILTER_H
#define _DFILTER_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <stdbool.h>
MISRAC_ENABLE

#include "Types.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DFilter
{
protected:
    bool myReset;
    bool enabled;

public:
    DFilter(void);
    virtual float32_t run(float32_t input);
    virtual void reset(void);
    bool getEnabled(void);
    void setEnabled(bool state);
};

#endif /* _DFILTER_H */

