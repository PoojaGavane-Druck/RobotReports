/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DPV624.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     12 April 2020
*
* @brief    The PV624 instrument class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "misra.h"

MISRAC_DISABLE
#include "main.h"
#include <os.h>
#include <assert.h>
#include "usb_device.h"
#include <string.h>
MISRAC_ENABLE

#include "DPV624.h"
#include "DStepperMotor.h"
#include "DSlot.h"
#include "i2c.h"
#include "uart.h"
#include "Utilities.h"
#include "math.h"
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

//#define TEST_MOTOR

#define MOTOR_FREQ_CLK 12000000u

#define SP_ATMOSPHERIC 2u
#define SP_ABSOLUTE 1u
#define SP_GAUGE 0u

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
DPV624 *PV624;

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;
extern SMBUS_HandleTypeDef hsmbus1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern SPI_HandleTypeDef hspi2;

extern const unsigned int cAppDK;
extern const unsigned char cAppVersion[4];
extern const unsigned int mainBoardHardwareRevision;

#ifdef BOOTLOADER_IMPLEMENTED
/*  __FIXED_region_ROM_start__ in bootloader stm32l4r5xx_flash.icf */
const unsigned int bootloaderFixedRegionRomStart = 0x08000200u;
__no_init const unsigned int cblDK @ bootloaderFixedRegionRomStart + 0u;
__no_init const unsigned char cblVersion[4] @ bootloaderFixedRegionRomStart + 4u;
//__no_init const char cblInstrument[16] @ bootloaderFixedRegionRomStart + 8u;
#else
const unsigned int cblDK = 498u;
const unsigned char cblVersion[4] = {0, 99, 99, 99};
#endif

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
#include "Utilities.h"

#ifdef ENABLE_STACK_MONITORING
sStackMonitor_t stackArray;

void fillStack(char *addr, short value, size_t bytes)
{
    uint32_t count = (uint32_t)bytes;

    for(uint32_t i = 0u; i < count; i++)
    {
        addr[i] = (char)value;
    }
}

int lastTaskRunning;
int gfxLock;
int gfxLock2;
#endif


static const uint32_t stuckTolerance = 10u;
DPV624::DPV624(void)
{
    OS_ERR os_error;
    isEngModeEnable = false;
    myPowerState = E_POWER_STATE_OFF;
    pmUpgradePercent = 0u;
    instrumentMode.value = 0u;
    myPinMode = E_PIN_MODE_NONE;
    blState = BL_STATE_DISABLE;
    controllerDistance = 0.0f;

    myBlTaskState = E_BL_TASK_SUSPENDED;




    memset(&keepAliveCount[0], 0, 4u * eNumberOfTasks);
    memset(&keepAlivePreviousCount[0], 0, 4u * eNumberOfTasks);
    memset(&keepAliveIsStuckCount[0], 0, 4u * eNumberOfTasks);

    i2cInit(&hi2c4);

    persistentStorage = new DPersistent();

    uartInit(&huart2);
    uartInit(&huart3);
    uartInit(&huart4);
    uartInit(&huart5);

    // enable deferred IWDG now after posssible FW upgrade is complete
    //EnableDeferredIWDG();

    extStorage = new DExtStorage(&os_error);
    validateApplicationObject(os_error);

    powerManager = new DPowerManager(&hsmbus1, &os_error);
    validateApplicationObject(os_error);

    logger = new DLogger(&os_error);
    validateApplicationObject(os_error);

    errorHandler = new DErrorHandler(&os_error);
    validateApplicationObject(os_error);

    keyHandler = new DKeyHandler(&os_error);
    validateApplicationObject(os_error);

    stepperMotor = new DStepperMotor();

    instrument = new DInstrument(&os_error);
    validateApplicationObject(os_error);

    commsOwi = new DCommsOwi("commsOwi", &os_error);
    validateApplicationObject(os_error);

    commsUSB = new DCommsUSB("commsUSB", &os_error);
    validateApplicationObject(os_error);
#if 1
    commsBluetooth = new DCommsBluetooth("commsBLE", &os_error);
    handleOSError(&os_error);
#endif
    valve1 = new DValve(&htim1,
                        TIM1,
                        TIM_CHANNEL_1,
                        VALVE1_DIR_PC7_GPIO_Port,
                        VALVE1_DIR_PC7_Pin,
                        VALVE1_ENABLE_GPIO_Port,
                        VALVE1_ENABLE_Pin,
                        0u);

    validateApplicationObject(os_error);

    valve2 = new DValve(&htim5,
                        TIM5,
                        TIM_CHANNEL_2,
                        VALVE2_DIR_PC8_GPIO_Port,
                        VALVE2_DIR_PC8_Pin,
                        VALVE2_ENABLE_GPIO_Port,
                        VALVE2_ENABLE_Pin,
                        0u);

    validateApplicationObject(os_error);

    valve3 = new DValve(&htim3,
                        TIM3,
                        TIM_CHANNEL_1,
                        VALVE3_DIR_PF3_GPIO_Port,
                        VALVE3_DIR_PF3_Pin,
                        VALVE3_ENABLE_GPIO_Port,
                        VALVE3_ENABLE_Pin,
                        1u);

    validateApplicationObject(os_error);

#if 1
    // Start the UI task first
    userInterface = new DUserInterface(&os_error);
    validateApplicationObject(os_error);
#endif

    isPrintEnable = true;


    //managePower();
    setPowerState(E_POWER_STATE_ON);

    // Show yellow LED to indicate that system is turning on
    userInterface->statusLedControl(eStatusProcessing,
                                    E_LED_OPERATION_SWITCH_ON,
                                    65535u,
                                    E_LED_STATE_SWITCH_ON,
                                    0u);
    //setAquisationMode(E_REQUEST_BASED_ACQ_MODE);
}


/**
 * @brief   DInstrument class deconstructor
 * @param   void
 * @retval  void
 */
DPV624::~DPV624(void)
{

}
/**
* @brief    handleError - process messaages from the rest of the system
* @param    errorCode - enumerated error code identifier value
* @param    param -    optional parameter value TODO - Not currently used
* @param    blocking - optional parameter if true to stop here for showstopper faults TODO - blocking error won't cause watchdog to trip
* @return   None
*/
void DPV624::handleError(eErrorCode_t errorCode,
                         eErrorStatus_t errStatus,
                         uint32_t paramValue,
                         uint16_t errInstance,
                         bool isFatal)
{
    if(errorHandler != NULL)
    {
        errorHandler->handleError(errorCode,
                                  errStatus,
                                  paramValue,
                                  errInstance,
                                  isFatal);
    }
}

