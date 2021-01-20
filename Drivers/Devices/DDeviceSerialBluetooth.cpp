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
* @file     DDeviceSerialBluetooth.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     13 November 2020
*
* @brief    The Bluetooth serial communications driver base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DDeviceSerialBluetooth.h"
#include "DLock.h"
//#include "gpio.h"
#include "cUartBluetooth.h"
#include "UART.h"
#include "cBL652.h"
//#include "stm32l4xx_hal_uart.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
extern UART_HandleTypeDef huart1;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DDeviceSerialBluetooth class constructor
 * @param   void
 * @retval  void
 */
DDeviceSerialBluetooth::DDeviceSerialBluetooth()
{
    createMutex("BLE");

    //Initialise BL652
    BL652_initialise(eBL652_MODE_DISABLE);
#ifdef BLUETOOTH_MODULE_ENABLED
    //initialise Bluetooth UART
    UARTn_init( &huart1 );
#endif
    //turn on power - Bluetooth is always on as external sensors are plug-and-play
    //UART_Bluetooth_power(eUART_Bluetooth_POWER_ON);
}

/**
 * @brief   Clear Receive buffer
 * @param   void
 * @retval  void
 */
void DDeviceSerialBluetooth::clearRxBuffer(void)
{
    DLock is_on(&myMutex);
#ifdef BLUETOOTH_MODULE_ENABLED
    if( false == UARTn_ClearRcvBuffer( &huart1 ))
    {
        //TODO Error
    }
#endif
}

/**
 * @brief   Send String
 * @param   str - pointer to null-terminated character string to transmit
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialBluetooth::sendString(char *str)
{
    DLock is_on(&myMutex);
#ifdef BLUETOOTH_MODULE_ENABLED
    UARTn_send(&huart1, ( uint8_t* )str, (uint32_t)strlen(str));
#endif
    return true;
}

/**
 * @brief   Receive String
 * @param   pStr - address of pointer to string
 * @param   waitTime - time in ms to wait for receive string (0u = wait forever)
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialBluetooth::receiveString(char **pStr, uint32_t waitTime)
{
    bool flag = false;
#ifdef BLUETOOTH_MODULE_ENABLED
    DLock is_on(&myMutex);

    if (UARTn_rcvWait(&huart1, waitTime))
    {
        *pStr = (char *)UARTn_getRcvBuffer(&huart1);

        if (*pStr != NULL)
        {
            flag = true;
        }
    }
#endif
    return flag;
}

/**
 * @brief   Send string and then wait for specified wait time for the expected reply.
 * @note    This is a combined send and receive with a resource lock around it.
 * @param   str - pointer to character string to transmit
 * @param   pStr - address of pointer to string
 * @param   waitTime - time in ms to wait for receive string (0u = wait forever)
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialBluetooth::query(char *str, char **pStr, uint32_t waitTime)
{
    bool flag = false;
#ifdef BLUETOOTH_MODULE_ENABLED
    //lock resource
    DLock is_on(&myMutex);

    //TODO: it is safe to call function in same thread as resource will still be locked.
    //Check that is true.

    //clear receive buffer

    if( false == UARTn_ClearRcvBuffer( &huart1 ))
    {
        //TODO Error
    }

    //send command
    UARTn_send(&huart1, ( uint8_t* )str, (uint32_t)strlen(str));

    //wait for response
    if (UARTn_rcvWait(&huart1,waitTime))
    {
        //pass back received reply
        *pStr = ( char * )UARTn_getRcvBuffer(&huart1);

        if (*pStr != NULL)
        {
            flag = true;
        }
    }
#endif
    return flag;
}
