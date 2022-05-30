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
* @file     DCommsState.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     25 March 2020
*
* @brief    The communications class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDuci.h"
#include "Utilities.h"
#include "DPV624.h"
#include "main.h"
#include "uart.h"
#include "smartBattery.h"
#include "cBL652.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/


/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/

/**
 * @brief   DCommsState class constructor
 * @param   commsMedium reference to comms medium
 * @retval  void
 */
DCommsStateDuci::DCommsStateDuci(DDeviceSerial *commsMedium, DTask *task)
    : DCommsState(commsMedium, task)
{
    myParser = NULL;
    nextState = E_STATE_DUCI_LOCAL;
    errorStatusRegister.value = 0u;
}

/**
 * @brief   Create DUCI command set: the common commands that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateDuci::createCommands(void)
{
    myParser->addCommand("BS", "=i",            "?",            NULL,    fnGetBS,   E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("BU", "",      "[i]?",            NULL,       fnGetBU,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("BT", "i=i,[i],[i],[i],[i]", "i?", fnSetBT, fnGetBT, E_PIN_MODE_NONE, E_PIN_MODE_NONE); //bluetooth test command
    myParser->addCommand("KM", "=c",    "?",            fnSetKM,    fnGetKM,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);   //UI (key) mode
    myParser->addCommand("RE", "",      "?",            NULL,       fnGetRE,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);   //error status
    myParser->addCommand("RI", "",      "?",            NULL,       fnGetRI,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("IS", "",      "[i]?",         NULL,       fnGetIS,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("RV", "",      "[i],[i]?",     NULL,       fnGetRV,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("DK", "",      "[i][i]?",      NULL,       fnGetDK,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //query DK number
    myParser->addCommand("RB", "",      "[i]?",         NULL,       fnGetRB,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("PV", "",      "?",            NULL,       fnGetPV,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("QV", "",      "[i][i]?",      NULL,       fnGetQV,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //query DK number
    myParser->addCommand("SZ", "",      "?",            NULL,       fnGetSZ,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    //myParser->addCommand("UF", "",      "[i]?",         NULL,       fnGetUF,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
}

/**
 * @brief   Init DUCI
 * @param   void
 * @return  void
 */
void DCommsStateDuci::initialise(void)
{
}

/**
 * @brief   Set the DUCI initial state
 * @param   void
 * @return  returns current DUCI state
 */
eStateDuci_t DCommsStateDuci::run(void)
{
    return E_STATE_DUCI_LOCAL;
}


/**
 * @brief   sends string over communication interface
 * @param   *str string to transmit
 * @return  returns status true or false
 */
bool DCommsStateDuci::sendString(char *str)  //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;

    if(myCommsMedium != NULL)
    {
        successFlag = myParser->prepareTxMessage(str, myTxBuffer, myTxBufferSize);

        if(successFlag == true)
        {
            successFlag = myCommsMedium->sendString(myTxBuffer);
        }
    }

    return successFlag;
}

/**
 * @brief   transmits message and receive response
 * @param   *str message to transmit
 * @param   **pstr message to hold the response
 * @return  returns status true or false
 */
bool DCommsStateDuci::query(char *str, char **pStr)
{
    bool successFlag = false;

    if(myCommsMedium != NULL)
    {
        successFlag = myParser->prepareTxMessage(str, myTxBuffer, myTxBufferSize);

        if(successFlag == true)
        {
            successFlag = myCommsMedium->query(myTxBuffer, pStr, commandTimeoutPeriod);
        }
    }

    return successFlag;
}
/**
 * @brief   to receive the string
 * @param   **pstr message to hold the receive string
 * @return  returns status true or false
 */
bool DCommsStateDuci::receiveString(char **pStr) //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;
    enableSerialPortTxLine(UART_PORT4);

    if(myCommsMedium != NULL)
    {
        successFlag = myCommsMedium->receiveString(pStr, commandTimeoutPeriod);
    }

    return successFlag;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")

