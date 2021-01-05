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
* @file     DSensorBarometer.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     05 June 2020
*
* @brief    The barometer sensor base class header file
*/

#ifndef __DSENSOR_CHIP_BAROMETER_H
#define __DSENSOR_CHIP_BAROMETER_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensor.h"
#include "DRange.h"
#include "i2c.h"
/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DSensorChipBarometer : public DSensor
{
   eSensorError_t readByte(uint8_t RegAddr, uint8_t *value);
   eSensorError_t writeByte(uint8_t RegAddr, uint8_t value);
   eSensorError_t readPresureAndTemp(float32_t *pressure_hpa, float32_t *temp_Celcius);
   
private:
    uint32_t barometerIdentity;
    eI2CElement_t i2cn;
public:
    DSensorChipBarometer(void);

    virtual eSensorError_t initialise();
    virtual eSensorError_t close();
    virtual eSensorError_t measure();
    virtual void getIdentity(uint32_t *identity);
    virtual void setIdentity(uint32_t identity);
};

#endif /* __DSENSOR_BAROMETER_H */
