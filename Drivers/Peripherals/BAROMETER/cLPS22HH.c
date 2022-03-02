/*!
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the
* property of Baker Hughes and its suppliers, and affiliates if any.
* The intellectual and technical concepts contained herein are
* proprietary to Baker Hughes and its suppliers and affiliates
* and may be covered by U.S. and Foreign Patents, patents in
* process, and are protected by trade secret or copyright law.
* Dissemination of this information or reproduction of this
* material is strictly forbidden unless prior written permission
* is obtained from Baker Hughes.
*
* @file     cLPS22HH.c
*
* @author   Elvis Esa
* @date     July 2020
*
* This file is the barometer application driver which provides the function
* calls to the application to initialise, trigger, convert and close the
* barometer device LPS22HH.
*/

/* Includes ------------------------------------------------------------------*/
#include "misra.h"
MISRAC_DISABLE
#include "main.h"
#include <stdbool.h>
#include <string.h>
MISRAC_ENABLE
#include "cLPS22HH.h"
#include "I2c.h"

/* Imported Variables --------------------------------------------------------*/

#ifdef NO_HARDWARE_AVAILABLE
/* No import of SPI variable */
#else
//extern SPI_HandleTypeDef hspi1;
#endif

/* Exported types ------------------------------------------------------------*/

/* None */

/* Exported constants --------------------------------------------------------*/

const uint32_t cLPS22HH_Const_WhoAmI            = 0xB3u;

/* Exported macro ------------------------------------------------------------*/

/* None */

/* Exported functions --------------------------------------------------------*/

bool LPS22HH_initialise(eBaro_t eBaro);
bool LPS22HH_trigger(void);
bool LPS22HH_read(float *pPrs_hPa);
bool LPS22HH_close(void);

/* Private Functions prototypes ----------------------------------------------*/

static uint32_t LPS22HH_setupConfig(uint32_t pBlocking);
static float LPS22HH_convPrsData(const uint8_t *pPressData);
static float LPS22HH_convTempData(const uint8_t *pTempData);

/* Private includes ----------------------------------------------------------*/

/* None */

/* Private defines -----------------------------------------------------------*/

/*******************************/
#ifdef NO_HARDWARE_AVAILABLE

/* Simulated setting */

/*#define BAROM_INT_DRDY_PG2   // Barometer DRDY pin input */
#define DEF_LPS22HH_RD_DRDY    0u

#else

/*#define BAROM_INT_DRDY_PG3   // Barometer DRDY pin input */
#define DEF_LPS22HH_RD_DRDY    HAL_GPIO_ReadPin( GPIOG, GPIO_PIN_3 )

/* EETODO */
/* define I2C4_SCL_PD12 */
/* define I2C4_SDA_PD13 */

#endif
/*******************************/

/* LPS22HH CHIP Default Value on power up */
#define DEF_LPS22HH_REG_DEFAULT_IFCONFIG        (( uint8_t )0x00u )
#define DEF_LPS22HH_REG_DEFAULT_CTRLREG1        (( uint8_t )0x00u )
#define DEF_LPS22HH_REG_DEFAULT_CTRLREG2        (( uint8_t )0x10u )
#define DEF_LPS22HH_REG_DEFAULT_CTRLREG3        (( uint8_t )0x00u )
#define DEF_LPS22HH_REG_DEFAULT_FIFOCTRL        (( uint8_t )0x00u )
#define DEF_LPS22HH_REG_DEFAULT_FIFOWTM         (( uint8_t )0x00u )

/* LPS22HH CHIP Bits for IFCONFIG reg 0Eh)*/
#define DEF_LPS22HH_BIT_IFCONFIG_DIS_IC3        (( uint8_t )0x02u)
#define DEF_LPS22HH_BIT_IFCONFIG_DIS_PD         (( uint8_t )0x04u)

