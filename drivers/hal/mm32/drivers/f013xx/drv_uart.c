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
 * @brief       This file implements uart driver for mm32
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>
#include "string.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.uart"
#include <drv_log.h>

#include "mm32_hal.h"
#include "drv_uart.h"
#include "mm32_it.h"

typedef struct mm32_uart_xfer
{
    os_size_t   count;
    os_uint8_t *buffer;
    os_size_t   size;
    os_uint8_t  status;
}mm32_uart_xfer_t;

typedef struct mm32_uart
{
    struct os_serial_device serial;
    mm32_uart_info_t *uart_info;
    mm32_uart_xfer_t tx_info;
    mm32_uart_xfer_t rx_info;
    os_list_node_t list;
}mm32_uart_t;

static os_list_node_t mm32_uart_list = OS_LIST_INIT(mm32_uart_list);

void HAL_UART_IRQHandler(UART_TypeDef *huart)
{
    struct mm32_uart *uart;
    os_base_t level;

    os_list_for_each_entry(uart, &mm32_uart_list, struct mm32_uart, list)
    {
        if (uart->uart_info->huart != huart)
            continue;
    
        level = os_irq_lock();
        
        if (RESET != UART_GetITStatus(huart, UART_ISR_RX_INTF))
        {
            *(uart->rx_info.buffer + uart->rx_info.count) = UART_ReceiveData(uart->uart_info->huart);
            UART_ClearITPendingBit(huart, UART_ISR_RX_INTF);
            uart->rx_info.count++;
            if(uart->rx_info.count == uart->rx_info.size)
            {
                UART_ITConfig(uart->uart_info->huart, UART_ISR_RX_INTF, DISABLE);
                os_hw_serial_isr_rxdone((struct os_serial_device *)uart, uart->rx_info.count);
                uart->rx_info.status = RESET;
            }
        }
        os_irq_unlock(level);
    }
}

void hal_uart_dmatx_callback(void *handle, irqn_type_flag type_flag)
{
    os_base_t level = 0;
    struct mm32_uart *uart = OS_NULL;

    os_list_for_each_entry(uart, &mm32_uart_list, struct mm32_uart, list)
    {
        if (uart->uart_info->huart != (UART_TypeDef *)handle)
            continue;

        level = os_irq_lock();
        if(type_flag == TYPE_DMA_IT_TC)
        {
            DMA_Cmd(uart->uart_info->dma_rx_channel, DISABLE);
            os_hw_serial_isr_txdone((struct os_serial_device *)uart);
            uart->tx_info.status = RESET;
        }
        os_irq_unlock(level);
    }
}

void hal_uart_dmarx_callback(void *handle, irqn_type_flag type_flag)
{
    os_base_t level = 0;
    struct mm32_uart *uart = OS_NULL;

    os_list_for_each_entry(uart, &mm32_uart_list, struct mm32_uart, list)
    {
        if (uart->uart_info->huart != (UART_TypeDef *)handle)
            continue;

        level = os_irq_lock();
        if(type_flag == TYPE_DMA_IT_TC)
        {
            DMA_Cmd(uart->uart_info->dma_rx_channel, DISABLE);
            uart->rx_info.count = uart->rx_info.size;
            os_hw_serial_isr_rxdone((struct os_serial_device *)uart, uart->rx_info.count);
            uart->rx_info.status = RESET;
        }
        os_irq_unlock(level);
    }
}

