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
* @file     uart.c
* @version  1.0
* @author   Julio Andrade
* @date     Jul 2, 2018
* @brief
*/

#include "misra.h"
MISRAC_DISABLE
#include <stdio.h>
#include <stm32l4xx_hal.h>
#include <ctype.h>
#include <cpu.h>
#include <lib_mem.h>
#include <os.h>
//#include <bsp_clk.h>
#include <bsp_os.h>
#include <bsp_int.h>
#include <os_app_hooks.h>
#include <stdbool.h>
#include <string.h>
#include <DLib_Product_string.h>

MISRAC_ENABLE

#include "uart.h"
#include "main.h"

#define MAX_SEM_NAME_SIZE 50

volatile uint32_t intRcvFlag = (uint32_t)(0);

static char uartRcvSemNames[MAX_NUM_OF_UART_PORTS][MAX_SEM_NAME_SIZE] =
                                                            { "Usart1RCV",
                                                              "Usart2RCV",
                                                              "Usart3RCV",
                                                              "Uart4RCV",
                                                              "Uart5RCV",
                                                            };

static  char uartSendSemNames[MAX_NUM_OF_UART_PORTS][MAX_SEM_NAME_SIZE] =
                                                          { "Usart1Send",
                                                            "Usart2Send",
                                                            "Usart3Send",
                                                            "Uart4Send",
                                                            "Uart5Send",
                                                          };
static const uint16_t receiveBufferSize[MAX_NUM_OF_UART_PORTS] =
                                                         {
                                                          USART1_RX_BUFF_SIZE,
                                                          USART2_RX_BUFF_SIZE,
                                                          USART3_RX_BUFF_SIZE,
                                                          UART4_RX_BUFF_SIZE,
                                                          UART5_RX_BUFF_SIZE
                                                         };

static uint8_t usart1RxBuffer[USART1_RX_BUFF_SIZE];
static uint8_t usart2RxBuffer[USART2_RX_BUFF_SIZE];
static uint8_t usart3RxBuffer[USART3_RX_BUFF_SIZE];
static uint8_t uart4RxBuffer[UART4_RX_BUFF_SIZE];
static uint8_t uart5RxBuffer[UART4_RX_BUFF_SIZE];

static bool rxReady[MAX_NUM_OF_UART_PORTS] = {true, true, true, true, true};
static eUartnTerm_t termType[MAX_NUM_OF_UART_PORTS] = {eUARTn_Term_None,
                                                       eUARTn_Term_None,
                                                       eUARTn_Term_None,
                                                       eUARTn_Term_None,
                                                       eUARTn_Term_None};
static eUartnType_t commType[MAX_NUM_OF_UART_PORTS] = {
                                                        eUARTn_Type_Master,
                                                        eUARTn_Type_Master,
                                                        eUARTn_Type_Slave,
                                                        eUARTn_Type_Slave,
                                                        eUARTn_Type_Master
                                                      };

static OS_SEM uartSemSend[MAX_NUM_OF_UART_PORTS];
static OS_SEM uartSemRcv[MAX_NUM_OF_UART_PORTS];
static OS_ERR p_err[MAX_NUM_OF_UART_PORTS];

static uint16_t expectedNumOfBytes[MAX_NUM_OF_UART_PORTS];

static UART_HandleTypeDef *UartHandle[MAX_NUM_OF_UART_PORTS];
static bool UART_IsRX(UART_HandleTypeDef *uhandle);

bool setExpectedNumOfBytes(PortNumber_t portNumber,
                           uint16_t expectedBytesCount);


 
bool enableSerialPortTxLine(PortNumber_t portNumber)
{
    bool retStatus = true;
#ifndef NUCLEO_BOARD
     switch(portNumber)
     {
        case UART_PORT1:
           HAL_GPIO_WritePin(BT_ENABLE_PB9_GPIO_Port , BT_ENABLE_PB9_Pin, GPIO_PIN_SET);
          break;
       
        case UART_PORT2:
           HAL_GPIO_WritePin(PM620_IF_UART2_RXEN_PA4_GPIO_Port, PM620_IF_UART2_RXEN_PA4_Pin ,  GPIO_PIN_SET);
          break;
          
        case UART_PORT3:
          
          break;    
          
        case UART_PORT4:  
          HAL_GPIO_WritePin(DPI620G_IF_RXEN_PD5_GPIO_Port , DPI620G_IF_RXEN_PD5_Pin, GPIO_PIN_SET);
          break;
          
        case UART_PORT5:
          
          break;  
         
        default:
          retStatus = false;
         break;
     }     
#else   
     switch(portNumber)
     {
        case UART_PORT1:
        case UART_PORT4:  
          break;
        case UART_PORT2:
           HAL_GPIO_WritePin(USART2_PM620_TX_ENABLE_GPIO_Port , USART2_PM620_TX_ENABLE_PIN, GPIO_PIN_SET);
          break;
        case UART_PORT3:
          HAL_GPIO_WritePin(USART3_DPI620G_TX_ENABLE_GPIO_Port , USART3_DPI620G_TX_ENABLE_PIN, GPIO_PIN_SET);
          break;     
        case UART_PORT5:
          HAL_GPIO_WritePin(USART5_RS485_TX_ENABLE_GPIO_Port , USART5_RS485_TX_ENABLE_PIN, GPIO_PIN_SET);
          break;  
         
        default:
          retStatus = false;
         break;
     }
#endif
   return retStatus;
}

