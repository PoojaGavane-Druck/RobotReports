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
* @file     DDeviceSerialRS485.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The RS485 serial communications driver base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DDeviceSerialOwiInterface2.h"
#include "uart.h"
#include "DLock.h"


/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define WAIT_TILL_END_OF_FRAME_RECEIVED 0u

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DDeviceSerialRS485 class constructor
 * @param   void
 * @retval  void
 */
DDeviceSerialOwiInterface2::DDeviceSerialOwiInterface2()
{
    createMutex("OwiInterface2");

    //enable the comms medium
    enableSerialPortTxLine(UART_PORT4);
}

/**
 * @brief   Clear Receive buffer
 * @param   void
 * @retval  void
 */
void DDeviceSerialOwiInterface2::clearRxBuffer(void)
{
    DLock is_on(&myMutex);
    ClearUARTxRcvBuffer(UART_PORT4);
}

/**
 * @brief   Send String
 * @param   str - pointer to null-terminated character string to transmit
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialOwiInterface2::sendString(char *str)
{
    DLock is_on(&myMutex);
    sendOverUART4((uint8_t *)str, (uint32_t)strlen(str));
    return true;
}

/**
 * @brief   Receive String
 * @param   pStr - address of pointer to string
 * @param   waitTime - time in ms to wait for receive string
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialOwiInterface2::receiveString(char **pStr, uint32_t waitTime)
{
    bool flag = false;

    DLock is_on(&myMutex);
    ClearUARTxRcvBuffer((PortNumber_t)(UART_PORT4));

    if(waitToReceiveOverUart4(WAIT_TILL_END_OF_FRAME_RECEIVED, waitTime))
    {

        flag = getHandleToUARTxRcvBuffer(UART_PORT4, (uint8_t **)pStr);

        if(*pStr == NULL)
        {
            flag = false;
        }
    }

    return flag;
}

/**
 * @brief   Send string and then wait for specified wait time for the expected reply.
 * @note    This is a combined send and receive with a resource lock around it.
 * @param   str - pointer to character string to transmit
 * @param   pStr - address of pointer to string
 * @param   waitTime - time in ms to wait for receive string
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialOwiInterface2::query(char *str, char **pStr, uint32_t waitTime)
{
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    //TODO: it is safe to call function in same thread as resource will still be locked.
    //Check that is true.

    //clear recieve buffer
    ClearUARTxRcvBuffer(UART_PORT4);

    //send command
    sendOverUART4((uint8_t *)str, (uint32_t)strlen(str));

    //wait for response
    if(waitToReceiveOverUart4(WAIT_TILL_END_OF_FRAME_RECEIVED, waitTime))
    {
        flag = getHandleToUARTxRcvBuffer(UART_PORT4, (uint8_t **)pStr);

        if(*pStr == NULL)
        {
            flag = false;
        }
    }

    return flag;
}

bool DDeviceSerialOwiInterface2::read(uint8_t **pStr,
                                      uint32_t numOfBytesToRead,
                                      uint32_t *numOfBytesRead,
                                      uint32_t waitTime)
{
    bool flag = false;
    uint16_t receivedByteCount = 0u;
    DLock is_on(&myMutex);

    ClearUARTxRcvBuffer((PortNumber_t)(UART_PORT4));

    waitToReceiveOverUart4(numOfBytesToRead, waitTime);
    flag = getAvailableUARTxReceivedByteCount(UART_PORT4,
            &receivedByteCount);

    if(true == flag)
    {
        *numOfBytesRead = receivedByteCount;
    }

    else
    {
        *numOfBytesRead = 0u;
    }


    flag = getHandleToUARTxRcvBuffer(UART_PORT4, (uint8_t **)pStr);

    if(*pStr == NULL)
    {
        flag = false;
    }

    return flag;
}


bool DDeviceSerialOwiInterface2::write(uint8_t *str, uint32_t numOfBytesToWrite)
{
    DLock is_on(&myMutex);
    sendOverUART4((uint8_t *)str, numOfBytesToWrite);
    return true;
}

bool DDeviceSerialOwiInterface2::query(uint8_t *str,
                                       uint32_t cmdLength,
                                       uint8_t **pStr,
                                       uint32_t responseLen,
                                       uint32_t waitTime)
{
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    //TODO: it is safe to call function in same thread as resource will still be locked.
    //Check that is true.

    //clear recieve buffer
    ClearUARTxRcvBuffer(UART_PORT4);

    //send command
    sendOverUART4((uint8_t *)str, cmdLength);

    //wait for response
    if(waitToReceiveOverUart4(responseLen, waitTime))
    {
        flag = getHandleToUARTxRcvBuffer(UART_PORT4, (uint8_t **)pStr);

        if(*pStr == NULL)
        {
            flag = false;
        }
    }

    return flag;
}