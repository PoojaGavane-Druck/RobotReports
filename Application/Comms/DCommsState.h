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
* @file     DCommsState.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The comms class header file
*/

#ifndef __DCOMMS_STATE_H
#define __DCOMMS_STATE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdbool.h>
MISRAC_ENABLE

#include "DDeviceSerial.h"
#include "DParse.h"
#include "Types.h"
#include "DTask.h"
/* Defines and constants  -------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/


typedef enum
{
    E_STATE_COMMS_OWNED = 0,
    E_STATE_COMMS_REQUESTED,
    E_STATE_COMMS_RELINQUISHED

} eStateComms_t;



typedef enum
{
    E_STATE_DUCI_LOCAL = 0,
    E_STATE_DUCI_REMOTE,
    E_STATE_DUCI_PROD_TEST,
    E_STATE_DUCI_ENG_TEST,
    E_STATE_DUCI_DATA_DUMP,
    E_STATE_DUCI_SIZE

} eStateDuci_t;
typedef enum
{

    E_COMMS_MASTER_NONE = 0,
    E_COMMS_MASTER_OWI,
    E_COMMS_DUCI_OVER_USB,
    E_COMMS_DUCI_OVER_BLUETOOTH,

} eCommMasterInterfaceType_t;
typedef union
{
    uint32_t all;

    struct
    {
        uint32_t connected      : 1;
        uint32_t supported      : 1;
        uint32_t identified     : 1;
        uint32_t reserved       : 29;
    };

} sDevConnectionStatus_t;

typedef struct
{
    sDevConnectionStatus_t status;
    uint32_t dk;
    sVersion_t version;
    uint32_t serialNumber;

} sExternalDevice_t;

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsState
{
private:

protected:
    DDeviceSerial *myCommsMedium;

    DTask *myTask;

    char *myTxBuffer;

    uint32_t myTxBufferSize;



    uint32_t commandTimeoutPeriod; //time in (ms) to wait for a response to a command
    uint32_t shutdownTimeout;
    uint32_t shutdownTime;

    virtual void createCommands(void);

    void clearRxBuffer(void); //Temporarily overriden - all comms has own buffer which base class could clear


    static eStateComms_t commsOwnership;




public:
    DCommsState(DDeviceSerial *commsMedium, DTask *task);
    virtual ~DCommsState(void);

    static sExternalDevice_t externalDevice;

    virtual void initialise(void);

    virtual void suspend(void);

    virtual void resume(void);

    virtual void cleanup(void);

    virtual eStateDuci_t run(void);

};

#endif /* __DCOMMS_STATE_H */
