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
 * @file        posix_serial.c
 *
 * @brief       posix_serial
 *
 * @details     posix_serial
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <arch_interrupt.h>
#include <os_assert.h>

#include <device.h>
#include <drv_cfg.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <termios.h>
#include <unistd.h>

struct posix_serial_uart
{
    struct os_serial_device serial;

    struct termios oldt;
    
    os_bool_t   rx_isr_enabled;

    os_uint8_t *buff;
    os_size_t   size;
    os_size_t   count;
};

int posix_serial_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct posix_serial_uart *uart = os_container_of(serial, struct posix_serial_uart, serial);
    
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);

    struct termios newt;

    tcgetattr(STDIN_FILENO, &uart->oldt);
    newt = uart->oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    uart->rx_isr_enabled = OS_FALSE;

    return 0;
}

int posix_serial_deinit(struct os_serial_device *serial)
{
    struct posix_serial_uart *uart = os_container_of(serial, struct posix_serial_uart, serial);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &uart->oldt);

    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags & ~O_NONBLOCK);

    return 0;
}

static int posix_serial_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct posix_serial_uart *uart = os_container_of(serial, struct posix_serial_uart, serial);

    os_base_t level;

    level = os_irq_lock();

    uart->buff  = buff;
    uart->size  = size;
    uart->count = 0;

    uart->rx_isr_enabled = OS_TRUE;

    os_irq_unlock(level);
    
    return 0;
}

static int posix_serial_stop_recv(struct os_serial_device *serial)
{
    struct posix_serial_uart *uart = os_container_of(serial, struct posix_serial_uart, serial);

    os_base_t level;

    level = os_irq_lock();
    
    uart->rx_isr_enabled = OS_FALSE;

    uart->buff  = OS_NULL;
    uart->size  = 0;
    uart->count = 0;

    os_irq_unlock(level);
    
    return 0;
}

static void posix_serial_rx_process(struct posix_serial_uart *uart)
{
    os_base_t level;
    
    level = os_irq_lock();
    int count = fread(uart->buff + uart->count, 1, uart->size - uart->count, stdin);
    os_irq_unlock(level);   
    if (count > 0)
    {
        uart->count += count;

        OS_ASSERT(uart->count <= uart->size);
        
        if (uart->count == uart->size)
        {
            uart->rx_isr_enabled = OS_FALSE;
            os_hw_serial_isr_rxdone((struct os_serial_device *)uart, uart->size);
        }
    }
}

static int posix_serial_recv_count(struct os_serial_device *serial)
{    
    struct posix_serial_uart *uart = os_container_of(serial, struct posix_serial_uart, serial);

    if (uart->rx_isr_enabled == OS_TRUE)
    {
        posix_serial_rx_process(uart);
        return uart->count;
    }
    else
    {
        return 0;
    }
}

static int posix_serial_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
     os_base_t level;
     
     level = os_irq_lock();


    for (i = 0; i < size; i++)
    {
        printf("%c", buff[i]);
        fflush(stdout);
    }

    os_irq_unlock(level); 

    return size;
}

static const struct os_uart_ops posix_serial_ops = {
    .init         = posix_serial_init,
    .deinit       = posix_serial_deinit,

    .start_recv   = posix_serial_start_recv,
    .stop_recv    = posix_serial_stop_recv,
    .recv_count   = posix_serial_recv_count,
    
    .poll_send    = posix_serial_poll_send,
};

static struct posix_serial_uart uart;

static int os_hw_posix_serial_init(void)
{
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    memset(&uart, 0, sizeof(uart));

    uart.serial.ops    = &posix_serial_ops;
    uart.serial.config = config;

    os_hw_serial_register(&uart.serial, OS_USING_POSIX_SERIAL_NAME, OS_NULL);

    return 0;
}

OS_CORE_INIT(os_hw_posix_serial_init, OS_INIT_SUBLEVEL_LOW);

