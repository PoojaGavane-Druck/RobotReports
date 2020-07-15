/**
* BHGE Confidential
* Copyright 2019.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are protected by trade
* secret or copyright law.  Dissemination of this information or reproduction of this material is strictly forbidden unless prior written
* permission is obtained from Baker Hughes.
*
* @file     gpio.c
* @version  1.0
* @author   julio Andrade
* @date     Jun 22, 2018
* @brief GPIO configuration
*/

/******************************************************************************************** INCLUDES ************************************************************************************************/
#include "wdg.h"
#include "gpio.h"

#include "uart.h"
#include "overcurrent.h"
#include "KeyTask.h"
#include "OvercurrentTask.h"

/*************************************************************************************GLOBALS AND DEFINES ************************************************************************************************/
/* Macro to enable all interrupts. */
#define EnableInterrupts() __enable_irq()

/* Macro to disable all interrupts. */
#define DisableInterrupts() __disable_irq()
static OS_SEM gpioIntSem ;
static gpioButtons_t flag;

/********************************************************************************** TYPEDEFS AND VARIABLES ************************************************************************************************/
typedef struct
{
    GPIO_TypeDef *Port;
    GPIO_InitTypeDef GpioSettings;
    int externalInterrupt;
} sGpioSettingPorts;

static sGpioSettingPorts portSettings[] =
{
#ifdef DEBUG
    /*    */{ ON_OFF_SW_GPIO_PORT,     { ON_OFF_SW_GPIO_PIN,       GPIO_MODE_OUTPUT_PP,         GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 }, /* Enable GPIO clock for LED2(PA5)*/
#endif
    /*    */{ FILTER_GPIO_PORT,         { FILTER_GPIO_PIN,          GPIO_MODE_IT_RISING_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, FILTER_EXTI_IRQn    },
    /*    */{ LEAK_GPIO_PORT,           { LEAK_GPIO_PIN,            GPIO_MODE_IT_RISING_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, LEAK_EXTI_IRQn      },
    /*    */{ TARE_GPIO_PORT,           { TARE_GPIO_PIN,            GPIO_MODE_IT_RISING_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, TARE_EXTI_IRQn      },
    /*    */{ UNITS_GPIO_PORT,          { UNITS_GPIO_PIN,           GPIO_MODE_IT_RISING_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, UNITS_EXTI_IRQn     },
    /*    */{ BACKLIGHT_GPIO_PORT,      { BACKLIGHT_GPIO_PIN,       GPIO_MODE_IT_RISING_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, BACKLIGHT_EXTI_IRQn },
    /*    */{ EXTSENAL_GPIO_PORT,       { EXTSENAL_GPIO_PIN,        GPIO_MODE_IT_FALLING,        GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, EXTSENAL_EXTI_IRQn  },
    /*    */{ ON_OFF_GPIO_PORT,         { ON_OFF_GPIO_PIN,          GPIO_MODE_IT_RISING_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, ON_OFF_EXTI_IRQn    },
    /*    */{ EXTSENCURR_GPIO_PORT,     { EXTSENCURR_GPIO_PIN,      GPIO_MODE_OUTPUT_PP,         GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 },
    /*    */{ EXTSEN_ON_OFF_GPIO_PORT,  { EXTSEN_ON_OFF_GPIO_PIN,   GPIO_MODE_OUTPUT_PP,         GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 },
    /*    */{ BRDV0_GPIO_PORT,          { BRDV0_GPIO_PIN,           GPIO_MODE_INPUT,             GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 },
    /*    */{ BRDV1_GPIO_PORT,          { BRDV1_GPIO_PIN,           GPIO_MODE_INPUT,             GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 },
    /*    */{ BRDV2_GPIO_PORT,          { BRDV2_GPIO_PIN,           GPIO_MODE_INPUT,             GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 },
    /*    */{ BRDV3_GPIO_PORT,          { BRDV3_GPIO_PIN,           GPIO_MODE_INPUT,             GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 }, /* Enable GPIO clock for LED2(PA5)*/
    /*    */{ DSP0_GPIO_PORT,           { DSP0_GPIO_PIN,            GPIO_MODE_INPUT,             GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 }, /* Enable GPIO clock for LED2(PA5)*/
    /*    */{ DSP1_GPIO_PORT,           { DSP1_GPIO_PIN,            GPIO_MODE_INPUT,             GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 }, /* Enable GPIO clock for LED2(PA5)*/
    /*    */{ RS485_REN_GPIO_PORT,      { RS485_REN_GPIO_PIN,       GPIO_MODE_OUTPUT_PP,         GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 }, /* Enable GPIO clock for LED2(PA5)*/
    /*    */{ RS485_TEN_GPIO_PORT,      { RS485_TEN_GPIO_PIN,       GPIO_MODE_OUTPUT_PP,         GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 }, /* Enable GPIO clock for LED2(PA5)*/
    /*    */{ INTPS_ON_OFF_PORT,        { INTPS_ON_OFF_PIN,         GPIO_MODE_OUTPUT_PP,         GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 }, /* Enable GPIO clock for LED2(PA5)*/
    /*    */{ LCD_BACKLIGHT_GPIO_PORT,  { LCD_BACKLIGHT_GPIO_PIN,   GPIO_MODE_OUTPUT_PP,         GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 },
    /*    */{ DISP_RST_GPIO_PORT,       { DISP_RST_GPIO_PIN,        GPIO_MODE_OUTPUT_PP,         GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, NULL }, 0 },
};

/************************************************************************** PROTOTYPES *************************************************************************************************/
static void gpioEnableClock(GPIO_TypeDef *port);
static void enableOnOffIrqOnly(void);

/************************************************************************* FUNCTIONS ***************************************************************************************************/


/**
 * @brief ports based on the portSettings table
 */

void gpioInit(void)
{
    uint8_t portSize = (uint8_t)(sizeof(portSettings) / sizeof(portSettings[0]));
    uint8_t i;

    OS_ERR gpioSemErr;
    flag.bytes = 0u;

    OSSemCreate(&gpioIntSem, "GpioSem", (OS_SEM_CTR)0, &gpioSemErr ); /* Create GPIO interrupt semaphore */

    if (gpioSemErr != OS_ERR_NONE )
    {
        //Enter logical error code here
    }

    for (i = 0u; i < portSize; i++)
    {
        gpioEnableClock(portSettings[i].Port);
        HAL_GPIO_Init(portSettings[i].Port, &(portSettings[i].GpioSettings));

        if (portSettings[i].externalInterrupt != 0)
        {
            /*Enable and set Power EXTI Interrupt to the lowest priority */
            HAL_NVIC_SetPriority((IRQn_Type)(portSettings[i].externalInterrupt), 0x0Fu, 0x00u); //interrupt priority
            HAL_NVIC_EnableIRQ((IRQn_Type)portSettings[i].externalInterrupt);
        }
    }
}


/**
 * @brief fetches the board version from gpio
 * @return board version structure
 */

gpioBV_t gpioGetBoardVer(void)
{
    gpioBV_t boardVer;

    boardVer.bytes   = 0u;
    boardVer.bit.bv0 = gpioGetBRV0();
    boardVer.bit.bv1 = gpioGetBRV1();
    boardVer.bit.bv2 = gpioGetBRV2();
    boardVer.bit.bv3 = gpioGetBRV3();

    return boardVer;
}


/**
 * @brief fetches the display version from gpio
 * @return display version structure
 */

gpioDISPV_t gpioGetDispVer(void)
{
    gpioDISPV_t dispVer;

    dispVer.bytes = 0u;
    dispVer.bit.dsp0 = gpioGetDISP0();
    dispVer.bit.dsp1 = gpioGetDISP1();

    return dispVer;
}


/**
 * @brief fetches the state of button gpio buttons
 * @return returns button gpio structure
 */

gpioButtons_t GPIO_getButtons(void)
{
    gpioButtons_t status;
    status.bytes = (uint32_t)~0u;

    //inverted bytes
    status.bit.TopLeft = gpioGetFilter();
    status.bit.BottomMiddle = gpioGetLeak();
    status.bit.TopMiddle = gpioGetTare();
    status.bit.TopRight = gpioGetUnits();
    status.bit.BottomLeft = gpioGetBacklight();
    status.bytes = ~status.bytes;

    //non inverted
    status.bit.BottomRight = gpioGetOnOff();

    return status;
}


/**
 * @brief Enables all the GPIO interrupts
 */

void gpioEnIrq(void)
{
    flag.bytes = 0u;

    HAL_NVIC_EnableIRQ(ON_OFF_EXTI_IRQn);
    HAL_NVIC_EnableIRQ(LEAK_EXTI_IRQn);
    HAL_NVIC_EnableIRQ(FILTER_EXTI_IRQn);
    HAL_NVIC_EnableIRQ(TARE_EXTI_IRQn);
    HAL_NVIC_EnableIRQ(UNITS_EXTI_IRQn);
    HAL_NVIC_EnableIRQ(BACKLIGHT_EXTI_IRQn);
}


/**
 * @brief Disables all GPIO interrupts
 */

void GPOIO_DisIRQ(void)
{
    HAL_NVIC_DisableIRQ(ON_OFF_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(LEAK_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(FILTER_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(TARE_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(UNITS_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(BACKLIGHT_EXTI_IRQn);
}


/**
 * @brief Disables all GPIO interrupts except the on/off button which is enabled
 */

static void enableOnOffIrqOnly(void)
{
    HAL_NVIC_DisableIRQ(LEAK_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(FILTER_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(TARE_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(UNITS_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(BACKLIGHT_EXTI_IRQn);

    HAL_NVIC_EnableIRQ(ON_OFF_EXTI_IRQn);
}


/**
 * @brief GPIO interrupt request wait/ semaphore pend
 * @param max maximum wait limit
 * @param buttonFflag gpio button type
 * @return os error for sempahore pend operation in mainapp
 * @return wait state
 */

OS_ERR gpio_IRQWait(uint32_t max, gpioButtons_t *buttonFflag)
{
    OS_ERR os_err = OS_ERR_NONE;
    OS_ERR tm_os_err = OS_ERR_NONE;

    //wait for semaphore from key press IRQ
    OSSemPend(&gpioIntSem, max, OS_OPT_PEND_BLOCKING, (CPU_TS *)&max, &os_err);

    //if arrive here from a user key press then received key code wait for debounce time and read keys' state
    if (flag.bit.remote == 0u)
    {
        //wait 1/10th of a second to debounce
        OSTimeDlyHMSM(0u, 0u, 0u, 100u, OS_OPT_TIME_HMSM_STRICT, &tm_os_err);

        //re-read the key status
        *buttonFflag = GPIO_getButtons();
    }
    else //if arrive here from a remote DUCI command then just copy the received key code
    {
        *buttonFflag = flag;
    }

    return os_err;
}


/**
 * @brief Posts semaphore to GPIO interrupts
 * @param remote if true,instrument is in remote state else in local/service state
 * @param keys button for which interrupt is posted
 * @return os error flag for semaphore post staus
 */

OS_ERR gpioKeyPost(bool remote, gpioButtons_t keys)
{
    OS_ERR  os_err = OS_ERR_NONE;
    keys.bit.remote = (uint32_t)(remote) ? (uint32_t)1u : (uint32_t)(0u);
    flag = keys;
    OSSemPost(&gpioIntSem, OS_OPT_POST_ALL, &os_err );
    return os_err;
}


/**
 * @brief Sets the instrument poweroff state
 * and wakeup on interrupt,suspend ticks and deinit RCC,switch off software power
 */

void GPIOsleep(void)
{
    disableBacklight();
    extSensorOnOffLatchDisable();
    intPsDisable();
    uartDeInit();
    __HAL_RCC_CLEAR_RESET_FLAGS();
    enableOnOffIrqOnly();
    HAL_SuspendTick();
    HAL_RCC_DeInit();
    onOffSwDisable();
}


/**
 *  @brief External interrupt handler
 */

void EXTI1_IRQHandler(void)
{
    DisableInterrupts();
    OSIntEnter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    OSIntExit();
}


/**
*  @brief external interrupt handler
*/

void EXTI2_IRQHandler(void)
{
    DisableInterrupts();
    OSIntEnter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    OSIntExit();
}


/**
 *  @brief external interrupt handler
 */

void EXTI3_IRQHandler(void)
{
    DisableInterrupts();
    OSIntEnter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
    OSIntExit();
}


/**
 *  @brief external interrupt handler
 */

void EXTI4_IRQHandler(void)
{
    DisableInterrupts();
    OSIntEnter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    OSIntExit();
}


/**
 *  @brief external interrupt handler
 */

void EXTI9_5_IRQHandler(void)
{
    DisableInterrupts();
    OSIntEnter();

    for(uint8_t i=5u; i<=9u; i++)
    {
        HAL_GPIO_EXTI_IRQHandler(((uint16_t)1u<<i));
    }

    OSIntExit();
}


/**
 *  @brief external interrupt handler
 */

void EXTI15_10_IRQHandler(void)
{
    DisableInterrupts();
    OSIntEnter();

    for(uint8_t i=10u; i<=15u; i++)
    {
        HAL_GPIO_EXTI_IRQHandler(((uint16_t)1u<<i));
    }

    OSIntExit();
}


/**
 *  @brief external interrupt handler
 */

void EXTI0_IRQHandler(void)
{
    DisableInterrupts();
    OSIntEnter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    OSIntExit();
}


/**
 * @brief  call back function for ext1 interrupt
 * @param GPIO_Pin souce pin of interrupt
 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == EXTSENAL_GPIO_PIN)
    {
        overcurrentAlarmCallBack();
    }

    OS_ERR p_err = gpioKeyPost(false, GPIO_getButtons());

    if (p_err != OS_ERR_NONE)
    {
        //setError(E_ERROR_GPIO_TASK);
    }

    EnableInterrupts();
}


/**
 * @breif Enable clock for a specific port
 * @param port GPIO port
 */

static void gpioEnableClock(GPIO_TypeDef *port)
{
    uint32_t setport = (uint32_t)port;

    switch (setport)
    {
        case GPIOA_BASE:

            __HAL_RCC_GPIOA_CLK_ENABLE();
            break;

        case GPIOB_BASE:

            __HAL_RCC_GPIOB_CLK_ENABLE();
            break;

        case GPIOC_BASE:

            __HAL_RCC_GPIOC_CLK_ENABLE();
            break;

        case GPIOH_BASE:

            __HAL_RCC_GPIOH_CLK_ENABLE() ;
            break;

        default:
            //setError(E_ERROR_GPIO_DRIVER);
            break;
    }
}


/*
 *@brief Initialises power button and software on/off button
*/

void powerEnableInit(void)
{

    GPIO_InitTypeDef gpioSettings;

    /* Initialize soft on/off gpio*/
    gpioSettings.Mode  = GPIO_MODE_OUTPUT_PP;
    gpioSettings.Pin   = ON_OFF_SW_GPIO_PIN;
    gpioSettings.Pull  = GPIO_NOPULL;
    gpioSettings.Speed = GPIO_SPEED_FREQ_LOW;

    gpioEnableClock(ON_OFF_SW_GPIO_PORT);
    HAL_GPIO_Init(ON_OFF_SW_GPIO_PORT, &gpioSettings);

    /* Initialize gpio on/off button*/
    gpioSettings.Mode  = GPIO_MODE_IT_RISING_FALLING;
    gpioSettings.Pin   = ON_OFF_GPIO_PIN;
    gpioSettings.Pull  = GPIO_NOPULL;
    gpioSettings.Speed = GPIO_SPEED_FREQ_LOW;

    gpioEnableClock(ON_OFF_GPIO_PORT);
    HAL_GPIO_Init(ON_OFF_GPIO_PORT, &gpioSettings);
    HAL_NVIC_SetPriority((IRQn_Type)ON_OFF_EXTI_IRQn, 0x0Fu, 0x00u); //interrupt priority
    HAL_NVIC_EnableIRQ((IRQn_Type)ON_OFF_EXTI_IRQn);
}


/*
 *@brief Disable S/w on/off switch to remove power latching to processor
*/

void onOffSwDisable(void)
{
    HAL_GPIO_WritePin((GPIO_TypeDef *)ON_OFF_SW_GPIO_PORT,  ON_OFF_SW_GPIO_PIN, GPIO_PIN_RESET);

    /*This delay is to allow the FET to discharge completely. The leakage current for FET is 77uA which is much less
    than the Micro's requirement of 4mA to drive. So once the FET is discharged completely with the little leakage current,
    the micro will continue to remian in RESET state*/
    while(1)
    {

        watchdogRefresh();
    }
}