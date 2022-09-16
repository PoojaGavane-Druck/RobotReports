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
* @file     cBL652.c
*
* @author   Elvis Esa
* @date     Nov 2020
*
* This file consists of the BL652 functions to setup the device mode specifically
* RUN and TEST mode and provide functions to the application to
* instigate radio testing of TX Single frequency (Variable + constant carrier
* all specific parameters are modifiable (validated at the low level functions).
* RX single frequency parameters modyfiable.  Also provide function to the application
* to enable to switch modes (validated) and end test (report).
*/

/* Includes ------------------------------------------------------------------*/

#include "misra.h"
MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
MISRAC_ENABLE
#include "Utilities.h"
#include "cBL652.h"
#include "UART.h"

/* Imported Variables --------------------------------------------------------*/

#define WAIT_TILL_END_OF_FRAME_RECEIVED 0u

#define OK_RESPONSE_LENGTH      9
extern UART_HandleTypeDef huart1;  // BLE Uart (DPI610E)

/* Exported types ------------------------------------------------------------*/

/* None */

/* Exported constants --------------------------------------------------------*/

/* None */

/* Exported macro ------------------------------------------------------------*/

/* None */

/* Exported functions --------------------------------------------------------*/

int32_t BL652_getReport(void);
void BL652_incPingCount(void);
int32_t BL652_getPingCount(void);
void BL652_setPingCount(const int32_t value);
bool BL652_initialise(const eBL652mode_t pMode);
uint32_t BL652_sendAtCmd(const eBLE652commands_t pAtCmd);
bool BL652_dtmEndTest(uint16_t *pReportOut);
bool BL652_dtmRXtest(const int16_t pFreq, const uint8_t pPhy);
bool BL652_dtmTXtest(const int16_t pFreq, const uint8_t pPhy, const uint8_t pPktType, const uint8_t pPktLen, const int8_t pTxPower);
uint32_t BL652_setAdvertName(uint8_t *serialNum);

/* Private Functions prototypes ----------------------------------------------*/

static uint32_t BL652_DTM_Test_Setup(const eBL652dtmTestSetupControl_t pControl, const uint8_t pParameter);
static uint32_t BL652_DTM_Test_End(const eBL652dtmTestEndControl_t pControl, const uint8_t pParameter, uint16_t *pReportOut);
static uint32_t BL652_DTM_Test_TxRx(const eBL652dtmCmd_t pCmd, const int16_t pFrequency, const uint8_t pLength, const eBL652dtmTxRxTestPkt_t pPktType);
static uint32_t BL652_vfyReply(const eBLE652commands_t pAtCmd, uint8_t *pStr);
static uint32_t BL652_DTM_Test_Exit(void);
static uint32_t BL652_setATmode(void);
static uint32_t BL652_setDTMmode(void);
static uint32_t BL652_mode(eBL652mode_t pMode);
static uint32_t BL652_txRxDtm(uint32_t *pSdata, uint32_t *pRdata);
static uint32_t BL652_ValidateEvent(const eBLE652Event_t pExpectedEvent, const uint32_t pActualEvent);
static uint32_t BL652_setDTMframe(uint8_t *pMsg);
static uint32_t BL652_sendAT_Null(void);
static uint32_t BL652_sendAT_Dtm(void);
static uint32_t BL652_sendDTM_Null(void);

/* Private defines -----------------------------------------------------------*/

#ifdef NO_HARDWARE_AVAILABLE

/* Simulated setting */
#define DEF_BL652_DISABLE()        {__NOP();}
#define DEF_BL652_ENABLE()         {__NOP();}

#define DEF_BL652_DEVMODE()        {__NOP();}
#define DEF_BL652_RUNMODE()        {__NOP();}

#else

/*#define */
#define DEF_BL652_DISABLE()        {HAL_GPIO_WritePin( GPIOB, GPIO_PIN_9, GPIO_PIN_RESET );DEF_DELAY_TX_10ms;}//sleep(2u);}
#define DEF_BL652_ENABLE()         {HAL_GPIO_WritePin( GPIOB, GPIO_PIN_9, GPIO_PIN_SET );DEF_DELAY_TX_10ms;}//sleep(2u);}

#define DEF_BL652_DEVMODE()        {HAL_GPIO_WritePin( GPIOD, GPIO_PIN_7, GPIO_PIN_SET );}
#define DEF_BL652_RUNMODE()        {HAL_GPIO_WritePin( GPIOD, GPIO_PIN_7, GPIO_PIN_RESET );}

/*#define BT_USART1_TX_PB6  ->O  */
/*#define BT_USART1_RX_PB7  <-I  */

#endif

#define DEF_DTM_BAUDRATE                        (( uint32_t )19200u )
#define DEF_AT_BAUDRATE                         (( uint32_t )115200u )

//As per Nordic
#define DEF_RXTEST_DEFAULT_LENGTH               (( uint8_t )0x10u )
#define DEF_RXTEST_DEFAULT_PWR                  (( int16_t )4 )
#define DEF_RXTEST_DEFAULT_PKTTYPE              (( eBL652dtmTxRxTestPkt_t )0x00u )
#define DEF_TXTEST_VDR_MIN_TXPOWER              (( int )-40 )
#define DEF_TXTEST_VDR_MAX_TXPOWER              (( int )4 )

#define DEF_STR_AT_CMD_NULL                     "AT\r"
#define DEF_STR_AT_CMD_MAC                      "ATI 14\r"
#define DEF_STR_AT_CMD_SWV                      "ATI 3\r"
#define DEF_STR_AT_CMD_DEV                      "ATI 0\r"
#define DEF_STR_AT_CMD_DIR                      "AT+DIR\r"
#define DEF_STR_AT_CMD_RUN                      "AT+RUN \"$autorun$\"\r"
#define DEF_STR_AT_CMD_FS_CLR                   "AT&F*\r"
#define DEF_STR_AT_CMD_DTM                      dtmATmsg
#define DEF_STR_AT_RPY_NULL                     "\n00\r"
#define DEF_STR_AT_RPY_MAC                      "\n10\t14\t01 &&&&&&&&&&&&\r"
#define DEF_STR_AT_RPY_SWV                      "\n10\t3\t##.#.#.#\r"
#define DEF_STR_AT_RPY_DEV                      "\n10\t2\tBL652\r"
#define DEF_STR_AT_RPY_DIR                      "\n06\t$autorun$\r"

#define DEF_BL652_ASCII_NUMBER_MIN              (( uint32_t )0x2Fu )
#define DEF_BL652_ASCII_NUMBER_MAX              (( uint32_t )0x3Au )
#define DEF_BL652_ASCII_LETTER_MIN              (( uint32_t )0x40u )
#define DEF_BL652_ASCII_LETTER_MAX              (( uint32_t )0x5Bu )

#define DEF_BL652_DTM_SETUP_CMD                 (( uint16_t )0x0000u )
#define DEF_BL652_DTM_RXTEST_CMD                (( uint16_t )0x4000u )
#define DEF_BL652_DTM_TXTEST_CMD                (( uint16_t )0x8000u )
#define DEF_BL652_DTM_TESTEND_CMD               (( uint16_t )0xC000u )
#define DEF_BL652_DTM_CMD_MASK                  (( uint16_t )0xC000u )

#define eBL652_DTM_SETUP_PARAM_RESET            (( uint16_t )0x00u )
#define eBL652_DTM_SETUP_PARAM_CLEAR_DL_BITS    (( uint16_t )0x00u )
#define eBL652_DTM_SETUP_PARAM_REC_TX_STANDARD  (( uint16_t )0x00u )
#define eBL652_DTM_TSTEND_PARAM                 (( uint16_t )0x00u )
#define eBL652_DTM_BASE_FREQUENCY               (( uint16_t )2402u )

#define DEF_RETIRES_U32                         (( uint32_t )3u )
#define DEF_MAX_INTEGER                         (( int32_t  )0x7FFFFFFFu )
#define DEF_BL652_DTM_EXIT_CMD                  (( uint32_t )0x00003FFFu )

#define DEF_DELAY_TX_10ms                       sleep(20u)

#define FOR_ADVERTISEMENT_SERIAL_NUMBER_START_INDEX   4
#define SERIAL_NUMBER_START_INDEX_IN_SBA_COMMAND      6
#define DEVICE_SERIAL_NUMBER_LENGTH 12
/* Private variables ---------------------------------------------------------*/

static uint8_t dtmATmsg[] = "AT+DTM 0x&&&&&&&&\r";
static eBL652mode_t gMode = eBL652_MODE_DISABLE;

static int32_t gTestEndreport;
static int32_t gPingCount;
static uint8_t  recMsg[DEF_BL652_MAX_REPLY_BUFFER_LENGTH];

static uint8_t AdvertName[] = "PV624_xxxxxxxxxxx\r";
static uint8_t sbaCmdStartAdvertising[15] = "ZZZ PV        ";

static uint8_t okResponse[]      = "#BR132!\n\r";

/* Private consts ------------------------------------------------------------*/

// NOTE: # = numeric, @ = alpha/letter, & = alphanumeric
static const sBLE652commands_t sBLE652atCommand[eBL652_CMD_MAX] =  {{ DEF_STR_AT_CMD_DEV, sizeof(DEF_STR_AT_CMD_DEV) - 1u, DEF_STR_AT_RPY_DEV, sizeof(DEF_STR_AT_RPY_DEV) - 1u },
    { DEF_STR_AT_CMD_SWV, sizeof(DEF_STR_AT_CMD_SWV) - 1u, DEF_STR_AT_RPY_SWV, sizeof(DEF_STR_AT_RPY_SWV) - 1u },
    { DEF_STR_AT_CMD_MAC, sizeof(DEF_STR_AT_CMD_MAC) - 1u, DEF_STR_AT_RPY_MAC, sizeof(DEF_STR_AT_RPY_MAC) - 1u },
    { DEF_STR_AT_CMD_NULL, sizeof(DEF_STR_AT_CMD_NULL) - 1u, DEF_STR_AT_RPY_NULL, sizeof(DEF_STR_AT_RPY_NULL) - 1u },
    { DEF_STR_AT_CMD_DTM, sizeof(dtmATmsg) - 1u, "", 0u },
    { DEF_STR_AT_CMD_DIR, sizeof(DEF_STR_AT_CMD_DIR) - 1u, DEF_STR_AT_RPY_DIR, sizeof(DEF_STR_AT_RPY_DIR) - 1u},
    { DEF_STR_AT_CMD_RUN, sizeof(DEF_STR_AT_CMD_RUN) - 1u, "", 0u },
    { DEF_STR_AT_CMD_FS_CLR, sizeof(DEF_STR_AT_CMD_FS_CLR) - 1u, "", 0u },
};

/************************************/
/* Bluetooth core specification 5.2 */
/************************************/
static const sBLE652dtmParameterRanges_t sBL652dtmTstSetupParamRng[eBL652_DTM_TS_CONTROL_MAX] =   {{ 0x00u, 0x03u, 0x00u },
    { 0x00u, 0x0Fu, 0x00u },
    { 0x04u, 0x13u, 0x00u },
    { 0x00u, 0x07u, 0x00u },
    { 0x00u, 0x03u, 0x00u },
    { 0x00u, 0x10u, 0x00u },
    { 0x00u, 0xFFu, 0x00u },
    { 0x01u, 0x02u, 0x00u },
    { 0x01u, 0x4Bu, 0x01u }
};


static const sBLE652dtmParameterRanges_t sBL652dtmTstEndParamRng    = { 0x00u, 0x03u, 0x00u };
static const sBLE652dtmParameterRanges_t sBL652dtmTxRxFreqParamRng  = { 0x00u, 0x27u, 0x00u };
static const sBLE652dtmParameterRanges_t sBL652dtmTxRxLenParamRng   = { 0x00u, 0x3Fu, 0x00u };
static const sBLE652dtmParameterRanges_t sBL652dtmTxRxPktParamRng   = { 0x00u, 0x01u, 0x00u }; // only allows 0 & 1
static uint8_t deviceSerialNumber[DEVICE_SERIAL_NUMBER_LENGTH] = "0123456789";

/*- External Accessible Functions --------------------------------------------*/
/*!
* @brief : This function sets the Bluetooth advertisement name
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : None
* @note          : None
* @warning       : None
*/
uint32_t BL652_setAdvertName(uint8_t *serialNum)
{
    uint32_t lError = 0u;
    uint32_t NameIndex = 8u;
    uint16_t numofBytesReceived = 0u;

    // Add the serial number to the advert name after the product name
    for(NameIndex = 8u; NameIndex < 19u; NameIndex++)
    {
        AdvertName[NameIndex] = *serialNum++;
    }

    if(false == ClearUARTxRcvBuffer(UART_PORT1))
    {
        lError |= 1u;
    }

    // Send the advert name to the BL652 via the UART
    if(false == sendOverUSART1(&AdvertName[0], (uint32_t)sizeof(AdvertName)))
    {
        lError |= 1u;
    }

    if(false == waitToReceiveOverUsart1(WAIT_TILL_END_OF_FRAME_RECEIVED, 5000u))
    {
        lError |= 1u;
        recMsg[sizeof(lError)] = '\0';
        memcpy_s(recMsg, DEF_BL652_MAX_REPLY_BUFFER_LENGTH, (uint8_t *)&lError, (uint32_t)sizeof(lError));
    }

    else
    {
        uint8_t *replyPtr = NULL;
        getHandleToUARTxRcvBuffer(UART_PORT1, (uint8_t **)&replyPtr);
        getAvailableUARTxReceivedByteCount(UART_PORT1,
                                           (uint16_t *) &numofBytesReceived);
        memset_s(recMsg,  sizeof(recMsg),  0, sizeof(recMsg));

        if(numofBytesReceived >= DEF_BL652_MAX_REPLY_BUFFER_LENGTH)
        {
            numofBytesReceived = (uint16_t)DEF_BL652_MAX_REPLY_BUFFER_LENGTH - 1u;
        }

        memcpy_s(recMsg, DEF_BL652_MAX_REPLY_BUFFER_LENGTH, replyPtr, (uint32_t)numofBytesReceived);
    }

    //set the termination type, communication type and baud rate of the uart to receive data
    lError |= UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Slave, eUARTn_Baud_115200);

    if(false == ClearUARTxRcvBuffer(UART_PORT1))
    {
        lError |= 1u;
    }

    DEF_DELAY_TX_10ms;
    return lError;
}

