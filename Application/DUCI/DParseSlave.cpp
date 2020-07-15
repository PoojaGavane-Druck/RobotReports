/**
* BHGE Confidential
* Copyright 2020. Baker Hughes.
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

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

/* Constants & Defines ----------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/**
* @brief	Constructor
* @param    osErr is pointer to OS error
* @return   void
*/
DParseSlave::DParseSlave(void *creator, OS_ERR *osErr)
: DParse(creator, osErr)
{
}

/**
 * @brief   Check if the character is my start character
 * @param   ch - is the character
 * @return  true if ch is a start character for parser, else false
 */
bool DParseSlave::isMyStartCharacter(char ch)
{
    bool isMyStart = true;
    echoCommand = false;
    messageType = E_DUCI_COMMAND;

    switch (ch)
    {
        case '*':
            echoCommand = true;
            break;

        case '#':
            break;

        default:
            isMyStart = false;
            messageType = E_DUCI_UNEXPECTED;
            break;
    }

    return isMyStart;
}
