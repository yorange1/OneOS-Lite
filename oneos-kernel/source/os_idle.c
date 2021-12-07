#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <arch_interrupt.h>

#include "os_kernel_internal.h"

#define IDLE_TAG                    "IDLE"

static OS_TASK_STACK_DEFINE(gs_os_idle_stack, OS_IDLE_TASK_STACK_SIZE);
static os_task_t gs_os_idle_task;

static void _k_idle_task_entry(void *arg)
{
    OS_UNREFERENCE(arg);
    
    for(int i=0;i<100000;i++)
    {
        i++;
    }

    while (1)
    {
        /* TODO: */
        ;
#ifdef OS_USING_TICKLESS_LPMGR
        extern void os_low_power_manager(void);
        os_low_power_manager();
#endif
    }
}

void k_idle_task_init(void)
{
    os_err_t ret;
    
    /* Initialize idle task */
    ret = os_task_init(&gs_os_idle_task,
                       OS_IDLE_TASK_NAME,
                       _k_idle_task_entry,
                       OS_NULL,
                       OS_TASK_STACK_BEGIN_ADDR(gs_os_idle_stack),
                       OS_TASK_STACK_SIZE(gs_os_idle_stack),
                       OS_TASK_PRIORITY_MAX - 1);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why initialize idle task failed?");
    }

    /* Startup */
    ret = os_task_startup(&gs_os_idle_task);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why startup idle task failed?");
    }
    
    return;
}

