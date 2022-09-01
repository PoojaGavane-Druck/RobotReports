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
#include "Utilities.h"
#include "cBL652.h"
MISRAC_DISABLE

#include <assert.h>
#include "ospi_nor_mx25l25645.h"
MISRAC_ENABLE

/* Error handler instance parameter starts from 4001 to 4100 */

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define FIT_SET_POINT_COUNT     44995u
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
DProductionTest *DProductionTest::myInstance = NULL;
int8_t secondMicroDk[7] = "0509";
/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DProductionTest class constructor
 * @param   void
 * @return  void
 */
DProductionTest::DProductionTest(void)
{

    myName = "fExtAndBaro";

    eepromSelfTestStatus = 0;       //test status values: 0 = in progress (or not started); 1 = passed; -1 = failed
    norFlashSelfTestStatus = 0;     //test status values: 0 = in progress (or not started); 1 = passed; -1 = failed
    usbSelfTestStatus = 0;          //test status values: 0 = in progress (or not started); 1 = passed; -1 = failed

    myReadyState = false;           //task is not yet up and running

    invalidateCalOperationStatus = 0;

    memset((void *)&myEventFlags, 0, sizeof(OS_FLAG_GRP));

    memset((void *)&myMutex, 0, sizeof(OS_MUTEX));

    myWaitFlags = EV_FLAG_SELF_TEST_EEPROM | EV_FLAG_TASK_SELF_TEST_FLASH | EV_FLAG_TASK_SELF_TEST_USB | EV_FLAG_TASK_INVALIDATE_CAL_DATA;
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
    memset((void *)&myEventFlags, 0, sizeof(OS_FLAG_GRP));
    RTOSFlagCreate(&myEventFlags, NULL, (OS_FLAGS)0, &os_error);

    if(os_error == static_cast<OS_ERR>(OS_ERR_NONE))
    {
        //create mutex for resource locking
        memset((void *)&myMutex, 0, sizeof(OS_MUTEX));
        RTOSMutexCreate(&myMutex, (CPU_CHAR *)NULL, &os_error);
    }

    if(os_error == static_cast<OS_ERR>(OS_ERR_NONE))
    {
        //create task for production test
        memset((void *)&myTaskTCB, 0, sizeof(OS_TCB));
        RTOSTaskCreate(&myTaskTCB,
                       (CPU_CHAR *)myName,          //no name given to task
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

    if(!ok)
    {

        PV624->handleError(E_ERROR_OS,
                           eSetError,
                           (uint32_t)os_error,
                           4001u);
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
    while(DEF_TRUE)
    {
        PV624->keepAlive(eProductionTestTask);

        actualEvents = RTOSFlagPend(&thisTask->myEventFlags,
                                    thisTask->myWaitFlags, (OS_TICK)0u,
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);

        //check flags to determine what to execute
        if((actualEvents & EV_FLAG_SELF_TEST_EEPROM) == EV_FLAG_SELF_TEST_EEPROM)
        {
            thisTask->performEepromSelfTest();
        }

        if((actualEvents & EV_FLAG_TASK_SELF_TEST_FLASH) == EV_FLAG_TASK_SELF_TEST_FLASH)
        {
            thisTask->performSpiFlashSelfTest();
        }

        //check flags to determine what to execute
        if((actualEvents & EV_FLAG_TASK_INVALIDATE_CAL_DATA) == EV_FLAG_TASK_INVALIDATE_CAL_DATA)
        {
            thisTask->performCalDataInvalidateOperation();
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
 * @brief   Post event flags
 * @param   flags is one or more event flags
 * @return  void
 */
void DProductionTest::postEvent(OS_FLAGS flags)
{
    OS_ERR os_error = OS_ERR_NONE;

    //signal event to task
    RTOSFlagPost(&myEventFlags, flags, OS_OPT_POST_FLAG_SET, &os_error);

    if(os_error != static_cast<OS_ERR>(OS_ERR_NONE))
    {
        PV624->handleError(E_ERROR_OS,
                           eSetError,
                           (uint32_t)os_error,
                           4002u);
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
    int32_t deviceId =  -1;
    uint32_t barometerId;
    bool retStatus = false;

    //write code here to fetch the barometer id
    retStatus = PV624->instrument->getBarometerIdentity((uint32_t *)&barometerId);

    if(true == retStatus)
    {
        deviceId = (int32_t)barometerId;
    }

    else
    {
        deviceId =  -1;
    }

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
    switch(subTestIndex)
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
    norFlashSelfTestStatus = 0;  //test status value: 0 = in progress (or not started)
    postEvent(EV_FLAG_TASK_SELF_TEST_FLASH);
}

/**
 * @brief   Perform flash self test
 * @param   void
 * @return  void
 */
void DProductionTest::performSpiFlashSelfTest(void)
{
    norFlashSelfTestStatus = 0;

    tOSPINORStatus mx25Status = OSPI_NOR_SelfTest();

    //save result of test (with locked resource) - read here so multiple reads will return the same result
    DLock is_on(&myMutex);
    norFlashSelfTestStatus = (mx25Status == (int)OSPI_NOR_SUCCESS) ? 1 : -1;
}

/**
 * @brief   Query SPI flash memory self test
 * @param   void
 * @return  test status value: 0 = in progress (or not started); 1 = passed; -1 = failed
 */
int32_t DProductionTest::querySpiFlashSelfTest(void)
{
    DLock is_on(&myMutex);
    return norFlashSelfTestStatus;
}

/**
 * @brief   Query secondary micro DK number
 * @param   void
 * @return  test status value: 0 = in progress (or not started); 1 = passed; -1 = failed
 */
int32_t DProductionTest::querySecondMicroDKnumber(void)
{
    int32_t deviceId = -1;
    bool successFlag = false;
    char dkStr[7u];
    successFlag = PV624->getDK(E_ITEM_PV624_2ND_MICRO, E_COMPONENENT_APPLICATION, dkStr);

    if(successFlag)
    {
        deviceId = memcmp(dkStr, secondMicroDk, (size_t)6);
    }

    if(0 == deviceId)
    {
        deviceId = 1;
    }

    return deviceId;
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
    bool successFlag = false;
    //write code here to fetch the bluetooth id
    successFlag = PV624->checkBluetoothCommInterface();

    if(successFlag)
    {
        deviceId = 1;
    }

    return deviceId;
}

/**
 * @brief   To glow Led
 * @param   led Number ( 1 to 9):
 * @return  void
 */
void DProductionTest::switchOnLed(int32_t ledIndex)
{
    ledsTest((eLED_Num_t)ledIndex, LED_ON);
}


/**
 * @brief   To glow off the Led
 * @param   led Number ( 1 to 9):
 * @return  void
 */
void DProductionTest::switchOffLed(int32_t ledIndex)
{
    ledsTest((eLED_Num_t)ledIndex, LED_OFF);

}
/**
 * @brief   Get keys
 * @param   void
 * @return  keys as decimal keymask value
 */
uint32_t DProductionTest::getKeys(void)
{
    uint32_t retVal = 0u;

    retVal = PV624->keyHandler->getKeys();

    return retVal;
}

/**
 * @brief   Set keys
 * @param   keys as decimal integer value
* @param    duration --- key press  time
 * @return  void
 */
void DProductionTest::setKeys(uint32_t keys, uint32_t duration)
{
    PV624->keyHandler->setKeys(keys, duration);
}



int32_t DProductionTest::getTemperatureSensorDeviceId(void)
{
    int32_t deviceId =  -1;
    uint16_t sensorID = 0u;
    //sensorID = PV624->temperatureSensor->GetTemperatureSensorDeviceID();
    deviceId = (int32_t)sensorID;
    return deviceId;
}


void DProductionTest::setStepperMotorParam(int32_t param)
{
// PV624->stepperMotor->writeAbsolutePosition(param);
}
int32_t DProductionTest::get24VoltSupplyStatus(void)
{
    uint32_t deviceId = 0u;
    bool retStatus = false;
    retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_24VOLT_STATUS,
                (uint32_t *)&deviceId);

    if(false == retStatus)
    {
        deviceId = 0u;
    }

    return (int32_t)deviceId;
}
int32_t DProductionTest::get6VoltSupplyStatus(void)
{
    uint32_t deviceId = 0u;
    bool retStatus = false;
    retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_6VOLT_STATUS,
                (uint32_t *)&deviceId);

    if(false == retStatus)
    {
        deviceId = 0u;
    }

    return (int32_t)deviceId;
}
int32_t DProductionTest::get5VoltSupplyStatus(void)
{
    uint32_t deviceId = 0u;
    bool retStatus = false;
    retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_5VOLT_STATUS,
                (uint32_t *)&deviceId);

    if(false == retStatus)
    {
        deviceId = 0u;
    }

    return (int32_t)deviceId;
}
int32_t DProductionTest::get5VoltPm620SupplyStatus(void)
{
    uint32_t deviceId = 0u;
    bool retStatus = false;
    retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_5VOLT_STATUS,
                (uint32_t *)&deviceId);

    if(false == retStatus)
    {
        deviceId = 0u;
    }

    return (int32_t)deviceId;
}

int32_t DProductionTest::testValve1(int32_t subTestIndex)
{
    int32_t retVal =  -1;

    if((eValveState_t) subTestIndex <= (eValveState_t)VALVE_STATE_OFF)
    {
        PV624->valve1->valveTest((eValveState_t) subTestIndex);
        retVal = 0;
    }


    return retVal;
}
int32_t DProductionTest::testValve2(int32_t subTestIndex)
{
    int32_t retVal =  -1;

    if((eValveState_t) subTestIndex <= (eValveState_t)VALVE_STATE_OFF)
    {
        PV624->valve2->valveTest((eValveState_t) subTestIndex);
        retVal = 0;
    }

    return retVal;
}
int32_t DProductionTest::testValve3(int32_t subTestIndex)
{
    int32_t retVal =  -1;

    if((eValveState_t) subTestIndex <= (eValveState_t)VALVE_STATE_OFF)
    {
        PV624->valve3->valveTest((eValveState_t) subTestIndex);
        retVal = 0;
    }

    return retVal;
}

void DProductionTest::displayBatteryStatus(float *pPercentCapacity,
        uint32_t *pChargingStatus)
{
    PV624->getBatLevelAndChargingStatus(pPercentCapacity, pChargingStatus);
}

int32_t DProductionTest::getPM620DeviceId(void)
{
    int32_t deviceId = -1;
    uint32_t sn = 0u;
    sn = PV624->getSerialNumber(1u);  //1: to read PM620 Sensor ID
    deviceId = (int32_t)sn;
    return deviceId;
}

int32_t DProductionTest::getBatteryId(void)
{
    uint32_t deviceId = 0u;
    bool retStatus = false;
    retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_SERIAL_NUMBER,
                (uint32_t *)&deviceId);

    if(false == retStatus)
    {
        deviceId = 0u;
    }

    return (int32_t)deviceId;
}

int32_t DProductionTest::getBatteryChargerId(void)
{
    int32_t deviceId =  -1;
    PV624->powerManager->battery->getValue(eCurrent, &deviceId);

    if(deviceId >= 1000)
    {
        deviceId = 1;
    }

    else
    {
        deviceId = 0;
    }

    return deviceId;
}

float32_t DProductionTest::get24VoltSupplyValue(void)
{
    float32_t supplyValue = 0.0f;
    bool retStatus = false;
    retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_24VOLT_VALUE,
                (float32_t *)&supplyValue);

    if(false == retStatus)
    {
        supplyValue = 0.0f;
    }

    return supplyValue;
}
float32_t DProductionTest::get6VoltSupplyValue(void)
{
    float32_t supplyValue = 0.0f;
    bool retStatus = false;
    retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_6VOLT_VALUE,
                (float32_t *)&supplyValue);

    if(false == retStatus)
    {
        supplyValue = 0.0f;
    }

    return supplyValue;
}

