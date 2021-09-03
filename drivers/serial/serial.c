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
 * @file        serial.c
 *
 * @brief       This file provides functions for serial.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <device.h>
#include <os_memory.h>
#include <os_util.h>
#include <os_assert.h>
#include <drv_cfg.h>

#define DBG_TAG "serial"

#ifdef OS_USING_VFS_DEVFS
#include <vfs_posix.h>

#ifdef OS_USING_POSIX_TERMIOS
#include <posix_termios.h>
#endif

/* It's possible the 'getc/putc' is defined by stdio.h in gcc/newlib. */
#ifdef getc
#undef getc
#endif

#ifdef putc
#undef putc
#endif

#endif

#ifdef OS_SERIAL_IDLE_TIMER

#define OS_SERIAL_RX_TIMER_STATUS_NONE   0
#define OS_SERIAL_RX_TIMER_STATUS_ON     1
#define OS_SERIAL_RX_TIMER_STATUS_OFF    2

static void _serial_rx_timer(void *parameter)
{
    os_base_t level;
    
    struct os_serial_device *serial;

    OS_ASSERT(parameter);
    serial  = (struct os_serial_device *)parameter;

    level = os_irq_lock();

    if (serial->rx_timer_status != OS_SERIAL_RX_TIMER_STATUS_ON)
    {
        os_irq_unlock(level);
        return;
    }

    int count = serial->ops->recv_count(serial);

    if (serial->rx_count == count && count != 0)
    {
        serial->ops->stop_recv(serial);
        os_hw_serial_isr_rxdone(serial, count);
    }
    else
    {
        serial->rx_count = count;
    }
    
    os_irq_unlock(level);
}

#endif

static void _serial_start_recv(struct os_serial_device *serial)
{
    serial->rx_count = 0;
    serial->ops->start_recv(serial, serial->rx_fifo->line_buff, serial->config.rx_bufsz);

#ifdef OS_SERIAL_IDLE_TIMER
    if (serial->rx_timer_status != OS_SERIAL_RX_TIMER_STATUS_NONE)
        serial->rx_timer_status = OS_SERIAL_RX_TIMER_STATUS_ON;
#endif
}

static int _serial_rx(struct os_serial_device *serial, os_uint8_t *data, int length)
{
    int size;
    os_base_t level;

    if (length <= 0)
        return 0;

    OS_ASSERT(serial != OS_NULL);

    level = os_irq_lock();

    OS_ASSERT(serial->rx_fifo != OS_NULL);

    size = rb_ring_buff_get(&serial->rx_fifo->rbuff, data, length);
    serial->parent.rx_count = rb_ring_buff_data_len(&serial->rx_fifo->rbuff);
    if (size != 0)
    {
        _serial_start_recv(serial);
    }

    os_irq_unlock(level);
    return size;
}

static int _serial_tx(struct os_serial_device *serial, const os_uint8_t *data, int length)
{
    int count;
    os_base_t level;
    struct os_serial_tx_fifo *tx_fifo;

    level = os_irq_lock();

    tx_fifo = serial->tx_fifo;

    length = rb_ring_buff_put(&tx_fifo->rbuff, data, length);

    if (length == 0)
        goto end;
    
    if (tx_fifo->line_buff_count != 0)
        goto end;

    count = rb_ring_buff_get(&tx_fifo->rbuff, tx_fifo->line_buff, serial->config.tx_bufsz);

    serial->flag |= OS_SERIAL_FLAG_TX_BUSY;
    tx_fifo->line_buff_count = serial->ops->start_send(serial, tx_fifo->line_buff, count);
    OS_ASSERT(tx_fifo->line_buff_count > 0);

    if (tx_fifo->line_buff_count < count)
    {
        rb_ring_buff_put(&tx_fifo->rbuff, tx_fifo->line_buff + tx_fifo->line_buff_count, count - tx_fifo->line_buff_count);
    }

end:
    OS_ASSERT(tx_fifo->line_buff_count != 0);
    serial->parent.tx_count = rb_ring_buff_data_len(&tx_fifo->rbuff);
    os_irq_unlock(level);
    return length;
}

