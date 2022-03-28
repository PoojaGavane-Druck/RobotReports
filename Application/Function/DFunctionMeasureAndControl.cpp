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
* @file     DFunctionMeasureAddExtBaro.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     02 June 2020
*
* @brief    The DFunctionMeasureAddExtBaro class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include "app_cfg.h"
MISRAC_ENABLE

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
CPU_STK measureAndControlTaskStack[APP_CFG_MEASURE_CONTROL_TASK_STACK_SIZE];
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

    myTaskId = eMeasureAndControlTask;

    myFunction = E_FUNCTION_GAUGE;
    myMode = E_CONTROLLER_MODE_VENT;
    myNewMode = E_CONTROLLER_MODE_VENT;
    newSetPointReceivedFlag = false;
    myAbsoluteReading = 0.0f;
    myGaugeReading = 0.0f;
    myBarometerReading = 0.0f;
    myReading = 0.0f;

    myCurrentPressureSetPoint = (float)(0);
    pressureController = new DController();

    //create the slots as appropriate for the instance
    capabilities.calibrate = (uint32_t)1;
    capabilities.leakTest = (uint32_t)1;
    capabilities.switchTest = (uint32_t)1;

    myAcqMode = (eAquisationMode_t)E_CONTINIOUS_ACQ_MODE;
    createSlots();
    start();
    //events in addition to the default ones in the base class
    myWaitFlags |= EV_FLAG_TASK_STARTUP | EV_FLAG_TASK_NEW_CONTROLLER_MODE_RECIEVED | EV_FLAG_TASK_NEW_BARO_VALUE | EV_FLAG_TASK_NEW_SET_POINT_RECIEVED;
}

/**
 * @brief   DFunctionMeasureAndControl class constructor
 * @param   void
 * @retval  void
 */
DFunctionMeasureAndControl::~DFunctionMeasureAndControl()
{
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
    eSensorType_t  senType = E_SENSOR_TYPE_GENERIC;

    if(NULL != myBarometerSlot)
    {
        mySlot->getValue(E_VAL_INDEX_SENSOR_TYPE, (uint32_t *)&senType);
        mySlot->getValue(E_VAL_INDEX_VALUE, &value);
        myBarometerSlot->getValue(E_VAL_INDEX_VALUE, &barometerReading);

        setValue(E_VAL_INDEX_BAROMETER_VALUE, barometerReading);

        if((eSensorType_t)E_SENSOR_TYPE_PRESS_ABS == senType)
        {
            setValue(EVAL_INDEX_ABS, value);
            setValue(EVAL_INDEX_GAUGE, (value - barometerReading));
        }

        else if((eSensorType_t)E_SENSOR_TYPE_PRESS_GAUGE == senType)
        {
            setValue(EVAL_INDEX_GAUGE, value);
            setValue(EVAL_INDEX_ABS, (value + barometerReading));
        }

        else
        {
            /* do nothing */
        }


    }
    else
    {
        if((eSensorType_t)E_SENSOR_TYPE_PRESS_ABS == senType)
        {
            setValue(EVAL_INDEX_ABS, value);
        }

        else if((eSensorType_t)E_SENSOR_TYPE_PRESS_GAUGE == senType)
        {
            setValue(EVAL_INDEX_GAUGE, value);
        }

        else
        {
            /* Do nothing*/
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
    uint32_t controllerShutdown = 0u;

    //start my main slot - this is set up by each derived class and cannot be NULL
    if(mySlot != NULL)
    {
        mySlot->start();
    }

    if(myBarometerSlot != NULL)
    {
        myBarometerSlot->start();
    }

    myState = E_STATE_SHUTDOWN;

    while(runFlag == true)
    {
#ifdef TASK_HEALTH_MONITORING_IMPLEMENTED
        PV624->keepAlive(myTaskId);
#endif
        actualEvents = RTOSFlagPend(&myEventFlags,
                                    myWaitFlags, (OS_TICK)500u,
                                    OS_OPT_PEND_BLOCKING | OS_OPT_PEND_FLAG_SET_ANY | OS_OPT_PEND_FLAG_CONSUME,
                                    &cpu_ts,
                                    &os_error);

#ifdef ENABLE_STACK_MONITORING
        lastTaskRunning = myTaskId;
#endif
        bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE)) || (os_error == static_cast<OS_ERR>(OS_ERR_TIMEOUT));

        if(!ok)
        {
            MISRAC_DISABLE
#ifdef ASSERT_ENABLED
            assert(false);
#endif
            MISRAC_ENABLE
            PV624->handleError(E_ERROR_OS,
                               eSetError,
                               (uint32_t)os_error,
                               (uint16_t)5);
        }

        //check for events
        if(ok)
        {
            //check for shutdown first
            if((actualEvents & EV_FLAG_TASK_SHUTDOWN) == EV_FLAG_TASK_SHUTDOWN)
            {
                //shut down the main slot before exiting
                myState = E_STATE_SHUTTING_DOWN;
            }

            else if((actualEvents & EV_FLAG_TASK_STARTUP) == EV_FLAG_TASK_STARTUP)
            {
                if(mySlot != NULL)
                {
                    mySlot->powerUp();
                }

                if(myBarometerSlot != NULL)
                {
                    myBarometerSlot->powerUp();
                }

                myState = E_STATE_RUNNING;
            }

            else
            {
                switch(myState)
                {
                case E_STATE_SHUTDOWN:
                    break;

                case E_STATE_RUNNING:
                    handleEvents(actualEvents);
                    break;

                case E_STATE_SHUTTING_DOWN:
                    controllerShutdown = shutdownSequence();

                    if(1u == controllerShutdown)
                    {
                        // Shutdown sensor slots only after controller does not require readings from the sensor
                        if(mySlot != NULL)
                        {
                            mySlot->powerDown();
                        }

                        if(myBarometerSlot != NULL)
                        {
                            myBarometerSlot->powerDown();
                        }

                        myState = E_STATE_SHUTDOWN;
                    }

                    break;

                default:
                    break;
                }
            }
        }
    }
}