bool disableSerialPortTxLine(PortNumber_t portNumber)
{
     bool retStatus = true;

#ifndef NUCLEO_BOARD
     switch(portNumber)
     {
        case UART_PORT1:
           HAL_GPIO_WritePin(BT_ENABLE_PB9_GPIO_Port , BT_ENABLE_PB9_Pin, GPIO_PIN_RESET);
          break;
       
        case UART_PORT2:
           HAL_GPIO_WritePin(PM620_IF_UART2_RXEN_PA4_GPIO_Port, PM620_IF_UART2_RXEN_PA4_Pin ,  GPIO_PIN_RESET);
          break;
          
        case UART_PORT3:
          
          break;    
          
        case UART_PORT4:  
          HAL_GPIO_WritePin(DPI620G_IF_RXEN_PD5_GPIO_Port , DPI620G_IF_RXEN_PD5_Pin, GPIO_PIN_RESET);
          break;
          
        case UART_PORT5:
          
          break;  
         
        default:
          retStatus = false;
         break;
     }     
#else
     switch(portNumber)
     {
        case UART_PORT1:
        case UART_PORT4:  
          break;
        case UART_PORT2:
           HAL_GPIO_WritePin(USART2_PM620_TX_ENABLE_GPIO_Port , USART2_PM620_TX_ENABLE_PIN, GPIO_PIN_RESET);
          break;
        case UART_PORT3:
          HAL_GPIO_WritePin(USART3_DPI620G_TX_ENABLE_GPIO_Port , USART3_DPI620G_TX_ENABLE_PIN, GPIO_PIN_RESET);
          break;     
        case UART_PORT5:
          HAL_GPIO_WritePin(USART5_RS485_TX_ENABLE_GPIO_Port , USART5_RS485_TX_ENABLE_PIN, GPIO_PIN_RESET);
          break;  
         
        default:
          retStatus = false;
         break;
     }
#endif
     return retStatus;
}

bool setExpectedNumOfBytes(PortNumber_t portNumber,
                           uint16_t expectedBytesCount)
{
    bool retStatus = false;
    if(portNumber >= MAX_NUM_OF_UART_PORTS)
    {
      retStatus = false;
    }
    else
    {
       expectedNumOfBytes[portNumber] = expectedBytesCount;
       retStatus = true;
    }
                                                                   
    return retStatus;
}


static PortNumber_t getUartPortNumber( UART_HandleTypeDef *huart )
{
    PortNumber_t portNumber = UART_INVALID_PORTNUMBER;

    // This was ex 705 code so disabled misra - MISRA 2004 Rule 11.3
    MISRAC_DISABLE
    if ( huart->Instance == USART1 )
    {
        portNumber = UART_PORT1;
    }
    else if ( huart->Instance == USART2 )
    {
        portNumber = UART_PORT2;
    }
    else if ( huart->Instance == USART3 )
    {
        portNumber = UART_PORT3;
    }
    else if ( huart->Instance == UART4 )
    {
        portNumber = UART_PORT4;
    }
    else if ( huart->Instance == UART5 )
    {
        portNumber = UART_PORT5;
    }
    else
    {
        portNumber = UART_INVALID_PORTNUMBER;
    }
    MISRAC_ENABLE

    return( portNumber );
}

/**
 * @brief this function inits the UART driver
 */
//bool uartInit(USART_ConfigParams configParams)
bool uartInit(UART_HandleTypeDef *huart)
{
    bool bError = false;
    PortNumber_t portNumber = UART_INVALID_PORTNUMBER;
    bool isConfigValid = false;
   
    
    portNumber = getUartPortNumber(huart);
    if(UART_INVALID_PORTNUMBER != portNumber)
    {
      isConfigValid = true;
    }
    
    if(true ==  isConfigValid)
    {
      UartHandle[portNumber] = huart;
      if(false == bError)
      {
        OSSemCreate(&uartSemRcv[portNumber],
                  uartRcvSemNames[portNumber],  
                  (OS_SEM_CTR)0, 
                  &p_err[portNumber]);
      }
      
      if(p_err[portNumber] != OS_ERR_NONE)
      {
          bError = true;
      }
      
      if(false == bError)
      {
        OSSemCreate(&uartSemSend[portNumber],
                  uartSendSemNames[portNumber], 
                  (OS_SEM_CTR)0,   
                  &p_err[portNumber]);
      }

      if (p_err[portNumber]  != OS_ERR_NONE)
      {
          bError = true;
      }
      
      
     
      disableSerialPortTxLine(portNumber);
    }
    
    if( (true == bError ) || (false == isConfigValid) )
    {
        bError =true;
        //setError(E_ERROR_UART_DRIVER);
    }
    return bError;
}

