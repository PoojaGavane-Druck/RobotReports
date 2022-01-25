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
* @file     PersistentSettings.h
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     17 June 2020
*
* @brief    The persistent (non-volatile) user settings header file
*/
//*********************************************************************************************************************

#ifndef _PERSISTENT_SETTINGS_H
#define _PERSISTENT_SETTINGS_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <stdbool.h>
MISRAC_ENABLE

#include "Types.h"
#include "DInstrument.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
//Persistent data structure for user settings


typedef struct
{
    uint32_t            revision;         //Revision of persistent data structure
    uint32_t            calPin;       //user calibration PIN
    uint32_t            autoPowerdown;    //auto-powerdown time in mins (0 = disabled)

} sUserSettings_t;

typedef struct
{
    union
    {
        sUserSettings_t data;                           //Persistent data structure
        char bytes[sizeof(sUserSettings_t)];            //byte array
    };

    uint32_t crc;                                       //cyclic redundancy check for this data structure

} sPersistentSettings_t;

#endif // _PERSISTENT_SETTINGS_H
