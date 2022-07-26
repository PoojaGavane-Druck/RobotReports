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
* @file     validation.h
*
* @author   Tushar Mittal
* @date     July 2022
*
*/

#ifndef __CVALIDATION_H
#define __CVALIDATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
  
/* Private defines -----------------------------------------------------------*/
extern uint32_t __ICFEDIT_region_ROM_start__;
extern uint32_t __ICFEDIT_region_ROM_end__;
extern uint32_t __ICFEDIT_region_BANK_1_start__;
extern uint32_t __ICFEDIT_region_BANK_1_end__;


/* Constants necessary for Flash CRC calculation  (last block - 64 bytes - separated for CRC) */
#define BANK_1_START (uint8_t *)&__ICFEDIT_region_BANK_1_start__ 
#define BANK_1_END   (uint8_t *)((uint8_t *)&__ICFEDIT_region_BANK_1_end__
#define BANK_1_SIZE  (uint32_t)(ROM_END - ROM_START + 1u)
  
/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/

extern const uint32_t cBank1AppStartAddr;
extern const uint32_t cBank2AppStartAddr;
extern const uint32_t cBankBothAppStartAddr;
extern const uint32_t cFlashBank1CrcAddr;
extern const uint32_t cFlashBank2CrcAddr;
extern const uint32_t cFlashBankBothCrcAddr;
extern const uint32_t cAppHeaderOffset; 

extern const uint32_t cFlashBank1AppSizeAddr;
extern const uint32_t cFlashBank2AppSizeAddr;
extern const uint32_t cFlashBankBothAppSizeAddr;
extern const uint32_t cMaxAppImageLength;


/* Exported macro ------------------------------------------------------------*/

// None

/* Exported functions prototypes ---------------------------------------------*/

extern uint32_t validation_bankCRC( const uint32_t bank, const uint32_t actualCrc );

#ifdef __cplusplus
}
#endif

#endif /* __CVALIDATION_H */