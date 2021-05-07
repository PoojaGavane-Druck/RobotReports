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
* @file     DCommsStateProdTest.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The communications production test state class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateProdTest.h"
#include "DParseSlave.h"
#include "dpv624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define SLAVE_PROD_TEST_COMMANDS_ARRAY_SIZE  16  //this is the maximum no of commands supported in DUCI prod test slave mode (can be increased if more needed)
#define PRODUCTION_TEST_BUILD

/* Variables --------------------------------------------------------------------------------------------------------*/
sDuciCommand_t duciSlaveProdTestCommands[SLAVE_PROD_TEST_COMMANDS_ARRAY_SIZE];

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DCommsStateProdTest class constructor
 * @param   commsMedium reference to comms medium
 * @param   task is pointer to own task
 * @retval  void
 */
DCommsStateProdTest::DCommsStateProdTest(DDeviceSerial *commsMedium, DTask *task)
    : DCommsStateDuci(commsMedium, task)
{
    OS_ERR os_error = OS_ERR_NONE;
    myParser = new DParseSlave((void *)this, &duciSlaveProdTestCommands[0], (size_t)SLAVE_PROD_TEST_COMMANDS_ARRAY_SIZE, &os_error);
    createDuciCommands();
    commandTimeoutPeriod = 200u; //time in (ms) to wait for a response to a command (0 means wait forever)
}

/**
 * @brief   Create DUCI command set
 * @param   void
 * @return  void
 */
void DCommsStateProdTest::createDuciCommands(void)
{
    //create common commands - that apply to all states
    DCommsStateDuci::createCommands();

#ifdef PRODUCTION_TEST_BUILD
    //Create DUCI command set
    myParser->addCommand("KP", "=i,[i]",       "?",         fnSetKP,                fnGetKP,                0xFFFFu);
    myParser->addCommand("SD", "=d",           "?",         fnSetSD,                fnGetSD,                0xFFFFu);
    myParser->addCommand("ST", "=t",           "?",         fnSetST,                fnGetST,                0xFFFFu);
    myParser->addCommand("TM", "[=][$]",       "",          fnSetTM,                NULL,                   0xFFFFu);
    myParser->addCommand("TP", "i,[=][i]",     "[i]?",      fnSetTP,                fnGetTP,                0xFFFFu);
    
#endif
}

/**********************************************************************************************************************
 * DISABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum.
 * DISABLE MISRA C 2004 CHECK for Rule 10.1 as (enum) conversion from unsigned char to int is illegal
 **********************************************************************************************************************/
_Pragma ("diag_suppress=Pm017,Pm128")

/**
 * @brief   Run function for this class
 * @param   void
 * @retval  void
 */
eStateDuci_t DCommsStateProdTest::run(void)
{
    char *buffer;

    sInstrumentMode_t mask;
    mask.value = 0u;
    mask.test = 1u;

    //Entry
    //DPI610E->userInterface->setMode(mask);

#ifdef PRODUCTION_TEST_BUILD

    nextState = E_STATE_DUCI_PROD_TEST;

    myProductionTest = DProductionTest::getInstance();

    if (myProductionTest != NULL)
    {
        myProductionTest->start();

        errorStatusRegister.value = 0u;   //clear DUCI error status register
        externalDevice.status.all = 0u;

        sDuciError_t duciError;         //local variable for error status
        duciError.value = 0u;

        //DO
        clearRxBuffer();
        while (nextState == E_STATE_DUCI_PROD_TEST)
        {
            if (myTask != NULL)
            {
#ifdef TASK_MONITOR_IMPLEMENTED              
                myTask->keepAlive();
#endif
            }

            if (receiveString(&buffer))
            {
                duciError = myParser->parse(buffer);
                clearRxBuffer();

                errorStatusRegister.value |= duciError.value;
            }
        }
    }
#endif

    //Exit
    //ToDO: need to update mask
    #ifdef USER_INTERFACE_ENABLED
    PV624->userInterface->clearMode(mask);
#endif
    return E_STATE_DUCI_PROD_TEST;
}

