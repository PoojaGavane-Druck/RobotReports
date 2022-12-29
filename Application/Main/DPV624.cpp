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
#include "ospi_nor_mx25l25645.h"
#include "usbd_cdc_if.h"
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
#define MAX_FLASH_TEST_FILENAME 28u

#define SP_ATMOSPHERIC 2u
#define SP_ABSOLUTE 1u
#define SP_GAUGE 0u

#define DELAY_1_SECONDS         1000u

#define SIZE_REG_ADDR 1u
#define SIZE_REG_DATA 2u

#define TMP1075_ADDR 0x90u
#define REG_ADDR_TEMP 0x00u
#define REG_ADDR_CFGR 0x01u
#define REG_ADDR_LLIM 0x02u
#define REG_ADDR_HLIM 0x03u
#define REG_ADDR_DIEID 0x0Fu

#define TMP1075_RESOLUTION 0.0625f
#define MAX_TEMP_LIM 127.9375f
#define MIN_TEMP_LIM -128.0f

#define TEMP_SENSOR_DEVICE_ID 0x7500u
/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/
DPV624 *PV624 = NULL;

OS_TMR shutdownTimer;

extern I2C_HandleTypeDef hi2c4;
extern I2C_HandleTypeDef hi2c3;
extern SMBUS_HandleTypeDef hsmbus1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern SPI_HandleTypeDef hspi2;
extern IWDG_HandleTypeDef hiwdg;

extern eUpgradeStatus_t upgradeStatus;         // Used for getting Error code and Status of FW Upgrade

extern const unsigned int cAppDK;
extern const unsigned char cAppVersion[4];

extern const uint32_t mainBoardHardwareRevision;
char flashTestFilePath[] = "\\LogFiles\\FlashTestData.csv";
char flashTestLine[] = "PV624 external flash test";

char blFwVersion[] = "##.##.#.#";
char *blFwVersionPtr;
char blAppVersion[SB_APPLICATION_VER_SIZE + 1u] = "DK0XXX VXX.XX.XX";
char *blAppVerPtr;

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
void shutdownTimerCallback(void *p_tmr, void *p_arg);

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

/**
* @brief    Callback from Micrium timer expiry when started for a confirmed shutdown
* @param    void *p_tmr, void *p_arg
* @return   void
*/
void shutdownTimerCallback(void *p_tmr, void *p_arg)
{
    OS_ERR os_error = OS_ERR_NONE;
    RTOSTmrStop(&shutdownTimer, OS_OPT_TMR_NONE, (void *)0, &os_error);

    eShutdown_t shutdownType = (eShutdown_t)(int)(int *)p_arg;

    switch(shutdownType)
    {
    case E_SHUTDOWN_POWER_OFF:
        break;

    case E_SHUTDOWN_RESTART:
    case E_SHUTDOWN_FW_UPGRADE:
//        HAL_IWDG_Refresh(&hiwdg);
//        HAL_Delay(1000u);
        NVIC_SystemReset();
        break;

    default:
        break;
    }
}

