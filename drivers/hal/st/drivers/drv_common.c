/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        drv_common.c
 *
 * @brief       This file provides systick time init/IRQ and board init functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <arch_interrupt.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_memory.h>
#include <dma/dma.h>
#include "drv_common.h"
#include "board.h"
#include "drv_gpio.h"
#include "drv_sdram.h"

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#include <timer/clocksource_cortexm.h>
#endif

#include <timer/hrtimer.h>

#if !defined(OS_USING_SYSTICK_FOR_KERNEL_TICK) && !defined(OS_USING_HRTIMER_FOR_KERNEL_TICK)
#error:OS_USING_SYSTICK_FOR_KERNEL_TICK or OS_USING_HRTIMER_FOR_KERNEL_TICK must define
#endif

static volatile os_bool_t hardware_init_done = OS_FALSE;

static os_uint32_t mult_systick2msec = 1;
static os_uint32_t shift_systick2msec = 0;

extern __IO uint32_t uwTick;

uint32_t HAL_GetTick(void)
{
    if (hardware_init_done)
    {
        return (os_uint64_t)uwTick * mult_systick2msec >> shift_systick2msec;
    }
    else
    {
        return uwTick;
    }
}

void os_tick_handler(void)
{
    uwTick++;
    
    os_tick_increase();
    
#ifdef OS_USING_CLOCKSOURCE
    os_clocksource_update();
#endif
}

#ifdef OS_USING_SYSTICK_FOR_KERNEL_TICK

void HAL_IncTick(void)
{
    uwTick++;
    os_tick_handler();
}

static void cortexm_systick_kernel_tick_init(void)
{
    /* systick for kernel tick */
    SysTick->LOAD  = SystemCoreClock / OS_TICK_PER_SECOND;      /* set reload register */
    SysTick->VAL   = 0UL;                                       /* Load the systick Counter Value */
    SysTick->CTRL |= 3;
}

#endif

#ifdef OS_USING_SYSTICK_FOR_CLOCKEVENT

void HAL_IncTick(void)
{
    uwTick++;
    
    if (hardware_init_done)
    {
        cortexm_systick_clockevent_isr();
    }
}

#endif

void _Error_Handler(char *s, int num)
{
    volatile int loop = 1;
    while (loop);
}

int hardware_init(void);

/**
 ***********************************************************************************************************************
 * @brief           This function will initial STM32 board.
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
static os_err_t os_hw_board_init(void)
{
#ifdef BSP_TEXT_SECTION_ADDR
#ifdef SERIES_STM32F0
    memcpy((void*)0x20000000, (void*)BSP_TEXT_SECTION_ADDR, 0xBC);
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_SYSCFG_REMAPMEMORY_SRAM();
#else
    SCB->VTOR = BSP_TEXT_SECTION_ADDR;
#endif
#endif

#ifdef SCB_EnableICache
    /* Enable I-Cache---------------------------------------------------------*/
    SCB_EnableICache();
#endif

#ifdef SCB_EnableDCache
    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();
#endif

    hardware_init_done = OS_FALSE;

    os_irq_enable();

    hardware_init();

    HAL_SuspendTick();

#ifdef HAL_SDRAM_MODULE_ENABLED
    SDRAM_Init();
#endif

    /* Heap initialization */
#if defined(OS_USING_SYS_HEAP)
    if((os_size_t)HEAP_END > (os_size_t)HEAP_BEGIN)
    {
        os_sys_heap_init();
        os_sys_heap_add((void *)HEAP_BEGIN, (os_size_t)HEAP_END - (os_size_t)HEAP_BEGIN, OS_MEM_ALG_DEFAULT);
    }
#endif

#ifdef OS_USING_DMA
    os_dma_mem_init();
#endif

    return OS_EOK;
}
OS_CORE_INIT(os_hw_board_init, OS_INIT_SUBLEVEL_HIGH);

void cortexm_systick_init(void)
{
#ifdef OS_USING_SYSTICK_FOR_KERNEL_TICK
    cortexm_systick_kernel_tick_init();
#elif defined(OS_USING_SYSTICK_FOR_CLOCKSOURCE)
    cortexm_systick_clocksource_init();
#elif defined(OS_USING_SYSTICK_FOR_CLOCKEVENT)
    cortexm_systick_clockevent_init();
#endif
}

static os_err_t board_post_init(void)
{
#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

#if defined(OS_USING_DWT_FOR_CLOCKSOURCE) && defined(DWT)
    cortexm_dwt_init();
#endif

    calc_mult_shift(&mult_systick2msec, &shift_systick2msec, OS_TICK_PER_SECOND, 1000, 1);

    cortexm_systick_init();

    hardware_init_done = OS_TRUE;

    return OS_EOK;
}

OS_POSTCORE_INIT(board_post_init, OS_INIT_SUBLEVEL_LOW);

void os_hw_cpu_reset(void)
{
    HAL_NVIC_SystemReset();
}
    
