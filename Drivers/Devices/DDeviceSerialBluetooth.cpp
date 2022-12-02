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
#include "DPV624.h"
#include "DLock.h"
#include "cUartBluetooth.h"
#include "UART.h"
#include "cBL652.h"

/* Error handler instance parameter starts from 5901 to 6000 */

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define WAIT_TILL_END_OF_FRAME_RECEIVED 0u
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

    bool ok = true;

    //Initialise BL652
    ok = BL652_initialise(eBL652_MODE_DEV);

    if(!ok)
    {
        PV624->errorHandler->handleError(E_ERROR_CODE_DRIVER_BLUETOOTH,
                                         eSetError,
                                         0u,
                                         5901u,
                                         false);
    }

    PV624->setBlState(BL_STATE_DISABLE);

    //initialise Bluetooth UART
    ok = uartInit(&huart1);

    if(ok)
    {
        PV624->errorHandler->handleError(E_ERROR_CODE_DRIVER_BLUETOOTH,
                                         eSetError,
                                         0u,
                                         5902u,
                                         false);
    }

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

    if(false == ClearUARTxRcvBuffer(UART_PORT1))
    {
        PV624->errorHandler->handleError(E_ERROR_CODE_DRIVER_BLUETOOTH,
                                         eSetError,
                                         0u,
                                         5903u,
                                         false);
    }
}

/**
 * @brief   Send String
 * @param   str - pointer to null-terminated character string to transmit
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialBluetooth::sendString(char *str, uint32_t buffSize)
{
    DLock is_on(&myMutex);
    memset_s(blTxString, TX_BUFFER_SIZE, 0, TX_BUFFER_SIZE);
    memcpy_s(blTxString, TX_BUFFER_SIZE, "vw ", (size_t)3);
    memcpy_s(&blTxString[3], TX_BUFFER_SIZE - 3u, (int8_t *)str, (size_t)strnlen_s(str, buffSize));
    uint32_t blLength = (uint32_t)strnlen_s(blTxString, buffSize);
    blTxString[blLength] = '\0';
    sendOverUSART1((uint8_t *)blTxString, (uint32_t)strnlen_s(blTxString, TX_BUFFER_SIZE));

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
    DLock is_on(&myMutex);

    if(waitToReceiveOverUsart1(WAIT_TILL_END_OF_FRAME_RECEIVED, waitTime))
    {
        flag = getHandleToUARTxRcvBuffer(UART_PORT1, (uint8_t **)pStr);

        if(*pStr != NULL)
        {
            flag = true;
        }
    }

    return flag;
}

/**
 * @brief   Query device ID
 * @param   buffer to return value in
 * @param   buffer size
 * @retval  flag: true if success, fals if failure
 */
bool DDeviceSerialBluetooth::getDeviceId(char *buffer, int32_t size)
{
    bool flag = false;

#if 1    //TODO HSB: ELVIS?
    snprintf_s(buffer, (size_t)16, "BL652");
    flag = true;
#endif

    return flag;
}
