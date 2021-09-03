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
 * @file        drv_uart.c
 *
 * @brief       This file implements uart driver for nrf5
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <board.h>
//#include <os_irq.h>
#include <os_memory.h>
#include <bus/bus.h>
#include "drv_uart.h"
#include "drv_gpio.h"

#define ENABLE_UART_EVENTS_RXDXDY (NRF_UARTE0->INTEN |= 0x00000004)
#define DISABLE_UART_EVENTS_RXDXDY (NRF_UARTE0->INTEN &= 0xFFFFFFFB)
#define DMA_RX_MAX_SIZE 0xFF

enum{
    UARTE_RECV_IDLE,
    UARTE_RECV_SART,
    UARTE_RECV_ING,
    UARTE_RECV_FINISH,
};

enum { FALSE = 0, TRUE = 1 };
typedef enum {RESET = 0, SET = !RESET} FlagStatus;

typedef enum{
    UART_RX_NO_NEED_RESTART = 0,
    UART_RX_NEED_RESTART = 1
}UART_RX_NEED_RESTART_T;

struct nrf5_uart
{
    struct os_serial_device serial_dev;
    struct nrf5_uart_info *huart;
    
    const char *device_name;
    os_uint8_t *buffer;
    os_size_t rx_length;
    os_size_t rx_size;
    os_size_t rx_buff_head;
    os_size_t tx_size;
    bool poll_send;
    int rx_state;
    bool isInit;
    
    os_list_node_t list;
};

static os_list_node_t nrf5_uart_list = OS_LIST_INIT(nrf5_uart_list);

static int nrf5_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size);

static void nrf5_uarte_evt_handler(nrfx_uarte_event_t const * p_event, void *p_context)
{
    os_base_t level;
    struct nrf5_uart *uart = (struct nrf5_uart *)p_context;

    level = os_irq_lock();

    switch (p_event->type)
    {
        case NRFX_UARTE_EVT_RX_DONE:
            uart->rx_length += uart->huart->uart.p_reg->RXD.AMOUNT;
            
            if ( uart->rx_length >= uart->rx_size || 
                (uart->rx_state == UARTE_RECV_FINISH && uart->rx_length > 0) )
            {
                uart->rx_state = UARTE_RECV_IDLE;
                os_hw_serial_isr_rxdone(&uart->serial_dev, uart->rx_length);
            }
            else if (uart->rx_state == UARTE_RECV_ING)
            {
                uint32_t rx_space_remain = uart->rx_size - uart->rx_buff_head;
                
                uint32_t rx_size = min(rx_space_remain, DMA_RX_MAX_SIZE);
                
                nrfx_uarte_rx(&uart->huart->uart, uart->buffer + uart->rx_buff_head, rx_size);
                
                uart->rx_buff_head += rx_size;
            }
            break;

        case NRFX_UARTE_EVT_ERROR:
            (void)nrfx_uarte_rx(&(uart->huart->uart), uart->buffer, 1);
            break;
        
        case NRFX_UARTE_EVT_TX_DONE:
            //if(uart->serial_dev.parent.open_flag&OS_SERIAL_FLAG_TX_NOPOLL)
            if(uart->poll_send != true)
            {
                os_hw_serial_isr_txdone(&uart->serial_dev);
            }
            uart->tx_size = 0;
            break;

        default:
            break;
    }

    os_irq_unlock(level);
}

void nrf5_uart_recfg(void)
{
    nrfx_uarte_t uart0 = NRFX_UARTE_INSTANCE(0);
    struct nrf5_uart *uart;

    os_list_for_each_entry(uart, &nrf5_uart_list, struct nrf5_uart, list){__NOP();}
    
    nrfx_uarte_config_t config = NRFX_UARTE_DEFAULT_CONFIG;

    config.baudrate = NRF_UARTE_BAUDRATE_115200;

    config.parity = NRF_UARTE_PARITY_EXCLUDED;


    config.hwfc = NRF_UARTE_HWFC_DISABLED;
    config.interrupt_priority = 4;
    config.pselrxd = 8;
    config.pseltxd = 6;
    config.p_context = (void *)uart;
    
    nrfx_uarte_init(&uart0,(nrfx_uarte_config_t const *)&config,nrf5_uarte_evt_handler);
}