float32_t DProductionTest::get5VoltSupplyValue(void)
{
    float32_t supplyValue = 0.0f;
    bool retStatus = false;
    retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_5VOLT_VALUE,
                (float32_t *)&supplyValue);

    if(false == retStatus)
    {
        supplyValue = 0.0f;
    }

    return supplyValue;
}


void DProductionTest ::ledsTest(eLED_Num_t ledNumber, eLED_OnOffState_t onOffState)
{
    GPIO_PinState  pinState = GPIO_PIN_RESET;

    if((eLED_OnOffState_t) LED_OFF == onOffState)
    {
        pinState = GPIO_PIN_RESET;
    }

    else if((eLED_OnOffState_t) LED_ON == onOffState)
    {
        pinState = GPIO_PIN_SET;
    }

    else
    {
        /*do Nothing */
    }

    switch(ledNumber)
    {
    case LED_5:
        HAL_GPIO_WritePin(BAT_LEVEL1_PF2_GPIO_Port, BAT_LEVEL1_PF2_Pin, pinState);
        break;

    case LED_4:
        HAL_GPIO_WritePin(BAT_LEVEL2_PF4_GPIO_Port, BAT_LEVEL2_PF4_Pin, pinState);
        break;

    case LED_3:
        HAL_GPIO_WritePin(BAT_LEVEL3_PF5_GPIO_Port, BAT_LEVEL3_PF5_Pin, pinState);
        break;

    case LED_2:
        HAL_GPIO_WritePin(BAT_LEVEL4_PC9_GPIO_Port, BAT_LEVEL4_PC9_Pin, pinState);
        break;

    case LED_1:
        HAL_GPIO_WritePin(BAT_LEVEL5_PD8_GPIO_Port, BAT_LEVEL5_PD8_Pin, pinState);
        break;

    case LED_6:
        HAL_GPIO_WritePin(STATUS_GREEN_PF10_GPIO_Port, STATUS_GREEN_PF10_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(STATUS_RED_PE2_GPIO_Port, STATUS_RED_PE2_Pin, pinState);

        break;

    case LED_7:
        HAL_GPIO_WritePin(STATUS_RED_PE2_GPIO_Port, STATUS_RED_PE2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(STATUS_GREEN_PF10_GPIO_Port, STATUS_GREEN_PF10_Pin, pinState);
        break;

    case LED_8:
        HAL_GPIO_WritePin(STATUS_RED_PE2_GPIO_Port, STATUS_RED_PE2_Pin, pinState);
        HAL_GPIO_WritePin(STATUS_GREEN_PF10_GPIO_Port, STATUS_GREEN_PF10_Pin, pinState);
        break;

    case LED_9:
        HAL_GPIO_WritePin(BT_INDICATION_PE4_GPIO_Port, BT_INDICATION_PE4_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BT_INDICATION_PE5_GPIO_Port, BT_INDICATION_PE5_Pin, pinState);

        break;

    case LED_10:
        HAL_GPIO_WritePin(BT_INDICATION_PE5_GPIO_Port, BT_INDICATION_PE5_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BT_INDICATION_PE4_GPIO_Port, BT_INDICATION_PE4_Pin, pinState);
        break;

    case LED_11:
        HAL_GPIO_WritePin(BT_INDICATION_PE4_GPIO_Port, BT_INDICATION_PE4_Pin, pinState);
        HAL_GPIO_WritePin(BT_INDICATION_PE5_GPIO_Port, BT_INDICATION_PE5_Pin, pinState);
        break;

    default:
        break;
    }
}

int32_t DProductionTest::getBarometerReading(float32_t *measValue)
{
    int32_t retValue =  -1;
    bool retStatus = false;
    retStatus = PV624->instrument->getReading(E_VAL_INDEX_BAROMETER_VALUE,
                measValue);

    if(false == retStatus)
    {
        retValue =  -1;
    }

    else
    {
        retValue = 0;
    }

    return retValue;
}


int32_t DProductionTest::getPM620Reading(float32_t *measValue)
{
    int32_t retValue =  -1;
    bool retStatus = false;
    retStatus = PV624->instrument->getReading(E_VAL_INDEX_VALUE,
                measValue);

    if(false == retStatus)
    {
        retValue = -1;
    }

    else
    {
        retValue = 0;
    }

    return retValue;
}



int32_t DProductionTest::controlChargerEnablePin(int32_t param)
{
    int32_t retStatus =  0;

    if((param >= 0) && (param <= 1))
    {

        if(0 == param)
        {
            HAL_GPIO_WritePin(CHGEN_PG10_GPIO_Port, CHGEN_PG10_Pin, GPIO_PIN_RESET);
        }

        else if(1 == param)
        {
            HAL_GPIO_WritePin(CHGEN_PG10_GPIO_Port, CHGEN_PG10_Pin, GPIO_PIN_SET);
        }

        else
        {
            retStatus =  -1;
        }

    }

    else
    {
        retStatus =  -1;
    }

    return retStatus;
}

/**
 * @brief   Perform EEPROM self test
 * @param   void
 * @return  void
 */
void DProductionTest::invalidateCalibrationData(void)
{
    DLock is_on(&myMutex);
    invalidateCalOperationStatus = 0;  //test status value: 0 = in progress (or not started)
    postEvent(EV_FLAG_TASK_INVALIDATE_CAL_DATA);
}

/**
 * @brief   Perform EEPROM self test
 * @param   void
 * @return  void
 */
void DProductionTest::performCalDataInvalidateOperation(void)
{
    //perform the test
    PV624->invalidateCalibrationData();

    //save result of test (with locked resource) - read here so multiple reads will return the same result
    DLock is_on(&myMutex);
    invalidateCalOperationStatus = PV624->queryInvalidateCalOpeResult();
}

/**
 * @brief   Query EEPROM self test
 * @param   void
 * @return  test status value: 0 = in progress (or not started); 1 = passed; -1 = failed
 */
int32_t DProductionTest::queryInvalidateCalOpeResult(void)
{
    DLock is_on(&myMutex);
    //return the last test result
    return invalidateCalOperationStatus;
}

/**
 * @brief   moves the motor till forward end and then return to home position
 * @param   none
 * @retval  true = success, false = failed
 */
bool DProductionTest::moveMotorTillForwardEnd(void)
{
    bool successFlag = false;
    DLock is_on(&myMutex);
    successFlag = PV624->moveMotorTillForwardEnd();
    return successFlag;
}

/**
 * @brief   moves the motor till reverse end and then return to home position
 * @param   none
 * @retval  true = success, false = failed
 */
bool DProductionTest::moveMotorTillReverseEnd(void)
{
    bool successFlag = false;
    DLock is_on(&myMutex);
    successFlag = PV624->moveMotorTillReverseEnd();
    return successFlag;
}

/**
 * @brief   moves the motor till forward end and then return to home position
 * @param   none
 * @retval  true = success, false = failed
 */
bool DProductionTest::moveMotorTillForwardEndThenHome(void)
{
    bool successFlag = false;
    DLock is_on(&myMutex);
    successFlag = PV624->moveMotorTillForwardEndThenHome();
    return successFlag;
}

/**
 * @brief   moves the motor till reverse end and then return to home position
 * @param   none
 * @retval  true = success, false = failed
 */
bool DProductionTest::moveMotorTillReverseEndThenHome(void)
{
    bool successFlag = false;
    DLock is_on(&myMutex);
    successFlag = PV624->moveMotorTillReverseEndThenHome();
    return successFlag;
}

/**
 * @brief   moves the motor for requested number of steps
 * @param   stepCnt number of steps
 * @retval  true = success, false = failed
 */
bool DProductionTest::moveMotor(int32_t stepCnt)
{
    bool successFlag = false;
    int32_t readSteps = 0;
    eMotorError_t motorError = eMotorErrorNone;
    DLock is_on(&myMutex);
    motorError = PV624->stepperMotor->move(stepCnt, &readSteps);
    sleep(100u);

    if((eMotorError_t)eMotorErrorNone == motorError)
    {
        motorError = PV624->stepperMotor->move(0, &readSteps);

        if((eMotorError_t)eMotorErrorNone == motorError)
        {
            stepCount = readSteps;

            if(stepCount == stepCnt)
            {
                successFlag = true;
            }
        }

    }


    return successFlag;
}

/**
 * @brief   query number steps moved by motor
 * @param   pointer to variable to return number of steps moved by motor
 * @retval  true = success, false = failed
 */
bool DProductionTest::queryMotorStepCount(int32_t *stepCnt)
{
    bool successFlag = false;
    int32_t readSteps = 0;
    eMotorError_t motorError = eMotorErrorNone;

    if(NULL != stepCnt)
    {
        DLock is_on(&myMutex);
        motorError = PV624->stepperMotor->move(0, &readSteps);

        if((eMotorError_t)eMotorErrorNone == motorError)
        {
            stepCount = readSteps;
            *stepCnt = stepCount;
            successFlag = true;
        }
    }

    return successFlag;
}

/**
 * @brief   get motor connection status
 * @param   none
 * @retval  true = success, false = failed
 */
bool DProductionTest::getMotorStatus(void)
{
    bool successFlag = false;
    eMotorError_t motorError = eMotorErrorNone;
    int32_t readSteps = 0;
    DLock is_on(&myMutex);

    successFlag = PV624->moveMotorTillReverseEndThenHome();

    if(successFlag)
    {
        motorError = PV624->stepperMotor->move(5, &readSteps);
        sleep(100u);

        if((eMotorError_t)eMotorErrorNone == motorError)
        {
            motorError = PV624->stepperMotor->move(0, &readSteps);

            if((eMotorError_t)eMotorErrorNone == motorError)
            {
                stepCount = readSteps;

                if(stepCount == 5)
                {
                    successFlag = true;
                }
            }

        }
    }

    return successFlag;
}

/**
 * @brief   this function writes set point count 44995
 * @param   void
 * @return  keys as decimal keymask value
 */
uint32_t DProductionTest::fitForSetPointCount(void)
{
    uint32_t retVal = 0u;

    retVal = PV624->updateSetPointCount(FIT_SET_POINT_COUNT);

    return retVal;
}