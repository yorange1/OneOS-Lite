#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BOARD_HK32F103_MINI
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M3

/* Kernel */

#define OS_CUSTOMIZED_CONFIG
#define OS_NAME_MAX 15
#define OS_TASK_PRIORITY_MAX 32
#define OS_MAIN_TASK_STACK_SIZE 2048
#define OS_IDLE_TASK_STACK_SIZE 512
#define OS_RECYCLE_TASK_STACK_SIZE 512
#define OS_USING_TIMER
#define OS_TIMER_TASK_STACK_SIZE 512
#define OS_USING_MUTEX
#define OS_USING_SEMAPHORE
#define OS_USING_EVENT
#define OS_USING_MESSAGEQUEUE
#define OS_USING_MAILBOX
#define OS_USING_SYS_HEAP
#define OS_USING_MEM_HEAP
#define OS_USING_ALG_SMALL
#define OS_MEM_ALGORITHM_MINIMIZATION
#define OS_DEBUG
#define EXCEPTION_SIMPLE_HANDLE
#define OS_TICK_PER_SECOND 100
#define OS_SCHEDULE_TIME_SLICE 10

/* Task */

#define OS_NAME_MAX_15
#define OS_TASK_PRIORITY_32
/* end of Task */

/* Timer */

/* end of Timer */

/* IPC */

/* end of IPC */

/* Memory */

/* algorithm */

#define OS_USING_ALG_FIRSTFIT
/* end of algorithm */
#define OS_USING_MEM_POOL
/* end of Memory */

/* Debug */

#define OS_USING_ASSERT
#define OS_USING_KERNEL_LOCK_CHECK
#define OS_USING_KERNEL_DEBUG
#define KLOG_GLOBAL_LEVEL_WARNING
#define KLOG_GLOBAL_LEVEL 1
#define KLOG_USING_COLOR
#define KLOG_WITH_FUNC_LINE
#define OS_USING_OVERFLOW_CHECK
/* end of Debug */
/* end of Kernel */

/* Drivers */

/* Device */

#define OS_USING_DEVICE
#define OS_DEVICE_NAME_MAX 15
/* end of Device */
#define OS_USING_FAL

/* Console */

#define OS_USING_CONSOLE
#define OS_CONSOLE_DEVICE_NAME "uart1"
#define OS_LOG_BUFF_SIZE 256
/* end of Console */

/* Serial */

#define OS_USING_SERIAL
#define OS_SERIAL_IDLE_TIMER
#define OS_SERIAL_RX_BUFSZ 64
#define OS_SERIAL_TX_BUFSZ 64
/* end of Serial */

/* Timer */

#define OS_USING_TIMER_DRIVER
#define OS_USING_CLOCKSOURCE
#define OS_USING_CLOCKEVENT
#define OS_USING_HRTIMER
#define OS_USING_HRTIMER_FOR_KERNEL_TICK

/* cortex-m hardware timer config */

#define OS_USING_SYSTICK_FOR_CLOCKSOURCE
/* end of cortex-m hardware timer config */
/* end of Timer */

/* LPMGR */

/* end of LPMGR */
#define OS_USING_PUSH_BUTTON
#define OS_USING_LED

/* ADC */

/* end of ADC */
#define OS_USING_PIN
#define OS_PIN_MAX_CHIP 1

/* HAL */

#define MANUFACTOR_HK32
#define SERIES_HK32F1
#define SOC_HK32F103VET6
#define BSP_BOOT_OPTION
#define BSP_TEXT_SECTION_ADDR 0x08000000
#define BSP_TEXT_SECTION_SIZE 0x00080000
#define BSP_DATA_SECTION_ADDR 0x20000000
#define BSP_DATA_SECTION_SIZE 0x00010000
#define BSP_USING_PIN
#define BSP_USING_USART
#define BSP_USING_USART1
#define BSP_USING_USART2
#define BSP_USING_USART3
#define BSP_USING_TIMER
#define BSP_USING_TIMER2
#define BSP_USING_TIMER3
#define BSP_USING_TIMER4
#define BSP_USING_TIMER6
#define BSP_USING_FLASH
/* end of HAL */

/* RTC */

/* end of RTC */
/* end of Drivers */

/* Components */

/* Atest */

#define OS_USING_ATEST
#define ATEST_TASK_STACK_SIZE 4096
#define ATEST_TASK_PRIORITY 20
/* end of Atest */

/* Dlog */

#define OS_USING_DLOG
#define DLOG_PRINT_LVL_I
#define DLOG_GLOBAL_PRINT_LEVEL 6
#define DLOG_COMPILE_LVL_I
#define DLOG_COMPILE_LEVEL 6
#define DLOG_USING_ISR_LOG
#define DLOG_USING_FILTER
#define DLOG_USING_ASYNC_OUTPUT
#define DLOG_ASYNC_OUTPUT_BUF_SIZE 2048
#define DLOG_ASYNC_OUTPUT_TASK_STACK_SIZE 2048
#define DLOG_ASYNC_OUTPUT_TASK_PRIORITY 30

/* Log format */

#define DLOG_WITH_FUNC_LINE
#define DLOG_USING_COLOR
#define DLOG_OUTPUT_TIME_INFO
/* end of Log format */

/* Dlog backend option */

#define DLOG_BACKEND_USING_CONSOLE
/* end of Dlog backend option */
/* end of Dlog */
#define OS_USING_NEWLIB_ADAPTER
#define OS_USING_ARMCCLIB_ADAPTER
#define OS_USING_LIBC_ADAPTER

/* Molink */

/* end of Molink */

/* OTA */

/* CMIOT Fota */

/* end of CMIOT Fota */
/* end of OTA */

/* Shell */

#define OS_USING_SHELL
#define SHELL_TASK_NAME "shell"
#define SHELL_TASK_PRIORITY 20
#define SHELL_TASK_STACK_SIZE 2048
#define SHELL_USING_DESCRIPTION
#define SHELL_CMD_SIZE 80
#define SHELL_PROMPT_SIZE 256
#define SHELL_ARG_MAX 10
/* end of Shell */

/* Socket */

/* end of Socket */

/* Cplusplus */

/* end of Cplusplus */
/* end of Components */

/* Thirdparty */

/* Submodule */

#define OS_USING_SUBMODULE
/* end of Submodule */
/* end of Thirdparty */

#endif /* __ONEOS_CONFIG_H__ */

