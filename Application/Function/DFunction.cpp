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
* @file     DFunction.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     28 April 2020
*
* @brief    The DFunction class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include <stdio.h>
#include <stdint.h>
#include <os.h>
MISRAC_ENABLE

#include "DPV624.h"

#include "DFunction.h"
#include "memory.h"
#include "DProcessFilter.h"
#include "DProcessTare.h"
#include "DProcessMax.h"
#include "DProcessMin.h"
//#include "DProcessAlarmSensorHi.h"
//#include "DProcessAlarmSensorLo.h"
//#include "DProcessAlarmUserHi.h"
//#include "DProcessAlarmUserLo.h"
#include "DUserInterface.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DFunction class constructor
 * @param   void
 * @retval  void
 */
DFunction::DFunction()
: DTask()
{
    OS_ERR os_error;
    myFunction = E_FUNCTION_NONE;
    
    mySlot = NULL;

    //create mutex for resource locking
    char *name = "Func";
    OSMutexCreate(&myMutex, (CPU_CHAR*)name, &os_error);

    if (os_error != (OS_ERR)OS_ERR_NONE)
    {
        //TODO: myStatus.osError = 1u;
    }

    //specify the flags that this function must respond to (add more as necessary in derived class)
    myWaitFlags =   EV_FLAG_TASK_SHUTDOWN |
                    EV_FLAG_TASK_NEW_VALUE |
                    EV_FLAG_TASK_NEW_SETPOINT |
                    EV_FLAG_TASK_SENSOR_CAL_REJECTED |
                    EV_FLAG_TASK_SENSOR_FAULT |
                    EV_FLAG_TASK_SENSOR_CONNECT |
                    EV_FLAG_TASK_SENSOR_DISCONNECT |
                    EV_FLAG_TASK_SENSOR_PAUSE;

    //add processes
    //processes[E_PROCESS_FILTER] = new DProcessFilter(myChannelIndex);
    //processes[E_PROCESS_TARE] = new DProcessTare(myChannelIndex);
    //processes[E_PROCESS_MAXIMUM] = new DProcessMax(myChannelIndex);
    //processes[E_PROCESS_MINIMUM] = new DProcessMin(myChannelIndex);
    //processes[E_PROCESS_SENSOR_ALARM_HI] = new DProcessAlarmSensorHi(myChannelIndex);
    //processes[E_PROCESS_SENSOR_ALARM_LO] = new DProcessAlarmSensorLo(myChannelIndex);
    //processes[E_PROCESS_USER_ALARM_HI] = new DProcessAlarmUserHi(myChannelIndex);
    //processes[E_PROCESS_USER_ALARM_LO] = new DProcessAlarmUserLo(myChannelIndex);

    //real time values
    myReading = 0.0f;                    //processed measurement value
    myPosFullscale = 0.0f;               //Positive fullscale of function sensor
    myNegFullscale = 0.0f;               //Negative fullscale of function sensor
    myResolution = 0.0f;                 //Resolution (accuracy of measurements)
}

/**
 * @brief   Create function slots
 * @param   void
 * @retval  void
 */
void DFunction::createSlots(void)
{
    //each instance overrides this function
}

/**
 * @brief   Get function settings
 * @param   void
 * @retval  void
 */
void DFunction::getSettingsData(void)
{
}

/**
 * @brief   Validate function settings as retrieved from persistent storage
 * @note    Invalid/uninitialised data is set to defaults
 * @param   void
 * @retval  void
 */
void DFunction::validateSettings(void)
{
}

/**
 * @brief   Apply function settings as retrieved from persistent storage
 * @param   void
 * @retval  void
 */
void DFunction::applySettings(void)
{
}