static const uint32_t stuckTolerance = 10u;
DPV624::DPV624(void):
    extStorage(NULL),
    persistentStorage(NULL),
    powerManager(NULL),
    logger(NULL),
    errorHandler(NULL),
    keyHandler(NULL),
    instrument(NULL),
    stepperMotor(NULL),
    commsOwi(NULL),
    commsUSB(NULL),
    commsBluetooth(NULL),
    valve1(NULL),
    valve2(NULL),
    valve3(NULL),
    userInterface(NULL)
{

    isEngModeEnable = false;
    isPrintEnable = false;

    blFitted = false;
    blState = BL_STATE_NONE;


    initTempSensor();

    waitOnSecondaryStartup();
    resetQspiFlash();

    pmUpgradePercent = 0u;
    pmUpgradeStatus = false;
    instrumentMode.value = 0u;
    myPinMode = E_PIN_MODE_NONE;
    blState = BL_STATE_NONE;
    controllerDistance = 0.0f;
    myOvershootDisabled = E_OVERSHOOT_DISABLED;

    myBlTaskState = E_BL_TASK_SUSPENDED;

    memset_s(&keepAliveCount[0], sizeof(keepAliveCount), 0, 4u * eNumberOfTasks);
    memset_s(&keepAlivePreviousCount[0], sizeof(keepAlivePreviousCount), 0, 4u * eNumberOfTasks);
    memset_s(&keepAliveIsStuckCount[0], sizeof(keepAliveIsStuckCount), 0, 4u * eNumberOfTasks);

    myMode = E_SYS_MODE_OFF;

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
    bool fwUpgradeStatus = true;

    extStorage = new DExtStorage(&os_error);
    handleOSError(&os_error);

    powerManager = new DPowerManager(&hsmbus1, &os_error);
    handleOSError(&os_error);

    logger = new DLogger(&os_error);
    handleOSError(&os_error);

    errorHandler = new DErrorHandler(&os_error);
    handleOSError(&os_error);

    stepperMotor = new DStepperMotor();

    commsUSB = new DCommsUSB("commsUSB", &os_error);
    handleOSError(&os_error);

    // upgrade FW
    if(E_PARAM_FW_UPGRADE_PENDING == persistentStorage->getFWUpgradePending())
    {
        persistentStorage->setFWUpgradePending(E_PARAM_FW_UPGRADE_NOT_PENDING);
        // allow dependent application objects to initialise
        sleep(DELAY_1_SECONDS);
        setSysMode(E_SYS_MODE_FW_UPGRADE);

        if(false == extStorage->validateAndUpgradeFw())
        {
            fwUpgradeStatus = false;
        }
    }

    if(false == fwUpgradeStatus)
    {
    }

    keyHandler = new DKeyHandler(&os_error);
    handleOSError(&os_error);

    instrument = new DInstrument(&os_error);
    handleOSError(&os_error);

    // Moved before FW Upgrade
//    stepperMotor = new DStepperMotor();

    commsOwi = new DCommsOwi("commsOwi", &os_error);
    handleOSError(&os_error);

    // Moved before FW Upgrade
//    commsUSB = new DCommsUSB("commsUSB", &os_error);
//    handleOSError(&os_error);

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

    setSysMode(resetToPowerUp);

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
    if(NULL != persistentStorage)
    {
        delete persistentStorage;
    }

    if(NULL != powerManager)
    {
        delete powerManager;
    }

    if(NULL != logger)
    {
        delete logger;
    }

    if(NULL != errorHandler)
    {
        delete errorHandler;
    }

    if(NULL != keyHandler)
    {
        delete keyHandler;
    }

    if(NULL != instrument)
    {
        delete instrument;
    }

    if(NULL != stepperMotor)
    {
        delete stepperMotor;
    }

    if(NULL != commsOwi)
    {
        delete commsOwi;
    }

    if(NULL != commsBluetooth)
    {
        delete commsBluetooth;
    }

    if(NULL != valve1)
    {
        delete valve1;
    }

    if(NULL != valve2)
    {
        delete valve2;
    }

    if(NULL != valve3)
    {
        delete valve3;
    }

    if(NULL != userInterface)
    {
        delete userInterface;
    }

    if(NULL != extStorage)
    {
        delete extStorage;
    }

    if(NULL != commsUSB)
    {
        delete commsUSB;
    }

    CDC_free_FS();
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
eSysMode_t DPV624::getResetCause(void)
{
    eSysMode_t sysMode = E_SYS_MODE_OFF;       // Init at none

    uint32_t rccReset = 0u;
    uint32_t wdgReset = 0u;

    rccReset = RCC->CSR;                    // Read the micro controller RCC register reset flags
    wdgReset = RCC->CSR;

    rccReset = rccReset & RESET_REM_MASK;   // Mask bottom part of the register value

    wdgReset = wdgReset & RESET_IWDG;

    if((rccReset & RESET_IWDG) == RESET_IWDG)
    {
        /* there was a watchdog timer reset, clear all flags and keep the system in shutdown mode */
        RCC->CSR = RCC->CSR | CSR_RVMF;

    }

    if((rccReset & RESET_SW) == RESET_SW)
    {
        // Reset was only caused by software
        RCC->CSR = RCC->CSR | CSR_RVMF; // Clear the flags
        sysMode = E_SYS_MODE_RUN;
    }

    return sysMode;
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
        if((eSysMode_t)E_SYS_MODE_RUN == myMode)
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
        if((eSysMode_t)E_SYS_MODE_RUN == myMode)
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
    if((eSysMode_t)E_SYS_MODE_OFF == myMode)
    {
        // PV624 is off, we have to turn it on
        startup();
    }

    else if(((eSysMode_t)E_SYS_MODE_POWER_UP == myMode) || ((eSysMode_t)E_SYS_MODE_RUN == myMode))
    {
        // Deinitialize USB
        MX_USB_DEVICE_DeInit();
        // PV624 is ON, we have to turn it off
        shutdown();
    }

    else
    {

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
    if((eSysMode_t)E_SYS_MODE_RUN == myMode)
    {
        setSysMode(E_SYS_MODE_POWER_DOWN);
        instrument->shutdown();
    }
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
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_Delay((uint16_t)(10));
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay((uint16_t)(100));
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
}

/**
 * @brief   Perform EEPROM test
 * @param   void
 * @retval  void
 */
void DPV624::performEEPROMTest(void)
{
    persistentStorage->selfTest();
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
bool DPV624::getVersion(uint32_t item, char itemverStr[10])
{
    bool status = false;
    itemverStr[0] = '\0';
    uSensorIdentity_t identity;

    if(item < E_RV_CMD_ITEM_MAX)
    {
        switch(item)
        {
        case E_RV_CMD_ITEM_APPLICATION:
        {
            snprintf_s(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)cAppVersion[1], (uint8_t)cAppVersion[2], (uint8_t)cAppVersion[3]);
            status = true;
            break;
        }

        case E_RV_CMD_ITEM_BOOTLOADER:
        {
            snprintf_s(itemverStr, 10u, "%02d.%02d.%02d", (uint8_t)cblVersion[1], (uint8_t)cblVersion[2], (uint8_t)cblVersion[3]);
            status = true;
            break;
        }


        case E_RV_CMD_ITEM_PM_APPLICATION:
        {
            status = instrument->getExternalSensorAppIdentity(&identity);

            if(status)
            {
                snprintf_s(itemverStr, 10u, "%02d.%02d.%02d", identity.major, identity.minor, identity.build);
            }

            status = true;
            break;
        }

        case E_RV_CMD_ITEM_PM_BOOTLOADER:
        {
            status = instrument->getExternalSensorBootLoaderIdentity(&identity);

            if(status)
            {
                snprintf_s(itemverStr, 10u, "%02d.%02d.%02d",
                           identity.major, identity.minor, identity.build);
            }

            status = true;
            break;
        }

        case E_RV_CMD_ITEM_SECOND_MICRO_APPLICATION:
        {
            sVersion_t secondaryAppVersion;
            secondaryAppVersion.all = 0u;
            stepperMotor->getAppVersion(&secondaryAppVersion);
            snprintf_s(itemverStr, 10u, "%02d.%02d.%02d",
                       (uint8_t)secondaryAppVersion.major,
                       (uint8_t)secondaryAppVersion.minor,
                       (uint8_t)secondaryAppVersion.build);
            status = true;
            break;
        }

        case E_RV_CMD_ITEM_SECOND_MICRO_BOOTLOADER:
        {
            sVersion_t secondaryBootVersion;
            secondaryBootVersion.all = 0u;
            stepperMotor->getBootVersion(&secondaryBootVersion);
            snprintf_s(itemverStr, 10u, "%02d.%02d.%02d",
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



    return status;
}

/**
 * @brief   Get DK of specified item as string
 * @param   item - value: 0 = Bootloader
 *                        1 = Application
 * @param   char dkStr[7] - pointer variable for return value
 * @retval  true = success, false = failed
 */
bool DPV624::getDK(uint32_t item, char dkStr[7])
{
    bool status = false;
    uSensorIdentity_t identity;

    if(item < (uint32_t)E_RV_CMD_ITEM_MAX)
    {

        switch(item)
        {
        case E_RV_CMD_ITEM_APPLICATION:
        {
            snprintf_s(dkStr, 7u, "%04d", cAppDK);
            status = true;
            break;
        }

        case E_RV_CMD_ITEM_BOOTLOADER:
        {
            snprintf_s(dkStr, 7u, "%04d", cblDK);
            status = true;
            break;
        }

        case E_RV_CMD_ITEM_PM_APPLICATION:
        {
            status = instrument->getExternalSensorAppIdentity(&identity);

            if(status)
            {
                snprintf_s(dkStr, 7u, "%04d", identity.dk);
            }

            status = true;
            break;
        }

        case E_RV_CMD_ITEM_PM_BOOTLOADER:
        {
            status = instrument->getExternalSensorBootLoaderIdentity(&identity);

            if(status)
            {
                snprintf_s(dkStr, 7u, "%04d", identity.dk);
            }

            status = true;
            break;
        }


        case E_RV_CMD_ITEM_SECOND_MICRO_APPLICATION:
        {
            uint32_t secondaryAppDk = 0u;

            stepperMotor->getAppDk(&secondaryAppDk);
            snprintf_s(dkStr, 7u, "%04d", secondaryAppDk);
            status = true;
            break;
        }

        case E_RV_CMD_ITEM_SECOND_MICRO_BOOTLOADER:
        {
            uint32_t secondaryBootDk = 0u;

            stepperMotor->getBootDk(&secondaryBootDk);
            snprintf_s(dkStr, 7u, "%04d", secondaryBootDk);
            status = true;
            break;
        }

        default:
            break;


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
    memset_s(nameStr, (size_t)13, 0, (size_t)13);
    strncpy_s(nameStr, (size_t)13, cAppInstrument, strlen(cAppInstrument));
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
        successFlag = instrument->getNegFullscale(fs);
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
        successFlag = instrument->getPM620Type(sensorType);
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
        successFlag = instrument->getControllerMode(controllerMode);
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
bool DPV624::getFilterCoeff(float32_t *filterCoeff)
{

    bool successFlag = false;

    if(NULL != filterCoeff)
    {
        successFlag = instrument->getFilterCoeff(filterCoeff);
    }

    return successFlag;
}

/**
 * @brief   Set controller mode
 * @param   controller mode - pointer to variable for return value
 * @retval  true = success, false = failed
*/
bool DPV624::setFilterCoeff(float32_t filterCoeff)
{
    return instrument->setFilterCoeff(filterCoeff);
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
        successFlag = instrument->getVentRate(rate);
    }

    return successFlag;
}

/**
 * @brief   Reset the filter coefficients
 * @param   controller mode - pointer to variable for return value
 * @retval  true = success, false = failed
*/
bool DPV624::resetDisplayFilter(void)
{
    bool successFlag = false;

    successFlag = instrument->resetDisplayFilter();

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
        if((date->year >= MIN_ALLOWED_YEAR) && (date->year <= MAX_ALLOWED_YEAR))
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

            if((numOfDays <= 0) && (manufactureDate.year >= MIN_ALLOWED_YEAR) &&
                    (manufactureDate.year <= MAX_ALLOWED_YEAR)) //Manufacturing date should not be greater than current date
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

            else
            {
                successFlag = false;
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

    if(false == queryPowerdownAllowed())
    {
        upgradeStatus = E_UPGRADE_ERROR_DEVICE_BUSY;
        ok = false;
    }

    if(ok)
    {
        // Allow firmware upgrade when only DC Power present
        if(true == powerManager->checkBatteryComm())
        {
            // Do Firmware Upgrade only if Battery Capacity is >25%

            if((float32_t)(BATTERY_CAP_25_PC) < percentCap)
            {
                ok = true;
            }

            else
            {
                ok = false;
                upgradeStatus = E_UPGRADE_ERROR_BATTERY_TOO_LOW;
            }
        }

        else
        {
            ok = false;
            upgradeStatus = E_UPGRADE_ERROR_BATTERY_NOT_PRESENT;
        }
    }

    if(ok)
    {
        // Check persistent storage writes
        // set and clear flag
        ok &= PV624->persistentStorage->setFWUpgradePending(E_PARAM_FW_UPGRADE_PENDING);
        ok &= (E_PARAM_FW_UPGRADE_PENDING == PV624->persistentStorage->getFWUpgradePending());
        ok &= PV624->persistentStorage->setFWUpgradePending(E_PARAM_FW_UPGRADE_NOT_PENDING);
        ok &= (E_PARAM_FW_UPGRADE_NOT_PENDING == PV624->persistentStorage->getFWUpgradePending());


        if(!ok)
        {
            upgradeStatus = E_UPGRADE_ERROR_PERSISTENT_STORAGE_WRITE_FAIL;
        }
    }

    if(ok)
    {
        if((eSysMode_t)E_SYS_MODE_RUN == getSysMode())
        {
            ok = true;
        }

        else
        {
            ok = false;
        }
    }

    if(ok)
    {
        // Do validation before restart to validate correct version
        if(true == extStorage->validateMainFwFile())
        {
            if(true == extStorage->validateSecondaryFwFile())
            {
                setSysMode(E_SYS_MODE_FW_UPGRADE);
                // set flag to upgrade after reset
                persistentStorage->setFWUpgradePending(E_PARAM_FW_UPGRADE_PENDING);
                // Wait for instrument to shutdown
                performShutdown(E_SHUTDOWN_FW_UPGRADE);
            }

            else
            {
                ok = false;
            }
        }

        else
        {
            ok = false;
        }
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
    uSensorIdentity_t identity;

    instrument->getExternalSensorAppIdentity(&identity);

    if(472u != identity.dk)
    {
        instrument->getPositiveFS((float *)&fsPressure);
        zeroPc = fabs(value) * 100.0f / fsPressure;

        if(1.0f > zeroPc)
        {
            zeroVal = value;
            status = instrument->setSensorZeroValue(sensor, zeroVal);
        }
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

        handleError(E_ERROR_BAROMETER_SENSOR_CAL_STATUS,
                    eClearError,
                    0u,
                    6427u);

        handleError(E_ERROR_EEPROM,
                    eClearError,
                    0u,
                    6424u);
        successFlag = instrument->reloadCalibration();
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
void DPV624::getPmUpgradePercentage(uint32_t *percentage, uint32_t *pUpgradeStatus)
{
    if(NULL != percentage)
    {
        *percentage = pmUpgradePercent;
        *pUpgradeStatus = pmUpgradeStatus;
    }
}

/**
 * @brief   sets the completion status of PM620 firmware upgrade interms of percentage
 * @param   uint32_t how much percentage completed
            uint32_t upgradeStatus - Upgrade status at that percentage 1 - pass, 0m - fail
 * @retval  void
 */
void DPV624::setPmUpgradePercentage(uint32_t percentage, uint32_t pUpgradeStatus)
{
    pmUpgradePercent = percentage;
    pmUpgradeStatus = pUpgradeStatus;
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
        case E_RV_CMD_ITEM_BOARD:
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

bool DPV624::manageBlueToothConnection(eBL652State_t newState)
{
    bool statusFlag = false;

    blFwVersionPtr = blFwVersion;
    blAppVerPtr =  blAppVersion;

    switch(newState)
    {
    case BL_STATE_DISCOVER:

        setBluetoothTaskState(E_BL_TASK_SUSPENDED);

        if(blState == (eBL652State_t)BL_STATE_NONE)
        {
            statusFlag = checkBlModulePresence();

            if(statusFlag == true)
            {
                blState = BL_STATE_DISCOVER;
                readBlFirmwareVersion();
            }
        }

        break;

    case BL_STATE_ENABLE:
    {
        setBluetoothTaskState(E_BL_TASK_SUSPENDED);

        if(blState == (eBL652State_t)BL_STATE_DISCOVER)
        {
            statusFlag = commsBluetooth->startApplication();

            if(statusFlag)
            {
                statusFlag = commsBluetooth->getAppVersion((uint8_t *)blAppVerPtr, sizeof(blAppVersion));
            }

            if(statusFlag == true)
            {
                blState = BL_STATE_ENABLE;
            }
        }

        else
        {
            // No action required as the BLE application has already been started
            //blState = BL_STATE_ENABLE;
        }
    }
    break;

    case BL_STATE_START_ADVERTISING:
    {
        setBluetoothTaskState(E_BL_TASK_SUSPENDED);

        statusFlag = commsBluetooth->startAdverts((uint8_t *)blAppVerPtr, sizeof(blAppVersion));

        if(statusFlag == true)
        {
            blState = BL_STATE_RUN_ADV_IN_PROGRESS;

            userInterface->bluetoothLedControl(eBlueToothPairing,
                                               E_LED_OPERATION_TOGGLE,
                                               65535u,
                                               E_LED_STATE_SWITCH_ON,
                                               UI_DEFAULT_BLINKING_RATE);

            setBluetoothTaskState(E_BL_TASK_RUNNING);
        }
    }
    break;

    case BL_STATE_DISABLE:
    {
        if(blState == (eBL652State_t)BL_STATE_START_ADVERTISING)
        {
            statusFlag = commsBluetooth->stopAdverts();

            if(statusFlag == true)
            {
                blState = BL_STATE_DISABLE;

                // stop flashing bluetooth status icon to indicate bluetooth advertising stopped
                userInterface->bluetoothLedControl(eBlueToothError,
                                                   E_LED_OPERATION_SWITCH_OFF,
                                                   0u,
                                                   E_LED_STATE_SWITCH_OFF,
                                                   UI_DEFAULT_BLINKING_RATE);
            }
        }

        else if(blState == (eBL652State_t)BL_STATE_PAIRED)
        {
            blState = BL_STATE_DISABLE;

            statusFlag = commsBluetooth->disconnect();

            if(statusFlag == true)
            {
                // stop flashing bluetooth status icon to indicate bluetooth advertising stopped
                userInterface->bluetoothLedControl(eBlueToothError,
                                                   E_LED_OPERATION_SWITCH_OFF,
                                                   0u,
                                                   E_LED_STATE_SWITCH_OFF,
                                                   UI_DEFAULT_BLINKING_RATE);
            }
        }

        else
        {
            blState = BL_STATE_DISABLE;

            // stop flashing bluetooth status icon to indicate bluetooth advertising stopped
            userInterface->bluetoothLedControl(eBlueToothError,
                                               E_LED_OPERATION_SWITCH_OFF,
                                               0u,
                                               E_LED_STATE_SWITCH_OFF,
                                               UI_DEFAULT_BLINKING_RATE);

            statusFlag = true;
        }
    }
    break;

    case BL_STATE_DISCONNECT:
    {
        if(blState != (eBL652State_t)BL_STATE_DISABLE)
        {
            // BLE Disconnected by peer
            setBlState(BL_STATE_ADV_TIMEOUT);

            userInterface->bluetoothLedControl(eBlueToothPairing,
                                               E_LED_OPERATION_SWITCH_OFF,
                                               65535u,
                                               E_LED_STATE_SWITCH_OFF,
                                               UI_DEFAULT_BLINKING_RATE);
            statusFlag = true;
        }
    }
    break;

    case BL_STATE_NO_COMMUNICATION:
    {

        statusFlag = commsBluetooth->disconnect();


        setBlState(BL_STATE_ADV_TIMEOUT);

        userInterface->bluetoothLedControl(eBlueToothPairing,
                                           E_LED_OPERATION_SWITCH_OFF,
                                           65535u,
                                           E_LED_STATE_SWITCH_OFF,
                                           UI_DEFAULT_BLINKING_RATE);
    }
    break;

    case BL_STATE_RUN_ENCRYPTION_ESTABLISHED:
    {
        setBluetoothTaskState(E_BL_TASK_RUNNING);
        setBlState(BL_STATE_RUN_ENCRYPTION_ESTABLISHED);
    }
    break;

    default:
        break;
    }

    return true;




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
 * @brief   Get the connected sensor brand min
 * @param   *brandMin - pointer to variable to return min value string
 * @retval  true = success, false = failed
 */
bool DPV624::getSensorBrandMin(char *brandMin, uint32_t bufLen)
{
    bool successFlag = false;

    if((NULL != brandMin) && (bufLen > 0u))
    {
        instrument->getSensorBrandMin(brandMin, bufLen);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Get the connected sensor brand  max information
 * @param   *brandMin - pointer to variable to return max value string
 * @retval  true = success, false = failed
 */
bool DPV624::getSensorBrandMax(char *brandMax, uint32_t bufLen)
{
    bool successFlag = false;

    if((NULL != brandMax) && (bufLen > 0u))
    {
        instrument->getSensorBrandMax(brandMax, bufLen);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Get the connected sensor brand sesnor type information
 * @param   *brandMin - pointer to variable to return sensor type
 * @retval  true = success, false = failed
 */
bool DPV624::getSensorBrandType(char *brandType,
                                uint32_t bufLen)
{
    bool successFlag = false;

    if((NULL != brandType) && (bufLen > 0u))
    {
        instrument->getSensorBrandType(brandType, bufLen);
        successFlag = true;
    }

    return successFlag;
}

/**
 * @brief   Get the connected sensor brand uints information
 * @param   *brandMin - pointer to variable to return units supported by sensor
 * @retval  true = success, false = failed
 */
bool DPV624::getSensorBrandUnits(char *brandUnits, uint32_t bufLen)
{
    bool successFlag = false;

    if((NULL != brandUnits) && (bufLen > 0u))
    {
        instrument->getSensorBrandUnits(brandUnits, bufLen);
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
                    (date->year >= MIN_ALLOWED_YEAR) &&
                    (date->year <= MAX_ALLOWED_YEAR))
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
    HAL_GPIO_WritePin(SECOND_MICRO_RESET_PB13_GPIO_Port,
                      SECOND_MICRO_RESET_PB13_Pin,
                      GPIO_PIN_RESET);
}


/**
* @brief Reset stepper micro controller
* @param void
* @retval void
*/
void DPV624::releaseStepperMicroReset(void)
{
    /* Reset the stepper controller micro */
    HAL_GPIO_WritePin(SECOND_MICRO_RESET_PB13_GPIO_Port,
                      SECOND_MICRO_RESET_PB13_Pin,
                      GPIO_PIN_SET);
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
    valve3->setValveTime(150u);
    valve3->reConfigValve(E_VALVE_MODE_PWMA);
    valve3->triggerValve(VALVE_STATE_ON);
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
#if 0
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

#endif
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
    float32_t distanceTravelled = 0.0f;
    distanceTravelled = getDistanceTravelled();

    if(distanceTravelled > MAX_ALLOWED_DISTANCE_TRAVELLED)
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
        //HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        break;

    case E_SYS_MODE_RUN:
        //HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
        break;

    case E_SYS_MODE_FW_UPGRADE:
        //HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        break;

    case E_SYS_MODE_POWER_DOWN:
        // Cannot disable interrupts in power down mode, key handler needs to run
        //HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        break;

    case E_SYS_MODE_DIAGNOSTIC_TEST:
        break;

    case E_SYS_MODE_OFF:
        shutdownPeripherals();
        break;

    default:
        break;
    }
}

/**
* @brief  Shuts down all peripherals including valves, secondary micro, ble of the PV624
* @param void
* @retval void
*/
void DPV624::shutdownPeripherals(void)
{
    BL652_initialise(eBL652_MODE_DISABLE);
    // Close vent valve
    valve3->triggerValve(VALVE_STATE_OFF);
    // Close outlet valve - isolate pump from generating vaccum
    valve1->triggerValve(VALVE_STATE_OFF);
    // Close inlet valve - isolate pump from generating pressure
    valve2->triggerValve(VALVE_STATE_OFF);

    sleep(20u);     // Give some time for valves to turn off
    // Disable all valves
    valve1->disableValve();
    valve2->disableValve();
    valve3->disableValve();

    // Turn off 24V suply
    powerManager->turnOffSupply(eVoltageLevelTwentyFourVolts);
    // Turn off 5V PM620 supply
    powerManager->turnOffSupply(eVoltageLevelFiveVolts);
    // Hold the stepper micro controller in reset
    holdStepperMotorReset();
    // Hold BLE in reset - TODO
    // Turn off LEDs
    userInterface->statusLedControl(eStatusProcessing,
                                    E_LED_OPERATION_SWITCH_OFF,
                                    65535u,
                                    E_LED_STATE_SWITCH_OFF,
                                    0u);
    userInterface->bluetoothLedControl(eBlueToothPurple,
                                       E_LED_OPERATION_SWITCH_OFF,
                                       65535u,
                                       E_LED_STATE_SWITCH_OFF,
                                       0u);


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

/**
 * @brief   reads optical board Status
 * @param   void
 * @retval  returns true if board is present else false
 */
bool DPV624::readOpticalBoardStatus(void)
{
    bool successFlag = false;
    // Read optical board GPIO to check if board is available
    optBoardStatus = !(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_14));
    successFlag = (optBoardStatus == 1u) ? true : false;
    return successFlag;
}

/**
 * @brief   run device dignostics and return test results
 * @param   void
 * @retval  returns run diagnostics result
 */
uint32_t DPV624::runDiagnostics(void)
{
    eSysMode_t currMode = myMode;

    myMode = E_SYS_MODE_DIAGNOSTIC_TEST;

    bool successFlag = false;
    uint32_t val = 0u;
    int32_t retVal = -1;

    char dkStr[7u];
    int8_t secondUcDkNum[5] = "0509";
    runDiagnosticsStatus_t dignosticsStatus;

    dignosticsStatus.bytes = 0u;

    userInterface->saveStatusLedState();
    userInterface->saveBluetoothLedState();

    userInterface->ledsOnAll();

    successFlag = persistentStorage->selfTest();

    if(successFlag)
    {
        dignosticsStatus.bit.eeprom = 1u;
    }

    successFlag = testExternalFlash();

    if(successFlag)
    {
        dignosticsStatus.bit.extFlash = 1u;
    }

    successFlag = powerManager->getValue(EVAL_INDEX_BATTERY_24VOLT_STATUS, &val);

    if(successFlag)
    {
        dignosticsStatus.bit.twentyFourVolts = 1u;
    }

    successFlag = powerManager->getValue(EVAL_INDEX_BATTERY_6VOLT_STATUS, &val);

    if(successFlag)
    {
        dignosticsStatus.bit.fivePointFiveVolts = 1u;
    }

    successFlag = powerManager->getValue(EVAL_INDEX_BATTERY_5VOLT_STATUS, &val);

    if(successFlag)
    {
        dignosticsStatus.bit.fiveVolts = 1u;
    }

    successFlag = getDK(E_RV_CMD_ITEM_SECOND_MICRO_APPLICATION, dkStr);

    if(successFlag)
    {
        retVal = memcmp(dkStr, secondUcDkNum, (size_t)5);
    }

    successFlag = (retVal == 0) ? true : false;

    if(successFlag)
    {
        dignosticsStatus.bit.secondMicroController = 1u;
    }

    successFlag = readOpticalBoardStatus();

    if(successFlag)
    {
        dignosticsStatus.bit.opticalBoard = 1u;
    }

    successFlag = moveMotorTillForwardEnd();

    if(successFlag)
    {
        dignosticsStatus.bit.optSensorExtended = 1u;
    }

    successFlag = moveMotorTillReverseEndThenHome();

    if(successFlag)
    {
        dignosticsStatus.bit.optSensorRetracted = 1u;
    }

    successFlag = powerManager->checkBatteryComm();

    if(successFlag)
    {
        dignosticsStatus.bit.smBusBatteryComm = 1u;
    }

    successFlag = powerManager->checkBatteryChargerComm();

    if(successFlag)
    {
        dignosticsStatus.bit.smBusBatChargerComm = 1u;
    }

    successFlag = checkBluetoothCommInterface();

    if(successFlag)
    {
        dignosticsStatus.bit.bl652Commm = 1u;
    }


    myMode = currMode;

    userInterface->ledsOffAll();

    userInterface->restoreStatusLedState();
    userInterface->restoreBluetoothLedState();

    return dignosticsStatus.bytes;
}

/**
* @brief    Create Error log file if not present. ALso add column headers
* @note     Filename is always:
*                (i) saved in LogFiles folder in the root directory of the file system
*                (ii) given the '.csv' extension
* @param    filename is a char array representing the filename
*           if value is NULL (default parameter) then the name is auto-generated
* @return   log error status
*/
bool DPV624::testExternalFlash(void)
{
    bool ok = true;
    bool successFlag = false;
    uint32_t isEqual = 0u;

    //continue with whatever file name is to be used
    //create file
    ok = PV624->extStorage->openFile(flashTestFilePath, false);

    if(!ok)
    {
        PV624->extStorage->close();
        ok = PV624->extStorage->openFile(flashTestFilePath, true);
    }

    else
    {
        ok = PV624->extStorage->erase(flashTestFilePath);

        if(ok)
        {
            ok = PV624->extStorage->openFile(flashTestFilePath, true);
        }
    }

    if(ok)
    {
        ok = PV624->extStorage->writeLine(flashTestLine, sizeof(flashTestLine));

        if(ok)
        {
            // Close the file
            ok &= PV624->extStorage->close();

            if(ok)
            {
                // Open the file
                ok = PV624->extStorage->openFile(flashTestFilePath, false);

                if(ok)
                {
                    // Read data from the file
                    char flashReadData[MAX_FLASH_TEST_FILENAME];

                    ok = PV624->extStorage->readLine(flashReadData, sizeof(flashReadData), sizeof(flashTestLine));
                    isEqual = compareArrays((const uint8_t *)flashReadData, (const uint8_t *)flashTestLine, sizeof(flashTestLine));
                }
            }
        }
    }

    ok &= PV624->extStorage->close();

    if(ok)
    {
        successFlag = isEqual ? true : false;
    }

    else
    {
        successFlag = false;
    }

    return successFlag;
}
/**
 * @brief   checks bluetooth communication interface  working or not
 * @param   void
 * @retval  returns true if suucceeded and false if it fails
 */
bool DPV624::checkBluetoothCommInterface(void)
{
    bool successFlag = true;

    setBluetoothTaskState(E_BL_TASK_SUSPENDED);

    successFlag = BL652_initialise(eBL652_MODE_COMM_INTERFACE_CHECK);


    return successFlag;
}

/**
 * @brief   update set point count into eeprom
 * @param   uint32_t new set point count
 * @retval  true if saved  sucessfully false if save fails
 */
bool DPV624::updateSetPointCount(uint32_t setPointCount)
{
    bool successFlag = false;
    successFlag = persistentStorage->updateSetPointCount(setPointCount);
    return successFlag;
}

/**
 * @brief   returns battery Mnaufacture Name
 * @param   pointer to buffer to return battery Manufacture Name
 * @param   buffer size
 * @retval  void
 */
void DPV624::getBatteryManufName(int8_t *batteryManuf,
                                 uint32_t bufSize)
{


    if((NULL != batteryManuf) && (bufSize > 0U))
    {
        powerManager->getValue(E_VAL_BATTERY_MANUF_NAME,
                               batteryManuf,
                               bufSize);
    }

}

/**
 * @brief   Perform shutdown
 * @param   shutdownType specifies shutdown, restart or upgrade
 * @retval  flag: true if ok, else false
 */
bool DPV624::performShutdown(eShutdown_t shutdownType, bool autoPowerDownForced)
{
    bool flag = false;

    // Remove all pressure before restart
    ventSystem();

    //do not allow shutdown if running a procedure/process (eg, leak test, active automation) or upgrading
    if((queryPowerdownAllowed()) || (autoPowerDownForced))
    {
        //ok to proceed
        flag = true;
//        fsOwnership = SHUTDOWN;

        // Configure shutdown timer to allow sufficient time for other tasks to shutdown gracefully first
        OS_ERR os_error = OS_ERR_NONE;
        RTOSTmrCreate(&shutdownTimer, "Shutdown timer", (OS_TICK)(OS_CFG_TMR_TASK_RATE_HZ * 4u), (OS_TICK)0u, OS_OPT_TMR_ONE_SHOT, (OS_TMR_CALLBACK_PTR)shutdownTimerCallback, (void *)(int)shutdownType, &os_error);
        RTOSTmrStart(&shutdownTimer, &os_error);
    }

    return flag;

}

/**
 * @brief   Power off/restart
 * @param   none
 * @retval  flag: true if ok to power off, else false
 */
bool DPV624::queryPowerdownAllowed(void)
{
    bool successFlag = true;
    eControllerMode_t controllerMode = E_CONTROLLER_MODE_NONE;

    // Query for running task
    getControllerMode(&controllerMode);

    if(((eControllerMode_t)E_CONTROLLER_MODE_CONTROL == controllerMode) || ((eControllerMode_t)E_CONTROLLER_MODE_RATE == controllerMode)
            || ((eControllerMode_t)E_CONTROLLER_MODE_FM_UPGRADE == controllerMode))
    {
        successFlag = false;
    }

    return(successFlag);
}


/**
 * @brief   Configure External Flash Memory (destructive)
 * @param   void
 * @retval  bool
 */
bool DPV624::configureExternalFlashMemory(void)
{
    return extStorage->configure();
}

/**
 * @brief   Get status of External Flash Memory
 * @param   uint32_t *bytesUsed, uint32_t *bytesTotal
 * @retval  bool
 */
bool DPV624::getExternalFlashStatus(uint32_t *bytesUsed, uint32_t *bytesTotal)
{
    return extStorage->getStatus(bytesUsed, bytesTotal);
}

/**
 * @brief   Checks if AC present and returns the value
 * @param   void
 * @retval  returns true if ac present otherwise false
 */
bool DPV624::getIsAcPresent(void)
{
    return powerManager->getIsAcPresent();
}

/**
 * @brief   set distance travelled value with specific value
 * @param   float32_t new new distance travelled value
 * @retval  true if saved  sucessfully false if save fails
 */
bool DPV624::setDistanceTravelled(float32_t distanceTravelled)
{
    bool successFlag = false;
    successFlag = persistentStorage->updateDistanceTravelled(distanceTravelled);

    if(successFlag)
    {
        controllerDistance = distanceTravelled;
    }

    return successFlag;
}

/**
 * @brief   Get calibration type
 * @param   calType - function specific calibration type (0 = user calibration)
 * @param   range - sensor range
 * @retval  true = success, false = failed
 */
bool DPV624::getCalibrationType(int32_t *calType, uint32_t *range)
{
    return instrument->getCalibrationType(calType, range);
}

/**
 * @brief   Sets the value of controller overshoot to enabled or disabled
 * @param   void
 * @retval  returns false if the value is invalid, else true
 */
bool DPV624::setOvershootState(uint32_t overshootState)
{
    bool flag = false;

    if(overshootState < E_OVERSHOOT_MAX)
    {
        myOvershootDisabled = (eOvershootDisabled_t)(overshootState);
        flag = true;
    }

    return flag;
}

/**
 * @brief   Sets the value of controller overshoot to enabled or disabled
 * @param   void
 * @retval  returns false if the value is invalid, else true
 */
void DPV624::getOvershootState(uint32_t *overshootState)
{
    *overshootState = (uint32_t)(myOvershootDisabled);
}

/**
 * @brief         : Gets the indication of whether the bluetooth module is fitted
 * @param[in]     : None
 * @param[out]    : None
 * @param[in,out] : None
 * @retval        : Bluetooth fitted indication
 */
bool DPV624::checkBlModulePresence()
{
    if((blState == (eBL652State_t)BL_STATE_DISABLE)  ||
            (blState == (eBL652State_t)BL_STATE_NONE))
    {
        blFitted = commsBluetooth->checkBlModulePresence();
    }

    return  blFitted;
}

/**
 * @brief         : Gets the bluetooth application software version
 * @param[in]     : None
 * @param[out]    : Version
 * @param[in,out] : None
 * @retval        : None
 */
void DPV624::getBlApplicationVersion(char *version, uint16_t len)
{
    if((eBL652State_t)BL_STATE_NONE == blState)
    {
        manageBlueToothConnection(BL_STATE_DISCOVER);
        manageBlueToothConnection(BL_STATE_ENABLE);
    }

    else if((eBL652State_t)BL_STATE_DISCOVER == blState)
    {
        manageBlueToothConnection(BL_STATE_ENABLE);
    }

    else
    {
        /* do nothing */
    }

    if((version != NULL) && (len > 0u))
    {
        memset_s(version, (rsize_t)len, 0, (rsize_t)len);
        memcpy_s(version, (rsize_t)len, blAppVersion, ((rsize_t)len)); // subtract 2 to  remove \n and \r
    }
}
/**
 * @brief   Query bluetooth device firmware version
 * @param   buffer to return value in
 * @param   None
 * @retval  flag: true if bluetooth FW version read is sucessful, False if the read has failed
 */
void DPV624::getBlFirmwareVersion(char *version, uint16_t len)
{
    if((blState == (eBL652State_t)BL_STATE_DISABLE) ||
            ((blState == (eBL652State_t)BL_STATE_NONE)))
    {
        manageBlueToothConnection(BL_STATE_DISCOVER);
    }

    if((version != NULL) && (len > 0u))
    {
        memset_s(version, (rsize_t)len, 0, (rsize_t)len);
        snprintf_s(version, (uint32_t)len, blFwVersion);
    }
}
/**
 * @brief   Query bluetooth device firmware version
 * @param   buffer to return value in
 * @param   None
 * @retval  flag: true if bluetooth FW version read is sucessful, False if the read has failed
 */
void DPV624::readBlFirmwareVersion(void)
{
    if(blState == (eBL652State_t)BL_STATE_DISCOVER)
    {
        commsBluetooth->getFWVersion(blFwVersionPtr);
    }
}
/**
 * @brief   Disconnects the bluetooth connection
 * @param   None
 * @param   None
 * @retval  None
 */
void DPV624::disconnectBL(void)
{
    bool sucessFlag = false;
    sucessFlag = commsBluetooth->disconnect();

    if(sucessFlag)
    {
        setBlState(BL_STATE_ADV_TIMEOUT);
    }
}

/**
* @brief   Initializes the temperature sensor on V3 boards to set it to max limit. This disables the reset output
        of the sensor which in turn does not reset the stepper driver directly in case the temperature reaches over
        its fixed limit of 80 deg C.
        TODO!!
        Eventually, this function needs to use the TMP1075 library
* @param   void
* @retval  returns false if init fails or temp sensor is unavailable
*/
bool DPV624::initTempSensor(void)
{
    bool sensorInit = false;
    uint32_t deviceId = 0u;
    uint32_t highLimit = 0u;
    int32_t lowLimit = 0;
    uint32_t temp = 0u;
    float32_t setHighLimit = 0.0f;
    float32_t setLowLimit = 0.0f;

    uint8_t dataBuff[10] = { 0u };

    dataBuff[0] = REG_ADDR_DIEID;
    HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)(TMP1075_ADDR), dataBuff, (uint16_t)(SIZE_REG_ADDR), 1000u);
    HAL_I2C_Master_Receive(&hi2c3, (uint16_t)(TMP1075_ADDR), dataBuff, (uint16_t)(SIZE_REG_DATA), 1000u);
    deviceId = (((uint32_t)(dataBuff[0])) << 8u) | ((uint32_t)(dataBuff[1]));

    if(deviceId == TEMP_SENSOR_DEVICE_ID)
    {
        /* There is a temperature sensor connected to this board. First set the temperature sensor high limit to 127.95
        degrees C. Then set the low limit to -128 degrees C. Read both the values back to ensure that the limits are
        properly set */
        highLimit = (uint16_t)((float32_t)(MAX_TEMP_LIM / TMP1075_RESOLUTION) * 16.0f);

        dataBuff[0] = REG_ADDR_HLIM;
        dataBuff[1] = (uint8_t)((highLimit >> 8) & 0xFFu);
        dataBuff[2] = (uint8_t)(highLimit & 0xFFu);
        HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)(TMP1075_ADDR), dataBuff, (uint16_t)(SIZE_REG_ADDR + SIZE_REG_DATA), 1000u);

        lowLimit = (int32_t)((float32_t)(MIN_TEMP_LIM / TMP1075_RESOLUTION) * 16.0f);

        dataBuff[0] = REG_ADDR_LLIM;
        dataBuff[1] = (uint8_t)(((uint32_t)(lowLimit) >> 8) & 0xFFu);
        dataBuff[2] = (uint8_t)((uint32_t)(lowLimit) & 0xFFu);
        HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)(TMP1075_ADDR), dataBuff, (uint16_t)(SIZE_REG_ADDR + SIZE_REG_DATA), 1000u);

        dataBuff[0] = REG_ADDR_HLIM;
        HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)(TMP1075_ADDR), dataBuff, (uint16_t)(SIZE_REG_ADDR), 1000u);
        HAL_I2C_Master_Receive(&hi2c3, (uint16_t)(TMP1075_ADDR), dataBuff, (uint16_t)(SIZE_REG_DATA), 1000u);
        temp = (((uint32_t)(dataBuff[0])) << 8u) | ((uint32_t)(dataBuff[1]));
        setHighLimit = (float32_t)(temp) * (TMP1075_RESOLUTION) / 16.0f;

        dataBuff[0] = REG_ADDR_LLIM;
        HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)(TMP1075_ADDR), dataBuff, (uint16_t)(SIZE_REG_ADDR), 1000u);
        HAL_I2C_Master_Receive(&hi2c3, (uint16_t)(TMP1075_ADDR), dataBuff, (uint16_t)(SIZE_REG_DATA), 1000u);
        temp = (((uint32_t)(dataBuff[0])) << 8u) | ((uint32_t)(dataBuff[1]));
        temp = temp >> 4u;
        temp = ~(temp);
        temp = temp + 1u;
        temp = temp << 4u;
        temp = temp & 0x0000FFFFu;
        setLowLimit = -((float32_t)(temp) * (TMP1075_RESOLUTION) / 16.0f);

        if(floatEqual(MAX_TEMP_LIM, setHighLimit))
        {
            if(floatEqual(MIN_TEMP_LIM, setLowLimit))
            {
                sensorInit = true;
            }

            else
            {
                sensorInit = false;
            }
        }

        else
        {
            sensorInit = false;
        }
    }

    else
    {
        sensorInit = false;
    }

    return sensorInit;
}