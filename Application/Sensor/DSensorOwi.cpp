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
* @file     DSensorDuci.cpp
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
#include <os.h>
MISRAC_ENABLE

#include "DSensorOwi.h"
#include "DDeviceSerial.h"
#include "DDeviceSerialOwiInterface1.h"
#include "DDeviceSerialOwiInterface2.h"
#include "DOwiParse.h"
#include "DPV624.h"

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
      myComms = (DDeviceSerial*) new DDeviceSerialOwiInterface1();
      myDevice = myComms; //in case some client wants to interrogate

    }
    else if(OWI_INTERFACE_2 == myInterfaceNumber)
    {
      myComms = (DDeviceSerial*) new DDeviceSerialOwiInterface2();
      myDevice = myComms; //in case some client wants to interrogate
    }
    else
    {
      
    }
        
    myTxBuffer = (uint8_t*)myComms->getTxBuffer();
    myTxBufferSize = myComms->getTxBufferSize();

    /* changed to 10000 for testing original value - 500 - Makarand - TDOD */
    commandTimeoutPeriod = 1250u;
    
    if (myParser == NULL)
    {
        myParser = new DOwiParse((void *)this, &os_error);
        createOwiCommands();
    }

   
    return E_SENSOR_ERROR_NONE;
}

/**
 * @brief   Close sensor
 * @param   void
 * @retval  void
 */
eSensorError_t DSensorOwi::close(void)
{
    //grab the serial comms
  
    /* This is disbled because transit from one function to another
      is not working if enabled, Root cause ? */
    //PV624->serialComms->release(this);
  
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
 * @brief   Create DUCI command set - the common commands - that apply to all DUCI sensors
 * @param   void
 * @return  void
 */
void DSensorOwi::createOwiCommands(void)
{
    
}

/*
 * @brief Read App id info of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readAppIdentity(void)
{
  return E_SENSOR_ERROR_NONE;
}

/*
 * @brief Read Bootloader id info of external sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readBootLoaderIdentity(void)
{
  return E_SENSOR_ERROR_NONE;
}
/*
 * @brief   Send query command Owi sensor
 * @param   command string
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
    myParser->CalculateAndAppendCheckSum((uint8_t*) myTxBuffer, 1u, &cmdLength);  
    
    myParser->getResponseLength(cmd, &responseLength);
    
    if (myComms->query(myTxBuffer, cmdLength, &buffer, responseLength, commandTimeoutPeriod) == true)
    {
        if((uint32_t)(0) == responseLength)
        {
            myComms->getRcvBufLength((uint16_t*)(&responseLength));
        }
        owiError = myParser->parse(cmd, buffer,responseLength);

        //if this transaction is ok, then we can use the received value
        if (owiError.value != 0u)
        {
            sensorError = E_SENSOR_ERROR_COMMAND;
        }
    }
    else
    {
        sensorError = E_SENSOR_ERROR_COMMS;
    }

    return sensorError;
}



/* Public function --------------------------------------------------------------------------------------------------*/


// read sensor error status********************************************************************************************
/*
 * @brief Read DUCI sensor error status
 * @param void
 * @return sensor error code
 */

eSensorError_t DSensorOwi::readStatus(void)
{
    return E_SENSOR_ERROR_NONE;
}





// read serial number *************************************************************************************************
/*
 * @brief Read serial number of DUCI sensor
 * @param void
 * @return sensor error code
 */

eSensorError_t DSensorOwi::readSerialNumber(void)
{
    return E_SENSOR_ERROR_NONE;
}



// read fullscale and type ********************************************************************************************
/*
 * @brief Read upper fullscale and type of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readFullscaleAndType(void)
{
    return E_SENSOR_ERROR_NONE;
}

/*
 * @brief Read lower fullscale and type of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readNegativeFullscale(void)
{
    return E_SENSOR_ERROR_NONE;
}





// read measured value ************************************************************************************************
/*
 * @brief Read measured value of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::measure()
{
    return E_SENSOR_ERROR_NONE;
}



// read cal date ******************************************************************************************************
/*
 * @brief Read cal date of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readCalDate()
{
    return E_SENSOR_ERROR_NONE;
}

/*
 * @brief Read manufacture date of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readManufactureDate()
{
    return E_SENSOR_ERROR_NONE;
}





// read cal interval **************************************************************************************************
/*
 * @brief Read cal interval of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readCalInterval()
{
    return E_SENSOR_ERROR_NONE;
}





//*********************************************************************************************************************
/**
 * @brief   Set sensor cal mode
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

/*
 * @brief Sets the Last Cal date
 * @param eDateTime
 * @return sensor error code
 */
eSensorError_t DSensorOwi::writeCalDate(RTC_DateTypeDef eDateTime)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    return sensorError;
}
//
///*
// * @brief not implemented for this code
// * @param dValue
// * @return sensor error code
// */
//eSensorError_t DSensorOwi::setCalPoint(float32_t dValue )
//{
//    return E_SENSOR_ERROR_NONE;
//}
//
///*
// * @brief Sets up a new zero value in offset register of sensor after checking that the zero value doesn't exceed +/- 5% of full scale
//  * @return sensor error code
// */
//eSensorError_t DSensorOwi::performZero(void)
//{
//    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
//    return sensorError;
//}
//
///*
// * @brief   Stores the new Calibrated gain and offset in the sensor register
// * @note    3-pt cal not supported
// * @return  sensor error code
// */
//eSensorError_t DSensorOwi::calAccept(void)
//{
//    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
//    return sensorError;
//}
//
///*
// * @brief restores the old gain and offset value upon user abort of calibration
// * @return sensor error code
// */
//eSensorError_t DSensorOwi::calAbort(void)
//{
//    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
//    return sensorError;
//}

/*
 * @brief reads the offset value from DUCI sensor
 * @param pZero
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readZero(float32_t *pZero )
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    return sensorError;
}

/*
 * @brief sets DUCI sensor's Calibration Interval in days
 * @param Cal interval in days
 * @return sensor error code
 */
eSensorError_t DSensorOwi::writeCalInterval(uint32_t calInterval)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
    return sensorError;
}

