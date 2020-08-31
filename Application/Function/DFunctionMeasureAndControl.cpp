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
* @file     DFunctionMeasureAddExtBaro.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     02 June 2020
*
* @brief    The DFunctionMeasureAddExtBaro class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
//#include "misra.h"
//
//MISRAC_DISABLE
//#include <stdio.h>
//#include <os.h>
//MISRAC_ENABLE
//
//#include "DDPI610E.h"

#include "DFunctionMeasureAndControl.h"
#include "DSlotMeasurePressureExt.h"
#include "DSlotMeasureBarometer.h"
#include "DPV624.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DFunctionMeasureAndControl class constructor
 * @param   void
 * @retval  void
 */
DFunctionMeasureAndControl::DFunctionMeasureAndControl()
: DFunctionMeasure()
{
    myName = "fExtAndBaro";
    myFunction = E_FUNCTION_EXT_PRESSURE;

    //create the slots as appropriate for the instance
    createSlots();
    start();
    //events in addition to the default ones in the base class
    //myWaitFlags |= EV_FLAG_TASK_SENSOR_???;
}

/**
 * @brief   Create function slots
 * @param   void
 * @retval  void
 */
void DFunctionMeasureAndControl::createSlots(void)
{

    mySlot = new DSlotMeasurePressureExt(this);

    myBarometerSlot = new DSlotMeasureBarometer(this);

}
/**
 * @brief   Run processing
 * @param   void
 * @return  void
 */
void DFunctionMeasureAndControl::runProcessing(void)
{
   DFunction::runProcessing();  
  //get value of compensated measurement from Barometer sensor
   float32_t value;
   float32_t barometerReading;
   if(NULL != myBarometerSlot)
   {
      mySlot->getValue(E_VAL_INDEX_VALUE, &value);
      myBarometerSlot->getValue(E_VAL_INDEX_VALUE, &barometerReading);
      
      setValue(E_VAL_INDEX_BAROMETER_VALUE, barometerReading);
      
      switch(myType)
       {
          case E_SENSOR_TYPE_PRESS_ABS:           
            setValue(EVAL_INDEX_PSEUDO_ABS, value);
            setValue(EVAL_INDEX_PSEUDO_GAUGE, (value - barometerReading));
          break;
          case E_SENSOR_TYPE_PRESS_GAUGE:  /*2 - gauge pressure sensor */
            setValue(EVAL_INDEX_PSEUDO_GAUGE, value);
            setValue(EVAL_INDEX_PSEUDO_ABS, (value + barometerReading));
            
          break;
          
          default:        
          break;
      }
      
   }
   
}

/**
 * @brief   Get function settings
 * @param   void
 * @retval  void
 */
void DFunctionMeasureAndControl::getSettingsData(void)
{

}

