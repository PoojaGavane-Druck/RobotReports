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
//#include "Duci.h"

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/
//#define ___SINGLETON
class DCommsStateRemote : public DCommsStateDuci
{
private:
    static DCommsStateRemote *myInstance;

   DCommsStateRemote(DDeviceSerial *commsMedium, DTask *task);

    //call back functions must be declared as static -each has an instance version below (in the public methods)
    static sDuciError_t fnSetKP(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetSF(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetSP(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetST(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetSD(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetSN(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnSetCM(void *instance, sDuciParameter_t * parameterArray);

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


    sDuciError_t fnSetKP(sDuciParameter_t * parameterArray);
    sDuciError_t fnSetSF(sDuciParameter_t * parameterArray);
    sDuciError_t fnGetSP(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetST(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetSD(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetSN(sDuciParameter_t * parameterArray);
    sDuciError_t fnSetCM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnGetKM(sDuciParameter_t * parameterArray);
    virtual sDuciError_t fnSetKM(sDuciParameter_t * parameterArray);
    
    
};

#endif /* __DCOMMS_STATE_REMOTE_H */
