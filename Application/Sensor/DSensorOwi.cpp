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
* @file     DSensorOwi.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     14 April 2020
*
* @brief    The DUCI sensor class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <rtos.h>
MISRAC_ENABLE

#include "DSensorOwi.h"
#include "DDeviceSerial.h"
#include "DSensorOwiAmc.h"
#include "DDeviceSerialOwiInterface1.h"
#include "DDeviceSerialOwiInterface2.h"
#include "DOwiParse.h"
#include "DPV624.h"

/* Error handler instance parameter starts from 4601 to 4700 */

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSensorDuci class constructor
 * @param   comms is the reference to the comms instance for this sensor
 * @retval  void
 */
DSensorOwi::DSensorOwi(OwiInterfaceNo_t interfaceNumber)
    : DSensorExternal()
{
    myInterfaceNumber = interfaceNumber;
    memset_s((uint8_t *)&connectedDevice, sizeof(sExternalDevice_t), 0, sizeof(sExternalDevice_t));
    myComms = NULL;
    myTxBuffer = NULL;
    myTxBufferSize = 0u;
    myParser = NULL;
}

/**
 * @brief   Initialisation function
 * @param   void
 * @retval  sensor error code
 */
eSensorError_t DSensorOwi::initialise()
{
    OS_ERR os_error;

    if(OWI_INTERFACE_1 == myInterfaceNumber)
    {
        myComms = (DDeviceSerial *) new DDeviceSerialOwiInterface1();
        myDevice = myComms; //in case some client wants to interrogate

    }

    else if(OWI_INTERFACE_2 == myInterfaceNumber)
    {
        myComms = (DDeviceSerial *) new DDeviceSerialOwiInterface2();
        myDevice = myComms; //in case some client wants to interrogate
    }

    else
    {

    }

    myTxBuffer = (uint8_t *)myComms->getTxBuffer();
    myTxBufferSize = myComms->getTxBufferSize();

    /* changed to 10000 for testing original value - 500 - Makarand - TDOD */
    commandTimeoutPeriod = 200u;

    if(myParser == NULL)
    {
        myParser = new DOwiParse((void *)this, &os_error);
        createOwiCommands();
    }


    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Close sensor
 * @param   void
 * @retval  error code
 */
eSensorError_t DSensorOwi::close(void)
{

    if(NULL != myParser)
    {
        delete myParser;
    }

    if(NULL != myComms)
    {
        delete myComms;
    }

    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Create OWI command set - the common commands - that apply to all OWI sensors
 * @param   void
 * @return  void
 */
void DSensorOwi::createOwiCommands(void)
{

}

/**
 * @brief Read App id info of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readAppIdentity(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief Read Bootloader id info of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readBootLoaderIdentity(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Send query command Owi sensor
 * @param   command number
 * @return  sensor error code
 */
eSensorError_t DSensorOwi::sendQuery(uint8_t cmd)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;

    sOwiError_t owiError;
    owiError.value = 0u;
    uint8_t *buffer;
    uint32_t responseLength = 0u;
    unsigned int cmdLength;

    myTxBuffer[0] =  OWI_SYNC_BIT | OWI_TYPE_BIT | cmd ;

    //prepare the message for transmission
    myParser->CalculateAndAppendCheckSum((uint8_t *) myTxBuffer, 1u, &cmdLength);

    myParser->getResponseLength(cmd, &responseLength);

    if((E_AMC_SENSOR_CMD_READ_COEFFICIENTS == cmd) ||
            (E_AMC_SENSOR_CMD_READ_CAL_DATA == cmd)
      )
    {
        commandTimeoutPeriod = 5000u;
    }

    else
    {
        commandTimeoutPeriod = 200u;
    }

    if(myComms->query(myTxBuffer, cmdLength, &buffer, responseLength, commandTimeoutPeriod) == true)
    {
        if(0u == responseLength)
        {
            myComms->getRcvBufLength((uint16_t *)(&responseLength));
        }

        owiError = myParser->parse(cmd, buffer, responseLength);

        //if this transaction is ok, then we can use the received value
        if(owiError.value != 0u)
        {
            sensorError = E_SENSOR_ERROR_COMMAND;
        }
    }

    else
    {
        sensorError = E_SENSOR_ERROR_COMMS;
    }

    commandTimeoutPeriod = 200u;
    return sensorError;
}




/**
 * @brief Read Owi sensor error status
 * @param void
 * @return sensor error code
 */

eSensorError_t DSensorOwi::readStatus(void)
{
    return E_SENSOR_ERROR_NONE;
}


/**
 * @brief Read serial number of Owi sensor
 * @param void
 * @return sensor error code
 */

eSensorError_t DSensorOwi::readSerialNumber(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief Read upper fullscale and type of Owi sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readFullscaleAndType(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief Read lower fullscale and type of Owi sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readNegativeFullscale(void)
{
    return E_SENSOR_ERROR_NONE;
}


/**
 * @brief Read measured value of Owi sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::measure()
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief Read cal date of Owi sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readCalDate()
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief Read manufacture date of Owi sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readManufactureDate()
{
    return E_SENSOR_ERROR_NONE;
}


/**
 * @brief Read cal interval of Owi sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readCalInterval()
{
    return E_SENSOR_ERROR_NONE;
}


/**
 * @brief   Set Owi sensor cal mode
 * @param   void
 * @return  error code
 */
eSensorError_t DSensorOwi::writeCalMode(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Exit sensor cal mode
 * @param   void
 * @return  error code
 */
eSensorError_t DSensorOwi::writeCalModeExit(void)
{
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief Sets the Last Cal date
 * @param eDateTime
 * @return sensor error code
 */
eSensorError_t DSensorOwi::writeCalDate(RTC_DateTypeDef eDateTime)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    return sensorError;
}

/**
 * @brief reads the offset value from Owi sensor
 * @param pZero
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readZero(float32_t *pZero)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    return sensorError;
}

/**
 * @brief sets Owi sensor's Calibration Interval in days
 * @param Cal interval in days
 * @return sensor error code
 */
eSensorError_t DSensorOwi::writeCalInterval(uint32_t calInterval)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
    return sensorError;
}