void DFunctionMeasureAndControl::runFunction(void)
{
  //this is a while loop that pends on event flags
    bool runFlag = true;
    OS_ERR os_error = OS_ERR_NONE;
    CPU_TS cpu_ts;
    OS_FLAGS actualEvents;

    //start my main slot - this is set up by each derived class and cannot be NULL
    if (mySlot != NULL)
    {
        mySlot->start();
    }
    
    if(myBarometerSlot != NULL)
    {
      myBarometerSlot->start();
    }

    while (runFlag == true)
    {
        actualEvents = OSFlagPend(  &myEventFlags,
                                    myWaitFlags, (OS_TICK)500u,
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);

        bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT));

        if(!ok)
        {
    MISRAC_DISABLE
#ifdef ASSERT_ENABLED
            assert(false);
#endif
    MISRAC_ENABLE
            error_code_t errorCode;
            errorCode.bit.osError = SET;
            PV624->errorHandler->handleError(errorCode, os_error);
        }

        //check for events
        if (ok)
        {
            //check for shutdown first
            if ((actualEvents & EV_FLAG_TASK_SHUTDOWN) == EV_FLAG_TASK_SHUTDOWN)
            {
                //shut down the main slot before exiting
                if (mySlot != NULL)
                {
                    mySlot->shutdown();
                }
                
                if(myBarometerSlot != NULL)
                {
                  myBarometerSlot->shutdown();
                }

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
 * @brief   Get Value
 * @param   index is function/sensor specific
 * @param   pointer to variable for return value of compensated and processed measurement value in selects user units
 * @return  true if successful, else false
 */
bool DFunctionMeasureAndControl::getValue(eValueIndex_t index, float32_t *value)
{
   
    bool successFlag = false;

    if (mySlot != NULL)
    {
        DLock is_on(&myMutex);
        successFlag = true;

        switch (index)
        {
            case E_VAL_INDEX_VALUE:    //index 0 = processed value
                if((eFunction_t)E_FUNCTION_EXT_PRESSURE == myFunction)
                {
                  *value = myReading;
                }
                else if((eFunction_t)E_FUNCTION_PSEUDO_ABS == myFunction)
                {
                  *value = myPseudoAbsoluteReading;
                }
                else if((eFunction_t)(eFunction_t)E_FUNCTION_PSEUDO_GAUGE == myFunction)
                {
                  *value = myPseudoGaugeReading;
                }
                else if((eFunction_t)E_FUNCTION_BAROMETER == myFunction)
                {
                  *value = myBarometerReading;
                }
                else
                {
                  *value = myReading;
                }
                break;

            case E_VAL_INDEX_POS_FS:    //index 1 = positive FS
                *value = getPosFullscale();
                break;

            case E_VAL_INDEX_NEG_FS:    //index 2 = negative FS
                *value = getNegFullscale();
                break;
            case E_VAL_INDEX_POS_FS_ABS:                
                 *value = myAbsPosFullscale ;
                break;

            case E_VAL_INDEX_NEG_FS_ABS:                
                   *value =   myAbsNegFullscale;
                break;

            case E_VAL_INDEX_RESOLUTION:    //index 3 = resolution
                *value = getResolution();
                break;

            default:
                successFlag = false;
                break;
        }
        if((false == successFlag) && (NULL != myBarometerSlot))
        {
          switch (index)
          {
            case E_VAL_INDEX_BAROMETER_VALUE:
                *value = myBarometerReading;
                break;

            default:
                successFlag = false;
                break;
          }
        }
    }

    return successFlag;
}


/**
 * @brief   Set floating point value
 * @param   index is function/sensor specific value identifier
 * @param   value to set
 * @return  true if successful, else false
 */
bool DFunctionMeasureAndControl::setValue(eValueIndex_t index, float32_t value)
{
  bool successFlag = false;

    successFlag = DFunction::setValue(index, value);
    if((false == successFlag) && (NULL != myBarometerSlot))
    {
      DLock is_on(&myMutex);
      successFlag = true;
      if(NULL != mySlot)
      {
        switch (index)
        {
          case EVAL_INDEX_PSEUDO_GAUGE:
              myPseudoGaugeReading = value;
              break;
              
          case EVAL_INDEX_PSEUDO_ABS:
              myPseudoAbsoluteReading = value;
              break;
              
          default:
              successFlag = false;
              break;
        }
      }
      
      if(NULL != myBarometerSlot)
      {
        switch (index)
        {
          case E_VAL_INDEX_BAROMETER_VALUE:
              myBarometerReading = value;
              break;         

          default:
              successFlag = false;
              break;
        }
      }
    }
    

    return successFlag;
}



/**
 * @brief   Handle function events
 * @param   event flags
 * @return  void
 */
void DFunctionMeasureAndControl::handleEvents(OS_FLAGS actualEvents)
{
   

    if ((actualEvents & EV_FLAG_TASK_NEW_VALUE) == EV_FLAG_TASK_NEW_VALUE)
    {
        //process and update value and inform UI
        runProcessing();

        //TODo: Screw Controler calls starts here 
    }

    //only if setpoints can change in an automated way (eg, ramp, step, etc)
    if ((actualEvents & EV_FLAG_TASK_NEW_SETPOINT) == EV_FLAG_TASK_NEW_SETPOINT)
    {
        //ToDo: Need to implement   
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_FAULT) == EV_FLAG_TASK_SENSOR_FAULT)
    {
        //ToDo: Need to implement 
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_DISCONNECT) == EV_FLAG_TASK_SENSOR_DISCONNECT)
    {
       //Todo Notify Error Handler
       //Todo Update LED Status
       sensorRetry();
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_PAUSE) == EV_FLAG_TASK_SENSOR_PAUSE)
    {
       //Todo Notify Error Handler
       //Todo Update LED Status
        
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CONNECT) == EV_FLAG_TASK_SENSOR_CONNECT)
    {
        //update sensor information
        updateSensorInformation();

       //Todo Notify Error Handler
       //Todo Update LED Status
        sensorContinue();
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_REJECTED) == EV_FLAG_TASK_SENSOR_CAL_REJECTED)
    {
        //ToDO: Need to implement
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DEFAULT) == EV_FLAG_TASK_SENSOR_CAL_DEFAULT)
    {
        //ToDO: Need to implement
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DUE) == EV_FLAG_TASK_SENSOR_CAL_DUE)
    {
        //ToDO: Need to implement
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DATE) == EV_FLAG_TASK_SENSOR_CAL_DATE)
    {
       //ToDO: Need to implement
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_ZERO_ERROR) == EV_FLAG_TASK_SENSOR_ZERO_ERROR)
    {
        //ToDO: Need to implement
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_IN_LIMIT) == EV_FLAG_TASK_SENSOR_IN_LIMIT)
    {
       //ToDO: Need to implement
    }

    if ((actualEvents & EV_FLAG_TASK_SENSOR_NEW_RANGE) == EV_FLAG_TASK_SENSOR_NEW_RANGE)
    {
        //update sensor information as range change may change resolution and no of decimal points
        updateSensorInformation();

        
    }
}