/**
 * @brief   This function gracefully executes the shutdown sequence for the PV624
 * @param   void
 * @param   void
 * @return  NA
 */
uint32_t DFunctionMeasureAndControl::shutdownSequence(void)
{
    uint32_t controllerStatus = 0u;
    uint32_t controllerShutdown = 0u;
    /* This function executes the shutdown sequence for the PV624
    If the PV624 is pressurized, we need to vent the pressure to atmosphere first before shutting down
    */

    // First make the task only responsive to new sensor values
    myWaitFlags = EV_FLAG_TASK_NEW_VALUE | EV_FLAG_TASK_NEW_BARO_VALUE | EV_FLAG_TASK_STARTUP;

    // Check controller status
    PV624->getControllerStatus(&controllerStatus);

    if((VENTED == (VENTED & controllerStatus)) && (PISTON_CENTERED == (PISTON_CENTERED & controllerStatus)))
    {
        // Already vented proceed with shutdown
        controllerShutdown = 1u;
    }

    else
    {
        // Not vented, change mode to vent
        PV624->setControllerMode(E_CONTROLLER_MODE_VENT);
        // There has to be some timeout here, if the controller does not shut down within some time... TODO MSD
    }

    return controllerShutdown;
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

    if(mySlot != NULL)
    {
        DLock is_on(&myMutex);
        successFlag = true;

        switch(index)
        {
        case E_VAL_INDEX_VALUE:    //index 0 = processed value
            if((eFunction_t)E_FUNCTION_ABS == myFunction)
            {
                *value = myAbsoluteReading;
            }

            else if((eFunction_t)(eFunction_t)E_FUNCTION_GAUGE == myFunction)
            {
                *value = myGaugeReading;
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

        case EVAL_INDEX_GAUGE:
            *value = myGaugeReading;
            break;

        case EVAL_INDEX_ABS:
            *value = myAbsoluteReading;
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
                myBarometerSlot->getValue(E_VAL_INDEX_POS_FS, value);
            }

            else
            {
                mySlot->getValue(E_VAL_INDEX_POS_FS, value);
            }

            break;

        case E_VAL_INDEX_SENSOR_NEG_FS:          //negative full scale
            if((eFunction_t)E_FUNCTION_BAROMETER == myFunction)
            {
                myBarometerSlot->getValue(E_VAL_INDEX_NEG_FS, value);
            }

            else
            {
                mySlot->getValue(E_VAL_INDEX_NEG_FS, value);
            }

            break;

        case E_VAL_INDEX_PRESSURE_SETPOINT:
            *value = myCurrentPressureSetPoint;
            break;

        case E_VAL_CURRENT_PRESSURE:
            break;

        default:
            successFlag = false;
            break;
        }

        if((false == successFlag) && (NULL != myBarometerSlot))
        {
            switch(index)
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
            switch(index)
            {
            case EVAL_INDEX_GAUGE:
                myGaugeReading = value;
                break;

            case EVAL_INDEX_ABS:
                myAbsoluteReading = value;
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
            switch(index)
            {
            case E_VAL_INDEX_BAROMETER_VALUE:
                myBarometerReading = value;
                successFlag = true;
                break;

            case E_VAL_INDEX_PRESSURE_SETPOINT:
                successFlag = true;
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

        if((eControllerMode_t)E_CONTROLLER_MODE_CONTROL == myMode)
        {
            PV624->incrementSetPointCount();
        }
    }

    if(EV_FLAG_TASK_NEW_SET_POINT_RECIEVED == (actualEvents & EV_FLAG_TASK_NEW_SET_POINT_RECIEVED))
    {
        //ToDO: Handle new set point
        newSetPointReceivedFlag = true;
    }

    if((actualEvents & EV_FLAG_TASK_NEW_VALUE) == EV_FLAG_TASK_NEW_VALUE)
    {
        uint32_t sensorMode;
        mySlot->getValue(E_VAL_INDEX_SENSOR_MODE, &sensorMode);

        if((eSensorMode_t)E_SENSOR_MODE_FW_UPGRADE  > (eSensorMode_t)sensorMode)
        {
            //process and update value and inform UI
            runProcessing();
            //disableSerialPortTxLine(UART_PORT3);

            if(true == PV624->engModeStatus())
            {
                PV624->commsUSB->postEvent(EV_FLAG_TASK_NEW_VALUE);
            }

            else
            {

                pressureInfo_t pressureInfo;

                getPressureInfo(&pressureInfo);
                pressureController->pressureControlLoop(&pressureInfo);
                HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_1);
                setPmSampleRate();

                //mySlot->postEvent(EV_FLAG_TASK_SENSOR_TAKE_NEW_READING);

            }
        }

    }

    if((actualEvents & EV_FLAG_TASK_NEW_BARO_VALUE) == EV_FLAG_TASK_NEW_BARO_VALUE)
    {
        //process and update value and inform UI
        runProcessing();
        //TODo: Screw Controler calls starts here
    }

    //only if setpoints can change in an automated way (eg, ramp, step, etc)
    if((actualEvents & EV_FLAG_TASK_NEW_SETPOINT) == EV_FLAG_TASK_NEW_SETPOINT)
    {
        //ToDo: Need to implement

    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_FAULT) == EV_FLAG_TASK_SENSOR_FAULT)
    {
        //ToDo: Need to implement
    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_DISCONNECT) == EV_FLAG_TASK_SENSOR_DISCONNECT)
    {
        //Todo Update LED Status
        sensorRetry();
        //mySlot->postEvent(EV_FLAG_TASK_SENSOR_RETRY);
    }

    if((actualEvents & EV_FLAG_TASK_BARO_SENSOR_DISCONNECT) == EV_FLAG_TASK_SENSOR_DISCONNECT)
    {
        //Todo Notify Error Handler
        PV624->handleError(E_ERROR_BAROMETER_SENSOR,
                           eSetError,
                           (uint32_t)0,
                           (uint16_t)7);
    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_PAUSE) == EV_FLAG_TASK_SENSOR_PAUSE)
    {
        //Todo Notify Error Handler
        //Todo Update LED Status

    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_CONNECT) == EV_FLAG_TASK_SENSOR_CONNECT)
    {
        //update sensor information
        updateSensorInformation();
        sensorContinue();
    }

    if((actualEvents & EV_FLAG_TASK_BARO_SENSOR_CONNECT) == EV_FLAG_TASK_BARO_SENSOR_CONNECT)
    {

        PV624->handleError(E_ERROR_BAROMETER_SENSOR,
                           eClearError,
                           (uint32_t)0,
                           (uint16_t)9);
    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_CAL_REJECTED) == EV_FLAG_TASK_SENSOR_CAL_REJECTED)
    {
        //ToDO: Need to implement
    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DEFAULT) == EV_FLAG_TASK_SENSOR_CAL_DEFAULT)
    {
        //ToDO: Need to implement
    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DUE) == EV_FLAG_TASK_SENSOR_CAL_DUE)
    {
        //ToDO: Need to implement
    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_CAL_DATE) == EV_FLAG_TASK_SENSOR_CAL_DATE)
    {
        //ToDO: Need to implement
    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_ZERO_ERROR) == EV_FLAG_TASK_SENSOR_ZERO_ERROR)
    {
        //ToDO: Need to implement
    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_IN_LIMIT) == EV_FLAG_TASK_SENSOR_IN_LIMIT)
    {
        //ToDO: Need to implement
    }

    if((actualEvents & EV_FLAG_TASK_SENSOR_NEW_RANGE) == EV_FLAG_TASK_SENSOR_NEW_RANGE)
    {
        //update sensor information as range change may change resolution and no of decimal points
        updateSensorInformation();


    }
}

