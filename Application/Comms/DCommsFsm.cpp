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
* @file     DCommsFsm.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The communications finite state machine base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsFsm.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsFsm class constructor
 * @param   void
 * @retval  void
 */
DCommsFsm::DCommsFsm(void)
{
}

/**
 * @brief   Create required states of the state machine
 * @param   commsMedium is pointer to serial comms medium
 * @retval  void
 */
void DCommsFsm::createStates(DDeviceSerial *commsMedium)
{
}

/**
 * @brief   DCommsFsm run function - once started, it runs forever
 * @param   void
 * @retval  void
 */
void DCommsFsm::run(void)
{
    myCurrentState = myInitialState;

    while (DEF_TRUE)
    {
        myCurrentState = myStateArray[myCurrentState]->run();
    }
}

/**
 * @brief   Suspend state machine
 * @param   void
 * @retval  void
 */
void DCommsFsm::suspend(void)
{
    myStateArray[myCurrentState]->suspend();
}

/**
 * @brief   Resume state machine
 * @param   void
 * @retval  void
 */
void DCommsFsm::resume(void)
{
    myStateArray[myCurrentState]->resume();
}

/**
 * @brief   Get external device connection status
 * @param   void
 * @retval  pointer to for returning connected device status
 */
sExternalDevice_t *DCommsFsm::getConnectedDeviceInfo(void)
{
    return &DCommsState::externalDevice;
}
