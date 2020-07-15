/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                             OS TICK BSP
*
* Filename : bsp_os.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu_core.h>
#include  <os.h>

#include "stm32l4xx_hal.h"
#include "bsp_os.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#ifndef  OS_CFG_DYN_TICK_EN                                     /* Dynamic tick only available for uCOS-III             */
#define  OS_CFG_DYN_TICK_EN          DEF_DISABLED
#endif

#if (OS_CFG_DYN_TICK_EN == DEF_ENABLED)
#define  BSP_OS_LPTIMER_MAX_VALUE           DEF_INT_16U_MAX_VAL /* For a 16-bit counter  */
#define  BSP_OS_LPTICK_TO_OSTICK(lptick)    ((lptick) / LPTickPerOSTick) /* must be exact or round down */
#define  BSP_OS_OSTICK_TO_LPTICK(ostick)    ((ostick) * LPTickPerOSTick)
#define  HANDLE_TICK_OVERUN /* If time goes past ISR match/reload then include the difference */
#endif


/*
*********************************************************************************************************
*                                           LOCAL VARIABLES
*
*********************************************************************************************************
*/

#if (OS_CFG_DYN_TICK_EN == DEF_ENABLED)
LPTIM_HandleTypeDef *TimerHandlePtr;
int LPTickPerOSTick;
static OS_TICK SaveTickGet = 0u;    /* Base saved for OS_DynTickGet time calculation*/
static OS_TICK OSTicks2Go = 0u;     /*  Timer delta interval in equivalent OS ticks */
static OS_TICK TickBase = 0u;       /* Base used for next time calculation*/

#endif

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (OS_CFG_DYN_TICK_EN == DEF_ENABLED)
static  HAL_StatusTypeDef  BSP_OS_DynamicTickInit(LPTIM_HandleTypeDef *lptim);
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          BSP_OS_TickInit()
*
* Description : Initializes the tick interrupt for the OS.
*
* Argument(s) : Instance of LP Timer.
*
* Return(s)   : none.
*
* Note(s)     : (1) Must be called prior to OSStart() in main().
*
*               (2) This function ensures that the tick interrupt is disabled until BSP_OS_TickEn() is
*                   called in the startup task.
*********************************************************************************************************
*/

HAL_StatusTypeDef  BSP_OS_TickInit (LPTIM_HandleTypeDef *lptim)
{
#if (OS_CFG_DYN_TICK_EN != DEF_ENABLED)
    CPU_INT32U  cpu_freq;
    CPU_SR_ALLOC();

    cpu_freq = BSP_ClkFreqGet(CLK_ID_HCLK);                     /* Determine SysTick reference freq.                    */

    CPU_CRITICAL_ENTER();
    OS_CPU_SysTickInitFreq(cpu_freq);                           /* Init uC/OS periodic time src (SysTick).              */
    BSP_OS_TickDisable();                                       /* See Note #2.                                         */
    CPU_CRITICAL_EXIT();
    return HAL_OK;
#else
    return BSP_OS_DynamicTickInit(lptim);                                   /* Initialize dynamic tick.                             */
#endif
}


/*
*********************************************************************************************************
*                                         BSP_OS_TickEnable()
*
* Description : Enable the OS tick interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none
*********************************************************************************************************
*/

void  BSP_OS_TickEnable (void)
{
#if (OS_CFG_DYN_TICK_EN != DEF_ENABLED)
    CPU_REG_NVIC_ST_CTRL |= (CPU_REG_NVIC_ST_CTRL_TICKINT |     /* Enables SysTick exception request                    */
                             CPU_REG_NVIC_ST_CTRL_ENABLE);      /* Enables SysTick counter                              */
#else
  __HAL_LPTIM_CLEAR_FLAG(TimerHandlePtr, 0xff);
  __HAL_LPTIM_ENABLE_IT(TimerHandlePtr, LPTIM_IT_ARRM);
  __HAL_LPTIM_ENABLE(TimerHandlePtr);  
  __HAL_LPTIM_START_CONTINUOUS(TimerHandlePtr);                    /* Start the Timer count generation.                    */
#endif
}


/*
*********************************************************************************************************
*                                        BSP_OS_TickDisable()
*
* Description : Disable the OS tick interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none
*********************************************************************************************************
*/

void  BSP_OS_TickDisable (void)
{
#if (OS_CFG_DYN_TICK_EN != DEF_ENABLED)
    CPU_REG_NVIC_ST_CTRL &= ~(CPU_REG_NVIC_ST_CTRL_TICKINT |    /* Disables SysTick exception request                   */
                              CPU_REG_NVIC_ST_CTRL_ENABLE);     /* Disables SysTick counter                             */
#else
    __HAL_LPTIM_DISABLE(TimerHandlePtr);              /* Stop the Timer count generation.                     */
#endif
}

