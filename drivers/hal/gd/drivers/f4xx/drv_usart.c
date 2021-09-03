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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for gd32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <board.h>

#include <os_memory.h>
#include <bus/bus.h>
#include "drv_usart.h"
#include "drv_gpio.h"

struct gd32_usart
{
    struct os_serial_device serial_dev;

    os_uint8_t *buffer;
    const char *device_name;

    uint32_t husart;
    os_ubase_t usart_base;
    os_size_t count;
    os_size_t size;
    os_int32_t state;
    
    os_list_node_t list;
};

static os_err_t gd32_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{

    return OS_EOK;
}

static int gd32_usart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    return OS_EOK;
}

static int gd32_usart_stop_send(struct os_serial_device *serial)
{
    return OS_EOK;
}

static int gd32_usart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct gd32_usart *usart;

    OS_ASSERT(serial != OS_NULL);

    usart = os_container_of(serial, struct gd32_usart, serial_dev);    

    OS_ASSERT(usart != OS_NULL);

    usart->count = 0;
    usart->buffer = buff;
    usart->size = size;
    usart->state = SET;

    usart_interrupt_enable(usart->husart, USART_INT_RBNE);
    usart_interrupt_enable(usart->husart, USART_INT_IDLE);

    return OS_EOK;
}



static int gd32_usart_stop_recv(struct os_serial_device *serial)
{
    struct gd32_usart *usart;

    OS_ASSERT(serial != OS_NULL);

    usart = os_container_of(serial, struct gd32_usart, serial_dev);

    OS_ASSERT(usart != OS_NULL);

    usart->count = 0;
    usart->buffer = OS_NULL;
    usart->state = RESET;

    usart_interrupt_disable(usart->husart, USART_INT_RBNE);


    return OS_EOK;
}

static int gd32_usart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct gd32_usart *usart;
    os_size_t i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);

    usart = os_container_of(serial, struct gd32_usart, serial_dev);

    OS_ASSERT(usart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();
        usart_data_transmit(usart->husart, buff[i]);
        while (RESET == usart_flag_get(usart->husart, USART_FLAG_TBE));
        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops gd32_usart_ops = {
    .init         = gd32_configure,

    .start_send   = gd32_usart_start_send,
    .stop_send    = gd32_usart_stop_send,

    .start_recv   = gd32_usart_start_recv,
    .stop_recv    = gd32_usart_stop_recv,

    .poll_send    = gd32_usart_poll_send,
};

/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
static os_list_node_t gd32_usart_list = OS_LIST_INIT(gd32_usart_list);

void usart_irq_handler(void)
{
    os_base_t level;
    struct gd32_usart *usart;

    os_list_for_each_entry(usart, &gd32_usart_list, struct gd32_usart, list)
    {
        if(RESET != usart_interrupt_flag_get(usart->husart, USART_INT_FLAG_RBNE))
        {
            level = os_irq_lock();
            /* receive data */
            *(usart->buffer + usart->count) = usart_data_receive(usart->husart);
            usart->count++;
            if(usart->count == usart->size)
            {
                usart_interrupt_disable(usart->husart, USART_INT_RBNE);
                usart_interrupt_disable(usart->husart, USART_INT_IDLE);
                os_hw_serial_isr_rxdone((struct os_serial_device *)usart, usart->count);
            }
            os_irq_unlock(level);
            break;
        }
        else if(RESET != usart_interrupt_flag_get(usart->husart, USART_INT_FLAG_IDLE))
        {
            level = os_irq_lock();
            //read USART_STAT0£¬and read USART_DATA£¬then USART_INT_FLAG_IDLE will be cleaned
            usart_data_receive(usart->husart);
            if (usart->count != 0)
            {
                usart_interrupt_disable(usart->husart, USART_INT_RBNE);
                usart_interrupt_disable(usart->husart, USART_INT_IDLE);
                os_hw_serial_isr_rxdone((struct os_serial_device *)usart, usart->count);
            }
            os_irq_unlock(level);
        }
    }    
}

void USART1_IRQHandler(void)
{
    usart_irq_handler();
}

void USART0_IRQHandler(void)
{
    usart_irq_handler();
}

void UART6_IRQHandler(void)
{
    usart_irq_handler();
}

void USART2_IRQHandler(void)
{
    usart_irq_handler();
}

void gd_eval_com_init(struct gd32_usart_info *usart_info)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable((rcu_periph_enum)usart_info->pin_clk);

    /* enable USART clock */
    rcu_periph_clock_enable((rcu_periph_enum)usart_info->usart_clk);

    /* connect port to USARTx_Tx */
    gpio_af_set(usart_info->pin_port, usart_info->gpio_af_idx, usart_info->tx_pin);

    /* connect port to USARTx_Rx */
    gpio_af_set(usart_info->pin_port, usart_info->gpio_af_idx, usart_info->rx_pin);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(usart_info->pin_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, usart_info->tx_pin);
    gpio_output_options_set(usart_info->pin_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usart_info->tx_pin);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(usart_info->pin_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, usart_info->rx_pin);
    gpio_output_options_set(usart_info->pin_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usart_info->rx_pin);

    /* USART configure */
    usart_deinit(usart_info->husart);
    usart_baudrate_set(usart_info->husart, usart_info->baud_rate);
    usart_receive_config(usart_info->husart, USART_RECEIVE_ENABLE);
    usart_transmit_config(usart_info->husart, USART_TRANSMIT_ENABLE);
    usart_enable(usart_info->husart);

}

