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
* @file     DDeviceSerialRS485.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The RS485 serial communications driver base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DDeviceSerialRS485.h"
#include "uart.h"
#include "DLock.h"
#include "gpio.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DDeviceSerialRS485 class constructor
 * @param   void
 * @retval  void
 */
DDeviceSerialRS485::DDeviceSerialRS485()
{
    createMutex("RS485");
    USART_ConfigParams configParams;
    configParams.baudRate = BAUDRATE_38400;
    configParams.dataLength = DATA_LENGTH_8BITS;
    configParams.direction = DIRECTION_TX_RX;
    configParams.flowControlMode = FLOW_CONTROL_NONE;
    configParams.numOfStopBits = STOPBITS_1;
    configParams.overSamplingType = OVER_SAMPLE_BY_16;
    configParams.parityType = PARITY_NONE;
    configParams.portNumber = UART_PORT5;
    
    uartInit(configParams);

    //TODO:enable the comms medium
    //extSensorOnOffLatchEnable();
}

/**
 * @brief   Clear Receive buffer
 * @param   void
 * @retval  void
 */
void DDeviceSerialRS485::clearRxBuffer(void)
{
    DLock is_on(&myMutex);
    ClearUARTxRcvBuffer(UART_PORT5);
}

/**
 * @brief   Send String
 * @param   str - pointer to null-terminated character string to transmit
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialRS485::sendString(char *str)
{
    DLock is_on(&myMutex);
    sendOverUART5((uint8_t*)str, (uint32_t)strlen(str));
    return true;
}

/**
 * @brief   Receive String
 * @param   pStr - address of pointer to string
 * @param   waitTime - time in ms to wait for receive string
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialRS485::receiveString(char **pStr, uint32_t waitTime)
{
    bool flag = false;

    DLock is_on(&myMutex);

    if (waitToReceiveOverUart5(0u, waitTime))
    {
        flag = getHandleToUARTxRcvBuffer(UART_PORT5,(uint8_t **)pStr);       

        if (*pStr == NULL)
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
bool DDeviceSerialRS485::query(char *str, char **pStr, uint32_t waitTime)
{
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    //TODO: it is safe to call function in same thread as resource will still be locked.
    //Check that is true.

    //clear recieve buffer
     ClearUARTxRcvBuffer(UART_PORT5);

    //send command
    sendOverUART5((uint8_t*)str, (uint32_t)strlen(str));

    //wait for response
    if (waitToReceiveOverUart5(0u,waitTime))
    {
       flag = getHandleToUARTxRcvBuffer(UART_PORT5, (uint8_t **)pStr);  
       
        if (*pStr == NULL)
        {
            flag = false;
        }
    }

    return flag;
}