/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */
void DFunction::start(void)
{
    OS_ERR err;

    //get stack area from the memory partition memory block for function tasks
    myTaskStack = (CPU_STK*)OSMemGet((OS_MEM*)&memPartition, (OS_ERR*)&err);

    if (err == (OS_ERR)OS_ERR_NONE)
    {
        //memory block from the partition obtained, so can go ahead and run
        activate(myName, (CPU_STK_SIZE)MEM_PARTITION_BLK_SIZE, (OS_PRIO)5u, (OS_MSG_QTY)10u, &err);
    }
    else
    {
        myTaskStack = NULL;
        //report error
    }
}

void DFunction::runFunction(void)
{
    //this is a while loop that pends on event flags
    bool runFlag = true;
    OS_ERR os_error;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;

    //start my main slot - this is set up by each derived class and cannot be NULL
    mySlot->start();

    //enter while loop
    while (runFlag == true)
    {
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)5000u,
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);
        //check for events
        if (os_error == (OS_ERR)OS_ERR_TIMEOUT)
        {
            //no events
        }
        else if (os_error != (OS_ERR)OS_ERR_NONE)
        {
            //report error
        }
        else
        {
            //check for shutdown first
            if ((actualEvents & EV_FLAG_TASK_SHUTDOWN) == EV_FLAG_TASK_SHUTDOWN)
            {
                //shut down the main slot before exiting
                mySlot->shutdown();
                runFlag = false;
            }
            else
            {
                handleEvents(actualEvents);
            }
        }
    }
}

/**
 * @brief   Run processing
 * @param   void
 * @return  void
 */
void DFunction::runProcessing(void)
{
    //get value of compensated measurement from sensor
    float32_t value;
    mySlot->getValue(E_VAL_INDEX_VALUE, &value);
    setValue(E_VAL_INDEX_VALUE, value);
}


/**
 * @brief   Handle function events
 * @param   event flags
 * @return  void
 */
void DFunction::handleEvents(OS_FLAGS actualEvents)
{
 
    DUserInterface* ui = PV624->userInterface;

    if ((actualEvents & EV_FLAG_TASK_NEW_VALUE) == EV_FLAG_TASK_NEW_VALUE)
    {
        //process and update value and inform UI
        runProcessing();
#ifdef UI_ENABLED
        ui->updateReading(myChannelIndex);
#endif
    }
    if ((actualEvents & EV_FLAG_TASK_SENSOR_DISCONNECT) == EV_FLAG_TASK_SENSOR_DISCONNECT)
    {
        sensorRetry();
#ifdef UI_ENABLED
        ui->sensorDisconnected(myChannelIndex);
#endif
    }
#ifdef UI_ENABLED
    //only if setpoints can change in an automated way (eg, ramp, step, etc)
    if ((actualEvents & EV_FLAG_TASK_NEW_SETPOINT) == EV_FLAG_TASK_NEW_SETPOINT)
    {
        ui->notify(E_UI_MSG_NEW_SETPOINT, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_FAULT) == EV_FLAG_TASK_SENSOR_FAULT)
    {
        ui->notify(E_UI_MSG_SENSOR_FAULT, myChannelIndex);
    }



    if ((actualEvents & EV_FLAG_TASK_SENSOR_PAUSE) == EV_FLAG_TASK_SENSOR_PAUSE)
    {
        ui->sensorPaused(myChannelIndex);
    }
#endif
    if ((actualEvents & EV_FLAG_TASK_SENSOR_CONNECT) == EV_FLAG_TASK_SENSOR_CONNECT)
    {

//update sensor information
        updateSensorInformation();
#ifdef UI_ENABLED
        //inform UI tha we are ready to run
        ui->sensorConnected(myChannelIndex);                     //TODO: Discuss with Simon: which of these two ways to do this? WAY 1
        //ui->notify(E_UI_MSG_SENSOR_CONNECTED, myChannelIndex); //TODO: Discuss with Simon: which of these two ways to do this? WAY 2
#endif
    }
#ifdef UI_ENABLED
    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_REJECTED) == EV_FLAG_TASK_SENSOR_CAL_REJECTED)
    {
        ui->notify(E_UI_MSG_CAL_REJECTED, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DEFAULT) == EV_FLAG_TASK_SENSOR_CAL_DEFAULT)
    {
        ui->notify(E_UI_MSG_CAL_DEFAULT, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DUE) == EV_FLAG_TASK_SENSOR_CAL_DUE)
    {
        ui->notify(E_UI_MSG_CAL_DUE, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DATE) == EV_FLAG_TASK_SENSOR_CAL_DATE)
    {
        ui->notify(E_UI_MSG_CAL_DATE_BAD, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_ZERO_ERROR) == EV_FLAG_TASK_SENSOR_ZERO_ERROR)
    {
        ui->notify(E_UI_MSG_ZERO_ERROR, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_IN_LIMIT) == EV_FLAG_TASK_SENSOR_IN_LIMIT)
    {
        ui->notify(E_UI_MSG_SETPOINT_REACHED, myChannelIndex);
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_NEW_RANGE) == EV_FLAG_TASK_SENSOR_NEW_RANGE)
    {
        ui->notify(E_UI_MSG_AUTO_RANGE, myChannelIndex);
    }