/* LPS22HH CHIP Register Bit Masks for used registers (unused registers are at default power values) */
// CTRLREG 1 (10h)
#define DEF_LPS22HH_BIT_CTRLREG1_ODR_MASK       (( uint8_t )0x8Fu)
#define DEF_LPS22HH_BIT_CTRLREG1_ODR_ONESHOT    (( uint8_t )0x00u)
#define DEF_LPS22HH_BIT_CTRLREG1_ODR_1HZ        (( uint8_t )0x10u)
#define DEF_LPS22HH_BIT_CTRLREG1_ODR_10HZ       (( uint8_t )0x20u)
#define DEF_LPS22HH_BIT_CTRLREG1_ODR_25HZ       (( uint8_t )0x30u)
#define DEF_LPS22HH_BIT_CTRLREG1_ODR_50HZ       (( uint8_t )0x40u)
#define DEF_LPS22HH_BIT_CTRLREG1_EN_LPFP        (( uint8_t )0x08u)
#define DEF_LPS22HH_BIT_CTRLREG1_LPFP_CFG       (( uint8_t )0x04u)
#define DEF_LPS22HH_BIT_CTRLREG1_BDU            (( uint8_t )0x02u)
#define DEF_LPS22HH_BIT_CTRLREG1_SIM            (( uint8_t )0x01u)

// CTRLREG 2 (11h)
#define DEF_LPS22HH_BIT_CTRLREG2_BOOT           (( uint8_t )0x80u)
#define DEF_LPS22HH_BIT_CTRLREG2_INT_H_L        (( uint8_t )0x40u)
#define DEF_LPS22HH_BIT_CTRLREG2_PP_OD          (( uint8_t )0x20u)
#define DEF_LPS22HH_BIT_CTRLREG2_IF_ADD_INC     (( uint8_t )0x10u)
#define DEF_LPS22HH_BIT_CTRLREG2_SWRESET        (( uint8_t )0x04u)
#define DEF_LPS22HH_BIT_CTRLREG2_LOW_NOISE_EN   (( uint8_t )0x02u)
#define DEF_LPS22HH_BIT_CTRLREG2_ONE_SHOT       (( uint8_t )0x01u)

// CTRLREG 3 (12h)
#define DEF_LPS22HH_BIT_CTRLREG3_DRDY_EN        (( uint8_t )0x04u)

//FIFO CTRL (13h)
#define DEF_LPS22HH_BIT_FIFOCTRL_F_MODE_MASK    (( uint8_t )0xF8u)
#define DEF_LPS22HH_BIT_FIFOCTRL_STOP_ON_WTM    (( uint8_t )0x08u)
#define DEF_LPS22HH_BIT_FIFOCTRL_F_MODE_BYPASS  (( uint8_t )0x00u)
#define DEF_LPS22HH_BIT_FIFOCTRL_F_MODE_FIFO    (( uint8_t )0x01u)

//FIFO WTM REG (14h)
#define DEF_LPS22HH_FIFO_WATERMARK              (( uint8_t )0x0Fu )

//INT_SOURCE (24h)
#define DEF_LPS22HH_BIT_INTSOURCE_BOOT_ON       (( uint8_t )0x80u )

//FIFO_STATUS2 (26h)
#define DEF_LPS22HH_BIT_FIFOSTATUS2_FIFO_WTM_IA (( uint8_t )0x80u )

/* Private variables ---------------------------------------------------------*/

_Pragma("diag_suppress=Pm093") /* DISABLE MISRA C 2004 CHECK for Rule 18.4 - using to convert bytes to integer in ADC conversion result */
typedef union
{
    int32_t value;
    uint8_t dataU8[4];
} uRdData_t;
_Pragma("diag_default=Pm093")  /* ENABLE MISRA C 2004 CHECK for Rule 18.4 */

/* Private consts ------------------------------------------------------------*/

/* LPS22HH Registers */
static const uint8_t cLPS22HH_Reg_IFConfig             = 0x0Eu;
static const uint8_t cLPS22HH_Reg_WhoAmI               = 0x0Fu;
static const uint8_t cLPS22HH_Reg_CtrlReg1             = 0x10u;
static const uint8_t cLPS22HH_Reg_CtrlReg2             = 0x11u;
static const uint8_t cLPS22HH_Reg_FifoCtrl             = 0x13u;
static const uint8_t cLPS22HH_Reg_IntSource            = 0x24u;

/* LPS22HH Chip Constants */
uint32_t gLPS22HH_Const_ICAddrShifted       = 0xBAu; /* 01011101 << 1 = 10111010 (0xBA) as LSB is R/W  */

/* Register Settings for IF config */
static const uint8_t cLPS22HH_Setting_IFConfig_IC3disable = (DEF_LPS22HH_REG_DEFAULT_IFCONFIG  | DEF_LPS22HH_BIT_IFCONFIG_DIS_PD  | DEF_LPS22HH_BIT_IFCONFIG_DIS_IC3);