static void mm32_uart_init(mm32_uart_info_t *uart_info)
{
    if ((uart_info->tx_pin_port == GPIOA) || (uart_info->tx_pin_port == GPIOA))
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    if ((uart_info->tx_pin_port == GPIOB) || (uart_info->tx_pin_port == GPIOB))
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    if ((uart_info->tx_pin_port == GPIOC) || (uart_info->tx_pin_port == GPIOC))
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
    if ((uart_info->tx_pin_port == GPIOD) || (uart_info->tx_pin_port == GPIOD))
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);

    if (uart_info->huart == UART1)
        RCC_APB2PeriphClockCmd(uart_info->uart_clk, ENABLE);
    else
        RCC_APB1PeriphClockCmd(uart_info->uart_clk, ENABLE);
    
    GPIO_Init(uart_info->tx_pin_port, &uart_info->tx_pin_info);
    GPIO_Init(uart_info->rx_pin_port, &uart_info->rx_pin_info);
    NVIC_Init(&uart_info->uart_nvic_info);
    UART_Init(uart_info->huart, &uart_info->uart_info);
    UART_Cmd(uart_info->huart, ENABLE);

    GPIO_PinAFConfig(uart_info->tx_pin_port, uart_info->tx_pin_source, uart_info->pin_af);
    GPIO_PinAFConfig(uart_info->rx_pin_port, uart_info->rx_pin_source, uart_info->pin_af);
    if (uart_info->dma_tx_channel != 0)
        dma_info_table_config(uart_info->dma_tx_channel, uart_info->huart, hal_uart_dmarx_callback);
    if (uart_info->dma_rx_channel != 0)
        dma_info_table_config(uart_info->dma_rx_channel, uart_info->huart, hal_uart_dmatx_callback);
}

static os_err_t mm32_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct mm32_uart *uart;
    os_uint32_t        data_bits;
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial);

    uart->uart_info->uart_info.UART_BaudRate = cfg->baud_rate;

    switch (cfg->stop_bits)
    {
        case STOP_BITS_1:
            uart->uart_info->uart_info.UART_StopBits = UART_StopBits_1;
            break;
        case STOP_BITS_2:
            uart->uart_info->uart_info.UART_StopBits = UART_StopBits_2;
            break;
        default:
            return OS_EINVAL;
    }
    switch (cfg->parity)
    {
        case PARITY_NONE:
            uart->uart_info->uart_info.UART_Parity = UART_Parity_No;
            data_bits                = cfg->data_bits;
            break;
        default:
            return OS_EINVAL;
    }

    switch (data_bits)
    {
        case DATA_BITS_8:
            uart->uart_info->uart_info.UART_WordLength = UART_WordLength_8b;
            break;    
        default:
            return OS_EINVAL;
    }

    UART_Init(uart->uart_info->huart, &uart->uart_info->uart_info);
    UART_ITConfig(uart->uart_info->huart, UART_IT_RXIEN, ENABLE);
    UART_Cmd(uart->uart_info->huart, ENABLE);
    NVIC_EnableIRQ((IRQn_Type)uart->uart_info->uart_nvic_info.NVIC_IRQChannel);

    return OS_EOK;
    
}

void mm32_uart_dma_transfer(mm32_uart_info_t *uart_info, DMA_InitTypeDef *dma_info)
{
    if (dma_info->DMA_DIR == DMA_DIR_PeripheralDST)
    {
        RCC_AHBPeriphClockCmd(uart_info->dma_tx_clk, ENABLE);
        DMA_DeInit(uart_info->dma_tx_channel);
        DMA_Init(uart_info->dma_tx_channel, dma_info);
        DMA_ITConfig(uart_info->dma_tx_channel, DMA_IT_TC, ENABLE);
        UART_DMACmd(uart_info->huart, UART_DMAReq_EN, ENABLE);
        DMA_Cmd(uart_info->dma_tx_channel, ENABLE);
        NVIC_Init(&uart_info->dma_tx_nvic_info);
    }
    else if (dma_info->DMA_DIR == DMA_DIR_PeripheralSRC)
    {
        RCC_AHBPeriphClockCmd(uart_info->dma_rx_clk, ENABLE);
        DMA_DeInit(uart_info->dma_rx_channel);
        DMA_Init(uart_info->dma_rx_channel, dma_info);
        DMA_ITConfig(uart_info->dma_rx_channel, DMA_IT_TC, ENABLE);
        UART_DMACmd(uart_info->huart, UART_DMAReq_EN, ENABLE);
        DMA_Cmd(uart_info->dma_rx_channel, ENABLE);
        NVIC_Init(&uart_info->dma_rx_nvic_info);
    }
}

