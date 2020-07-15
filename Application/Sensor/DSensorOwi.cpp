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
      myComms = new DDeviceSerailOwiInterface1();
      myDevice = myComms; //in case some client wants to interrogate

    }
    else if(OWI_INTERFACE_2 == myInterfaceNumber)
    {
      myComms = new DDeviceSerailOwiInterface2();
      myDevice = myComms; //in case some client wants to interrogate
    }
    else
    {
      
    }
        
    myTxBuffer = myComms->getTxBuffer();
    myTxBufferSize = myComms->getTxBufferSize();

    commandTimeoutPeriod = 500u;

    if (myParser == NULL)
    {
        myParser = new DOwiParse(void *)this, &os_error);
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
    PV624->serialComms->release(this);

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
 * @brief   Send query command Owi sensor
 * @param   command string
 * @return  sensor error code
 */
eSensorError_t DSensorOwi::sendQuery(int8_t cmd)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;

    sOwiError_t owiError;
    owiError.value = 0u;
    char *buffer;
    
    uint32_t responseLength;
    uint32_t cmdLength;
    myTxBuffer[0] = cmd;
    //prepare the message for transmission
    myParser->CalculateAndAppendCheckSum( myTxBuffer, myTxBufferSize, cmdLength);

    myParser->getResponseLength(cmd, responseLength);
    //read serial number
    if (myComms->query(myTxBuffer, cmdLength, &buffer, responseLength, commandTimeoutPeriod) == true)
    {
        owiError = myParser->parse(buffer);

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

/*
 * @brief   Send command to DUCI sensor (with no reply expected)
 * @param   command string
 * @return  sensor error code
 */
eSensorError_t DSensorOwi::sendCommand(char *cmd)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;

    //send command
    myParser->prepareTxMessage(cmd, myTxBuffer, myTxBufferSize);

    //read serial number
    if (myComms->sendString(myTxBuffer) == false)
    {
        sensorError = E_SENSOR_ERROR_COMMS;
    }

    return sensorError;
}

/* Public function --------------------------------------------------------------------------------------------------*/
// read sensor identification information *****************************************************************************
/*
 * @brief Read id info of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorOwi::readIdentity(void)
{
    return sendQuery("#RI?");
}

/*
 * @brief   Handle id info of DUCI sensor
 * @param   pointer to sensor instance
 * @param   parsed array of parameters
 * @return  sensor error code
 */
