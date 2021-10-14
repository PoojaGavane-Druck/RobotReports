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
* @file     DDeviceSerial.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     03 April 2020
*
* @brief    The serial communications driver base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DDeviceSerial.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DDeviceSerial class constructor
 * @param   void
 * @retval  void
 */
DDeviceSerial::DDeviceSerial()
: DDevice()
{
}

/**
 * @brief   Clear Receive buffer
 * @param   void
 * @retval  void
 */
void DDeviceSerial::clearRxBuffer(void)
{
    //TODO: driver has its own buffer - do we want to pass this one instead?
//    for (uint32_t i = 0u; i < SERIAL_RECEIVE_BUFFER_SIZE; i++)
//    {
//        rxBuffer[i] = '\0';
//    }
}

/**
 * @brief   Send String
 * @param   str - pointer to character string to transmit
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerial::sendString(char *str)
{
    return false;
}

/**
 * @brief   Receive String
 * @param   pStr - address of pointer to string
 * @param   waitTime - time in ms to wait for receive string
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerial::receiveString(char **pStr, uint32_t waitTime)
{
    return false;
}

/**
 * @brief   Send string and then wait for specified wait time for the expected reply
 * @param   str - pointer to character string to transmit
 * @param   pStr - address of pointer to string
 * @param   waitTime - time in ms to wait for receive string
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerial::query(char *str, char **pStr, uint32_t waitTime)
{
  //This is a combined send and receive with a resource lock around it

    return false;
}

/**
 * @brief   Get address of transmit buffer
 * @param   void
 * @retval  address of transmit buffer
 */
char *DDeviceSerial::getTxBuffer(void)
{
    return txBuffer;
}

/**
 * @brief   Returns the number of bytes in the UART receive buffer
 * @param   str - pointer to null-terminated character string to transmit
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerial::getRcvBufLength(uint16_t *length)
{
    return false;
}


/**
 * @brief   Get size of transmit buffer
 * @param   void
 * @retval  size of transmit buffer
 */
uint32_t DDeviceSerial::getTxBufferSize(void)
{
    return TX_BUFFER_SIZE;
}


bool DDeviceSerial::read(uint8_t **pStr, uint32_t numOfBytesToRead, uint32_t *numOfBytesRead, uint32_t waitTime)
{
  return false;
}


bool DDeviceSerial::write(uint8_t *str, uint32_t numOfBytesToWrite)
{
  return false;
}

bool DDeviceSerial::query(uint8_t *str,
                  uint32_t cmdLength,
                   uint8_t **pStr,
                   uint32_t responseLen,
                   uint32_t waitTime)
{
  return false;
}