/**
* @brief    handleError - process messaages from the rest of the system
* @param    errorCode - enumerated error code identifier value
* @param    param -    optional parameter value TODO - Not currently used
* @param    blocking - optional parameter if true to stop here for showstopper faults TODO - blocking error won't cause watchdog to trip
* @return   None
*/
void DPV624::handleError(eErrorCode_t errorCode,
                         eErrorStatus_t errStatus,
                         float paramValue,
                         uint16_t errInstance,
                         bool isFatal)
{
    if(errorHandler != NULL)
    {
        errorHandler->handleError(errorCode,
                                  errStatus,
                                  paramValue,
                                  errInstance,
                                  isFatal);
    }
}

/**
 * @brief   validates the application object was created without error
 * @retval  None
 */
void DPV624::validateApplicationObject(OS_ERR os_error)
{
    bool ok = (os_error == static_cast<OS_ERR>(OS_ERR_NONE));

    if(!ok)
    {
#ifdef ASSERT_IMPLEMENTED
        MISRAC_DISABLE
        assert(false);
        MISRAC_ENABLE
#endif

        if((PV624 != NULL) && (errorHandler != NULL))
        {

            PV624->handleError(E_ERROR_OS,
                               eSetError,
                               (uint32_t)os_error,
                               1u);
        }

        else
        {
            // Resort to global error handler
            Error_Handler();
        }
    }
}

/**
 * @brief   Starts the PV624
 * @note    NA
 * @param   void
 * @retval  character string
 */
void DPV624::managePower(void)
{
    // CHeck the current state of the PV624
    if((uint32_t)(E_POWER_STATE_OFF) == myPowerState)
    {
        // PV624 is off, we have to turn it on
        startup();
    }

    else
    {
        // PV624 is ON, we have to turn it off
        shutdown();
    }
}

/**
 * @brief   Starts the PV624
 * @note    NA
 * @param   void
 * @retval  character string
 */
void DPV624::startup(void)
{
    // A reset re initializes the micro controller and thus the remaining system
    resetSystem();
}

/**
 * @brief   Shuts the PV624 down
 * @note    NA
 * @param   void
 * @retval  character string
 */
void DPV624::shutdown(void)
{
    setPowerState(E_POWER_STATE_OFF);
    instrument->shutdown();
}

/**
 * @brief   Resets the main micro controller and thereby the PV624
 * @note    NA
 * @param   void
 * @retval  character string
 */
void DPV624::resetSystem(void)
{
    // Cause an intentional system reset, so everything will be re initialized
    NVIC_SystemReset();
}

/**
 * @brief   Holds the stepper motor micro in reset
 * @note    NA
 * @param   void
 * @retval  character string
 */
void DPV624::holdStepperMotorReset(void)
{
    // Cause an intentional system reset, so everything will be re initialized
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
}


/**
 * @brief   Sets the power state of PV624
 * @note    NA
 * @param   void
 * @retval  character string
 */
void DPV624::setPowerState(ePowerState_t powerState)
{
    myPowerState = powerState;
}

/**
 * @brief   Shuts the PV624 down
 * @note    NA
 * @param   void
 * @retval  character string
 */
ePowerState_t DPV624::getPowerState(void)
{
    return myPowerState;
}

/**
 * @brief   Sets the power state of PV624
 * @note    NA
 * @param   void
 * @retval  character string
 */
void DPV624::setBluetoothTaskState(eBluetoothTaskState_t blTaskState)
{
    myBlTaskState = blTaskState;
}

/**
 * @brief   Shuts the PV624 down
 * @note    NA
 * @param   void
 * @retval  character string
 */
eBluetoothTaskState_t DPV624::getBluetoothTaskState(void)
{
    return myBlTaskState;
}

/**
 * @brief   Get serial number
 * @note    Instrument "serial number" if actually a free format ASCII character string
 * @param   void
 * @retval  character string
 */
uint32_t DPV624::getSerialNumber(uint32_t snType)
{
    uint32_t sn = 0u;

    if(0u == snType)
    {
        sn =  persistentStorage->getSerialNumber();
    }

    else
    {
        instrument->getSensorSerialNumber(&sn);
    }

    return sn;
}

/**
 * @brief   Set serial number
 * @note    Instrument "serial number" if actually a free format ASCII character string
 * @param   str - string
 * @retval  true = success, false = failed
 */
bool DPV624::setSerialNumber(uint32_t newSerialNumber)
{
    bool flag = false;

    if(newSerialNumber != 0XFFFFFFFFu)  //one byte less for null terminator
    {
#if 0
        //update string value
        sConfig_t *configData = persistentStorage->getConfigDataAddr();
        configData->serialNumber = newSerialNumber;

        //save in persistent storage
        if(persistentStorage->saveConfigData() == true)
        {
            flag = true;
        }

#endif
        flag = persistentStorage->setSerialNumber(newSerialNumber);
    }

    return flag;
}

/**
 * @brief   Get stepper motor instance
 * @param   void
 * @retval  current setting for region of use
 */
DStepperMotor *DPV624::getStepperMotorInstance(void)
{
    return stepperMotor;
}

/**
 * @brief   Get region of use
 * @param   void
 * @retval  current setting for region of use
 */
eRegionOfUse_t DPV624::getRegion(void)
{
    return (eRegionOfUse_t)0u;
}

/**
 * @brief   Set region of use
 * @param   region - are of use
 * @retval  true = success, false = failed
 */
bool DPV624::setRegion(eRegionOfUse_t region)
{
    return false;
}


/**
 * @brief   Query if instrument variant is intrinsically variant
 * @param   void
 * @retval  true if IS variant, else false (commercial)
 */
int32_t DPV624::queryEEPROMTest(void)
{
    int32_t result = 0;

    sPersistentDataStatus_t status = persistentStorage->getStatus();

    switch(status.selfTestResult)
    {
    case 2:
        result = 1;
        break;

    case 3:
        result = -1;
        break;

    default:
        break;
    }

    return result;
}

