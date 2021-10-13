#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BOARD_HC32L1
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M0

/* Kernel */

#define OS_MINIMAL_CONFIG
#define OS_NAME_MAX 0
#define OS_TASK_PRIORITY_MAX 8
#define OS_MAIN_TASK_STACK_SIZE 1300
#define OS_IDLE_TASK_STACK_SIZE 128
#define OS_RECYCLE_TASK_STACK_SIZE 128
#define OS_USING_TIMER
#define OS_TIMER_TASK_STACK_SIZE 256
#define OS_USING_SEMAPHORE
#define OS_USING_MESSAGEQUEUE
#define OS_USING_SYS_HEAP
#define OS_USING_MEM_HEAP
#define OS_USING_ALG_SMALL
#define OS_MEM_ALGORITHM_MINIMIZATION
#define OS_DEBUG
#define EXCEPTION_SIMPLE_HANDLE
#define OS_TICK_PER_SECOND 100
#define OS_SCHEDULE_TIME_SLICE 10
/* end of Kernel */

/* Drivers */

/* Device */

#define OS_USING_DEVICE
#define OS_DEVICE_NAME_MAX 15
#define OS_USING_DRIVER_SIMPLE
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
#define OS_SERIAL_RX_BUFSZ 512
#define OS_SERIAL_TX_BUFSZ 64
/* end of Serial */

/* Timer */

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

#define MANUFACTOR_HC32
#define SERIES_HC32L19
#define SOC_HC32L19X
#define BSP_BOOT_OPTION
#define BSP_TEXT_SECTION_ADDR 0x00000000
#define BSP_TEXT_SECTION_SIZE 0x00040000
#define BSP_DATA_SECTION_ADDR 0x20000000
#define BSP_DATA_SECTION_SIZE 0x00008000
#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART1
#define BSP_USING_ON_CHIP_FLASH
/* end of HAL */

/* RTC */

/* end of RTC */
/* end of Drivers */

/* Components */

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