/**
 * @brief gets the size of the buffer holding the UART receive string
 * @return size of the buffer
 */
bool getAvailableUARTxReceivedByteCount(PortNumber_t portNumber,
                                            uint16_t* avlBytes)
{
    bool retStatus = false;
    if((portNumber >= MAX_NUM_OF_UART_PORTS) || (avlBytes == NULL))
    {
      retStatus = false;
    }
    else
    {
       *avlBytes = (UartHandle[portNumber]->RxXferSize - UartHandle[portNumber]->RxXferCount);
       retStatus = true;
    }
                                                                   
    return retStatus;
}

/**
 * @brief de-intialize UART
 */
bool uartDeInit(PortNumber_t portNumber)
{
    bool retStatus = true;
    if(portNumber >= MAX_NUM_OF_UART_PORTS) 
    {
      retStatus = false;
    }
    else
    {
      if (HAL_UART_DeInit(UartHandle[portNumber]) != HAL_OK)
      {
          //setError(E_ERROR_UART_DRIVER);
        retStatus = false;
      }
      else
      {
        retStatus = true;
      }
    }
    return retStatus;
}

/**
 * @brief sends a string
 * @param aTxBuffer
 * @param size
 */
void sendOverUSART1(uint8_t *aTxBuffer, uint32_t size)
{
    bool bError = false;
    
    rxReady[UART_PORT1] = false; //suspend receiving

    OSSemSet(&uartSemSend[UART_PORT1], (OS_SEM_CTR)0, &p_err[UART_PORT1]);

    enableSerialPortTxLine(UART_PORT1);

    if (HAL_UART_Transmit_IT(UartHandle[UART_PORT1], (uint8_t *)aTxBuffer, (uint16_t)size) != HAL_OK)
    {
        bError = true;
    }

    OSSemPend(&uartSemSend[UART_PORT1], (OS_TICK)500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT1]);

    //the function that posts the TX_SEMA also disable transmit but in case of timeout
    //we should do it here as well, just to make sure
    if (p_err[UART_PORT1] != OS_ERR_NONE)
    {
        disableSerialPortTxLine(UART_PORT1);
        bError = true;
    }

    rxReady[UART_PORT1] = true;  //resume receiving

    if (bError == true)
    {
        //setError(E_ERROR_UART_DRIVER);
    }
    
    

}

/**
 * @brief sends a string
 * @param aTxBuffer
 * @param size
 */
void sendOverUSART2(uint8_t *aTxBuffer, uint32_t size)
{

    bool bError = false;
    
    rxReady[UART_PORT2] = false; //suspend receiving
 
    OSSemSet(&uartSemSend[UART_PORT2], (OS_SEM_CTR)0, &p_err[UART_PORT2]);

    disableSerialPortTxLine(UART_PORT2);

    if (HAL_UART_Transmit_IT(UartHandle[UART_PORT2], (uint8_t *)aTxBuffer, (uint16_t)size) != HAL_OK)
    {
        bError = true;
    }

    OSSemPend(&uartSemSend[UART_PORT2], (OS_TICK)500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT2]);

    //the function that posts the TX_SEMA also disable transmit but in case of timeout
    //we should do it here as well, just to make sure
    if (p_err[UART_PORT2] != OS_ERR_NONE)
    {
        enableSerialPortTxLine(UART_PORT2);
        bError = true;
    }

    rxReady[UART_PORT2] = true;  //resume receiving

    if (bError == true)
    {
        //setError(E_ERROR_UART_DRIVER);
    }
    
}

void sendOverUSART3(uint8_t *aTxBuffer, uint32_t size)
{  

    bool bError = false;
    
    rxReady[UART_PORT3] = false; //suspend receiving

    OSSemSet(&uartSemSend[UART_PORT3], (OS_SEM_CTR)0, &p_err[UART_PORT3]);

    enableSerialPortTxLine(UART_PORT3);

    if (HAL_UART_Transmit_IT(UartHandle[UART_PORT3], (uint8_t *)aTxBuffer, (uint16_t)size) != HAL_OK)
    {
        bError = true;
    }

    OSSemPend(&uartSemSend[UART_PORT3], (OS_TICK)500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT3]);

    //the function that posts the TX_SEMA also disable transmit but in case of timeout
    //we should do it here as well, just to make sure
    if (p_err[UART_PORT3] != OS_ERR_NONE)
    {
        disableSerialPortTxLine(UART_PORT3);
        bError = true;
    }

    rxReady[UART_PORT3] = true;  //resume receiving

    if (bError == true)
    {
        //setError(E_ERROR_UART_DRIVER);
    }
    
}