/**
 * @brief   Reset stepper micro controller
 * @param   void
 * @retval  void
 */
void DPV624::resetStepperMicro(void)
{
    /* Reset the stepper controller micro */

    /* Generate a 100 ms reset */
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_Delay((uint16_t)(10));
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_Delay((uint16_t)(100));
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_SET);
}

/**
 * @brief   Perform EEPROM test
 * @param   void
 * @retval  void
 */
void DPV624::performEEPROMTest(void)
{
    return persistentStorage->selfTest();
}

/**
 * @brief   Get pressed key
 * @param   void
 * @retval  true = success, false = failed
 */
uint32_t DPV624::getKey(void)
{
    return 0u;
}

/**
 * @brief   Emulate key press
 * @param   key - key id
 * @param   pressType - 0 - short press, 1 = long press
 * @retval  true = success, false = failed
 */
bool DPV624::setKey(uint32_t key, uint32_t pressType)
{
    return false;
}

/**
 * @brief   Get Date in RTC
 * @param   date - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getDate(sDate_t *date)
{
    return getSystemDate(date);
}

/**
 * @brief   Set Date in RTC
 * @param   date - user specified date to set
 * @retval  true = success, false = failed
 */
bool DPV624::setDate(sDate_t *date)
{
    return setSystemDate(date);
}

/**
 * @brief   Get system time from RTC
 * @param   time - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getTime(sTime_t *time)
{
    return getSystemTime(time);
}

/**
 * @brief   Set system time in RTC
 * @param   time - pointer to system time structure containing time to set the RTC
 * @retval  true = success, false = failed
 */
bool DPV624::setTime(sTime_t *time)
{
    return setSystemTime(time);
}

/**
 * @brief   Get version of specified item
 * @param   item - value: 0 = PV624
 *                        1 = PM620
 * @param   component - value: 0 = Application
 *                        1 = Bootloader
 *
 * @param   itemver - pointer variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getVersion(uint32_t item, uint32_t component, char itemverStr[10])
{
    bool status = false;
    itemverStr[0] = '\0';

    if(item <= 1u)
    {

        if(0u == item)
        {
            switch(component)
            {
            case 0:
            {
                snprintf(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)cAppVersion[1], (uint8_t)cAppVersion[2], (uint8_t)cAppVersion[3]);
                status = true;
                break;
            }

            case 1:
            {
                snprintf(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)cblVersion[1], (uint8_t)cblVersion[2], (uint8_t)cblVersion[3]);
                status = true;
                break;
            }

            case 6:
            {
                sVersion_t secondaryAppVersion;
                secondaryAppVersion.all = 0u;
                stepperMotor->getAppVersion(&secondaryAppVersion);
                snprintf(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)secondaryAppVersion.major,
                         (uint8_t)secondaryAppVersion.minor,
                         (uint8_t)secondaryAppVersion.build);
                status = true;
                break;
            }

            case 7:
            {
                sVersion_t secondaryBootVersion;
                secondaryBootVersion.all = 0u;
                stepperMotor->getBootVersion(&secondaryBootVersion);
                snprintf(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)secondaryBootVersion.major,
                         (uint8_t)secondaryBootVersion.minor,
                         (uint8_t)secondaryBootVersion.build);
                status = true;
                break;
            }


            default:
            {
                status = false;
                break;
            }
            }
        }

        else
        {
            uSensorIdentity_t identity;

            switch(component)
            {
            case 0:
            {
                status = instrument->getExternalSensorAppIdentity(&identity);

                if(status)
                {
                    snprintf(itemverStr, 10u, "%02d.%02d.%02d", identity.major, identity.minor, identity.build);
                }

                status = true;
                break;
            }

            case 1:
            {
                status = instrument->getExternalSensorBootLoaderIdentity(&identity);

                if(status)
                {
                    snprintf(itemverStr, 10u, "%02d.%02d.%02d", identity.major, identity.minor, identity.build);
                }

                status = true;
                break;
            }

            default:
            {
                status = false;
                break;
            }
            }
        }
    }



    return status;
}

/**
 * @brief   Get DK of specified item as string
 * @param   item - value: 0 = Bootloader
 *                        1 = Application
 * @param   char dkStr[7] - pointer variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getDK(uint32_t item, uint32_t component, char dkStr[7])
{
    bool status = false;

    if(item <= 1u)
    {

        if(0u == item)
        {
            switch(component)
            {
            case 0:
            {
                snprintf(dkStr, 7u, "%04d", cAppDK);
                status = true;
                break;
            }

            case 1:
            {
                snprintf(dkStr, 7u, "%04d", cblDK);
                status = true;
                break;
            }

            default:
            {
                status = false;
                break;
            }
            }
        }

        else
        {
            uSensorIdentity_t identity;

            switch(component)
            {
            case 0:
            {
                status = instrument->getExternalSensorAppIdentity(&identity);

                if(status)
                {
                    snprintf(dkStr, 7u, "%04d", identity.dk);
                }

                status = true;
                break;
            }

            case 1:
            {
                status = instrument->getExternalSensorBootLoaderIdentity(&identity);

                if(status)
                {
                    snprintf(dkStr, 7u, "%04d", identity.dk);
                }

                status = true;
                break;
            }

            default:
            {
                status = false;
                break;
            }
            }
        }
    }


    return status;
}

/**
 * @brief   Get instrument name
 * @param   char nameStr[16] - pointer variable for return value
 * @retval  void
 */
void DPV624::getInstrumentName(char nameStr[13])
{
    // overrule stored cblInstrument and cAppInstrument values
    strncpy(nameStr,  "PV624HYBRID", (size_t)13);
}
/**
 * @brief   Get positive fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getPosFullscale(float32_t  *fs)
{
    return instrument->getPosFullscale(fs);
}

/**
 * @brief   Get negative fullscale of channel function
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getNegFullscale(float32_t  *fs)
{
    return instrument->getNegFullscale(fs);
}

/**
 * @brief   Get sensor type
 * @param   sensorType - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getSensorType(eSensorType_t *sensorType)
{
    return instrument->getSensorType(sensorType);
}

/**
 * @brief   Get sensor type for engineering protocol
 * @param   sensorType - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getPM620Type(uint32_t *sensorType)
{
    return instrument->getPM620Type(sensorType);
}

/**
 * @brief   Get controller mode
 * @param   controller mode - pointer to variable for return value
 * @retval  true = success, false = failed
*/
bool DPV624::getControllerMode(eControllerMode_t *controllerMode)
{
    return instrument->getControllerMode(controllerMode);
}

