/**
* BHGE Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DCommsUSB.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     26 May 2020
*
* @brief    The serial USB comms class header file
*/

#ifndef __DCOMMS_USB_H
#define __DCOMMS_USB_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DComms.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsUSB : public DComms
{
public:
    DCommsUSB(char *mediumName, OS_ERR *osErr);
    void initialise(void);
};

#endif /* __DCOMMS_USB_H */