#endif
}

/**
 * @brief   Update sensor information locally
 * @param   void
 * @return  void
 */
void DFunction::updateSensorInformation(void)
{
    if (mySlot != NULL)
    {
#if 0
        DSensor * sensor = mySlot->getSensor();

        if (sensor != NULL)
        {
            DLock is_on(&myMutex);

            myPosFullscale = sensor->getFullScaleMax();
            myNegFullscale = sensor->getFullScaleMin();
            myAbsPosFullscale = sensor->getAbsFullScaleMax();
            myAbsNegFullscale = sensor->getAbsFullScaleMin();
            myResolution = sensor->getResolution();
            myType = sensor->getSensorType();
            sensor->getCalDate((eSensorCalType_t)E_SENSOR_CAL_TYPE_USER,(sDate_t*)&myUserCalibrationDate);
            sensor->getManufactureDate((sDate_t*)&myManufactureDate);
        }
#else
        DLock is_on(&myMutex);
        mySlot->getValue(E_VAL_INDEX_POS_FS, &myPosFullscale);
        mySlot->getValue(E_VAL_INDEX_NEG_FS, &myNegFullscale);
        mySlot->getValue(E_VAL_INDEX_POS_FS_ABS, &myAbsPosFullscale);
        mySlot->getValue(E_VAL_INDEX_NEG_FS_ABS, &myAbsNegFullscale);
        mySlot->getValue(E_VAL_INDEX_RESOLUTION, &myResolution);
        mySlot->getValue(E_VAL_INDEX_USER_CAL_DATE,(sDate_t*)&myUserCalibrationDate);
        mySlot->getValue(E_VAL_INDEX_MANUFACTURING_DATE,(sDate_t*)&myManufactureDate);
        mySlot->getValue(E_VAL_INDEX_SENSOR_TYPE, (uint32_t*)&myType);
        

#endif
    }
}

/**
 * @brief   Clean up after termination of task
 * @param   void
 * @return  void
 */
void DFunction::cleanUp(void)
{
    OS_ERR err;

    //if a stack was allocated then free that memory to the partition
    if (myTaskStack != NULL)
    {
        //Return the stack memory block back to the partition
        OSMemPut((OS_MEM*)&memPartition, (void*)myTaskStack, (OS_ERR*)&err);

        if (err == (OS_ERR)OS_ERR_NONE)
        {
            //memory block from the partition obtained
        }

        myTaskStack = NULL;
    }

    //signal shutdown to UI
#ifdef UI_ENABLE
    PV624->userInterface->functionShutdown(myChannelIndex);
#endif
}

/**
 * @brief   read function output
 * @param   index is function specific
 * @param   pointer for return value of output setpoint
 * @return  true if successful, else false
 */
