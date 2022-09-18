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
* @file     DCommsUSB.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     26 May 2020
*
* @brief    The USB communications class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsSerial.h"
#include "DDeviceSerialInterface.h"
#include "DCommsFsmSerial.h"

/* Error handler instance parameter starts from 1001 to 1100 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsOwi class constructor
 * @param   mediumName
 * @retval  osError
 */
DCommsSerial::DCommsSerial(char *mediumName, OS_ERR *osErr)
    : DComms()
{
    myName = mediumName;

    //start the comms task
    start(mediumName, osErr);
}

/**
 * @brief   DCommsOwi class destructor
 * @param   void
 * @retval  void
 */
DCommsSerial::~DCommsSerial(void)
{
    if(NULL != commsMedium)
    {
        delete commsMedium;
    }

    if(NULL != myCommsFsm)
    {
        delete myCommsFsm;
    }
}
/**
 * @brief   DCommsOwi initialisation function (overrides the base class)
 * @param   void
 * @retval  void
 */
void DCommsSerial::initialise(void)
{
    //specify the comms medium
    commsMedium = new DDeviceSerialInterface();

    //create the comms state machine- having setup the medium first
    myCommsFsm = new DCommsFsmSerial();
}
