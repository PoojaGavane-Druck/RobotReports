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
* @author   Nageswara Rao
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




/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateDuci : public DCommsState
{
private:
    //common commands
    static sDuciError_t fnGetKM(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetKM(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetRE(void *instance, sDuciParameter_t * parameterArray);
   
    
    static sDuciError_t fnGetRI(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetIV(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetIS(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetDK(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetRB(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetPV(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetRF(void *instance, sDuciParameter_t *parameterArray);
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
    DCommsStateDuci(DDeviceSerial *commsMedium, DTask *task);

    virtual void initialise(void);



    virtual eStateDuci_t run(void);
   
    static sDuciError_t fnGetSD(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetST(void *instance, sDuciParameter_t * parameterArray); 
    static sDuciError_t fnGetRV(void *instance, sDuciParameter_t * parameterArray); 
    static sDuciError_t fnGetSN(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetCM(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetCI(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetPT(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetSP(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetCN(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetCS(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetPS(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetCC(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetIZ(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetSC(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetCD(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetRD(void *instance, sDuciParameter_t * parameterArray);
    
    //command handlers for this instance
    virtual sDuciError_t fnGetKM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnSetKM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetRE(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetSN(sDuciParameter_t * parameterArray);   
    virtual sDuciError_t fnGetRI(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetIV(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetIS(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetSD(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetST(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetRV(sDuciParameter_t * parameterArray); 
    virtual sDuciError_t fnGetCM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetDK(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetCI(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetPT(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetSP(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetCN(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetCS(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetPS(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetCC(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetIZ(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetRB(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetSC(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetCD(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetPV(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetRD(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetRF(sDuciParameter_t *parameterArray);
    
};

#endif /* __DCOMMS_STATE_H */
