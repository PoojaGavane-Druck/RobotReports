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
* @file     DCommsFsmSerial.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 June 2020
*
* @brief    The communications finite state machine class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsFsmSerial.h"

MISRAC_DISABLE
#include <lib_def.h>
MISRAC_ENABLE

#include "Utilities.h"
#include "DCommsStateEngPro.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

//#define ENG_ALGO_TESTING
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsFsmUsb class constructor
 * @param   void
 * @retval  void
 */
DCommsFsmSerial::DCommsFsmSerial(void)
    : DCommsFsm()
{
}

/**
 * @brief   DCommsFsmUsb class destructor
 * @param   void
 * @retval  void
 */
DCommsFsmSerial::~DCommsFsmSerial(void)
{
    for(uint32_t index = (uint32_t)E_STATE_DUCI_LOCAL;
            index < (uint32_t)E_STATE_DUCI_SIZE;
            index++)
    {
        if(NULL != myStateArray[index])
        {
            delete  myStateArray[index];
        }
    }

    delete[] myStateArray;
}

/**
 * @brief   Create required states of the state machine
 * @param   commsMedium is pointer to serial comms medium
 * @param   task is  pointer to own task
 * @retval  void
 */
void DCommsFsmSerial::createStates(DDeviceSerial *commsMedium, DTask *task)
{

    //create all the states of the 'finite state machine'
    uint32_t sizeOfMyStateArray = sizeof(myStateArray) / sizeof(DCommsState *);

    if(E_STATE_DUCI_LOCAL < sizeOfMyStateArray)
    {
        myStateArray[E_STATE_DUCI_LOCAL] = new DCommsStateEngPro(commsMedium, task);
    }

    if(E_STATE_DUCI_REMOTE < sizeOfMyStateArray)
    {
        myStateArray[E_STATE_DUCI_REMOTE] = NULL;
    }

    if(E_STATE_DUCI_PROD_TEST < sizeOfMyStateArray)
    {
        myStateArray[E_STATE_DUCI_PROD_TEST] =  NULL;
    }



    //always starts in local mode (DUCI master)
    myInitialState = E_STATE_DUCI_LOCAL;
}