/**
 * @brief   sets PM Sample rate based on PM type (Terps or Non Terps)
 * @param   void
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::setPmSampleRate(void)
{
    bool retVal = false;
    controllerStatus_t status;
    status.bytes = 0u;
    uint32_t sensorType = 0u;

    getValue(E_VAL_INDEX_CONTROLLER_STATUS_PM, (uint32_t *)(&status.bytes));
    PV624->getPM620Type(&sensorType);

    if((status.bit.measure == 1u) || (status.bit.fineControl) || (status.bit.venting))
    {
        if((sensorType & (uint32_t)(PM_ISTERPS)) == 1u)
        {
            mySlot->setValue(E_VAL_INDEX_SAMPLE_RATE, 0x09u);
        }

        else
        {
            mySlot->setValue(E_VAL_INDEX_SAMPLE_RATE, 0x07u);
        }

        mySlot->setValue(E_VAL_INDEX_SAMPLE_TIMEOUT, TIMEOUT_NON_COARSE_CONTROL);
    }

    else
    {
        if((sensorType & (uint32_t)(PM_ISTERPS)) == 1u)
        {
            mySlot->setValue(E_VAL_INDEX_SAMPLE_RATE, 0x07u);
            mySlot->setValue(E_VAL_INDEX_SAMPLE_TIMEOUT, TIMEOUT_COARSE_CONTROL_PM620T);
        }

        else
        {
            mySlot->setValue(E_VAL_INDEX_SAMPLE_RATE, 0x04u);
            mySlot->setValue(E_VAL_INDEX_SAMPLE_TIMEOUT, TIMEOUT_COARSE_CONTROL_PM620);
        }
    }

    return retVal;
}

/**
 * @brief   Sets all the pressure information required by the controller
 * @param   info - Pointer to pressure info structure contains measured pressure related parameter info
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::getPressureInfo(pressureInfo_t *info)
{
    /*
        absolutePressure = ptrPressureInfo->absolutePressure;
        gaugePressure = ptrPressureInfo->gaugePressure;
        atmosphericPressure = ptrPressureInfo->atmosphericPressure;;
        pressureSetPoint = ptrPressureInfo->pressureSetPoint;
        myMode = ptrPressureInfo->mode;
        setPointType = ptrPressureInfo->setPointType;
        pidParams.controlledPressure = ptrPressureInfo->pressure;
        pidParams.pressureAbs = absolutePressure;
        pidParams.pressureGauge = gaugePressure;
        pidParams.pressureBaro = atmosphericPressure;
        pidParams.pressureOld = ptrPressureInfo->oldPressure;
        pidParams.elapsedTime = ptrPressureInfo->elapsedTime;
    */
    bool status = true;

    eSensorType_t sensorType = (eSensorType_t)(0);
    eFunction_t function = E_FUNCTION_GAUGE;
    info->setPointType = eGauge;
    float pressureVal = 0.0f;
    float barometerVal = 0.0f;

    PV624->getSensorType(&sensorType);
    PV624->getFunction(&function);

    PV624->instrument->getReading((eValueIndex_t)E_VAL_CURRENT_PRESSURE, &pressureVal);
    PV624->instrument->getReading((eValueIndex_t)E_VAL_INDEX_BAROMETER_VALUE, &barometerVal);

    getValue(EVAL_INDEX_GAUGE, &info->gaugePressure);
    getValue(EVAL_INDEX_ABS, &info->absolutePressure);
    getValue(E_VAL_INDEX_BAROMETER_VALUE, &barometerVal);
    info->atmosphericPressure = barometerVal;
    getValue(E_VAL_INDEX_PRESSURE_SETPOINT, &info->pressureSetPoint);
    getFunction((eFunction_t *)&info->setPointType);
    getValue(E_VAL_INDEX_CONTROLLER_MODE, (uint32_t *)&info->mode);
    //PV624->getSetPointType((uint32_t *)(&info->setPointType));
    //PV624->getControllerMode(&info->mode);
    return status;
}