/*!
* @brief : This function initialises the BL652 into the mode requested
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - true = ok, false = fail
* @note          : None
* @warning       : None
*/
bool BL652_initialise(const eBL652mode_t pMode)
{
    uint32_t lError = 0u;
    bool lok = true;

    gTestEndreport = 0;
    gPingCount = 0;

    lError = BL652_mode(pMode);

    if(lError)
    {
        lok = false;
    }

    return(lok);
}

/*!
* @brief : This function transmits the AT frame in pSdata, and places the received response in pRdata
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
uint32_t BL652_sendAtCmd(const eBLE652commands_t pAtCmd)
{
    uint32_t lError = 0u;
    uint16_t numofBytesReceived = 0u;

    if(pAtCmd >= eBL652_CMD_MAX)
    {
        lError = 1u;
    }

    else
    {
        if(false == sendOverUSART1(sBLE652atCommand[pAtCmd].cmdSend, (uint32_t)sBLE652atCommand[pAtCmd].cmdSendLength))
        {
            lError |= 1u;
        }

        if(false == waitToReceiveOverUsart1(WAIT_TILL_END_OF_FRAME_RECEIVED, 250u))
        {
            lError |= 1u;
            recMsg[sizeof(lError)] = '\0';
            memcpy_s(recMsg, DEF_BL652_MAX_REPLY_BUFFER_LENGTH, (uint8_t *)&lError, (uint32_t)sizeof(lError));
        }

        else
        {
            getAvailableUARTxReceivedByteCount(UART_PORT1,
                                               (uint16_t *) &numofBytesReceived);

            if(sBLE652atCommand[pAtCmd].cmdReplyLength == numofBytesReceived)
            {
                uint8_t *replyPtr = NULL;
                size_t replyLength = (size_t)0;
                getHandleToUARTxRcvBuffer(UART_PORT1, (uint8_t **)&replyPtr);
                lError |= BL652_vfyReply(pAtCmd, replyPtr);

                if(0u == lError)
                {
                    memset_s(recMsg, sizeof(recMsg), 0, sizeof(recMsg));

                    if((uint16_t)sBLE652atCommand[pAtCmd].cmdReplyLength >= DEF_BL652_MAX_REPLY_BUFFER_LENGTH)
                    {
                        replyLength = (uint16_t)DEF_BL652_MAX_REPLY_BUFFER_LENGTH - 1u;
                    }

                    else
                    {
                        replyLength = (size_t)sBLE652atCommand[pAtCmd].cmdReplyLength;
                    }

                    memcpy_s(recMsg, DEF_BL652_MAX_REPLY_BUFFER_LENGTH, replyPtr, (uint32_t)replyLength);
                }
            }

            else
            {
                lError |= 1u;
            }
        }
    }

    // Clear error if its a DTM mode set command as it doesn't have a reply
    if((lError) && (pAtCmd == eBL652_CMD_DTM))
    {
        lError = 0u;
    }

    if(pAtCmd == eBL652_CMD_RUN)
    {
        //Set the global mode indication to be RUN mode
        gMode = eBL652_MODE_RUN;

        //set the termination type, communication type and baud rate of the uart to receive data
        //lError |= UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Slave, eUARTn_Baud_115200);
    }

    if(false == ClearUARTxRcvBuffer(UART_PORT1))
    {
        lError |= 1u;
    }

    DEF_DELAY_TX_10ms;

    return(lError);
}

/*!
* @brief : This function end the radio test by sending the testEnd command
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - true = ok, false = fail
* @note          : None
* @warning       : None
*/
bool BL652_dtmEndTest(uint16_t *pReportOut)
{
    uint32_t lError = 0u;
    bool lok = true;

    if((gMode == eBL652_MODE_TESTING) && (pReportOut != NULL))
    {
        lError = BL652_DTM_Test_End(eBL652_DTM_TE_CONTROL_CMD, (uint8_t)eBL652_DTM_TSTEND_PARAM, pReportOut);
    }

    else
    {
        lError = 1u;
    }

    if(lError)
    {
        lok = false;
        gTestEndreport = 0;
    }

    else
    {
        if(pReportOut != NULL)
        {
            gTestEndreport = (int32_t)(*pReportOut);
            gMode = eBL652_MODE_DTM;
        }
    }

    return(lok);
}

