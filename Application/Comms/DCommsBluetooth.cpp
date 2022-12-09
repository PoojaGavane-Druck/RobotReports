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
* @file     DCommsBluetooth.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     13 November 2020
*
* @brief    The Bluetooth communications class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsBluetooth.h"
#include "DDeviceSerialBluetooth.h"
#include "DCommsFsmBluetooth.h"

/* Error handler instance parameter starts from 301 to 400 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsBluetooth class constructor
 * @param   medium name
 * @param   os error
 * @retval  void
 */
DCommsBluetooth::DCommsBluetooth(char *mediumName, OS_ERR *os_error)
    : DComms()
{
    myName = mediumName;

    //start the comms task
    start(mediumName, os_error);
}

/* @brief   DCommsBluetooth class destructor
* @param   void
* @retval  void
*/
DCommsBluetooth::~DCommsBluetooth()
{
    if(NULL != commsMedium)
    {
        delete commsMedium;
    }

    if(NULL != myCommsFsm)
    {
        delete myCommsFsm;
    }
}
/**
 * @brief   DCommsBluetooth initialisation function (overrides the base class)
 * @param   void
 * @retval  void
 */
void DCommsBluetooth::initialise(void)
{

    myTaskId = eCommunicationOverBluetoothTask;
    //specify the comms medium
    commsMedium = new DDeviceSerialBluetooth();

    //create the comms state machine- having setup the medium first
    myCommsFsm = new DCommsFsmBluetooth();
}

/**
 * @brief   Set (enter/exit) test mode
 * @param   state: value true = enter test mode, false = exit test mode
 * @retval  void
 */
void DCommsBluetooth::setTestMode(bool state)
{
    if(myCommsFsm != NULL)
    {
        if(state == true)
        {
            myCommsFsm->suspend();
        }

        else
        {
            myCommsFsm->resume();
        }
    }
}

/**
 * @brief   Query Bluetooth device ID
 * @param   buffer to return value in
 * @param   buffer size
 * @retval  flag: true if success, fals if failure
 */
bool DCommsBluetooth::getDeviceId(char *buffer, int32_t size)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        flag = myMedium->getDeviceId(buffer, size);
    }

    return flag;
}
/**
 * @brief   Set Bluetooth Pairing Status
 * @param   pairing status
 * @param   None
 * @retval  flag: true if success, fals if failure
 */
bool DCommsBluetooth::setPairingStatus(eBL652PairingMode_t pairState)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        flag = myMedium->setPairingStatus(pairState);
    }

    return flag;
}

/**
 * @brief   Get the indication of if the bluetooth module is fitted
 * @param   None
 * @param   None
 * @retval  Bluetooth module indication
 */
bool DCommsBluetooth::checkBlModulePresence(void)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        flag = myMedium->checkBlModulePresence();
    }

    return flag;
}

/**
 * @brief   Query Bluetooth FW software version
 * @param   buffer to return value in
 * @param   None
 * @retval  flag: true if success, false if failure
 */
bool DCommsBluetooth::getFWVersion(char *str)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        flag = myMedium->getFWVersion(str);
    }

    return flag;
}

/**
 * @brief   Query Bluetooth application software version
 * @param   buffer to return value in
 * @param   None
 * @retval  flag: true if success, false if failure
 */
bool DCommsBluetooth::getAppVersion(char *str)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        flag = myMedium->getAppVersion(str);
    }

    return flag;
}

/**
 * @brief   Start Bluetooth adverts
 * @param   None
 * @param   None
 * @retval  flag: true if success, false if failure
 */
bool DCommsBluetooth::startAdverts(uint8_t *str, uint32_t strLen)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if((myMedium != NULL) && (str != NULL))
    {
        flag = myMedium->startAdverts(str, strLen);
    }

    return flag;
}

/**
 * @brief   Stop Bluetooth adverts
 * @param   None
 * @param   None
 * @retval  flag: true if success, false if failure
 */
bool DCommsBluetooth::stopAdverts(void)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        flag = myMedium->stopAdverts();
    }

    return flag;
}

/**
 * @brief   Disconnect Bluetooth
 * @param   None
 * @param   None
 * @retval  flag: true if success, false if failure
 */
bool DCommsBluetooth::disconnect(void)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        flag = myMedium->disconnect();
    }

    return flag;
}

/**
 * @brief   Start the Bluetooth Application
 * @param   None
 * @param   None
 * @retval  flag: true if success, false if failure
 */
bool DCommsBluetooth::startApplication(void)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        flag = myMedium->startApplication();
    }

    return flag;
}

/**
 * @brief   Set Bluetooth Device Mode
 * @param   Mode
 * @param   None
 * @retval  flag: true if success, false if failure
 */
bool DCommsBluetooth::setDeviceMode(eBL652mode_t commsMode)
{
    bool flag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        myMedium->setDeviceMode(commsMode);
    }

    return flag;
}

/**
 * @brief   erases the file system in BL652
 * @param   None
 * @param   None
 * @retval  flag: true if file system successfully erased, otherwise false
 */
bool DCommsBluetooth::eraseBL652FileSystem(void)
{
    bool sucessFlag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *) commsMedium;

    if(myMedium != NULL)
    {
        sucessFlag = myMedium->eraseBL652FileSystem();
    }

    return sucessFlag;
}

/**
 * @brief   open a file in BL652 to copy smart basic APp
 * @param   None
 * @param   None
 * @retval  flag: true if file is opened in BL652, otherwise false
 */
bool DCommsBluetooth::openFileInBL652ToCopyApp(void)
{

    bool sucessFlag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *) commsMedium;

    if(myMedium != NULL)
    {
        sucessFlag = myMedium->openFileInBL652ToCopyApp();
    }

    return sucessFlag;
}
/**
 * @brief   writes the count number of bytes into BL652
 * @param   None
 * @param   None
 * @retval  flag: true if write is successful, otherwise false
 */
bool DCommsBluetooth::writeToTheBl652Module(uint8_t *bufferPtr, uint8_t count)
{
    bool sucessFlag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *) commsMedium;

    if((myMedium != NULL) && (bufferPtr != NULL) && (count > 0u))
    {
        sucessFlag = myMedium->writeToTheBl652Module(bufferPtr, count);
    }

    return sucessFlag;
}
/**
 * @brief   open a file in BL652 to copy smart basic APp
 * @param   None
 * @param   None
 * @retval  flag: true if file is opened in BL652, otherwise false
 */
bool DCommsBluetooth::closeFile(void)
{
    bool sucessFlag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        sucessFlag = myMedium->closeFile();
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
bool DCommsBluetooth::resetBL652(void)
{
    bool sucessFlag = false;

    DDeviceSerialBluetooth *myMedium = (DDeviceSerialBluetooth *)commsMedium;

    if(myMedium != NULL)
    {
        sucessFlag = myMedium->resetBL652();
    }

    return sucessFlag;
}