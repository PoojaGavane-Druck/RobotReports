/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DParseMasterSlave.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     08 April 2020
*
* @brief    The DUCI Master/Slave parser class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DParseMasterSlave.h"

/* Constants & Defines ----------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/**
* @brief    Constructor
* @param    creator is the comms state that created this instance
* @param    commandArray is the array of commands created for the owning creator
* @param    maxCommands is the size of the commandArray
* @param    os_error is pointer to OS error
* @return   void
*/
DParseMasterSlave::DParseMasterSlave(void *creator, sDuciCommand_t *commandArray, size_t maxCommands, OS_ERR *os_error)
    : DParse(creator, os_error)
{
    commands = commandArray;
    capacity = maxCommands;

    terminatorCrLf = false;     //use 'LF only' terminator
}
