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
 * @file        drv_uart.h
 *
 * @brief       This file implements uart driver for mm32
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_types.h>
#include "mm32_hal.h"

typedef struct mm32_uart_info
{
    UART_TypeDef           *huart;
    os_uint32_t             uart_clk;
    GPIO_InitTypeDef        tx_pin_info;
    os_uint8_t              tx_pin_source;
    GPIO_TypeDef           *tx_pin_port;
    GPIO_InitTypeDef        rx_pin_info;
    os_uint8_t              rx_pin_source;
    GPIO_TypeDef           *rx_pin_port;
    os_uint8_t              pin_af;
    UART_InitTypeDef        uart_info;
    NVIC_InitTypeDef        uart_nvic_info;
    DMA_InitTypeDef         dma_info;
    os_uint32_t             dma_tx_clk;
    DMA_Channel_TypeDef    *dma_tx_channel;
    NVIC_InitTypeDef        dma_tx_nvic_info;
    os_uint32_t             dma_rx_clk;
    DMA_Channel_TypeDef    *dma_rx_channel;
    NVIC_InitTypeDef        dma_rx_nvic_info;
}mm32_uart_info_t;
