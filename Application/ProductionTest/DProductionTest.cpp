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
* @file     DProductionTest.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     18 September 2020
*
* @brief    The production test functions source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DProductionTest.h"
#include "DPV624.h"
#include "DLock.h"
#include "main.h"


MISRAC_DISABLE

#include <assert.h>
#include "ospi_nor_mx25l25645.h"
MISRAC_ENABLE

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
DProductionTest *DProductionTest::myInstance = NULL;

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProductionTest class constructor
 * @param   void
 * @return  void
 */
DProductionTest::DProductionTest(void)
{
    eepromSelfTestStatus = 0;       //test status values: 0 = in progress (or not started); 1 = passed; -1 = failed
    spiFlashSelfTestStatus = 0;     //test status values: 0 = in progress (or not started); 1 = passed; -1 = failed
    usbSelfTestStatus = 0;          //test status values: 0 = in progress (or not started); 1 = passed; -1 = failed

    myReadyState = false;           //task is not yet up and running

    myWaitFlags = EV_FLAG_SELF_TEST_EEPROM | EV_FLAG_TASK_SELF_TEST_FLASH | EV_FLAG_TASK_SELF_TEST_USB;
}

/**
 * @brief   Create and start up production test task
 * @param   void
 * @return  void
 */
void DProductionTest::start(void)
{
    OS_ERR os_error = OS_ERR_NONE;
    CPU_STK_SIZE stackSize = (CPU_STK_SIZE)256u;
    CPU_STK_SIZE stackBytes = stackSize * (CPU_STK_SIZE)sizeof(CPU_STK_SIZE);

    // Prerequisites
    MISRAC_DISABLE
    assert(PV624 != NULL);   
    assert(PV624->keyHandler != NULL);
    //ToDo: Need to add other required modules
   
    MISRAC_ENABLE

    //create event flags for production test
    OSFlagCreate(&myEventFlags, NULL, (OS_FLAGS)0, &os_error);

    if (os_error == static_cast<OS_ERR>(OS_ERR_NONE))
    {
        //create mutex for resource locking
        OSMutexCreate(&myMutex, (CPU_CHAR*)NULL, &os_error);
    }

    if (os_error == static_cast<OS_ERR>(OS_ERR_NONE))
    {
        //create task for production test
        OSTaskCreate(&myTaskTCB,
                     (CPU_CHAR *)NULL,          //no name given to task
                     DProductionTest::runFunction,
                     (void *)this,
                     (OS_PRIO)5u,
                     (CPU_STK *)new char[stackBytes],
                     (CPU_STK_SIZE)(stackSize / 10u),
                     (CPU_STK_SIZE)stackSize,
                     (OS_MSG_QTY)10u,           //task queue size
                     (OS_TICK)0u,
                     (void *)0u,
                     (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     &os_error);
    }

    //if one or more OS functions above failed in any way then need to raise error
    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_OBJ_CREATED));
    if (!ok)
    {
        //something went wrong
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE

        error_code_t errorCode;
        errorCode.bit.osError = SET;
        PV624->handleError(errorCode, os_error);
    }
}

/**
 * @brief   Production test task main loop function
 * @param   p_arg pointer to this instance
 * @return  void
 */
void DProductionTest::runFunction(void *p_arg)
{
    //this is a while loop that pends on event flags
    OS_ERR os_error = OS_ERR_NONE;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
    DProductionTest *thisTask = (DProductionTest *)p_arg;

    //instance will override initialise function if required
    thisTask->initialise();

    //task is now up and running
    thisTask->setReadyState();

    //task main loop
    while (DEF_TRUE)
    {
        actualEvents = OSFlagPend(&thisTask->myEventFlags,
                                  thisTask->myWaitFlags, (OS_TICK)0u,
                                  OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                  &cpu_ts,
                                  &os_error);

        //check flags to determine what to execute
        if ((actualEvents & EV_FLAG_SELF_TEST_EEPROM) == EV_FLAG_SELF_TEST_EEPROM)
        {
            thisTask->performEepromSelfTest();
        }

        if ((actualEvents & EV_FLAG_TASK_SELF_TEST_FLASH) == EV_FLAG_TASK_SELF_TEST_FLASH)
        {
            thisTask->performSpiFlashSelfTest();
        }

        if ((actualEvents & EV_FLAG_TASK_SELF_TEST_USB) == EV_FLAG_TASK_SELF_TEST_USB)
        {
            thisTask->performUsbSelfTest();
        }
    }

    //clean up would normally be done here, but this task never terminates, so not needed (should never get here)
}

