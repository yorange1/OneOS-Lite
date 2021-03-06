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
 * @file        drv_usart.h
 *
 * @brief        This file provides functions declaration for usart driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include "gd32e50x.h"
#include "gd32e50x_misc.h"
#include <board.h>
//#include <os_irq.h>
#include <os_memory.h>
#include <bus/bus.h>

#include "gd32e50x_usart.h"
#include <gd32e50x.h>


#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"

#define COMn                             2U

#define EVAL_COM0                        USART0
#define EVAL_COM0_CLK                    RCU_USART0
#define EVAL_COM0_TX_PIN                 GPIO_PIN_9
#define EVAL_COM0_RX_PIN                 GPIO_PIN_10
#define EVAL_COM0_GPIO_PORT              GPIOA
#define EVAL_COM0_GPIO_CLK               RCU_GPIOA
#define EVAL_COM0_GPIO_AF_IDX            0
#define EVAL_COM0_BAUDRATE               115200U

#define EVAL_COM1                        USART1
#define EVAL_COM1_CLK                    RCU_USART1
#define EVAL_COM1_TX_PIN                 GPIO_PIN_2
#define EVAL_COM1_RX_PIN                 GPIO_PIN_3
#define EVAL_COM1_GPIO_PORT              GPIOA
#define EVAL_COM1_GPIO_CLK               RCU_GPIOA
#define EVAL_COM1_GPIO_AF_IDX            0
#define EVAL_COM1_BAUDRATE               115200U




struct gd32_usart_info {
    os_uint32_t husart;
    os_uint32_t usart_clk;
    os_uint32_t baud_rate;
    os_uint32_t irq;
    os_uint32_t tx_pin;
    os_uint32_t rx_pin;
    os_uint32_t pin_port;
    os_uint32_t pin_clk;
    os_uint32_t gpio_af_idx;
};

int os_hw_usart_init(void);

#endif /* __DRV_USART_H__ */

/******************* end of file *******************/