void sendOverUART4(uint8_t *aTxBuffer, uint32_t size)
{   

    bool bError = false;
    
    rxReady[UART_PORT4] = false; //suspend receiving

    OSSemSet(&uartSemSend[UART_PORT4], (OS_SEM_CTR)0, &p_err[UART_PORT4]);

    disableSerialPortTxLine(UART_PORT4);

    if (HAL_UART_Transmit_IT(UartHandle[UART_PORT4], (uint8_t *)aTxBuffer, (uint16_t)size) != HAL_OK)
    {
        bError = true;
    }

    OSSemPend(&uartSemSend[UART_PORT4], (OS_TICK)500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT4]);

    //the function that posts the TX_SEMA also disable transmit but in case of timeout
    //we should do it here as well, just to make sure
    if (p_err[UART_PORT4] != OS_ERR_NONE)
    {
        enableSerialPortTxLine(UART_PORT4);
        bError = true;
    }

    rxReady[UART_PORT4] = true;  //resume receiving

    if (bError == true)
    {
        //setError(E_ERROR_UART_DRIVER);
    }
    
}

void sendOverUART5(uint8_t *aTxBuffer, uint32_t size)
{   

    bool bError = false;
    
    rxReady[UART_PORT5] = false; //suspend receiving

    OSSemSet(&uartSemSend[UART_PORT5], (OS_SEM_CTR)0, &p_err[UART_PORT5]);

    enableSerialPortTxLine(UART_PORT5);

    if (HAL_UART_Transmit_IT(UartHandle[UART_PORT5], (uint8_t *)aTxBuffer, (uint16_t)size) != HAL_OK)
    {
        bError = true;
    }

    OSSemPend(&uartSemSend[UART_PORT5], (OS_TICK)500u, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT5]);

    //the function that posts the TX_SEMA also disable transmit but in case of timeout
    //we should do it here as well, just to make sure
    if (p_err[UART_PORT5] != OS_ERR_NONE)
    {
        disableSerialPortTxLine(UART_PORT5);
        bError = true;
    }

    rxReady[UART_PORT4] = true;  //resume receiving

    if (bError == true)
    {
        //setError(E_ERROR_UART_DRIVER);
    }
    
}
/**
 * @brief   Clear buffer and set interrupts and semaphore to receive message
 * @return  void
 */
bool ClearUARTxRcvBuffer(PortNumber_t portNumber)
{
    bool rcvState = false;
    HAL_StatusTypeDef uartError = HAL_ERROR;
    if(portNumber < MAX_NUM_OF_UART_PORTS)
    {
      HAL_UART_AbortReceive_IT(UartHandle[portNumber]);
       switch(portNumber)
       {
          case UART_PORT1:
             memset(&usart1RxBuffer[0], 0x00, (uint32_t)receiveBufferSize[UART_PORT1]);

             OSSemSet(&uartSemRcv[UART_PORT1], (OS_SEM_CTR)0, &p_err[UART_PORT1]);

              HAL_UART_Receive_IT(UartHandle[UART_PORT1], 
                                                       (uint8_t *)&usart1RxBuffer[0], 
                                                       (uint16_t)receiveBufferSize[UART_PORT1]);            
            break;
          case UART_PORT2:
              memset(&usart2RxBuffer[0], 0x00, (uint32_t)receiveBufferSize[UART_PORT2]);

             OSSemSet(&uartSemRcv[UART_PORT2], (OS_SEM_CTR)0, &p_err[UART_PORT2]);

             HAL_UART_Receive_IT(UartHandle[UART_PORT2], 
                                                       (uint8_t *)&usart2RxBuffer[0], 
                                                       (uint16_t)receiveBufferSize[UART_PORT2]);              

            break;
          case UART_PORT3:
              memset(&usart3RxBuffer[0], 0x00, (uint32_t)receiveBufferSize[UART_PORT3]);

             OSSemSet(&uartSemRcv[UART_PORT3], (OS_SEM_CTR)0, &p_err[UART_PORT3]);

             HAL_UART_Receive_IT(UartHandle[UART_PORT3], 
                                                       (uint8_t *)&usart3RxBuffer[0], 
                                                       (uint16_t)receiveBufferSize[UART_PORT3]);              

            break;
          case UART_PORT4:
              memset(&uart4RxBuffer[0], 0x00, (uint32_t)receiveBufferSize[UART_PORT4]);

             OSSemSet(&uartSemRcv[UART_PORT4], (OS_SEM_CTR)0, &p_err[UART_PORT4]);

             HAL_UART_Receive_IT(UartHandle[UART_PORT4], 
                                                       (uint8_t *)&uart4RxBuffer[0], 
                                                       (uint16_t)receiveBufferSize[UART_PORT4]);              

            break;  
         
          case UART_PORT5:
              memset(&uart5RxBuffer[0], 0x00, (uint32_t)receiveBufferSize[UART_PORT5]);

             OSSemSet(&uartSemRcv[UART_PORT5], (OS_SEM_CTR)0, &p_err[UART_PORT5]);

             HAL_UART_Receive_IT(UartHandle[UART_PORT5], 
                                                       (uint8_t *)&uart5RxBuffer[0], 
                                                       (uint16_t)receiveBufferSize[UART_PORT5]);              

            break;    
          default:
            rcvState = false;
           break;
       }
    

      switch (uartError)
      {
          case HAL_ERROR:
              //setError(E_ERROR_UART_DRIVER);
              break;

          case HAL_BUSY:
              break;

          case HAL_OK:
              rcvState = true;
              break;

          default:
              break;
      }
    }
    return rcvState;
}

