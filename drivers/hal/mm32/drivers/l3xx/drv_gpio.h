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
 * @file        drv_gpio.h
 *
 * @brief       This file provides struct/macro declaration and functions declaration for mm32 gpio driver.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-11   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include <drv_common.h>
#include "board.h"

enum GPIO_PORT_INDEX
{
    GPIO_INDEX_A = 0,
    GPIO_INDEX_B,
    GPIO_INDEX_C,
    GPIO_INDEX_D,
    GPIO_INDEX_E,
    GPIO_INDEX_F,
    GPIO_INDEX_G,
    GPIO_INDEX_H,
    GPIO_INDEX_I,
    GPIO_INDEX_J,
    GPIO_INDEX_K,
    GPIO_INDEX_L,
    GPIO_INDEX_M,
    GPIO_INDEX_N,
    GPIO_INDEX_O,
    GPIO_INDEX_P,
    GPIO_INDEX_Q,
    GPIO_INDEX_R,
    GPIO_INDEX_S,
    GPIO_INDEX_T,
    GPIO_INDEX_U,
    GPIO_INDEX_V,
    GPIO_INDEX_W,
    GPIO_INDEX_X,
    GPIO_INDEX_Y,
    GPIO_INDEX_Z,
    GPIO_INDEX_MAX
};

#define GPIO_PIN_PER_PORT           (16)

#define __PORT_INDEX(pin)           (pin / GPIO_PIN_PER_PORT)
#define __PIN_INDEX(pin)            (pin % GPIO_PIN_PER_PORT)

#define GET_PIN(PORTx, PIN)         (((GPIO_INDEX_##PORTx - GPIO_INDEX_A) * GPIO_PIN_PER_PORT) + PIN)

struct pin_pull_state
{
    os_int8_t pull_state;
};

typedef struct gpio_port_info
{
    GPIO_TypeDef           *port;
    os_uint32_t             periph;
    os_uint8_t              port_source;
    struct pin_pull_state   pin_pulls[GPIO_PIN_PER_PORT];
}gpio_port_info_t;

typedef struct pin_irq_table
{
    os_uint16_t             pinbit;
    os_uint8_t              pin_source;
    os_uint32_t             irqbit;
    IRQn_Type               irqno;
}pin_irq_table_t;

typedef void (*RCC_PeriphClockCmd)(u32 ahb_periph, FunctionalState state);
typedef void (*GPIO_EXTILineCFG)(uint8_t GPIO_PortSource, uint8_t GPIO_PinSource);
typedef struct
{
    RCC_PeriphClockCmd      gpio_rcc_clkcmd;
    RCC_PeriphClockCmd      exti_rcc_clkcmd;
    os_uint32_t             exit_periph;
    GPIO_EXTILineCFG        exti_lineconfig;
    os_uint8_t              port_info_tab_size;
    os_uint8_t              irq_tab_size;
    os_uint8_t              irq_hdr_tab_size;
    os_uint8_t              gpio_pin_max;
    gpio_port_info_t       *info_tab;
    const pin_irq_table_t  *irq_tab;
    struct os_pin_irq_hdr  *irq_hdr_tab;
}mm32_gpio_info_t;

void pin_irq_hdr(int irq_index);
int os_hw_pin_init(void);

#endif /* __DRV_GPIO_H__ */
