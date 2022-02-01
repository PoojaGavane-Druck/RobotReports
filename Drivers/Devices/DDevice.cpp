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
* @file     DDevice.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The device driver base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DDevice.h"

MISRAC_DISABLE
#include <stdio.h>
MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DDevice class constructor
 * @param   void
 * @retval  void
 */
DDevice::DDevice()
{
    opened = false; //can't be opened already
}

/**
 * @brief   DDevice class destructor
 * @param   void
 * @retval  void
 */
DDevice::~DDevice()
{
}

/**
 * @brief   Create mutex (resource lock) for this device
 * @param   pointer to character string representing name of mutex
 * @retval  flag - true if created ok, else false
 */
bool DDevice::createMutex(char *name)
{
    bool flag = false;

    OS_ERR os_error = (OS_ERR)OS_ERR_NONE;
    RTOSMutexCreate(&myMutex, (CPU_CHAR *)name, &os_error);

    if(os_error != (OS_ERR)OS_ERR_NONE)
    {
        flag = false;
    }

    return flag;
}

/**
 * @brief   Open driver and initialise as required
 * @param   void
 * @retval  void
 */
void DDevice::open(void)
{
    opened = true;
}

/**
 * @brief   Close driver and gracefully shutdown and free resources as appropriate
 * @param   void
 * @retval  void
 */
void DDevice::close()
{
    opened = false;
}

/**
 * @brief   Query if device driver is already opened
 * @param   void
 * @retval  true if already opened, else false
 */
bool DDevice::isOpen()
{
    return opened;
}

/**
 * @brief   Perform device specific self-test
 * @param   void
 * @retval  true if test passed, else false
 */
bool DDevice::selfTest(void)
{
    return true;
}