static int _serial_poll_tx(struct os_serial_device *serial, const os_uint8_t *data, int length)
{
    int count;
    int send_index = 0;
    
    OS_ASSERT(serial != OS_NULL);    

    while (send_index < length)
    {
        count = serial->ops->poll_send(serial, data + send_index, length - send_index);

        if (count <= 0)
        {
            break;
        }
        
        send_index += count;
    }

    return send_index;
}

#ifdef OS_SERIAL_IDLE_TIMER
#ifdef OS_USING_TICKLESS_LPMGR
static os_err_t os_serial_control(struct os_device *dev, int cmd, void *args);

int serial_suspend(struct os_device *device, os_uint8_t mode)
{
    return os_serial_control(device, OS_DEVICE_CTRL_SUSPEND, NULL);
}

void serial_resume(struct os_device *device, os_uint8_t mode)
{
    os_serial_control(device, OS_DEVICE_CTRL_RESUME, NULL);
}

static struct os_lpmgr_device_ops serial_lpmgr_ops =
{
    serial_suspend,
    serial_resume,
};

#endif
#endif

static os_err_t os_serial_init(struct os_device *dev)
{
    os_err_t result = OS_EOK;
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    
    serial = (struct os_serial_device *)dev;

    OS_ASSERT(serial->tx_fifo == OS_NULL);
    OS_ASSERT(serial->rx_fifo == OS_NULL);

    /* sem */
    os_sem_init(&serial->sem, dev->name, 1, 1);

    /* rx buff */
    serial->rx_fifo = (struct os_serial_rx_fifo *)os_calloc(1, sizeof(struct os_serial_rx_fifo) + serial->config.rx_bufsz * 2);
    OS_ASSERT(serial->rx_fifo != OS_NULL);
    serial->rx_fifo->line_buff = (os_uint8_t *)(serial->rx_fifo + 1);            
    rb_ring_buff_init(&serial->rx_fifo->rbuff, serial->rx_fifo->line_buff + serial->config.rx_bufsz, serial->config.rx_bufsz);
    dev->rx_size = serial->config.rx_bufsz;
    dev->rx_count = 0;
    /* tx buff */
    if (serial->ops->start_send != OS_NULL)
    {
        struct os_serial_tx_fifo *tx_fifo;

        tx_fifo = (struct os_serial_tx_fifo *)os_calloc(1, sizeof(struct os_serial_tx_fifo) + serial->config.tx_bufsz * 2);
        OS_ASSERT(tx_fifo != OS_NULL);

        tx_fifo->line_buff = (os_uint8_t *)(tx_fifo + 1);
        tx_fifo->line_buff_count = 0;
        rb_ring_buff_init(&tx_fifo->rbuff, tx_fifo->line_buff + serial->config.tx_bufsz, serial->config.tx_bufsz);
        
        serial->tx_fifo = tx_fifo;
    }
    dev->tx_size = serial->config.tx_bufsz;
    dev->tx_count = 0;
    /* lowlevel initialization */
    if (serial->ops->init)
        result = serial->ops->init(serial, &serial->config);

#ifdef OS_SERIAL_IDLE_TIMER
    if (serial->ops->recv_count != OS_NULL)
    {
        if (serial->rx_timer_status == OS_SERIAL_RX_TIMER_STATUS_NONE)
        {
            serial->rx_timer_status = OS_SERIAL_RX_TIMER_STATUS_OFF;
            os_timer_init(&serial->rx_timer, 
                      device_name(&serial->parent),
                      _serial_rx_timer, 
                      serial, 
                      ((OS_TICK_PER_SECOND / 100) != 0) ? (OS_TICK_PER_SECOND / 100) : 1,
                      OS_TIMER_FLAG_PERIODIC);

            os_timer_start(&serial->rx_timer);
#ifdef OS_USING_TICKLESS_LPMGR
            os_lpmgr_device_register(dev, &serial_lpmgr_ops);
#endif
        }
    }
#endif

    _serial_start_recv(serial);

    return result;
}