/*!
* @brief : This function configures the BL652 with the transmit test parameters
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - true = ok, false = fail
* @note          : All parameters are validated at the lower level
* @warning       : None
*/
//uint32_t testArray[50];
//uint32_t testArrayIndex;
bool BL652_dtmTXtest(const int16_t pFreq, const uint8_t pPhy, const uint8_t pPktType, const uint8_t pPktLen, const int8_t pTxPower)
{
    uint32_t lError = 0u;
    bool lok = true;

//    for( int i = 0; i < 50; i++ ) testArray[i] = 0x55555555u;
//    testArrayIndex = 0u;

    uint8_t lPhy = (pPhy * 0x04u) +  0x04u;

    if(gMode == eBL652_MODE_DTM)
    {

        if(0u == lError)
        {
            lError |= BL652_DTM_Test_Setup(eBL652_DTM_TS_CONTROL_RESET, (uint8_t)eBL652_DTM_SETUP_PARAM_RESET);
        }

        if(0u == lError)
        {
            lError |= BL652_DTM_Test_TxRx(eBL652_DTM_CMD_TX_TEST, (int16_t)(pTxPower), eBL652_DTM_VDRSPEC_TXPOWER, eBL652_DTM_TXRX_PKT_VDRSPEC);
        }

        if(0u == lError)
        {
            lError |= BL652_DTM_Test_Setup(eBL652_DTM_TS_CONTROL_UPPERLENGTH, (uint8_t)eBL652_DTM_SETUP_PARAM_CLEAR_DL_BITS);
        }

        if(0u == lError)
        {
            lError |= BL652_DTM_Test_Setup(eBL652_DTM_TS_CONTROL_MODINDEX, (uint8_t)eBL652_DTM_SETUP_PARAM_REC_TX_STANDARD);
        }

        if(pPktType < (uint8_t)eBL652_DTM_TXRX_PKT_ALT)    // PRBS, 11110000 only allowed - anything else is CC
        {
            if(0u == lError)
            {
                lError |= BL652_DTM_Test_Setup(eBL652_DTM_TS_CONTROL_PHY, lPhy);
            }

            if(0u == lError)
            {
                lError |= BL652_DTM_Test_TxRx(eBL652_DTM_CMD_TX_TEST, (pFreq), pPktLen, (eBL652dtmTxRxTestPkt_t)pPktType);
            }
        }

        else
        {
            if(0u == lError)
            {
                lError |= BL652_DTM_Test_TxRx(eBL652_DTM_CMD_TX_TEST, (pFreq), eBL652_DTM_VDRSPEC_CC, eBL652_DTM_TXRX_PKT_VDRSPEC);
            }
        }
    }

    else
    {
        lError |= 1u;
    }

    if(lError)
    {
        lok = false;
    }

    else
    {
        gMode = eBL652_MODE_TESTING;
        gTestEndreport = 0;
    }

    return(lok);
}

