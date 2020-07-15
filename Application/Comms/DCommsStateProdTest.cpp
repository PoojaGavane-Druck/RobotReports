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
* @file     DCommsStateProdTest.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The communications production test state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateProdTest.h"
#include "DParseSlave.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateProdTest class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateProdTest::DCommsStateProdTest(DDeviceSerial *commsMedium)
: DCommsState(commsMedium)
{
    OS_ERR os_error;
    myParser = new DParseSlave((void *)this, &os_error);
}

/**
 * @brief   Run function for this class
 * @param   void
 * @retval  void
 */
eStateDuci_t DCommsStateProdTest::run(void)
{
    sInstrumentMode_t mask;
    mask.value = 0u;
    mask.test = 1u;

    //Entry
    PV624->userInterface->setMode(mask);

    nextState = E_STATE_DUCI_PROD_TEST;

    errorStatusRegister.value = 0u;   //clear DUCI error status register
    externalDevice.status.all = 0u;

    //Exit
    PV624->userInterface->clearMode(mask);

    return E_STATE_DUCI_LOCAL;
}

/**
 * @brief   Create DUCI command set
 * @param   void
 * @return  void
 */
void DCommsStateProdTest::createDuciCommands(void)
{
    //create common commands - that apply to all states
    DCommsState::createDuciCommands();
}

