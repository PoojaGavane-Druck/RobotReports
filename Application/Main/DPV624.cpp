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

/* Error handler instance parameter starts from 6401 to 6500 */
/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

//#define TEST_MOTOR

#define MOTOR_FREQ_CLK 12000000u
#define BOOTLOADER_IMPLEMENTED 1

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
DPV624::DPV624(void):
    persistentStorage(NULL),
    powerManager(NULL),
    logger(NULL),
    errorHandler(NULL),
    keyHandler(NULL),
    instrument(NULL),
    stepperMotor(NULL),
    commsOwi(NULL),
    commsBluetooth(NULL),
    valve1(NULL),
    valve2(NULL),
    valve3(NULL),
    userInterface(NULL)
{

    isEngModeEnable = false;
    isPrintEnable = false;

    waitOnSecondaryStartup();
    resetQspiFlash();

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

    myMode = E_SYS_MODE_RUN;

    /* Power on or off logic for the PV624. Since the battery could be or could not be connected electrically,
    inserting the battery could cause an electrical connection to be established and start the PV624. In order that
    the PV624 starts up only at the power button press, check that the system reset was caused only by a software
    reset condition and not a power ON reset or a PIN reset.

    In case of either of the two conditions, initialize all the objects but keep the PV624 is shutdown mode

    Also, in case of reset not generated by SW only, only keyHandler and UI tasks could be running, investigate if
    other objects need to be created as well */

    resetToPowerUp = getResetCause();

    i2cInit(&hi2c4);

    persistentStorage = new DPersistent();

    uartInit(&huart2);
    uartInit(&huart3);
    uartInit(&huart4);
    uartInit(&huart5);
}

/**
 * @brief   Create PV624 application objects
 * @param   void
 * @retval  void
 */
void DPV624::createApplicationObjects(void)
{
    OS_ERR os_error = OS_ERR_NONE;

    extStorage = new DExtStorage(&os_error);
    handleOSError(&os_error);

    powerManager = new DPowerManager(&hsmbus1, &os_error);
    handleOSError(&os_error);

    logger = new DLogger(&os_error);
    handleOSError(&os_error);

    errorHandler = new DErrorHandler(&os_error);
    handleOSError(&os_error);

    keyHandler = new DKeyHandler(&os_error);
    handleOSError(&os_error);

    instrument = new DInstrument(&os_error);
    handleOSError(&os_error);

    stepperMotor = new DStepperMotor();

    commsOwi = new DCommsOwi("commsOwi", &os_error);
    handleOSError(&os_error);

    commsUSB = new DCommsUSB("commsUSB", &os_error);
    handleOSError(&os_error);

    commsBluetooth = new DCommsBluetooth("commsBLE", &os_error);
    handleOSError(&os_error);

    valve1 = new DValve(&htim1,
                        TIM1,
                        TIM_CHANNEL_1,
                        VALVE1_DIR_PC7_GPIO_Port,
                        VALVE1_DIR_PC7_Pin,
                        VALVE1_ENABLE_GPIO_Port,
                        VALVE1_ENABLE_Pin,
                        0u);


    valve2 = new DValve(&htim5,
                        TIM5,
                        TIM_CHANNEL_2,
                        VALVE2_DIR_PC8_GPIO_Port,
                        VALVE2_DIR_PC8_Pin,
                        VALVE2_ENABLE_GPIO_Port,
                        VALVE2_ENABLE_Pin,
                        0u);


    valve3 = new DValve(&htim3,
                        TIM3,
                        TIM_CHANNEL_1,
                        VALVE3_DIR_PF3_GPIO_Port,
                        VALVE3_DIR_PF3_Pin,
                        VALVE3_ENABLE_GPIO_Port,
                        VALVE3_ENABLE_Pin,
                        1u);



    // Start the UI task first
    userInterface = new DUserInterface(&os_error);
    handleOSError(&os_error);

    setPowerState(resetToPowerUp);

    // enable deferred IWDG now after posssible FW upgrade is complete
    EnableDeferredIWDG();

    initClassVariables();
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
 * @brief   initializes all the class variables used by the PV624
 * @param   void
 * @retval  void
 */
void DPV624::initClassVariables(void)
{
    float32_t distance = 0.0f;

    distance = getDistanceTravelled();

    setDistanceTravelledByController(distance);
}

/**
 * @brief   Checks and returns the cause of micro controller reset
 * @param   void
 * @retval  E_POWER_STATE_ON - if SW reset is the cause, E_POWER_STATE_OFF - all other times
 */
ePowerState_t DPV624::getResetCause(void)
{
    ePowerState_t state = E_POWER_STATE_OFF;     // Always OFF, only true if reset caused only by SW

    uint32_t rccReset = 0u;

    rccReset = RCC->CSR;                    // Read the micro controller RCC register reset flags
    rccReset = rccReset & RESET_REM_MASK;   // Mask bottom part of the register value

    if((RESET_SW | RESET_PIN | RESET_POR) == rccReset)
    {
        // Reset was only caused by software
        state = E_POWER_STATE_ON;
    }

    return state;
}

/**
 * @brief   Wait for secondary micro controller to startup after issuing a reset
 * @param   void
 * @retval  void
 */
void DPV624::waitOnSecondaryStartup(void)
{
    uint32_t pinVal = 0u;
    uint32_t timeout = 0u;

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay(10u);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);

    while((0u == pinVal) && (timeout < 10u))
    {
        pinVal = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8);
        timeout = timeout + 1u;
        HAL_Delay(25u);
    }

    if(1u == pinVal)
    {
        pinVal = 0u;
    }
}