/**
 * @brief   Get Value
 * @param   index is function/sensor specific
 * @param   pointer to variable for return value of requested parameter
 * @return  true if successful, else false
 */
bool DFunctionMeasureAndControl::getValue(eValueIndex_t index, uint32_t *value)
{
    bool successFlag = false;

    uint32_t manID = (uint32_t)(0);
    uint32_t sensorType = (uint32_t)(0);

    if((mySlot != NULL) && (NULL != myBarometerSlot))
    {
        DLock is_on(&myMutex);
        successFlag = true;

        switch(index)
        {
        case E_VAL_INDEX_CONTROLLER_MODE:
            *value = (uint32_t)myMode;
            break;

        case E_VAL_INDEX_CONTROLLER_STATUS:
            *value = (uint32_t)myStatus.bytes;
            break;

        case E_VAL_INDEX_CONTROLLER_STATUS_PM:
            *value = (uint32_t)myStatusPm.bytes;
            break;

        case E_VAL_INDEX_PM620_APP_IDENTITY:
        case E_VAL_INDEX_PM620_BL_IDENTITY:
            mySlot->getValue(index, value);
            successFlag = true;
            break;

        case E_VAL_INDEX_SENSOR_TYPE:         //positive full scale
            if((eFunction_t)E_FUNCTION_BAROMETER == myFunction)
            {
                myBarometerSlot->getValue(E_VAL_INDEX_SENSOR_TYPE, value);
            }

            else
            {
                mySlot->getValue(E_VAL_INDEX_SENSOR_TYPE, value);
            }

            break;

        case E_VAL_INDEX_CAL_INTERVAL:
            if((eFunction_t)E_FUNCTION_BAROMETER == myFunction)
            {
                myBarometerSlot->getValue(E_VAL_INDEX_CAL_INTERVAL, value);
            }

            else
            {
                mySlot->getValue(E_VAL_INDEX_CAL_INTERVAL, value);
            }

            break;

        case EVAL_INDEX_BAROMETER_ID:
            myBarometerSlot->getValue(EVAL_INDEX_SENSOR_MANF_ID, value);
            successFlag = true;
            break;

        case E_VAL_INDEX_PM620_TYPE:
            mySlot->getValue(E_VAL_INDEX_SENSOR_TYPE, &sensorType);
            mySlot->getValue(EVAL_INDEX_SENSOR_MANF_ID, &manID);

            *value = (uint32_t)(sensorType << 16);

            *value = *value | manID;
            break;

#if 0

        case EVAL_INDEX_SENSOR_MANF_ID:
            mySlot->getValue(EVAL_INDEX_SENSOR_MANF_ID, value);
            successFlag = true;
            break;
#endif

        case EVAL_INDEX_PM620_ID:    //index 0 = processed value
            break;

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

        switch(index)
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
                myBarometerSlot->setValue(E_VAL_INDEX_CAL_INTERVAL, value);
            }

            else
            {
                mySlot->setValue(E_VAL_INDEX_CAL_INTERVAL, value);
            }

            break;

        case E_VAL_INDEX_CONTROLLER_STATUS:
            myStatus.bytes = value;
            break;

        case E_VAL_INDEX_CONTROLLER_STATUS_PM:
            myStatusPm.bytes = value;
            break;

        default:
            successFlag = false;
            break;
        }

    }


    return successFlag;
}

