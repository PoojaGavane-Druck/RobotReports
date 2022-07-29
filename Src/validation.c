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
* @file     validation.c
*
* @author   Tushar Mittal
* @date     July 2022
*
* Functions for validation routines used in the bootloader
*/

/* Includes -------------------------------------------------------------------------------*/

#include "misra.h"
MISRAC_DISABLE
#include "main.h"
MISRAC_ENABLE
#include "validation.h"
#include "crc.h"

/* Private defines ------------------------------------------------------------------------*/
#define DEF_bankStartAddr      (( uint32_t )0x08000000u )
#define DEF_AppImageStartAddr  (( uint32_t )0x08020000u )   // Starting of the application image WARNING: must be on 4k page boundary
#define DEF_AppImageEndAddr    (( uint32_t )0x080FFFFFu )   // Ending of the application image WARNING: must be on 4k page boundary - this must be greater than DEF_AppImageStartAddr
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// WARNING : Must be aligned with FIXED_ROM_CONTENT area in the linker script - all fixed area should be common //
///////           across all applications                                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////                         
#define DEF_AppHeaderOffset    (( uint32_t )0x00000300u )   // End of header information (below)
#define DEF_AppDKAddress       (( uint32_t )( DEF_AppImageStartAddr + 0x00000200u ))   // Fixed location in application Defined in linker script
#define DEF_AppVersionAddress  (( uint32_t )( DEF_AppImageStartAddr + 0x00000204u ))   // Fixed location in application Defined in linker script
#define DEF_AppCRCAddress      (( uint32_t )( DEF_AppImageStartAddr + 0x00000208u ))   // Fixed location in application Defined in linker script
#define DEF_InstTypeAddress    (( uint32_t )( DEF_AppImageStartAddr + 0x0000020Cu ))   // Fixed location in application Defined in linker script
#define DEF_AppSizeAddress     (( uint32_t )( DEF_AppImageStartAddr + 0x0000021Cu ))   // Fixed location in application Defined in linker script
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEF_Bank1Offset        (( uint32_t )0x00000000u )   // Offset to bank1
#define DEF_Bank2Offset        (( uint32_t )0x00100000u )   // Offset to bank2
#define DEF_BankBothOffset     (( uint32_t )0x00000000u )   // Offset to both banks
#define DEF_MaxAppImageLength  (( uint32_t )( DEF_AppImageEndAddr - DEF_AppImageStartAddr + 1u ))   // Max length of application image only (i.e 0x8xFFFFF - 0x8x20000)

#define DEF_AppHeaderOffset    (( uint32_t )0x00000300u )   // End of header information (below)

/* Exported Function Prototypes -----------------------------------------------------------*/
uint32_t validation_bankCRC( const uint32_t bank, const uint32_t actualCrc );

/* Private Function Prototypes ------------------------------------------------------------*/

/* Private Variables ----------------------------------------------------------------------*/
const uint32_t cBank1AppStartAddr  = ( DEF_AppImageStartAddr + DEF_Bank1Offset );
const uint32_t cBank2AppStartAddr  = ( DEF_AppImageStartAddr + DEF_Bank2Offset );
const uint32_t cBankBothAppStartAddr  = ( DEF_AppImageStartAddr + DEF_BankBothOffset );
const uint32_t cFlashBank1CrcAddr = ( DEF_AppCRCAddress + DEF_Bank1Offset );
const uint32_t cFlashBank2CrcAddr = ( DEF_AppCRCAddress + DEF_Bank2Offset );
const uint32_t cFlashBankBothCrcAddr = ( DEF_AppCRCAddress + DEF_BankBothOffset );

const uint32_t cAppHeaderOffset = ( DEF_AppHeaderOffset ); 

const uint32_t cFlashBank1AppSizeAddr = ( DEF_AppSizeAddress + DEF_Bank1Offset );
const uint32_t cFlashBank2AppSizeAddr = ( DEF_AppSizeAddress + DEF_Bank2Offset );
const uint32_t cFlashBankBothAppSizeAddr = ( DEF_AppSizeAddress + DEF_BankBothOffset );
const uint32_t cMaxAppImageLength = DEF_MaxAppImageLength;

/* Global Variables -----------------------------------------------------------------------*/

/* External Variables ---------------------------------------------------------------------*/

extern const size_t MAIN_ROM_CONTENT_size;
extern const unsigned char crc_start;
extern CRC_HandleTypeDef CrcHandle;

/*-----------------------------------------------------------------------------------------*/

/*!
* @brief : Validate CRC of the specified bank
*
* @param[in]     : uint32_t bank - 1(app) or 2(temp)
*                : uint32_t actualCrc - actual CRC we need
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t error - 1=crc fail, 0=crc ok
* @note          : None
* @warning       : None
*/
// BANK1 Erase validate (VALIDATION 5)
// BANK1 CRC (VALIDATION 6)
// BANK2 CRC (VALIDATION 8)
uint32_t validation_bankCRC( const uint32_t bank, const uint32_t actualCrc )
{
  uint32_t calculatedCrc1 = 0u;
  uint32_t error  = 0u;
  uint32_t length = 0u;
  uint8_t* dataPtr;
  calculatedCrc1 = 0u;
  // Set addresses to compute crc from based on bank
  if( bank == FLASH_BANK_1 )
  {
    dataPtr = ( uint8_t* )cBank1AppStartAddr + cAppHeaderOffset;
    length = *(( uint32_t* )cFlashBank1AppSizeAddr);                           // Point to image size location in bank 1
  }
  else if( bank == FLASH_BANK_2 )
  {
    dataPtr = ( uint8_t* )cBank2AppStartAddr + cAppHeaderOffset; 
    length = *(( uint32_t* )cFlashBank2AppSizeAddr);                           // Point to image size location in Bank2
  }
  else if( bank == FLASH_BANK_BOTH )
  {
    dataPtr = ( uint8_t* )cBankBothAppStartAddr + cAppHeaderOffset; 
    length = *(( uint32_t* )cFlashBankBothAppSizeAddr);                           // Point to image size location in both banks
  }
  else
  {
    error = 1u;
  }
  
  // Check if we are within address range
  if( length > ( cMaxAppImageLength - 1u ))
  {
    error |= 1u;
  }
  
  // Check validity of size
  //error |= validation_bankBoundary( bank, ( uint32_t )dataPtr, length ); 
  
  if( error == 0u )
  {
    // Calculate CRC and compare
    calculatedCrc1 = HAL_CRC_Calculate( &CrcHandle, ( uint32_t* )dataPtr, length );

    if( calculatedCrc1 != actualCrc )
    {
      error |= 1u;
    }
  }
  
  return( error );
}