/**
 * @brief   Set task ready state true
 * @param   void
 * @return  void
 */
void DProductionTest::setReadyState(void)
{
    DLock is_on(&myMutex);
    myReadyState = true;
}

/**
 * @brief   Query task ready state
 * @param   void
 * @return  true is task is up and running, else false
 */
bool DProductionTest::isRunning(void)
{
    DLock is_on(&myMutex);
    return myReadyState;
}

/**
 * @brief   initialise for production testing
 * @note    On entering production test mode, the UUT application automatically initialises the following GPIO to the
 *          levels shown:
 *
 *               > INT_PS_SUPP_ON_NOFF_PG6 = logic 0  (to disable internal sensor supply)
 *               > USB_PEN_PC9 = 0  (to disable USB LDO IC19)
 *
 *          The following pins will have booted in analogue mode and should be kept in analogue mode until needed:
 *               > USB_UC_DAT_N    (pin PA11)
 *               > USB_UC_DAT_P    (pin PA12)
 *               > USB_ENUM_PA8
 *
 * @param   void
 * @return  void
 */
void DProductionTest::initialise(void)
{
}

/**
 * @brief   Perform internal pressure sensor test
 * @param   subTestIndex values are interpreted as follows:
 *
 *          10 = Write logic 0 to INT_PS_SUPP_ON_NOFF_PG6
 *          11 = Write logic 1 to INT_PS_SUPP_ON_NOFF_PG6
 *
 *          20 = Write logic 0 to INT_PS_I2C3_SCL_PG7
 *          21 = Write logic 1 to INT_PS_I2C3_SCL_PG7
 *
 *          30 = Write logic 0 to INT_PS_I2C3_SDA_PG8
 *          31 = Write logic 1 to INT_PS_I2C3_SDA_PG8

 * @return  void
 */
void DProductionTest::pressureSensorTest(int32_t subTestIndex)
{
    switch (subTestIndex)
    {
        case 10:
            //write code here to write logic 0 to INT_PS_SUPP_ON_NOFF_PG6
            break;

        case 11:
            //write code here to write logic 1 to INT_PS_SUPP_ON_NOFF_PG6
            break;

        case 20:
            //write code here to write logic 0 to INT_PS_I2C3_SCL_PG7
            break;

        case 21:
            //write code here to write logic 1 to INT_PS_I2C3_SCL_PG7
            break;

        case 30:
            //write code here to write logic 0 to INT_PS_I2C3_SDA_PG8
            break;

        case 31:
            //write code here to write logic 1 to INT_PS_I2C3_SDA_PG8
            break;

        default:
            //ignore
            break;
    }
}

/**
 * @brief   Post event flags
 * @param   flags is one or more event flags
 * @return  void
 */
void DProductionTest::postEvent(OS_FLAGS flags)
{
    OS_ERR os_error = OS_ERR_NONE;

    //signal event to task
    OSFlagPost(&myEventFlags, flags, OS_OPT_POST_FLAG_SET, &os_error);

    if (os_error != static_cast<OS_ERR>(OS_ERR_NONE))
    {
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
        error_code_t errorCode;
        errorCode.bit.osError = SET;
        PV624->handleError(errorCode, os_error);
    }
}

/**
 * @brief   Perform EEPROM self test
 * @param   void
 * @return  void
 */
void DProductionTest::eepromSelfTest(void)
{
    DLock is_on(&myMutex);
    eepromSelfTestStatus = 0;  //test status value: 0 = in progress (or not started)
    postEvent(EV_FLAG_SELF_TEST_EEPROM);
}

/**
 * @brief   Perform EEPROM self test
 * @param   void
 * @return  void
 */
void DProductionTest::performEepromSelfTest(void)
{
    //perform the test
    PV624->performEEPROMTest();

    //save result of test (with locked resource) - read here so multiple reads will return the same result
    DLock is_on(&myMutex);
    eepromSelfTestStatus = PV624->queryEEPROMTest();
}

/**
 * @brief   Query EEPROM self test
 * @param   void
 * @return  test status value: 0 = in progress (or not started); 1 = passed; -1 = failed
 */
int32_t DProductionTest::queryEepromSelfTest(void)
{
    DLock is_on(&myMutex);
    //return the last test result
    return eepromSelfTestStatus;
}