/**********************************************************************************************************************
 * RE-ENABLE MISRA C 2004 CHECK for Rule 5.2 as symbol hides enum (OS_ERR enum which violates the rule).
 * RE-ENABLE MISRA C 2004 CHECK for Rule 10.1 as enum is unsigned char
 **********************************************************************************************************************/
_Pragma ("diag_default=Pm017,Pm128")

#ifdef PRODUCTION_TEST_BUILD

/**
 * @brief   DUCI handler for KM Command – Get front panel keypad operating mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnGetKM(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        bool running = false;

        //only return production test state once task is up and running. Until then keep reporting 'local mode'
        if (myProductionTest != NULL)
        {
            if (myProductionTest->isRunning() == true)
            {
                running = true;
            }
        }

        if (running == true)
        {
            sendString("!KM=S");
        }
        else
        {
            sendString("!KM=L");
        }
    }

    return duciError;
}

/**
 * @brief   DUCI handler for KM Command – Set front panel keypad operating mode
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetKM(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted KM message in this state is a command type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        switch(parameterArray[1].charArray[0])
        {
#if 0
            case 'L':    //enter local mode
                //can't go back to local - but could force reset if required (not implemented that way here)
                break;
#endif
            case 'S':    //enter production test mode
                //ignore, as already in this mode
                break;

            default:
                duciError.invalid_args = 1u;
                break;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SD Command – Get date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnGetSD(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateProdTest *myInstance = (DCommsStateProdTest*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetSD(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for SD Command – Set date
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetSD(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateProdTest *myInstance = (DCommsStateProdTest*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetSD(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for ST Command – Get time
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnGetST(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateProdTest *myInstance = (DCommsStateProdTest*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetST(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for ST Command – Set time
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetST(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateProdTest *myInstance = (DCommsStateProdTest*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetST(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for KP Command - Key Press Emulation
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnGetKP(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateProdTest *myInstance = (DCommsStateProdTest*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetKP(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for TP Command - Get self-test status
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnGetTP(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateProdTest *myInstance = (DCommsStateProdTest*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnGetTP(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}



/**
 * @brief   DUCI call back function for TP Command - Perform self-test
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetKP(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateProdTest *myInstance = (DCommsStateProdTest*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetKP(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for TM Command - Display Test Message
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetTM(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateProdTest *myInstance = (DCommsStateProdTest*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetTM(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI call back function for TP Command - Perform self-test
 * @param   instance is a pointer to the FSM state instance
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetTP(void *instance, sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    DCommsStateProdTest *myInstance = (DCommsStateProdTest*)instance;

    if (myInstance != NULL)
    {
        duciError = myInstance->fnSetTP(parameterArray);
    }
    else
    {
        duciError.unhandledMessage = 1u;
    }

    return duciError;
}

/**
 * @brief   DUCI handler for SD Command – Get date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnGetSD(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        sendString("!SD=01/01/2020");
    }

    return duciError;
}

/**
 * @brief   DUCI handler for SD Command – Set date
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetSD(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //TODO: Get real value
        sendString("Set SD=TODO");
    }

    return duciError;
}

/**
 * @brief   DUCI handler for ST Command – Get time
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnGetST(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //TODO: Get real value
        sendString("!ST=12:00:00");
    }

    return duciError;
}

/**
 * @brief   DUCI handler for ST Command – Set time
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetST(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //TODO: Get real value
        sendString("Set ST=TODO");
    }

    return duciError;
}

/**
 * @brief   DUCI handler for KP Command - Key Press Emulation
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnGetKP(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        uint32_t value = myProductionTest->getKeys();

        snprintf(myTxBuffer, 16u, "!KP=%x", value);
        sendString(myTxBuffer);
    }

    return duciError;
}

/**
 * @brief   DUCI handler for TP Command - Get self-test status
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnGetTP(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        eTestPointNumber_t index = (eTestPointNumber_t)parameterArray[0].intNumber;
        int32_t value = 0;
        float32_t floatValue = 0.0f;
        eArgType_t returnValueType = argCustom;
        switch (index)
        {
            case E_TP3_EEPROM_SELF_TEST:
                value = myProductionTest->queryEepromSelfTest();
                returnValueType = argInteger;
                break;

            case E_TP4_BAROMETER_DEVICE_ID:
                value = myProductionTest->getBarometerDeviceId();
                returnValueType = argInteger;
                break;

            case E_TP9_POWER_BUTTON_MONITOR:
                value = myProductionTest->powerButtonMonitor();
                returnValueType = argInteger;
                break;

            case E_TP20_SPI_FLASH_SELF_TEST:
                value = myProductionTest->querySpiFlashSelfTest();
                returnValueType = argInteger;
                break;     

            case E_TP90_BLUETOOTH_DEVICE_ID:
                value = myProductionTest->getBluetoothDeviceId();
                returnValueType = argInteger;
                break;


            case E_TP102_STEPPER_MOTOR_DRIVER_ID :
                value = myProductionTest->getStepperMotorDeviceId();
                returnValueType = argInteger;
              break;
              
            case E_TP103_24VOLT_SUPPLY_STATUS:
                value = myProductionTest->get24VoltSupplyStatus();
                returnValueType = argInteger;
              break;
              
            case E_TP104_6VOLT_SUPPLY_STATUS:
              value = myProductionTest->get6VoltSupplyStatus();
               returnValueType = argInteger;
              break;
              
            case E_TP105_5VOLT_SUPPLY_STATUS:
                value = myProductionTest->get5VoltSupplyStatus();
                returnValueType = argInteger;
              break;
              
            case E_TP106_PM620_5VOLT_STATUS:
              value = myProductionTest->get5VoltPm620SupplyStatus();
              returnValueType = argInteger;
              break;

            case E_TP110_TEMPERATURE_SENSOR_ID:
              value = myProductionTest->getTemperatureSensorDeviceId();
                returnValueType = argInteger;
              break;
              
            case E_TP111_PM620_SENSOR_ID:
              value = myProductionTest->getPM620DeviceId();
                returnValueType = argInteger;
              break;
              
            case E_TP112_BATTERY_ID:
                value = myProductionTest->getBatteryId();
                returnValueType = argInteger;
              break;
              
            case E_TP113_BATTERY_CHARGER_ID:
              value = myProductionTest->getBatteryChargerId();
                returnValueType = argInteger;
              break;      
              
            case E_TP114_GET_24V_VALUE:
                floatValue = myProductionTest->get24VoltSupplyValue();
                returnValueType = argValue;
              break;
              
            case E_TP115_GET_6V_VALUE :
                floatValue = myProductionTest->get6VoltSupplyValue();
                returnValueType = argValue;
              break;
              
            case E_TP116_GET_5V_VALUE :
                floatValue = myProductionTest->get5VoltSupplyValue();
                returnValueType = argValue;
              break;
              
           case E_TP118_BAROMETER_READING:
              myProductionTest->getBarometerReading((float32_t *)&floatValue);
              returnValueType = argValue;
           break;
           
           case E_TP119_PM620_READING:
              myProductionTest->getPM620Reading((float32_t *)&floatValue);
              returnValueType = argValue;
           break;
           
          case E_TP120_IR_SENSOR_ADC_COUNTS:
              value = (int32_t)myProductionTest->getIrSensorAdcCounts();
              returnValueType = argInteger;
          break;
          
          case E_TP124_INVALIDATE_CAL_DATA:
                value = myProductionTest->queryInvalidateCalOpeResult();
                returnValueType = argInteger;
                break;
            default:
                duciError.commandFailed = 1u;
                break;
        }

        //if command parameter is good then output result
        if (duciError.commandFailed == 0u)
        {
            if((eArgType_t)argInteger == returnValueType)
            {
              snprintf(myTxBuffer, 16u, "!TP%d=%d", index, value);
              sendString(myTxBuffer);
            }
            else  if((eArgType_t)argValue == returnValueType)
            {
              snprintf(myTxBuffer, 16u, "!TP%d=%5.3f", index, floatValue);
              sendString(myTxBuffer);
            }
            else
            {
              /* Do nothing */
            }
        }
    }

    return duciError;
}



