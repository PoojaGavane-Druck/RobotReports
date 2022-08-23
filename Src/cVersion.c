/*!
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the
* property of Baker Hughes and its suppliers, and affiliates if any.
* The intellectual and technical concepts contained herein are
* proprietary to Baker Hughes and its suppliers and affiliates
* and may be covered by U.S. and Foreign Patents, patents in
* process, and are protected by trade secret or copyright law.
* Dissemination of this information or reproduction of this
* material is strictly forbidden unless prior written permission
* is obtained from Baker Hughes.
*
* @file     cVersion.c
*
*
* @author   Elvis Esa
* @date     September 2020
*
* Versioning/crc information reflected in the linker script which gets written
* to the flash to the appropriate area on compilation.
*/

#include "main.h"

/////////////////////////////////////////////////////////////////////
#define BUILD_NUMBER 6u
#define MAJOR_VERSION_NUMBER 0u
#define MINOR_VERSION_NUMBER 18u


extern const uint32_t __checksum;

const uint8_t  crc_start        @ "crc_start_mark" = 0;
const uint8_t  crc_end[4]       @ "crc_end_mark" = {0, 0, 0, 0};

const uint32_t mainBoardHardwareRevision = 1u;
#pragma section = "MAIN_ROM_CONTENT"
const size_t MAIN_ROM_CONTENT_size    @ "ROM_length_used" = __section_size("MAIN_ROM_CONTENT");
// Application version number unused.Major.Minor.Issue. BUILD_NUMBER must be defined in the environment.
const uint8_t  cAppVersion[4]     @ "applicationVersion" = {0u,
                                                            MAJOR_VERSION_NUMBER,
                                                            MINOR_VERSION_NUMBER,
                                                            BUILD_NUMBER % 100
                                                           };
const uint32_t   cAppDK             @ "applicationDk"      = 499u;         // Application DK number
const int8_t     cAppInstrument[16] @ "instrumentType"     = "PV624-BASE";    // Instrument Type


/////////////////////////////////////////////////////////////////////