/**
 * @brief waits for a message to be received
 * @param max timeout for the waiting semaphore (in milliseconds)
 */
bool waitToReceiveOverUsart1(uint32_t numberOfToRead, uint32_t timeout)
{
    bool wait = false;
    expectedNumOfBytes[UART_PORT1] = (uint16_t)numberOfToRead;
    OSSemPend(&uartSemRcv[UART_PORT1], timeout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT1]);

    if (p_err[UART_PORT1] == OS_ERR_NONE)
    {
        wait = true;
    }

    return wait;
}


/**
 * @brief waits for a message to be received
 * @param max timeout for the waiting semaphore (in milliseconds)
 */
bool waitToReceiveOverUsart2(uint32_t numberOfToRead, uint32_t timeout)
{
    bool wait = false;
        
    expectedNumOfBytes[UART_PORT2] = (uint16_t)numberOfToRead;
    OSSemPend(&uartSemRcv[UART_PORT2], timeout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT2]);
    
    if (p_err[UART_PORT2] == OS_ERR_NONE)
    {
        wait = true;
    }

    return wait;
}

/**
 * @brief waits for a message to be received
 * @param max timeout for the waiting semaphore (in milliseconds)
 */
bool waitToReceiveOverUsart3(uint32_t numberOfToRead, uint32_t timeout)
{
    bool wait = false;
    
    expectedNumOfBytes[UART_PORT3] = (uint16_t)numberOfToRead;
    OSSemPend(&uartSemRcv[UART_PORT3], timeout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT3]);

    if (p_err[UART_PORT3] == OS_ERR_NONE)
    {
        wait = true;
    }

    return wait;
}

/**
 * @brief waits for a message to be received
 * @param max timeout for the waiting semaphore (in milliseconds)
 */
bool waitToReceiveOverUart4(uint32_t numberOfToRead, uint32_t timeout)
{
    bool wait = false;

    expectedNumOfBytes[UART_PORT4] = (uint16_t)numberOfToRead; 
    
    OSSemPend(&uartSemRcv[UART_PORT4], timeout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT4]);

    if (p_err[UART_PORT4] == OS_ERR_NONE)
    {
        wait = true;
    }

    return wait;
}

/**
 * @brief waits for a message to be received
 * @param max timeout for the waiting semaphore (in milliseconds)
 */
bool waitToReceiveOverUart5(uint32_t numberOfToRead, uint32_t timeout)
{
    bool wait = false;

    expectedNumOfBytes[UART_PORT5] = (uint16_t)numberOfToRead;
    
    OSSemPend(&uartSemRcv[UART_PORT5], timeout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &p_err[UART_PORT5]);

    if (p_err[UART_PORT5]== OS_ERR_NONE)
    {
        wait = true;
    }

    return wait;
}
/**
 * @brief  UART error callbacks
 * @param  huart: UART handle
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if(USART1 == huart->Instance)
  {
    ClearUARTxRcvBuffer(UART_PORT1);
  }
  else if(USART2 == huart->Instance)
  {
    ClearUARTxRcvBuffer(UART_PORT2);
  }
  else if(USART3 == huart->Instance)
  {
    ClearUARTxRcvBuffer(UART_PORT3);
  }
  else if(UART4 == huart->Instance)
  {
    ClearUARTxRcvBuffer(UART_PORT4);
  }
  else if(UART5 == huart->Instance)
  {
    ClearUARTxRcvBuffer(UART_PORT5);
  }
  else
  {
    /* Do nothing*/
  }
}

/**
 * @brief  Tx Transfer completed callback and triggers semaphore when finishing
 * @param  huart: UART huart.
 */
    
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // Set transmission flag: transfer complete 
    
  if(USART1 == huart->Instance)
  {
    rxReady[UART_PORT1] = true;
    disableSerialPortTxLine(UART_PORT1);
    OSSemPost(&uartSemSend[UART_PORT1], OS_OPT_POST_1, &p_err[UART_PORT1]);
  }
  else if(USART2 == huart->Instance)
  {
    rxReady[UART_PORT2] = true;
    enableSerialPortTxLine(UART_PORT2);
    OSSemPost(&uartSemSend[UART_PORT2], OS_OPT_POST_1, &p_err[UART_PORT2]);
  }
  else if(USART3 == huart->Instance)
  {
    rxReady[UART_PORT3] = true;
    disableSerialPortTxLine(UART_PORT3);
    OSSemPost(&uartSemSend[UART_PORT3], OS_OPT_POST_1, &p_err[UART_PORT3]);
  }
  else if(UART4 == huart->Instance)
  {
    rxReady[UART_PORT4] = true;
    enableSerialPortTxLine(UART_PORT4);
    OSSemPost(&uartSemSend[UART_PORT4], OS_OPT_POST_1, &p_err[UART_PORT4]);
  }
  else if(UART5 == huart->Instance)
  {
    rxReady[UART_PORT5] = true;
    disableSerialPortTxLine(UART_PORT5);
    OSSemPost(&uartSemSend[UART_PORT5], OS_OPT_POST_1, &p_err[UART_PORT5]);
  }
  else
  {
    /* Do Nothing */
  }
    
}