/*!
* @brief : This function configures the BL652 with the receive test parameters
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - true = ok, false = fail
* @note          : All parameters are validated at the lower level
* @warning       : None
*/
bool BL652_dtmRXtest(const int16_t pFreq, const uint8_t pPhy)
{
    uint32_t lError = 0u;
    bool lok = true;

//    for( int i = 0; i < 50; i++ ) testArray[i] = 0x55555555u;
//    testArrayIndex = 0u;

    uint8_t lPhy = (pPhy * 0x04u) +  0x04u;

    if((gMode == eBL652_MODE_DTM))
    {
        if(0u == lError)
        {
            lError |= BL652_DTM_Test_Setup(eBL652_DTM_TS_CONTROL_RESET, (uint8_t)eBL652_DTM_SETUP_PARAM_RESET);
        }

        if(0u == lError)
        {
            lError |= BL652_DTM_Test_TxRx(eBL652_DTM_CMD_TX_TEST, DEF_RXTEST_DEFAULT_PWR, (uint8_t)eBL652_DTM_VDRSPEC_TXPOWER, eBL652_DTM_TXRX_PKT_VDRSPEC);
        }

        if(0u == lError)
        {
            lError |= BL652_DTM_Test_Setup(eBL652_DTM_TS_CONTROL_UPPERLENGTH, (uint8_t)eBL652_DTM_SETUP_PARAM_CLEAR_DL_BITS);
        }

        if(0u == lError)
        {
            lError |= BL652_DTM_Test_Setup(eBL652_DTM_TS_CONTROL_MODINDEX, (uint8_t)eBL652_DTM_SETUP_PARAM_REC_TX_STANDARD);
        }

        if(0u == lError)
        {
            lError |= BL652_DTM_Test_Setup(eBL652_DTM_TS_CONTROL_PHY, lPhy);
        }

        if(0u == lError)
        {
            lError |= BL652_DTM_Test_TxRx(eBL652_DTM_CMD_RX_TEST, (pFreq), DEF_RXTEST_DEFAULT_LENGTH, DEF_RXTEST_DEFAULT_PKTTYPE);
        }
    }

    else
    {
        lError = 1u;
    }

    if(lError)
    {
        lok = false;
    }

    else
    {
        gMode = eBL652_MODE_TESTING;
        gTestEndreport = 0;
    }

    return(lok);
}

/*!
* @brief : This function gets the RX test report for Radio Test
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : int32_t gTestEndreport - the test report (inc ack)
* @note          : None
* @warning       : Cleared at start of a test (RX or TX)
*/
int32_t BL652_getReport(void)
{
    return(gTestEndreport);
}

/*!
* @brief : This function gets the interactive mode command response
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint8_t* - address of the response string
* @note          : None
* @warning       : None
*/
uint8_t *BL652_getResponse(void)
{
    return(&recMsg[0]);
}

/*!
* @brief : This function sets the Ping Count to a value for Radio Test
*
* @param[in]     : int32_t value - value to set the ping count to
* @param[out]    : None
* @param[in,out] : None
* @return        : None
* @note          : None
* @warning       : None
*/
void BL652_setPingCount(const int32_t value)
{
    gPingCount = value;
}

/*!
* @brief : This function gets the Ping Count for Radio Test
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : int32_t gPingCount - the current pingcount value
* @note          : None
* @warning       : None
*/
int32_t BL652_getPingCount(void)
{
    return(gPingCount);
}

/*!
* @brief : This function increments the Ping Count for Radio Test
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : None
* @note          : None
* @warning       : None
*/
void BL652_incPingCount(void)
{
    if(gPingCount < DEF_MAX_INTEGER)
    {
        gPingCount++;
    }
}

