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
#include "DDeviceSerialOwiInterface1.h"
#include "uart.h"
#include "DLock.h"

/* Error handler instance parameter starts from 6001 to 6100 */

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define WAIT_TILL_END_OF_FRAME_RECEIVED 0u
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DDeviceSerialOwiInterface1 class constructor
 * @param   void
 * @retval  void
 */
DDeviceSerialOwiInterface1::DDeviceSerialOwiInterface1()
{
    createMutex("OwiInterface1");

}

/**
 * @brief   Clear Receive buffer
 * @param   void
 * @retval  void
 */
void DDeviceSerialOwiInterface1::clearRxBuffer(void)
{
    DLock is_on(&myMutex);
    ClearUARTxRcvBuffer(UART_PORT2);
}

/**
 * @brief   Send String
 * @param   str - pointer to null-terminated character string to transmit
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialOwiInterface1::sendString(char *str, uint32_t buffSize)
{
    DLock is_on(&myMutex);
    sendOverUSART2((uint8_t *)str, (uint32_t)strnlen_s(str, buffSize));
    return true;
}


/**
 * @brief   Returns the number of bytes in the UART receive buffer
 * @param   str - pointer to null-terminated character string to transmit
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialOwiInterface1::getRcvBufLength(uint16_t *length)
{
    bool flag = false;

    flag = getAvailableUARTxReceivedByteCount(UART_PORT2, length);

    return flag;
}

/**
 * @brief   Receive String
 * @param   pStr - address of pointer to string
 * @param   waitTime - time in ms to wait for receive string
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialOwiInterface1::receiveString(char **pStr, uint32_t waitTime)
{
    bool flag = false;

    DLock is_on(&myMutex);

    if(waitToReceiveOverUsart2(WAIT_TILL_END_OF_FRAME_RECEIVED, waitTime))
    {
        flag = getHandleToUARTxRcvBuffer(UART_PORT2, (uint8_t **)pStr);

        if(*pStr == NULL)
        {
            flag = false;
        }

    }

    return flag;
}

bool DDeviceSerialOwiInterface1::read(uint8_t **pStr,
                                      uint32_t numOfBytesToRead,
                                      uint32_t *numOfBytesRead,
                                      uint32_t waitTime)
{
    bool flag = false;
    uint16_t receivedByteCount = 0u;
    DLock is_on(&myMutex);

    if(waitToReceiveOverUsart2(numOfBytesToRead, waitTime))
    {
        flag = getAvailableUARTxReceivedByteCount(UART_PORT2, &receivedByteCount);
        {
            if(true == flag)
            {
                *numOfBytesRead = receivedByteCount;
            }

            else
            {
                *numOfBytesRead = 0u;
            }
        }
        flag = getHandleToUARTxRcvBuffer(UART_PORT2, (uint8_t **)pStr);

        if(*pStr == NULL)
        {
            flag = false;
        }
    }

    return flag;
}
bool DDeviceSerialOwiInterface1::write(uint8_t *str, uint32_t numOfBytesToWrite)
{
    DLock is_on(&myMutex);
    sendOverUSART2(str, numOfBytesToWrite);
    return true;
}

bool DDeviceSerialOwiInterface1::query(uint8_t *str,
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
    ClearUARTxRcvBuffer(UART_PORT2);

    //send command
    sendOverUSART2(str, cmdLength);

    //wait for response
    if(waitToReceiveOverUsart2(responseLen, waitTime))
    {
        flag = getHandleToUARTxRcvBuffer(UART_PORT2, (uint8_t **)pStr);

        if(*pStr == NULL)
        {
            flag = false;
        }
    }

    return flag;
}