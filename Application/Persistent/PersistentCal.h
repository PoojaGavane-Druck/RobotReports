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
* @file     PersistentCal.h
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     17 June 2020
*
* @brief    The persistent (non-volatile) calibration data header file
*/
//*********************************************************************************************************************

#ifndef _PERSISTENT_CAL_H
#define _PERSISTENT_CAL_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <stdbool.h>
MISRAC_ENABLE

#include "Types.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
#define MAX_CAL_POINTS      3    //Three-point cal is the most
#define MAX_CAL_RANGES      2    //support up to two ranges for each sensor

/* Types ------------------------------------------------------------------------------------------------------------*/
//individual coordinate of a calibration on a "y = mx + c" fit
typedef struct
{
    float32_t x;    //input
    float32_t y;    //output
} sCalPoint_t;

//derived cal data for a straight line (y = mx + c), applicable to each segment
typedef struct
{
    float32_t m;    //gain
    float32_t c;    //offset
} sCalSegment_t;

//sensor cal data structure, allows for a fixed number of ranges and a cal interval
typedef struct
{
    //sCalRange_t cal[MAX_CAL_RANGES];
    sDate_t       calDate;                  //calibration date
    uint32_t      numPoints;            //no of cal points
    uint32_t      numSegments;          //no of straight line segments
    sCalPoint_t   calPoints[MAX_CAL_POINTS];    //cal points used
    sCalSegment_t segments[MAX_CAL_POINTS - 1]; //array of straight line segments
    float32_t     breakpoint[MAX_CAL_POINTS - 2]; //segment breakpoints
    uint32_t      calInterval;
    sDate_t       nextCalDate;                  //Next calibration date
    uint32_t      nextCalDateSetStatus;
    uint32_t      calDateSetStatus;
    uint32_t      calIntervalSetStatus;


} sSensorCal_t;

// Tells about it is calibrated or not
typedef enum calibrationStatus_tag
{
    SENSOR_CALIBRATED     =       0X45454545u,
    SENSOR_NOT_CALIBRATED =       0XFFFFFFFFu
} eCalibrationStatus_t;

//sensor data structure, allows for a fixed number of ranges
typedef struct
{
    sSensorCal_t  data;
    eCalibrationStatus_t calStatus;
    uint32_t      crc;
} sSensorData_t;

//non-volatile data structure for instrument calibration data
typedef struct
{
    uint32_t            revision;           //revision of the data structure
    sDate_t             modifiedDate;       //creation or 'last modified' date
    sDate_t             calDate;            //instrument calibration date
    uint32_t            calInterval;        //instrument calibration interval
    sSensorData_t       measureBarometer;   //cal data for barometer

} sCalData_t;

//data structure with checksum
typedef struct
{
    union
    {
        sCalData_t  data;                       //configuration data
        char        bytes[sizeof(sCalData_t)];  //byte array
    };
    uint32_t crc;                               //cyclic redundancy check for this data structure
} sPersistentCal_t;

#endif // _PERSISTENT_CAL_H