/*-------------------------- STATIC Functions ----------------------------------*/

/*!
* @brief : This function initialises the BL652 into the appropriate mode
*
* @param[in]     : eBL652mode_t pMode - set BL652 to mode: DISABLE, RUN, DEV, DTM
* @param[out]    : None
* @param[in,out] : None
* @return        : bool lok - 1 = ok, 0 = fail
* @note          : None
* @warning       : Only set mode once after initialisation, as subsequent sets
*                  will reset the BL652 (ensure that is intended) as 652 only reads
*                  mode pin at reset.
*                  WARNING: Once DTM mode is enetered you cannot exit unless you erase filesystem (not implemented)
*/
static uint32_t BL652_mode(eBL652mode_t pMode)
{
    uint32_t lError = 0u;
    //uint8_t  recMsg[DEF_BL652_MAX_REPLY_BUFFER_LENGTH];

    if(pMode < eBL652_MODE_MAX)
    {
        switch(pMode)
        {
        case eBL652_MODE_DISABLE:
        {
            //DEF_BL652_DISABLE()
            //DEF_BL652_DEVMODE()
            gMode = pMode;
        }
        break;

        case eBL652_MODE_ENABLE:
        {
            DEF_BL652_ENABLE()

            // Initialise the UART for Master Mode at 115200 baud
            lError |= UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Master, eUARTn_Baud_115200);
        }
        break;

        case eBL652_MODE_RUN:
        {
            // Just run the application (if not in DTM) otherwise application will not run (use eBL652_MODE_RUN_DTM in this case)

            // Set UART to Slave Mode
            //lError |= UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Slave, eUARTn_Baud_115200);
            UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Slave, eUARTn_Baud_115200);

            DEF_BL652_DISABLE()
            DEF_BL652_RUNMODE()
            DEF_BL652_ENABLE()
            gMode = pMode;
        }
        break;

        case eBL652_MODE_DTM:
        {
            DEF_BL652_DISABLE()
            DEF_BL652_DEVMODE()
            DEF_BL652_ENABLE()

            lError |= BL652_setDTMmode(); // Once in DTM you cannot exit permanently unless you erase filesystem
        }
        break;

        case eBL652_MODE_DEV:
        {
            DEF_BL652_DISABLE()
            DEF_BL652_DEVMODE()
            DEF_BL652_ENABLE()

            lError |= BL652_setATmode(); //Exits DTM if its in DTM and enters AT mode, otherwise just enters AT mode

            if(lError == 0u)
            {
                lError |= BL652_sendAtCmd(eBL652_CMD_Device);
                //lError |= BL652_sendAtCmd( eBL652_CMD_SWVersion);
            }
        }
        break;

        case eBL652_MODE_RUN_DTM: // This mode is for when you have a VSP app and in DTM mode and want to RUN app
        {
            // Note temporary done this way to exit DTM (DTM can only exit permanently if you erase filesystem)
            DEF_BL652_DISABLE()
            DEF_BL652_RUNMODE()
            DEF_BL652_ENABLE()

            BL652_DTM_Test_Exit();

            gMode = pMode;
            lError |= UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Slave, eUARTn_Baud_115200);
        }
        break;

        case eBL652_MODE_RUN_INITIATE_ADVERTISING:
            //lError |= UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Slave, eUARTn_Baud_115200);
            UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Slave, eUARTn_Baud_115200);

#if 0
            DEF_BL652_DISABLE()
            //sleep(100u);
            //DEF_BL652_RUNMODE()
            DEF_BL652_DEVMODE()
            //sleep(100u);
            DEF_BL652_ENABLE()
            //sleep(100u);
#endif
            BL652_sendAtCmd(eBL652_CMD_RUN);

            gMode = pMode;

            break;

        case eBL652_MODE_COMM_INTERFACE_CHECK:
            UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Slave, eUARTn_Baud_115200);

            DEF_BL652_DISABLE()
            DEF_BL652_RUNMODE()
            DEF_BL652_ENABLE()
            lError |= BL652_sendAtCmd(eBL652_CMD_Device);
            break;

        case eBL652_MODE_RUN_DEEP_SLEEP:
            break;

        default:
            lError |= 1u;
            break;
        }
    }

    else
    {
        lError |= 1u;
    }

    if(lError)
    {
        DEF_BL652_DISABLE()
        DEF_BL652_DEVMODE()
        gMode = eBL652_MODE_DISABLE;
    }

    return(lError);
}

/* ELVIS */

/*!
* @brief : This function sets the test setup command (validated)
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t BL652_DTM_Test_Setup(const eBL652dtmTestSetupControl_t pControl, const uint8_t pParameter)
{
    uTestSetupEndFormat_t lDtmTxFrame = { 0 };
    uTestSetupEndFormat_t lDtmRxFrame = { 0 };
    uint32_t lError = 0u;

    if((pControl >= eBL652_DTM_TS_CONTROL_MAX) || (pParameter < sBL652dtmTstSetupParamRng[pControl].parameterMin) || (pParameter > sBL652dtmTstSetupParamRng[pControl].parameterMax))
    {
        lError |= 1u;
    }

    else
    {
        lDtmTxFrame.u32data = 0u;
        lDtmTxFrame.field.cmd = (uint32_t)eBL652_DTM_CMD_TEST_SETUP;
        lDtmTxFrame.field.control = (uint32_t)pControl;
        lDtmTxFrame.field.parameter = (uint32_t)pParameter;
    }

    if(0u == lError)
    {
        lError |= BL652_txRxDtm(&lDtmTxFrame.u32data, &lDtmRxFrame.u32data);
        lError |= BL652_ValidateEvent(eBL652_EVENT_TEST, lDtmRxFrame.u32data);
    }

    return(lError);
}
/* ELVIS */