#ifdef STMTICK
/*
*********************************************************************************************************
*                                            HAL_InitTick()
*
* Description : This function has been overwritten from the STM32Cube HAL libraries because Micrium's RTOS
*               has its own Systick initialization and because it is recomended to initialize the tick
*               after multi-tasking has started.
*
* Argument(s) : TickPriority          Tick interrupt priority.
*
* Return(s)   : HAL_OK.
*
* Caller(s)   : HAL_InitTick ()) is called automatically at the beginning of the program after reset by
*               HAL_Init() or at any time when clock is configured, by HAL_RCC_ClockConfig().
*
* Note(s)     : none.
*********************************************************************************************************
*/

HAL_StatusTypeDef  HAL_InitTick (uint32_t TickPriority)
{
  // This should be done in main because it affects entire system
  //  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITY_GROUP4);

    return (HAL_OK);
}


/*
*********************************************************************************************************
*                                            HAL_GetTick()
*
* Description : This function has been overwritten from the STM32Cube HAL libraries because Micrium's OS's
*               has their own Tick counter values.
*
* Argument(s) : None.
*
* Return(s)   : Tick current value.
*
* Caller(s)   : STM32Cube HAL driver files.
*
* Note(s)     : (1) Please ensure that the Tick Task has a higher priority than the App Start Task.
*********************************************************************************************************
*/

uint32_t  HAL_GetTick (void)
{
    CPU_INT32U  os_tick_ctr;
#if (OS_VERSION >= 30000u)
    OS_ERR      os_err;
#endif

#if (OS_VERSION >= 30000u)
    os_tick_ctr = OSTimeGet(&os_err);
#else
    os_tick_ctr = OSTimeGet();
#endif

    return os_tick_ctr;
}
#endif

/*
*********************************************************************************************************
*********************************************************************************************************
**                                      uC/OS-III DYNAMIC TICK
*********************************************************************************************************
*********************************************************************************************************
*/

#if !defined(LPCLKHZ)
#error "Dynamic Tick: LSI Clock Frequency undefined."
#endif


#if (OS_CFG_DYN_TICK_EN == DEF_ENABLED)

/*
*********************************************************************************************************
*                                      BSP_OS_DynamicTickInit()
*
* Description : Initialize timer to use for dynamic tick.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  HAL_StatusTypeDef  BSP_OS_DynamicTickInit (LPTIM_HandleTypeDef *Handle)
{
  TimerHandlePtr = Handle;
  /* Ensure LSI clock is ON and READY. Else set  Timeout state
   * This avoids endless wait loop if programmer messed up clock setup.
   */
   if (((RCC->CSR & RCC_CSR_LSIRDY) != RCC_CSR_LSIRDY) || \
            ((RCC->CSR & RCC_CSR_LSION) != RCC_CSR_LSION))
   {
       return HAL_TIMEOUT;
   }
  /* Start the timer interrupt mode generation.           */
  /* Better if this was just a constant */
  LPTickPerOSTick = (LPCLKHZ  + (OSCfg_TickRate_Hz>>1))/ OSCfg_TickRate_Hz ;
  /* Prescaler and round result */
  //LPTickPerOSTick = (LPTickPerOSTick + 64) / 128;
  
  /* Change the TIM state*/
  TimerHandlePtr->State = HAL_LPTIM_STATE_BUSY;
  /* Reset the timer counter value                        */
  __HAL_LPTIM_ENABLE(TimerHandlePtr);  
  __HAL_LPTIM_CLEAR_FLAG(TimerHandlePtr, 0xff);
  __HAL_LPTIM_AUTORELOAD_SET(TimerHandlePtr, BSP_OS_OSTICK_TO_LPTICK(1) );         /* Re-configure auto-reload register and period.        */
  OSTicks2Go = 1;
  while (!__HAL_LPTIM_GET_FLAG(TimerHandlePtr, LPTIM_FLAG_ARROK));  /* Wait for it to load */
  __HAL_LPTIM_DISABLE(TimerHandlePtr);                            /* Stop the Timer count generation.                     */
  /* Change the TIM state*/
  TimerHandlePtr->State = HAL_LPTIM_STATE_READY;
  return HAL_OK;
}


/*
*********************************************************************************************************
*                                        LPTIMx_IRQHandler()
*
* Description : BSP-level ISR handler for LPTIM.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  LPTIM1_IRQHandler (void)
{
  OS_TICK lpticks, ticks;
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  if (LowPowerMode) ExitLowPowerMode();
  OSIntEnter();
  CPU_CRITICAL_EXIT();


  /* Micrium say tick count return  should not exceed interval requested. Even if timer is late.
  * But that means late timer will cause loss of time. Simple fix in Micrium os_tick.c permits
  * return of excess so that time loss is not cumulative. The fix is to check calculation:
  *     if (elapsed >  p_tcb2->TickRemain) // Dont subtract, TickRemain-elapsed. Use zero.
  *
  */
#ifdef HANDLE_TICK_OVERUN
  /* Read count twice as per manual */
  do {
    lpticks = TimerHandlePtr->Instance->CNT;       /* Get the timer counter value. */  
  }
  while (lpticks != TimerHandlePtr->Instance->CNT);
  ticks = BSP_OS_LPTICK_TO_OSTICK(lpticks);
