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
* @file     DCommsStateDevDiscovery.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     14 April 2020
*
* @brief    The communications external device discovery state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDevDiscovery.h"
#include "DParseMaster.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateDevDiscovery class constructor
 * @param   commsMedium is pointer to
 * @retval  void
 */
DCommsStateDevDiscovery::DCommsStateDevDiscovery(DDeviceSerial *commsMedium)
: DCommsState(commsMedium)
{
    OS_ERR os_error;

    myParser = new DParseMaster((void *)this, &os_error);

    createDuciCommands();
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for this class
 * @note    In this state this function periodically reads device ID and serial number. If the connected device is
 *          valid then it stays in this state until the instrument function requests use of the connected sensor; at
 *          which point control of comms will be handed over to that function. In the meantime, it is important to
 *          keep polling like this so we know it is still connected. Disconnection will force the nextState to revert
 *          back to 'local'. The polling is reduced in frequency.
 * @param   void
 * @retval  duciError is the DUCI status
 */
eStateDuci_t DCommsStateDevDiscovery::run(void)
{
    OS_ERR os_err;
    char *buffer;

    //Entry
    errorStatusRegister.value = 0u; //clear DUCI error status register

    //clear connected device status
    externalDevice.status.all = 0u;         //start with cleared status
    externalDevice.status.connected = 1u;   //must be connected to get here

    if (isDeviceSupported() == true)
    {
        externalDevice.status.supported = 1u;
    }

    sDuciError_t duciError;         //local variable for error status
    duciError.value = 0u;

    //DO - just poll for serial number to (a) get he serial number and then (b) check that connection remains made
    nextState = E_STATE_DUCI_DEVICE_DISCOVERY;

    while (nextState == E_STATE_DUCI_DEVICE_DISCOVERY)
    {
        if (commsOwnership == E_STATE_COMMS_REQUESTED)
        {
            commsOwnership = E_STATE_COMMS_RELINQUISHED;
        }
        else if (commsOwnership == E_STATE_COMMS_RELINQUISHED)
        {
            nextState = E_STATE_DUCI_EXTERNAL;
        }
        else
        {
            //read serial number
            if (query("#SN?", &buffer) == true)
            {
                duciError = myParser->parse(buffer);

                errorStatusRegister.value |= duciError.value;

                //Wait 1 second and then check if still connected //TODO Wait for task message to hand over control
                OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
            }
            else //TODO: if timeout
            {
                //go back to local state as we have disconnected
                nextState = E_STATE_DUCI_LOCAL;
            }
        }
    }

    //Exit

    return nextState;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")

/**
 * @brief   Check the connected external device is supported
 * @note    Connected device DK and firmware version is already updated before entry into this state
 * @param   void
 * @return  flag - true if supported, else false
 */
bool DCommsStateDevDiscovery::isDeviceSupported(void)
{
    bool flag = true;

    switch (externalDevice.dk)
    {
        case 481u: //external pressure sensor (DPS5000/PM705E)
            break;

        case 483u: //RTD probe interface
            break;

        default:
            flag = false;
            break;
    }

    return flag;
}

sDuciError_t DCommsStateDevDiscovery::fnSetSN(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_REPLY)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //save current serial number
        uint32_t lastSerialNumber = externalDevice.serialNumber;

        //update the value
        externalDevice.serialNumber = (uint32_t)parameterArray[1].intNumber;

        //if serial number has changed since the last read then sensor has been switched so we need to go back
        //to local state and start again with the identification process
        if (externalDevice.status.identified == 1u)
        {
            if (lastSerialNumber != externalDevice.serialNumber)
            {
                //go back to local state as we have disconnected and reconnected to a different device
                nextState = E_STATE_DUCI_LOCAL;
            }
        }
        else
        {
            //mark the serial number as having been read
            externalDevice.status.identified = 1u;

            //user interface needs to be informed of this status change
            //TODO: DPI610E->userInterface->sensorConnected(true); <- make this a class function
        }
    }

    return duciError;
}