/**
 * @brief   Resets the QSPI flash memory chip
 * @param   void
 * @retval  void
 */
void DPV624::resetQspiFlash(void)
{
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_Delay(100u);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_SET);
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
        // Handle errors only when PV624 is powered up
        if((ePowerState_t)(E_POWER_STATE_ON) == myPowerState)
        {
            errorHandler->handleError(errorCode,
                                      errStatus,
                                      paramValue,
                                      errInstance,
                                      isFatal);
        }
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
        if((ePowerState_t)(E_POWER_STATE_ON) == myPowerState)
        {
            errorHandler->handleError(errorCode,
                                      errStatus,
                                      paramValue,
                                      errInstance,
                                      isFatal);
        }
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
        osErrStatusDuringObectsCreation = eSetError;
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
 * @param   uint32_t tells which serial number requested (PM620 or PV624)
 * @retval  uint32_t returns requested serial number
 */
uint32_t DPV624::getSerialNumber(uint32_t snType)
{
    uint32_t sn = 0u;

    if((uint32_t)E_ITEM_PV624 == snType)
    {
        sn =  persistentStorage->getSerialNumber();
    }

    else if((uint32_t)E_ITEM_PM620 == snType)
    {
        instrument->getSensorSerialNumber(&sn);
    }

    else
    {
        /*do nothing */
    }

    return sn;
}

/**
 * @brief   Sets PV624 serial number
 * @uint32_t serial number to be set
 * @retval  true = success, false = failed
 */
