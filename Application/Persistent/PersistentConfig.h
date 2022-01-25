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
* @file     PersistentConfig.h
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     17 June 2020
*
* @brief    The persistent (non-volatile) configuration data header file
*/

#ifndef _PERSISTENT_CONFIG_H
#define _PERSISTENT_CONFIG_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

#include "Types.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define INSTRUMENT_ID_SIZE     4u

/* Types ------------------------------------------------------------------------------------------------------------*/

/*Area of use � region of the world in which the instrument is to be used ------------------------------------------*/
typedef enum
{
    E_REGION_NOT_SET = 0,   //Not yet set
    E_REGION_WORLD,         //Rest of the world
    E_REGION_EUROPE,        //Europe
    E_REGION_USA_CANADA,    //North America (USA & Canada)
    E_REGION_CHINA,         //China
    E_REGION_JAPAN,         //Japan
    E_REGION_SOUTH_KOREA,   //South Korea
    E_REGION_NUMBER         //NOTE: this must always be the last entry

} eRegionOfUse_t;


/*non-volatile data structure for instrument factory configuration*/
typedef struct
{
    uint32_t        revision;   //Revision of persistent data structure
    uint32_t        serialNumber;   //instrument serial number - may be alphanumeric string
    eRegionOfUse_t      region;      //area of use
    eInstrumentType_t   instrumentType;    //instrument variant (eg, standard or aeronautical)

} sConfig_t;

typedef struct
{
    union
    {
        sConfig_t   data;                               //configuration data
        char        bytes[sizeof(sConfig_t)];           //byte array
    };

    uint32_t crc;                                       //cyclic redundancy check for this data structure

} sPersistentConfig_t;

#endif // _PERSISTENT_CONFIG_H
