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
* @file     DDeviceSerialUSB.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     05 May 2020
*
* @brief    The USB serial communications driver base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DDeviceSerialUSB.h"
#include "DLock.h"

MISRAC_DISABLE
#include "usb_device.h"
#include "usbd_cdc_if.h"
MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
#define RX_SEMA usbSemRcv
extern OS_SEM usbSemRcv;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DDeviceSerialUSB class constructor
 * @param   void
 * @retval  void
 */
DDeviceSerialUSB::DDeviceSerialUSB()
{
    createMutex("USB");
}

/**
 * @brief   Clear Receive buffer
 * @param   void
 * @retval  void
 */
void DDeviceSerialUSB::clearRxBuffer(void)
{
    DLock is_on(&myMutex);
    VCP_clear();
}

/**
 * @brief   Send String
 * @param   str - pointer to null-terminated character string to transmit
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialUSB::sendString(char *str)
{
    //lock resource
    //DLock is_on(&myMutex);

    //send command
    CDC_Transmit_FS((uint8_t *)str, (uint16_t)strlen((char *)str));

    return true;
}



/**
* @brief   Send String
* @param   str - pointer to null-terminated character string to transmit
* @retval  flag - true = success, false = failed
*/
bool DDeviceSerialUSB::write(uint8_t *str, uint32_t numOfBytesToWrite)
{
    //lock resource
    //DLock is_on(&myMutex);

    //send command
    CDC_Transmit_FS((uint8_t *)str, (uint16_t)numOfBytesToWrite);

    return true;
}
/**
 * @brief   Receive String
 * @param   pStr - address of pointer to string
 * @param   waitTime - time in ms to wait for receive string
 * @retval  flag - true = success, false = failed
 */
bool DDeviceSerialUSB::receiveString(char **pStr, uint32_t waitTime)
{
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    //wait for response
    if(rcvWait(waitTime))
    {
        *pStr = (char *)VCP_read();

        if(*pStr != NULL)
        {
            flag = true;
        }
    }

    return flag;
}


bool DDeviceSerialUSB::read(uint8_t **pStr, uint32_t numOfBytesToRead, uint32_t *numOfBytesRead, uint32_t waitTime)
{
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    //wait for response
    if(rcvWait(waitTime))
    {
        *pStr = (uint8_t *)VCP_read();

        if(*pStr != NULL)
        {
            *numOfBytesRead = numOfBytesToRead;
            flag = true;
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
bool DDeviceSerialUSB::query(char *str, char **pStr, uint32_t waitTime)
{
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    //clear recieve buffer
    VCP_clear();

    //send command
    CDC_Transmit_FS((uint8_t *)str, (uint16_t)strlen((char *)str));

    //wait for response
    if(rcvWait(waitTime))
    {
        *pStr = (char *)VCP_read();

        if(*pStr != NULL)
        {
            flag = true;
        }
    }

    return flag;
}

/**
 * @brief waits for a message to be received
 * @param max timeout for the waiting semaphore (in milliseconds)
 */
bool DDeviceSerialUSB::rcvWait(uint32_t max)
{
    bool wait = false;

    OS_ERR p_err;
    OSSemPend(&RX_SEMA, max, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err);

    if(p_err == static_cast<OS_ERR>(OS_ERR_NONE))
    {
        wait = (VCP_kbhit() != 0u);
    }

    return wait;
}
