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
* @file     DInstrument.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The instrument base class header file
*/

#ifndef _DINSTRUMENT_H
#define _DINSTRUMENT_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

#include "DFunctionMeasure.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/


/* Variables --------------------------------------------------------------------------------------------------------*/

class DInstrument
{
private:
   
protected:
    DFunctionMeasure *myCurrentFunction;
public:
    DInstrument(OS_ERR *osErr);

    bool setFunction( eFunction_t func);
    bool getReading( eValueIndex_t index, float32_t *reading);
    bool getPosFullscale( float32_t *fs);
    bool getBarometerIdentity( uint32_t *identity);
    bool getNegFullscale(float32_t *fs);
    bool getManufactureDate( sDate_t *manfDate);
    bool getUserCalDate(sDate_t* caldate);
    bool getFactoryCalDate (sDate_t* caldate);
    bool getSensorType( eSensorType_t *pSenType);
    bool sensorContinue(void);
    bool sensorRetry(void);
};

#endif // _DINSTRUMENT_H