/**
 * @brief   Set controller mode
 * @param   controller mode - pointer to variable for return value
 * @retval  true = success, false = failed
*/
bool DPV624::setControllerMode(eControllerMode_t newCcontrollerMode)
{
    return instrument->setControllerMode(newCcontrollerMode);
}

/**
 * @brief   Get controller mode
 * @param   controller mode - pointer to variable for return value
 * @retval  true = success, false = failed
*/
bool DPV624::getVentRate(float *rate)
{
    return instrument->getVentRate(rate);
}

/**
 * @brief   Set controller mode
 * @param   controller mode - pointer to variable for return value
 * @retval  true = success, false = failed
*/
bool DPV624::setVentRate(float rate)
{
    return instrument->setVentRate(rate);
}


/**
 * @brief   Get cal interval
 * @param   interval is pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getCalInterval(uint32_t sensor, uint32_t *interval)
{
    bool flag = false;
    eFunction_t func = E_FUNCTION_GAUGE;
    //if function on specified channel is not being calibrated then we are setting the instrument's cal interval
    //flag = instrument->getFunction((eFunction_t *)&func);

    //if(true == flag)
    //{
    if(1u == sensor)
    {
        //"channel" can only be 0 if updating the instrument-wide cal interval
        if(NULL != interval)
        {
            *interval = persistentStorage->getCalInterval();
            flag = true;
        }
    }

    else
    {
        //set cal interval for the sensor being calibrated
        flag = instrument->getCalInterval(interval);
    }

    //}

    return flag;
}

/**
 * @brief   set cal interval
 * @param   interval  value
 * @retval  true = success, false = failed
 */
bool DPV624::setCalInterval(uint32_t sensor, uint32_t interval)
{
    bool flag = false;
    //eFunction_t func = E_FUNCTION_GAUGE;
    //if function on specified channel is not being calibrated then we are setting the instrument's cal interval
    //flag = instrument->getFunction((eFunction_t *)&func);

    //if(true == flag)
    //{
    if(1u ==  sensor)
    {
        flag = persistentStorage->setCalInterval(interval);

        if(true == flag)
        {
            flag = instrument->setCalInterval(sensor, interval);
        }

    }

    else
    {
        //Not allowed to write PM620 Calibration interval
        flag = false;
    }

    //}

    return flag;
}

bool DPV624::getPressureSetPoint(float *pSetPoint)
{
    bool successFlag = false;

    if(NULL != pSetPoint)
    {
        successFlag = instrument->getPressureSetPoint(pSetPoint);
    }

    else
    {
        successFlag =  false;
    }

    return successFlag;
}

bool DPV624::setPressureSetPoint(float newSetPointValue)
{
    bool successFlag = false;

    successFlag = instrument->setPressureSetPoint(newSetPointValue);

    return successFlag;
}

/**
 * @brief   get Instrument function
 * @param   func is the function itself
 * @retval  true if activated successfully, else false
 */
bool DPV624::getFunction(eFunction_t *pFunc)
{
    return instrument->getFunction(pFunc);
}

void DPV624::takeNewReading(uint32_t rate)
{
    instrument->takeNewReading(rate);
}

bool DPV624::setControllerStatusPm(uint32_t status)
{
    return instrument->setControllerStatusPm(status);
}

bool DPV624::getControllerStatusPm(uint32_t *status)
{
    return instrument->getControllerStatusPm(status);
}


bool DPV624::setControllerStatus(uint32_t newStatus)
{
    return instrument->setControllerStatus(newStatus);
}


#if 1
/**
 * @brief   Get Controller Status
 * @param   void
 * @retval  uint32_t controller status
 */
bool DPV624::getControllerStatus(uint32_t *controllerStatus)
{
    return instrument->getControllerStatus(controllerStatus);
}
#endif


/**
 * @brief   Get current PIN mode (ie, protection level)
 * @param   void
 * @retval  PIN mode
 */
ePinMode_t DPV624::getPinMode(void)
{
    return myPinMode;
}


/**
 * @brief   Get current PIN mode (ie, protection level)
 * @param   mode - new PIN mode
 * @retval  true = success, false = failed
 */
bool DPV624::setPinMode(ePinMode_t mode)
{
    bool flag = false;

    //can only change mode if either current mode or new mode is E_PIN_MODE_NONE;
    //ie cannot switch modes without going through 'no pin' mode
    if((myPinMode == (ePinMode_t)E_PIN_MODE_NONE) || (mode == (ePinMode_t)E_PIN_MODE_NONE))
    {
        myPinMode = mode;
        flag = true;
    }

    return flag;
}

/**
 * @brief   Set Instrument Port Configuration for USB
 * @param   mode - 0 for communications mode; 1 for storage mode
 * @retval  none
 */
void DPV624::setUsbInstrumentPortConfiguration(int32_t mode)
{
//#ifdef PORT_SWITCHING_IMPLEMENTED
    MX_USB_DEVICE_SetUsbMode((eUsbMode_t)mode);
//#endif
}

/**
 * @brief   Get Instrument Port Configuration for USB
 * @param   none
 * @retval  mode - 0 for communications mode; 1 for storage mode
 */
int32_t DPV624::getUsbInstrumentPortConfiguration()
{
//#ifdef PORT_SWITCHING_IMPLEMENTED
    return (int32_t)MX_USB_DEVICE_GetUsbMode();
//#else
    //   return (int32_t)0;
//#endif

}

/**
 * @brief   Get cal date
 * @param   instrument channel
 * @param   pointer to date structure for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getManufactureDate(sDate_t *date)
{
    bool flag = false;

    if(NULL != date)
    {
        //get address of manufacure date structure in persistent storage TODO
        //sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        //date->day = manufactureDate.day;
        //date->month = manufactureDate.month;
        //date->year = manufactureDate.year;

        flag = persistentStorage->getManufacturingDate(date);
    }

    return flag;
}

/**
 * @brief   Set cal date
 * @param   instrument channel
 * @param   pointer to date structure
 * @retval  true = success, false = failed
 */