/**
 * @brief   take readings at requested rate
 * @param   rate -
 * @retval  void
 */
void DFunctionMeasureAndControl::takeNewReading(uint32_t rate)
{
    mySlot->setValue(E_VAL_INDEX_SAMPLE_RATE, rate);
    mySlot->postEvent(EV_FLAG_TASK_SENSOR_TAKE_NEW_READING);
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


    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
    {
        flag = myBarometerSlot->setCalibrationType(calType, range);

        //processes should not run when entering calibration mode
        if(flag == true)
        {
            suspendProcesses(true);
        }
    }


    return flag;
}

/**
 * @brief   Get required number of calibration points
 * @param   numCalPoints - pointer to variable for return value (required number of calibration points)
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::getRequiredNumCalPoints(uint32_t *numCalPoints)
{
    bool flag = false;
    *numCalPoints = 0u;

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
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


    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
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

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
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

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
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

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
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

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
    {
        flag = myBarometerSlot->acceptCalibration();

        //processes can resume when exit calibration mode
        if(flag == true)
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

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
    {
        flag = myBarometerSlot->abortCalibration();

        //processes can resume when exit calibration mode
        if(flag == true)
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

    if(myBarometerSlot != NULL)
    {
        flag = myBarometerSlot->reloadCalibration();
    }

    return flag;
}


/**
 * @brief   Get cal date
 * @param   pointer to date structure for return value
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::getCalDate(sDate_t *date)
{
    bool flag = false;

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
    {
        if(supportsCalibration() == true)
        {
            //mySlot must be non-null to get here, so no need to check again (use range 0as date is same on all ranges)
            flag = myBarometerSlot->getCalDate(date);
        }
    }

    return flag;
}

/**
 * @brief   Set cal date
 * @param   pointer to date structure for return value (Calibration Date)
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::setCalDate(sDate_t *date)
{
    bool flag = false;

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
    {
        if(supportsCalibration() == true)
        {
            //mySlot must be non-null to get here, so no need to check again (use range 0as date is same on all ranges)
            flag = myBarometerSlot->setCalDate(date);
        }
    }

    return flag;
}

/**
 * @brief   to check calibration supports or not
 * @param   void
 * @retval  flag: true = success, false = failed
 */
