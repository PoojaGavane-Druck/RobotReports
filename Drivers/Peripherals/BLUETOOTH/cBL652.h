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
* @file     cBL652.h
*
* @author   Elvis Esa
* @date     Nov 2020
*
* __CBL652_H header file
*/

#ifndef __CBL652_H
#define __CBL652_H

#ifdef __cplusplus
extern "C" {
#endif


/* Private defines -----------------------------------------------------------*/

#define DEF_BL652_MAX_CMD_BUFFER_LENGTH      (( uint32_t )16u )
#define DEF_BL652_MAX_REPLY_BUFFER_LENGTH    (( uint32_t )100u )


/* Typedefs ------------------------------------------------------------------*/

typedef enum
{
    eBL652_MODE_DISABLE = 0,
    eBL652_MODE_RUN = 1,
    eBL652_MODE_DTM = 2,
    eBL652_MODE_DEV = 3,
    eBL652_MODE_RUN_DTM = 4,
    eBL652_MODE_RUN_INITIATE_ADVERTISING = 5,
    eBL652_MODE_RUN_DEEP_SLEEP = 6,
    eBL652_MODE_TESTING = 7,
    eBL652_MODE_ENABLE = 8,
    eBL652_MODE_MAX = 9,
    eBL652_MODE_END = 0xFFFFFFFFu
} eBL652mode_t;

/*****************************************************************************/
/* WARNING do not change or reorder the typedef below (commands to BL652)    */
/*****************************************************************************/
typedef enum { eBL652_DTM_CMD_TEST_SETUP = 0, eBL652_DTM_CMD_RX_TEST = 1, eBL652_DTM_CMD_TX_TEST = 2, eBL652_DTM_CMD_TEST_END = 3,
               eBL652_DTM_CMD_MAX = 4, eBL652_DTM_CMD_END = 0xFFFFFFFFu
             } eBL652dtmCmd_t;

typedef enum { eBL652_DTM_TS_CONTROL_RESET = 0, eBL652_DTM_TS_CONTROL_UPPERLENGTH = 1, eBL652_DTM_TS_CONTROL_PHY = 2, eBL652_DTM_TS_CONTROL_MODINDEX = 3,
               eBL652_DTM_TS_CONTROL_FEATURES = 4, eBL652_DTM_TS_CONTROL_OCTETS = 5, eBL652_DTM_TS_CONTROL_TONE = 6, eBL652_DTM_TS_CONTROL_TONESAMPLE = 7,
               eBL652_DTM_TS_CONTROL_ANTENNA = 8, eBL652_DTM_TS_CONTROL_MAX = 9,
               eBL652_DTM_TS_CONTROL_END = 0xFFFFFFFFu
             } eBL652dtmTestSetupControl_t;

typedef enum { eBL652_DTM_TE_CONTROL_CMD = 0, eBL652_DTM_TE_CONTROL_MAX = 4, eBL652_DTM_TE_CONTROL_END = 0xFFFFFFFFu } eBL652dtmTestEndControl_t;

typedef enum { eBL652_DTM_TXRX_PKT_PRBS9 = 0, eBL652_DTM_TXRX_PKT_HL = 1, eBL652_DTM_TXRX_PKT_ALT = 2, eBL652_DTM_TXRX_PKT_VDRSPEC = 3,
               eBL652_DTM_TXRX_PKT_MAX = 4, eBL652_DTM_TXRX_PKT_END = 0xFFFFFFFFu
             } eBL652dtmTxRxTestPkt_t;

typedef enum { eBL652_DTM_VDRSPEC_CC = 0, eBL652_DTM_VDRSPEC_TXPOWER = 2, eBL652_DTM_VDRSPEC_END = 3, eBL652_DTM_VDRSPEC_MAX = 0xFFFFFFFFu } eBL652dtmVdrSpecCmd_t;

typedef enum { eBL652_EVENT_TEST = 0, eBL652_EVENT_PACKET = 1, eBL652_EVENT_MAX = 2, eBL652_EVENT_END = 0xFFFFFFFFu } eBLE652Event_t;

/*****************************************************************************/

/*****************************************************************************/
/* WARNING do not change order of the typedef below as its an index to array */
/*****************************************************************************/
typedef enum { eBL652_CMD_Device = 0, eBL652_CMD_SWVersion = 1, eBL652_CMD_MACaddress = 2, eBL652_CMD_NULL = 3, eBL652_CMD_DTM = 4, eBL652_CMD_DIR = 5, eBL652_CMD_RUN = 6, eBL652_CMD_FS_CLEAR = 7, eBL652_CMD_MAX = 8, eBL652_CMD_END = 0xFFFFFFFFu } eBLE652commands_t;
/*****************************************************************************/

typedef union uTestSetupEndFormat
{
#pragma bitfields=disjoint_types
    struct sTestSetupEndFormat_t
    {
        uint32_t parameter : 8;
        uint32_t control   : 6;
        uint32_t cmd       : 2;
        uint32_t reserved  : 16;
    } field;
#pragma bitfields=default
    uint32_t u32data;
} uTestSetupEndFormat_t;

typedef union uTestTxRxFormat
{
#pragma bitfields=disjoint_types
    struct suTestTxRxFormat
    {
        uint32_t pkt       : 2;
        uint32_t length    : 6;
        uint32_t frequency : 6;
        uint32_t cmd       : 2;
        uint32_t reserved  : 16;
    } field;
#pragma bitfields=default
    uint32_t u32data;
} uTestTxRxFormat_t;

typedef union uTestStatusEvent
{
#pragma bitfields=disjoint_types
    struct sTestStatusEvent
    {
        uint32_t st        : 1;
        uint32_t response  : 14;
        uint32_t ev        : 1;
        uint32_t reserved  : 16;
    } field;
#pragma bitfields=default
    uint32_t u32data;
} uTestStatusEvent_t;

typedef union uPacketReportEvent
{
#pragma bitfields=disjoint_types
    struct sPacketReportEvent
    {
        uint32_t packetCnt : 15;
        uint32_t ev        : 1;
        uint32_t reserved  : 16;
    } field;
#pragma bitfields=default
    uint32_t u32data;
} uPacketReportEvent_t;

typedef struct
{
    uint8_t bl652_rd_device[DEF_BL652_MAX_CMD_BUFFER_LENGTH];
    uint8_t bl652_rd_mac[DEF_BL652_MAX_CMD_BUFFER_LENGTH];
    uint8_t bl652_rd_version[DEF_BL652_MAX_CMD_BUFFER_LENGTH];
} sBLE652param_t;

typedef struct
{
    uint8_t *cmdSend;
    uint8_t cmdSendLength;
    uint8_t *cmdReply;
    uint8_t cmdReplyLength;
} sBLE652commands_t;

typedef struct
{
    uint8_t parameterMin;
    uint8_t parameterMax;
    uint32_t msb;
} sBLE652dtmParameterRanges_t;



/* Private includes ----------------------------------------------------------*/

/* None */

/* Private defines -----------------------------------------------------------*/

//#define DEF_BL652_DTM_SETUP_MAX                 (( uint16_t )10u )
//#define eBL652_DTM_SETUP_PARAM_PHY_LE_1M        (( uint16_t )0x04u )
//#define eBL652_DTM_SETUP_PARAM_PHY_LE_2M        (( uint16_t )0x08u )
//#define eBL652_DTM_SETUP_PARAM_PHY_LE_CODED_S8  (( uint16_t )0x0Cu )
//#define eBL652_DTM_SETUP_PARAM_PHY_LE_CODED_S2  (( uint16_t )0x10u )
//#define eBL652_DTM_SETUP_PARAM_REC_TX_STABLE    (( uint16_t )0x04u )
//#define eBL652_DTM_SETUP_PARAM_MAX_TX_POWER     (( uint16_t )0x7Fu )
//#define eBL652_DTM_SETUP_PARAM_MIN_TX_POWER     (( uint16_t )0x7Eu )

/* Exported types ------------------------------------------------------------*/

/* None */

/* Exported constants --------------------------------------------------------*/

/* None */

/* Exported macro ------------------------------------------------------------*/

/* None */

/* Exported functions prototypes ---------------------------------------------*/

extern int32_t BL652_getReport(void);
extern void BL652_incPingCount(void);
extern int32_t BL652_getPingCount(void);
extern uint8_t *BL652_getResponse(void);
extern uint32_t BL652_sendAtCmd(const eBLE652commands_t pAtCmd);
extern void BL652_setPingCount(const int32_t value);
extern bool BL652_dtmEndTest(uint16_t *pReportOut);
extern bool BL652_initialise(const eBL652mode_t pMode);
extern bool BL652_dtmRXtest(const int16_t pFreq, const uint8_t pPhy);
extern bool BL652_dtmTXtest(const int16_t pFreq, const uint8_t pPhy, const uint8_t pPktType, const uint8_t pPktLen, const int8_t pTxPower);
extern void BL652_setAdvertName(uint8_t *serialNum);
extern uint32_t BL652_startAdvertising(uint8_t *serailNo);
/* ---------------------------------------------------------------------------*/
/*
The BLE 2-wire UART DTM interface standard reserves Packet Type (payload parameter) binary value '11' for a Vendor Specific packet payload. The DTM to Serial adaptation layer maps this to value 0xFFF..FFF in the dtm_cmd interface. The rationale for this mapping is to allow later extensions to a 4-bit Packet Type field, as specified in the HCI interface and in the DTM PDU layout.

The Vendor Specific payload (parameter 4) is interpreted as follows, if parameter 1 (command) is set to Transmitter Test (binary '10') and parameter 4 (payload) to Vendor Specific (binary '11' in the 2wire physical interface, all bits set to 1 in the dtmlib interface):

If parameter 3 (length) is set to 0 (symbol CARRIER_TEST), an unmodulated carrier is turned on at the channel indicated by parameter 2 (freq). It remains turned on until a TEST_END or RESET command is issued.
Parameter 3 (length) equal to 1 (symbol CARRIER_TEST_STUDIO) is used by the nRFgo studio to indicate that an unmodulated carrier is turned on at the channel. It remains turned on until a TEST_END or RESET command is issued.
If parameter 3 (length) is set to 2 (symbol SET_TX_POWER), parameter 2 (freq) sets the TX power in dBm. The valid TX powers are specified in the product specification ranging from -40 to +4. 0 dBm is the reset value. Only the 6 least significant bits will fit in the length field, so the two most significant bits are calculated by the DTM module (which is possible because the 6 least significant bits of all valid TX powers are unique). The TX power can be modified only when no Transmitter Test or Receiver Test is running.
If parameter 3 (length) is set to 3 (symbol SELECT_TIMER), parameter 2 (freq) selects the timer to be used by the DTM - Direct Test Mode library for Transmitter Test timing. Valid timer identifiers are 0, 1, and 2. Configuring the timer to be used allows the library to be integrated in a larger test environment where other modules may be occupying the default timer (timer 0).
If parameter 3 (length) is set to 4 (symbol SET_NRF21540_TX_POWER), parameter 2 (freq) selects one of two the predefined nRF21540 power mode. Valid power modes are 1(+20dBm) and 2(+10dBm). This command should be used only when testing with nRF21540.
All other values of parameter 2 (freq) and 4 (length) are reserved.
*/

#ifdef __cplusplus
}
#endif

#endif /* __CBL652_H */