/* Register Settings for SW reset */
static const uint8_t cLPS22HH_Setting_CtrlReg2_swReset = (DEF_LPS22HH_REG_DEFAULT_CTRLREG2 | DEF_LPS22HH_BIT_CTRLREG2_SWRESET);

/* Register Settings on Trigger */
static const uint8_t cLPS22HH_Setting_FifoCtrl_trig    = (DEF_LPS22HH_REG_DEFAULT_FIFOCTRL | DEF_LPS22HH_BIT_FIFOCTRL_STOP_ON_WTM | DEF_LPS22HH_BIT_FIFOCTRL_F_MODE_FIFO);
static const uint8_t cLPS22HH_Setting_CtrlReg1_trig    = (DEF_LPS22HH_REG_DEFAULT_CTRLREG1 | DEF_LPS22HH_BIT_CTRLREG1_ODR_25HZ | DEF_LPS22HH_BIT_CTRLREG1_EN_LPFP | DEF_LPS22HH_BIT_CTRLREG1_LPFP_CFG);

/* Setup Array : CtrlReg1(10h), CtrlReg2(11h), CtrlReg3(12h), FifoCtrl(13h), FifoWtm(14h) */
static const uint8_t cLPS22HH_setup_config[5]          = { (DEF_LPS22HH_REG_DEFAULT_CTRLREG1 | DEF_LPS22HH_BIT_CTRLREG1_ODR_ONESHOT | DEF_LPS22HH_BIT_CTRLREG1_EN_LPFP | DEF_LPS22HH_BIT_CTRLREG1_LPFP_CFG),
                                                           (DEF_LPS22HH_REG_DEFAULT_CTRLREG2 | DEF_LPS22HH_BIT_CTRLREG2_LOW_NOISE_EN),
                                                           (DEF_LPS22HH_REG_DEFAULT_CTRLREG3 | DEF_LPS22HH_BIT_CTRLREG3_DRDY_EN),
                                                           (DEF_LPS22HH_REG_DEFAULT_FIFOCTRL | DEF_LPS22HH_BIT_FIFOCTRL_STOP_ON_WTM | DEF_LPS22HH_BIT_FIFOCTRL_F_MODE_BYPASS),
                                                           (DEF_LPS22HH_FIFO_WATERMARK)
                                                         };

/* File constants */
static const uint8_t cWrSetupConfigLength              = (sizeof(cLPS22HH_setup_config));          // Configuration data write length
static const uint8_t cRdSetupConfigLength              = (sizeof(cLPS22HH_setup_config) + 1u);     // Configuration data read length (we also read ID i.e +1)

/*----------------------------------------------------------------------------*/

