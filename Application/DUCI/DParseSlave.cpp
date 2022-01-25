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
* @file     DParseSlave.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The DUCI Slave parser class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DParseSlave.h"

/* Constants & Defines ----------------------------------------------------------------------------------------------*/
#define SLAVE_COMMANDS_ARRAY_SIZE  96  //this is the maximum no of commands supported in DUCI slave mode (can be increased if more needed)

/* Variables --------------------------------------------------------------------------------------------------------*/
sDuciCommand_t duciSlaveCommands[SLAVE_COMMANDS_ARRAY_SIZE];

/**
* @brief    Constructor
* @param    creator is the comms state that created this instance
* @param    commandArray is the array of commands created for the owning creator
* @param    maxCommands is the size of the commandArray
* @param    os_error is pointer to OS error
* @return   void
*/
DParseSlave::DParseSlave(void *creator, sDuciCommand_t *commandArray, size_t maxCommands, OS_ERR *os_error)
    : DParse(creator, os_error)
{
    commands = commandArray;
    capacity = maxCommands;

    stripTrailingChecksum = false;  //do not strip the trailing ":nn" characters from string as they could be data
}

/**
 * @brief   Check if the character is my start character
 * @param   ch - is the character
 * @return  true if ch is a start character for parser, else false
 */
bool DParseSlave::isMyStartCharacter(char ch)
{
    bool isMyStart = true;
    messageType = E_DUCI_COMMAND;

    switch(ch)
    {
    case '*':
    case '#':
        break;

    default:
        isMyStart = false;
        messageType = E_DUCI_UNEXPECTED;
        break;
    }

    return isMyStart;
}
