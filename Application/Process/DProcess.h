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
* @file     DProcess.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     16 June 2020
*
* @brief    The process data base class header file
*
**********************************************************************************************************************/

#ifndef __DPROCESS_H
#define __DPROCESS_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <os.h>
#include <stdint.h>
#include <stdbool.h>
MISRAC_ENABLE

#include "Types.h"
#include "DLock.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define BIG_POSITIVE_NUMBER 1E10f
#define BIG_NEGATIVE_NUMBER -1E10f

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Class definition -------------------------------------------------------------------------------------------------*/

class DProcess
{
protected:
    uint32_t myChannelIndex; //quick means of knowing which channel this process belongs to
    eProcess_t myProcessIndex;

    float32_t myInput;
    float32_t myOutput;

    OS_MUTEX myMutex;

    bool myEnabledState;



    virtual void setInput(float32_t value);
    virtual void setOutput(float32_t value);

public:
    DProcess(uint32_t channelIndex);

    virtual float32_t run(float32_t input);

    virtual void enable(void);
    virtual void disable(void);
    virtual void reset(void);

    virtual float32_t getInput(void);
    virtual float32_t getOutput(void);

    virtual float32_t getParameter(uint32_t index = 0u);
    virtual void setParameter(float32_t value, uint32_t index = 0u);
};

#endif // __DPROCESS_H
