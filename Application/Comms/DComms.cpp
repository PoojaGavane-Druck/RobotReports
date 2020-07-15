/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DComms.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The communications class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DComms.h"

MISRAC_DISABLE
#include <os.h>
MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define COMMS_TASK_STK_SIZE   512u    //this is not bytes (CPU_STK is 4 bytes, so multiply by 4 for stack size in bytes)

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DComms class constructor
 * @param   void
 * @retval  void
 */
DComms::DComms()
: DTask()
{
    //safe to 'new' a stack here as it is never 'free'd.
    CPU_STK_SIZE stackBytes = COMMS_TASK_STK_SIZE * (CPU_STK_SIZE)sizeof(CPU_STK_SIZE);
    myTaskStack = (CPU_STK *)new char[stackBytes];
}

/**
 * @brief   DComms class constructor
 * @param   mediumName
 * @param   osErr
 * @retval  void
 */
void DComms::start(char *mediumName, OS_ERR *osErr)
{
    //start off with no comms state machine specified
    myCommsFsm = NULL;

    activate(mediumName, (CPU_STK_SIZE)COMMS_TASK_STK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)10u, osErr);
}

/**
 * @brief   DComms initialisation function
 * @param   void
 * @retval  void
 */
void DComms::initialise(void)
{
}

/**
 * @brief   DCommsSerial run function
 * @param   void
 * @retval  void
 */
void DComms::runFunction(void)
{
    if (myCommsFsm != NULL)
    {
        myCommsFsm->createStates(commsMedium);
        myCommsFsm->run();
    }
}

/**
 * @brief   Get comms medium
 * @param   void
 * @retval  commsMedium - returns the pointer to its comms medium
 */
DDeviceSerial *DComms::getMedium(void)
{
    return commsMedium;
}

/**
 * @brief   Get external device connection status
 * @param   device is a pointer to for returned connected device status
 * @retval  void
 */
void DComms::getConnectedDeviceInfo(sExternalDevice_t *device)
{
    sExternalDevice_t *connectionStatus = myCommsFsm->getConnectedDeviceInfo();

    device->status = connectionStatus->status;
    device->dk = connectionStatus->dk;
    device->version = connectionStatus->version;
    device->serialNumber = connectionStatus->serialNumber;
}

/**
 * @brief   Grab (lock) comms medium resource
 * @param   sensor is pointer to a sensor instance
 * @retval  flag to indicate success (true) or failure (false) of the request
 */
bool DComms::grab(DSensor *sensor)
{
    bool flag = false;

    if (myCommsFsm != NULL)
    {
        myCommsFsm->suspend();
        flag = true;
    }

    return flag;
}

/**
 * @brief   Release comms medium resource
 * @param   sensor is pointer to a sensor instance
 * @retval  flag to indicate success (true) or failure (false) of the request
 */
bool DComms::release(DSensor *sensor)
{
    bool flag = false;

    if (myCommsFsm != NULL)
    {
        myCommsFsm->resume();
        flag = true;
    }

    return flag;
}