bool DPV624::setCalDate(sDate_t *date)
{
    bool flag = false;


    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
#if 0
        sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        calDataBlock->calDate.day = date->day;
        calDataBlock->calDate.month = date->month;
        calDataBlock->calDate.year = date->year;

        calDataBlock->measureBarometer.data.calDate.day = date->day;
        calDataBlock->measureBarometer.data.calDate.month = date->month;
        calDataBlock->measureBarometer.data.calDate.year = date->year;

        flag = persistentStorage->saveCalibrationData();
#endif
        flag = persistentStorage->setCalibrationDate(date);
    }

    return flag;
}

/**
 * @brief   Set cal date
 * @param   instrument channel
 * @param   pointer to date structure
 * @retval  true = success, false = failed
 */
bool DPV624::setManufactureDate(sDate_t *date)
{
    bool flag = false;


    if(NULL != date)
    {
        manufactureDate.day = date->day;
        manufactureDate.month = date->month;
        manufactureDate.year = date->year;

        flag = persistentStorage->setManufacturingDate(date);
    }

    return flag;
}

bool DPV624::getSensorBrandUnits(char *brandUnits)
{

    return true;
}

/**
 * @brief   Set calibration type
 * @param   calType - function specific calibration type (0 = user calibration)
 * @param   range - sensor range
 * @retval  true = success, false = failed
 */
bool DPV624::setCalibrationType(int32_t calType, uint32_t range)
{
    bool retStatus = false;

    retStatus = instrument->setCalibrationType(calType, range);



    return retStatus;
}

/**
 * @brief   Start sampling at current cal point
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::startCalSampling(void)
{
    return instrument->startCalSampling();
}

/**
 * @brief   Set calibration point
 * @param   point indicates the cal point number (1 - required no of points)
 * @param   user supplied calibration value
 * @retval  true = success, false = failed
 */
bool DPV624::setCalPoint(uint32_t calPoint, float32_t value)
{
    return instrument->setCalPoint(calPoint, value);
}

/**
 * @brief   Cal accept
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::acceptCalibration(void)
{
    return instrument->acceptCalibration();
}

/**
 * @brief   Abort calibration
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::abortCalibration(void)
{
    return instrument->abortCalibration();
}

/**
 * @brief   Sets required number of calibration points
 * @param   uint32_t number of calpoints
 * @retval  true = success, false = failed
 */
bool DPV624::setRequiredNumCalPoints(uint32_t numCalPoints)
{
    return instrument->setRequiredNumCalPoints(numCalPoints);
}

/**
 * @brief   Perform application firmware upgrade
 * @param   void
 * @retval  flag: true if ok, else false
 */
bool DPV624::performUpgrade(void)
{
    bool ok = true;


    return ok;
}

/**
 * @brief   Perform application firmware upgrade
 * @param   void
 * @retval  flag: true if ok, else false
 */
bool DPV624::performPM620tUpgrade(void)
{
    bool ok = true;
    ok = instrument->upgradeSensorFirmware();
    return ok;
}

/*****************************************************************************/
/* SUPPRESS: floating point values shall not be tested for exact equality or
inequality (MISRA C 2004 rule 13.3) */
/*****************************************************************************/
_Pragma("diag_suppress=Pm046")
/**
 * @brief   Set channel sensor zero value
 * @param   value - user provided value to set as the zero
 * @retval  true = success, false = failed
 */
bool DPV624::setZero(uint32_t sensor, float32_t value)
{
    /* Test of zero val setting, this has to be written to mem ? */
    float fsPressure = 0.0f;
    float zeroPc = 0.0f;
    bool status = false;

    instrument->getPositiveFS((float *)&fsPressure);
    zeroPc = fabs(value) * 100.0f / fsPressure;

    if(1.0f > zeroPc)
    {
        zeroVal = value;
        status = instrument->setSensorZeroValue(sensor, zeroVal);
    }

    return status;
}
/*****************************************************************************/
/* SUPPRESS: floating point values shall not be tested for exact equality or
inequality (MISRA C 2004 rule 13.3) */
/*****************************************************************************/
_Pragma("diag_default=Pm046")




/**
 * @brief   Get battery status from Coulomb handler
 * @param   pointer to sBatteryStatus
 * @retval  none
 */
void DPV624::getBatteryStatus(sBatteryStatus_t *sBatteryStatus)
{

}

/**
 * @brief   Save current cal as backup
 * @param   void
 * @retval  flag: true = success, false = failed
 */
bool DPV624::backupCalDataSave(void)
{
    bool flag = false;

    if(persistentStorage != NULL)
    {
        flag = persistentStorage->saveAsBackupCalibration();
    }

    return flag;
}


/**
 * @brief   Restore backup cal as current
 * @param   void
 * @retval  flag: true = success, false = failed
 */
bool DPV624::backupCalDataRestore(void)
{
    bool flag = false;

    if(persistentStorage != NULL)
    {
        flag = persistentStorage->loadBackupCalibration();

        if(instrument != NULL)
        {
            flag &= instrument->reloadCalibration();
        }
    }

    return flag;
}

/**
 * @brief   Query if instrument variant is intrinsically variant
 * @param   void
 * @retval  true if IS variant, else false (commercial)
 */
int32_t DPV624::queryInvalidateCalOpeResult(void)
{
    int32_t result = 0;

    sPersistentDataStatus_t status = persistentStorage->getStatus();

    switch(status.invalidateCalOperationResult)
    {
    case 2:
        result = 1;
        break;

    case 3:
        result = -1;
        break;

    default:
        break;
    }

    return result;
}

/**
 * @brief   Get remaining number of samples at current cal point
 * @param   pointer to variable for return value of remaining number of samples
 * @retval  true = success, false = failed
 */
bool DPV624::getCalSamplesRemaining(uint32_t *samples)
{
    return instrument->getCalSamplesRemaining(samples);
}

