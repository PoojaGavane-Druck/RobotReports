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
* @file     DParseMaster.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The DUCI Master parser class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DParseMaster.h"

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
DParseMaster::DParseMaster(void *creator, sDuciCommand_t *commandArray, size_t maxCommands, OS_ERR *os_error)
    : DParse(creator, os_error)
{
    commands = commandArray;
    capacity = maxCommands;

    checksumEnabled = true;     //by default, use checksum in DUCI master mode
    terminatorCrLf = false;     //use 'LF only' terminator
}

/**
 * @brief   Check if the character is my start character
 * @param   ch - is the character
 * @return  true if ch is a start character for parser, else false
 */
bool DParseMaster::isMyStartCharacter(char ch)
{
    bool isMyStart = false;
    messageType = E_DUCI_UNEXPECTED;

    if(ch == '!')
    {
        isMyStart = true;
        messageType = E_DUCI_REPLY;
    }

    return isMyStart;
}

