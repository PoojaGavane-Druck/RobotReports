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
* @file     cLPS22HH.h
*
* @author   Elvis Esa
* @date     July 2020
*
* __CLPS22HH_H header file
*/

#ifndef __CLPS22HH_H
#define __CLPS22HH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/

/* None */

/* Private defines -----------------------------------------------------------*/

/* None */

/* Exported types ------------------------------------------------------------*/
typedef enum { eBARO_INTERNAL_ON_BOARD = 0, eBARO_INTERNAL_LEADED = 1, eBARO_END = 0xFFFFFFFFu } eBaro_t;

/* None */

/* Exported constants --------------------------------------------------------*/

extern const uint32_t cLPS22HH_Const_WhoAmI;

/* Exported macro ------------------------------------------------------------*/

/* None */

/* Exported functions prototypes ---------------------------------------------*/

extern bool LPS22HH_initialise(eBaro_t eBaro);
extern bool LPS22HH_trigger(void);
extern bool LPS22HH_read(float *pPrs_hPa);
extern bool LPS22HH_close(void);

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __CLPS22HH_H */