/**
 * @brief   Get required number of calibration points
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::getRequiredNumCalPoints(eSensor_t sensorType, uint32_t *numCalPoints)
{
    return instrument->getRequiredNumCalPoints(sensorType, numCalPoints);
}

/**
 * @brief   Get channel sensor zero value
 * @param   channel - instrument channel
 * @param   value - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getZero(float32_t *value)
{
    //TODO HSB:
    //*value = zeroVal;
    //return true;
    return instrument->getSensorZeroValue(0u, value);

}

/**
 * @brief   Get cal date
 * @param   instrument channel
 * @param   pointer to date structure for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getCalDate(sDate_t *date)
{
    bool flag = false;

    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
        //sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        //date->day = calDataBlock->measureBarometer.data.calDate.day;
        //date->month = calDataBlock->measureBarometer.data.calDate.month;
        //date->year = calDataBlock->measureBarometer.data.calDate.year;

        //flag = true;
        flag = persistentStorage->getCalibrationDate(date);
    }

    return flag;
}

/**
 * @brief   Invalidate Calibratin Data
 * @param   void
 * @retval  void
 */
bool DPV624::invalidateCalibrationData(void)
{
    return persistentStorage->invalidateCalibrationData();
}

/**
 * @brief   writes data over USB
 * @param   buf - pointer to null-terminated character string to transmit
* @param   bufSIze - number of bytes to write
 * @retval  flag - true = success, false = failed
 */
bool DPV624::print(uint8_t *buf, uint32_t bufSize)
{
    bool retStatus = false;
    DDeviceSerial *myCommsMedium;
    myCommsMedium = commsUSB->getMedium();

    if(NULL != buf)
    {
        if(true == isPrintEnable)
        {
            retStatus = myCommsMedium->write(buf, bufSize);
        }

        else
        {
            retStatus = true;
        }
    }

    return retStatus;
}

/**
* @brief   sets isPrintEnable status flag
* @param   newState - true - enable print, flase disable print
* @retval  flag - true = success, false = failed
*/
void DPV624::setPrintEnable(bool newState)
{
    isPrintEnable = newState;
}

/**
* @brief   returns eng mode status
* @param   void
* @retval  true = if  eng mode enable , false = if engmode not enabled
*/
bool DPV624::engModeStatus(void)
{
    return isEngModeEnable;
}
/**
 * @brief   Sets aquisation mode of pressure slot and barometer slot
 * @param   newAcqMode : new Aquisation mode
 * @retval  void
 */
bool DPV624::setAquisationMode(eAquisationMode_t newAcqMode)
{
    bool retStatus = false;

    retStatus = instrument->setAquisationMode(newAcqMode);

    if(true == retStatus)
    {
        if((eAquisationMode_t)E_REQUEST_BASED_ACQ_MODE == newAcqMode)
        {
            isEngModeEnable = true;
        }

        else
        {
            isEngModeEnable = false;
        }
    }

    return retStatus;
}

/**
 * @brief   Sets aquisation mode of pressure slot and barometer slot
 * @param   newAcqMode : new Aquisation mode
 * @retval  void
 */
void DPV624::getPmUpgradePercentage(uint32_t *percentage)
{
    *percentage = pmUpgradePercent;
}

/**
 * @brief   Sets aquisation mode of pressure slot and barometer slot
 * @param   newAcqMode : new Aquisation mode
 * @retval  void
 */
void DPV624::setPmUpgradePercentage(uint32_t percentage)
{
    pmUpgradePercent = percentage;
}

/**
 * @brief   increments the setpoint count and saves into eeprom
 * @param   void
 * @retval  true if saved  sucessfully false if save fails
 */
bool DPV624::incrementSetPointCount(uint32_t *pSetPointCount)
{
    bool successFlag = false;
    uint32_t setPointCount = 0u;

    if(NULL != pSetPointCount)
    {
        //increment set point counts
        if(persistentStorage->incrementSetPointCount(&setPointCount) == true)
        {
            *pSetPointCount = setPointCount;
            successFlag = true;
        }
    }

    return successFlag;
}

/**
 * @brief   retruns set point count
 * @param   void
 * @retval  set Point Count
 */
uint32_t DPV624::getSetPointCount(void)
{
    return persistentStorage->getSetPointCount();
}

/**
 * @brief   get the battery percentage and charginging status
 * @param   *pPercentCapacity    to return percentage capacity
 * @return  *pChargingStatus     to return charging Status
 */
void DPV624::getBatLevelAndChargingStatus(float *pPercentCapacity,
        uint32_t *pChargingStatus)
{
    powerManager->getBatLevelAndChargingStatus(pPercentCapacity,
            pChargingStatus);
}

/**
 * @brief   updates battery status
 * @param   void
 * @return  void
 */
void DPV624::updateBatteryStatus(void)
{
    userInterface->updateBatteryStatus(BATTERY_LEDS_DISPLAY_TIME, BATTERY_LED_UPDATE_RATE);
}

/**
 * @brief   getCommModeStatus bits
 * @param   none
 * @return sInstrumentMode_t
 */
sInstrumentMode_t DPV624::getCommModeStatus(void)
{
    return instrumentMode;
}
/**
 * @brief   setCommModeStatus bits
 * @param   comInterface   OWU/USB/BlueTooth
 * @param   commMode       Local/Remote/Test
 * @return void
 */
void DPV624::setCommModeStatus(eCommInterface_t comInterface, eCommModes_t commMode)
{
    if((eCommInterface_t) E_COMM_OWI_INTERFACE == comInterface)
    {
        switch(commMode)
        {
        case E_COMM_MODE_LOCAL:
            instrumentMode.remoteOwi = 0u;
            break;

        case E_COMM_MODE_REMOTE:
            instrumentMode.remoteOwi = 1u;
            break;

        default:
            break;

        }
    }

    if((eCommInterface_t) E_COMM_BLUETOOTH_INTERFACE == comInterface)
    {
        switch(commMode)
        {
        case E_COMM_MODE_LOCAL:
            instrumentMode.remoteBluetooth = 0u;
            break;

        case E_COMM_MODE_REMOTE:
            instrumentMode.remoteBluetooth = 1u;
            break;

        default:
            break;

        }
    }

    if((eCommInterface_t) E_COMM_USB_INTERFACE == comInterface)
    {
        switch(commMode)
        {
        case E_COMM_MODE_LOCAL:
            instrumentMode.remoteUsb = 0u;
            instrumentMode.remoteUsbTest = 0u;
            break;

        case E_COMM_MODE_REMOTE:
            instrumentMode.remoteUsb = 1u;
            break;

        case E_COMM_MODE_TEST:
            instrumentMode.remoteUsb = 0u;
            instrumentMode.remoteUsbTest = 1u;
            break;

        default:
            break;

        }
    }
}
/**
 * @brief   Get version of specified item
 * @param   item - value: 2 = Board (PCA) - version and IS bit
 *                        3 = Display Board
 *                        5 = Keypad Id
 *
 * @param   itemver - pointer variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getVersion(uint32_t item, uint32_t *itemver)
{
    bool ok = false;

    switch(item)
    {
    case 2:
    {
        *itemver = mainBoardHardwareRevision;
        ok = true;
        break;
    }


    default:
    {
        break;
    }
    }

    return ok;
}

/**
 * @brief   Get board version of main board
 * @param   void
 * @retval  uint32_t
 */
