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

#include "drv_usart.h"

struct gd32_usart
{
    struct os_serial_device serial_dev;

	os_uint8_t *buffer;
	const char *device_name;

    os_uint32_t husart;
	os_size_t count;
	os_size_t size;
	os_int32_t state;
    
    os_list_node_t list;
};

static os_err_t gd32_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
	struct gd32_usart *uart;
    os_uint32_t        data_bits;
    os_uint32_t        stop_bits;
    os_uint32_t        word_len;
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    os_uint32_t parity;
    uart = os_container_of(serial, struct gd32_usart, serial_dev);
    
	stop_bits = stop_bits;
	parity = parity;
    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        stop_bits = USART_STB_1BIT;
        break;
    case STOP_BITS_2:
        stop_bits = USART_STB_2BIT;
        break;
    default:
        return OS_EINVAL;
    }
    
    switch (cfg->parity)
    {
    case PARITY_NONE:
        parity = USART_PM_NONE;
        data_bits                = cfg->data_bits;
        break;
    case PARITY_ODD:
        parity = USART_PM_ODD;
        data_bits                = cfg->data_bits + 1;
        break;
    case PARITY_EVEN:
        parity = USART_PM_EVEN;
        data_bits                = cfg->data_bits + 1;
        break;
    default:
        return OS_EINVAL;
    }
    
    switch (data_bits)
    {
    case DATA_BITS_8:
        word_len = USART_WL_8BIT;
        break;
    case DATA_BITS_9:
        word_len = USART_WL_9BIT;
        break;
    default:
        return OS_EINVAL;
    }
    
    usart_stop_bit_set(uart->husart, cfg->stop_bits);
    usart_baudrate_set(uart->husart, cfg->baud_rate);
    usart_parity_config(uart->husart, cfg->parity);
    usart_word_length_set(uart->husart, word_len);
    
    return OS_EOK;
}

static int gd32_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
	return OS_EOK;
}

static int gd32_uart_stop_send(struct os_serial_device *serial)
{
	return OS_EOK;
}

static int gd32_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
	struct gd32_usart * usart;

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



static int gd32_uart_stop_recv(struct os_serial_device *serial)
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

static int gd32_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
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

	.start_send   = gd32_uart_start_send,
	.stop_send    = gd32_uart_stop_send,

	.start_recv   = gd32_uart_start_recv,
	.stop_recv    = gd32_uart_stop_recv,

	.poll_send    = gd32_uart_poll_send,
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
    struct gd32_usart *usart;

    os_list_for_each_entry(usart, &gd32_usart_list, struct gd32_usart, list)
    {
        if(RESET != usart_interrupt_flag_get(usart->husart, USART_INT_FLAG_RBNE))
        {
            usart_interrupt_flag_clear(usart->husart, USART_INT_FLAG_RBNE);
            /* receive data */
            *(usart->buffer + usart->count) = usart_data_receive(usart->husart);
            usart->count++;

            if(usart->count == usart->size)
            {
                usart_interrupt_disable(usart->husart, USART_INT_RBNE);
                usart_interrupt_disable(usart->husart, USART_INT_IDLE);
                os_hw_serial_isr_rxdone((struct os_serial_device *)usart, usart->count);
            }

            break;
        }
        else if(RESET != usart_interrupt_flag_get(usart->husart, USART_INT_FLAG_IDLE))
        {
            //read USART_STAT0��and read USART_DATA��then USART_INT_FLAG_IDLE will be cleaned
            usart_data_receive(usart->husart);
            if (usart->count != 0)
            {
                usart_interrupt_disable(usart->husart, USART_INT_RBNE);
                usart_interrupt_disable(usart->husart, USART_INT_IDLE);
                os_hw_serial_isr_rxdone((struct os_serial_device *)usart, usart->count);
            }
        }
    }    
}


void USART0_IRQHandler(void)
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
    gpio_init(usart_info->pin_port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, usart_info->tx_pin);

    /* connect port to USARTx_Rx */
    gpio_init(usart_info->pin_port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, usart_info->rx_pin);

    /* USART configure */
    usart_deinit(usart_info->husart);
    usart_baudrate_set(usart_info->husart, usart_info->baud_rate);
    usart_receive_config(usart_info->husart, USART_RECEIVE_ENABLE);
    usart_transmit_config(usart_info->husart, USART_TRANSMIT_ENABLE);
    usart_enable(usart_info->husart);
}

void gd32_usart_init(struct gd32_usart_info *usart_info)
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