static void gd32_usart_init(struct gd32_usart_info *usart_info)
{

    nvic_irq_enable(usart_info->irq, 0, 0);
    gd_eval_com_init(usart_info);

    return;
}

static uint32_t console_uart = 0;//&huart1;
void __os_hw_console_output(char *str)
{
    if (console_uart == 0)
        return;

    while (*str)
    {
        if (*str == '\n')
        {
            usart_data_transmit(console_uart, '\r');
            while (RESET == usart_flag_get(console_uart, USART_FLAG_TBE));
        }
        
        usart_data_transmit(console_uart, *str);
        while (RESET == usart_flag_get(console_uart, USART_FLAG_TBE));
        str++;
    }

}

int gd32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{     
    os_base_t   level;
    
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    struct gd32_usart_info *usart_info = (struct gd32_usart_info *)dev->info;
    struct gd32_usart *usart = os_calloc(1, sizeof(struct gd32_usart));
    
    OS_ASSERT(usart);
    
    usart->husart = usart_info->husart;
    
    
    
    os_err_t result  = 0;
    
    struct os_serial_device *serial = &usart->serial_dev;

    //usart init
    gd32_usart_init(usart_info);
    
    serial->ops    = &gd32_usart_ops;
    serial->config = config;
    
    level = os_irq_lock();
    os_list_add_tail(&gd32_usart_list, &usart->list);
    os_irq_unlock(level);
    
    result = os_hw_serial_register(serial, dev->name, NULL);
    
    OS_ASSERT(result == OS_EOK);
    
    return result;
}


OS_DRIVER_INFO gd32_usart_driver = {
    .name   = "Usart_Type",
    .probe  = gd32_usart_probe,
};

OS_DRIVER_DEFINE(gd32_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);


#include "string.h"
static int gd32_uart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    if(!strcmp(dev->name, OS_CONSOLE_DEVICE_NAME))
    {
        struct gd32_usart_info * usart_info = (struct gd32_usart_info *)dev->info;
        gd32_usart_init(usart_info);
        console_uart = usart_info->husart;
    }
    return OS_EOK;
}

OS_DRIVER_INFO gd32_uart_early_driver = {
    .name   = "Usart_Type",
    .probe  = gd32_uart_early_probe,
};

OS_DRIVER_DEFINE(gd32_uart_early_driver, CORE, OS_INIT_SUBLEVEL_MIDDLE);