static os_err_t os_serial_deinit(struct os_device *dev)
{
    os_base_t level;
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(dev->ref_count == 0);
    
    serial = (struct os_serial_device *)dev;
    
    level = os_irq_lock();

#ifdef OS_SERIAL_DELAY_CLOSE
    if (serial->flag & OS_SERIAL_FLAG_TX_BUSY)
    {
        os_irq_unlock(level);
        return OS_EBUSY;
    }
#endif

    if (serial->ops->stop_send)
        serial->ops->stop_send(serial);

    if (serial->ops->stop_recv)
        serial->ops->stop_recv(serial);

    if(serial->ops->deinit)
        serial->ops->deinit(serial);

    os_irq_unlock(level);

    os_sem_wait(&serial->sem, OS_WAIT_FOREVER);

#ifdef OS_SERIAL_IDLE_TIMER
    if (serial->rx_timer_status != OS_SERIAL_RX_TIMER_STATUS_NONE)
    {
        os_timer_deinit(&serial->rx_timer);
        serial->rx_timer_status = OS_SERIAL_RX_TIMER_STATUS_NONE;
#ifdef OS_USING_TICKLESS_LPMGR
        os_lpmgr_device_unregister(dev, &serial_lpmgr_ops);
#endif
    }
#endif

    /* rx fifo */
    if (serial->rx_fifo != OS_NULL)
    {
        os_free(serial->rx_fifo);
        serial->rx_fifo = OS_NULL;
    }

    /* tx fifo */
    if (serial->tx_fifo != OS_NULL)
    {
        os_free(serial->tx_fifo);
        serial->tx_fifo = OS_NULL;
    }

    os_sem_post(&serial->sem);

    os_sem_deinit(&serial->sem);

    return OS_EOK;
}

static os_size_t os_serial_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    
    if (size == 0)
        return 0;

    serial = (struct os_serial_device *)dev;

    return _serial_rx(serial, (os_uint8_t *)buffer, size);
}

static os_size_t os_serial_write(struct os_device *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    if (size == 0)
        return 0;

    serial = (struct os_serial_device *)dev;

    if (os_console_get_device() == dev)
        return _serial_poll_tx(serial, (const os_uint8_t *)buffer, size);

    if (serial->ops->start_send != OS_NULL)
        return _serial_tx(serial, (const os_uint8_t *)buffer, size);

    return _serial_poll_tx(serial, (const os_uint8_t *)buffer, size);
}

#ifdef OS_USING_POSIX_TERMIOS
struct speed_baudrate_item
{
    speed_t speed;
    int     baudrate;
};

const static struct speed_baudrate_item _tbl[] =
{
    {B2400, BAUD_RATE_2400},
    {B4800, BAUD_RATE_4800},
    {B9600, BAUD_RATE_9600},
    {B19200, BAUD_RATE_19200},
    {B38400, BAUD_RATE_38400},
    {B57600, BAUD_RATE_57600},
    {B115200, BAUD_RATE_115200},
    {B230400, BAUD_RATE_230400},
    {B460800, BAUD_RATE_460800},
    {B921600, BAUD_RATE_921600},
    {B2000000, BAUD_RATE_2000000},
    {B3000000, BAUD_RATE_3000000},
};

static speed_t _get_speed(int baudrate)
{
    int index;

    for (index = 0; index < sizeof(_tbl) / sizeof(_tbl[0]); index++)
    {
        if (_tbl[index].baudrate == baudrate)
            return _tbl[index].speed;
    }

    return B0;
}

static int _get_baudrate(speed_t speed)
{
    int index;

    for (index = 0; index < sizeof(_tbl) / sizeof(_tbl[0]); index++)
    {
        if (_tbl[index].speed == speed)
            return _tbl[index].baudrate;
    }

    return 0;
}

static void _tc_flush(struct os_serial_device *serial, int queue)
{
    os_base_t level;
    os_uint8_t c;
    int ch = -1;

    struct os_device *        device  = OS_NULL;

    OS_ASSERT(serial != OS_NULL);

    device  = &(serial->parent);

    switch (queue)
    {
    case TCIFLUSH:
    case TCIOFLUSH:

        OS_ASSERT(serial->rx_fifo != OS_NULL);
        level = os_irq_lock();
        rb_ring_buff_reset(&serial->rx_fifo->rbuff);
        os_irq_unlock(level);

        break;

    case TCOFLUSH:
        break;
    }
}

#endif

