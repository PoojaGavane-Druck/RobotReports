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
*                                             Altera Nios II
*
* Filename : cpu_cache_niosII.c
* Version  : V1.31.05
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define    MICRIUM_SOURCE
#define    NIOSII_CACHE_MODULE
#include  <cpu_cache.h>

#include  <system.h>
#include  <sys/alt_cache.h>


/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/


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


/*
*********************************************************************************************************
*                                       LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            CPU_Cache_Init()
*
* Description : Initializes the data and instruction caches of the Nios II.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : The Nios-II cache(s) are initialized before main() is called by the Altera HAL.
*********************************************************************************************************
*/

void  CPU_Cache_Init (void)
{

}


/*
*********************************************************************************************************
*                                        CPU_DCache_RangeFlush()
*
* Description : Flushes a range of the data cache.
*
* Argument(s) : addr_start      Byte address of the beginning of the range.
*
*               len             Size in bytes of the range.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  CPU_DCache_RangeFlush (void      *addr_start,
                             CPU_ADDR   len)
{
    alt_dcache_flush(addr_start, len);
}


/*
*********************************************************************************************************
*                                         CPU_DCache_RangeInv()
*
* Description : Invalidates a range of the data cache.
*
* Argument(s) : addr_start      Byte address of the beginning of the range.
*
*               len             Size in bytes of the range.
*
* Return(s)   : none.
*
* Note(s)     : The Nios-II CPU doesn't support clearing the cache in two steps, i.e. invalidate then
*               flush. The flush instruction invalidates the cache line before flushing it.
*********************************************************************************************************
*/

void  CPU_DCache_RangeInv (void      *addr_start,
                           CPU_ADDR   len)
{
   (void)&addr_start;                                           /* Prevent possible 'variable unused' warning.          */
   (void)&len;                                                  /* Prevent possible 'variable unused' warning.          */
}

