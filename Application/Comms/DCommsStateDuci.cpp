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

/* Error handler instance parameter starts from 1301 to 1400 */
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
 * @brief   DCommsState class destructor
 * @param   void
 * @retval
 */
DCommsStateDuci::~DCommsStateDuci(void)
{
}
/**
 * @brief   Create DUCI command set: the common commands that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateDuci::createCommands(void)
{
    // B
    myParser->addCommand("BU", "",      "[i]?",         NULL,       fnGetBU,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    // C
    myParser->addCommand("CN", "=i",    "[i]?",        NULL,       fnGetCN,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    // D
    myParser->addCommand("DK", "",      "[i][i]?",      NULL,       fnGetDK,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //query DK number
    // E
    // F
    // G
    // H
    // I
    myParser->addCommand("IS", "",      "[i]?",         NULL,       fnGetIS,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    // J
    // K
    myParser->addCommand("KM", "=c",    "?",            fnSetKM,    fnGetKM,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);   //UI (key) mode
    // L
    // M
    // N
    // O
    // P
    myParser->addCommand("PV", "",      "?",            NULL,       fnGetPV,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);

    // Q
    myParser->addCommand("QV", "",      "[i][i]?",      NULL,       fnGetQV,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE); //query DK number
    // R
    myParser->addCommand("RB", "",      "[i]?",         NULL,       fnGetRB,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("RE", "",      "?",            NULL,       fnGetRE,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);   //error status
    myParser->addCommand("RI", "",      "?",            NULL,       fnGetRI,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    myParser->addCommand("RV", "",      "[i],[i]?",     NULL,       fnGetRV,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    // S
    myParser->addCommand("SZ", "",      "?",            NULL,       fnGetSZ,    E_PIN_MODE_NONE,          E_PIN_MODE_NONE);
    // T
    // U
    // V
    // W
    // X
    // Y
    // Z
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
            successFlag = myCommsMedium->sendString(myTxBuffer, myTxBufferSize);
        }
    }

    return successFlag;
}
#if 0
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
#endif
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
 * @brief   DUCI handler for BT Command – Get function
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
            snprintf_s(myTxBuffer, myTxBufferSize - 1u, "!BT0=%d", BL652_getPingCount());
            sendString(myTxBuffer);
            break;

        case 1:
            // Special Query command for EMC OTA testing - it pre-increments the counter then returns the value
            BL652_incPingCount();
            snprintf_s(myTxBuffer, myTxBufferSize - 1u, "!BT1=%d", BL652_getPingCount());
            sendString(myTxBuffer);
            break;

        case 20:
            snprintf_s(myTxBuffer, myTxBufferSize - 1u, "!BT20=%04x", BL652_getReport());
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
 * @brief   DUCI handler for BT Command – Set function
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
 * @brief   DUCI call back function for BT Command – Get function
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
 * @brief   DUCI call back function for BT Command – Set function
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

        snprintf_s(myTxBuffer, 32u, "!RE=%08X", errorStatusRegister.value);
        sendString(myTxBuffer);

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
        uint32_t sn = 0u;

        if((0 == index) || (1 == index))
        {
            sn = PV624->getSerialNumber((uint32_t)(index));
            snprintf_s(myTxBuffer, 16u, "!SN%d=%d", index, sn);
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
        PV624->getDK(0u, 0u, dkStr);
        PV624->getVersion(0u, 0u, versionStr);
        snprintf_s(myTxBuffer, 32u, "!RI=DK%s,V%s", dkStr, versionStr);
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
    float minPressure = 0.0f;
    float maxPressure = 0.0f;
    eSensorType_t senType;

    if(1 == parameterArray[0].intNumber)
    {
        PV624->getBaroPosFullscale((float *) &maxPressure);
        PV624->getBaroNegFullscale((float *) &minPressure);
        senType = (eSensorType_t)E_SENSOR_TYPE_PRESS_BARO;

        if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!IS1=%f,%f,%d", minPressure, maxPressure, (uint32_t)senType))
        {
            sendString(myTxBuffer);
        }
    }

    else if(0 == parameterArray[0].intNumber)
    {
        PV624->getPosFullscale((float *) &maxPressure);
        PV624->getNegFullscale((float *) &minPressure);
        PV624->getSensorType((eSensorType_t *) &senType);

        if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!IS0=%4.2f,%5.2f,%d", minPressure, maxPressure, (uint32_t)senType))
        {
            sendString(myTxBuffer);
        }

    }

    else
    {
        duciError.invalid_args = 1u;
    }



    errorStatusRegister.value = 0u; //clear error status register as it has been read now

    return duciError;
}

/**
 * @brief   DUCI call back function for ST Command – Get time
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
            snprintf_s(myTxBuffer, 24u, "!ST=%02u:%02u:%02u", rtcTime.hours, rtcTime.minutes, rtcTime.seconds);
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
            snprintf_s(myTxBuffer, 24u, "!SD=%02u/%02u/%04u", date.day, date.month, date.year);
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
            snprintf_s(myTxBuffer, 24u, "!RD=%02u/%02u/%04u", date.day, date.month, date.year);
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
    int retValue = 0;

    //only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        int32_t component = parameterArray[0].intNumber;
        int32_t item = parameterArray[1].intNumber;
        char versionStr[10u];


        if((item >= 0) && (item <= 2))
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
                    retValue = snprintf_s(myTxBuffer, 32u, "!RV%d,%d=V%s", component, item, versionStr);
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
        if((duciError.value == 0u) && (retValue > 0))
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
            snprintf_s(myTxBuffer, 6u, "!CM=%01u", (uint32_t)controllerMode);
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
            case 6:
            case 7:
            {
                if(PV624->getDK((uint32_t)item, (uint32_t)component, dkStr))
                {
                    snprintf_s(myTxBuffer, 20u, "!DK%d,%d=DK%s", item, component, dkStr);
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

        if(0u == parameterArray[0].uintNumber)
        {
            //get cal interval
            if(PV624->getCalInterval(parameterArray[1].uintNumber, &interval) == true)
            {
                snprintf_s(myTxBuffer,
                           TX_BUFFER_SIZE,
                           "!CI%d%d=%u",
                           parameterArray[0].uintNumber, parameterArray[1].uintNumber,
                           interval);
                sendString(myTxBuffer);
            }

            else
            {
                duciError.commandFailed = 1u;
            }
        }

        else
        {
            duciError.invalid_args = 0u;
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
            snprintf_s(myTxBuffer, 12u, "!PT=%u", curfunc);
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
            snprintf_s(myTxBuffer, 20u, "!SP=%7.3f", setPointValue);
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
            snprintf_s(myTxBuffer, 20u, "!VR=%7.3f", ventRate);
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
            snprintf_s(myTxBuffer, 12u, "!CS=%u", samples);
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

    char brandUnits[10];
    char brandMin[8];
    char brandMax[8];
    char brandType[8];

    memset_s(brandUnits, sizeof(brandUnits), 0, sizeof(brandUnits));
    memset_s(brandMin, sizeof(brandMin), 0, sizeof(brandMin));
    memset_s(brandMax, sizeof(brandMax), 0, sizeof(brandMax));
    memset_s(brandType, sizeof(brandType), 0, sizeof(brandType));

    if(0 == parameterArray[0].intNumber)
    {

        PV624->getSensorBrandMin(brandMin, sizeof(brandMin));
        PV624->getSensorBrandMax(brandMax, sizeof(brandMax));
        PV624->getSensorBrandType(brandType, sizeof(brandType));
        PV624->getSensorBrandUnits(brandUnits, sizeof(brandUnits));
        errorStatusRegister.value = 0u; //clear error status register as it has been read now

        if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!BU0=%s,%s,%s,%s", brandMin, brandMax, brandType, brandUnits))
        {
            sendString(myTxBuffer);
        }
    }

    else
    {
        errorStatusRegister.invalid_args = 1u;
        duciError.invalid_args = 1u;
    }





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

    if(0 == parameterArray[0].intNumber)
    {
        if(1 == parameterArray[1].intNumber)
        {
            if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!QV%d,%d=02.00.00", parameterArray[0].intNumber, parameterArray[1].intNumber))
            {
                sendString(myTxBuffer);
            }

            errorStatusRegister.value = 0u; //clear error status register as it has been read now
        }

        else
        {
            duciError.invalid_args = 1u;
        }
    }

    else
    {
        duciError.invalid_args = 1u;
    }



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
        //int32_t index = 0;
        uint32_t numCalPoints = 0u;
        eSensor_t sensorType = (eSensor_t)parameterArray[0].intNumber;

        if((eSensor_t)E_BAROMETER_SENSOR == sensorType)
        {
            if(PV624->getRequiredNumCalPoints(sensorType, &numCalPoints) == true)
            {
                //we only have fixed no of cal points so always min = max number
                snprintf_s(myTxBuffer, 32u, "!CN%d=%u,%u", sensorType, numCalPoints, numCalPoints);
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
    float measVal = 0.0f;

    deviceStatus_t devStat;
    devStat.bytes = 0u;
    devStat = PV624->errorHandler->getDeviceStatus();

    uint32_t controllerStatus = (uint32_t)0;
    PV624->getControllerStatus((uint32_t *) controllerStatus);
    PV624->instrument->getReading((eValueIndex_t)E_VAL_INDEX_VALUE, (float *) &measVal);

    if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!PS=%10.5f %08X %08X", measVal,  devStat.bytes, controllerStatus))
    {
        sendString(myTxBuffer);
    }

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
        if(0 == parameterArray[0].intNumber)
        {
            float32_t value;

            if(PV624->getZero(&value) == true)
            {
                duciError.value = 0u;

                if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!IZ%d=%10.5f", parameterArray[0].intNumber, value))
                {
                    sendString(myTxBuffer);
                }
            }

            else
            {
                duciError.commandFailed = 1u;
            }
        }

        else
        {
            duciError.invalid_args = 0u;
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
        uint32_t uintVal = 0u;
        int32_t intVal = 0;
        float32_t floatVal = 0.0f;

        //check the parameters
        switch(index)
        {
        case 0: // Battery Voltage in volts
            PV624->powerManager->battery->getValue(eVoltage, &floatVal);
            snprintf_s(myTxBuffer, 16u, "!RB%d=%03f", index, floatVal);
            break;

        case 1: // Battery current in mA
            PV624->powerManager->battery->getValue(eCurrent, &intVal);
            snprintf_s(myTxBuffer, 16u, "!RB%d=%d", index, intVal);
            break;

        case 2: // Battery level percentage
            PV624->powerManager->battery->getValue(ePercentage, &floatVal);
            snprintf_s(myTxBuffer, 16u, "!RB%d=%03f", index, floatVal);
            break;

        case 3: // Battery remaining mAh
            PV624->powerManager->battery->getValue(eRemainingCapacity, &uintVal);
            snprintf_s(myTxBuffer, 16u, "!RB%d=%d", index, uintVal);
            break;

        case 4: // Battery remaining minutes
            PV624->powerManager->battery->getValue(eRunTimeToEmpty, &uintVal);
            snprintf_s(myTxBuffer, 16u, "!RB%d=%d", index, uintVal);
            break;

        case 5: // DC state
            PV624->powerManager->ltc4100->getIsAcPresent(&uintVal);
            snprintf_s(myTxBuffer, 16u, "!RB%d=%d", index, uintVal);
            break;

        case 6:
            //snprintf(myTxBuffer, 16u, "!RB%d=%d", index, value);
            PV624->powerManager->getBatTemperature(&floatVal);
            PV624->powerManager->battery->getValue(eCurrent, &intVal);
            snprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!RB%d=%d %10.5f", index, intVal, floatVal);
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
        snprintf_s(myTxBuffer, 32u, "!SC0=%d", PV624->getUsbInstrumentPortConfiguration());
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
                snprintf_s(myTxBuffer, 24u, "!CD%d=%02u/%02u/%04u", index, date.day, date.month, date.year);
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
                snprintf_s(myTxBuffer, 24u, "!CD%d=%02u/%02u/%04u", index, date.day, date.month, date.year);
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
    float measVal = 0.0f;
    float32_t baroVal = 0.0f;
    deviceStatus_t devStat;
    devStat.bytes = 0u;

    uint32_t controllerStatus = 0u;
    PV624->instrument->getReading((eValueIndex_t)E_VAL_INDEX_VALUE, (float *) &measVal);
    PV624->instrument->getReading((eValueIndex_t)E_VAL_INDEX_BAROMETER_VALUE, (float *) &baroVal);

    devStat = PV624->errorHandler->getDeviceStatus();
    PV624->getControllerStatus((uint32_t *)&controllerStatus);

    if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!PV=%10.5f,%08X,%08X,%10.5f", measVal, devStat.bytes, controllerStatus, baroVal))
    {
        sendString(myTxBuffer);
    }

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

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {
        int32_t index = parameterArray[0].intNumber;
        uint32_t upgradePc = 0u;
        uint32_t upgradeStatus = 0u;

        if(index == UPGRADE_PM620_FIRMWARE)
        {
            PV624->getPmUpgradePercentage(&upgradePc, &upgradeStatus);

            if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!UF%d=%d,%d", index, upgradePc, upgradeStatus))
            {
                sendString(myTxBuffer);
            }
        }

        else
        {
            duciError.invalid_args = 1u;
        }


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
                snprintf_s(myTxBuffer, 24u, "!ND%d=%02u/%02u/%04u", index, date.day, date.month, date.year);
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

//only accepted message in this state is a reply type
    if(myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }

    else
    {

        uint32_t setPointCnt = 0u;
        float disTravelled = 0.0f;

        setPointCnt = PV624->getSetPointCount();
        PV624->getDistanceTravelledByController(&disTravelled);   // Send in meters
        disTravelled = disTravelled / 1000.0f;

        if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!SZ=%d,%5.3f", setPointCnt, disTravelled))
        {
            sendString(myTxBuffer);
        }
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

            if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "ds"))
            {
                sendString(myTxBuffer);
            }

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
        if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!BS%01d", PV624->getBlState()))
        {
            sendString(myTxBuffer);
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for command CA - Get Calibration Data
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCA(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetCA(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for command CA - Get Calibration Data
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCA(sDuciParameter_t *parameterArray)
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
        float32_t offsetsData[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        PV624->getCalOffsets(&offsetsData[0]);

        if(sprintf_s(myTxBuffer, TX_BUFFER_SIZE, "!CA=%.2f,%.2f,%.2f,%.2f", offsetsData[0],
                     offsetsData[1],
                     offsetsData[2],
                     offsetsData[3]))
        {
            sendString(myTxBuffer);
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for PP Command – Get current PIN protection mode
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetPP(void *instance, sDuciParameter_t *parameterArray)   //* @note   =3i",          "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci *)instance;

    if(myInstance != NULL)
    {
        duciError = myInstance->fnGetPP(parameterArray);
    }

    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for PP Command – Get current PIN protection mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetPP(sDuciParameter_t *parameterArray)
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
        uint32_t pinValue = 0u;

        switch(PV624->getPinMode())
        {
        case E_PIN_MODE_NONE:
            pinValue = E_REMOTE_PIN_NONE;
            break;

        case E_PIN_MODE_CALIBRATION:
            pinValue = E_REMOTE_PIN_CALIBRATION;
            break;

        case E_PIN_MODE_CONFIGURATION:
            pinValue = E_REMOTE_PIN_CONFIGURATION;
            break;

        case E_PIN_MODE_FACTORY:
            pinValue = E_REMOTE_PIN_FACTORY;
            break;

        case E_PIN_MODE_ENGINEERING:
            pinValue = E_REMOTE_PIN_ENGINEERING;
            break;

        case E_PIN_MODE_UPGRADE:
            pinValue = E_REMOTE_PIN_UPGRADE;
            break;

        default:
            duciError.invalidMode = 1u;
            break;
        }

        //reply only if all is well
        if(duciError.value == 0u)
        {
            snprintf_s(myTxBuffer, 16u, "!PP=%03u", pinValue);
            sendString(myTxBuffer);
        }
    }

    return duciError;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma("diag_default=Pm017,Pm128")