#else
   ticks = 0;
#endif
  /* Save base for next tick calculation */
  TickBase = SaveTickGet = ticks;
  
  /*Report elapsed - which can be higher than desired OSTicks2Go. */
  OSTimeDynTick(OSTicks2Go + ticks);
  OSIntExit();
}

/*
*********************************************************************************************************
*                                          OS_DynTickGet()
*
* Description : Get the OS Tick Counter as if it was running continuously.
*
* Argument(s) : none.
*
* Return(s)   : The effective OS Tick Counter.
*
* Caller(s)   : OS_TaskBlock, OS_TickListInsertDly and OSTimeGet.
*
*               This function is an INTERNAL uC/OS-III function & MUST NOT be called by application
*               function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

OS_TICK  OS_DynTickGet (void)
{
  OS_TICK  ticks;


  /* Read twice as per manual */
  do {
    ticks = TimerHandlePtr->Instance->CNT;  
  }
  while (ticks != TimerHandlePtr->Instance->CNT);
  
  /* if ISR end is already? */
  
  if(__HAL_LPTIM_GET_FLAG(TimerHandlePtr, LPTIM_FLAG_ARRM) != RESET)
  {
    /* This should only happen if dynamic tick ISR is masked. 
      Report same time as ISR does (will) - which can be higher than desired OSTicks2Go.
    */
#ifdef HANDLE_TICK_OVERUN    
    ticks = BSP_OS_LPTICK_TO_OSTICK(ticks) + OSTicks2Go;
#else
    ticks =  OSTicks2Go;
#endif
  }
  else
  { 
    // Base for OS_DynTickGet was saved at last  ISR
    ticks = BSP_OS_LPTICK_TO_OSTICK(ticks);
  }
  // Base for OS_DynTickSet is this - unless ISR overides it.
  TickBase = ticks;
  
  return ticks - SaveTickGet;
}


/*
*********************************************************************************************************
*                                        OS_DynTickSet()
*
* Description : Set the number of OS Ticks to wait before calling OSTimeTick.
*
* Argument(s) : ticks       number of OS Ticks to wait.
*
* Return(s)   : Number of effective OS Ticks until next OSTimeTick.
*
* Caller(s)   : OS_TickTask and OS_TickListInsert.
*
*               This function is an INTERNAL uC/OS-III function & MUST NOT be called by application
*               function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

OS_TICK  OS_DynTickSet (OS_TICK  ticks)
{
  OS_TICK  lpticks ;
  /* Limit to range of timer*/
  OS_TICK limit = BSP_OS_LPTICK_TO_OSTICK(BSP_OS_LPTIMER_MAX_VALUE + 1);

  /* Ticks requested by OS is based on last update of OS tick counter.
   * The OS tick counter was updated by either ISR or by last call to OS_DynTickGet.
   * The LP timer could now be at a higher count if system is heavily loaded.
   * So simply reloading (and resetting) now will cause an error in OS time.
   * TickBase is used to capture the correct base for new delta.
   * Also update SaveTickGet because we have a new base for future OS_DynTickGet.
    */

  /* ticks == 0 means infinite */
  if (ticks == 0)
  {
      SaveTickGet = TickBase;   
      ticks = limit;
  }
  else
  {
    // Add base to create reload value (in OS ticks)
    ticks += SaveTickGet = TickBase;   
    // Limit value
    if (ticks > limit) 
    {
        ticks = limit;
    }
  }
  /* Timer reload value that is exact number of ticks.
   * ISR happens cycle after  match so subtract 1 clock.
  */
  lpticks = BSP_OS_OSTICK_TO_LPTICK(ticks) - 1;
#ifdef HANDLE_TICK_OVERUN    

  /* If current timer count is already higher than new ARR, it will immediately
   * reset one clock after ARR load and interrupt. (tested)
   * That will be functional but cause a loss of long term count accuracy. 
   * Avoid by setting OSTicks2Go based on current count.
   */
  OS_TICK current_count;
  do {
    current_count = TimerHandlePtr->Instance->CNT;  
  }
  while (current_count != TimerHandlePtr->Instance->CNT);
  if (current_count >= lpticks)
  {
    ticks = BSP_OS_LPTICK_TO_OSTICK(current_count);
  }
#endif
  
  /* The increment in OS ticks  will be this on next ISR */
  OSTicks2Go = ticks - TickBase;

  /* Note reload will take 2 counter clock cycles - which is relatively slow.
  *  So we dont wait and waste time.
  */
  __HAL_LPTIM_CLEAR_FLAG(TimerHandlePtr, 0xff);
  __HAL_LPTIM_AUTORELOAD_SET(TimerHandlePtr, lpticks  ); 
  //while (!__HAL_LPTIM_GET_FLAG(TimerHandlePtr->Instance,LPTIM_FLAG_ARROK));  /* Wait for it to load */
  /* Return the number of effective OS ticks that will... */
  return (OSTicks2Go);        /* ...elapse before the next interrupt.                 */
}


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of uC/OS-III Dynamic Tick module.                */