static int mm32_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct mm32_uart *uart;
    DMA_InitTypeDef DMA_InitStruct;

    OS_ASSERT(serial != OS_NULL);
    uart = os_container_of(serial, struct mm32_uart, serial);
    OS_ASSERT(uart != OS_NULL);

    DMA_InitStruct = uart->uart_info->dma_info;
    
    uart->tx_info.count = 0;
    uart->tx_info.buffer = (os_uint8_t *)buff;
    uart->tx_info.size = size;
    uart->tx_info.status = SET;
    if (uart->uart_info->dma_rx_channel != 0)
    {
        DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
        DMA_InitStruct.DMA_PeripheralBaseAddr = (u32)&uart->uart_info->huart->TDR;
        DMA_InitStruct.DMA_MemoryBaseAddr = (u32)buff;
        DMA_InitStruct.DMA_BufferSize = size;
        mm32_uart_dma_transfer(uart->uart_info, &DMA_InitStruct);
    }
    
    return size;
}

static int mm32_uart_stop_send(struct os_serial_device *serial)
{
    struct mm32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = os_container_of(serial, struct mm32_uart, serial);
    OS_ASSERT(uart != OS_NULL);
    
    if (uart->uart_info->dma_rx_channel != 0)
    {
        DMA_Cmd(uart->uart_info->dma_tx_channel, DISABLE);
        UART_DMACmd(uart->uart_info->huart, UART_DMAReq_EN, DISABLE);
        DMA_ITConfig(uart->uart_info->dma_tx_channel, DMA_IT_TC, DISABLE);
    }
    uart->tx_info.status = RESET;
    return 0;
}

static int mm32_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct mm32_uart *uart;
    DMA_InitTypeDef DMA_InitStruct;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial);

    OS_ASSERT(uart != OS_NULL);

    DMA_InitStruct = uart->uart_info->dma_info;

    uart->rx_info.count = 0;
    uart->rx_info.buffer = buff;
    uart->rx_info.size = size;
    uart->rx_info.status = SET;
    if (uart->uart_info->dma_rx_channel != 0)
    {
        DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_InitStruct.DMA_PeripheralBaseAddr = (u32)&uart->uart_info->huart->RDR;
        DMA_InitStruct.DMA_MemoryBaseAddr = (u32)buff;
        DMA_InitStruct.DMA_BufferSize = size;
        mm32_uart_dma_transfer(uart->uart_info, &DMA_InitStruct);
    }
    else
    {
        UART_ITConfig(uart->uart_info->huart, UART_IT_RXIEN, ENABLE);
    }

    return OS_EOK;
}

static int mm32_uart_stop_recv(struct os_serial_device *serial)
{
    struct mm32_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial);

    OS_ASSERT(uart != OS_NULL);

    uart->rx_info.count = 0;
    uart->rx_info.buffer = OS_NULL;
    if (uart->uart_info->dma_rx_channel != 0)
    {
        DMA_Cmd(uart->uart_info->dma_rx_channel, DISABLE);
        UART_DMACmd(uart->uart_info->huart, UART_DMAReq_EN, DISABLE);
        DMA_ITConfig(uart->uart_info->dma_rx_channel, DMA_IT_TC, DISABLE);
    }
    else
    {
        UART_ITConfig(uart->uart_info->huart, UART_IT_RXIEN, DISABLE);
    }
    uart->rx_info.status = RESET;
    return OS_EOK;
}