/*!
* @brief : This function sets the test end command (validated)
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t BL652_DTM_Test_End(const eBL652dtmTestEndControl_t pControl, const uint8_t pParameter, uint16_t *pReportOut)
{
    uTestSetupEndFormat_t lDtmTxFrame = { 0 };
    uTestSetupEndFormat_t lDtmRxFrame = { 0 };
    uint32_t lError = 0u;

    if((pParameter < sBL652dtmTstEndParamRng.parameterMin) || (pParameter > sBL652dtmTstEndParamRng.parameterMax) || (pControl != eBL652_DTM_TE_CONTROL_CMD) || (pReportOut == NULL))
    {
        lError |= 1u;
    }

    else
    {
        lDtmTxFrame.u32data = 0u;
        lDtmTxFrame.field.cmd = (uint32_t)eBL652_DTM_CMD_TEST_END;
        lDtmTxFrame.field.control = (uint32_t)pControl;
        lDtmTxFrame.field.parameter = (uint32_t)pParameter;
    }

    if((0u == lError) && (pReportOut != NULL))
    {
        lError |= BL652_txRxDtm(&lDtmTxFrame.u32data, &lDtmRxFrame.u32data);
        lError |= BL652_ValidateEvent(eBL652_EVENT_PACKET, lDtmRxFrame.u32data);

        if(0u == lError)
        {
            *pReportOut = (uint16_t)lDtmRxFrame.u32data;  // only lower 16bit hold data
        }

        else
        {
            *pReportOut = 0xFFFFu;
        }
    }

    return(lError);
}

/*!
* @brief : This function sends the TX/RX test command (validated)
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t BL652_DTM_Test_TxRx(const eBL652dtmCmd_t pCmd, const int16_t pFrequency, const uint8_t pLength, const eBL652dtmTxRxTestPkt_t pPktType)
{
    uTestTxRxFormat_t lDtmTxFrame = { 0 };
    uTestTxRxFormat_t lDtmRxFrame = { 0 };
    uint32_t lError = 0u;

#pragma diag_suppress=Pm136, Pm128 /* Disable MISRA C 2004 rule 10.3, 10.1 */
    //Range checked so allow misra rule
    uint16_t  pFreqIdx = ((pFrequency - eBL652_DTM_BASE_FREQUENCY) / 2);        // Find freq offset
#pragma diag_default=Pm136 /* Disable MISRA C 2004 rule 10.3, 10.1 */

    if(pPktType == eBL652_DTM_TXRX_PKT_VDRSPEC)
    {
        if(pLength == (uint8_t)eBL652_DTM_VDRSPEC_TXPOWER)
        {
            if(((pFrequency < DEF_TXTEST_VDR_MIN_TXPOWER) || (pFrequency > DEF_TXTEST_VDR_MAX_TXPOWER)))
            {
                lError |= 1u;
            }

            else
            {
#pragma diag_suppress=Pm128, Pm136 /* Disable MISRA C 2004 rule 10.1, 10.3 */
                // Range checked so allow misra rule
                pFreqIdx = pFrequency; // Equate TXP value
#pragma diag_default=Pm128, Pm136 /* Disable MISRA C 2004 rule 10.1, 10.3 */
            }
        }

        else if(pLength == (uint8_t)eBL652_DTM_VDRSPEC_CC)
        {
            if((pFreqIdx < sBL652dtmTxRxFreqParamRng.parameterMin) || (pFreqIdx > sBL652dtmTxRxFreqParamRng.parameterMax) || ((pFrequency % 2) > 0))
            {
                lError |= 1u;
            }
        }

        else
        {
            lError |= 1u;
        }
    }

    else
    {
        if((pFreqIdx < sBL652dtmTxRxFreqParamRng.parameterMin) || (pFreqIdx > sBL652dtmTxRxFreqParamRng.parameterMax) || ((pFrequency % 2) > 0)
                || ((pLength < sBL652dtmTxRxLenParamRng.parameterMin) || (pLength > sBL652dtmTxRxLenParamRng.parameterMax))
                || ((pPktType < sBL652dtmTxRxPktParamRng.parameterMin) || (pPktType > sBL652dtmTxRxPktParamRng.parameterMax))
                || ((pCmd != eBL652_DTM_CMD_RX_TEST) && (pCmd != eBL652_DTM_CMD_TX_TEST)))
        {
            lError |= 1u;
        }
    }

    if(0u == lError)
    {
        lDtmTxFrame.u32data = 0u;
        lDtmTxFrame.field.cmd = (uint32_t)pCmd;
        lDtmTxFrame.field.frequency = (uint32_t)pFreqIdx;
        lDtmTxFrame.field.length = (uint32_t)pLength;
        lDtmTxFrame.field.pkt = (uint32_t)pPktType;

        lError |= BL652_txRxDtm(&lDtmTxFrame.u32data, &lDtmRxFrame.u32data);
        lError |= BL652_ValidateEvent(eBL652_EVENT_TEST, lDtmRxFrame.u32data);
    }

    return(lError);
}