bool DFunctionMeasureAndControl::supportsCalibration(void)
{
    bool flag = false;

    //check if function is calibratable
    if(capabilities.calibrate == 1u)
    {
        DLock is_on(&myMutex);

        //also needs the sensor to require at least one calibration point

        if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
        {
            uint32_t numCalPoints;

            if(myBarometerSlot->getRequiredNumCalPoints(&numCalPoints) == true)
            {
                //must have non-zero calibration points for it to be calibratable
                if(numCalPoints > 0u)
                {
                    flag = true;
                }
            }
        }
    }

    return flag;
}

/**
 * @brief   Get cal interval
 * @param   calInterval is pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::getCalInterval(uint32_t *interval)
{
    bool flag = false;

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
    {
        if(supportsCalibration() == true)
        {
            //mySlot must be non-null to get here, so no need to check again
            flag = myBarometerSlot->getCalInterval(interval);
        }
    }

    return flag;
}

/**
 * @brief   Set cal interval
 * @param   cal interval value
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::setCalInterval(uint32_t interval)
{
    bool flag = false;

    if((myBarometerSlot != NULL) && ((eFunction_t)E_FUNCTION_BAROMETER == myFunction))
    {
        if(supportsCalibration() == true)
        {
            //mySlot must be non-null to get here, so no need to check again
            flag = myBarometerSlot->setCalInterval(interval);
        }
    }

    return flag;
}

/**
 * @brief   Set cal interval
 * @param   cal interval value
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::initController(void)
{
    bool flag = false;

    if(pressureController != NULL)
    {
        pressureController->initialize();
        flag = true;
    }

    return flag;
}

/**
 * @brief   Sets aquisation mode of pressure slot and barometer slot
 * @param   newAcqMode : new Aquisation mode
 * @retval  true = success, false = failed
 */
