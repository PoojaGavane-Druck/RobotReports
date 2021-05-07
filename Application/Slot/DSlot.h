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
* @file     DSlot.h
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DSlot base class header file
*/

#ifndef _DSLOT_H
#define _DSLOT_H

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//MISRAC_ENABLE

#include "DTask.h"
#include "DSensor.h"

/* Defines ----------------------------------------------------------------------------------------------------------*/
/*  TASK FLAGS, NOTE:
    -----------------
    An event flags group has one bit per event. There are up to 32 event flags in a group.
    The lower 16 bits are reserved for global system/wide events that all DTask instances may pend on
    Only the higher 16 bits are specified here. ** DO NOT SPECIFY BITS 0-15 HERE **
*/
//Only DSlot and its derived classes may pend on these messages
#define EV_FLAG_TASK_SLOT_FREE_1                0x00010000u //Slot use only: available for use if needed
#define EV_FLAG_TASK_SLOT_FREE_2                0x00020000u //Slot use only: available for use if needed
#define EV_FLAG_TASK_SLOT_FREE_3                0x00040000u //Slot use only: available for use if needed
#define EV_FLAG_TASK_SLOT_FREE_4                0x00080000u //Slot use only: available for use if needed
#define EV_FLAG_TASK_SLOT_FREE_5                0x00100000u //Slot use only: available for use if needed
#define EV_FLAG_TASK_SLOT_SENSOR_CONTINUE       0x00200000u //Slot use only: go-ahead signal for sensor to resume after a pause
#define EV_FLAG_TASK_SLOT_SYNCHRONISE           0x00400000u //Slot use only: signal for a synchronised measurement
#define EV_FLAG_TASK_SLOT_SENSOR_RETRY          0x00800000u //Slot use only: go-ahead signal for sensor to re-start/retry after, for example after a fault
#define EV_FLAG_TASK_SLOT_CAL_SET_TYPE          0x01000000u //Slot use only: set calibration type
#define EV_FLAG_TASK_SLOT_CAL_START_SAMPLING    0x02000000u //Slot use only: start cal sampling
#define EV_FLAG_TASK_SLOT_CAL_SET_POINT         0x04000000u //Slot use only: apply cal point value
#define EV_FLAG_TASK_SLOT_CAL_ACCEPT            0x08000000u //Slot use only: accept calibration
#define EV_FLAG_TASK_SLOT_CAL_ABORT             0x10000000u //Slot use only: abort calibration
#define EV_FLAG_TASK_SLOT_CAL_SET_DATE          0x20000000u //Slot use only: set calibration date
#define EV_FLAG_TASK_SLOT_CAL_SET_INTERVAL      0x40000000u //Slot use only: set calibration interval
#define EV_FLAG_TASK_SLOT_CAL_RELOAD            0x80000000u //Slot use only: reload calibration data

/* Types ------------------------------------------------------------------------------------------------------------*/
typedef enum : uint32_t
{
    E_SENSOR_STATUS_DISCOVERING = 0u,
    E_SENSOR_STATUS_IDENTIFYING,
    E_SENSOR_STATUS_READY,
    E_SENSOR_STATUS_RUNNING,
    E_SENSOR_STATUS_DISCONNECTED

} eSensorStatus_t;

/* Variables --------------------------------------------------------------------------------------------------------*/

class DSlot : public DTask
{
protected:
    OS_FLAGS myWaitFlags;                   //events (flags) to which the function will respond
    OS_MUTEX myMutex;
    OS_TICK mySampleInterval;               //currently active interval (period) between measurements
    OS_TICK myDefaultSampleInterval;        //interval (period) between measurements in normal mode
    OS_TICK myCalSampleInterval;            //interval (period) between measurements when calibrating
    OS_TICK myMinSampleInterval;            //minimum interval (period) between measurements
//    OS_TICK myMaxSampleInterval;            //maximum interval (period) between measurements

    uint32_t mySampleTime;                  //tick count at last measurement
    uint32_t mySyncTime;                    //tick count at the point of time of synchronisation event
    uint32_t mySensorLatency;               //this is the time in milliseconds that it takes to take a measurement
    DSensor *mySensor;
    DTask *myOwner;

    uint32_t myCalInterval;                 //cal interval in days - used as a mailbox for communicating between slot and sensor
    sDate_t myCalDate;                      //last user cal date - used as a mailbox for communicating between slot and sensor

    uint32_t myCalType;                     //sensor calibration type
    uint32_t myCalRange;                    //sensor range being calibrated
    uint32_t myCalPointIndex;               //calibration point number used when performing sensor calibration adjustment
    float32_t myCalPointValue;              //calibration point actual value used when performing sensor calibration adjustment
    uint32_t myCalSamplesRemaining;         //calibration samples remaining
    
    eSensorStatus_t myState;

    void updateSensorStatus(eSensorError_t sensorError);
    
    virtual eSensorError_t handleCalibrationEvents(OS_FLAGS actualEvents);
    
    virtual bool sensorSetCalibrationType(void);
    virtual bool sensorStartCalSampling(void);
    virtual bool sensorGetCalSamplesRemaining(void);
    virtual bool sensorSetCalPoint(void);
    virtual bool sensorAcceptCalibration(void);
    virtual bool sensorAbortCalibration(void);
public:
    DSlot(DTask *owner);

    virtual void start(void);
    virtual void runFunction(void);
    virtual void cleanUp(void);

    DSensor *getSensor(void);
    virtual bool getValue(eValueIndex_t index, float32_t *value);    //get specified floating point function value
    virtual bool setValue(eValueIndex_t index, float32_t value);     //set specified floating point function value

    virtual bool getValue(eValueIndex_t index, uint32_t *value);    //get specified integer function value
    virtual bool setValue(eValueIndex_t index, uint32_t value);     //set specified integer function value
    
    virtual bool getValue(eValueIndex_t index, sDate_t* date);    //get specified integer function value

    void pause(void);
    void resume(void);
    void synchronise(void);
    void retry(void);

    //functions that must go through the slot and not directly to sensor
    bool setCalibrationType(int32_t calType, uint32_t range);
    bool getRequiredNumCalPoints(uint32_t *numCalPoints);
    bool setRequiredNumCalPoints(uint32_t numCalPoints);
    bool startCalSampling(void);
    virtual bool getCalSamplesRemaining(uint32_t *samples);
    bool setCalPoint(uint32_t calPoint, float32_t value);
    bool acceptCalibration(void);
    bool abortCalibration(void);

    bool setSampleInterval(uint32_t interval);
    uint32_t getSampleInterval(void);

 
    
    virtual bool setOutput(uint32_t index, float32_t value);

};

#endif // _DSLOT_H
