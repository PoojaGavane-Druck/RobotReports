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
* @file     Application.h
* @version  1.00.00
* @author   Nageswara Pydisetty
* @date     21 January 2020
*
* @brief    The application header file
*/
//*********************************************************************************************************************
#ifndef __APPLICATION_H
#define __APPLICATION_H

#ifdef __cplusplus
extern "C"
{
/* External C language linkage */
#endif

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdint.h>
#include <rtos.h>
MISRAC_ENABLE

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/
void createAppTask(OS_ERR *os_error);
void applicationErrorHandler(void);


#ifdef __cplusplus
}                                                               /* End of external C language linkage */
#endif

#endif /* __APPLICATION_H */
