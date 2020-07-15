/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     PersistentFunctions.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     22 June 2020
*
* @brief    The persistent (non-volatile) f settings header file
*/

#ifndef _PERSISTENT_FUNCTIONS_H
#define _PERSISTENT_FUNCTIONS_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <stdbool.h>
MISRAC_ENABLE

#include "Types.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define SCALING_CAPTION_SIZE    16u

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum : int32_t
{
    E_INIT_STATE_NOT_SET = 0,
    E_INIT_STATE_DISABLED,
    E_INIT_STATE_ENABLED,
    E_INIT_STATE_DEFINED

} eInitState_t;

//user alarm settings
typedef struct
{
    eInitState_t        state;                          //user alarm state (not-set, defined, disabled or enabled)
    float32_t			high;				            //high threshold setting for user alarm
    float32_t			low;                            //low threshold setting for user alarm

} sAlarmSettings_t;

//filter settings
typedef struct
{
    eInitState_t        state;                          //filter state (not-set, defined, disabled or enabled)
    float32_t			band;				            //threshold within which values are filtered
    float32_t			timeConstant;                   //proportion of change to add at each sample (not really a TC)

} sFilterSettings_t;

//scaling settings
typedef struct
{
    eInitState_t        state;                         //scaling state (not-set, defined, disabled or enabled)
    char                caption[SCALING_CAPTION_SIZE]; //threshold within which values are filtered
    sCoordinates_t      coordinates[2];                //two coordinates defining the linear translation to user units

} sScaling_t;

//measure function settings
typedef struct
{
    eFunction_t         function;                       //my function identification
    eFunctionDir_t      direction;                      //my measure/source direction
    eUnits_t            units;                          //units selected
    sFilterSettings_t   filter;                         //filter settings
    sAlarmSettings_t    alarm;                          //alarm settings
    sScaling_t          scaling;                        //scaling settings
    int32_t             mode;                           //this element has function specific meaning

} sFunctionSetting_t;

////measure RTD function settings
//typedef struct
//{
//    sFunctionSetting_t  generic;                        //generic measure function settings
//    eRtdSensorMode_t    mode;                           //temperature or resistance
//} sRtdFunctionSetting_t;
//
////measure HART function settings
//typedef struct
//{
//    sFunctionSetting_t  generic;                        //generic measure function settings
//    bool                resistor;                       //HART resistor on/off
//
//} sHartFunctionSetting_t;
//
////source function settings
//typedef struct
//{
//    eFunction_t         function;                       //my function identification
//    eFunctionDir_t      direction;                      //my measure/source direction
//    eUnits_t            units;                          //units selected
//
//} sSourceFunctionSetting_t;

//Persistent data structure for user settings
typedef struct
{
    uint32_t			        revision;			 	//Revision of persistent data structure

    sFunctionSetting_t          measureMA;              //measure mA function settings
    sFunctionSetting_t          measureMV;              //measure mV function settings
    sFunctionSetting_t          measureVolts;           //measure Volts function settings
    sFunctionSetting_t          measureIntP;            //measure internal pressure function settings
    sFunctionSetting_t          measureExtP;            //measure external pressure function settings
    sFunctionSetting_t          measureBarometer;       //measure barometer function settings

    sFunctionSetting_t          measureDiffIntExtP;     //Internal pressure - external pressure function settings
    sFunctionSetting_t          measureDiffIntBaro;     //Internal pressure - barometer function settings
    sFunctionSetting_t          measureDiffExtBaro;     //External pressure + barometer function settings

    sFunctionSetting_t          measureSumIntExtP;      //Internal pressure + external pressure function settings
    sFunctionSetting_t          measureSumIntBaro;      //Internal pressure + barometer function settings
    sFunctionSetting_t          measureSumExtBaro;      //External pressure + barometer function settings

    sFunctionSetting_t          measureRTD;             //measure RTD function settings
    sFunctionSetting_t          measureHART;            //HART function settings

//    sSourceFunctionSetting_t    sourceMA;               //source mA function settings - not required as there are no variables
//    sSourceFunctionSetting_t    sourceCh2Volts;         //CH2 voltage function settings

} sFunctionsData_t;

typedef struct
{
    union
    {
        sFunctionsData_t data;                          //Persistent data structure
        char bytes[sizeof(sFunctionsData_t)];           //byte array
    };

    uint32_t crc;                                       //cyclic redundancy check for this data structure

} sPersistentFunctions_t;

#endif // _PERSISTENT_FUNCTIONS_H