/*!
* @brief : This function initialises the LPS22HH.  It initially checks the BOOT and the SW_RESET flags
* To ensure these are not set (as they should not be set - checked for safety).  If the chip
* BOOT and RESET flags are clear, the chip is in the correct state to accept configuration settings,
* therefore the chip is setup and validated.  An issue with the BOOT or RESET or settings will result
* in an error flag being raised.
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - 1 = ok, 0 = fail
* @note          : SW_RESET is check because we use SW_RESET when closing the device - this is just for safety
*                  as SW_RESET takes 50us there is no chance we would initialise again within this time.
* @warning       : WARNING: Assumes instrument boot time is longer than the chip boot time of 5ms
*                : WARNING: SW_RESET is 50us
*/
bool LPS22HH_initialise(eBaro_t eBaro)
{
    uint32_t lError = 0u;
    bool lok = false;
    uint8_t lI2cData;

    switch(eBaro)
    {
    case eBARO_INTERNAL_ON_BOARD:
        gLPS22HH_Const_ICAddrShifted = 0xB8u; /* 01011100 << 1 = 10111000 (0xB8) as LSB is R/W */
        break;

    case eBARO_INTERNAL_LEADED:
    default:
        gLPS22HH_Const_ICAddrShifted = 0xBAu; /* 01011101 << 1 = 10111010 (0xBA) as LSB is R/W  */
        break;
    }

    // Read chip boot state
    if(I2C_ReadBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_IntSource, I2C_MEMADD_SIZE_8BIT, &lI2cData, 1u, DEF_NON_BLOCKING) != HAL_OK)
    {
        lError |= 1u;
    }

    else
    {
        //Validate chip BOOT state (The device should have booted as instrument take >5ms to power up - for safety only)
        if((lI2cData & DEF_LPS22HH_BIT_INTSOURCE_BOOT_ON) > 0u)
        {
            lError |= 1u;
        }

        else
        {
            // Read SW RESET of chip is complete (just for safety)
            if(I2C_ReadBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_CtrlReg2, I2C_MEMADD_SIZE_8BIT, &lI2cData, 1u, DEF_NON_BLOCKING) != HAL_OK)
            {
                lError |= 1u;
            }

            else
            {
                //Validate SW RESET
                if((lI2cData & DEF_LPS22HH_BIT_CTRLREG2_SWRESET) > 0u)
                {
                    lError |= 1u;
                }

                else
                {
                    lError |= LPS22HH_setupConfig(DEF_NON_BLOCKING);     // Setup and validate device if a valid ID and SW Boot/Reset is ok

                    // SET IF config register and turn off I3C and pull down resistor
                    MISRAC_DISABLE

                    if(I2C_WriteBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_IFConfig, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&cLPS22HH_Setting_IFConfig_IC3disable, (uint16_t)1u, DEF_NON_BLOCKING) != HAL_OK)
                    {
                        lError |= 1u;
                    }

                    MISRAC_ENABLE

                    // Read setting of IFCONFIG
                    if(I2C_ReadBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_IFConfig, I2C_MEMADD_SIZE_8BIT, &lI2cData, (uint16_t)1u, DEF_NON_BLOCKING) != HAL_OK)
                    {
                        lError |= 1u;
                    }

                    // Validate setting of IFCONFIG
                    if(lI2cData == cLPS22HH_Setting_IFConfig_IC3disable)
                    {
                        // All ok - Do nothing
                    }
                    else
                    {
                        lError |= 1u;
                    }

                    GPIO_InitTypeDef  GPIO_InitStruct;
                    // Change DRDY I/P to INPUT pull up enabled
                    GPIO_InitStruct.Pin = BAROM_INT_DRDY_PD11_Pin;
                    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
                    GPIO_InitStruct.Pull = GPIO_PULLUP;
                    HAL_GPIO_Init(BAROM_INT_DRDY_PD11_GPIO_Port, &GPIO_InitStruct);
                }
            }
        }
    }

    if(0u == lError)
    {
        lok = true;
    }

    return(lok);
}

/*!
* @brief : This function is called by the application to set the LPS22HH to start converting data to
* FIFO at the specified rate.  The device is setup with the FIFOCTRL reg set to FIFO and the CTRLREG1
* set to start accumulating samples.
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - 1 = ok, 0 = fail
* @note          : None
* @warning       : NOT VALIDATED (i.e settings to LPS22HH are not readback) as its done periodically
*/
bool LPS22HH_trigger(void)
{
    uint8_t data;
    uint32_t lError = 0u;
    bool lok = false;

    // Done this way for misra
    data = cLPS22HH_Setting_FifoCtrl_trig;

    // Set LPS22HH to accumulate into FIFO
    if(I2C_WriteBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_FifoCtrl, I2C_MEMADD_SIZE_8BIT, &data, 1u, DEF_NON_BLOCKING) != HAL_OK)
    {
        lError |= 1u;
    }

    // Done this way for misra
    data = cLPS22HH_Setting_CtrlReg1_trig;

    // Set LPS22HH to start SAMPLING
    if(I2C_WriteBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_CtrlReg1, I2C_MEMADD_SIZE_8BIT, &data, 1u, DEF_NON_BLOCKING) != HAL_OK)
    {
        lError |= 1u;
    }

    if(0u == lError)
    {
        lok = true;
    }

    return(lok);
}

