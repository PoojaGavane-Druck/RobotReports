/**
* BHGE Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DSlotPseudo.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     02 May 2020
*
* @brief    The DSlotPseudo class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <os.h>
MISRAC_ENABLE

#include "DSlotPseudo.h"
#include "DSensorPseudo.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSlotPseudo class constructor
 * @note    An owner is needed if sensor events are to be sent back. This is expected to be a DFunction instance.
 * @param   owner: the task that created this slot
 * @retval  void
 */
DSlotPseudo::DSlotPseudo(DTask *owner)
: DSlot(owner)
{
    mySensor = new DSensorPseudo();
    myName = "sPseudo";

    //specify the flags that this function must respond to (add more as necessary in derived class)
    myWaitFlags |=  EV_FLAG_TASK_SENSOR_CONTINUE |
                    EV_FLAG_TASK_SENSOR_RETRY |
                    EV_FLAG_TASK_NEW_VALUE |
                    EV_FLAG_TASK_SENSOR_DISCONNECT |
                    EV_FLAG_TASK_SENSOR_PAUSE |
                    EV_FLAG_TASK_SENSOR_CONNECT;
}

/**
 * @brief   Add slots with the specified association bewteen them
 * @param   pointer to primary slot instance (must not be NULL)
 * @param   pointer to secondary slot instance  (must not be NULL)
 * @param   association is an instruction on how to use the second slot
 * @retval  void
 */
void DSlotPseudo::addSlots(DSlot *primSlot, DSlot *secSlot, eSlotAssociation_t assoc)
{
    myPrimarySlot = primSlot;
    mySecondarySlot = secSlot;
    mySlotAssociation = assoc;
}

/**
 * @brief   Set or change the slot association
 * @param   association is an instruction on how to combine the two slots
 * @retval  void
 */
void DSlotPseudo::setSlotAssociation(eSlotAssociation_t assoc)
{
    mySlotAssociation = assoc;
}

/**
 * @brief   Run DSlotPseudo task funtion
 * @param   void
 * @retval  void
 */
void DSlotPseudo::runFunction(void)
{
    //this is a while loop that pends on event flags
    bool runFlag = true;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;
    eSensorError_t sensorError;
    DSensor *primarySensor = NULL;
    DSensor *secondarySensor = NULL;
    float32_t primaryRdg;
    float32_t secondaryRdg;

    //kick off the slots first
    if (myPrimarySlot != NULL)
    {
        myPrimarySlot->start();
        primarySensor = myPrimarySlot->getSensor();
    }

    if (mySecondarySlot != NULL)
    {
        mySecondarySlot->start();
        secondarySensor = mySecondarySlot->getSensor();
    }

    sensorError = mySensor->initialise();

    while (runFlag == true)
    {
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)500u, //runs, nominally, at 2Hz by default
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
        //check for events
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
            //TODO:
        }
        else if (os_error != (OS_ERR)OS_ERR_NONE)
        {
            //report error
        }
        else
        {
            if ((actualEvents & EV_FLAG_TASK_SHUTDOWN) == EV_FLAG_TASK_SHUTDOWN)
            {
                if (myPrimarySlot != NULL)
                {
                    myPrimarySlot->shutdown();
                }

                if (mySecondarySlot != NULL)
                {
                    mySecondarySlot->shutdown();
                }

                runFlag = false;
            }

            if ((actualEvents & EV_FLAG_TASK_NEW_VALUE) == EV_FLAG_TASK_NEW_VALUE)
            {
                //this could be from either of the two sub-slots
                primaryRdg = primarySensor->getMeasurement();
                secondaryRdg = secondarySensor->getMeasurement();

                //combine these to set the pseudo measurement value
                switch (mySlotAssociation)
                {
                    case E_SLOT_ASSOC_DIFF:
                        mySensor->setMeasurement(primaryRdg - secondaryRdg);
                        break;

                    case E_SLOT_ASSOC_DIFF_REVERSE:
                        mySensor->setMeasurement(secondaryRdg - primaryRdg);
                        break;

                    case E_SLOT_ASSOC_SUM:
                        mySensor->setMeasurement(primaryRdg + secondaryRdg);
                        break;

                     default:
                        break;
                }

                //notify parent that we have a new combined reading
                myOwner->postEvent(EV_FLAG_TASK_NEW_VALUE);
            }

            //events to manage sensor states and faults
            if ((actualEvents & EV_FLAG_TASK_SENSOR_DISCONNECT) == EV_FLAG_TASK_SENSOR_DISCONNECT)
            {
                //notify parent that we have hit a problem and are awaiting next action from higher level functions
                //TODO: In case of pseudo sensors the status may have to be checked to see which of the two sub-sensors
                //caused the fault
                myOwner->postEvent(EV_FLAG_TASK_SENSOR_DISCONNECT);
            }

            if ((actualEvents & EV_FLAG_TASK_SENSOR_PAUSE) == EV_FLAG_TASK_SENSOR_PAUSE)
            {
               //TODO:
            }

            //connection event comes from the sensors - just do both as we don't know the source of the signal
            //if the sensor is not in the right state then the instruction should not do anything
            if ((actualEvents & EV_FLAG_TASK_SENSOR_CONNECT) == EV_FLAG_TASK_SENSOR_CONNECT)
            {
                if (myPrimarySlot != NULL)
                {
                    myPrimarySlot->resume();
                }

                if (mySecondarySlot != NULL)
                {
                    mySecondarySlot->resume();
                }
            }

            //retry instruction comes from function
            if ((actualEvents & EV_FLAG_TASK_SENSOR_RETRY) == EV_FLAG_TASK_SENSOR_RETRY)
            {
                if (myPrimarySlot != NULL)
                {
                    myPrimarySlot->retry();
                }

                if (mySecondarySlot != NULL)
                {
                    mySecondarySlot->retry();
                }
            }
        }
    }

    sensorError = mySensor->close();
}