/**
 * @brief  Rx Transfer completed callback and triggers semaphore
 * @param  huart: UART huart
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // Set transmission flag: transfer complete 
  if(USART1 == huart->Instance)
  {
    OSSemPost(&uartSemRcv[UART_PORT1], OS_OPT_POST_1, &p_err[UART_PORT1]);
  }
  else if(USART2 == huart->Instance)
  {
    OSSemPost(&uartSemRcv[UART_PORT2], OS_OPT_POST_1, &p_err[UART_PORT2]);
  }
  else if(USART3 == huart->Instance)
  {
    OSSemPost(&uartSemRcv[UART_PORT3], OS_OPT_POST_1, &p_err[UART_PORT3]);
  }
  else if(UART4 == huart->Instance)
  {
    OSSemPost(&uartSemRcv[UART_PORT4], OS_OPT_POST_1, &p_err[UART_PORT4]);
    disableSerialPortTxLine(UART_PORT4);
  }
  else if(UART5 == huart->Instance)
  {
    OSSemPost(&uartSemRcv[UART_PORT5], OS_OPT_POST_1, &p_err[UART_PORT5]);
  }
  else
  {
    
  }
}

/**
 * @brief  This function handles USART1 interrupt request.
 */
void USART1_IRQHandler(void)
{
   
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
    if ((UART_IsRX(UartHandle[UART_PORT1]) == true) && (rxReady[UART_PORT1] == true))
    {
        uint16_t rxDataReg = UartHandle[UART_PORT1]->Instance->RDR;

        if (((rxDataReg != 0u) && (0u == expectedNumOfBytes[UART_PORT1])) ||
            (expectedNumOfBytes[UART_PORT1] > 0u))
        {
            //prevent buffer overflow by not allowing the count to go beyond buffer capacity
            uint16_t index = (UartHandle[UART_PORT1]->RxXferSize - UartHandle[UART_PORT1]->RxXferCount) % UartHandle[UART_PORT1]->RxXferSize;

            if (UartHandle[UART_PORT1]->RxXferCount > 1u)
            {
                UartHandle[UART_PORT1]->RxXferCount--;
            }

            UartHandle[UART_PORT1]->pRxBuffPtr[index] = (uint8_t)rxDataReg;

            if(((index+1u) >= expectedNumOfBytes[UART_PORT1]) && 
                (expectedNumOfBytes[UART_PORT1] > 0u)
               )
            {
               HAL_UART_RxCpltCallback(UartHandle[UART_PORT1]);
            }
            //check if this is the terminating character
            else if ((rxDataReg == '\n') && (0u == expectedNumOfBytes[UART_PORT1]))  
            {
                HAL_UART_RxCpltCallback(UartHandle[UART_PORT1]);
            }
            else
            {
              
            }
           
        }
    }
    else
    {
        HAL_UART_IRQHandler(UartHandle[UART_PORT1]);
    }

     OSIntExit();
}


/**
 * @brief  This function handles USART1 interrupt request.
 */
void USART2_IRQHandler(void)
{
      CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
    volatile uint16_t value = (uint16_t)(0);

   
    if ((UART_IsRX(UartHandle[UART_PORT2]) == true) && (rxReady[UART_PORT2] == true))
    {
        uint32_t rxDataReg = UartHandle[UART_PORT2]->Instance->RDR;

         if (((rxDataReg != 0u) && (0u == expectedNumOfBytes[UART_PORT2])) ||
            (expectedNumOfBytes[UART_PORT2] > 0u))
        {
            //prevent buffer overflow by not allowing the count to go beyond buffer capacity
            uint16_t index = (UartHandle[UART_PORT2]->RxXferSize - UartHandle[UART_PORT2]->RxXferCount) % UartHandle[UART_PORT2]->RxXferSize;
            
            if (UartHandle[UART_PORT2]->RxXferCount > 1u)
            {
                UartHandle[UART_PORT2]->RxXferCount--;
            }

            UartHandle[UART_PORT2]->pRxBuffPtr[index] = (uint8_t)rxDataReg;

           if(((index+1u) >= expectedNumOfBytes[UART_PORT2]) && 
                (expectedNumOfBytes[UART_PORT2] > 0u))
            {
               HAL_UART_RxCpltCallback(UartHandle[UART_PORT2]);
               //intRcvFlag = (uint32_t)(1);
               value = index;
               
            }
            //check if this is the terminating character
           /*
             else if ((rxDataReg == '\n') && (0u == expectedNumOfBytes[UART_PORT2]))  
            {
                HAL_UART_RxCpltCallback(UartHandle[UART_PORT2]);
                //intRcvFlag = (uint32_t)(1);
                disableSerialPortTxLine(UART_PORT3);
                
            }
*/
           else
           {
             
           }
        }
    }
    else
    {
        HAL_UART_IRQHandler(UartHandle[UART_PORT2]);
    }

     OSIntExit();
}


