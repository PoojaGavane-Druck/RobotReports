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

#ifndef __DCOMMS_OPERATION_MODE_H
#define __DCOMMS_OPERATION_MODE_H


/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsState.h"
//#include "Duci.h"
//#defined

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/
#define ___SINGLETON
class DCommsOperationMode :
{
private:
    static DCommsStateWrite *myInstance;

#ifdef ___SINGLETON
    DCommsStateRemote(DDeviceSerial *commsMedium);
#endif

    //call back functions must be declared as static -each has an instance version below (in the public methods)
    static sDuciError_t fnSetKP(void *instance, sDuciParameter_t * parameterArray);
    static sDuciError_t fnGetRI(void *instance, sDuciParameter_t * parameterArray);

protected:
    virtual void createDuciCommands(void);

public:
    //public methods
#ifdef ___SINGLETON
    static DCommsStateRemote *getInstance(void) //singleton pattern
    {
        if (myInstance == NULL)
        {
            myInstance = new DCommsStateRemote(NULL);
        }

        return myInstance;
    }
#else
    DCommsStateRemote(DDeviceSerial *commsMedium);
#endif

    DDeviceSerial *getCommsMedium(void);
    bool setCommsMedium(DDeviceSerial *commsMedium);

    virtual eStateDuci_t run(void);

    //command handlers for this instance
    sDuciError_t fnGetKM(sDuciParameter_t * parameterArray);
    sDuciError_t fnSetKM(sDuciParameter_t * parameterArray);

    sDuciError_t fnSetKP(sDuciParameter_t * parameterArray);

    sDuciError_t fnGetRI(sDuciParameter_t * parameterArray);
};

#endif /* __DCOMMS_STATE_REMOTE_H */
