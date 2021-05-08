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
DCommsStateDuci::DCommsStateDuci(DDeviceSerial *commsMedium, DTask* task)
:DCommsState(commsMedium, task)
{
  
}

/**
 * @brief   Create DUCI command set - the common commands - that apply to all states
 * @param   void
 * @return  void
 */
void DCommsStateDuci::createCommands(void)
{
    myParser->addCommand("KM", "=c",    "?",           fnSetKM,    fnGetKM,    0xFFFFu);   //UI (key) mode
    myParser->addCommand("RE", "",      "?",            NULL,       fnGetRE,    0xFFFFu);   //error status    
    myParser->addCommand("RI", "",      "?",            NULL,       fnGetRI,    0xFFFFu);
    myParser->addCommand("IV", "",      "[i],[i]?",     NULL,       fnGetIV,    0xFFFFu);
    myParser->addCommand("IS", "",      "[i]?",         NULL,       fnGetIS,    0xFFFFu);
    myParser->addCommand("RV", "",      "[i],[i]?",     NULL,       fnGetRV,    0xFFFFu);
    myParser->addCommand("DK", "",      "[i][i]?",      NULL,       fnGetDK,    0xFFFFu); //query DK number
    myParser->addCommand("PS", "",      "?",            NULL,       fnGetRE,    0xFFFFu);   //error status   
    myParser->addCommand("CC", "",      "?",            NULL,       fnGetCC,    0xFFFFu);   //error status  
}

void DCommsStateDuci::initialise(void)
{
}





eStateDuci_t DCommsStateDuci::run(void)
{
  return E_STATE_DUCI_LOCAL;
}


///**********************************************************************************************************************
// * DISABLE MISRA C 2004 CHECK for Rule 10.3. Ignoring this - explicit conversion from 'signed int' to 'char' is safe
// **********************************************************************************************************************/
//_Pragma ("diag_suppress=Pm136")
//
////prepare message in txBuffer
//bool DCommsState::prepareMessage(char *str)
//{
//    bool successFlag = false;
//
//    uint32_t size = strlen(str);
//
//    if (size < (TX_BUFFER_SIZE - 6u))
//    {
//        int32_t checksum = 0;
//
//        //checksum only necessary if enabled
//        if (myParser->getChecksumEnabled() == true)
//        {
//            for (uint32_t i = 0u; i < size; i++)
//            {
//                txBuffer[i] = str[i];
//                checksum += (int32_t)str[i];
//            }
//
//            //checksum include the semi-colon
//            txBuffer[size++] = ':';
//            checksum += (int32_t)':';
//
//            checksum %= 100;
//
//            txBuffer[size++] = '0' + (char)(checksum / 10);
//            txBuffer[size++] = '0' + (char)(checksum % 10);
//        }
//        else
//        {
//            strncpy(txBuffer, str, size);
//        }
//
//        if (myParser->getTerminatorCrLf() == true)
//        {
//            txBuffer[size++] = '\r';    //CR sent only if enabled
//        }
//
//        txBuffer[size++] = '\n';         //always send LF
//        txBuffer[size] = '\0';           //always null terminate
//
//        successFlag = true;
//    }
//
//    return successFlag;
//}
//
///**********************************************************************************************************************
// * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as we are using OS_ERR enum which violates the rule
// **********************************************************************************************************************/
//_Pragma ("diag_default=Pm136")

bool DCommsStateDuci::sendString(char *str)  //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;

    if (myCommsMedium != NULL)
    {
        successFlag = myParser->prepareTxMessage(str, myTxBuffer, myTxBufferSize);

        if (successFlag == true)
        {
            successFlag = myCommsMedium->sendString(myTxBuffer);
        }
    }

    return successFlag;
}

bool DCommsStateDuci::query(char *str, char **pStr)
{
    bool successFlag = false;

    if (myCommsMedium != NULL)
    {
        successFlag = myParser->prepareTxMessage(str, myTxBuffer, myTxBufferSize);

        if (successFlag == true)
        {
            successFlag = myCommsMedium->query(myTxBuffer, pStr, commandTimeoutPeriod);
        }
    }

    return successFlag;
}