bool DFunctionMeasureAndControl::setAquisationMode(eAquisationMode_t newAcqMode)
{
    bool retStatus = true;

    if(mySlot != NULL)
    {
        mySlot->setAquisationMode(newAcqMode);
    }

    else
    {
        retStatus = false;
    }

    if(NULL != myBarometerSlot)
    {
        myBarometerSlot->setAquisationMode(newAcqMode);
    }

    else
    {
        retStatus = false;
    }

    return retStatus;
}

/**
 * @brief   upgrades PM620 sensor firmware
 * @param   void :
 * @retval  returns true for success  and false for failure
 */
bool DFunctionMeasureAndControl::upgradeSensorFirmware(void)
{
    bool retStatus = false;
    uSensorIdentity_t sensorId;
    sensorId.dk = (uint32_t)0;
    mySlot->getValue(E_VAL_INDEX_PM620_APP_IDENTITY, &sensorId.value);

    if(PM620_TERPS_APP_DK_NUMBER == sensorId.dk)
    {
        mySlot->upgradeSensorFirmware();
        retStatus = true;
    }

    return retStatus;
}

/**
 * @brief   Starts the PV624
 * @param   newAcqMode : new Aquisation mode
 * @retval  void
 */
void DFunctionMeasureAndControl::startUnit(void)
{
    postEvent(EV_FLAG_TASK_STARTUP);
}

/**
 * @brief   Stops the PV624
 * @param   newAcqMode : new Aquisation mode
 * @retval  void
 */
void DFunctionMeasureAndControl::shutdownUnit(void)
{
    postEvent(EV_FLAG_TASK_SHUTDOWN);
}

/**
 * @brief   Start function
 * @param   void
 * @retval  void
 */
void DFunctionMeasureAndControl::start(void)
{

    OS_ERR err;

    myTaskStack = (CPU_STK *)&measureAndControlTaskStack[0];

#ifdef ENABLE_STACK_MONITORING
    stackArray.uiStack.addr = (void *)myTaskStack;
    stackArray.uiStack.size = (uint32_t)(APP_CFG_MEASURE_CONTROL_TASK_STACK_SIZE * 4u);
    fillStack((char *)myTaskStack, 0xBB, (size_t)(APP_CFG_MEASURE_CONTROL_TASK_STACK_SIZE * 4u));
#endif
    activate(myName, (CPU_STK_SIZE)APP_CFG_MEASURE_CONTROL_TASK_STACK_SIZE, (OS_PRIO)4u, (OS_MSG_QTY)10u, &err);


}