/**
 * @brief   Get barometer Id
 * @note    The id is read as a hexadecimal byte e.g. B3
 * @param   void
 * @return  deviceId as decimal integer value
 */
int32_t DProductionTest::getBarometerDeviceId(void)
{
    int32_t deviceId = 0;

    //write code here to fetch the barometer id

    return deviceId;
}

/**
 * @brief   Software shutdown
 * @note    Sets SOFT_ON_NOFF high/low by setting pin SOFT_ON_NOFF_PC2 high/low and latching it through IC1 by
 *          toggling pin SOFT_LATCH_CTRL_PC3
 * @param   subTestIndex value meaning: 0 = write logic 0 to SOFT_ON_NOFF; 1 = Write logic 1 to SOFT_ON_NOFF
 * @return  void
 */
void DProductionTest::softwareShutdown(int32_t subTestIndex)
{
    switch (subTestIndex)
    {
        case 0:
            //write code here to write logic 0 to SOFT_ON_NOFF
            break;

        case 1:
            //write code here to write logic 1 to SOFT_ON_NOFF
            break;

        default:
            //ignore
            break;
    }
}

/**
 * @brief   Get power button status
 * @param   void
 * @return  status value meaning: 0 = button pressed (PBUTT_ON_NOFF_PA0 = 1); 1 = button released (PBUTT_ON_NOFF_PA0 = 0)

 */
int32_t DProductionTest::powerButtonMonitor(void)
{
    int32_t status = 0;

    //write code here to read pin status

    return status;
}

/**
 * @brief   Request flash self test
 * @param   void
 * @return  void
 */
void DProductionTest::spiFlashSelfTest(void)
{
    DLock is_on(&myMutex);
    spiFlashSelfTestStatus = 0;  //test status value: 0 = in progress (or not started)
    postEvent(EV_FLAG_TASK_SELF_TEST_FLASH);
}

/**
 * @brief   Perform flash self test
 * @param   void
 * @return  void
 */
void DProductionTest::performSpiFlashSelfTest(void)
{
    spiFlashSelfTestStatus = 0;

    tOSPINORStatus status1, status2;
    __IO uint8_t step = 0u;

    // Verify NOR flash device IDs
    uint8_t deviceIdExpected[3] = {0xc2, 0x20, 0x19};
    uint8_t deviceIdActual[3];
    OSPI_NOR_ReadManufDeviceID(&deviceIdActual[0], &deviceIdActual[1], &deviceIdActual[2]);
    if (memcmp(deviceIdActual, deviceIdExpected, 3u) == 0)
    {
        // Attempt chip erase
        if (OSPI_NOR_ChipErase() == (tOSPINORStatus)OSPI_NOR_SUCCESS)
        {
            HAL_Delay(300u);

            // Insufficient test time available to test entire device, so
            // attempt write verification to 8 different blocks in approx. 5s
            for (uint32_t block = 0u; block < 512u; block += 64u)
            {
                uint32_t address = block * 0x10000u;

                status1 = OSPI_NOR_EraseWriteQPIExample(address, &step);
                HAL_Delay(300u);
                status2 = OSPI_NOR_EraseWriteQPIExample(address, &step);

                if ((status1 == (tOSPINORStatus)OSPI_NOR_FAIL) || (status2 == (tOSPINORStatus)OSPI_NOR_FAIL))
                {
                    spiFlashSelfTestStatus = -1;
                    break;
                }
            }
        }
        else
        {
            spiFlashSelfTestStatus = -1;
        }
    }
    else
    {
        spiFlashSelfTestStatus = -1;
    }

    if (spiFlashSelfTestStatus != -1)
    {
        spiFlashSelfTestStatus = 1;
    }

    //save result of test (with locked resource) - read here so multiple reads will return the same result
    DLock is_on(&myMutex);
}

/**
 * @brief   Query SPI flash memory self test
 * @param   void
 * @return  test status value: 0 = in progress (or not started); 1 = passed; -1 = failed
 */
int32_t DProductionTest::querySpiFlashSelfTest(void)
{
    DLock is_on(&myMutex);
    return spiFlashSelfTestStatus;
}



/**
 * @brief   Enable/disable USB LDO IC19
 * @param   subTestIndex values are interpreted as follows:
 *
 *          0 = Disable LDO (Write logic 0 to USB_PEN_PC9)
 *          1 = Enable LDO (Write logic 1 to USB_PEN_PC9)
 *
 * @return  void
 */
