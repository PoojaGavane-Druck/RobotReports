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
* @file     Types.h
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     24 March 2020
*
* @brief    The utilities and helper functions header file
*/
//*********************************************************************************************************************
#ifndef __TYPES_H
#define __TYPES_H

#ifdef __cplusplus
extern "C"
{
/* External C language linkage */
#endif

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <rtos.h>
MISRAC_ENABLE

////////////////////////////////////////  LEGACY STUFF -> to be removed
#define  FALSE      0
#define  TRUE       1

#define UPGRADE_PV624_FIRMWARE  1
#define UPGRADE_PM620_FIRMWARE  2
/*Data Types*/
typedef void (*fnPtrFunction)(void);

/*direct messages sent to measure task have this bit field*/
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t event      : 8;
        uint32_t param8     : 8;
        uint32_t param16    : 16;
    };

} sTaskMessage_t;

/*direct messages sent to measure task have this bit field*/
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t led        : 3;
        uint32_t colour     : 4;
        uint32_t operation  : 4;
        uint32_t blinkingRate : 4;
        uint32_t ledStateAfterTimeout : 1;
        uint32_t displayTime    : 16;
    };

} sLedMessage_t;

//Instrument mode - all bits 0 means local mode, else remote or test as indicated by individual bits
typedef union
{
    uint32_t value;

    struct
    {
        uint32_t remoteOwi    : 1;
        uint32_t remoteUsb       : 1;
        uint32_t remoteBluetooth : 1;
        uint32_t remoteUsbTest   : 1;
    };

} sInstrumentMode_t;

typedef enum
{
    E_COMM_OWI_INTERFACE,
    E_COMM_USB_INTERFACE,
    E_COMM_BLUETOOTH_INTERFACE
} eCommInterface_t;

typedef enum
{
    E_COMM_MODE_LOCAL,
    E_COMM_MODE_REMOTE,
    E_COMM_MODE_TEST
} eCommModes_t;

typedef enum
{
    E_INSTRUMENT_TYPE_STD,
    E_INSTRUMENT_TYPE_AERO

} eInstrumentType_t;

typedef union
{
    uint32_t all;

    struct
    {
        uint32_t build      : 8;
        uint32_t minor      : 8;
        uint32_t major      : 8;
        uint32_t reserved   : 8;
    };

} sVersion_t;

typedef enum
{
    E_LED_OPERATION_SWITCH_OFF,
    E_LED_OPERATION_SWITCH_ON,
    E_LED_OPERATION_TOGGLE,
    E_LED_OPERATION_NONE
} eLedOperation_t;

typedef enum
{
    E_LED_STATE_SWITCH_OFF,
    E_LED_STATE_SWITCH_ON
} eLedState_t;

/* Types ------------------------------------------------------------------------------------------------------------*/


//function ids
typedef enum
{
    E_FUNCTION_GAUGE = 0,
    E_FUNCTION_ABS,
    E_FUNCTION_BAROMETER,
    E_FUNCTION_MAX,
    E_FUNCTION_NONE
} eFunction_t;

typedef enum
{
    E_POWER_STATE_OFF = 0,
    E_POWER_STATE_ON
} ePowerState_t;

typedef enum
{
    E_BL_TASK_SUSPENDED = 0,
    E_BL_TASK_RUNNING
} eBluetoothTaskState_t;

typedef enum
{
    E_CHANNEL_0 = (uint32_t)0X01,
    E_CHANNEL_1 = (uint32_t)0X02,
    E_CHANNEL_2 = (uint32_t)0X04,
    E_CHANNEL_3 = (uint32_t)0x08

} eChannelSelection_t;

