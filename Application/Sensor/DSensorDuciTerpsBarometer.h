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
* @file     DSensorDuciTERPS.h
* @version  1.00.00
* @author   Nageswara Rao P
* @date     07 July 2020
*
* @brief    The PM705E (DPS5000 RS485) sensor base class header file
*/

#ifndef __DSENSOR_TERPS_H
#define __DSENSOR_TERPS_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensorDuci.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DSensorDuciTerpsBarometer : public DSensorDuci
{
private:
    //

protected:
    virtual void createDuciCommands(void);

public:
    DSensorDuciTerpsBarometer(void);
};

#endif /* __DSENSOR_PM705E_H */