/*!
* @brief : This function is called by the application to read the pressure data
* from the LPS22HH that is encoded as hPa. The device is set back to BYPASS and ONE_SHOT
* to clear FIFO and conserve power respectively.
*
* @param[in]     : None
* @param[out]    : float* pPrs_hPa - pointer to write converted pressure value (hPa).
* @param[in,out] : None
* @return        : bool lok - 1 = ok, 0 = fail
* @note          : None
* @warning       : Call this only when the conversion time (i.e time to fill WTM level by the device)
*                  has passed as semaphores are not used e.g @25Hz ODR = WTM/25 = 128/25 = 5.12 seconds.
*/
bool LPS22HH_read(float *pPrs_hPa)
{
    uint32_t lError = 0u;
    uint8_t  lI2cRdData[5];
    bool lok = false;

    if(DEF_LPS22HH_RD_DRDY)
    {
        // PV (LSB) = (OUT_H & OUT_L & OUT_XL) / 4096 = hPa
        //cLPS22HB_RegPressOutXL_Addr, cTotalNoOfPrsTempRegs

        //0x28 = P_XL
        //0x29 = P_L
        //0x2A = P_H
        //0x2B = T_L
        //0x2C = T_H

        if(I2C_ReadBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)0x28u, I2C_MEMADD_SIZE_8BIT, &lI2cRdData[0], 5u, DEF_NON_BLOCKING) != HAL_OK)
        {
            lError |= 1u;
        }

        // **** EE TODO : Await response from ST about sample Discard for >99.99% */

        *pPrs_hPa = LPS22HH_convPrsData(lI2cRdData);

        // Invalidate barometer reading if pressure measurement is outside acceptable range
        if((*pPrs_hPa < 675.0f) || (*pPrs_hPa > 1265.0f))
        {
            lError |= 1u;
        }

        // Read temperature data
        if(I2C_ReadBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)0x2Bu, I2C_MEMADD_SIZE_8BIT, &lI2cRdData[0], 5u, DEF_NON_BLOCKING) != HAL_OK)
        {
            lError |= 1u;
        }

        // Invalidate barometer reading if temperature measurement is outside acceptable range
        float temperature = LPS22HH_convTempData(lI2cRdData);

        if((temperature < -10.0f) || (temperature > 50.0f))
        {
            lError |= 1u;
        }
    }

    else
    {
        lError |= 1u;
    }

    // Configure LPS22HH to startup defaults (low power / bypass)
    lError |= LPS22HH_setupConfig(DEF_NON_BLOCKING);

    if(0u == lError)
    {
        lok = true;
    }

    return(lok);
}

/*!
* @brief : This function set the device back to defaults ( low power )
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - 1 = ok, 0 = fail
* @note          : None
* @warning       : None
*/
bool LPS22HH_close(void)
{
    uint8_t data;
    uint32_t lError = 0u;
    bool lok = false;

    // Done this way for misra
    data = DEF_LPS22HH_REG_DEFAULT_CTRLREG1;

    // Set LPS22HH to restore ODR oneshot mode for low power mode
    if(I2C_WriteBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_CtrlReg1, I2C_MEMADD_SIZE_8BIT, &data, 1u, DEF_NON_BLOCKING) != HAL_OK)
    {
        lError |= 1u;
    }

    // Done this way for misra
    data = cLPS22HH_Setting_CtrlReg2_swReset;

    // Set LPS22HH to RESET chip (low power)
    if(I2C_WriteBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_CtrlReg2, I2C_MEMADD_SIZE_8BIT, &data, 1u,  DEF_NON_BLOCKING) != HAL_OK)
    {
        lError |= 1u;
    }

    if(0u == lError)
    {
        lok = true;
    }

    return(lok);
}

/*!
* @brief : This function initialises the LPS22HH to the required configuration but the device
* is left in a low power state. The configuration data is written to the device and validated (including ID)
* to ensure we have the correct device and parameter settings.
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : Device is left in a low power state (no collecting of data at this point)
* @warning       : WARNING: The chip defaults are the base of the configuration - we only change
*                  what is required.
*/
static uint32_t LPS22HH_setupConfig(uint32_t pBlocking)
{
    uint32_t lError = 0u;
    uint8_t  lI2cData[(sizeof(cLPS22HH_setup_config) + 1u)]; // Config [5] + whoami data [1]

// Exception Need to disable MISRA rule 11.5 as we have a constant array to send
    MISRAC_DISABLE

    //Initialisation block to write
    //Write data to registers (5 items)
    if(I2C_WriteBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_CtrlReg1, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&cLPS22HH_setup_config[0], (uint16_t)cWrSetupConfigLength, pBlocking) != HAL_OK)
    {
        lError |= 1u;
    }

    MISRAC_ENABLE

    //Block read registers including chip ID (6 items)
    if(I2C_ReadBuffer(I2Cn4, (uint16_t)gLPS22HH_Const_ICAddrShifted, (uint16_t)cLPS22HH_Reg_WhoAmI, I2C_MEMADD_SIZE_8BIT, &lI2cData[0], (uint16_t)cRdSetupConfigLength, pBlocking) != HAL_OK)
    {
        lError |= 1u;
    }

    // Validate ID & written register settings
    if((lI2cData[0] == cLPS22HH_Const_WhoAmI)
            && (lI2cData[1] == cLPS22HH_setup_config[0])
            && (lI2cData[2] == cLPS22HH_setup_config[1])
            && (lI2cData[3] == cLPS22HH_setup_config[2])
            && (lI2cData[4] == cLPS22HH_setup_config[3])
            && (lI2cData[5] == cLPS22HH_setup_config[4]))
    {
        // Everything matches - ok
    }
    else
    {
        lError |= 1u;
    }

    return(lError);
}

