/*
*********************************************************************************************************
*                                                uC/CPU
*                                    CPU CONFIGURATION & PORT LAYER
*
*                         (c) Copyright 2004-2019; Silicon Laboratories Inc.,
*                                400 W. Cesar Chavez, Austin, TX 78701
*
*                   All rights reserved. Protected by international copyright laws.
*
*                  Your use of this software is subject to your acceptance of the terms
*                  of a Silicon Labs Micrium software license, which can be obtained by
*                  contacting info@micrium.com. If you do not agree to the terms of this
*                  license, you may not use this software.
*
*                  Please help us continue to provide the Embedded community with the finest
*                  software available. Your honesty is greatly appreciated.
*
*                    You can find our product's documentation at: doc.micrium.com
*
*                          For more information visit us at: www.micrium.com
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                        CPU CACHE IMPLEMENTATION
*
*                                           Synopsys ARC EM6
*
* Filename : cpu_cache_em6.c
* Version  : V1.31.05
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define    MICRIUM_SOURCE
#include  <cpu_cache.h>
#include  <lib_def.h>


/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

                                                                /* Cache operations.                                    */
#define  CPU_CACHE_EM6_INVALIDATE                  CPU_AR_DC_IVDL
#define  CPU_CACHE_EM6_FLUSH                       CPU_AR_DC_FLDL


/*
*********************************************************************************************************
*                                            LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  CPU_Cache_Do (CPU_INT32U   what,
                    void        *addr_start,
                    CPU_ADDR     len);


/*
*********************************************************************************************************
*                                       LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            CPU_Cache_Init()
*
* Description : Initializes the cache.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  CPU_Cache_Init (void)
{
#if (defined(CPU_CFG_CACHE_MGMT_EN) && CPU_CFG_CACHE_MGMT_EN == DEF_ENABLED)
                                                                /* Enable data cache.                                   */
    CPU_AR_WR(CPU_AR_DC_CTRL, CPU_AR_DC_CTRL_DC_EN);

                                                                /* Invalidate data cache.                               */
    CPU_AR_WR(CPU_AR_DC_IVDC, CPU_AR_DC_IVDC_IV);

                                                                /* Flush data cache.                                    */
    CPU_AR_WR(CPU_AR_DC_FLSH, CPU_AR_DC_FLSH_FL);

    while ((CPU_AR_RD(CPU_AR_DC_CTRL) & CPU_AR_DC_CTRL_FS) > 0u) {
        ;
    }
#endif
}


/*
*********************************************************************************************************
*                                        CPU_DCache_RangeFlush()
*
* Description : Flushes a range of the data cache to the main memory.
*
* Argument(s) : addr_start  Byte address of the beginning of the range.
*
*               len         Size in bytes of the range.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  CPU_DCache_RangeFlush (void      *addr_start,
                             CPU_ADDR   len)
{
#if (defined(CPU_CFG_CACHE_MGMT_EN) && CPU_CFG_CACHE_MGMT_EN == DEF_ENABLED)
                                                                /* Flush range, if found, to main memory                */
    CPU_Cache_Do(CPU_CACHE_EM6_FLUSH, addr_start, len);
#else
                                                                /* Prevent possible compiler warning.                   */
   (void)&addr_start;
   (void)&len;
#endif
}


/*
*********************************************************************************************************
*                                         CPU_DCache_RangeInv()
*
* Description : Invalidates a range of the data cache.
*
* Argument(s) : addr_start  Byte address of the beginning of the range.
*
*               len         Size in bytes of the range.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  CPU_DCache_RangeInv (void      *addr_start,
                           CPU_ADDR   len)
{
#if (defined(CPU_CFG_CACHE_MGMT_EN) && CPU_CFG_CACHE_MGMT_EN == DEF_ENABLED)
                                                                /* Invalidate range, if found, in cache                 */
    CPU_Cache_Do(CPU_CACHE_EM6_INVALIDATE, addr_start, len);
#else
                                                                /* Prevent possible compiler warning.                   */
   (void)&addr_start;
   (void)&len;
#endif
}


/*
*********************************************************************************************************
*                                             CPU_Cache_Do()
*
* Description : Does 'what' to range within addr_start and addr_start + len.
*
* Argument(s) : what        Cache action to perform:
*
*                               CPU_CACHE_EM6_INVALIDATE    Invalidate range.
*                               CPU_CACHE_EM6_FLUSH         Flush range.
*
*               addr_start  Byte address of the beginning of the range.
*
*               len         Size in bytes of the range.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  CPU_Cache_Do (CPU_INT32U   what,
                    void        *addr_start,
                    CPU_ADDR     len)
{
#if (defined(CPU_CFG_CACHE_MGMT_EN) && CPU_CFG_CACHE_MGMT_EN == DEF_ENABLED)
    CPU_INT32U  addr;
    CPU_INT32U  lines;


    addr  = ((CPU_INT32U)addr_start) & CPU_CACHE_DATA_LINE_MASK;

    lines = len / CPU_CACHE_DATA_LINE_SIZE;
    if ((len % CPU_CACHE_DATA_LINE_SIZE) != 0) {
        lines++;
    }

    while (lines > 0) {
        CPU_AR_WR(what, addr);
        addr += CPU_CACHE_DATA_LINE_SIZE;
        lines--;
    }
#else
                                                                /* Prevent possible compiler warning.                   */
   (void)&what;
   (void)&addr_start;
   (void)&len;
#endif
}