bool DFunction::getOutput(uint32_t index, float32_t *value)
{
    bool success = false;

    if (mySlot != NULL)
    {
        DSensor * sensor = mySlot->getSensor();

        if (sensor != NULL)
        {
            //index not used - but can be if future need to have multiple outputs on a sensor
            *value = sensor->getOutput();
            success = true;
        }
    }

    return success;
}

/**
 * @brief   Set output for this function
 * @note    This is used by source slots to update setpoint.
 *          Operations that change sensor values must go through the slot and not directly to sensor
 * @param   index is function specific
 * @param   value is the output setpoint
 * @return  true if successful, else false
 */
bool DFunction::setOutput(uint32_t index, float32_t value)
{
    return false;
}

/**
 * @brief   Get Value
 * @param   index is function/sensor specific
 * @param   pointer to variable for return value of compensated and processed measurement value in selects user units
 * @return  true if successful, else false
 */
bool DFunction::getValue(eValueIndex_t index, float32_t *value)
{
    bool successFlag = false;

    if (mySlot != NULL)
    {
        DLock is_on(&myMutex);
        successFlag = true;

        switch (index)
        {
            case E_VAL_INDEX_VALUE:    //index 0 = processed value
                *value = myReading;
                break;

            case E_VAL_INDEX_POS_FS:    //index 1 = positive FS
                *value = myPosFullscale;
                break;

            case E_VAL_INDEX_NEG_FS:    //index 2 = negative FS
                *value = myNegFullscale;
                break;
            case E_VAL_INDEX_POS_FS_ABS:                
                  *value = myAbsPosFullscale;
                break;

            case E_VAL_INDEX_NEG_FS_ABS:                
                   *value = myAbsNegFullscale;
                break;

            case E_VAL_INDEX_RESOLUTION:    //index 3 = resolution
                *value = myResolution;
                break;

            default:
                successFlag = false;
                break;
        }        
    }

    return successFlag;
}

/**
 * @brief   Signal sensor to continue/resume
 * @param   void
 * @retval  true if all's well, else false
 */
bool DFunction::sensorContinue(void)
{
    bool success = false;

    if (mySlot != NULL)
    {
        //apply the user settings for this function
        applySettings();

        //and go!
        mySlot->resume();
        success = true;
    }

    return success;
}

/**
 * @brief   Signal sensor to retry after failure reported
 * @param   void
 * @retval  true if all's well, else false
 */
bool DFunction::sensorRetry(void)
{
    bool success = false;

    if (mySlot != NULL)
    {
        mySlot->retry();
        success = true;
    }

    return success;
}



/**
 * @brief   Set processed measurement value
 * @param   value of processed measurement value
 * @retval  void
 */
void DFunction::setReading (float32_t value)
{
    DLock is_on(&myMutex);
    myReading = value;
}

/**
 * @brief   Get positive fullscale of function sensor
 * @param   void
 * @retval  value of positive fullscale of function sensor
 */
float32_t DFunction::getPosFullscale(void)
{
    DLock is_on(&myMutex);
    return myPosFullscale;
}

/**
 * @brief   Set
 * @param   value of positive fullscale of function sensor
 * @retval  void
 */
void DFunction::setPosFullscale(float32_t value)
{
    DLock is_on(&myMutex);
    myPosFullscale = value;
}

/**
 * @brief   Get negative fullscale of function sensor
 * @param   void
 * @retval  value of negative fullscale of function sensor
 */
float32_t DFunction::getNegFullscale(void)
{
    DLock is_on(&myMutex);
    return myNegFullscale;
}

/**
 * @brief   Set negative fullscale of function sensor
 * @param   value of negative fullscale of function sensor
 * @retval  void
 */
void DFunction::setNegFullscale(float32_t value)
{
    DLock is_on(&myMutex);
    myNegFullscale = value;
}

/**
 * @brief   Get absolute positive fullscale of function sensor
 * @param   void
 * @retval  value of absolute positive fullscale of function sensor
 */
float32_t DFunction::getAbsPosFullscale(void)
{
    DLock is_on(&myMutex);
    return myAbsPosFullscale;
}

