#ifndef __ARCH_TASK_H__
#define __ARCH_TASK_H__

#include <oneos_config.h>
#include <os_types.h>

#ifdef __cplusplus
    extern "C" {
#endif

#define OS_ARCH_STACK_ALIGN_SIZE   8

#ifdef ARCH_ARM_CORTEX_M33_TZ
#include <arch_task_tz.h>
#else
#include <arch_task_ntz.h>
#endif

extern void       *os_hw_stack_init(void        (*task_entry)(void *arg),
                                    void         *arg,
                                    void         *stack_begin,
                                    os_uint32_t   stack_size,
                                    void        (*task_exit)(void));

extern os_uint32_t os_hw_stack_max_used(void *stack_begin, os_uint32_t stack_size);

extern void        os_first_task_start(void);
extern void        os_task_switch(void);

#ifdef __cplusplus
    }
#endif

#endif /* __ARCH_TASK_H__ */