bool DPV624::setSerialNumber(uint32_t newSerialNumber)
{
    bool successFlag = false;

    if(newSerialNumber != 0XFFFFFFFFu)  //one byte less for null terminator
    {
        successFlag = persistentStorage->setSerialNumber(newSerialNumber);

        if(!successFlag)
        {
            handleError(E_ERROR_EEPROM,
                        eSetError,
                        0u,
                        6406u);
        }

        else
        {
            handleError(E_ERROR_EEPROM,
                        eClearError,
                        0u,
                        6407u);
        }
    }

    return successFlag;
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
 * @brief   Get Date in RTC
 * @param   date - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getDate(sDate_t *date)
{
    bool successFlag = false;

    if(NULL != date)
    {
        successFlag = getSystemDate(date);
    }

    return successFlag;
}

/**
 * @brief   Set Date in RTC
 * @param   date - user specified date to set
 * @retval  true = success, false = failed
 */
bool DPV624::setDate(sDate_t *date)
{
    bool successFlag = false;
    bool calDueStatus = false;

    if(NULL != date)
    {
        successFlag = setSystemDate(date);

        if(successFlag)
        {
            successFlag = isBarometerDueForCalibration(&calDueStatus);

            if(successFlag)
            {
                if(calDueStatus)
                {
                    handleError(E_ERROR_BAROMETER_OUT_OF_CAL,
                                eSetError,
                                0u,
                                6405u);
                }
            }
        }
    }

    return successFlag;
}

/**
 * @brief   Get system time from RTC
 * @param   time - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getTime(sTime_t *time)
{
    bool successFlag = false;

    if(NULL != time)
    {
        successFlag = getSystemTime(time);
    }

    return successFlag;
}

/**
 * @brief   Set system time in RTC
 * @param   time - pointer to system time structure containing time to set the RTC
 * @retval  true = success, false = failed
 */
bool DPV624::setTime(sTime_t *time)
{
    bool successFlag = false;

    if(NULL != time)
    {
        successFlag = setSystemTime(time);
    }

    return successFlag;
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

    if(item < E_ITEM_MAX)
    {

        if((uint32_t)E_ITEM_PV624 == item)
        {
            switch(component)
            {
            case E_COMPONENENT_APPLICATION:
            {
                snprintf(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)cAppVersion[1], (uint8_t)cAppVersion[2], (uint8_t)cAppVersion[3]);
                status = true;
                break;
            }

            case E_COMPONENENT_BOOTLOADER:
            {
                snprintf(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)cblVersion[1], (uint8_t)cblVersion[2], (uint8_t)cblVersion[3]);
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

        else if((uint32_t)E_ITEM_PM620 == item)
        {
            uSensorIdentity_t identity;

            switch(component)
            {
            case E_COMPONENENT_APPLICATION:
            {
                status = instrument->getExternalSensorAppIdentity(&identity);

                if(status)
                {
                    snprintf(itemverStr, 10u, "%02d.%02d.%02d", identity.major, identity.minor, identity.build);
                }

                status = true;
                break;
            }

            case E_COMPONENENT_BOOTLOADER:
            {
                status = instrument->getExternalSensorBootLoaderIdentity(&identity);

                if(status)
                {
                    snprintf(itemverStr, 10u, "%02d.%02d.%02d",
                             identity.major, identity.minor, identity.build);
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

        else if((uint32_t)E_ITEM_PV624_2ND_MICRO == item)
        {
            switch(component)
            {
            case E_COMPONENENT_APPLICATION:
            {
                sVersion_t secondaryAppVersion;
                secondaryAppVersion.all = 0u;
                stepperMotor->getAppVersion(&secondaryAppVersion);
                snprintf(itemverStr, 10u, "%02d.%02d.%02d",
                         (uint8_t)secondaryAppVersion.major,
                         (uint8_t)secondaryAppVersion.minor,
                         (uint8_t)secondaryAppVersion.build);
                status = true;
                break;
            }

            case E_COMPONENENT_BOOTLOADER:
            {
                sVersion_t secondaryBootVersion;
                secondaryBootVersion.all = 0u;
                stepperMotor->getBootVersion(&secondaryBootVersion);
                snprintf(itemverStr, 10u, "%02d.%02d.%02d",
                         (uint8_t)secondaryBootVersion.major,
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
            /* Do nothing */
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

    if(item < (uint32_t)E_ITEM_MAX)
    {

        if((uint32_t)E_ITEM_PV624 == item)
        {
            switch(component)
            {
            case E_COMPONENENT_APPLICATION:
            {
                snprintf(dkStr, 7u, "%04d", cAppDK);
                status = true;
                break;
            }

            case E_COMPONENENT_BOOTLOADER:
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

        else if((uint32_t)E_ITEM_PM620 == item)
        {
            uSensorIdentity_t identity;

            switch(component)
            {
            case E_COMPONENENT_APPLICATION:
            {
                status = instrument->getExternalSensorAppIdentity(&identity);

                if(status)
                {
                    snprintf(dkStr, 7u, "%04d", identity.dk);
                }

                status = true;
                break;
            }

            case E_COMPONENENT_BOOTLOADER:
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

        else if((uint32_t)E_ITEM_PV624_2ND_MICRO == item)
        {
            switch(component)
            {
            case E_COMPONENENT_APPLICATION:
            {
                uint32_t secondaryAppDk = 0u;

                stepperMotor->getAppDk(&secondaryAppDk);
                snprintf(dkStr, 7u, "%04d", secondaryAppDk);
                status = true;
                break;
            }

            case E_COMPONENENT_BOOTLOADER:
            {
                uint32_t secondaryBootDk = 0u;

                stepperMotor->getBootDk(&secondaryBootDk);
                snprintf(dkStr, 7u, "%04d", secondaryBootDk);
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
            /* Do nothing */
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
    bool successFlag = false;

    if(NULL != fs)
    {
        successFlag =  instrument->getNegFullscale(fs);
    }

    return successFlag;
}

/**
 * @brief   Get sensor type
 * @param   sensorType - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getSensorType(eSensorType_t *sensorType)
{
    bool successFlag = false;

    if(NULL != sensorType)
    {
        successFlag = instrument->getSensorType(sensorType);
    }

    return successFlag;
}

/**
 * @brief   Get sensor type for engineering protocol
 * @param   sensorType - pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getPM620Type(uint32_t *sensorType)
{
    bool successFlag = false;

    if(NULL != sensorType)
    {
        successFlag =  instrument->getPM620Type(sensorType);
    }

    return successFlag;
}

/**
 * @brief   Get controller mode
 * @param   controller mode - pointer to variable for return value
 * @retval  true = success, false = failed
*/
bool DPV624::getControllerMode(eControllerMode_t *controllerMode)
{
    bool successFlag = false;

    if(NULL != controllerMode)
    {
        successFlag =   instrument->getControllerMode(controllerMode);
    }

    return successFlag;
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

    bool successFlag = false;

    if(NULL != rate)
    {
        successFlag =   instrument->getVentRate(rate);
    }

    return successFlag;
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
 * @param   uint32_t sensor type (PM620 or Barometer)
 * @param   interval is pointer to variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getCalInterval(uint32_t sensor, uint32_t *interval)
{
    bool successFlag = false;

    if(NULL != interval)
    {
        if((uint32_t)E_BAROMETER_SENSOR == sensor)
        {
            //"channel" can only be 0 if updating the instrument-wide cal interval
            if(NULL != interval)
            {
                *interval = persistentStorage->getCalInterval();
                successFlag = true;
            }
        }

        else
        {
            //set cal interval for the sensor being calibrated
            successFlag = instrument->getCalInterval(interval);
        }
    }

    return successFlag;
}

/**
 * @brief   set cal interval
 * @param   interval  value
 * @retval  true = success, false = failed
 */
bool DPV624::setCalInterval(uint32_t sensor, uint32_t interval)
{
    bool successFlag = false;

    if((uint32_t)E_BAROMETER_SENSOR  ==  sensor)
    {
        successFlag = persistentStorage->setCalInterval(interval);

        if(!successFlag)
        {
            handleError(E_ERROR_EEPROM,
                        eSetError,
                        0u,
                        6408u);
        }

        else
        {
            handleError(E_ERROR_EEPROM,
                        eClearError,
                        0u,
                        6409u);
        }

        if(true == successFlag)
        {
            successFlag = instrument->setCalInterval(sensor, interval);
        }

    }

    else
    {
        //Not allowed to write PM620 Calibration interval
        successFlag = false;
    }

    return successFlag;
}

/**
 * @brief   get pressure set point value
 * @param   float * pointer a float data type to return pressure set point value
 * @retval  true = success, false = failed
 */
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

/**
 * @brief   set Pressure set point value
 * @param   float   set point value
 * @retval  true = success, false = failed
 */
bool DPV624::setPressureSetPoint(float setPointValue)
{
    bool successFlag = false;

    successFlag = instrument->setPressureSetPoint(setPointValue);

    return successFlag;
}

/**
 * @brief   get Instrument function
 * @param   func is the function itself
 * @retval  true if activated successfully, else false
 */
bool DPV624::getFunction(eFunction_t *pFunc)
{
    bool successFlag = false;

    if(NULL != pFunc)
    {
        instrument->getFunction(pFunc);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   It instruct to take new reading to PM620
 * @param   uint32 sample rate at which data aquisition to be done
 * @retval  void
 */
void DPV624::takeNewReading(uint32_t rate)
{
    instrument->takeNewReading(rate);
}

/**
 * @brief   Set Controller Status
 * @param   uint32_t status to set
 * @retval  true = success, false = failed
 */
bool DPV624::setControllerStatus(uint32_t statusInfo)
{
    return instrument->setControllerStatus(statusInfo);

}

/**
 * @brief   Get Controller Status
 * @param   uint32_t * pointer to variable to return controller status
 * @retval  true = success, false = failed
 */
bool DPV624::getControllerStatus(uint32_t *controllerStatus)
{
    bool successFlag = false;

    if(NULL != controllerStatus)
    {
        successFlag =  instrument->getControllerStatus(controllerStatus);
    }

    return successFlag;
}



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
    bool successFlag = false;

    //can only change mode if either current mode or new mode is E_PIN_MODE_NONE;
    //ie cannot switch modes without going through 'no pin' mode
    if((myPinMode == (ePinMode_t)E_PIN_MODE_NONE) || (mode == (ePinMode_t)E_PIN_MODE_NONE))
    {
        myPinMode = mode;
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Set Instrument Port Configuration for USB
 * @param   mode - 0 for communications mode; 1 for storage mode
 * @retval  none
 */
void DPV624::setUsbInstrumentPortConfiguration(int32_t mode)
{
    if(mode <= (int32_t)E_USBMODE_MSC)
    {
        MX_USB_DEVICE_SetUsbMode((eUsbMode_t)mode);
    }
}

/**
 * @brief   Get Instrument Port Configuration for USB
 * @param   none
 * @retval  mode - 0 for communications mode; 1 for storage mode
 */
int32_t DPV624::getUsbInstrumentPortConfiguration()
{
    return (int32_t)MX_USB_DEVICE_GetUsbMode();
}

/**
 * @brief   Get cal date
 * @param   pointer to date structure for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getManufactureDate(sDate_t *date)
{
    bool successFlag = false;

    if(NULL != date)
    {
        successFlag = persistentStorage->getManufacturingDate(date);
    }

    return successFlag;
}

/**
 * @brief   Set cal date
 * @param   pointer to date structure
 * @retval  true = success, false = failed
 */
bool DPV624::setCalDate(sDate_t *date)
{
    bool successFlag = false;

    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
        successFlag = persistentStorage->setCalibrationDate(date);

        if(!successFlag)
        {
            handleError(E_ERROR_EEPROM,
                        eSetError,
                        0u,
                        6410u);
        }

        else
        {
            handleError(E_ERROR_EEPROM,
                        eClearError,
                        0u,
                        6411u);
        }
    }



    return successFlag;
}

/**
 * @brief   Set cal date
 * @param   pointer to date structure
 * @retval  true = success, false = failed
 */
bool DPV624::setManufactureDate(sDate_t *date)
{
    bool successFlag = false;

    if(NULL != date)
    {
        manufactureDate.day = date->day;
        manufactureDate.month = date->month;
        manufactureDate.year = date->year;

        sDate_t curDate;
        successFlag = getSystemDate(&curDate);


        if(successFlag)
        {
            int32_t numOfDays = 0;
            numOfDays = getDateDiff(&curDate, &manufactureDate);

            if((numOfDays <= 0) && (manufactureDate.year >= MIN_ALLOWED_YEAR)) //Manufacturing date should not be greater than current date
            {
                successFlag = persistentStorage->setManufacturingDate(date);

                if(!successFlag)
                {
                    handleError(E_ERROR_EEPROM,
                                eSetError,
                                0u,
                                6412u);
                }

                else
                {
                    handleError(E_ERROR_EEPROM,
                                eClearError,
                                0u,
                                6413u);
                }
            }
        }
    }

    return successFlag;
}

/**
 * @brief   Set calibration type
 * @param   calType - function specific calibration type (0 = user calibration)
 * @retval  true = success, false = failed
 */
bool DPV624::setCalibrationType(int32_t calType, uint32_t range)
{
    bool successFlag = false;
    successFlag = instrument->setCalibrationType(calType, range);
    return successFlag;
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
    bool successFlag = false;
    successFlag = instrument->abortCalibration();
    return successFlag;
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
    float  percentCap = 0.0f;
    uint32_t chargingStatus = 0u;
    getBatLevelAndChargingStatus((float *)&percentCap, &chargingStatus);

    // Do Firmware Upgrade only if Battery Capacity is >25%
    if((float32_t)(BATTERY_CAP_25_PC) < percentCap)
    {
        ok = false;
    }

    else
    {
        ok = true;
    }

    if(ok)
    {
        if((eSysMode_t)E_SYS_MODE_RUN == getSysMode())
        {
            setSysMode(E_SYS_MODE_FW_UPGRADE);
            ok &= extStorage->upgradeFirmware(EV_FLAG_FW_VALIDATE_AND_UPGRADE);
            setSysMode(E_SYS_MODE_RUN);
        }
    }

    if(ok)
    {
        // set flag to upgrade after reset
        //persistentStorage->setFWUpgradePending(true);

        // Wait for instrument to shutdown
        //performShutdown(E_SHUTDOWN_FW_UPGRADE);
    }

    return ok;

}

/**
 * @brief   Perform application firmware upgrade
 * @param   void
 * @retval  flag: true if ok, else false
 */
bool DPV624::performPM620tUpgrade(void)
{
    bool successFlag = true;
    successFlag = instrument->upgradeSensorFirmware();
    return successFlag;
}

/*****************************************************************************/
/* SUPPRESS: floating point values shall not be tested for exact equality or
inequality (MISRA C 2004 rule 13.3) */
/*****************************************************************************/
_Pragma("diag_suppress=Pm046")
/**
 * @brief   Set  sensor zero value
 * @param   value - user provided value to set as the zero
 * @param   uint32_t - tells sensor type Barometer sensor or PM620
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
 * @brief   Save current cal as backup
 * @param   void
 * @retval  successFlag: true = success, false = failed
 */
bool DPV624::backupCalDataSave(void)
{
    bool successFlag = false;

    if(persistentStorage != NULL)
    {
        successFlag = persistentStorage->saveAsBackupCalibration();

        if(!successFlag)
        {
            handleError(E_ERROR_EEPROM,
                        eSetError,
                        0u,
                        6414u);
        }

        else
        {
            handleError(E_ERROR_EEPROM,
                        eClearError,
                        0u,
                        6415u);
        }
    }

    return successFlag;
}


/**
 * @brief   Restore backup cal as current
 * @param   void
 * @retval  successFlag: true = success, false = failed
 */
bool DPV624::backupCalDataRestore(void)
{
    bool successFlag = false;

    if(NULL != persistentStorage)
    {
        successFlag = persistentStorage->loadBackupCalibration();

        if(instrument != NULL)
        {
            successFlag &= instrument->reloadCalibration();

            if(successFlag)
            {
                handleError(E_ERROR_BAROMETER_CAL_DEFAULT,
                            eSetError,
                            0u,
                            6403u);
            }
        }
    }

    return successFlag;
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
    bool successFlag = false;

    if(NULL != samples)
    {
        successFlag =  instrument->getCalSamplesRemaining(samples);
    }

    return successFlag;
}

/**
 * @brief   Get required number of calibration points
 * @param   eSensor_t sensor type (Barometer or PM620)
 * @param   uint32_t * - pointer to variable for return  number of cal points
 * @retval  true = success, false = failed
 */
bool DPV624::getRequiredNumCalPoints(eSensor_t sensorType, uint32_t *numCalPoints)
{
    bool successFlag = false;

    if(NULL != numCalPoints)
    {
        successFlag = instrument->getRequiredNumCalPoints(sensorType, numCalPoints);
    }

    return successFlag;
}

/**
 * @brief   Get  sensor zero value
 * @param   value - pointer to variable for return zero offset value
 * @retval  true = success, false = failed
 */
bool DPV624::getZero(float32_t *value)
{
    bool successFlag = false;

    if(NULL != value)
    {
        successFlag =  instrument->getSensorZeroValue(0u, value);
    }

    return successFlag;
}

/**
 * @brief   Get cal date
 * @param   pointer to date structure to return calibration date
 * @retval  true = success, false = failed
 */
bool DPV624::getCalDate(sDate_t *date)
{
    bool successFlag = false;

    if(NULL != date)
    {
        successFlag = persistentStorage->getCalibrationDate(date);
    }

    return successFlag;
}

/**
 * @brief   Invalidate Calibratin Data
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::invalidateCalibrationData(void)
{
    bool successFlag = false;
    successFlag = persistentStorage->invalidateCalibrationData();

    if(!successFlag)
    {
        handleError(E_ERROR_EEPROM,
                    eSetError,
                    0u,
                    6423u);
    }

    else
    {
        handleError(E_ERROR_EEPROM,
                    eClearError,
                    0u,
                    6424u);
    }

    return successFlag;
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
* @retval  void
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
 * @param   acqMode : Aquisation mode to set
 * @retval  successFlag - true = success, false = failed
 */
bool DPV624::setAquisationMode(eAquisationMode_t acqMode)
{
    bool successFlag = false;

    successFlag = instrument->setAquisationMode(acqMode);

    if(true == successFlag)
    {
        if((eAquisationMode_t)E_REQUEST_BASED_ACQ_MODE == acqMode)
        {
            isEngModeEnable = true;
        }

        else
        {
            isEngModeEnable = false;
        }
    }

    return successFlag;
}

/**
 * @brief   gets the completion status of PM620 firmware upgrade interms of percentage
 * @param   uint32_t * : pointer to variable to return how much percentage completed
            uint32_t *upgradeStatus - pointer to the variable to see if upgrade has succeeded
 * @retval  void
 */
void DPV624::getPmUpgradePercentage(uint32_t *percentage, uint32_t *upgradeStatus)
{
    bool successFlag = false;

    if(NULL != percentage)
    {
        *percentage = pmUpgradePercent;
        *upgradeStatus = pmUpgradeStatus;
    }
}

/**
 * @brief   sets the completion status of PM620 firmware upgrade interms of percentage
 * @param   uint32_t how much percentage completed
            uint32_t upgradeStatus - Upgrade status at that percentage 1 - pass, 0m - fail
 * @retval  void
 */
void DPV624::setPmUpgradePercentage(uint32_t percentage, uint32_t upgradeStatus)
{
    pmUpgradePercent = percentage;
    pmUpgradeStatus = upgradeStatus;
}

/**
 * @brief   increments the setpoint count and saves into eeprom
 * @param   uint32_t * pointer to variable to return set point count value after increment
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

        if(!successFlag)
        {
            handleError(E_ERROR_EEPROM,
                        eSetError,
                        0u,
                        6416u);
        }

        if(getSetPointCount() >= MAX_ALLOWED_SET_POINT_COUNT)
        {
            errorHandler->updateDeviceStatus(E_ERROR_DEVICE_DUE_FOR_SERVICE, eSetError);
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
 * @param   *pChargingStatus     to return charging Status
 * @retval  void
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
    bool successFlag = false;

    if(NULL != itemver)
    {
        switch(item)
        {
        case E_COMPONENENT_BOARD_OR_OS:
        {
            *itemver = mainBoardHardwareRevision;
            successFlag = true;
            break;
        }


        default:
        {
            break;
        }
        }
    }

    return successFlag;
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
 * @param   fs - pointer to variable to return barometer positive full scale value
 * @retval  true = success, false = failed
 */
bool DPV624::getBaroPosFullscale(float32_t  *fs)
{
    bool successFlag = false;

    if(NULL != fs)
    {
        successFlag = instrument->getBaroPosFullscale(fs);
    }

    return successFlag;
}

/**
 * @brief   Get negative fullscale of barometer
 * @param   fs - pointer to variable to return barometer negative full scale value
 * @retval  true = success, false = failed
 */
bool DPV624::getBaroNegFullscale(float32_t  *fs)
{
    bool successFlag = false;

    if(NULL != fs)
    {
        successFlag = instrument->getBaroNegFullscale(fs);
    }

    return successFlag;
}

/**
 * @brief   moves the motor till forward end
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::moveMotorTillForwardEnd(void)
{
    return instrument->moveMotorTillForwardEnd();
}

/**
 * @brief   moves the motor till reverse end
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::moveMotorTillReverseEnd(void)
{
    return instrument->moveMotorTillReverseEnd();
}

/**
 * @brief   moves the motor till forward end and then return to home position
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::moveMotorTillForwardEndThenHome(void)
{
    return instrument->moveMotorTillForwardEndThenHome();
}

/**
 * @brief   moves the motor till reverse end and then return to home position
 * @param   void
 * @retval  true = success, false = failed
 */
bool DPV624::moveMotorTillReverseEndThenHome(void)
{
    return instrument->moveMotorTillReverseEndThenHome();
}

/**
 * @brief   Get the connected sensor brand information
 * @param   *brandMin - pointer to variable to return min value string
 * @param   *brandMin - pointer to variable to return max value string
 * @param   *brandMin - pointer to variable to return sensor type
 * @param   *brandMin - pointer to variable to return units supported by sensor
 * @retval  true = success, false = failed
 */
bool DPV624::getSensorBrandInfo(char *brandMin,
                                char *brandMax,
                                char *brandType,
                                char *brandUnits)
{
    bool successFlag = false;

    if((NULL != brandMin) &&
            (NULL != brandMax) &&
            (NULL != brandType) &&
            (NULL != brandUnits))
    {
        instrument->getSensorBrandInfo(brandMin, brandMax, brandType, brandUnits);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Set next calibration date
 * @param   pointer to date structure contains next calibration date
 * @retval  true = success, false = failed
 */
bool DPV624::setNextCalDate(sDate_t *date)
{
    bool successFlag = false;


    if(NULL != date)
    {
        sDate_t calDate;
        successFlag = PV624->getCalDate(&calDate);

        if(successFlag)
        {
            int32_t numOfDays = 0;
            numOfDays = getDateDiff(&calDate, date);

            /*Date dif between cal dand next cal NEXT cal date
              should be less than or equal to  max Cal Interval */
            if((numOfDays > 0) &&
                    (numOfDays <= (int32_t)MAX_CAL_INTERVAL) &&
                    (manufactureDate.year >= MIN_ALLOWED_YEAR) &&
                    (manufactureDate.year <= MIN_ALLOWED_YEAR))
            {
                successFlag = persistentStorage->setNextCalDate(date);

                if(!successFlag)
                {
                    handleError(E_ERROR_EEPROM,
                                eSetError,
                                0u,
                                6417u);
                }

                else
                {
                    handleError(E_ERROR_EEPROM,
                                eClearError,
                                0u,
                                6418u);
                }
            }
        }
    }

    return successFlag;
}

/**
 * @brief   Get next calibration date
 * @param   pointer to date structure to return next calibration date
 * @retval  true = success, false = failed
 */
bool DPV624::getNextCalDate(sDate_t *date)
{
    bool successFlag = false;

    if(NULL != date)
    {
        successFlag = persistentStorage->getNextCalDate(date);
    }

    return successFlag;
}

/**
 * @brief   Set instrument calibration date
 * @param   pointer to date structure contains instrument calibration date
 * @retval  true = success, false = failed
 */
bool DPV624::setInstrumentCalDate(sDate_t *date)
{
    bool successFlag = false;


    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
        sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        calDataBlock->calDate.day = date->day;
        calDataBlock->calDate.month = date->month;
        calDataBlock->calDate.year = date->year;

        successFlag = persistentStorage->saveCalibrationData();

        if(!successFlag)
        {
            handleError(E_ERROR_EEPROM,
                        eSetError,
                        0u,
                        6419u);
        }

        else
        {
            handleError(E_ERROR_EEPROM,
                        eClearError,
                        0u,
                        6420u);
        }

    }

    return successFlag;
}

/**
 * @brief   get instrument calibration date
 * @param   pointer to date structure to return instrument calibration date
 * @retval  true = success, false = failed
 */
bool DPV624::getInstrumentCalDate(sDate_t *date)
{
    bool successFlag = false;

    if(NULL != date)
    {
        //get address of calibration data structure in persistent storage
        sCalData_t *calDataBlock = persistentStorage->getCalDataAddr();

        date->day = calDataBlock->calDate.day;
        date->month = calDataBlock->calDate.month;
        date->year = calDataBlock->calDate.year;

        successFlag = true;
    }

    return successFlag;
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
        /* Also update the total distance travelled value */
        setDistanceTravelledByController(distanceTravelled);
        successFlag = persistentStorage->updateDistanceTravelled(newDistancetravelled);

        if(!successFlag)
        {
            handleError(E_ERROR_EEPROM,
                        eSetError,
                        0u,
                        6421u);
        }

        else
        {
            handleError(E_ERROR_EEPROM,
                        eClearError,
                        0u,
                        6422u);
        }
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
void DPV624::openVent(void)
{
    valve2->valveTest(VALVE_STATE_ON); // isolate pump outlet
    valve3->valveTest(VALVE_STATE_ON); // isolate pump outlet
    valve1->valveTest(VALVE_STATE_OFF); // isolate pump inlet
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
    if(bl652State <= (eBL652State_t)BL_STATE_MAX)
    {
        blState = bl652State;
    }
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
* @brief get sensor zero value
* @param uint32_t sensor type (Barometer or PM620)
* @param float32_t* pointer to return sensor zero value
* @retval  true = success, false = failed
*/
bool DPV624::getSensorZeroValue(uint32_t sensor, float32_t *value)
{
    bool successFlag = false;

    if(NULL != value)
    {
        successFlag = instrument->getSensorZeroValue(sensor, value);
    }

    return successFlag;
}

/**
* @brief Set the distance travelled by the controller
* @param distance - new travelled distance
* @retval  true = success, false = failed
*/
bool DPV624::resetDistanceTravelledByController(void)
{
    controllerDistance = 0.0f;
    return true;
}

/**
* @brief Set the distance travelled by the controller
* @param distance - new travelled distance
* @retval  true = success, false = failed
*/
bool DPV624::setDistanceTravelledByController(float32_t distance)
{
    controllerDistance = controllerDistance + distance;
    return true;
}

/**
* @brief get the distance travelled by the controller
* @param *distance - new travelled distance
* @retval  true = success, false = failed
*/
bool DPV624::getDistanceTravelledByController(float32_t *distance)
{
    bool successFlag = false;

    if(NULL != distance)
    {
        *distance = controllerDistance;
        successFlag = true;
    }

    return successFlag;
}

/**
* @brief    Sends a command and Fw Upgrade to Secondary uC
* @param    txData pointer to the image data
* @param    image length
* @param    *response pointer to variable to return response
* @retval   error status
*/
eMotorError_t DPV624::secondaryUcFwUpgrade(uint8_t *txData,
        uint8_t dataLength,
        uint8_t *response)
{
    eMotorError_t error = eMotorError;

    if((NULL != txData) &&
            (NULL != response))
    {
        error = stepperMotor->secondaryUcFwUpgrade(txData, dataLength, response);
    }

    return error;
}

/**
* @brief    Sends a command and Fw Upgrade to Secondary uC
* @param    uint32_t it contains secondary micro firmware image size
* @retval   error status
*/
eMotorError_t DPV624::secondaryUcFwUpgradeCmd(uint32_t fileSize,
        uint8_t *responseAck)
{
    eMotorError_t error = eMotorError;

    if(NULL != responseAck)
    {
        error = stepperMotor->secondaryUcFwUpgradeCmd(fileSize, responseAck);
    }

    return error;
}

/**
* @brief    tells device is due for service or not
* @param    void
* @retval   returns true if device due for service otherwise returns false
*/
bool DPV624::isDeviceDueForService(void)
{
    bool successFlag = false;
    uint32_t setPtCnt = 0u;
    setPtCnt = getSetPointCount();

    if(setPtCnt >= MAX_ALLOWED_SET_POINT_COUNT)
    {
        errorHandler->updateDeviceStatus(E_ERROR_DEVICE_DUE_FOR_SERVICE, eSetError);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   clears set point count and distance travelled
 * @param   void
 * @retval  true if cleared sucessfully otherwise returns false
 */
bool DPV624::clearMaintainceData(void)
{
    bool successFlag = false;
    successFlag = persistentStorage->clearMaintainceData();

    if(!successFlag)
    {

        handleError(E_ERROR_EEPROM,
                    eSetError,
                    0u,
                    6425u);
    }

    else
    {
        controllerDistance = 0.0f;

        handleError(E_ERROR_EEPROM,
                    eClearError,
                    0u,
                    6426u);
    }

    return successFlag;
}

/**
 * @brief   get barometer calibration offsets
 * @param   float* pointer to float array to return calibration offsets
 * @retval  true = success, false = failed
 */
bool DPV624::getCalOffsets(float32_t *pCalOffsets)
{
    bool successFlag = false;

    if(NULL != pCalOffsets)
    {
        persistentStorage->getCalOffsets(pCalOffsets);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief  This function checks if optical board is available and connected to PV624
 * @param  void
 * @retval true = success, false = failed
 */
void DPV624::setOpticalBoardStatus(void)
{

    // Read optical board GPIO to check if board is available
    optBoardStatus = !(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_14));

    if(0u == optBoardStatus)
    {
        // GPIO is high, board is not available, set error
        handleError(E_ERROR_OPTICAL_BOARD_NOT_FOUND,
                    eSetError,
                    0u,
                    6402u,
                    true);
    }


}

/**
 * @brief  This function checks if optical board is available and connected to PV624
 * @param void
 * @retval returns 1 if board is present and 0 if board is failed
 */
uint32_t DPV624::getOpticalBoardStatus(void)
{
    return optBoardStatus;
}

/**
 * @brief  This function whether barometer is due for calibration or not
 * @param bool * pointer to variable to return calibration due status :
 *               true means due for calibration
 *               false means not due for calibration,
 * @retval bool  return function exection status: True for success otherwise returns false
 */
bool DPV624::isBarometerDueForCalibration(bool *calDueStatus)
{
    bool successFlag = false;
    sTime_t timeVal;
    sDate_t sysDate;
    sDate_t baroCalDate;
    uint32_t sysDateInEpocheFormat = 0u;
    uint32_t baroCalDateInEpochFormat = 0u;
    uint32_t calIntervalDays = 0u;
    timeVal.hours = 0u;
    timeVal.minutes = 0u;
    timeVal.seconds = 0u;

    successFlag = getDate(&sysDate);

    if(successFlag)
    {
        successFlag = getCalDate(&baroCalDate);
    }

    if(successFlag)
    {
        successFlag = getCalInterval(1u, &calIntervalDays);
    }

    if(successFlag)
    {
        convertLocalDateTimeToTimeSinceEpoch(&sysDate,
                                             &timeVal,
                                             &sysDateInEpocheFormat);

        convertLocalDateTimeToTimeSinceEpoch(&baroCalDate,
                                             &timeVal,
                                             &baroCalDateInEpochFormat);

        if((sysDateInEpocheFormat - baroCalDateInEpochFormat) > calIntervalDays)
        {
            *calDueStatus = true;
        }

        else
        {
            *calDueStatus = false;
        }
    }

    return successFlag;
}

/**
 * @brief  This function sets system mode and perform actions with respect to mode
 * @param eSysMode_t  system mode to set
 * @retval void
 */
void DPV624::setSysMode(eSysMode_t sysMode)
{
    myMode = sysMode;

    switch(myMode)
    {
    case E_SYS_MODE_POWER_UP:
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        break;

    case E_SYS_MODE_RUN:
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
        break;

    case E_SYS_MODE_FW_UPGRADE:
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        break;

    case E_SYS_MODE_POWER_DOWN:
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        break;

    default:
        break;
    }
}
/**
* @brief  This function gets system mode
* @param void
* @retval return system mode
*/
eSysMode_t DPV624::getSysMode(void)
{
    return myMode;
}

/**
 * @brief get device's current status.
 *
 * @param None
 * @return deviceStatus_t Complete PV624 device status
 */
deviceStatus_t DPV624::getDeviceStatus(void)
{
    return errorHandler->getDeviceStatus();
}

/**
* @brief    updateDeviceStatus - updates the device status based on the error code
* @param    errorCode - enumerated erro code  value
* @param    eErrorStatus_t - error status 0: to clear error and 1:to Set Error
* @return   None
*/
void DPV624::updateDeviceStatus(eErrorCode_t errorCode,
                                eErrorStatus_t errStatus)
{
    errorHandler->updateDeviceStatus(errorCode, errStatus);
}

/**
* @brief  This function stop the motor and vent the pressure to atmosphere
* @param void
* @retval void
*/
void DPV624::ventSystem(void)
{
    stopMotor();
    openVent();
}

/**
 * @brief   returns barometer calibration status
 * @param   void
 * @retval  returns true if it si calibrated otherwise returns false
 */
bool DPV624::getBarometerCalStatus(void)
{
    return persistentStorage->getCalibrationStatus();
}