void DProductionTest::usb_LDO_IC19_Enable(int32_t subTestIndex)
{
    switch (subTestIndex)
    {
        case 0:
            //write code here to disable LDO (Write logic 0 to USB_PEN_PC9)
            break;

        case 1:
            //write code here to enable LDO (Write logic 1 to USB_PEN_PC9)
            break;

        default:
            //ignore
            break;
    }
}

/**
 * @brief   Configure USB
 * @note    Set up as follows:
 *          USB_DM_PA11 set to alternate function OTG_FS_DM
 *          USB_DP_PA12 set to alternate function OTG_FS_DP
 *          VBUS_DET_PA9 set to additional function ‘FS_OTG_VBUS’
 *          USB_ENUM_PA8 set to 1  (to enable immediate response from USB isolator when the Host applies VBUS)
 *
 * @param   void
 * @return  void
 */
void DProductionTest::configureUsb(void)
{
    //write code here
}

/**
 * @brief   Request USB self test
 * @param   void
 * @return  void
 */
void DProductionTest::usbSelfTest(void)
{
    DLock is_on(&myMutex);
    usbSelfTestStatus = 0;  //test status value: 0 = in progress (or not started)
    postEvent(EV_FLAG_TASK_SELF_TEST_USB);
}

/**
 * @brief   Perform EEPROM self test
 * @param   void
 * @return  void
 */
void DProductionTest::performUsbSelfTest(void)
{
    //write code to perform the test

    //save result of test (with locked resource) - read here so multiple reads will return the same result
    DLock is_on(&myMutex);
    usbSelfTestStatus = 0; //get result outcome here
}

/**
 * @brief   Query USB self test
 * @param   void
 * @return  test status value: 0 = in progress (or not started); 1 = passed; -1 = failed
 */
int32_t DProductionTest::queryUsbSelfTest(void)
{
    DLock is_on(&myMutex);
    return usbSelfTestStatus;
}

/**
 * @brief   Get Bluetooth device Id
 * @note    The id is the read as a hexadecimal byte e.g. 52832
 * @param   void
 * @return  deviceId as decimal integer value
 */
int32_t DProductionTest::getBluetoothDeviceId(void)
{
    int32_t deviceId = 0;

    //write code here to fetch the bluetooth id

    return deviceId;
}

/**
 * @brief   Bluetooth Reset (enable/disable)
 * @param   subTestIndex values are interpreted as follows:
 *
 *          0 = Write logic 0 to BT_ENABLE_PB9
 *          1 = Write logic 1 to BT_ENABLE_PB9
 *
 * @return  void
 */
void DProductionTest::bluetoothReset(int32_t subTestIndex)
{
    //write code here
}

/**
 * @brief   Set Led
 * @param   subTestIndex values are interpreted as follows:
 *
 *          0 = Write logic 0 to DEBUG_LED_PH1
 *          1 = Write logic 1 to DEBUG_LED_PH1
 *
 * @return  void
 */
void DProductionTest::setLed(int32_t ledIndex)
{
    //HAL_GPIO_WritePin(DEBUG_LED_PH1_GPIO_Port, DEBUG_LED_PH1_Pin, subTestIndex == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
}


/**
 * @brief   ReSet  LED 
 * @param   subTestIndex values are interpreted as follows:
 *
 *          0 = Write logic 0 to DEBUG_LED_PH1
 *          1 = Write logic 1 to DEBUG_LED_PH1
 *
 * @return  void
 */
void DProductionTest::resetLed(int32_t ledIndex)
{
    //HAL_GPIO_WritePin(DEBUG_LED_PH1_GPIO_Port, DEBUG_LED_PH1_Pin, subTestIndex == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
}


/**
 * @brief   Get keys
 * @param   void
 * @return  keys as decimal keymask value
 */
int32_t DProductionTest::getKeys(void)
{
    int32_t retVal = -1;

    retVal = PV624->keyHandler->getKeys();

    return retVal;
}

/**
 * @brief   Set keys
 * @param   keys as decimal integer value
 * @return  void
 */
void DProductionTest::setKeys(int32_t keys, int32_t duration)
{
    PV624->keyHandler->setKeys(keys, duration);
}

/**
 * @brief   Set Display Test Message
 * @param
 * @return  void
 */
void DProductionTest::displayTestMessage(char *str)
{
    
}



