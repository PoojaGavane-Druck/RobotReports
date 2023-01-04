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

/**
 * @brief   Start bluetooth device adverts
 * @param   None
 * @param   None
 * @retval  None
 */
bool DDeviceSerialBluetooth::startAdverts(uint8_t *str, uint32_t strLen)
{
    uint32_t retVal = 0u;
    bool flag = false;

    if(str != NULL)
    {
        uint32_t sn = 0u;
        sn = PV624->getSerialNumber(E_ITEM_PV624);
        uint8_t strSerNum[12] = "";
        memset_s(strSerNum, sizeof(strSerNum), 0, sizeof(strSerNum));
        sprintf_s((char *)strSerNum, (rsize_t)12, "%010d\r", sn);
        //lock resource
        DLock is_on(&myMutex);

        retVal = BL652_startAdvertising(strSerNum, str, strLen);

        if(!retVal)
        {
            flag = true;
        }
    }

    return flag;
}

/**
 * @brief   Start bluetooth application
 * @param   None
 * @param   None
 * @retval  None
 */
bool DDeviceSerialBluetooth::startApplication(void)
{

    DLock is_on(&myMutex);
    BL652_sendAtCmd(eBL652_CMD_RUN);
    return true;
}

/**
 * @brief   Stop bluetooth adverts
 * @param   None
 * @param   None
 * @retval  None
 */
bool DDeviceSerialBluetooth::stopAdverts()
{
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    //flag = BL652_stopAdverts();

    return flag;
}

/**
 * @brief   Set bluetooth device mode
 * @param   None
 * @param   None
 * @retval  None
 */
bool DDeviceSerialBluetooth::setDeviceMode(eBL652mode_t mode)
{
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    flag = BL652_initialise(mode);

    return flag;
}

/**
 * @brief   Disconnect bluetooth
 * @param   None
 * @param   None
 * @retval  None
 */
bool DDeviceSerialBluetooth::disconnect()
{
    bool flag = true;

    //lock resource
    DLock is_on(&myMutex);

    flag &= BL652_disconnect();

    flag &= BL652_stopAdverts();

    return flag;
}

/**
 * @brief   checks bluetooth communication interface  working or not
 * @param   void
 * @retval  returns true if suucceeded and false if it fails
 */
bool DDeviceSerialBluetooth::checkBlModulePresence(void)
{
    bool flag = false;
    uint32_t lError = 0u;
    //lock resource
    DLock is_on(&myMutex);
    lError |= BL652_sendAtCmd(eBL652_CMD_Device);

    if(!lError)
    {
        flag = true;
    }

    return flag;
}

/**
 * @brief   Query bluetooth device firmware version
 * @param   buffer to return value in
 * @param   None
 * @retval  flag: true if bluetooth FW version read is sucessful, False if the read has failed
 */
bool DDeviceSerialBluetooth::getFWVersion(char *str)
{
    uint32_t retVal = 0u;
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    //retVal = BL652_getFWVersion(str);
    retVal = BL652_getFirmwareVersion(eBL652_CMD_SWVersion, str);


    if(!retVal)
    {
        flag = true;
    }

    return flag;
}

/**
 * @brief   Query bluetooth device application version
 * @param   buffer to return value in
 * @param   None
 * @retval  flag: true if bluetooth App version read is sucessful, False if the read has failed
 */
bool DDeviceSerialBluetooth::getAppVersion(char *str)
{
    bool retVal = 0u;
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    retVal = BL652_getApplicationVersion(str);

    if(retVal)
    {
        flag = true;
    }

    return flag;
}

/**
 * @brief   erases the file system in BL652
 * @param   None
 * @param   None
 * @retval  flag: true if file system successfully erased, otherwise false
 */
bool DDeviceSerialBluetooth::eraseBL652FileSystem(void)
{
    // erase the bluetooth module file system
    // including the application if present

    //lock resource
    DLock is_on(&myMutex);
    BL652_sendAtCmd(eBL652_CMD_FS_CLEAR);
    return true;
}

/**
 * @brief   open a file in BL652 to copy smart basic APp
 * @param   None
 * @param   None
 * @retval  flag: true if file is opened in BL652, otherwise false
 */
bool DDeviceSerialBluetooth::openFileInBL652ToCopyApp(void)
{
    bool sucessFlag = false;
    uint32_t retVal = 0u;

    //lock resource
    DLock is_on(&myMutex);
    // open a file in the ble module to be written to
    retVal = BL652_sendAtCmd(eBL652_CMD_FOW);

    if(!retVal)
    {
        sucessFlag = true;
    }

    return sucessFlag;
}
/**
 * @brief   writes the count number of bytes into BL652
 * @param   None
 * @param   None
 * @retval  flag: true if write is successful, otherwise false
 */
bool DDeviceSerialBluetooth::writeToTheBl652Module(uint8_t *bufferPtr, uint8_t count)
{
    bool sucessFlag = false;
    uint32_t retVal = 0u;

    //lock resource
    DLock is_on(&myMutex);

    if((bufferPtr != NULL) && (count > 0u))
    {
        // open a file in the ble module to be written to
        retVal = BL652_writeModule(eBL652_CMD_FWRH, (uint8_t *)bufferPtr, count);

        if(!retVal)
        {
            sucessFlag = true;
        }
    }

    return sucessFlag;
}
/**
 * @brief   open a file in BL652 to copy smart basic APp
 * @param   None
 * @param   None
 * @retval  flag: true if file is opened in BL652, otherwise false
 */
bool DDeviceSerialBluetooth::closeFile(void)
{
    bool sucessFlag = false;
    uint32_t retVal = 0u;

    //lock resource
    DLock is_on(&myMutex);
    // open a file in the ble module to be written to
    retVal = BL652_sendAtCmd(eBL652_CMD_FCL);

    if(!retVal)
    {
        sucessFlag = true;
    }

    return sucessFlag;
}

/*!
* @brief : This function resets the Bluetooth module
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - true = ok, false = fail
* @note          : None
* @warning       : None
*/
bool DDeviceSerialBluetooth::resetBL652(void)
{
    bool sucessFlag = false;
    uint32_t retVal = 0u;

    //lock resource
    DLock is_on(&myMutex);
    // open a file in the ble module to be written to
    retVal = BL652_reset();

    if(!retVal)
    {
        sucessFlag = true;
    }

    return sucessFlag;
}

/*!
* @brief : This function resets the Bluetooth module
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - true = ok, false = fail
* @note          : None
* @warning       : None
*/
bool DDeviceSerialBluetooth::getFileListBl652(void)
{
    bool sucessFlag = false;
    uint32_t retVal = 0u;

    //lock resource
    DLock is_on(&myMutex);
    // open a file in the ble module to be written to
    retVal = BL652_sendAtCmd(eBL652_CMD_DIR);

    if(!retVal)
    {
        sucessFlag = true;
    }

    return sucessFlag;
}
/**
 * @brief   Query Checksum of &autorun$ file
 * @param   char *str: buffer to return checksum value
 * @retval  flag: true if &autorun$ file checksum read is sucessful, False if the read has failed
 */
bool DDeviceSerialBluetooth::getChecksumBl652(uint16_t *receivedChecksum)
{
    uint32_t retVal = 0u;
    bool flag = false;

    //lock resource
    DLock is_on(&myMutex);

    // get checksum command
    retVal = BL652_getChecksum(eBL652_CMD_ATI_C12C, receivedChecksum);

    if(!retVal)
    {
        flag = true;
    }

    return flag;
}
