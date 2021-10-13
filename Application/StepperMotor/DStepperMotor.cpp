/**
* Baker Hughes Confidential\n* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file		DStepperMotor.c
* @version	1.00.00
* @author	Makarand Deshmukh
* @date		20-09-2021
*
* @brief	Stepper Motor Class Source File
*/

/* Includes -----------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include <os.h>
MISRAC_ENABLE
#include "DStepperMotor.h"
#include "DBinaryParser.h"
#include "DStepperMotor.h"
#include "DPV624.h"

/* Defines and constants ----------------------------------------------------*/
#define MAX_CURRENT 2.0f // 2 Amps
#define RUN_CURRENT 2.0f // 2 Amps
#define HOLD_CURRENT 2.0f // 2 Amps
#define ACCL_CURRENT 2.0f // 2 Amps
#define DECEL_CURRENT 2.0f // 2 Amps

/* Types --------------------------------------------------------------------*/

/* Global Variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef hspi2;
/* File Statics -------------------------------------------------------------*/

/* User Code ----------------------------------------------------------------*/

/**
* @brief	DStepperMotor class constructor
* @param	void
* @retval	void
*/
DStepperMotor::DStepperMotor()
{
    /* Acceleration and deceleration parameters */
    commsMotor = new DCommsMotor(&hspi2);
  
    acclAlpha = (float)(DEFAULT_ACCL_ALPHA);
    acclBeta = (float)(DEFAULT_ACCL_BETA);
    decelAlpha = (float)(DEFAULT_DECEL_ALPHA);
    decelBeta = (float)(DEFAULT_DECEL_BETA);

	/* Step size */
    motorStepSize = (uint32_t)(0x0A); // This is from the 

	/* Motor parameters */
    totalStepCount = (int32_t)(0);
    homePosition = (int32_t)(0);
    stepCount = (int32_t)(0);
    minimumSpeed = (uint32_t)(DEFAULT_MINIMUM_SPEED);

#ifdef DIFFERENT_CURRENTS
	float runCurrent;
	float holdCurrent;
	float acclCurrent;
	float decelCurrent;
#endif

#ifndef DIFFERENT_CURRENTS
	/* Motor current 
	Only one current is now used for run, acclereration and deceleration */
	motorCurrent = (float)(DEFAULT_CURRENT);
#endif
    /* Set the required motor currents and operation constants */
    setOperationConstants();
    setCurrents();
    setStepSize();
}

/**
* @brief	DStepperMotor class destructor
* @param	void
* @retval	void
*/
DStepperMotor::~DStepperMotor()
{

}

/**
* @brief	Set the steps size for the motor
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::setStepSize(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandSetParameter);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);
    
    paramWrite.byteArray[3] = (uint8_t)(motorStepSize);
    paramWrite.byteArray[0] = (uint8_t)(0x16); // this is the register on the L6472

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    
    if((uint8_t)(0x3C) == paramRead.byteArray[1])
    {
        
    }
    
    return error;    
}

/**
* @brief	Sets the operating constants for accl and decel
* @param	void
* @retval	void
*/
void DStepperMotor::setOperationConstants(void)
{
    //writeAcclAlpha();
    //writeAcclBeta();
    //writeDecelAlpha();
    //writeDecelBeta();
    readAcclAlpha();
    readAcclBeta();
    readDecclAlpha();
    readDecclBeta();    
}

/**
* @brief	Sets the motor current values
* @param	void
* @retval	void
*/
void DStepperMotor::setCurrents(void)
{
#ifdef DIFFERENT_CURRENTS

#endif

#ifndef DIFFERENT_CURRENTS
    writeCurrent((float)(DEFAULT_CURRENT));
#endif
}

#pragma diag_suppress=Pm137

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeAcclAlpha(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandWriteAcclAlpha);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);
    
    paramWrite.uiValue = (uint32_t)(acclAlpha * 1000.0f);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    
    if((uint8_t)(0x3C) == paramRead.byteArray[1])
    {
        
    }
    
    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeAcclBeta(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandWriteAcclBeta);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    paramWrite.uiValue = (uint32_t)(acclBeta * 1000.0f);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}


/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeDecelAlpha(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandWriteDecelAlpha);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    paramWrite.uiValue = (uint32_t)(decelAlpha * 1000.0f);
    
    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeDecelBeta(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandWriteDecelBeta);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    paramWrite.uiValue = (uint32_t)(decelBeta * 1000.0f);
    
    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

#pragma diag_default=Pm137
/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readAcclAlpha(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandReadAcclAlpha);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);    
    
    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readAcclBeta(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandReadAcclBeta);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);

    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readDecclAlpha(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandReadDecelAlpha);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readDecclBeta(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandReadDecelBeta);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::move(int32_t ptrParam, int32_t* completedCount)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandMoveContinuous);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);
    
    paramWrite.iValue = ptrParam;

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    *completedCount = paramRead.iValue;
    
    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readStepCount(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandReadStepCount);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeMinimumSpeed(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandMinimumSpeed);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeMaximumSpeed(void)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readVersionInfo(sVersion_t *ver)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandReadVersionInfo);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

#ifdef DIFFERENT_CURRENTS
/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readHoldCurrent(float32_t* holdCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readRunCurrent(float32_t* runCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readAcclCurrent(float32_t* acclCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readDecelCurrent(float32_t* decelCur)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeHoldCurrent(float32_t holdCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeRunCurrent(float32_t runCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeAcclCurrent(float32_t acclCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeDecelCurrent(float32_t deccelCurrent)
{
    eMotorError_t error = eMotorErrorNone;

    return error;
}
#endif

#ifndef DIFFERENT_CURRENTS
/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readCurrent(void)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandReadHoldCurrent);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::writeCurrent(float current)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandWriteHoldCurrent);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.floatValue = current;
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}
#endif

/**
* @brief	Sets the motor current
* @param	void
* @retval	void
*/
eMotorError_t DStepperMotor::readSpeedAndCurrent(uint32_t *speed, float32_t *current)
{
    eMotorError_t error = eMotorErrorNone;
    uint8_t command = (uint8_t)(eCommandReadSpeedAndCurrent);
    sParameter_t paramWrite;
    sParameter_t paramRead;
    paramWrite.uiValue = (uint32_t)(0);
    paramRead.uiValue = (uint32_t)(0);

    commsMotor->query(command, paramWrite.byteArray, paramRead.byteArray);
    return error;
}