/*!
* @brief : This function send the test exit command (validated)
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t BL652_DTM_Test_Exit(void)
{
    uint32_t lError = 0u;
    uTestSetupEndFormat_t lDtmTxFrame = { 0 };
    uTestSetupEndFormat_t lDtmRxFrame = { 0 };

    lDtmTxFrame.u32data = 0u;
    lDtmTxFrame.field.cmd = 0x00u;
    lDtmTxFrame.field.control = 0x3fu;
    lDtmTxFrame.field.parameter = 0xffu;

    if(0u == lError)
    {
        lError |= BL652_txRxDtm(&lDtmTxFrame.u32data, &lDtmRxFrame.u32data);
    }

    return(lError);
}

/*!
* @brief : This function validates the received event with the required event to ensure frame is correct
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t BL652_ValidateEvent(const eBLE652Event_t pExpectedEvent, const uint32_t pActualEvent)
{
    uint32_t lError = 0u;
    uTestStatusEvent_t luStatusEv;

    luStatusEv.u32data = pActualEvent;    // Equate the EV bit as its the same position for poth event types

    if((eBL652_EVENT_TEST == pExpectedEvent)                  // If we expect a test event &
            && (eBL652_EVENT_TEST == luStatusEv.field.ev))            // If its a test event packet
    {
        if(luStatusEv.field.st)                               // If its an error
        {
            lError |= 1u;
        }
    }

    else if((eBL652_EVENT_PACKET == pExpectedEvent)           // If we expect a test event packet
            && ((uint32_t)eBL652_EVENT_PACKET == luStatusEv.field.ev))          // We should get it otherwise its an error
    {
        // No Error - Do nothing as packet event has no other information but count
    }
    else
    {
        lError |= 1u;
    }

    return(lError);
}

/*!
* @brief : This function transmits the DTM frame in pSdata, and places the received response in pRdata
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t BL652_txRxDtm(uint32_t *pSdata, uint32_t *pRdata)
{
    uint32_t lError = 0u;
    uint8_t  lTxData[2];
    uint8_t *ldataPtr;
    uint16_t numofBytesReceived = 0u;

    if((pSdata == NULL) || (pRdata == NULL))
    {
        lError |= 1u;  // Exit and set error
    }

    else
    {
        //MSB first for DTM
        ldataPtr = (uint8_t *)pSdata;
        lTxData[0] = ldataPtr[1];
        lTxData[1] = ldataPtr[0];

        if(false == sendOverUSART1(lTxData, sizeof(lTxData)))
        {
            lError |= 1u;
        }

        else
        {
            //testArray[testArrayIndex++] = *pSdata;
        }

        if(false == waitToReceiveOverUsart1(WAIT_TILL_END_OF_FRAME_RECEIVED, 150u))
        {
            lError |= 1u;
        }

        else
        {
            if((*pSdata) != DEF_BL652_DTM_EXIT_CMD)
            {
                getAvailableUARTxReceivedByteCount(UART_PORT1,
                                                   (uint16_t *) &numofBytesReceived);

                if(2u == (uint32_t)numofBytesReceived)
                {
                    uint8_t *ptr = NULL;
                    getHandleToUARTxRcvBuffer(UART_PORT1, (uint8_t **)&ptr);

                    ldataPtr = (uint8_t *)pRdata;
                    ldataPtr[1] = ptr[0];
                    ldataPtr[0] = ptr[1];

                    //testArray[testArrayIndex++] = *pRdata;
                }

                else
                {
                    lError |= 1u;
                }
            }
        }

        if(false == ClearUARTxRcvBuffer(UART_PORT1))
        {
            lError |= 1u;
        }

        DEF_DELAY_TX_10ms;
    }

    return(lError);
}

/* ELVIS */
/*!
* @brief : This function validates the passed string and length against the passed command
*
* @param[in]     :
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t BL652_vfyReply(const eBLE652commands_t pAtCmd, uint8_t *pStr)
{
    uint32_t lError = 0u;
    uint32_t lindex = 0u;


    if((pAtCmd < eBL652_CMD_MAX) && (pStr != NULL))
    {
        while((pStr[lindex] != '\r') && (lError == 0u))
        {
            if(sBLE652atCommand[pAtCmd].cmdReply[lindex] == pStr[lindex])
            {
                //ok
            }
            else
            {

                if(sBLE652atCommand[pAtCmd].cmdReply[lindex] == '#')
                {
                    if((pStr[lindex] > DEF_BL652_ASCII_NUMBER_MIN) && (pStr[lindex] < DEF_BL652_ASCII_NUMBER_MAX))
                    {
                        //ok we have a valid range
                    }
                    else
                    {
                        lError |= 1u;
                    }
                }

                else if(sBLE652atCommand[pAtCmd].cmdReply[lindex] == '@')
                {
                    if(((pStr[lindex] > DEF_BL652_ASCII_LETTER_MIN) && (pStr[lindex] < DEF_BL652_ASCII_LETTER_MAX)))
                    {
                        //ok we have a valid range
                    }
                    else
                    {
                        lError |= 1u;
                    }
                }

                else if(sBLE652atCommand[pAtCmd].cmdReply[lindex] == '&')
                {
                    if(((pStr[lindex] > DEF_BL652_ASCII_LETTER_MIN) && (pStr[lindex] < DEF_BL652_ASCII_LETTER_MAX))
                            || ((pStr[lindex] > DEF_BL652_ASCII_NUMBER_MIN) && (pStr[lindex] < DEF_BL652_ASCII_NUMBER_MAX)))
                    {
                        //ok we have a valid range
                    }
                    else
                    {
                        lError |= 1u;
                    }
                }

                else if(sBLE652atCommand[pAtCmd].cmdReply[lindex] == '.')
                {
                    if(pStr[lindex] != '.')
                    {
                        lError |= 1u;
                    }
                }

                else if(sBLE652atCommand[pAtCmd].cmdReply[lindex] == '\t')
                {
                    if(pStr[lindex] != '\t')
                    {
                        lError |= 1u;
                    }
                }

                else
                {
                    lError |= 1u;
                }
            }

            lindex++;

            if(lindex < DEF_BL652_MAX_REPLY_BUFFER_LENGTH)
            {
                // Do nothing as we are within the buffer
            }
            else
            {
                lError |= 1u;
            }
        }
    }

    else
    {
        lError |= 1u;
    }

    return(lError);
}

/*!
* @brief : This function sets the BL652 into DTM mode tries 4 times to change to
* DTM mode by initially checking if in AT then switching to DTM and checking if its
* in DTM mode.
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t BL652_setDTMmode(void)
{
    uint32_t lError = 0u;
    uint32_t lRetry = 0u;
    uint32_t lRetries = 0u;

    do
    {
        lRetry = 0u;
        BL652_sendAT_Null();
        lError = BL652_sendAT_Null();

        if(lError)
        {
            BL652_sendDTM_Null();
            lError = BL652_sendDTM_Null();

            if(lError)
            {
                lRetry = 1u;
            }
        }

        else
        {
            BL652_sendAT_Dtm();
            lError = BL652_sendAT_Dtm();

            if(lError)
            {
                lRetry = 1u;
            }

            else
            {
                BL652_sendDTM_Null();
                lError = BL652_sendDTM_Null();

                if(lError)
                {
                    lRetry = 1u;
                }
            }
        }

        lRetries++;
    }
    while((lRetry) && ((lRetries) < 3u));

    if(0u == lError)
    {
        gMode = eBL652_MODE_DTM;
    }

    return(lError);
}

/*!
* @brief : This function sets the BL652 into AT mode tries 4 times to change to
* AT mode by initially checking if in DTM then switching to AT and checking if its
* in AT mode.
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t BL652_setATmode(void)
{
    uint32_t lError = 0u;
    uint32_t lRetry = 0u;
    uint32_t lRetries = 0u;

    do
    {
        lRetry = 0u;
        BL652_sendDTM_Null();
        lError = BL652_sendDTM_Null();

        if(lError)
        {
            BL652_sendAT_Null();
            lError = BL652_sendAT_Null();

            if(lError)
            {
                lRetry = 1u;
            }
        }

        else
        {
            lError = BL652_DTM_Test_Exit();

            if(lError)
            {
                lRetry = 1u;
            }

            else
            {
                BL652_sendAT_Null();
                lError = BL652_sendAT_Null();

                if(lError)
                {
                    lRetry = 1u;
                }
            }
        }

        lRetries++;
    }
    while((lRetry) && (lRetries < DEF_RETIRES_U32));

    if(0u == lError)
    {
        gMode = eBL652_MODE_DEV;
    }

    return(lError);
}

/*!
* @brief : This function sets the BL652 enter dtm frame
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : Ensure frame is validated prior to calling this function
*/
static uint32_t BL652_setDTMframe(uint8_t *pMsg)
{
    uint32_t lError = 0u;

    if(pMsg != NULL)
    {
        dtmATmsg[9]  = pMsg[10];
        dtmATmsg[10] = pMsg[11];
        dtmATmsg[11] = pMsg[12];
        dtmATmsg[12] = pMsg[13];
        dtmATmsg[13] = pMsg[18];
        dtmATmsg[14] = pMsg[19];
        dtmATmsg[15] = pMsg[20];
        dtmATmsg[16] = pMsg[21];
    }

    else
    {
        lError |= 1u;
    }

    return(lError);
}

