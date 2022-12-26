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
*                                      APPLICATION CONFIGURATION
*
*                                            EXAMPLE CODE
*
* Filename : app_cfg.h
*********************************************************************************************************
*/

#ifndef  _APP_CFG_H_
#define  _APP_CFG_H_


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>


/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_CFG_STARTUP_TASK_PRIO              3u
#define  APP_CFG_EXTERNAL_SLOT_TASK_PRIO        3u
#define  APP_CFG_REF_SENSOR_TASK_PRIO	        3u
#define  APP_CFG_FUNCTION_TASK_PRIO	        4u
#define  APP_CFG_MEASURE_CONTROL_TASK_PRIO	4u
#define  APP_CFG_COMMS_TASK_PRIO	        5u
#define  APP_CFG_POWER_MANAGER_TASK_PRIO	5u
#define  APP_CFG_SLOT_TASK_PRIO	                5u
#define  APP_CFG_KEY_HANDLER_TASK_PRIO	        5u
#define  APP_CFG_BAROMETER_TASK_PRIO	        5u
#define  APP_CFG_USER_INTERFACE_TASK_PRIO	5u
		
		
#define  APP_CFG_EXT_STORAGE_TASK_PRIO	        14u
#define  APP_CFG_LOGGER_TASK_PRIO	        15u
#define  APP_CFG_PRODUCTION_TEST_TASK_PRIO	16u

/*
*********************************************************************************************************
*                                          TASK MESSAGE QUANTITIES
*                             
*********************************************************************************************************
*/
#define  APP_CFG_EXTERNAL_SLOT_TASK_MSG_QTY	10u
#define  APP_CFG_REF_SENSOR_TASK_MSG_QTY	10u
#define  APP_CFG_FUNCTION_TASK_MSG_QTY	        10u
#define  APP_CFG_MEASURE_CONTROL_TASK_MSG_QTY	10u
#define  APP_CFG_COMMS_TASK_MSG_QTY              0u
#define  APP_CFG_POWER_MANAGER_TASK_MSG_QTY	10u
#define  APP_CFG_SLOT_TASK_MSG_QTY	        10u
#define  APP_CFG_KEY_HANDLER_TASK_MSG_QTY	0u
#define  APP_CFG_BAROMETER_TASK_MSG_QTY	        10u
#define  APP_CFG_USER_INTERFACE_TASK_MSG_QTY	10u
		
		
#define  APP_CFG_EXT_STORAGE_TASK_MSG_QTY	1u
#define  APP_CFG_LOGGER_TASK_MSG_QTY	        80u
#define  APP_CFG_PRODUCTION_TEST_TASK_MSG_QTY	10u



/*
*********************************************************************************************************
*                                          TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_CFG_STARTUP_TASK_STK_SIZE                    4096u
#define  APP_CFG_EXT_STORAGE_TASK_STK_SIZE                8192u
#define  APP_CFG_KEY_HANDLER_TASK_STK_SIZE                 512u
#define  APP_CFG_DATALOGGER_TASK_STK_SIZE                 2048u
#define  APP_CFG_POWER_MANAGER_TASK_STACK_SIZE            1024u
#define  APP_CFG_BAROMETER_TASK_STACK_SIZE                1024u
#define  APP_CFG_REF_SENSOR_TASK_STACK_SIZE               8192u
#define  APP_CFG_USER_INTERFACE_TASK_STK_SIZE              512u
#define  APP_CFG_PRODUCTION_TEST_TASK_STK_SIZE             256u
#define  APP_CFG_MEASURE_CONTROL_TASK_STACK_SIZE          4096u
/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#define  APP_TRACE_LEVEL                   TRACE_LEVEL_OFF
#define  APP_TRACE                         printf

#define  APP_TRACE_INFO(x)    ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE (x)) : (void)0)
#define  APP_TRACE_DBG(x)     ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_TRACE (x)) : (void)0)


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of module include.              */