/*!
* @brief : This function converts the 24bit pressure data into pressure (hPa)
*
* @param[in]     : const uint8_t* pPressData - pointer to the i2c pressure data
* @param[out]    : None
* @param[in,out] : None
* @return        : float - converted pressure value (hPa)
* @note          : None
* @warning       : None
*/
static float LPS22HH_convPrsData(const uint8_t *pPressData)
{
    _Pragma("diag_suppress=Pm093") /* DISABLE MISRA C 2004 CHECK for Rule 18.4 - using to convert bytes to integer in ADC conversion result */
    uRdData_t lPressure;
    //lPressure.value = 0;
    memset((uint8_t *)&lPressure, 0, sizeof(uRdData_t));
    _Pragma("diag_default=Pm093")  /* ENABLE MISRA C 2004 CHECK for Rule 18.4 */

    //pPressData : LSB->XL L H<-MSB
    lPressure.dataU8[0] = pPressData[0];        // XL in Lower-LSB (L)
    lPressure.dataU8[1] = pPressData[1];        // L in Upper-LSB (H)
    lPressure.dataU8[2] = pPressData[2];        // H in Lower-MSB (L)

    //Add correct MSB bits in int32 (as data is 24bit inc. sign)
    if((lPressure.dataU8[2] & 0x80u) > 0u)     // 00 or FF in Upper MSB depending upon sign
    {
        lPressure.dataU8[3] = 0xffu;              // -Ve : FF in Upper MSB
    }

    else
    {
        lPressure.dataU8[3] = 0x00u;              // +Ve : 00 in Upper MSB
    }

    return(((float)lPressure.value) / 4096.0f);
}

/*!
* @brief : This function converts the 24bit temperature data into temperature (degC)
*
* @param[in]     : const uint8_t* pTempData - pointer to the i2c temperature data
* @param[out]    : None
* @param[in,out] : None
* @return        : float - converted temperature value (degC)
* @note          : None
* @warning       : None
*/
static float LPS22HH_convTempData(const uint8_t *pTempData)
{
    _Pragma("diag_suppress=Pm093") /* DISABLE MISRA C 2004 CHECK for Rule 18.4 - using to convert bytes to integer in ADC conversion result */
    uRdData_t lTemperature;
    //lTemperature.value = 0;
    memset((uint8_t *)&lTemperature, 0, sizeof(uRdData_t));
    _Pragma("diag_default=Pm093")  /* ENABLE MISRA C 2004 CHECK for Rule 18.4 */

    lTemperature.dataU8[0] = pTempData[0];        // TEMP_OUT_L
    lTemperature.dataU8[1] = pTempData[1];        // TEMP_OUT_H

    //Add correct MSB bits in int32 (as data is 16bit inc. sign)
    if((lTemperature.dataU8[1] & 0x80u) > 0u)     // 00 or FF in Upper MSB depending upon sign
    {
        lTemperature.dataU8[2] = 0xffu;              // -Ve : FF in Upper MSB
        lTemperature.dataU8[3] = 0xffu;              // -Ve : FF in Upper MSB
    }

    else
    {
        lTemperature.dataU8[2] = 0x00u;              // +Ve : 00 in Upper MSB
        lTemperature.dataU8[3] = 0x00u;              // +Ve : 00 in Upper MSB
    }

    return(((float)lTemperature.value) / 100.0f);
}