static os_err_t os_serial_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t ret = OS_EOK;
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    serial = (struct os_serial_device *)dev;

    switch (cmd)
    {
#ifdef OS_USING_TICKLESS_LPMGR
#ifdef OS_SERIAL_IDLE_TIMER
    case OS_DEVICE_CTRL_SUSPEND:
        if (serial->rx_timer_status != OS_SERIAL_RX_TIMER_STATUS_NONE)
        {
            os_timer_stop(&serial->rx_timer);
        }
        break;

    case OS_DEVICE_CTRL_RESUME:
        if (serial->rx_timer_status != OS_SERIAL_RX_TIMER_STATUS_NONE)
        {
            os_timer_start(&serial->rx_timer);
        }
        break;
#endif
#endif
    case OS_DEVICE_CTRL_CONFIG:
        if (args)
        {
            struct serial_configure *pconfig = (struct serial_configure *)args;
            if ((pconfig->rx_bufsz != serial->config.rx_bufsz || pconfig->tx_bufsz != serial->config.tx_bufsz) &&
                serial->parent.ref_count)
            {
                /*Can not change buffer size*/
                return OS_EBUSY;
            }
            /* Set serial configure */
            serial->config = *pconfig;
            if (serial->parent.ref_count)
            {
                if(serial->ops->deinit)
                    serial->ops->deinit(serial);

                /* Serial device has been opened, to configure it */
                if (serial->ops->init)
                {
                    ret = serial->ops->init(serial, (struct serial_configure *)args);
                    if (ret == OS_EOK)
                    {
                        _serial_start_recv(serial);
                    }
                    return ret;
                }
                else
                {
                    return OS_EOK;
                }
            }
        }
        break;

#ifdef OS_USING_POSIX_TERMIOS
    case TCGETA:
    {
        struct termios *tio = (struct termios *)args;
        if (tio == OS_NULL)
            return OS_EINVAL;

        tio->c_iflag = 0;
        tio->c_oflag = 0;
        tio->c_lflag = 0;

        /* Update oflag for console device */
        if (os_console_get_device() == dev)
            tio->c_oflag = OPOST | ONLCR;

        /* Set cflag */
        tio->c_cflag = 0;
        if (serial->config.data_bits == DATA_BITS_5)
            tio->c_cflag = CS5;
        else if (serial->config.data_bits == DATA_BITS_6)
            tio->c_cflag = CS6;
        else if (serial->config.data_bits == DATA_BITS_7)
            tio->c_cflag = CS7;
        else if (serial->config.data_bits == DATA_BITS_8)
            tio->c_cflag = CS8;

        if (serial->config.stop_bits == STOP_BITS_2)
            tio->c_cflag |= CSTOPB;

        if (serial->config.parity == PARITY_EVEN)
            tio->c_cflag |= PARENB;
        else if (serial->config.parity == PARITY_ODD)
            tio->c_cflag |= (PARODD | PARENB);

        cfsetospeed(tio, _get_speed(serial->config.baud_rate));
    }
    break;

    case TCSETAW:
    case TCSETAF:
    case TCSETA:
    {
        int baudrate;
        struct serial_configure config;

        struct termios *tio = (struct termios *)args;
        if (tio == OS_NULL)
            return OS_EINVAL;

        config = serial->config;

        baudrate         = _get_baudrate(cfgetospeed(tio));
        config.baud_rate = baudrate;

        switch (tio->c_cflag & CSIZE)
        {
        case CS5:
            config.data_bits = DATA_BITS_5;
            break;
        case CS6:
            config.data_bits = DATA_BITS_6;
            break;
        case CS7:
            config.data_bits = DATA_BITS_7;
            break;
        default:
            config.data_bits = DATA_BITS_8;
            break;
        }

        if (tio->c_cflag & CSTOPB)
            config.stop_bits = STOP_BITS_2;
        else
            config.stop_bits = STOP_BITS_1;

        if (tio->c_cflag & PARENB)
        {
            if (tio->c_cflag & PARODD)
                config.parity = PARITY_ODD;
            else
                config.parity = PARITY_EVEN;
        }
        else
            config.parity = PARITY_NONE;

        if (serial->ops->init)
            serial->ops->init(serial, &config);
    }
    break;
    case TCFLSH:
    {
        int queue = (int)args;

        _tc_flush(serial, queue);
    }

    break;
    case TCXONC:
        break;
#endif
#if 0 && defined(OS_USING_VFS_DEVFS)
    case FIONREAD:
    {
        os_size_t recved = 0;
        os_base_t level;

        level  = os_irq_lock();
        recved = rb_ring_buff_data_len(&serial->rx_fifo->rbuff);
        os_irq_unlock(level);

        *(os_size_t *)args = recved;
    }
    break;
#endif
    default:
        /* Control device */
        ret = OS_ENOSYS;
        break;
    }

    return ret;
}