/**
 * @brief   Set
 * @param   value of absolute positive fullscale of function sensor
 * @retval  void
 */
void DFunction::setAbsPosFullscale(float32_t value)
{
    DLock is_on(&myMutex);
    myAbsPosFullscale = value;
}

/**
 * @brief   Get abslolute negative fullscale of function sensor
 * @param   void
 * @retval  value of absolute negative fullscale of function sensor
 */
float32_t DFunction::getAbsNegFullscale(void)
{
    DLock is_on(&myMutex);
    return myAbsNegFullscale;
}

/**
 * @brief   Set abslolute negative fullscale of function sensor
 * @param   value of absolute negative fullscale of function sensor
 * @retval  void
 */
void DFunction::setAbsNegFullscale(float32_t value)
{
    DLock is_on(&myMutex);
    myAbsNegFullscale = value;
}

/**
 * @brief   Get resolution (accuracy of measurements)
 * @param   void
 * @retval  value of resolution (accuracy of measurements)
 */
float32_t DFunction::getResolution(void)
{
    DLock is_on(&myMutex);
    return myResolution;
}

/**
 * @brief   Set resolution (accuracy of measurements)
 * @param   value of resolution (accuracy of measurements)
 * @retval  void
 */
void DFunction::setResolution(float32_t value)
{
    DLock is_on(&myMutex);
    myResolution = value;
}


/**
 * @brief   Get sensor Type
 * @param   void
 * @retval  Sensor Type
 */
eSensorType_t DFunction::getSensorType(void)
{
    DLock is_on(&myMutex);
    return myType;
}

void DFunction::getManufactureDate(sDate_t *pManfDate)  
{
   DLock is_on(&myMutex);
   *pManfDate = myManufactureDate;
   
}

void DFunction::getCalDate(eSensorCalType_t caltype, sDate_t* pCalDate)
{
   DLock is_on(&myMutex);
     if( (eSensorCalType_t)E_SENSOR_CAL_TYPE_USER == caltype)
    {
      *pCalDate = myUserCalibrationDate;
    }
    else if((eSensorCalType_t)E_SENSOR_CAL_TYPE_FACTORY== caltype)
    {
      *pCalDate = myFactoryCalibrationDate;
    }
    else
    {
      /*DO Nothining. Added for Misra*/
    }
  
}


 bool DFunction::setFunction(eFunction_t func)
 {
    bool successFlag = true;
    if(func >= (eFunction_t)E_FUNCTION_MAX)
    {
      successFlag = false;
    }
    else
    {
      myFunction = func;
    }
    return successFlag;
 }
 
 eFunction_t DFunction::getFunction(void)
 {
   return myFunction;
 }
    
 bool getBarometerIdentity( uint32_t *identity)
 {
   return true;
 }
 
 /**
 * @brief   Set floating point value
 * @param   index is function/sensor specific value identifier
 * @param   value to set
 * @param   userUnits is true if value is specified in user units, false if base units
 * @return  true if successful, else false
 */
bool DFunction::setValue(eValueIndex_t index, float32_t value)
{
    bool successFlag = false;

    if (mySlot != NULL)
    {
        DLock is_on(&myMutex);
        successFlag = true;

        switch (index)
        {
            case E_VAL_INDEX_VALUE:               
                    myReading = value;
               break;

            case E_VAL_INDEX_POS_FS:
                     myPosFullscale = value;
                break;

            case E_VAL_INDEX_NEG_FS:                
                    myNegFullscale = value;
                break;

            case E_VAL_INDEX_POS_FS_ABS:
                    myAbsPosFullscale = value;
                break;

            case E_VAL_INDEX_NEG_FS_ABS:
                   myAbsNegFullscale = value;
                break;

            case E_VAL_INDEX_RESOLUTION:
                   myResolution = value;
                break;

            default:
                successFlag = false;
                break;
        }
    }

    return successFlag;
}
