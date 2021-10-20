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
* @file     DCommsStateRemote.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The comms remote state class header file
*/

#ifndef __DCOMMS_STATE_REMOTE_H
#define __DCOMMS_STATE_REMOTE_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDuci.h"


/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateRemote : public DCommsStateDuci
{
private:
    static DCommsStateRemote *myInstance;

   DCommsStateRemote(DDeviceSerial *commsMedium, DTask *task);

    //call back functions must be declared as static -each has an instance version below (in the public methods)
    static sDuciError_t fnSetKP(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetPT(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetST(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetSD(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetSN(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetCM(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetCI(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetSP(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetCT(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetCS(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetCP(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetCA(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetCX(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetCN(void *instance, sDuciParameter_t * parameterArray);
    
    static sDuciError_t fnSetUF(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetIZ(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetCB(void *instance, sDuciParameter_t *parameterArray);
    
    static sDuciError_t fnSetCD(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnGetPP(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetPP(void *instance, sDuciParameter_t *parameterArray);
    
    static sDuciError_t fnSetRD(void *instance, sDuciParameter_t *parameterArray);
protected:
   
  virtual void createCommands(void);
    
public:
    
    //public methods
    static DCommsStateRemote *getInstance(void) //singleton pattern
    {
        if (myInstance == NULL)
        {
            myInstance = new DCommsStateRemote(NULL, NULL);
        }

        return myInstance;
    }

    DDeviceSerial *getCommsMedium(void);
    bool setCommsMedium(DDeviceSerial *commsMedium);

    void setMyTask(DTask *task);
     
    virtual eStateDuci_t run(void);

    //command handlers for this instance
    static sDuciError_t fnSetSC(void *instance, sDuciParameter_t *parameterArray);

    sDuciError_t fnSetKP(sDuciParameter_t * parameterArray);
    sDuciError_t fnSetPT(sDuciParameter_t * parameterArray);    
    sDuciError_t fnSetST(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetSD(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetSN(sDuciParameter_t * parameterArray);
    sDuciError_t fnSetCM(sDuciParameter_t * parameterArray);
    sDuciError_t fnSetCI(sDuciParameter_t * parameterArray);
    sDuciError_t fnSetSP(sDuciParameter_t * parameterArray);
    sDuciError_t fnSetCT(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetCS(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetCP(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetCA(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetCX(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetCN(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetSC(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetUF(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetIZ(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetCB(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetCD(sDuciParameter_t *parameterArray);
    sDuciError_t fnGetPP(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetPP(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetRD(sDuciParameter_t *parameterArray);
    virtual sDuciError_t fnGetKM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnSetKM(sDuciParameter_t * parameterArray);
    
    
};

#endif /* __DCOMMS_STATE_REMOTE_H */
