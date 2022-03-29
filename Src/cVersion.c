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
#if 0
/////////////////////////////////////////////////////////////////////
extern const unsigned int __checksum;

const unsigned char  crc_start        @ "crc_start_mark" = 0;
const unsigned char  crc_end[4]       @ "crc_end_mark" = {0, 0, 0, 0};

const unsigned int mainBoardHardwareRevision = 0;
#pragma section = "MAIN_ROM_CONTENT"
const size_t MAIN_ROM_CONTENT_size    @ "ROM_length_used" = __section_size("MAIN_ROM_CONTENT");

const unsigned char  cAppVersion[4]     @ "applicationVersion" = {0, 0, 3, BUILD_NUMBER % 100};  // Application version number unused.Major.Minor.Issue. BUILD_NUMBER must be defined in the environment.
const unsigned int   cAppDK             @ "applicationDk"      = 492u;         // Application DK number
const char           cAppInstrument[16] @ "instrumentType"     = "PV624-BASE";    // Instrument Type
#else
const unsigned char  cAppVersion[4]      = {0, 0, 5, BUILD_NUMBER % 100};  // Application version number unused.Major.Minor.Issue. BUILD_NUMBER must be defined in the environment.
const unsigned int   cAppDK              = 499u;         // Application DK number
const char           cAppInstrument[16]  = "PV624-BASE";    // Instrument Type
const unsigned int mainBoardHardwareRevision = 0;
#endif

/////////////////////////////////////////////////////////////////////