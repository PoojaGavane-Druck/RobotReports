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
#include "uart.h"

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
    myMode = E_CONTROLLER_MODE_MEASURE;
    myNewMode = E_CONTROLLER_MODE_MEASURE;
    myStatus.bytes = (uint32_t)0;
    //create the slots as appropriate for the instance
    createSlots();
    start();
    //events in addition to the default ones in the base class
    myWaitFlags |= EV_FLAG_TASK_NEW_CONTROLLER_MODE_RECIEVED |EV_FLAG_TASK_NEW_SET_POINT_RECIEVED;
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
   float32_t value = 0.0f;
   float32_t barometerReading = 0.0f;
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
            errorCode.bytes = 0u;
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
            case E_VAL_INDEX_SENSOR_POS_FS:        //positive full scale
              if((eFunction_t)E_FUNCTION_BAROMETER == myFunction)
              {
                myBarometerSlot->getValue(E_VAL_INDEX_POS_FS,value);
              }
              else
              {
                mySlot->getValue(E_VAL_INDEX_POS_FS,value);
              }
              break;
            case E_VAL_INDEX_SENSOR_NEG_FS:          //negative full scale
              if((eFunction_t)E_FUNCTION_BAROMETER == myFunction)
              {
                myBarometerSlot->getValue(E_VAL_INDEX_NEG_FS,value);
              }
              else
              {
                mySlot->getValue(E_VAL_INDEX_NEG_FS,value);
              } 
              break;
              
            case E_VAL_INDEX_PRESSURE_SETPOINT:
              *value = myCurrentPressureSetPoint;
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
                successFlag = true;
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
          
          case E_VAL_INDEX_PRESSURE_SETPOINT:
            myCurrentPressureSetPoint = value;
            postEvent(EV_FLAG_TASK_NEW_SET_POINT_RECIEVED);
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
   if(EV_FLAG_TASK_NEW_CONTROLLER_MODE_RECIEVED  == (actualEvents & EV_FLAG_TASK_NEW_CONTROLLER_MODE_RECIEVED))
   {
     myMode = myNewMode;
   }

   if(EV_FLAG_TASK_NEW_SET_POINT_RECIEVED == (actualEvents &EV_FLAG_TASK_NEW_SET_POINT_RECIEVED))
   {
     //ToDO: Handle new set point
   }
    if ((actualEvents & EV_FLAG_TASK_NEW_VALUE) == EV_FLAG_TASK_NEW_VALUE)
    {
        //process and update value and inform UI
        runProcessing();
        disableSerialPortTxLine(UART_PORT3);          
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

bool DFunctionMeasureAndControl::getValue(eValueIndex_t index, uint32_t *value)
{
    bool successFlag = false;

    if ((mySlot != NULL) && (NULL != myBarometerSlot))
    {
        DLock is_on(&myMutex);
        successFlag = true;

        switch (index)
        {
            case E_VAL_INDEX_CONTROLLER_MODE:
              *value = (uint32_t)myMode;
              break;
              
            case E_VAL_INDEX_CONTROLLER_STATUS:
              *value = (uint32_t)myStatus.bytes;
              break;
              
            case E_VAL_INDEX_PM620_APP_IDENTITY:  
            case E_VAL_INDEX_PM620_BL_IDENTITY:
              mySlot->getValue(index,value);        
              successFlag = true;
              break;
            case E_VAL_INDEX_SENSOR_TYPE:         //positive full scale
              if((eFunction_t)E_FUNCTION_BAROMETER == myFunction)
              {
                myBarometerSlot->getValue(E_VAL_INDEX_SENSOR_TYPE,value);
              }
              else
              {
                mySlot->getValue(E_VAL_INDEX_SENSOR_TYPE,value);
              } 
              break;
            case E_VAL_INDEX_CAL_INTERVAL:
              if((eFunction_t)E_FUNCTION_BAROMETER == myFunction)
              {
                myBarometerSlot->getValue(E_VAL_INDEX_CAL_INTERVAL,value);
              }
              else
              {
                mySlot->getValue(E_VAL_INDEX_CAL_INTERVAL,value);
              } 
            break;
            case EVAL_INDEX_BAROMETER_ID:
              myBarometerSlot->getValue(EVAL_INDEX_SENSOR_MANF_ID,value);        
              successFlag = true;
            break;
            case EVAL_INDEX_PM620_ID:    //index 0 = processed value
                
            default:
                successFlag = false;
                break;
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
bool DFunctionMeasureAndControl::setValue(eValueIndex_t index, uint32_t value)
{
  bool successFlag = false;

    successFlag = DFunction::setValue(index, value);
    if((false == successFlag) && (NULL != myBarometerSlot))
    {
      DLock is_on(&myMutex);
      successFlag = true;

        switch (index)
        {
          case E_VAL_INDEX_CONTROLLER_MODE:
              if((eControllerMode_t)value <= ((eControllerMode_t)E_CONTROLLER_MODE_PAUSE))
              {
                myNewMode = (eControllerMode_t)value;
                postEvent(EV_FLAG_TASK_NEW_CONTROLLER_MODE_RECIEVED);
                successFlag = true;
              }
              else
              {
                successFlag = false;
              }
              break;
              
           case E_VAL_INDEX_CAL_INTERVAL:
              if((eFunction_t)E_FUNCTION_BAROMETER == myFunction)
              {
                myBarometerSlot->setValue(E_VAL_INDEX_CAL_INTERVAL,value);
              }
              else
              {
                mySlot->setValue(E_VAL_INDEX_CAL_INTERVAL,value);
              }
       
            break;
              
          default:
              successFlag = false;
              break;
        }    

    }
    

    return successFlag;
}



/**
 * @brief   Set calibration type
 * @param   calType - function specific calibration type (0 = user calibration)
 * @param   range - sensor range
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::setCalibrationType(int32_t calType, uint32_t range)
{
    bool flag = false;


    if (myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->setCalibrationType(calType, range);

        //processes should not run when entering calibration mode
        if (flag == true)
        {
            suspendProcesses(true);
        }
    }
    

    return flag;
}

/**
 * @brief   Get required number of calibration points
 * @param   void
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::getRequiredNumCalPoints(uint32_t *numCalPoints)
{
    bool flag = false;
    *numCalPoints = 0u;

    if (myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->getRequiredNumCalPoints(numCalPoints);
    }

    return flag;
}


/**
 * @brief   set required number of calibration points
 * @param   uint32_t   number of cal points
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::setRequiredNumCalPoints(uint32_t numCalPoints)
{
    bool flag = false;
    

    if (myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->setRequiredNumCalPoints(numCalPoints);
    }

    return flag;
}
/**
 * @brief   Start sampling at current cal point
 * @param   void
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::startCalSampling(void)
{
    bool flag = false;

    if (myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->startCalSampling();
    }
    
    return flag;
}

/**
 * @brief   Get remaining number of samples at current cal point
 * @param   pointer to variable for return value of remaining number of samples
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::getCalSamplesRemaining(uint32_t *samples)
{
    bool flag = false;
    *samples = 0u;

    if (myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->getCalSamplesRemaining(samples);
    }
    

    return flag;
}

/**
 * @brief   Set calibration point
 * @param   point indicates the cal point number (1 - required no of points)
 * @param   user supplied calibration value
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::setCalPoint(uint32_t calPoint, float32_t value)
{
    bool flag = false;

    if (myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->setCalPoint(calPoint, value);
    }
   
    return flag;
}

/**
 * @brief   Cal accept
 * @param   void
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::acceptCalibration(void)
{
    bool flag = false;

    if (myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->acceptCalibration();

        //processes can resume when exit calibration mode
        if (flag == true)
        {
            suspendProcesses(false);
        }
    }

    return flag;
}

/**
 * @brief   Abort calibration
 * @param   void
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::abortCalibration(void)
{
    bool flag = false;

    if (myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->abortCalibration();

        //processes can resume when exit calibration mode
        if (flag == true)
        {
            suspendProcesses(false);
        }
    }
    
    return flag;
}

/**
 * @brief   Reload calibration data
 * @param   void
 * @retval  flag: true = success, false = failed
 */
bool DFunctionMeasureAndControl::reloadCalibration(void)
{
    bool flag = true; //if no valid slot then return true meaning 'not required'

    if (myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->reloadCalibration();
    }

    return flag;
}