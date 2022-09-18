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
* @file     DSensorExternal.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     08 June 2020
*
* @brief    The external sensor base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensorExternal.h"

MISRAC_DISABLE
#include <stdio.h>
#include <math.h>
MISRAC_ENABLE

/* Error handler instance parameter starts from 4501 to 4600 */

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSensorExternal class constructor
 * @param   void
 * @retval  void
 */
DSensorExternal::DSensorExternal()
    : DSensor()
{
}

/**
 * @brief   DSensorExternal class destructor
 * @param   void
 * @retval  void
 */
DSensorExternal::~DSensorExternal(void)
{

}
/*
 * @brief Read App id info of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorExternal::readAppIdentity(void)
{
    return E_SENSOR_ERROR_FAULT;
}

/*
 * @brief Read Bootloader id info of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorExternal::readBootLoaderIdentity(void)
{
    return E_SENSOR_ERROR_FAULT;
}
/*
 * @brief Read serial number of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorExternal::readSerialNumber(void)
{
    return E_SENSOR_ERROR_FAULT;
}

/*
 * @brief Read upper fullscale and type of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorExternal::readFullscaleAndType(void)
{
    return E_SENSOR_ERROR_FAULT;
}

/*
 * @brief Read lower fullscale and type of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorExternal::readNegativeFullscale(void)
{
    return E_SENSOR_ERROR_FAULT;
}

/*
 * @brief Read cal date of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorExternal::readCalDate()
{
    return E_SENSOR_ERROR_FAULT;
}

/*
 * @brief Read manufacture date of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorExternal::readManufactureDate()
{
    return E_SENSOR_ERROR_FAULT;
}

/*
 * @brief Read cal interval of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorExternal::readCalInterval()
{
    return E_SENSOR_ERROR_FAULT;
}