static os_err_t nrf5_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    nrfx_uarte_config_t config = NRFX_UARTE_DEFAULT_CONFIG;
    struct nrf5_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);
    if(uart->huart == OS_NULL)
        return OS_ERROR;
	
    if(uart->isInit)
    {
        nrfx_uarte_uninit(&uart->huart->uart);
        uart->isInit = FALSE;
    }

    switch (cfg->baud_rate)
    {
        case BAUD_RATE_2400:
            config.baudrate = NRF_UARTE_BAUDRATE_2400;
            break;
        case BAUD_RATE_4800:
            config.baudrate = NRF_UARTE_BAUDRATE_4800;
            break;
        case BAUD_RATE_9600:
            config.baudrate = NRF_UARTE_BAUDRATE_9600;
            break;
        case BAUD_RATE_19200:
            config.baudrate = NRF_UARTE_BAUDRATE_19200;
            break;
        case BAUD_RATE_38400:
            config.baudrate = NRF_UARTE_BAUDRATE_38400;
            break;
        case BAUD_RATE_57600:
            config.baudrate = NRF_UARTE_BAUDRATE_57600;
            break;
        case BAUD_RATE_115200:
            config.baudrate = NRF_UARTE_BAUDRATE_115200;
            break;
        case BAUD_RATE_230400:
            config.baudrate = NRF_UARTE_BAUDRATE_230400;
            break;
        case BAUD_RATE_460800:
            config.baudrate = NRF_UARTE_BAUDRATE_460800;
            break;
        case BAUD_RATE_921600:
            config.baudrate = NRF_UARTE_BAUDRATE_921600;
            break;
        case BAUD_RATE_2000000:
        case BAUD_RATE_3000000:
            return -OS_EINVAL;
        default:
            config.baudrate = NRF_UARTE_BAUDRATE_115200;
            break;
    }

    if (cfg->parity == PARITY_NONE)
    {
        config.parity = NRF_UARTE_PARITY_EXCLUDED;
    }
    else
    {
        config.parity = NRF_UARTE_PARITY_INCLUDED;
    }
    
    config.hwfc = NRF_UARTE_HWFC_DISABLED;
    //config.interrupt_priority = 2;
    config.pselrxd = uart->huart->rx_pin;
    config.pseltxd = uart->huart->tx_pin;
    config.p_context = (void *)uart;
    
    nrfx_uarte_init(&uart->huart->uart,
                                     (nrfx_uarte_config_t const *)&config,
                                     nrf5_uarte_evt_handler);
    uart->isInit = TRUE;

    /* add blow code to feat user call config after device open */
    if( uart->rx_state != UARTE_RECV_IDLE ){
        uart->rx_state = UARTE_RECV_IDLE;
        //os_kprintf("manul set uart->rx_state to UARTE_RECV_IDLEUARTE_RECV_IDLE\r\n");
        nrf5_uart_start_recv(serial, serial->rx_fifo->line_buff, serial->config.rx_bufsz);
    }

    return OS_EOK;
}

static int nrf5_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int ret;
    struct nrf5_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);

    OS_ASSERT(uart != OS_NULL);
    
    uart->tx_size = size;
    uart->poll_send = false;
    ret = nrfx_uarte_tx(&uart->huart->uart,buff, size);

    return (ret == NRFX_SUCCESS ? size : 0);

}

static int nrf5_uart_stop_send(struct os_serial_device *serial)
{
    struct nrf5_uart *uart;
    OS_ASSERT(serial != OS_NULL);

    uart = (struct nrf5_uart*) serial;
    OS_ASSERT(uart != OS_NULL);

    nrfx_uarte_tx_abort(&uart->huart->uart);
    return OS_EOK;
}

static int nrf5_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct nrf5_uart *uart;
    OS_ASSERT(serial != OS_NULL);

    uart = (struct nrf5_uart*) serial;
    OS_ASSERT(uart != OS_NULL);

    if (uart->rx_state != UARTE_RECV_IDLE)
			return OS_EOK;
    
    uart->rx_length = 0;
    uart->rx_buff_head = 0;
    uart->buffer = buff;
    uart->rx_size = size;
    
    uint32_t rx_size = min(size, DMA_RX_MAX_SIZE);

    uart->rx_state = UARTE_RECV_SART;
    
    nrfx_uarte_rx(&uart->huart->uart, uart->buffer, rx_size);
    uart->rx_buff_head = rx_size;

    if (size > DMA_RX_MAX_SIZE)
    {
        rx_size = min(size - DMA_RX_MAX_SIZE, DMA_RX_MAX_SIZE);
        
        nrfx_uarte_rx(&uart->huart->uart, uart->buffer + uart->rx_buff_head, rx_size);
        uart->rx_buff_head += rx_size;
    }

    return OS_EOK;
}


