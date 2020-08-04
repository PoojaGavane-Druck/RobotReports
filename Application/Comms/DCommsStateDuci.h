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
* @file     DCommsState.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The comms class header file
*/

#ifndef __DCOMMS_STATE_DUCI_H
#define __DCOMMS_STATE_DUCI_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdbool.h>
MISRAC_ENABLE

#include "DCommsState.h"
#include "DDeviceSerial.h"
#include "DParse.h"
#include "Types.h"

/* Defines and constants  -------------------------------------------------------------------------------------------*/

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    E_STATE_DUCI_LOCAL = 0,
    E_STATE_DUCI_EXTERNAL,
    E_STATE_DUCI_REMOTE,
    E_STATE_DUCI_PROD_TEST,
    E_STATE_DUCI_DEVICE_DISCOVERY,
    E_STATE_DUCI_SIZE

} eStateDuci_t;



/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateDuci : public DCommsState
{
private:
    //common commands
    static sDuciError_t fnGetKM(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetKM(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetRE(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetSN(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetSN(void *instance, sDuciParameter_t * parameterArray);

    //bool prepareMessage(char *str);

protected:
     
    DParse *myParser;
    
    eStateDuci_t nextState;
    sDuciError_t errorStatusRegister;

    virtual void createCommands(void);

  
    bool sendString(char *str);  //TODO: Extend this to have more meaningful returned status
    bool receiveString(char **pStr); //TODO: Extend this to have more meaningful returned status
    bool query(char *str, char **pStr);
 

public:
    DCommsStateDuci(DDeviceSerial *commsMedium);

    virtual void initialise(void);



    virtual eCommOperationMode_t run(void);
   

    //command handlers for this instance
    virtual sDuciError_t fnGetKM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnSetKM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetRE(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetSN(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnSetSN(sDuciParameter_t * parameterArray);
};

#endif /* __DCOMMS_STATE_H */
