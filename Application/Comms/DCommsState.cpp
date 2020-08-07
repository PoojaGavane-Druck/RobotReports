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
* @file     DCommsState.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The communications class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsState.h"
#include "Utilities.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
sExternalDevice_t DCommsState::externalDevice = { 0 };
eStateComms_t DCommsState::commsOwnership = E_STATE_COMMS_OWNED;
eCommMasterInterfaceType_t DCommsState::currentWriteMaster = E_COMMS_MASTER_NONE;
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsState class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsState::DCommsState(DDeviceSerial *commsMedium)
{
    myCommsMedium = commsMedium;

    if (commsMedium != NULL)
    {
        myTxBuffer = myCommsMedium->getTxBuffer();
        myTxBufferSize = myCommsMedium->getTxBufferSize();
    }

    commandTimeoutPeriod = 500u; //default time in (ms) to wait for a response to a DUCI command

    commsOwnership = E_STATE_COMMS_OWNED;
    
    nextOperationMode = E_COMMS_READ_OPERATION_MODE;
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsState::createCommands(void)
{
   
}

void DCommsState::initialise(void)
{
}

/**
 * @brief   Suspend state machine
 * @param   void
 * @retval  void
 */
void DCommsState::suspend(void)
{
    commsOwnership = E_STATE_COMMS_REQUESTED;

    while (commsOwnership == (eStateComms_t)E_STATE_COMMS_REQUESTED)
    {
        //wait until request has been processed
        sleep(100u);
    }
}

/**
 * @brief   Resume state machine
 * @param   void
 * @retval  void
 */
void DCommsState::resume(void)
{
    commsOwnership = E_STATE_COMMS_OWNED;
}



void DCommsState::cleanup(void)
{
}

void DCommsState::clearRxBuffer(void) //Temporarily overriden - all comms has own buffer which base class could clear
{
    if (myCommsMedium != NULL)
    {
        myCommsMedium->clearRxBuffer();
    }
}


eCommOperationMode_t DCommsState::run(void)
{
  return E_COMMS_READ_OPERATION_MODE;
}