typedef enum
{
    E_VAL_INDEX_VALUE = 0,      //processed value
    E_VAL_INDEX_RAW_VALUE,      //calibrated measured value
    E_VAL_INDEX_POS_FS,         //positive full scale
    E_VAL_INDEX_NEG_FS,         //negative full scale
    E_VAL_INDEX_POS_FS_ABS,     //absolute positive full scale
    E_VAL_INDEX_NEG_FS_ABS,     //absolute negative full scale
    E_VAL_INDEX_RESOLUTION,     //resolution (precision) of measurement
    E_VAL_INDEX_DECIMAL_PLACES, //resolution (precision) expressed as number of decimal places
    E_VAL_INDEX_SERIAL_NUMBER,  //serial number
    E_VAL_INDEX_RANGE,           //range
    E_VAL_INDEX_BAROMETER_VALUE, //Barometer Processed Value
    E_VAL_INDEX_BAROMETER_RAW_VALUE,
    E_VAL_INDEX_USER_CAL_DATE,
    E_VAL_INDEX_FACTORY_CAL_DATE,
    E_VAL_INDEX_MANUFACTURING_DATE,
    E_VAL_INDEX_CAL_INTERVAL,
    E_VAL_INDEX_SENSOR_TYPE,
    EVAL_INDEX_GAUGE,
    EVAL_INDEX_ABS,
    EVAL_INDEX_BATTERY_TEMPERATURE,
    EVAL_INDEX_BATTERY_VOLTAGE,
    EVAL_INDEX_BATTERY_CURRENT,
    EVAL_INDEX_REMAINING_BATTERY_PERCENTAGE,
    EVAL_INDEX_REMAINING_BATTERY_LIFE,
    EVAL_INDEX_TIME_REQUIRED_FOR_FULL_CHARGE,
    EVAL_INDEX_REMAINING_BATTERY_CAPACITY,
    EVAL_INDEX_REMAINING_BATTERY_CAPACITY_WHEN_FULLY_CHARGED,
    EVAL_INDEX_DESIRED_CHARGING_CURRENT,
    EVAL_INDEX_DESIRED_CHARGING_VOLTAGE,
    EVAL_INDEX_BATTERY_STATUS_INFO,
    EVAL_INDEX_CHARGE_DISCHARGE_CYCLE_COUNT,
    EVAL_INDEX_BATTERY_SERIAL_NUMBER,
    EVAL_INDEX_BATTERY_5VOLT_VALUE,
    EVAL_INDEX_BATTERY_6VOLT_VALUE,
    EVAL_INDEX_BATTERY_24VOLT_VALUE,
    EVAL_INDEX_BATTERY_5VOLT_STATUS,
    EVAL_INDEX_BATTERY_6VOLT_STATUS,
    EVAL_INDEX_BATTERY_24VOLT_STATUS,
    EVAL_INDEX_BAROMETER_ID,
    EVAL_INDEX_BATTERY_ID,
    EVAL_INDEX_PM620_ID,
    EVAL_INDEX_BLUETOOTH_ID,
    EVAL_INDEX_TEMPERATURE_SENSOR_ID,
    EVAL_INDEX_SENSOR_MANF_ID,
    EVAL_INDEX_IR_SENSOR_ADC_COUNTS,
    E_VAL_INDEX_PM620_APP_IDENTITY,
    E_VAL_INDEX_PM620_BL_IDENTITY,
    E_VAL_INDEX_SENSOR_POS_FS,         //positive full scale
    E_VAL_INDEX_SENSOR_NEG_FS,         //negative full scale
    E_VAL_INDEX_CONTROLLER_MODE,
    E_VAL_INDEX_PRESSURE_SETPOINT,
    E_VAL_INDEX_SAMPLE_RATE,
    E_VAL_INDEX_PM620_TYPE,
    E_VAL_INDEX_CAL_POINT_VALUE,
    E_VAL_INDEX_SYNCH_TIME,
    E_VAL_INDEX_SAMPLE_TIME,
    E_VAL_INDEX_SAMPLE_TIMEOUT,
    E_VAL_INDEX_CAL_TYPE,
    E_VAL_INDEX_CAL_RANGE,
    E_VAL_INDEX_CAL_POINT,
    E_VAL_INDEX_CAL_SAMPLE_COUNT,
    E_VAL_INDEX_CONTROLLER_STATUS,
    E_VAL_INDEX_CONTROLLER_STATUS_PM,
    E_VAL_INDEX_BATTERY_RELATIVE_STATE_OF_CHARGE,
    E_VAL_INDEX_BATTERY_TIME_TO_EMPTY,
    E_VAL_INDEX_SENSOR_BRAND_UNITS,
    E_VAL_CURRENT_PRESSURE,
    E_VAL_INDEX_SENSOR_MODE,
    E_VAL_INDEX_CHARGING_STATUS,
    E_VAL_INDEX_PERCENTAGE_CAPACITY,
    E_VAL_INDEX_VENT_RATE,
    E_VAL_INDEX_BARO_SENSOR_POS_FS,         //positive full scale
    E_VAL_INDEX_BARO_SENSOR_NEG_FS,         //negative full scale
    E_VAL_INDEX_SENSOR_BRAND_MIN,
    E_VAL_INDEX_SENSOR_BRAND_MAX,
    E_VAL_INDEX_SENSOR_BRAND_TYPE

} eValueIndex_t;
//function measure/source direction
typedef enum
{
    E_FUNCTION_DIR_MEASURE = 0,
    E_FUNCTION_DIR_SOURCE

} eFunctionDir_t;