/*!
* @brief : This function sends AT null message
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : Sets Terminator and keeps it selected
*/
static uint32_t BL652_sendAT_Null(void)
{
    uint32_t lError = 0u;
    // uint8_t  recMsg[DEF_BL652_MAX_REPLY_BUFFER_LENGTH];

    lError |= UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Master, eUARTn_Baud_115200);
    lError |= BL652_sendAtCmd(eBL652_CMD_NULL);

    return(lError);
}

/*!
* @brief : This function sends AT DTM message
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : Sets Terminator and keeps it selected
*/
static uint32_t BL652_sendAT_Dtm(void)
{
    uint32_t lError = 0u;
    // uint8_t recMsg[DEF_BL652_MAX_REPLY_BUFFER_LENGTH];

    lError |= UARTn_TermType(&huart1, eUARTn_Term_CR, eUARTn_Type_Master, eUARTn_Baud_115200);
    lError |= BL652_sendAtCmd(eBL652_CMD_MACaddress);
    lError |= BL652_setDTMframe(recMsg);

    if(0u == lError)
    {
        lError |= BL652_sendAtCmd(eBL652_CMD_DTM);
    }

    return(lError);
}

/*!
* @brief : This function sends DTM null (reset) message
*
* @param[in]     : None
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : Sets Terminator and keeps it selected
*/
static uint32_t BL652_sendDTM_Null(void)
{
    uint32_t lError = 0u;

    lError |= UARTn_TermType(&huart1, eUARTn_Term_None, eUARTn_Type_Master, eUARTn_Baud_19200);

    if(0u == lError)
    {
        lError |= BL652_DTM_Test_Setup(eBL652_DTM_TS_CONTROL_RESET, (uint8_t)eBL652_DTM_SETUP_PARAM_RESET);
    }

    return(lError);
}

//*!
//* @brief : This function sends an AT command (as defined) and validates the response
//* and places the validated string into the destination pointer location.  An invalid
//* parse will results in an error being returned.
//*
//* @param[in]     : eBLE652commands_t pAtCmd - list of defined commands
//*                : uint8_t* pValidatedStr - destination of validated string
//* @param[out]    : None
//* @param[in,out] : None
//* @return        : uint32_t lError - 1 = fail, 0 = ok
//* @note          : None
//* @warning       : None
//*/
//static uint32_t BL652_sendAtCmd( eBLE652commands_t pAtCmd )
//{
//    uint32_t lError = 0u;
//    uint8_t  lRecStr[DEF_BL652_MAX_CMD_BUFFER_LENGTH];
//
//    if(( pAtCmd < eBL652_CMD_MAX ) && ( pValidatedStr != NULL ))
//    {
//        lError |= BL652_txRxAt( pAtCmd );
//    }
//
//    return( lError );
//}

/*!
* @brief : This function send command to smart basic App to start advertising
*
* @param[in]     : device serail numebr
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
uint32_t BL652_startAdvertising(uint8_t *serailNo)
{
    uint32_t lError = 0u;
    uint16_t numofBytesReceived = 0u;
    memset_s(deviceSerialNumber, sizeof(deviceSerialNumber), 0,  sizeof(deviceSerialNumber));
    memcpy_s(deviceSerialNumber,  sizeof(deviceSerialNumber), serailNo, 10u);

    memcpy_s(&sbaCmdStartAdvertising[SERIAL_NUMBER_START_INDEX_IN_SBA_COMMAND],
             (size_t)(DEVICE_SERIAL_NUMBER_LENGTH - SERIAL_NUMBER_START_INDEX_IN_SBA_COMMAND),
             &deviceSerialNumber[FOR_ADVERTISEMENT_SERIAL_NUMBER_START_INDEX],
             (size_t)6);

    // Only for test added by mak
    sbaCmdStartAdvertising[12] = 0x0Au;

    if(false == sendOverUSART1(sbaCmdStartAdvertising, (uint32_t)strlen((char const *)sbaCmdStartAdvertising)))
    {
        lError |= 1u;
    }

    if(false == waitToReceiveOverUsart1(WAIT_TILL_END_OF_FRAME_RECEIVED, 250u))
    {
        lError |= 1u;
        recMsg[sizeof(lError)] = '\0';
        memcpy_s(recMsg, DEF_BL652_MAX_REPLY_BUFFER_LENGTH, (uint8_t *)&lError, (uint32_t)sizeof(lError));
    }

    else
    {
        getAvailableUARTxReceivedByteCount(UART_PORT1,
                                           (uint16_t *) &numofBytesReceived);

        if(numofBytesReceived >= (uint16_t)OK_RESPONSE_LENGTH)
        {
            uint8_t *replyPtr = NULL;
            getHandleToUARTxRcvBuffer(UART_PORT1, (uint8_t **)&replyPtr);

            if(!memcmp(okResponse, replyPtr, (size_t)OK_RESPONSE_LENGTH))
            {
                lError = 0u;
            }

            else
            {
                lError |= 1u;
            }
        }

        else
        {
            lError |= 1u;
        }
    }

    DEF_DELAY_TX_10ms;

    return(lError);
}