uint32_t DPV624::getBoardRevision(void)
{
    uint32_t itemver;
    const uint32_t boardAndISVersionIndex = 2u;
    const uint32_t versionMask = 0x7u;
    getVersion(boardAndISVersionIndex, &itemver);
    return itemver & versionMask;
}

/**
 * @brief   Starts or stops bluetooth advertising
 * @param   newMode
 * @retval  returns true if suucceeded and false if it fails
 */
bool DPV624::manageBlueToothConnection(eBL652mode_t newMode)
{
    bool statusFlag = true;
    uint32_t retVal = 0u;
    // BT UART Off (for OTA DUCI)
    //commsBluetooth->setTestMode(true);// Test mode / disable / AT mode - stops duci comms on BT interface

    setBluetoothTaskState(E_BL_TASK_SUSPENDED);

    if(false == BL652_initialise(newMode))
    {
        userInterface->bluetoothLedControl(eBlueToothPairing,
                                           E_LED_OPERATION_TOGGLE,
                                           0u,
                                           E_LED_STATE_SWITCH_ON,
                                           UI_DEFAULT_BLINKING_RATE);
        statusFlag = false;
    }

    else
    {
        if(newMode == eBL652_MODE_RUN_INITIATE_ADVERTISING)
        {
            userInterface->bluetoothLedControl(eBlueToothPairing,
                                               E_LED_OPERATION_TOGGLE,
                                               0u,
                                               E_LED_STATE_SWITCH_ON,
                                               UI_DEFAULT_BLINKING_RATE);
            uint32_t sn = 0u;
            sn = persistentStorage->getSerialNumber();
            uint8_t strSerNum[12] = "";
            memset(strSerNum, 12, (size_t)0);
            sprintf((char *)strSerNum, "%010d\r", sn);
            retVal = BL652_startAdvertising(strSerNum);

            if(!retVal)
            {
                blState = BL_STATE_RUN_ADV_IN_PROGRESS;
            }

            setBluetoothTaskState(E_BL_TASK_RUNNING);

        }

        if((newMode == eBL652_MODE_RUN) || (newMode == eBL652_MODE_RUN_DTM))
        {
            // Only allow UART (for OTA DUCI) comms during BT OTA (Ping test)
            setBlStateBasedOnMode(newMode);
            commsBluetooth->setTestMode(false);
        }
    }

    return statusFlag;
}


/**
 * @brief   Delete Error Log file
 * @param   void
 * @retval  returns true if suucceeded and false if it fails
 */

bool DPV624::clearErrorLog(void)
{
    return logger->clearErrorLog();
}
/**
 * @brief   Delete Service Log File
 * @param   void
 * @retval  returns true if suucceeded and false if it fails
 */

bool DPV624::clearServiceLog(void)
{
    return logger->clearServiceLog();
}

/**
* @brief    Checks whether all tasks are running or not
*
* @param    void
*
* @return   returns true if all tasks are running else returns false
*/
bool DPV624::IsAllTasksAreAlive(void)
{
    bool healthy = true; /* Assume All tasks are healthy*/

    for(uint32_t taskNum = eBarometerTask; taskNum < eNumberOfTasks; taskNum++)
    {
        bool isEqualCount = keepAliveCount[taskNum] == keepAlivePreviousCount[taskNum];
        keepAlivePreviousCount[taskNum] = keepAliveCount[taskNum];
        keepAliveIsStuckCount[taskNum] = isEqualCount ? keepAliveIsStuckCount[taskNum] + 1u : 0u;

        if(keepAliveIsStuckCount[taskNum] > stuckTolerance)
        {
            healthy = false;
        }
    }

    return healthy;
}

/**
* @brief    increments the keepAliveCount of requested task
*
* @param    eTaskID_t task ID
*
* @return   void
*/
void DPV624::keepAlive(eTaskID_t taskNum)
{
    if((uint32_t)taskNum < (uint32_t)eNumberOfTasks)
    {
        keepAliveCount[taskNum]++;
    }
}
/**
 * @brief   Get positive fullscale of barometer
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getBaroPosFullscale(float32_t  *fs)
{
    return instrument->getBaroPosFullscale(fs);
}

/**
 * @brief   Get negative fullscale of barometer
 * @param   fs - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getBaroNegFullscale(float32_t  *fs)
{
    return instrument->getBaroNegFullscale(fs);
}

/**
 * @brief   moves the motor till forward end and then return to home position
 * @param   *count - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::moveMotorTillForwardEndThenHome(void)
{
    return instrument->moveMotorTillForwardEndThenHome();
}

/**
 * @brief   moves the motor till reverse end and then return to home position
 * @param   *count - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::moveMotorTillReverseEndThenHome(void)
{
    return instrument->moveMotorTillReverseEndThenHome();
}

/**
 * @brief   Get the connected sensor brand information
 * @param   *count - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getSensorBrandInfo(char *brandMin, char *brandMax, char *brandType, char *brandUnits)
{
    bool success = false;

    instrument->getSensorBrandInfo(brandMin, brandMax, brandType, brandUnits);

    success = true;

    return success;
}

/**
 * @brief   Set next calibration date
 * @param   pointer to date structure
 * @retval  true = success, false = failed
 */
bool DPV624::setNextCalDate(sDate_t *date)
{
    bool flag = false;


    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
        //sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        //calDataBlock->measureBarometer.data.nextCalDate.day = date->day;
        //calDataBlock->measureBarometer.data.nextCalDate.month = date->month;
        //calDataBlock->measureBarometer.data.nextCalDate.year = date->year;

        //flag = persistentStorage->saveCalibrationData();
        flag = persistentStorage->setNextCalDate(date);
    }

    return flag;
}

