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
    norFlashSelfTestStatus = 0;     //test status values: 0 = in progress (or not started); 1 = passed; -1 = failed
    usbSelfTestStatus = 0;          //test status values: 0 = in progress (or not started); 1 = passed; -1 = failed

    myReadyState = false;           //task is not yet up and running

    myWaitFlags = EV_FLAG_SELF_TEST_EEPROM | EV_FLAG_TASK_SELF_TEST_FLASH | EV_FLAG_TASK_SELF_TEST_USB |EV_FLAG_TASK_INVALIDATE_CAL_DATA;
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
    memset((void*)&myEventFlags, 0, sizeof(OS_FLAG_GRP));
    OSFlagCreate(&myEventFlags, NULL, (OS_FLAGS)0, &os_error);

    if (os_error == static_cast<OS_ERR>(OS_ERR_NONE))
    {
        //create mutex for resource locking
        memset((void*)&myMutex, 0, sizeof(OS_MUTEX));
        OSMutexCreate(&myMutex, (CPU_CHAR*)NULL, &os_error);
    }

    if (os_error == static_cast<OS_ERR>(OS_ERR_NONE))
    {
        //create task for production test
        memset((void*)&myTaskTCB, 0, sizeof(OS_TCB));
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

     
        PV624->handleError(E_ERROR_OS, 
                           eSetError,
                           (uint32_t)os_error,
                           (uint16_t)31);
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

           //check flags to determine what to execute
        if ((actualEvents & EV_FLAG_TASK_INVALIDATE_CAL_DATA) == EV_FLAG_TASK_INVALIDATE_CAL_DATA)
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
        
        PV624->handleError(E_ERROR_OS, 
                           eSetError,
                           (uint32_t)os_error,
                           (uint16_t)32);
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
    int32_t deviceId = (int32_t)-1;
    uint32_t barometerId;
    bool retStatus = false;
    deviceId = (int32_t)4001;
    //write code here to fetch the barometer id
    retStatus = PV624->instrument->getBarometerIdentity((uint32_t*)&barometerId);
    if(true == retStatus)
    {
      deviceId = (int32_t)barometerId;
    }
    else
    {
      deviceId = (int32_t)-1;
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
 * @brief   Get Bluetooth device Id
 * @note    The id is the read as a hexadecimal byte e.g. 52832
 * @param   void
 * @return  deviceId as decimal integer value
 */
int32_t DProductionTest::getBluetoothDeviceId(void)
{
    int32_t deviceId = 0;
    deviceId = (int32_t)4002;
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
 * @brief   To glow Led
 * @param   led Number ( 1 to 9):
 * @return  void
 */
void DProductionTest::switchOnLed(int32_t ledIndex)
{
    ledsTest((eLED_Num_t)ledIndex,LED_ON);
}


/**
 * @brief   To glow off the Led
 * @param   led Number ( 1 to 9):
 * @return  void
 */
void DProductionTest::switchOffLed(int32_t ledIndex)
{
    ledsTest((eLED_Num_t)ledIndex,LED_OFF);

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

/**
 * @brief   Set Display Test Message
 * @param
 * @return  void
 */
void DProductionTest::displayTestMessage(char *str)
{
    
}

int32_t DProductionTest::getTemperatureSensorDeviceId(void)
{
  int32_t deviceId = (int32_t)-1;
  uint16_t sensorID = (uint16_t)0;
  sensorID = PV624->temperatureSensor->GetTemperatureSensorDeviceID();
  deviceId = (int32_t)sensorID;
  return deviceId;
}
    
int32_t DProductionTest::getStepperMotorDeviceId(void)
{
  int32_t deviceId = (int32_t)-1;
  //deviceId = PV624->stepperMotor->fnGetAbsolutePosition();
  return deviceId;
}
void DProductionTest::setStepperMotorParam(int32_t param)
{
 // PV624->stepperMotor->writeAbsolutePosition(param);
}
int32_t DProductionTest::get24VoltSupplyStatus(void)
{
  int32_t deviceId = (int32_t)-1;
  deviceId = (int32_t)4006;
  return deviceId;
}
int32_t DProductionTest::get6VoltSupplyStatus(void)
{
  int32_t deviceId = (int32_t)-1;
  deviceId = (int32_t)4007;
  return deviceId;
}
int32_t DProductionTest::get5VoltSupplyStatus(void)
{
  int32_t deviceId = (int32_t)-1;
  deviceId = (int32_t)4008;
  return deviceId;
}
int32_t DProductionTest::get5VoltPm620SupplyStatus(void)
{
  int32_t deviceId = (int32_t)-1;
  deviceId = (int32_t)4009;
  return deviceId;
}
    
int32_t DProductionTest::testValve1(int32_t subTestIndex)
{
  int32_t retVal = (int32_t)-1;
  if((eValveFunctions_t) subTestIndex <= (eValveFunctions_t)E_VALVE_FUNCTION_CURRENT_REG2)
  {
    PV624->valve1->valveTest((eValveFunctions_t) subTestIndex );
    retVal = (int32_t)0;
  }
  
  
  return retVal;
}
int32_t DProductionTest::testValve2(int32_t subTestIndex)
{
  int32_t retVal = (int32_t)-1;
  if((eValveFunctions_t) subTestIndex <= (eValveFunctions_t)E_VALVE_FUNCTION_CURRENT_REG2)
  {
    PV624->valve2->valveTest((eValveFunctions_t) subTestIndex );
    retVal = (int32_t)0;
  }
  return retVal;
}
int32_t DProductionTest::testValve3(int32_t subTestIndex)
{
  int32_t retVal = (int32_t)-1;
  if((eValveFunctions_t) subTestIndex <= (eValveFunctions_t)E_VALVE_FUNCTION_CURRENT_REG2)
  {
    PV624->valve3->valveTest((eValveFunctions_t) subTestIndex );
    retVal = (int32_t)0;
  }
  return retVal;
}

void DProductionTest::displayBatteryStatus(void)
{
  
}

int32_t DProductionTest::getPM620DeviceId(void)
{
  int32_t deviceId = (int32_t)-1;
  deviceId = (int32_t)4010;
  return deviceId;
}

int32_t DProductionTest::getBatteryId(void)
{
  uint32_t deviceId = (uint32_t)0;
  bool retStatus = false;
  retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_SERIAL_NUMBER, 
                                            (uint32_t*)&deviceId); 
  
  if(false == retStatus)
  {
    deviceId = (uint32_t)0;
  }
  return (int32_t)deviceId;
}

int32_t DProductionTest::getBatteryChargerId(void)
{
  int32_t deviceId = (int32_t)-1;
  deviceId = (int32_t)4012;
  return deviceId;
}

float32_t DProductionTest::get24VoltSupplyValue(void)
{
  float32_t supplyValue = 0.0f;
   bool retStatus = false;
  retStatus = PV624->powerManager->getValue(EVAL_INDEX_BATTERY_24VOLT_VALUE, 
                                            (float32_t*)&supplyValue);  
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
                                            (float32_t*)&supplyValue); 
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
                                            (float32_t*)&supplyValue);  
  if(false == retStatus)
  {
    supplyValue = 0.0f;
  }
  return supplyValue;
}


void DProductionTest ::ledsTest(eLED_Num_t ledNumber, eLED_OnOffState_t onOffState)
{
  GPIO_PinState  pinState = GPIO_PIN_RESET;
  
  if( (eLED_OnOffState_t) LED_OFF == onOffState )
  {
    pinState = GPIO_PIN_RESET;
  }
  else if( (eLED_OnOffState_t) LED_ON == onOffState )
  {
    pinState = GPIO_PIN_SET;
  }
  else
  {
    /*do Nothing */
  }
  switch(ledNumber)
    {
  case LED_1:
        HAL_GPIO_WritePin(BAT_LEVEL1_PF2_GPIO_Port, BAT_LEVEL1_PF2_Pin, pinState);
      break;
   case LED_2:
        HAL_GPIO_WritePin(BAT_LEVEL2_PF4_GPIO_Port, BAT_LEVEL2_PF4_Pin, pinState);
      break;
   case LED_3:
       HAL_GPIO_WritePin(BAT_LEVEL3_PF5_GPIO_Port, BAT_LEVEL3_PF5_Pin, pinState);
      break;
   case LED_4:
        HAL_GPIO_WritePin(BAT_LEVEL4_PD10_GPIO_Port, BAT_LEVEL4_PD10_Pin, pinState);
      break;
   case LED_5:
      HAL_GPIO_WritePin(BAT_LEVEL5_PD8_GPIO_Port, BAT_LEVEL5_PD8_Pin, pinState);
      break;
   case LED_6:
      //HAL_GPIO_WritePin(STATUS_RED_PE2_GPIO_Port, STATUS_RED_PE2_Pin, pinState);
      break;
   case LED_7:
      //HAL_GPIO_WritePin(STATUS_GREEN_PF10_GPIO_Port, STATUS_GREEN_PF10_Pin, pinState);
      break;
   case LED_8:
      //HAL_GPIO_WritePin(STATUS_BLUE_PE4_GPIO_Port, STATUS_BLUE_PE4_Pin, pinState);
      break;
   case LED_9:   
      HAL_GPIO_WritePin(BT_INDICATION_PE5_GPIO_Port, BT_INDICATION_PE5_Pin, pinState);  
      break;
   default:
      break;
    }
}

int32_t DProductionTest::getBarometerReading(float32_t *measValue)
{
  int32_t retValue = (int32_t)-1;
  bool retStatus = false;
  retStatus = PV624->instrument->getReading(E_VAL_INDEX_BAROMETER_VALUE,
                                           measValue);
  if(false == retStatus)
  {
    retValue = (int32_t)-1;
  }
  else
  {
    retValue = (int32_t)0;
  }
  return retValue;
}


int32_t DProductionTest::getPM620Reading(float32_t *measValue)
{
  int32_t retValue = (int32_t)-1;
  bool retStatus = false;
  retStatus = PV624->instrument->getReading(E_VAL_INDEX_VALUE,
                                            measValue);
  if(false == retStatus)
  {
    retValue = (int32_t)-1;
  }
  else
  {
    retValue = (int32_t)0;
  }
  return retValue;
}


int32_t DProductionTest::getIrSensorAdcCounts(void)
{
 
  uint32_t val = 0u;
  int32_t adcCounts =(int32_t) -1;
  bool retStatus = false;
  retStatus = PV624->powerManager->getValue(EVAL_INDEX_IR_SENSOR_ADC_COUNTS, 
                                            (uint32_t*)&val);  
  if(true == retStatus)
  {
    adcCounts = (int32_t)val;
  }
  else
  {
    adcCounts = (int32_t) -1;
  }
  return adcCounts;

}



int32_t DProductionTest::controlChargerEnablePin(int32_t param)
{
  int32_t retStatus =(int32_t) 0;
  
  if((param >= 0) && (param <= 1))
  {
    
    if((int32_t) 0 == param)
    {
      HAL_GPIO_WritePin(CHGEN_PG10_GPIO_Port, CHGEN_PG10_Pin, GPIO_PIN_RESET);
    }
    else if((int32_t) 1 == param)
    {
     HAL_GPIO_WritePin(CHGEN_PG10_GPIO_Port, CHGEN_PG10_Pin, GPIO_PIN_SET); 
    }
    else
    {
      retStatus = (int32_t) -1;
    }
    
  }
  else
  {
    retStatus = (int32_t) -1;
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
