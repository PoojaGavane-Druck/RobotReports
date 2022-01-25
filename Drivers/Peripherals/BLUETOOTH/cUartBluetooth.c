/**
 * Baker Hughes Confidential
* Copyright 2019.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are protected by trade
* secret or copyright law.  Dissemination of this information or reproduction of this material is strictly forbidden unless prior written
* permission is obtained from Baker Hughes.
*
* @file     cUartBluetooth.c
* @author   Elvis Esa
* @date     13 November 2020
* @brief
*/

#include "misra.h"
#include "cUartBluetooth.h"
#include "Utilities.h"

//MISRAC_DISABLE
//#include <stm32l4xx_hal.h>
//#include <ctype.h>
//#include <cpu.h>
//#include <lib_mem.h>
//#include <os.h>
////#include <bsp_clk.h>
//#include <bsp_os.h>
////#include <bsp_int.h>
//#include <os_app_hooks.h>
//#include <string.h>
//#include <DLib_Product_string.h>
//MISRAC_ENABLE

// Definitions

// Static Variables

// Static Functions

/**
 * @brief this function performs the power on/off for the BLUETOOTH interface
 *        it also checks for the over current detection after power up
 *        (after the minimum power up delay)
 */
bool UART_Bluetooth_power(eUartPower_t ePower)
{
    return false;
}

/**
 * @brief This function returns the state of the overcurrent pin
 *        false = overcurrent, true = ok
 */
bool UART_Bluetooth_overCurrentOk(void)
{
    return false;
}

/**
 * @brief this function inits the UART driver
 */
void UART_Bluetooth_init(void)
{
}

/**
 * @brief gets the size of the buffer holding the UART receive string
 * @return size of the buffer
 */
uint16_t UART_Bluetooth_getSize(void)
{
    return 0u;
}

/**
 * @brief sends a string
 * @param aTxBuffer
 * @param size
 */
void UART_Bluetooth_send(char *aTxBuffer, uint32_t size)
{
}

/**
 * @brief   Clear buffer and set interrupts and semaphore to receive message
 * @return  void
 */
bool UART_Bluetooth_ClearRcvBuffer(void)
{
    return false;
}

/**
 * @brief waits for a message to be received
 * @param max timeout for the waiting semaphore (in milliseconds)
 */
bool UART_Bluetooth_rcvWait(uint32_t max)
{
    return false;
}

/**
 * @brief gets pointer to the buffer holding the UART receive string
 * @return the pointer to the union of buffers
 */
char *UART_Bluetooth_getRcvBuffer(void)
{
    return NULL;
}
