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
* @file     DCommsStateProdTest.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     01 April 2020
*
* @brief    The comms production test state class header file
*/

#ifndef __DCOMMS_STATE_PROD_TEST_H
#define __DCOMMS_STATE_PROD_TEST_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DCommsStateDuci.h"

#ifdef PRODUCTION_TEST_BUILD
#include "DProductionTest.h"
#endif

/* Types ------------------------------------------------------------------------------------------------------------*/

/* Variables -------------------------------------------------------------------------------------------------------*/

class DCommsStateProdTest : public DCommsStateDuci
{
#ifdef PRODUCTION_TEST_BUILD
private:
    DProductionTest *myProductionTest;

    static sDuciError_t fnGetKP(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetKP(void *instance, sDuciParameter_t *parameterArray);

    static sDuciError_t fnGetSD(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetSD(void *instance, sDuciParameter_t *parameterArray);

    static sDuciError_t fnGetST(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetST(void *instance, sDuciParameter_t *parameterArray);

    static sDuciError_t fnSetTM(void *instance, sDuciParameter_t *parameterArray);

    static sDuciError_t fnGetTP(void *instance, sDuciParameter_t *parameterArray);
    static sDuciError_t fnSetTP(void *instance, sDuciParameter_t *parameterArray);

    static sDuciError_t fnGetUI(void *instance, sDuciParameter_t *parameterArray);

    //command handlers for this instance
    sDuciError_t fnGetKM(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetKM(sDuciParameter_t *parameterArray);

    sDuciError_t fnGetKP(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetKP(sDuciParameter_t *parameterArray);

    sDuciError_t fnGetSD(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetSD(sDuciParameter_t *parameterArray);

    sDuciError_t fnGetST(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetST(sDuciParameter_t *parameterArray);

    sDuciError_t fnSetTM(sDuciParameter_t *parameterArray);

    sDuciError_t fnGetTP(sDuciParameter_t *parameterArray);
    sDuciError_t fnSetTP(sDuciParameter_t *parameterArray);

    sDuciError_t fnGetUI(sDuciParameter_t *parameterArray);
#endif

protected:
    virtual void createDuciCommands(void);

public:
    DCommsStateProdTest(DDeviceSerial *commsMedium, DTask *task);

    virtual eCommOperationMode_t run(void);
};

#endif /* __DCOMMS_STATE_PROD_TEST_H */