static OS_UNUSED int mm32_uart_recv_count(struct os_serial_device *serial)
{
    int state;
    struct mm32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = os_container_of(serial, struct mm32_uart, serial);
    OS_ASSERT(uart != OS_NULL);
    if (uart->uart_info->dma_rx_channel != 0)
        uart->rx_info.count = uart->rx_info.size - DMA_GetCurrDataCounter(uart->uart_info->dma_rx_channel);
    else
        state = uart->rx_info.count;

    return state;
}

static int mm32_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct mm32_uart *uart;	

    int i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();	
        UART_SendData(uart->uart_info->huart, *(buff + i));
        while(!UART_GetFlagStatus(uart->uart_info->huart, UART_CSR_TXC));
        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops mm32_uart_ops = 
{
    .init    = mm32_configure,

    .start_send   = mm32_uart_start_send,
    .stop_send    = mm32_uart_stop_send,

    .start_recv   = mm32_uart_start_recv,
    .stop_recv    = mm32_uart_stop_recv,
#ifdef OS_SERIAL_IDLE_TIMER
    .recv_count   = mm32_uart_recv_count,
#endif
    .poll_send    = mm32_uart_poll_send,
};

static void mm32_uart_parse_configs(struct mm32_uart *uart)
{
    struct os_serial_device *serial = &uart->serial;
    
    serial->config.baud_rate = uart->uart_info->uart_info.UART_BaudRate;

    switch (uart->uart_info->uart_info.UART_StopBits)
    {
    case UART_StopBits_1:
        serial->config.stop_bits = STOP_BITS_1;
        break;
    case UART_StopBits_2:
        serial->config.stop_bits = STOP_BITS_2;
        break;
    }
    switch (uart->uart_info->uart_info.UART_Mode)
    {
    case UART_Parity_No:
        serial->config.parity   = PARITY_NONE;
        break;
    case UART_Parity_Odd:
        serial->config.parity   = PARITY_ODD;
        break;
    case UART_Parity_Even:
        serial->config.parity   = PARITY_EVEN;
        break;
    }

    switch (uart->uart_info->uart_info.UART_WordLength)
    {
    case UART_WordLength_8b:
        serial->config.data_bits = DATA_BITS_8;
        break;
    }
}

static int mm32_uart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    os_err_t    result  = 0;
    os_base_t   level = 0;
    struct os_serial_device *serial;
    struct mm32_uart *uart = os_calloc(1, sizeof(struct mm32_uart));
    OS_ASSERT(uart);
    
    uart->uart_info = (struct mm32_uart_info *)dev->info;

    mm32_uart_init(uart->uart_info);
    
    serial = &uart->serial;
    serial->ops    = &mm32_uart_ops;
    serial->config = config;				
    mm32_uart_parse_configs(uart);
    
    level = os_irq_lock();
    os_list_add_tail(&mm32_uart_list, &uart->list);
    os_irq_unlock(level);

    result = os_hw_serial_register(serial, dev->name, NULL);    

    OS_ASSERT(result == OS_EOK);

    return result;
}

static UART_TypeDef * console_uart = 0;
void __os_hw_console_output(char *str)
{
    if (console_uart == 0)
        return;

    while (*str)
    {
        UART_SendData(console_uart, *str);
        while(!UART_GetFlagStatus(console_uart, UART_CSR_TXC));
        str++;
    }
}


OS_DRIVER_INFO mm32_uart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = mm32_uart_probe,
};

OS_DRIVER_DEFINE(mm32_uart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

static int mm32_uart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    mm32_uart_info_t *uart_info = (mm32_uart_info_t *)dev->info;
    if(!strcmp(dev->name, OS_CONSOLE_DEVICE_NAME))
    {
        mm32_uart_init(uart_info);
        console_uart = (UART_TypeDef *)uart_info->huart;
    }
    return OS_EOK;
}

OS_DRIVER_INFO mm32_uart_early_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = mm32_uart_early_probe,
};

OS_DRIVER_DEFINE(mm32_uart_early_driver, CORE, OS_INIT_SUBLEVEL_MIDDLE);

