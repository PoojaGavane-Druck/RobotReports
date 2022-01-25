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
* @file     PersistentMaintenanceData.h
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     22 June 2020
*
* @brief    The persistent (non-volatile) f settings header file
*/
//*********************************************************************************************************************

#ifndef _PERSISTENT_MAINTENANCE_H
#define _PERSISTENT_MAINTENANCE_H

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


typedef struct
{
    uint32_t     revision;      //Revision of persistent data structure

    uint32_t     numOfCompressorPumpHours ;   //Compressor pump Hours completed
    uint32_t     numOfStepperMotorHours;     //Steppor Motor Hours completed
    uint32_t     numOfSetPoints;   //number of set points completed

} sMaintenanceData_t;

typedef struct
{
    union
    {
        sMaintenanceData_t data;                          //Persistent data structure
        char bytes[sizeof(sMaintenanceData_t)];           //byte array
    };

    uint32_t crc;                                       //cyclic redundancy check for this data structure

} sPersistentMaintenanceData_t;

#endif // _PERSISTENT_FUNCTIONS_H
