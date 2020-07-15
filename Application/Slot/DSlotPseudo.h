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
* @file     DSlotPseudo.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     02 June 2020
*
* @brief    The DSlotPseudo base class header file
*/

#ifndef _DSLOT_PSEUDO_H
#define _DSLOT_PSEUDO_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

#include "DSlot.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum : uint32_t
{
    E_SLOT_ASSOC_DIFF = 0u,         //primary slot minus secondary slot
    E_SLOT_ASSOC_DIFF_REVERSE,      //secondary slot minus primary slot
    E_SLOT_ASSOC_SUM                //primary slot plus secondary slot

} eSlotAssociation_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DSlotPseudo : public DSlot
{
protected:
    DSlot *myPrimarySlot;
    DSlot *mySecondarySlot;
    eSlotAssociation_t mySlotAssociation;

public:
    DSlotPseudo(DTask *owner);

    virtual void runFunction(void);

    void addSlots(DSlot *primSlot, DSlot *secSlot, eSlotAssociation_t assoc);
    void setSlotAssociation(eSlotAssociation_t assoc);
};

#endif // _DSLOT_PSEUDO_H