/**
 * @brief   Get cal date
 * @param   instrument channel
 * @param   pointer to date structure for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getNextCalDate(sDate_t *date)
{
    bool flag = false;

    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
        //sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        //date->day = calDataBlock->measureBarometer.data.nextCalDate.day;
        //date->month = calDataBlock->measureBarometer.data.nextCalDate.month;
        //date->year = calDataBlock->measureBarometer.data.nextCalDate.year;

        //flag = true;
        flag = persistentStorage->getNextCalDate(date);
    }

    return flag;
}

/**
 * @brief   Set cal date
 * @param   pointer to date structure
 * @retval  true = success, false = failed
 */
bool DPV624::setInstrumentCalDate(sDate_t *date)
{
    bool flag = false;


    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
        sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        calDataBlock->calDate.day = date->day;
        calDataBlock->calDate.month = date->month;
        calDataBlock->calDate.year = date->year;

        flag = persistentStorage->saveCalibrationData();

    }

    return flag;
}

/**
 * @brief   Get cal date
 * @param   instrument channel
 * @param   pointer to date structure for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getInstrumentCalDate(sDate_t *date)
{
    bool flag = false;

    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
        sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        date->day = calDataBlock->calDate.day;
        date->month = calDataBlock->calDate.month;
        date->year = calDataBlock->calDate.year;

        flag = true;
    }

    return flag;
}

/**
 * @brief   Log set point infor into service log file
 * @param   setPointCount  : Number set points completed
 * @param   setPointValue  : Current set point value
 * @param   distanceTravelled : distance Travelled till now
 * @retval  true = success, false = failed
*/
bool DPV624::logSetPointInfo(uint32_t setPointCount,
                             float setPointValue,
                             float distanceTravelled)
{
    return logger->logServiceInfo(setPointCount, setPointValue, distanceTravelled);
}

/**
 * @brief   update distance travelled by piston into eeprom
 * @param   float32_t distance travelled
 * @retval  true if saved  sucessfully false if save fails
 */
bool DPV624::updateDistanceTravelled(float32_t distanceTravelled)
{
    bool successFlag = false;

    float32_t oldDistanceTravelled = getDistanceTravelled();
    float32_t newDistancetravelled = oldDistanceTravelled + distanceTravelled;

    if(false == floatEqual(oldDistanceTravelled, newDistancetravelled))
    {
        successFlag = persistentStorage->updateDistanceTravelled(newDistancetravelled);
    }

    else
    {
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   gives distance travelled by the piston
 * @param   void
 * @retval  returns distance travelled by the piston
 */
float32_t DPV624::getDistanceTravelled(void)
{
    return persistentStorage->getDistanceTravelled();
}



/**
* @brief hold stepper micro controller in reset state
* @param void
* @retval void
*/
void DPV624::holdStepperMicroInReset(void)
{
    /* hold the stepper controller micro in reset state */
}


/**
* @brief Reset stepper micro controller
* @param void
* @retval void
*/
void DPV624::releaseStepperMicroReset(void)
{
    /* Reset the stepper controller micro */

}

/**
* @brief stop motor
* @param void
* @retval void
*/
void DPV624::stopMotor(void)
{
    int32_t stepCnt = 0;
    stepperMotor->move(0, &stepCnt);
}

/**
* @brief vent System
* @param void
* @retval void
*/
void DPV624::ventSystem(void)
{
    valve2->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
    valve3->valveTest(E_VALVE_FUNCTION_FORWARD); // isolate pump outlet
    valve1->valveTest(E_VALVE_FUNCTION_REVERSE); // isolate pump inlet
}

/**
* @brief switch USB port configuration between MSC and VCP
* @param void
* @retval void
*/
void DPV624::switchUsbPortConfiguration(void)
{


    int32_t currentUsbMode = getUsbInstrumentPortConfiguration();

    if((eUsbMode_t)E_USBMODE_CDC == currentUsbMode)
    {
        setUsbInstrumentPortConfiguration((int32_t)E_USBMODE_MSC);
    }

    else if((eUsbMode_t)E_USBMODE_MSC == currentUsbMode)
    {
        setUsbInstrumentPortConfiguration((int32_t)E_USBMODE_CDC);
    }

    else
    {
        /* Do Nothing */
    }
}

/**
* @brief get bl652 state
* @param void
* @retval returns current eBL652State_t state
*/
eBL652State_t DPV624::getBlState(void)
{
    return blState;
}

/**
* @brief set bl652 state
* @param eBL652State_t blState
* @retval void
*/
void DPV624::setBlState(eBL652State_t bl652State)
{
    blState = bl652State;
}

/**
* @brief set bl652 state based on mode
* @param eBL652State_t blState
* @retval void
*/
void DPV624::setBlStateBasedOnMode(eBL652mode_t bl652Mode)
{
    // blState = bl652State;

    switch(bl652Mode)
    {
    case eBL652_MODE_DISABLE:
        blState = BL_STATE_DISABLE;
        break;

    case eBL652_MODE_RUN:
        blState = BL_STATE_RUN_STAND_BY;
        break;

    case eBL652_MODE_DTM:
        blState = BL_STATE_DTM;
        break;

    case eBL652_MODE_DEV:
        blState = BL_STATE_DEV_AT_CMDS;
        break;

    case eBL652_MODE_RUN_DTM:
        blState = BL_STATE_RUN_DTM;
        break;

    case eBL652_MODE_TESTING:

        break;

    case eBL652_MODE_ENABLE:
        break;

    default:
        break;
    }
}

/**
* @brief set bl652 state based on mode
* @param eBL652State_t blState
* @retval void
*/
bool DPV624::getSensorZeroValue(uint32_t sensor, float32_t *value)
{
    return instrument->getSensorZeroValue(sensor, value);
}

/**
* @brief Set the distance travelled by the controller
* @param distance - new travelled distance
* @retval void
*/
bool DPV624::setDistanceTravelledByController(float32_t distance)
{
    controllerDistance = controllerDistance + distance;
    return true;
}

/**
* @brief get the distance travelled by the controller
* @param *distance - new travelled distance
* @retval void
*/
bool DPV624::getDistanceTravelledByController(float32_t *distance)
{
    *distance = controllerDistance;
    return true;
}