/**
 * @brief  This function handles USART1 interrupt request.
 */
void USART3_IRQHandler(void)
{
      CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();

    if ((UART_IsRX(UartHandle[UART_PORT3]) == true) && (rxReady[UART_PORT3] == true))
    {
        uint32_t rxDataReg = UartHandle[UART_PORT3]->Instance->RDR;

        if (((rxDataReg != 0u) && (0u == expectedNumOfBytes[UART_PORT3])) ||
            (expectedNumOfBytes[UART_PORT3] > 0u))
        {
            //prevent buffer overflow by not allowing the count to go beyond buffer capacity
            uint16_t index = (UartHandle[UART_PORT3]->RxXferSize - UartHandle[UART_PORT3]->RxXferCount) % UartHandle[UART_PORT3]->RxXferSize;

            if (UartHandle[UART_PORT3]->RxXferCount > 1u)
            {
                UartHandle[UART_PORT3]->RxXferCount--;
            }

            UartHandle[UART_PORT3]->pRxBuffPtr[index] = (uint8_t)rxDataReg;

            if(((index + (uint16_t)(1)) >= expectedNumOfBytes[UART_PORT3]) && 
                (expectedNumOfBytes[UART_PORT3] > (uint16_t)(0)))
            {
               HAL_UART_RxCpltCallback(UartHandle[UART_PORT3]);
            }
            //check if this is the terminating character
            else if ((rxDataReg == '\n') && (0u == expectedNumOfBytes[UART_PORT3]))  
            {
                HAL_UART_RxCpltCallback(UartHandle[UART_PORT3]);
            }
            else
            {
              
            }
        }
    }
    else
    {
        HAL_UART_IRQHandler(UartHandle[UART_PORT3]);
    }

     OSIntExit();
}


/**
 * @brief  This function handles USART1 interrupt request.
 */
void UART4_IRQHandler(void)
{
      CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();

    if ((UART_IsRX(UartHandle[UART_PORT4]) == true) && (rxReady[UART_PORT4] == true))
    {
        uint32_t rxDataReg = UartHandle[UART_PORT4]->Instance->RDR;

         if (rxDataReg != 0u)
        {
            //prevent buffer overflow by not allowing the count to go beyond buffer capacity
            uint16_t index = (UartHandle[UART_PORT4]->RxXferSize - UartHandle[UART_PORT4]->RxXferCount) % UartHandle[UART_PORT4]->RxXferSize;

            if (UartHandle[UART_PORT4]->RxXferCount > 1u)
            {
                UartHandle[UART_PORT4]->RxXferCount--;
            }

            UartHandle[UART_PORT4]->pRxBuffPtr[index] = (uint8_t)rxDataReg;

           if ((uint8_t)rxDataReg == (uint8_t)0X0A) 
           {
             HAL_UART_RxCpltCallback(UartHandle[UART_PORT4]);
           }
        }
    }
    else
    {
        HAL_UART_IRQHandler(UartHandle[UART_PORT4]);
    }

     OSIntExit();
}


/**
 * @brief  This function handles USART1 interrupt request.
 */
void UART5_IRQHandler(void)
{
   
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();
  CPU_CRITICAL_EXIT();
    if ((UART_IsRX(UartHandle[UART_PORT5]) == true) && (rxReady[UART_PORT5] == true))
    {
        uint32_t rxDataReg = UartHandle[UART_PORT5]->Instance->RDR;

         if (((rxDataReg != 0u) && (0u == expectedNumOfBytes[UART_PORT5])) ||
            (expectedNumOfBytes[UART_PORT5] > 0u))
        {
            //prevent buffer overflow by not allowing the count to go beyond buffer capacity
            uint16_t index = (UartHandle[UART_PORT5]->RxXferSize - UartHandle[UART_PORT5]->RxXferCount) % UartHandle[UART_PORT5]->RxXferSize;

            if (UartHandle[UART_PORT5]->RxXferCount > 1u)
            {
                UartHandle[UART_PORT5]->RxXferCount--;
            }

            UartHandle[UART_PORT5]->pRxBuffPtr[index] = (uint8_t)rxDataReg;

            if(((index+1u) >= expectedNumOfBytes[UART_PORT5]) && 
                (expectedNumOfBytes[UART_PORT5] > 0u)
               )
            {
               HAL_UART_RxCpltCallback(UartHandle[UART_PORT5]);
            }
            //check if this is the terminating character
             else if ((rxDataReg == '\n') && (0u == expectedNumOfBytes[UART_PORT5]))  
            {
                HAL_UART_RxCpltCallback(UartHandle[UART_PORT5]);
            }
            else
            {
              
            }
        }
    }
    else
    {
        HAL_UART_IRQHandler(UartHandle[UART_PORT5]);
    }

     OSIntExit();
}
/**
 * @brief gets pointer to the buffer holding the UART receive string
 * @return the pointer to the union of buffers
 */