/**
 * @brief   DUCI handler for KP Command - Key Press Emulation
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetKP(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        uint32_t keyId = (uint32_t)parameterArray[1].intNumber;     //first parameter (after '=' sign) is key code
        uint32_t pressType = (uint32_t)parameterArray[2].intNumber; //second parameter is the press type (short or long)

        //syntax is =<parameter>[,=<parameter>]
        if ((keyId <= 1u) && (pressType <= 1u))
        {
            myProductionTest->setKeys(keyId, pressType);
        }
        else
        {
            duciError.commandFailed = 1u;
        }
    }

    return duciError;
}

/**
 * @brief   DUCI handler for TM Command - Display Test Message
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetTM(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        myProductionTest->displayTestMessage(parameterArray[1].charArray);
    }

    return duciError;
}

/**
 * @brief   DUCI handler for TP Command - Perform self-test
 * @param   parameterArray is the array of received command parameters
 * @retval  error status
 */
sDuciError_t DCommsStateProdTest::fnSetTP(sDuciParameter_t *parameterArray)
{
    sDuciError_t duciError;
    duciError.value = 0u;

   
    //only accepted message in this state is a reply type
    if (myParser->messageType != (eDuciMessage_t)E_DUCI_COMMAND)
    {
        duciError.invalid_response = 1u;
    }
    else
    {
        //syntax is =<index>[=<parameter>]
        switch (parameterArray[0].intNumber)
        {

            case E_TP3_EEPROM_SELF_TEST:
                myProductionTest->eepromSelfTest();
                break;
                
            case E_TP7_SOFTWARE_SHUT_DOWN:
                myProductionTest->softwareShutdown(parameterArray[2].intNumber);
                break;

            case E_TP20_SPI_FLASH_SELF_TEST:
                myProductionTest->spiFlashSelfTest();
                break;

            case E_TP91_BLUETOOTH_RESET:
                myProductionTest->bluetoothReset(parameterArray[2].intNumber);
                break;

            case E_TP100_SWITCH_ON_LED:
                myProductionTest->switchOnLed(parameterArray[2].intNumber);
                break;

           case E_TP101_SWITCH_OFF_LED:
               myProductionTest->switchOffLed(parameterArray[2].intNumber);
             break;
             

           case E_TP107_TEST_VALVE1:
             myProductionTest->testValve1(parameterArray[2].intNumber);
             break;
             
           case E_TP108_TEST_VALVE2:
             myProductionTest->testValve2(parameterArray[2].intNumber);
             break;
             
           case E_TP109_TEST_VALVE3:
             myProductionTest->testValve3(parameterArray[2].intNumber);
             break;
          
           case E_TP117_BATTERY_STATUS:
             myProductionTest->displayBatteryStatus();
               break;
               
          case E_TP121_SET_STEPPER_MOTOR_PARAM:
            myProductionTest->setStepperMotorParam(parameterArray[2].intNumber);
          break;
          
          case E_TP122_CONTROL_CHARGER_ENABLE_PIN:
            myProductionTest->controlChargerEnablePin(parameterArray[2].intNumber);
            break;
            
          case E_TP124_INVALIDATE_CAL_DATA:
                myProductionTest->invalidateCalibrationData();
                break;
                
            default:
                duciError.commandFailed = 1u;
                break;
        }
    }

    return duciError;
}

#endif //PRODUCTION_TEST_BUILD