sDuciError_t DSensorOwi::fnSetRI(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DSensorDuci *myInstance = (DSensorDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetRI(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/*
 * @brief   Handle id info of this instance of DUCI sensor //TODO: Repetition of code (same as DCommsStateLocal.cpp)
 * @param   parsed array of parameters
 * @return  sensor error code
 */
sDuciError_t DSensorOwi::fnSetRI(sDuciParameter_t * parameterArray)
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
        //get first parameter, which should be "DKnnnn" where n is a digit 0-9
        char *str = parameterArray[1].charArray;
        size_t size = strlen(str);

        if (size != (size_t)6)
        {
            duciError.invalid_args = 1u;
        }
        else
        {
            //check that the start characters are "DK"
            if (strncmp(str, "DK", (size_t)2u) != 0)
            {
                duciError.invalid_args = 1u;
            }
            else
            {
                uSensorIdentity_t sensorId;
                int32_t intValue;

                //skip over the two start characters
                str += 2;
                char *endptr;

                //expects exactly 4 digits after the "DK"
                duciError = myParser->getIntegerArg(str, &intValue, 4u, &endptr);

                sensorId.dk = (uint32_t)intValue;

                //if first parameter is ok then check version parameter
                if (duciError.value == 0u)
                {
                    str = parameterArray[2].charArray;

                    size = strlen(str);

                    //expects exactly 9 characters: Vnn.nn.nn (where 'n' is a digit '0'-'9'
                    if (size != (size_t)9)
                    {
                        duciError.invalid_args = 1u;
                    }
                    else
                    {
                        //check that the next characters is 'V'
                        if (*str++ != 'V')
                        {
                            duciError.invalid_args = 1u;
                        }
                        else
                        {
                            //expects exactly 2 digits next
                            duciError = myParser->getIntegerArg(str, &intValue, 2u, &endptr);

                            if (duciError.value == 0u)
                            {
                                sensorId.major = (uint32_t)intValue;

                                str = endptr;

                                //check that the next characters is '.'
                                if (*str++ != '.')
                                {
                                    duciError.invalid_args = 1u;
                                }
                                else
                                {
                                    //expects exactly 2 digits next
                                    duciError = myParser->getIntegerArg(str, &intValue, 2u, &endptr);

                                    if (duciError.value == 0u)
                                    {
                                        sensorId.minor = (uint32_t)intValue;

                                        str = endptr;

                                        //check that the next characters is '.'
                                        if (*str++ != '.')
                                        {
                                            duciError.invalid_args = 1u;
                                        }
                                        else
                                        {
                                            //expects exactly 2 digits next
                                            duciError = myParser->getIntegerArg(str, &intValue, 2u, &endptr);

                                            if (duciError.value == 0u)
                                            {
                                                sensorId.build = (uint32_t)intValue;
                                                setIdentity(sensorId);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return duciError;
}

// read sensor error status********************************************************************************************
/*
 * @brief Read DUCI sensor error status
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorDuci::readStatus(void)
{
    return sendQuery("#RE?");
}

/*
 * @brief   Handle DUCI sensor error status
 * @param   pointer to sensor instance
 * @param   parsed array of parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetRE(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DSensorDuci *myInstance = (DSensorDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetRE(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/*
 * @brief   Handle DUCI error status of this sensor instance
 * @param   parsed array of parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetRE(sDuciParameter_t * parameterArray)
{
    //DUCI status
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_REPLY)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        uint32_t errorStatus = (uint32_t)parameterArray[1].intNumber & DUCI_SENSOR_ERROR_MASK;
        updateStatus(errorStatus);
    }

    return duciError;
}

/*
 * @brief   Update status of DUCI sensor
 * @note    Only looking for zero error or calibration rejection
 * @param   errorStatus as reported by sensor
 * @return  void
 */
void DSensorDuci::updateStatus(uint32_t errorStatus)
{
    //sensor status
    sSensorStatus_t status;
    status.value = 0u;

    //check for zero error
    if ((errorStatus & DUCI_SENSOR_ERROR_ZERO) == DUCI_SENSOR_ERROR_ZERO)
    {
        status.zeroError = 1u;
    }

    //check for calibration error
    if ((errorStatus & DUCI_SENSOR_ERROR_CALIBRATION) == DUCI_SENSOR_ERROR_CALIBRATION)
    {
        status.calRejected = 1u;
    }

    setStatus(status);
}

// read serial number *************************************************************************************************
/*
 * @brief Read serial number of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorDuci::readSerialNumber(void)
{
    return sendQuery("#SN?");
}

/*
 * @brief   Handle serial number reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetSN(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DSensorDuci *myInstance = (DSensorDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetSN(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/*
 * @brief   Handle serial number of this instance of sensor
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetSN(sDuciParameter_t * parameterArray)
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
        setSerialNumber((uint32_t)parameterArray[1].intNumber);
    }

    return duciError;
}

// read fullscale and type ********************************************************************************************
/*
 * @brief Read upper fullscale and type of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorDuci::readFullscaleAndType(void)
{
    return sendQuery("#RF1?");
}

/*
 * @brief Read lower fullscale and type of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorDuci::readNegativeFullscale(void)
{
    return sendQuery("#RF2?");
}

/*
 * @brief   Handle fullscale and type reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetRF(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DSensorDuci *myInstance = (DSensorDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetRF(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/*
 * @brief   Handle fullscale and type this instance of sensor
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetRF(sDuciParameter_t * parameterArray)
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
        uint32_t index = (uint32_t)parameterArray[0].intNumber; //index

        char *valueStr = parameterArray[2].charArray;
        char *typeStr = valueStr;
        float32_t fullscale = strtof(valueStr, &typeStr);

        //TODO: check for invalid conversions

        if (typeStr != NULL)
        {
           char ch = *typeStr++;

            eSensorType_t sensorType;

            if (ch == 'A')
            {
                sensorType = E_SENSOR_TYPE_PRESS_ABS;
            }
            else if (ch == 'G')
            {
                sensorType = E_SENSOR_TYPE_PRESS_GAUGE;
            }
            else if (ch == 'D')
            {
                sensorType = E_SENSOR_TYPE_PRESS_DIFF;
            }
            else if (ch == 'B')
            {
                sensorType = E_SENSOR_TYPE_PRESS_BARO;
            }
            else if (ch == 'S')
            {
                sensorType = E_SENSOR_TYPE_PRESS_SG;
            }
            else
            {
                sensorType = E_SENSOR_TYPE_GENERIC;
            }

            //must be null terminated
            if (typeStr[0] != '\0')
            {
                duciError.invalid_args = 1u;
            }
            else
            {
                if (index == 1u)
                {
                    setFullScaleMax(fullscale);
                    setAbsFullScaleMax(fullscale); //TODO: 10% margin??

                    setSensorType(sensorType);
                }
                else if (index == 2u)
                {
                    setFullScaleMin(fullscale);
                    setAbsFullScaleMin(fullscale); //TODO: 10% margin??
                }
                else
                {
                    duciError.invalid_args = 1u;
                }
            }
        }
    }

    return duciError;
}

// read measured value ************************************************************************************************
/*
 * @brief Read measured value of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorDuci::measure()
{
    return sendQuery("#RP?");
}

/*
 * @brief   Handle measured value reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetRP(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DSensorDuci *myInstance = (DSensorDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetRP(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/*
 * @brief   Handle measured value reply of this instance of sensor
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetRP(sDuciParameter_t * parameterArray)
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
        //save measurement value
        float32_t value = (float32_t)parameterArray[1].floatValue;
        setMeasurement(value);
    }

    return duciError;
}

// read cal date ******************************************************************************************************
/*
 * @brief Read cal date of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorDuci::readCalDate()
{
    return sendQuery("#CD2?");
}

/*
 * @brief Read manufacture date of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorDuci::readManufactureDate()
{
    return sendQuery("#CD1?");
}

/*
 * @brief   Handle cal date reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetCD(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DSensorDuci *myInstance = (DSensorDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetCD(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/*
 * @brief   Handle cal date reply
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetCD(sDuciParameter_t * parameterArray)
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
        uint32_t index = (uint32_t)parameterArray[0].intNumber; //index
        sDate_t date;

        date.day = (uint32_t)parameterArray[2].intNumber; //day
        date.month = (uint32_t)parameterArray[3].intNumber; //month
        date.year = (uint32_t)parameterArray[4].intNumber; //year

        if (index == 1u) //manufacture date
        {
            setManufactureDate(&date);
        }
        else if (index == 2u)
        {
            setUserCalDate(&date);
        }
        else
        {
            duciError.invalid_args = 1u;
        }
    }

    return duciError;
}

// read cal interval **************************************************************************************************
/*
 * @brief Read cal interval of DUCI sensor
 * @param void
 * @return sensor error code
 */
eSensorError_t DSensorDuci::readCalInterval()
{
    return sendQuery("#CI?");
}

/*
 * @brief   Handle cal interval reply
 * @param   pointer sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetCI(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DSensorDuci *myInstance = (DSensorDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetCI(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/*
 * @brief   Handle cal interval reply for this sensor instance
 * @param   parsed array of received parameters
 * @return  sensor error code
 */
sDuciError_t DSensorDuci::fnSetCI(sDuciParameter_t * parameterArray)
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
        //save cal interval
        setCalInterval((uint32_t)parameterArray[1].intNumber);
    }

    return duciError;
}

//*********************************************************************************************************************
/**
 * @brief   Set sensor cal mode
 * @param   void
 * @return  error code
 */
eSensorError_t DSensorDuci::writeCalMode(void)
{
    return sendCommand("#PP=123");
}

/**
 * @brief   Exit sensor cal mode
 * @param   void
 * @return  error code
 */
eSensorError_t DSensorDuci::writeCalModeExit(void)
{
    return sendCommand("#PP=000");
}

/*
 * @brief Sets the Last Cal date
 * @param eDateTime
 * @return sensor error code
 */
eSensorError_t DSensorDuci::writeCalDate(RTC_DateTypeDef eDateTime)
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
//eSensorError_t DSensorDuci::setCalPoint(float32_t dValue )
//{
//    return E_SENSOR_ERROR_NONE;
//}
//
///*
// * @brief Sets up a new zero value in offset register of sensor after checking that the zero value doesn't exceed +/- 5% of full scale
//  * @return sensor error code
// */
//eSensorError_t DSensorDuci::performZero(void)
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
//eSensorError_t DSensorDuci::calAccept(void)
//{
//    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
//    return sensorError;
//}
//
///*
// * @brief restores the old gain and offset value upon user abort of calibration
// * @return sensor error code
// */
//eSensorError_t DSensorDuci::calAbort(void)
//{
//    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
//    return sensorError;
//}

/*
 * @brief reads the offset value from DUCI sensor
 * @param pZero
 * @return sensor error code
 */
eSensorError_t DSensorDuci::readZero(float32_t *pZero )
{
    eSensorError_t sensorError = E_SENSOR_ERROR_NONE;
    return sensorError;
}

/*
 * @brief sets DUCI sensor's Calibration Interval in days
 * @param Cal interval in days
 * @return sensor error code
 */
eSensorError_t DSensorDuci::writeCalInterval(uint32_t calInterval)
{
    eSensorError_t sensorError = E_SENSOR_ERROR_HAL;
    return sensorError;
}

