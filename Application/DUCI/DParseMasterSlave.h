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
* @file     DParseMasterSlave.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     08 April 2020
*
* @brief    The DUCI Master/Slave message parser class header file
*/

#ifndef __DPARSE_MASTER_SLAVE_H
#define __DPARSE_MASTER_SLAVE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DParse.h"

/* Defines  ---------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

class DParseMasterSlave : public DParse
{
public:
    DParseMasterSlave(void *creator, sDuciCommand_t *commandArray, size_t maxCommands, OS_ERR *os_error);
};

#endif /* __DPARSE_MASTER_SLAVE_H */
