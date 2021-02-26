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
* @file     DCommsStateRemoteBluetooth.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     13 November 2020
*
* @brief    The Bluetooth communications remote state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateRemoteBluetooth.h"
#include "DDPI610E.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateRemoteBluetooth class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateRemoteBluetooth::DCommsStateRemoteBluetooth(DDeviceSerial *commsMedium, DTask *task)
    : DCommsStateDuci(commsMedium, task)
{
    //get reference to the remote mode state (singleton) function
    
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for this class
 * @param   void
 * @retval  next state
 */
eCommOperationMode_t DCommsStateRemoteBluetooth::run(void)
{
    OS_ERR os_err;
    uint8_t* buffer;
    sInstrumentMode_t mask;
    mask.value = 0u;
    //    mask.test = 1u;
    mask.remoteSerial = 1u;
    //    mask.remoteUsb = 1u;

        //Entry
    PV624->userInterface->clearMode(mask);

    errorStatusRegister.value = 0u; //clear DUCI error status register

    externalDevice.status.all = 0u;


    
    while (nextOperationMode == E_COMMS_WRITE_OPERATION_MODE)
    {
        if (commsOwnership == E_STATE_COMMS_REQUESTED)
        {
            commsOwnership = E_STATE_COMMS_RELINQUISHED;
        }

        //Polling for external connection every 500 ms.
        OSTimeDlyHMSM(0u, 0u, 0u, 500u, OS_OPT_TIME_HMSM_STRICT, &os_err);

        if (commsOwnership == E_STATE_COMMS_OWNED)
        {
            //add query command checksum explicitly here as this is only required for outgoing; reply may or may not have one
             //if (waitForCommand(&buffer))
            if (0u)
            {
                //errorStatusRegister.value |= owiError.value;

                if (errorStatusRegister.value != 0u)
                {
                    //TODO: Handle Error
                }
            }
        }
    }



    return nextOperationMode;

   
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")