typedef enum
{
    E_VALVE_MODE_TDMA = 0u,
    E_VALVE_MODE_PWMA
} eValveMode_t;

typedef enum
{
    E_PIN_MODE_NONE          = 0x0000u,
    E_PIN_MODE_CALIBRATION   = 0x0001u, //remote PIN 123; local: user PIN (default 4321)
    E_PIN_MODE_CONFIGURATION = 0x0002u, //remote PIN 777, local PIN 1129
    E_PIN_MODE_FACTORY       = 0x0004u, //remote PIN 800, local PIN 8001  //TODO HSB: TBD
    E_PIN_MODE_ENGINEERING   = 0x0008u, //remote PIN 187, local PIN 1875
    E_PIN_MODE_UPGRADE       = 0x0010u,  //remote PIN 548, local PIN 5487
    E_PIN_MODE_OPTION_ENABLE = 0x0020u  //remote PIN 796, local PIN 7969

} ePinMode_t;

typedef enum
{
    eBarometerTask = 0u,
    ePM620Task = 1u,
    eMeasureAndControlTask = 2u,
    eUserInterfaceTask = 3u,
    eKeypadTask = 4u,
    eLoggerTask = 5u,
    eExternalStorageTask = 6u,
    eCommunicationOverOwiTask = 7u,
    eCommunicationOverUsbTask = 8u,
    eCommunicationOverBluetoothTask = 9u,
    ePowerManagerTask = 10u,
    eTaskNone = 11u,
    eNumberOfTasks = eTaskNone,
    eProductionTestTask = 12u

} eTaskID_t;
//date structure
typedef struct
{
    uint32_t day;
    uint32_t month;
    uint32_t year;

} sDate_t;

typedef struct
{
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;
    uint32_t milliseconds;      // add millisecond based on data log
} sTime_t;

typedef enum
{
    E_ERROR_CODE_NONE,
    E_ERROR_LOW_REFERENCE_SENSOR_VOLTAGE,
    E_ERROR_REFERENCE_SENSOR_COM,
    E_ERROR_BAROMETER_SENSOR,
    E_ERROR_STEPPER_CONTROLLER,
    E_ERROR_MOTOR_VOLTAGE,
    E_ERROR_STEPPER_DRIVER,
    E_ERROR_VALVE,
    E_ERROR_EEPROM,
    E_ERROR_BATTERY_WARNING_LEVEL,
    E_ERROR_BATTERY_CRITICAL_LEVEL,
    E_ERROR_EXTERNAL_FLASH_CORRUPT,
    E_ERROR_EXTERNAL_FLASH_WRITE,
    E_ERROR_ON_BOARD_FLASH,
    E_ERROR_OVER_TEMPERATURE,
    E_ERROR_BATTERY_COMM,
    E_ERROR_BATTERY_CHARGER_COMM,
    E_ERROR_OS,
    E_ERROR_CODE_EXTERNAL_STORAGE,
    E_ERROR_REFERENCE_SENSOR_OUT_OF_CAL,
    E_ERROR_BAROMETER_OUT_OF_CAL,
    E_ERROR_BAROMETER_SENSOR_MODE,
    E_ERROR_BAROMETER_SENSOR_CAL_STATUS,
    E_ERROR_BAROMETER_NOT_ENABLED,
    E_ERROR_CHARGER_CONNECTED,
    E_ERROR_CODE_DRIVER_BLUETOOTH,
    E_ERROR_CODE_IWDG,
    E_ERROR_CODE_FIRMWARE_UPGRADE_FAILED,
    E_ERROR_CODE_REMOTE_REQUEST_FROM_BT_MASTER,
    E_ERROR_CODE_REMOTE_REQUEST_FROM_OWI_MASTER,

} eErrorCode_t;