/**
 * @brief   DUCI handler for BT Command � Get function
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetBT(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //validate the parameters
        switch(parameterArray[0].intNumber)
        {
        case 0: //count test
            snprintf(myTxBuffer, myTxBufferSize - 1u, "!BT0=%d", BL652_getPingCount());
            sendString(myTxBuffer);
            break;

        case 1:
            // Special Query command for EMC OTA testing - it pre-increments the counter then returns the value
            BL652_incPingCount();
            snprintf(myTxBuffer, myTxBufferSize - 1u, "!BT1=%d", BL652_getPingCount());
            sendString(myTxBuffer);
            break;

        case 20:
            snprintf(myTxBuffer, myTxBufferSize - 1u, "!BT20=%04x", BL652_getReport());
            sendString(myTxBuffer);
            break;

        default: //unexpected value, error
            duciError.invalid_args = 1u;
            break;

        }
    }

    return duciError;
}

/**
 * @brief   DUCI handler for BT Command � Set function
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnSetBT(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //command format is <int><=><int><int>.....
        //validate the parameters
        switch(parameterArray[0].intNumber)
        {
        case 0: //count test
        {
            //jump over the equal sign parameter, the one after that is the count value
            int32_t value = parameterArray[2].intNumber;

            if(value == 0)
            {
                BL652_setPingCount(0);
            }

            else
            {
                BL652_incPingCount(); //increment count
            }
        }
        break;

        case 10: //start TX single frequency test
            if(false == BL652_dtmTXtest((int16_t)parameterArray[2].intNumber, (uint8_t)parameterArray[3].intNumber, (uint8_t)parameterArray[4].intNumber, (uint8_t)parameterArray[5].intNumber, (int8_t)parameterArray[6].intNumber))
            {
                duciError.commandFailed = 1u;
            }

            break;

        case 11: //start TX constant carrier
            if(false == BL652_dtmTXtest((int16_t)parameterArray[2].intNumber, 0u, (uint8_t)eBL652_DTM_TXRX_PKT_VDRSPEC, (uint8_t)eBL652_DTM_VDRSPEC_CC, (int8_t)parameterArray[3].intNumber))
            {
                duciError.commandFailed = 1u;
            }

            break;

        case 20: //start RX single frequency
            if(false == BL652_dtmRXtest((int16_t)parameterArray[2].intNumber, (uint8_t)parameterArray[3].intNumber))
            {
                duciError.commandFailed = 1u;
            }

            break;

        case 30: //end test
        {
            //jump over the equal sign parameter, the one after that is the count value
            int32_t value = parameterArray[2].intNumber;

            if(value == 0)
            {
                uint16_t report;

                if(false == BL652_dtmEndTest(&report))
                {
                    duciError.commandFailed = 1u;
                }
            }

            else
            {
                duciError.invalid_args = 1u;
            }
        }
        break;

        case 50: //set mode
        {
#if 0
            // BT UART Off (for OTA DUCI)
            PV624->commsBluetooth->setTestMode(true);// Test mode / disable / AT mode - stops duci comms on BT interface

            if(false == BL652_initialise((eBL652mode_t)parameterArray[2].intNumber))
            {
                duciError.commandFailed = 1u;
            }

            else
            {
                int32_t value = parameterArray[2].intNumber;
                PV624->setBlStateBasedOnMode((eBL652mode_t)value);

                if((value == (int32_t)eBL652_MODE_RUN)
                        || (value == (int32_t)eBL652_MODE_RUN_DTM))
                {
                    // Only allow UART (for OTA DUCI) comms during BT OTA (Ping test)
                    PV624->commsBluetooth->setTestMode(false);
                }
            }

#else
            PV624->manageBlueToothConnection((eBL652mode_t)parameterArray[2].intNumber);
#endif
        }
        break;

        default: //unexpected value, error
            duciError.invalid_args = 1u;
            break;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for BT Command � Get function
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetBT(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetBT(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for BT Command � Set function
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnSetBT(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetBT(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
* @brief    DUCI call back function for command KM -- change the mode
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
/* Static callback functions ----------------------------------------------------------------------------------------*/
sDuciError_t DCommsStateDuci::fnGetKM(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetKM(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   handler for set KM command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnSetKM(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetKM(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
* @brief    DUCI call back function for command RE --  read command execution error status
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetRE(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetRE(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
* @brief    DUCI call back function for command RI ---  read instrument ID
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetRI(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetRI(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
* @brief    DUCI call back function for command SN ---  read instrument serial number
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetSN(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetSN(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
* @brief    DUCI call back function for command IS --- Read Min,Max,Type,sensor BrandUnit command
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetIS(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetIS(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   handler for get KM command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetKM(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    duciError.unhandledMessage = 1u;
    return duciError;
}

/**
 * @brief   handler for set KM command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnSetKM(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    duciError.unhandledMessage = 1u;
    return duciError;
}

/**
 * @brief   handler for get RE command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRE(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        char buffer[32];
        snprintf(buffer, 32u, "!RE=%08X", errorStatusRegister.value);
        sendString(buffer);

        errorStatusRegister.value = 0u; //clear error status register as it has been read now
    }

    return duciError;
}

/**
 * @brief   handler for get SN command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSN(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        int32_t index = parameterArray[0].intNumber;
        uint32_t sn = (uint32_t)(0);

        if(((int32_t)(0) == index) || ((int32_t)(1) == index))
        {
            sn = PV624->getSerialNumber((uint32_t)(index));
            snprintf(myTxBuffer, 16u, "!SN%d=%d", index, sn);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }


    return duciError;
}


/**
 * @brief   handler for get RI command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRI(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        char dkStr[7];
        char versionStr[13u];
        PV624->getDK((uint32_t)(0), (uint32_t)(0), dkStr);
        PV624->getVersion((uint32_t)(0), (uint32_t)(0), versionStr);
        snprintf(myTxBuffer, 32u, "!RI=DK%s,V%s", dkStr, versionStr);
        sendString(myTxBuffer);
    }

    return duciError;
}


/**
 * @brief   handler for get IS command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetIS(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[44];
    float minPressure = 0.0f;
    float maxPressure = 0.0f;
    eSensorType_t senType;

    if(1 == parameterArray[0].intNumber)
    {
        PV624->getBaroPosFullscale((float *) &maxPressure);
        PV624->getBaroNegFullscale((float *) &minPressure);
        senType = (eSensorType_t)E_SENSOR_TYPE_PRESS_BARO;
        sprintf(buffer, "!IS1=%f,%f,%d", minPressure, maxPressure, (uint32_t)senType);
    }

    else
    {
        PV624->getPosFullscale((float *) &maxPressure);
        PV624->getNegFullscale((float *) &minPressure);
        PV624->getSensorType((eSensorType_t *) &senType);
        sprintf(buffer, "!IS0=%f,%f,%d", minPressure, maxPressure, (uint32_t)senType);

    }

    sendString(buffer);

    errorStatusRegister.value = 0u; //clear error status register as it has been read now

    return duciError;
}

/**
 * @brief   DUCI call back function for ST Command � Get time
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetST(void *instance, sDuciParameter_t *parameterArray)   //* @note =t",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetST(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for ST Command ? Get time
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetST(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        sTime_t rtcTime;

        //get RTC time
        if(PV624->getTime(&rtcTime) == true)
        {
            snprintf(myTxBuffer, 24u, "!ST=%02u:%02u:%02u", rtcTime.hours, rtcTime.minutes, rtcTime.seconds);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SD Command ? Get date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSD(void *instance, sDuciParameter_t *parameterArray)   //* @note =d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetSD(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for SD Command ? Get date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSD(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        sDate_t date;

        //get RTC date
        if(PV624->getDate(&date) == true)
        {
            snprintf(myTxBuffer, 24u, "!SD=%02u/%02u/%04u", date.day, date.month, date.year);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for RD Command ? Get date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRD(void *instance, sDuciParameter_t *parameterArray)   //* @note =d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetRD(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for RD Command ? Get date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRD(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        sDate_t date;

        //get RTC date
        if(PV624->getManufactureDate(&date) == true)
        {
            snprintf(myTxBuffer, 24u, "!RD=%02u/%02u/%04u", date.day, date.month, date.year);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}
/**
 * @brief   DUCI call back function for RV Command ? Read version
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRV(void *instance, sDuciParameter_t *parameterArray)   //* @note ",             "i?",            NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetRV(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for RV Command ? Read version
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRV(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        int32_t item = parameterArray[0].intNumber;
        int32_t component = parameterArray[1].intNumber;
        char versionStr[10u];

        if((item >= 0) && (item <= 1))
        {
            //check the parameters
            switch(component)
            {
            case 0: //application version
            case 1: //bootloader version
            case 2: //board (PCA) version
            {
                if(PV624->getVersion((uint32_t)item, (uint32_t)component, versionStr))
                {
                    snprintf(myTxBuffer, 32u, "!RV%d,%d=V%s", item, component, versionStr);
                }

                else
                {
                    duciError.commandFailed = 1u;
                }
            }
            break;

            default:
                duciError.invalid_args = 1u;
                break;
            }
        }

        else
        {
            duciError.invalid_args = 1u;
        }

        //reply only if index is valid
        if(duciError.value == 0u)
        {
            sendString(myTxBuffer);
        }

    }

    return duciError;
}


/**
* @brief    DUCI call back function for command CM --- read controller mode
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetCM(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetCM(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   handler for get CM command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCM(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        eControllerMode_t controllerMode = E_CONTROLLER_MODE_NONE;

        if(true == PV624->getControllerMode(&controllerMode))
        {
            snprintf(myTxBuffer, 6u, "!CM=%01u", (uint32_t)controllerMode);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}
/**
 * @brief   DUCI call back function for command DK - Get DK number of embedded application
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetDK(void *instance, sDuciParameter_t *parameterArray)   //* @note ",             "[i]?",          NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetDK(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for DK Command ? Read version
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetDK(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        int32_t item = parameterArray[0].intNumber;
        int32_t component = parameterArray[1].intNumber;


        if((item >= 0) && (item <= 1))
        {
            char dkStr[7u];

            //check the parameters
            switch(component)
            {
            case 0: //application version
            case 1: //bootloader version
            {
                if(PV624->getDK((uint32_t)item, (uint32_t)component, dkStr))
                {
                    snprintf(myTxBuffer, 20u, "!DK%d,%d=DK%s", item, component, dkStr);
                }

                else
                {
                    duciError.commandFailed = 1u;
                }
            }
            break;

            default:
                duciError.invalid_args = 1u;
                break;
            }
        }

        else
        {
            duciError.invalid_args = 1u;
        }

        //reply only if index is valid
        if(duciError.value == 0u)
        {
            sendString(myTxBuffer);
        }

    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CI - Get Cal Interval
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCI(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetCI(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for command CI - Get Cal Interval
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCI(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        uint32_t interval = 0u;

        //get cal interval
        if(PV624->getCalInterval(parameterArray[0].uintNumber, &interval) == true)
        {
            snprintf(myTxBuffer, 12u, "!CI%d=%u", parameterArray[0].uintNumber,
                     interval);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }

    }

    return duciError;
}

/**
* @brief    DUCI call back function for command PT --- get Pressure type
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetPT(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetPT(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for command PT - Get pressure type
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetPT(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        eFunction_t curfunc;

        //get cal interval
        if(PV624->getFunction(&curfunc) == true)
        {
            snprintf(myTxBuffer, 12u, "!PT=%u", curfunc);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }

    }

    return duciError;
}

/**
* @brief    DUCI call back function for command SP ---  send control point Value
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetSP(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetSP(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   handler for read SP command --- Send control point command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSP(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        float32_t setPointValue = 0.0f;

        if(true == PV624->getPressureSetPoint((float32_t *)&setPointValue))
        {
            snprintf(myTxBuffer, 20u, "!SP=%7.3f", setPointValue);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
* @brief    DUCI call back function for command SP ---  send control point Value
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetVR(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetVR(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   handler for read SP command --- Send control point command
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetVR(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        float32_t ventRate = 0.0f;

        if(true == PV624->getVentRate((float32_t *)&ventRate))
        {
            snprintf(myTxBuffer, 20u, "!VR=%7.3f", ventRate);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}
/**
 * @brief   DUCI call back function for command CS - Get no of samples remaining at current cal point
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCS(void *instance, sDuciParameter_t *parameterArray)   //* @note ",             "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetCS(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for command CS - Get no of samples remaining at current cal point
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCS(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a command type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        uint32_t samples;

        if(PV624->getCalSamplesRemaining(&samples) == true)
        {
            snprintf(myTxBuffer, 12u, "!CS=%u", samples);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI command for handling BU command
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetBU(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetBU(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command BU - get brand units for the requested sensor
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetBU(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[64];
    char brandUnits[10];
    char brandMin[8];
    char brandMax[8];
    char brandType[8];

    memset(brandUnits, 0, sizeof(brandUnits));
    memset(brandMin, 0, sizeof(brandMin));
    memset(brandMax, 0, sizeof(brandMax));
    memset(brandType, 0, sizeof(brandType));

    if(0 == parameterArray[0].intNumber)
    {
        PV624->getSensorBrandInfo(brandMin, brandMax, brandType, brandUnits);
        sprintf(buffer, "!BU0=%s,%s,%s,%s", brandMin, brandMax, brandType, brandUnits);
    }

    else
    {
        float minPressure = 0.0f;
        float maxPressure = 0.0f;
        eSensorType_t senType;

        PV624->getBaroPosFullscale((float *) &maxPressure);
        PV624->getBaroNegFullscale((float *) &minPressure);
        senType = (eSensorType_t)E_SENSOR_TYPE_PRESS_BARO;
        sprintf(buffer, "!BU1=%f,%f,%d,%s", minPressure, maxPressure, (uint32_t)senType, "mbar");
    }

    sendString(buffer);

    errorStatusRegister.value = 0u; //clear error status register as it has been read now

    return duciError;
}

/**
 * @brief   DUCI command for handling BU command
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetQV(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetQV(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command BU - get brand units for the requested sensor
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetQV(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[44];

    if(0 == parameterArray[0].intNumber)
    {
        if(1 == parameterArray[1].intNumber)
        {
            sprintf(buffer, "!QV%d,%d=02.00.00", parameterArray[0].intNumber, parameterArray[1].intNumber);
        }
    }

    sendString(buffer);

    errorStatusRegister.value = 0u; //clear error status register as it has been read now

    return duciError;
}

/**
 * @brief   DUCI call back function for command CN - Get number of cal points required
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCN(void *instance, sDuciParameter_t *parameterArray)   //* @note ",             "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetCN(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CN - Get number of cal points required
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCN(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        int32_t index = 0;
        uint32_t numCalPoints = 0u;
        eSensor_t sensorType = (eSensor_t)parameterArray[0].intNumber;

        if((eSensor_t)E_BAROMETER_SENSOR == sensorType)
        {
            if(PV624->getRequiredNumCalPoints(sensorType, &numCalPoints) == true)
            {
                //we only have fixed no of cal points so always min = max number
                snprintf(myTxBuffer, 32u, "!CN%d=%u,%u", sensorType, numCalPoints, numCalPoints);
                sendString(myTxBuffer);
            }

            else
            {
                duciError.commandFailed = 1u;
            }
        }

        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
* @brief    DUCI call back function for command PS ---  read measured pressure value, device status and controller status
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetPS(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetPS(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   handler for get PS command(read measured pressure value, device status and controller status)
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetPS(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[64];
    float measVal = 0.0f;

    deviceStatus_t devStat;
    devStat.bytes = 0u;
    devStat = PV624->errorHandler->getDeviceStatus();

    uint32_t controllerStatus = (uint32_t)0;
    PV624->getControllerStatus((uint32_t *) controllerStatus);
    PV624->instrument->getReading((eValueIndex_t)E_VAL_INDEX_VALUE, (float *) &measVal);
    sprintf(buffer, "!PS=%10.5f %08X %08X", measVal,  devStat.bytes, controllerStatus);
    sendString(buffer);

    errorStatusRegister.value = 0u; //clear error status register as it has been read now

    return duciError;
}
/**
 * @brief   DUCI call back function for IZ Command - Get zero value
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetIZ(void *instance, sDuciParameter_t *parameterArray)   //* @note [i],[=],[v]",  "[i]?",          NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetIZ(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for IZ Command - Get zero value
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetIZ(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        float32_t value;

        if(PV624->getZero(&value) == true)
        {
            duciError.value = 0u;
            sprintf(myTxBuffer, "!IZ0=%10.5f", value);
            sendString(myTxBuffer);
        }

        else
        {
            duciError.commandFailed = 1u;
        }

    }

    return duciError;
}


/**
 * @brief   DUCI call back function for RB Command ? Read battery value
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRB(void *instance, sDuciParameter_t *parameterArray)   //* @note ",             "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetRB(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for RB Command ? Read battery value
 * @note    RB[index]?  RB=<value>
 *
 *       where <index> specifies parameter to read (default 0)
 *          0 = battery voltage in Volts
 *          1 = current in mA
 *          2 = battery level (remaining capacity) as a percentage
 *          3 = state of charge in mAh
 *          4 = time to empty in minutes
 *          5 = DC_PRESENT state (ie, is DC supply plugged in)
 *
 *      <value> is the parameter value (voltage or current)
 *
 *      current value is:
 *          positive if battery is discharging
 *          negative is battery is charging
 *
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRB(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //validate index the parameter
        int32_t index = parameterArray[0].intNumber;
        uint32_t uintVal = (uint32_t)(0);
        int32_t intVal = (int32_t)(0);
        float32_t floatVal = (float32_t)(0);

        //check the parameters
        switch(index)
        {
        case 0: // Battery Voltage in volts
            PV624->powerManager->battery->getValue(eVoltage, &floatVal);
            snprintf(myTxBuffer, 16u, "!RB%d=%03f", index, floatVal);
            break;

        case 1: // Battery current in mA
            PV624->powerManager->battery->getValue(eCurrent, &intVal);
            snprintf(myTxBuffer, 16u, "!RB%d=%d", index, intVal);
            break;

        case 2: // Battery level percentage
            PV624->powerManager->battery->getValue(ePercentage, &floatVal);
            snprintf(myTxBuffer, 16u, "!RB%d=%03f", index, floatVal);
            break;

        case 3: // Battery remaining mAh
            PV624->powerManager->battery->getValue(eRemainingCapacity, &uintVal);
            snprintf(myTxBuffer, 16u, "!RB%d=%d", index, uintVal);
            break;

        case 4: // Battery remaining minutes
            PV624->powerManager->battery->getValue(eRunTimeToEmpty, &uintVal);
            snprintf(myTxBuffer, 16u, "!RB%d=%d", index, uintVal);
            break;

        case 5: // DC state
            PV624->powerManager->ltc4100->getIsAcPresent(&uintVal);
            snprintf(myTxBuffer, 16u, "!RB%d=%d", index, uintVal);
            break;

        case 6:
            //snprintf(myTxBuffer, 16u, "!RB%d=%d", index, value);
            PV624->powerManager->getBatTemperature(&floatVal);
            PV624->powerManager->battery->getValue(eCurrent, &intVal);
            sprintf(myTxBuffer, "!RB%d=%d %f", index, intVal, floatVal);
            break;

        default:
            duciError.invalid_args = 1u;
            break;
        }

        //reply only if index is valid
        if(duciError.value == 0u)
        {
            sendString(myTxBuffer);
        }


    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SC Command ? Get Instrument Port Configuration
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSC(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetSC(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for SC Command ? Get Instrument Port Configuration
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSC(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        snprintf(myTxBuffer, 32u, "!SC0=%d", PV624->getUsbInstrumentPortConfiguration());
        sendString(myTxBuffer);
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CD - Get Calibration Date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCD(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetCD(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CD - Get Calibration Date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCD(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //command format is <int><=><date>
        //validate the parameters
        int32_t index = parameterArray[0].intNumber;
        sDate_t date;

        switch(index)
        {

        case 0u:
            if((PV624->instrument->getSensorCalDate(&date)) == true)
            {
                snprintf(myTxBuffer, 24u, "!CD%d=%02u/%02u/%04u", index, date.day, date.month, date.year);
                sendString(myTxBuffer);
            }

            else
            {
                duciError.commandFailed = 1u;
            }

            break;

        case 1u:

            //get cal date
            if(PV624->getCalDate(&date) == true)
            {
                snprintf(myTxBuffer, 24u, "!CD%d=%02u/%02u/%04u", index, date.day, date.month, date.year);
                sendString(myTxBuffer);
            }

            else
            {
                duciError.commandFailed = 1u;
            }

            break;


        default:
            duciError.commandFailed = 1u;
            break;
        }
    }

    return duciError;
}

/**
* @brief    This function is to read pressure, device status, controller status
* @param        instance is a pointer to the FSM state instance
* @param        parameterArray is the array of received command parameters
* @retval   sDuciError_t command execution error status
*/
sDuciError_t DCommsStateDuci::fnGetPV(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetPV(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   handler for get PV command --- read pressure, device status, controller status
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetPV(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[64];
    float measVal = 0.0f;
    float32_t baroVal = 0.0f;
    deviceStatus_t devStat;
    devStat.bytes = 0u;

    uint32_t controllerStatus = (uint32_t)0;
    PV624->instrument->getReading((eValueIndex_t)E_VAL_INDEX_VALUE, (float *) &measVal);
    PV624->instrument->getReading((eValueIndex_t)E_VAL_INDEX_BAROMETER_VALUE, (float *) &baroVal);

    devStat = PV624->errorHandler->getDeviceStatus();
    PV624->getControllerStatus((uint32_t *)&controllerStatus);

    sprintf(buffer, "!PV=%10.5f,%08X,%08X,%10.5f", measVal, devStat.bytes, controllerStatus, baroVal);
    sendString(buffer);

    errorStatusRegister.value = 0u; //clear error status register as it has been read now

    return duciError;
}


/**
 * @brief   DUCI call back function for RF Command ? Read Full Scale value
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetUF(void *instance, sDuciParameter_t *parameterArray)   //* @note =d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetUF(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for RF Command ? Read Full Scale value
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetUF(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[32];

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        int32_t index = parameterArray[0].intNumber;
        uint32_t upgradePc = 0u;

        if(index == 1)
        {
            PV624->getPmUpgradePercentage(&upgradePc);
        }

        sprintf(buffer, "!UF%d=%d", index, upgradePc);
        sendString(buffer);
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CD - Get Calibration Date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetND(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetND(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CD - Get Calibration Date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetND(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //command format is <int><=><date>
        //validate the parameters
        int32_t index = parameterArray[0].intNumber;
        sDate_t date;

        switch(index)
        {

        case 0u:
            duciError.invalid_args = 1u;
            break;

        case 1u:

            //get cal date
            if(PV624->getNextCalDate(&date) == true)
            {
                snprintf(myTxBuffer, 24u, "!ND%d=%02u/%02u/%04u", index, date.day, date.month, date.year);
                sendString(myTxBuffer);
            }

            else
            {
                duciError.commandFailed = 1u;
            }

            break;


        default:
            duciError.commandFailed = 1u;
            break;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SZCommand ? Read set point count
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSZ(void *instance, sDuciParameter_t *parameterArray)   //* @note =d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetSZ(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/**
 * @brief   DUCI handler for SZ Command ? Read Fset point count
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSZ(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[32];

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {

        uint32_t setPointCnt = 0u;


        setPointCnt = PV624->getSetPointCount();


        sprintf(buffer, "!SZ=%d", setPointCnt);
        sendString(buffer);
    }

    return duciError;
}

/**
 * @brief   DUCI call back function to set bluetooth state which we received from BL652
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnSetBS(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnSetBS(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command BS - Set bluetooth status
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnSetBS(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        //command format is <int><=><date>
        //validate the parameters
        int32_t index = parameterArray[0].intNumber;


        switch(index)
        {

        case 0u:
            PV624->setBlState(BL_STATE_RUN_ADV_IN_PROGRESS);
            PV624->userInterface->bluetoothLedControl(eBlueToothPairing,
                    E_LED_OPERATION_TOGGLE,
                    0u,
                    E_LED_STATE_SWITCH_ON,
                    UI_DEFAULT_BLINKING_RATE);
            break;

        case 1u:
            PV624->setBlState(BL_STATE_RUN_CONNECTION_ESTABLISHED);
            PV624->userInterface->bluetoothLedControl(eBlueToothPairing,
                    E_LED_OPERATION_SWITCH_ON,
                    0u,
                    E_LED_STATE_SWITCH_ON,
                    UI_DEFAULT_BLINKING_RATE);
            break;

        case 2u:
            PV624->setBlState(BL_STATE_RUN_DEEP_SLEEP);
            sprintf(myTxBuffer, "ds");
            sendString(myTxBuffer);
            PV624->userInterface->bluetoothLedControl(eBlueToothPairing,
                    E_LED_OPERATION_SWITCH_OFF,
                    0u,
                    E_LED_STATE_SWITCH_OFF,
                    UI_DEFAULT_BLINKING_RATE);
            break;

        default:
            duciError.commandFailed = 1u;
            break;
        }
    }

    return duciError;
}


/**
 * @brief   DUCI call back function to current get bluetooth state
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetBS(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetBS(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command BS - Set bluetooth status
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetBS(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        sprintf(myTxBuffer, "!BS%01d", PV624->getBlState());
        sendString(myTxBuffer);
    }

    return duciError;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")


