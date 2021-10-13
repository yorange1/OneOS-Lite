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
#define OS_USING_MUTEX
#define OS_USING_SEMAPHORE
#define OS_USING_EVENT
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
#define OS_LOG_BUFF_SIZE 128
/* end of Console */

/* Serial */

#define OS_USING_SERIAL
#define OS_SERIAL_IDLE_TIMER
#define OS_SERIAL_RX_BUFSZ 16
#define OS_SERIAL_TX_BUFSZ 16
/* end of Serial */

/* Timer */

/* end of Timer */

/* LPMGR */

/* end of LPMGR */
#define OS_USING_PUSH_BUTTON
#define OS_USING_LED

/* ADC */

#define OS_USING_ADC
/* end of ADC */
#define OS_USING_PIN
#define OS_PIN_MAX_CHIP 1

/* HAL */

#define MANUFACTOR_HC32
#define SERIES_HC32L17
#define SOC_HC32L17X
#define BSP_BOOT_OPTION
#define BSP_TEXT_SECTION_ADDR 0x00002800
#define BSP_TEXT_SECTION_SIZE 0x00020000
#define BSP_DATA_SECTION_ADDR 0x20000000
#define BSP_DATA_SECTION_SIZE 0x00004000
#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART0
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

#define NET_USING_MOLINK
#define MOLINK_USING_NETCONN_OPS
#define MOLINK_USING_TCP
#define MOLINK_USING_UDP
#define MOLINK_USING_DNS
#define MOLINK_USING_SOCKETS_OPS
#define MOLINK_USING_IP
#define MOLINK_USING_IPV4
#define MOLINK_PLATFORM_MCU
#define MOLINK_USING_MO_LIB

/* Easylog */

/* end of Easylog */

/* Modules */

#define MOLINK_USING_M5311
#define MOLINK_USING_MULTI_MODULES
#define M5311_USING_NETCONN_OPS
#define M5311_USING_TCP
#define M5311_USING_UDP
#define M5311_USING_DNS
#define M5311_USING_SOCKETS_OPS

/* 4G Cat1 */

/* end of 4G Cat1 */

/* 4G Cat4 */

/* end of 4G Cat4 */

/* NB-IOT */

#define MOLINK_MULTI_USING_M5311

/* Config */

/* end of Config */
/* end of NB-IOT */

/* WiFi */

/* end of WiFi */
/* end of Modules */

/* Parser */

#define MOLINK_USING_PARSER
#define AT_PARSER_TASK_STACK_SIZE 512
#define AT_PARSER_PRINT_RAW
/* end of Parser */
/* end of Molink */

/* OTA */

/* CMIOT Fota */

#define FOTA_USING_CMIOT
#define CMIOT_NETWORKING_MODE_COAP
#define CMIOT_DEFAULT_NETWORK_PROTOCOL 1
#define CMIOT_ALGORITHM_LUSUN
#define CMIOT_FOTA_ALGORITHM 0
#define CMIOT_FOTA_WOSUN_VERSION "IOT4.0_R42641"
#define CMIOT_FOTA_LUSUN_VERSION "IOT5.0_LUSUN11_R50426"
#define CMIOT_SEGMENT_SIZE_INDEX_START 0
#define CMIOT_DEFAULT_SEGMENT_SIZE_INDEX 3
#define CMIOT_FOTA_SERVICE_OEM "LUOSY"
#define CMIOT_FOTA_SERVICE_MODEL "L496"
#define CMIOT_FOTA_SERVICE_PRODUCT_ID "1618920533"
#define CMIOT_FOTA_SERVICE_PRODUCT_SEC "fe225e8b0f6745cab03df5bd439e7b9f"
#define CMIOT_FOTA_SERVICE_DEVICE_TYPE "box"
#define CMIOT_FOTA_SERVICE_PLATFORM "STM32L4"
#define CMIOT_FIRMWARE_VERSION "1.1"
#define CMIOT_FOTA_OS_VERSION "ONEOS_V1.0"
/* end of CMIOT Fota */
/* end of OTA */

/* Shell */

/* end of Shell */

/* Socket */

#define NET_USING_BSD
#define BSD_USING_MOLINK
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