bool getHandleToUARTxRcvBuffer(PortNumber_t portNumber, uint8_t ** bufHdl)
{
    bool retStatus = true;
    
    switch(portNumber)
     {
        case UART_PORT1:
          *bufHdl = &usart1RxBuffer[0];
          break;
        case UART_PORT2:
          *bufHdl = &usart2RxBuffer[0];
          break;
        case UART_PORT3:
          *bufHdl = &usart3RxBuffer[0];
          break;
        case UART_PORT4:
          *bufHdl = &uart4RxBuffer[0];
          break;  
        case UART_PORT5:
          *bufHdl = &uart5RxBuffer[0];
          break;   
        default:
          retStatus = false;
         break;
     }
    return retStatus;
}

/**
 * @brief checks if the interrupt is receiving data
 * @param uhandle
 * @return true if is receiving data
 */
static bool UART_IsRX(UART_HandleTypeDef *uhandle)
{
    bool isReceiving = false;
    uint32_t isrflags = READ_REG(uhandle->Instance->ISR);
    uint32_t cr1its = READ_REG(uhandle->Instance->CR1);
    uint32_t cr3its = READ_REG(uhandle->Instance->CR3);

    /* UART in mode Receiver */

#if defined(USART_CR1_FIFOEN)
    if(((isrflags & USART_ISR_RXNE_RXFNE) != RESET)
            && (((cr1its & USART_CR1_RXNEIE_RXFNEIE) != RESET)
                || ((cr3its & USART_CR3_RXFTIE) != RESET)))
#else
    if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
#endif
    {
        isReceiving = true;
    }

    return isReceiving;
}

/*!
* @brief : This function changes the UART baudrate to either a valid 115200 or 19200
*
* @param[in]     :
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
static uint32_t UARTn_ChangeBaudRate( UART_HandleTypeDef *pHuart, const eUartnBaud_t baudRate )
{
    uint32_t lError = 0u;
    
    
    if( pHuart->Init.BaudRate == ( uint32_t )baudRate )
    {
        //Do nothing as baudrate is already set
    }
    else
    {
      
        if( HAL_UART_DeInit( pHuart ) != HAL_OK )
        {
            //Error_Handler();
            lError |= 1u;
        }

        pHuart->Init.BaudRate = baudRate;
        pHuart->Init.WordLength = UART_WORDLENGTH_8B;
        pHuart->Init.StopBits = UART_STOPBITS_1;
        pHuart->Init.Parity = UART_PARITY_NONE;
        pHuart->Init.Mode = UART_MODE_TX_RX;
        pHuart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
        pHuart->Init.OverSampling = UART_OVERSAMPLING_16;
        pHuart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
        pHuart->Init.ClockPrescaler = UART_PRESCALER_DIV1;
        pHuart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_DMADISABLEONERROR_INIT;
        pHuart->AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;

        if ( HAL_UART_Init( pHuart ) != HAL_OK )
        {
            //Error_Handler();
            lError |= 1u;
        }

        if (HAL_UARTEx_SetTxFifoThreshold( pHuart, UART_TXFIFO_THRESHOLD_1_8 ) != HAL_OK)
        {
            //Error_Handler();
            lError |= 1u;
        }

        if (HAL_UARTEx_SetRxFifoThreshold( pHuart, UART_RXFIFO_THRESHOLD_1_8 ) != HAL_OK)
        {
            //Error_Handler();
            lError |= 1u;
        }

        if (HAL_UARTEx_EnableFifoMode( pHuart ) != HAL_OK)
        {
            //Error_Handler();
            lError |= 1u;
        }
    }

    return( lError );
}

/*!
* @brief : This function sets the termination type, communication type and baud rate of
*        : the respected uart
*
* @param[in]     : UART_HandleTypeDef* pHuart - pointer to respective UART instance
*                : eUartnTerm_t pTermination - Termination for RX comms
*                : eUartnType_t pCommType - slave or master
*                : eUartBaud_t pBaud - baud rate
* @param[out]    : None
* @param[in,out] : None
* @return        : uint32_t lError - 1 = fail, 0 = ok
* @note          : None
* @warning       : None
*/
uint32_t UARTn_TermType( UART_HandleTypeDef *pHuart, 
                         const eUartnTerm_t pTermination, 
                         const eUartnType_t pCommType, 
                         const eUartnBaud_t pBaud )
{
    uint32_t lError = 0u;
    
    PortNumber_t portnumber; 
    portnumber = getUartPortNumber( pHuart );

    if(( UART_INVALID_PORTNUMBER == portnumber ) || 
       ( pTermination > eUARTn_Term_CR ) || 
       ( pCommType > eUARTn_Type_Master ) || 
       ( pBaud > eUARTn_Baud_115200 ))
    {
        lError |= 1u;
    }
    else
    {
        lError |= UARTn_ChangeBaudRate( pHuart, pBaud );
        rxReady[portnumber]  = false;
        termType[portnumber] = pTermination;
        commType[portnumber] = pCommType;

        if( false == ClearUARTxRcvBuffer( portnumber ))
        {
            lError |= 1u;
        }
    }

    return( lError );
}
