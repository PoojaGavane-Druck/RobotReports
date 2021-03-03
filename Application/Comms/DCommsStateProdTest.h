/**
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DCommsStateProdTest.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The comms production test state class header file
*/

#ifndef __DCOMMS_STATE_PROD_TEST_H
#define __DCOMMS_STATE_PROD_TEST_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDuci.h"
#define PRODUCTION_TEST_BUILD
#ifdef PRODUCTION_TEST_BUILD
#include "DProductionTest.h"
#endif

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_TP2_PRESSURE_SENSOR_TEST = 2,
    E_TP3_EEPROM_SELF_TEST = 3,
    E_TP4_BAROMETER_DEVICE_ID = 4,
    E_TP7_SOFTWARE_SHUT_DOWN = 7,
    E_TP9_POWER_BUTTON_MONITOR = 9,
    E_TP20_SPI_FLASH_SELF_TEST = 20,
    E_TP90_BLUETOOTH_DEVICE_ID = 90,
    E_TP91_BLUETOOTH_RESET = 91,
    E_TP100_SWITCH_ON_LED = 100,
    E_TP101_SWITCH_OFF_LED = 101,
    E_TP102_STEPPER_MOTOR_DRIVER_ID = 102,
    E_TP103_24VOLT_SUPPLY_STATUS = 103,
    E_TP104_6VOLT_SUPPLY_STATUS = 104,
    E_TP105_5VOLT_SUPPLY_STATUS = 105,
    E_TP106_PM620_5VOLT_STATUS = 106,
    E_TP107_TEST_VALVE1 = 107,
    E_TP108_TEST_VALVE2 = 108,
    E_TP109_TEST_VALVE3 = 109,
    E_TP110_TEMPERATURE_SENSOR_ID = 110,
    E_TP111_PM620_SENSOR_ID = 111,
    E_TP112_BATTERY_ID = 112,
    E_TP113_BATTERY_CHARGER_ID = 113,
    E_TP114_GET_24V_VALUE = 114,
    E_TP115_GET_6V_VALUE = 115,
    E_TP116_GET_5V_VALUE = 116,
    E_TP117_BATTERY_STATUS = 117,
    E_TP118_BAROMETER_READING =118,
    E_TP119_PM620_READING =119,
    E_TP120_IR_SENSOR_ADC_COUNTS=120
} eTestPointNumber_t;
/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateProdTest : public DCommsStateDuci
{
#ifdef PRODUCTION_TEST_BUILD
private:
    DProductionTest *myProductionTest;

    static sDuciError_t fnGetKP(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetKP(void *instance, sDuciParameter_t *parameterArray);

    static sDuciError_t fnGetSD(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetSD(void *instance, sDuciParameter_t *parameterArray);

    static sDuciError_t fnGetST(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetST(void *instance, sDuciParameter_t *parameterArray);

    static sDuciError_t fnSetTM(void *instance, sDuciParameter_t *parameterArray);

    static sDuciError_t fnGetTP(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetTP(void *instance, sDuciParameter_t *parameterArray);



    //command handlers for this instance
    sDuciError_t fnGetKM(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetKM(sDuciParameter_t *parameterArray);

    sDuciError_t fnGetKP(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetKP(sDuciParameter_t *parameterArray);

    sDuciError_t fnGetSD(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetSD(sDuciParameter_t *parameterArray);

    sDuciError_t fnGetST(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetST(sDuciParameter_t *parameterArray);

    sDuciError_t fnSetTM(sDuciParameter_t *parameterArray);

    sDuciError_t fnGetTP(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetTP(sDuciParameter_t *parameterArray);

   
#endif

protected:
    virtual void createDuciCommands(void);

public:
    DCommsStateProdTest(DDeviceSerial *commsMedium, DTask *task);

    virtual eStateDuci_t run(void);
};

#endif /* __DCOMMS_STATE_PROD_TEST_H */