bool DCommsStateDuci::receiveString(char **pStr) //TODO: Extend this to have more meaningful returned status
{
    bool successFlag = false;

    if (myCommsMedium != NULL)
    {
        successFlag = myCommsMedium->receiveString(pStr, commandTimeoutPeriod);
    }

    return successFlag;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")


/* Static callback functions ----------------------------------------------------------------------------------------*/
sDuciError_t DCommsStateDuci::fnGetKM(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetKM(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnSetKM(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetKM(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetRE(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetRE(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

//call back functions - each calls an instance method
sDuciError_t DCommsStateDuci::fnGetRI(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetRI(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetSN(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetSN(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetIV(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetIV(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetIS(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetIS(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}
/* instance versions of callback functions --------------------------------------------------------------------------*/
sDuciError_t DCommsStateDuci::fnGetKM(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    duciError.unhandledMessage = 1u;
    return duciError;
}

sDuciError_t DCommsStateDuci::fnSetKM(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    duciError.unhandledMessage = 1u;
    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetRE(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        char buffer[32];
        error_code_t errorCode;
        errorCode.bytes = 0u;
        errorCode = PV624->errorHandler->getErrors();
        sprintf(buffer, "!RE=%08X", errorCode.bytes);
        sendString(buffer);

        errorStatusRegister.value = 0u; //clear error status register as it has been read now
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetSN(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        snprintf(myTxBuffer, 16u, "!SN=%d", PV624->getSerialNumber());
        sendString(myTxBuffer);
    }

   
    return duciError;
}



sDuciError_t DCommsStateDuci::fnGetRI(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        char name[13];
        PV624->getInstrumentName(name);
        snprintf(myTxBuffer, 18u, "!RI=%s", name);
        sendString(myTxBuffer);
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetIV(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[32];
    float measVal = 0.0f;
    PV624->instrument->getReading( (eValueIndex_t)E_VAL_INDEX_VALUE,(float*) &measVal);
    sprintf(buffer, "!IV0=%10.5f",measVal);
    sendString(buffer);

    errorStatusRegister.value = 0u; //clear error status register as it has been read now
   
    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetIS(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[32];
    float minPressure = 0.0f;
    float maxPressure = 0.0f;
    eSensorType_t senType;
    PV624->getPosFullscale( (float*) &maxPressure);
    PV624->getNegFullscale((float*) &minPressure);
    PV624->getSensorType((eSensorType_t*) &senType);
    sprintf(buffer, "!IS=%f,%f,%d ",minPressure,maxPressure,(uint32_t)senType);
    sendString(buffer);

    errorStatusRegister.value = 0u; //clear error status register as it has been read now
   
    return duciError;
}

/**
 * @brief   DUCI call back function for ST Command – Get time
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetST(void *instance, sDuciParameter_t *parameterArray)   //* @note	=t",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
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
 * @brief   DUCI handler for ST Command – Get time
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetST(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        sTime_t rtcTime;

        //get RTC time
        if (PV624->getTime(&rtcTime) == true)
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
 * @brief   DUCI call back function for SD Command – Get date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSD(void *instance, sDuciParameter_t *parameterArray)   //* @note	=d",           "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
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
 * @brief   DUCI handler for SD Command – Get date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSD(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        sDate_t date;

        //get RTC date
        if (PV624->getDate(&date) == true)
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
 * @brief   DUCI call back function for RV Command – Read version
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRV(void *instance, sDuciParameter_t *parameterArray)   //* @note	",             "i?",            NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
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
 * @brief   DUCI handler for RV Command – Read version
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetRV(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
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
          switch (component)
          {
              case 0: //application version
              case 1: //bootloader version
              case 2: //board (PCA) version
              {
                  if (PV624->getVersion((uint32_t)item,(uint32_t)component, versionStr))
                  {
                      snprintf(myTxBuffer, 32u, "!RV%d%d=V%s", item, component, versionStr);
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
        if (duciError.value == 0u)
        {
            sendString(myTxBuffer);
        }
        
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetCM(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetCM(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetCM(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        eControllerMode_t controllerMode = E_CONTROLLER_MODE_NONE;
        if (true == PV624->getControllerMode(&controllerMode))
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
sDuciError_t DCommsStateDuci::fnGetDK(void *instance, sDuciParameter_t *parameterArray)   //* @note	",             "[i]?",          NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
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
 * @brief   DUCI handler for DK Command – Read version
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetDK(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
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
          switch (component)
          {
              case 0: //application version
              case 1: //bootloader version             
              {
                  if (PV624->getDK((uint32_t)item,(uint32_t)component, dkStr))
                  {
                      snprintf(myTxBuffer, 20u, "!DK%d%d=%s",item, component, dkStr);
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
        if (duciError.value == 0u)
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

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
            uint32_t interval;

            //get cal interval
            if (PV624->getCalInterval( &interval) == true)
            {
                snprintf(myTxBuffer, 12u, "!CI=%u", interval);
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
 * @brief   DUCI call back function for command SF - Get current function
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSF(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetSF(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}


/**
 * @brief   DUCI handler for command SF - Get Cal Interval
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetSF(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
           eFunction_t curfunc;
           uint32_t func = 0u;
            uint32_t pseudoType = 0u;
            //get cal interval
            if (PV624->getFunction( &curfunc) == true)
            {
                if((eFunction_t)E_FUNCTION_BAROMETER == curfunc)
                {
                  func = 7u;
                  pseudoType = 0u;
                }
                else if((eFunction_t)E_FUNCTION_PSEUDO_ABS == curfunc)
                {
                  func = 13u;
                  pseudoType = 0u;
                }
                else if((eFunction_t)E_FUNCTION_PSEUDO_GAUGE == curfunc)
                {
                  func = 13u;
                  pseudoType=1u;
                }
                else
                {
                  func = 0u;
                  pseudoType = 0u;
                }
                snprintf(myTxBuffer, 12u, "!SF=%u,%u", func,pseudoType);
                sendString(myTxBuffer);
            }
            else
            {
                duciError.commandFailed = 1u;
            }
        
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetSP(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetSP(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetSP(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

//only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        float32_t setPointValue = 0.0f;
        if (true == PV624->getPressureSetPoint((float32_t*)&setPointValue))
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
 * @brief   DUCI call back function for command CS - Get no of samples remaining at current cal point
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCS(void *instance, sDuciParameter_t *parameterArray)   //* @note	",             "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        uint32_t samples;

        if (PV624->getCalSamplesRemaining(&samples) == true)
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
 * @brief   DUCI call back function for command CN - Get number of cal points required
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateDuci::fnGetCN(void *instance, sDuciParameter_t *parameterArray)   //* @note	",             "?",             NULL,       NULL,      0xFFFFu);
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
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
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        uint32_t numCalPoints = 0u;

        if (PV624->getRequiredNumCalPoints(&numCalPoints) == true)
        {
            //we only have fixed no of cal points so always min = max number
            snprintf(myTxBuffer, 32u, "!CN=%u,%u", numCalPoints, numCalPoints);
            sendString(myTxBuffer);
        }
        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetPS(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetPS(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetPS(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[64];
    float measVal = 0.0f;
    
    error_code_t errorCode;
    errorCode.bytes = 0u;
    errorCode = PV624->errorHandler->getErrors();
    
    uint32_t controllerStatus = (uint32_t)0;
    PV624->getControllerStatus((uint32_t*) controllerStatus); 
    PV624->instrument->getReading( (eValueIndex_t)E_VAL_INDEX_VALUE,(float*) &measVal);
    sprintf(buffer, "!PS=%10.5f %08X %08X",measVal,  errorCode.bytes, controllerStatus);
    sendString(buffer);

    errorStatusRegister.value = 0u; //clear error status register as it has been read now
   
    return duciError;
}


sDuciError_t DCommsStateDuci::fnGetCC(void *instance, sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateDuci *myInstance = (DCommsStateDuci*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetCC(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

sDuciError_t DCommsStateDuci::fnGetCC(sDuciParameter_t * parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;
    char buffer[32];
    float measVal = 0.0f;
    
   
    
    uint32_t controllerStatus = (uint32_t)0;
    PV624->getControllerStatus((uint32_t*) controllerStatus); 
    sprintf(buffer, "!CC= %08X", controllerStatus);
    sendString(buffer);

    errorStatusRegister.value = 0u; //clear error status register as it has been read now
   
    return duciError;
}
/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")