/*sensor types */
typedef enum
{
    E_SENSOR_TYPE_GENERIC = 0,  /*0 - no type specified or can only be one type */
    E_SENSOR_TYPE_PRESS_ABS,    /*1 - absolute pressure sensor */
    E_SENSOR_TYPE_PRESS_GAUGE,  /*2 - gauge pressure sensor */
    E_SENSOR_TYPE_PRESS_DIFF,   /*3 - differential pressure sensor */
    E_SENSOR_TYPE_PRESS_SG,     /*4 - sealed gauge pressure sensor */
    E_SENSOR_TYPE_PRESS_BARO,   /*5 - barometric pressure sensor */
    E_SENSOR_TYPE_RESISTANCE,   /*6 - resistance sensor */
    E_SENSOR_TYPE_TEMPERATURE,  /*7 - temperature sensor */

    E_SENSOR_TYPE_MAX

} eSensorType_t;

typedef enum
{
    E_PROCESS_SENSOR_ALARM_HI,
    E_PROCESS_SENSOR_ALARM_LO,
    E_PROCESS_USER_ALARM_HI,
    E_PROCESS_USER_ALARM_LO,
    E_PROCESS_FILTER,
    E_PROCESS_MAXIMUM,
    E_PROCESS_MINIMUM,
    E_PROCESS_TARE,
    E_PROCESS_NUMBER

} eProcess_t;


typedef enum
{
    E_CONTINIOUS_ACQ_MODE = 0,
    E_REQUEST_BASED_ACQ_MODE

} eAquisationMode_t;

/*calibration type*/
typedef enum
{
    E_CAL_TYPE_USER = 0,    /* user calibration */
    E_CAL_TYPE_INTERNAL     /* reserved for factory or other special cal */

} eCalType_t;

typedef enum
{
    E_CONTROLLER_MODE_MEASURE = 0,
    E_CONTROLLER_MODE_CONTROL,
    E_CONTROLLER_MODE_VENT,
    E_CONTROLLER_MODE_RATE,
    E_CONTROLLER_MODE_PAUSE,
    E_CONTROLLER_MODE_FM_UPGRADE,
    E_CONTROLLER_MODE_NONE
} eControllerMode_t;

typedef enum
{
    E_PM620_SENSOR = 0,
    E_BAROMETER_SENSOR
} eSensor_t;
//Define exact width type for floationg point number
typedef float float32_t;

typedef struct
{
    float32_t x;    //input
    float32_t y;    //output

} sCoordinates_t;


typedef union
{
    uint8_t byteValue[4];
    float32_t floatValue;
} uFloat_t;

typedef union
{
    uint8_t byteValue[4];
    uint32_t uint32Value;
} uUint32_t;

typedef union
{
    uint8_t byteValue[2];
    uint16_t uint16Value;
} uUint16_t;

typedef union
{
    uint8_t byteValue[2];
    int16_t int16Value;
} uSint16_t;

typedef union
{
    uint8_t byteValue[4];
    int32_t int32Value;
} uSint32_t;

typedef enum
{
    eDataTypeNone = 0,
    eDataTypeBoolean,
    eDataTypeByte,
    eDataTypeUnsignedChar,
    eDataTypeSignedChar,
    eDataTypeUnsignedShort,
    eDataTypeSignedShort,
    eDataTypeUnsignedLong,
    eDataTypeSignedLong,
    eDataTypeFloat,
    eDataTypeDouble
} eDataType_t;
//battery status structure
typedef struct
{
    float32_t voltage; // battery voltage in Volts
    float32_t current; // current in mA
    float32_t bl;      // battery level (remaining capacity) as a percentage
    float32_t soc;     // state of charge in mAh
    uint32_t tte;     // time to empty in minutes
    uint32_t dc;           // DC_PRESENT state (ie, is DC supply plugged in)
    uint32_t bt;           // UP_BAT_SEL battery type state (false = Li-Ion, true = LiFePO4)

} sBatteryStatus_t;

typedef union
{
    char charArray[4];
    int32_t intValue;
    uint32_t uintValue;
    float32_t floatValue;
    bool flagValue;
} uParameter_t;


/* Prototypes -------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}                                                               /* End of external C language linkage */
#endif

#endif //__TYPES_H