static int nrf5_uart_stop_recv(struct os_serial_device *serial)
{
    struct nrf5_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    uart->rx_length = 0;
    uart->buffer = OS_NULL;
    uart->rx_state = UARTE_RECV_IDLE;
    nrfx_uarte_rx_abort(&uart->huart->uart);

    os_task_msleep(30);
    return OS_EOK;
}

static int nrf5_uart_recv_state(struct os_serial_device *serial)
{
    struct nrf5_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);
    
    OS_ASSERT(uart != OS_NULL);

    if (uart->huart->uart.p_reg->EVENTS_RXDRDY)
    {
        uart->huart->uart.p_reg->EVENTS_RXDRDY = 0;
        uart->rx_state = UARTE_RECV_ING;
    }
    else if (uart->rx_state == UARTE_RECV_ING)
    {
        uart->rx_state = UARTE_RECV_FINISH;
        nrfx_uarte_rx_abort(&uart->huart->uart);
    }

//    if (uart->rx_state == UARTE_RECV_IDLE)
//        return OS_SERIAL_FLAG_RX_IDLE;

    return 0;
}

static int nrf5_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int ret;
    int i;
    struct nrf5_uart *uart;
    uint8_t data;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);

    OS_ASSERT(uart != OS_NULL);
    uart->tx_size = 0;
    uart->poll_send = true;
	
    for (i = 0; i < size; i++)
    {
        data = buff[i];

        os_base_t level = os_irq_lock();

        nrfx_uarte_tx(&uart->huart->uart, &data, 1);

        while (!nrf_uarte_event_check(uart->huart->uart.p_reg, NRF_UARTE_EVENT_ENDTX));

        nrfx_uarte_tx_abort(&uart->huart->uart);

        os_irq_unlock(level);
    }

    ret = size;
    
    uart->tx_size = 0;

    return ret;
}

static os_err_t nrf5_uart_deinit(struct os_serial_device *serial)
{
    struct nrf5_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);
    if(uart->huart == OS_NULL)
        return OS_ERROR;
    
    if(uart->isInit)
    {
        nrfx_uarte_uninit(&uart->huart->uart);
        uart->isInit = FALSE;
    }
    
    return OS_EOK;
}


static const struct os_uart_ops nrf5_uart_ops = {
    .init    = nrf5_configure,
    .deinit       = nrf5_uart_deinit,
	
    .start_send   = nrf5_uart_start_send,
    .stop_send    = nrf5_uart_stop_send,

    .start_recv   = nrf5_uart_start_recv,
    .stop_recv    = nrf5_uart_stop_recv,
    .recv_count   = nrf5_uart_recv_state,

    .poll_send    = nrf5_uart_poll_send,   
};

int nrf5_uart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;

    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    struct nrf5_uart_info *uart_info = (struct nrf5_uart_info *)dev->info;
    struct nrf5_uart *uart = os_calloc(1, sizeof(struct nrf5_uart));

    OS_ASSERT(uart);
    uart->huart = uart_info;
    uart->isInit = FALSE;
    uart->rx_length = 0; 
    uart->rx_state = UARTE_RECV_IDLE;

    struct os_serial_device *serial = &uart->serial_dev;

    serial->ops    = &nrf5_uart_ops;
    serial->config = config;

    level = os_irq_lock();
    os_list_add_tail(&nrf5_uart_list, &uart->list);
    os_irq_unlock(level);
    
    result = os_hw_serial_register(serial, dev->name, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;

}
static struct nrf5_uart_info *console_uart = OS_NULL;

void __os_hw_console_output(char *str)
{
    if (console_uart == OS_NULL)
        return;

    while (*str)
    {
        nrfx_uarte_tx(&console_uart->uart, (os_uint8_t *)str, 1);
        str++;
    }

}

OS_DRIVER_INFO nrf5_uart_driver = {
    .name   = "uart_Type",
    .probe  = nrf5_uart_probe,
};

OS_DRIVER_DEFINE(nrf5_uart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

//static int nrf5_uart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
//{
//    if(!strcmp(dev->name,OS_CONSOLE_DEVICE_NAME))
//        console_uart = (struct nrf5_uart_info *)dev->info;
//    return OS_EOK;
//}

//OS_DRIVER_INFO nrf5_usart_early_driver = {
//    .name   = "uart_Type",
//    .probe  = nrf5_uart_early_probe,
//};

//OS_DRIVER_DEFINE(nrf5_usart_early_driver, CORE, OS_INIT_SUBLEVEL_MIDDLE);
