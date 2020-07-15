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
* @file     DParseSlave.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The DUCI Slave message parser class header file
*/

#ifndef __DPARSE_SLAVE_H
#define __DPARSE_SLAVE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DParse.h"

/* Defines  ---------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DParseSlave : public DParse
{
protected:
    virtual bool isMyStartCharacter(char ch);

public:
    DParseSlave(void *creator, OS_ERR *osErr);
};

#endif /* __DPARSE_SLAVE_H */