const static struct os_device_ops serial_ops = 
{
    .init    = os_serial_init,
    .deinit  = os_serial_deinit,
    .read    = os_serial_read,
    .write   = os_serial_write,
    .control = os_serial_control
};

#ifdef OS_SERIAL_DELAY_CLOSE
static void __os_serial_deinit(void *data)
{
    struct os_device *dev;

    dev = (struct os_device *)data;
    os_sem_wait(&dev->sem, OS_WAIT_FOREVER);
    os_serial_deinit(dev);
    dev->state = OS_DEVICE_STATE_CLOSE;
    os_sem_post(&dev->sem);
}
#endif

os_err_t os_hw_serial_register(struct os_serial_device *serial, const char *name, void *data)
{
    os_err_t          ret;
    struct os_device *device;
    OS_ASSERT(serial != OS_NULL);

    OS_ASSERT(serial->ops->start_recv != OS_NULL);

    device = &(serial->parent);

    device->type        = OS_DEVICE_TYPE_CHAR;

    device->ops = &serial_ops;
    device->user_data = data;

    /* Register a character device */
    ret = os_device_register(device, name);

#ifdef OS_SERIAL_DELAY_CLOSE
    os_work_init(&serial->work, __os_serial_deinit, &serial->parent);
#endif

    return ret;
}

void os_hw_serial_isr_rxdone(struct os_serial_device *serial, int count)
{
    int count_put;
    os_device_t *dev;

    OS_ASSERT(serial);
    OS_ASSERT(serial->rx_fifo != OS_NULL);
    OS_ASSERT(count != 0);

#ifdef OS_SERIAL_IDLE_TIMER
    if (serial->rx_timer_status != OS_SERIAL_RX_TIMER_STATUS_NONE)
        serial->rx_timer_status = OS_SERIAL_RX_TIMER_STATUS_OFF;
#endif

    dev = &serial->parent;
    count_put = rb_ring_buff_put(&serial->rx_fifo->rbuff, serial->rx_fifo->line_buff, count);
    dev->rx_count = rb_ring_buff_data_len(&serial->rx_fifo->rbuff);
    os_device_recv_notify(dev);

    if (count_put == count)
    {
        _serial_start_recv(serial);
    }
    else
    {
        /* drop some data */
    }
}

void os_hw_serial_isr_txdone(struct os_serial_device *serial)
{
    int count;
    os_device_t *dev;
    struct os_serial_tx_fifo *tx_fifo;

    dev = &serial->parent;

    tx_fifo = (struct os_serial_tx_fifo *)serial->tx_fifo;

    OS_ASSERT(tx_fifo->line_buff_count != 0);

    tx_fifo->line_buff_count = 0;

    count = rb_ring_buff_get(&tx_fifo->rbuff, tx_fifo->line_buff, serial->config.tx_bufsz);

    if (count == 0)
    {
        serial->flag &= ~OS_SERIAL_FLAG_TX_BUSY;
        
#ifdef OS_SERIAL_DELAY_CLOSE
        if (serial->parent.state == OS_DEVICE_STATE_CLOSING)
        {
            os_submit_work(&serial->work, 0);
        }
#endif
        dev->tx_count = 0;
        os_device_send_notify(dev);
    }
    else
    {
        tx_fifo->line_buff_count = serial->ops->start_send(serial, tx_fifo->line_buff, count);
        OS_ASSERT(tx_fifo->line_buff_count > 0);

        if (tx_fifo->line_buff_count < count)
        {
            rb_ring_buff_put(&tx_fifo->rbuff, tx_fifo->line_buff + tx_fifo->line_buff_count, count - tx_fifo->line_buff_count);
        }
        
        dev->tx_count = rb_ring_buff_data_len(&tx_fifo->rbuff);
    }
}

