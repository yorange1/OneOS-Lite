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
 * @brief       This file implements usart driver for lpc
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <drv_cfg.h>
#include <drv_usart.h>
#include <os_clock.h>
#include <string.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

typedef struct lpc_usart
{
    struct os_serial_device serial;
    lpc_usart_info_t *usart_info;
    hal_usart_transfer_t sendXfer;
    hal_usart_transfer_t receiveXfer;
    os_uint32_t clk_src;

    IRQn_Type irqn;
    usart_handle_t *usart_handle;
    usart_dma_handle_t *usart_DmaHandle;

    os_list_node_t list;
}lpc_usart_t;

static os_list_node_t lpc_usart_list = OS_LIST_INIT(lpc_usart_list);

static USART_Type *console_uart = OS_NULL;

void lpc_usart_irq_callback(lpc_usart_t *lpc_usart)
{
    if (USART_GetStatusFlags(lpc_usart->usart_info->usart_base) & (kUSART_RxFifoNotEmptyFlag | kUSART_RxError))
    {
        lpc_usart->receiveXfer.transfer.data[lpc_usart->receiveXfer.count_cur++] = USART_ReadByte(lpc_usart->usart_info->usart_base);
        if ((lpc_usart->receiveXfer.count_cur == lpc_usart->receiveXfer.transfer.dataSize) && (lpc_usart->receiveXfer.transfer.data != OS_NULL))
        {
            USART_DisableInterrupts(lpc_usart->usart_info->usart_base, kUSART_RxLevelInterruptEnable);
            
            os_base_t level;
            level = os_irq_lock();
            os_hw_serial_isr_rxdone((struct os_serial_device *)lpc_usart, lpc_usart->receiveXfer.count_cur);
            os_irq_unlock(level);
            
            lpc_usart->receiveXfer.transfer.data = OS_NULL;
            lpc_usart->receiveXfer.count_cur = 0;
            lpc_usart->receiveXfer.transfer.dataSize = 0;
        }
    }
    
    if (USART_GetStatusFlags(lpc_usart->usart_info->usart_base) & kUSART_RxError)
    {
        USART_ClearStatusFlags(lpc_usart->usart_info->usart_base, kUSART_RxError);
    }
    SDK_ISR_EXIT_BARRIER;
}

USART_IRQHandler_DEFINE(0);
USART_IRQHandler_DEFINE(1);
USART_IRQHandler_DEFINE(2);
USART_IRQHandler_DEFINE(3);
USART_IRQHandler_DEFINE(4);
USART_IRQHandler_DEFINE(5);
USART_IRQHandler_DEFINE(6);
USART_IRQHandler_DEFINE(7);
USART_IRQHandler_DEFINE(8);

void lpc_usart_dma_callback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData)
{
    lpc_usart_t *lpc_usart = (lpc_usart_t *)userData;
    
    if (kStatus_USART_TxIdle == status)
    {
        os_hw_serial_isr_txdone((struct os_serial_device *)lpc_usart);
    }
    
    if (kStatus_USART_RxIdle == status)
    {
        os_hw_serial_isr_rxdone((struct os_serial_device *)lpc_usart, lpc_usart->receiveXfer.transfer.dataSize);
    }
}

void lpc_usart_transfer_callback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    lpc_usart_t *lpc_usart = (lpc_usart_t *)userData;
    
    if (kStatus_USART_TxIdle == status)
    {
        os_hw_serial_isr_txdone((struct os_serial_device *)lpc_usart);
    }
    
    if (kStatus_USART_RxIdle == status)
    {
        os_hw_serial_isr_rxdone((struct os_serial_device *)lpc_usart, lpc_usart->receiveXfer.transfer.dataSize);
    }
}

static os_err_t lpc_usart_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    lpc_usart_t *lpc_usart = (lpc_usart_t *)serial;
    
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    
    LOG_D(DRV_EXT_TAG, "sofeware just use to config baud_rate, other param should use MCUXpresso config tool!");
        
    if (USART_SetBaudRate(lpc_usart->usart_info->usart_base, cfg->baud_rate, lpc_usart->clk_src) != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "%s set baudrate %d failed! this baudrate not support!", serial->parent.name, cfg->baud_rate);
        return OS_ERROR;
    }

    return OS_EOK;
}

static int lpc_usart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    status_t status = kStatus_Success;
    
    lpc_usart_t *lpc_usart;
    
    OS_ASSERT(serial != OS_NULL);

    lpc_usart = (lpc_usart_t *)serial;

    lpc_usart->sendXfer.transfer.data = (os_uint8_t *)buff;
    lpc_usart->sendXfer.transfer.dataSize = size;

    if (lpc_usart->usart_DmaHandle != OS_NULL)
    {
        status = USART_TransferSendDMA(lpc_usart->usart_info->usart_base, lpc_usart->usart_DmaHandle, &lpc_usart->sendXfer.transfer);
    }
    else if (lpc_usart->usart_handle != OS_NULL)
    {
        status = USART_TransferSendNonBlocking(lpc_usart->usart_info->usart_base, lpc_usart->usart_handle, &lpc_usart->sendXfer.transfer);
    }

    return (status == kStatus_Success) ? size : 0;
}

static int lpc_usart_stop_send(struct os_serial_device *serial)
{
    lpc_usart_t *lpc_usart;
    
    OS_ASSERT(serial != OS_NULL);

    lpc_usart = (lpc_usart_t *)serial;
    
    if (lpc_usart->usart_DmaHandle != OS_NULL)
    {
        USART_TransferAbortSendDMA(lpc_usart->usart_info->usart_base, lpc_usart->usart_DmaHandle);
    }
    else if (lpc_usart->usart_handle != OS_NULL)
    {
        USART_TransferAbortSend(lpc_usart->usart_info->usart_base, lpc_usart->usart_handle);
    }
    
    return OS_EOK;
}

static int lpc_usart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    status_t status = kStatus_Success;
    
    lpc_usart_t *lpc_usart;

    OS_ASSERT(serial != OS_NULL);

    lpc_usart = (lpc_usart_t *)serial;

    lpc_usart->receiveXfer.transfer.data = buff;
    lpc_usart->receiveXfer.transfer.dataSize = size;
    lpc_usart->receiveXfer.count_cur = 0;

    if (lpc_usart->usart_DmaHandle != OS_NULL)
    {
        status = USART_TransferReceiveDMA(lpc_usart->usart_info->usart_base, lpc_usart->usart_DmaHandle, &lpc_usart->receiveXfer.transfer);
    }
    else if (lpc_usart->irqn >= 14)
    {
        USART_EnableInterrupts(lpc_usart->usart_info->usart_base, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    }
    else if (lpc_usart->usart_handle != OS_NULL)
    {
        status = USART_TransferReceiveNonBlocking(lpc_usart->usart_info->usart_base, lpc_usart->usart_handle, &lpc_usart->receiveXfer.transfer, OS_NULL);
    }

    return (status == kStatus_Success) ? size : 0;
}

static int lpc_usart_stop_recv(struct os_serial_device *serial)
{
    lpc_usart_t *lpc_usart;

    OS_ASSERT(serial != OS_NULL);

    lpc_usart = (lpc_usart_t *)serial;

    if (lpc_usart->usart_DmaHandle != OS_NULL)
    {
        USART_TransferAbortReceiveDMA(lpc_usart->usart_info->usart_base, lpc_usart->usart_DmaHandle);
    }
    else if (lpc_usart->usart_handle != OS_NULL)
    {
        USART_TransferAbortReceive(lpc_usart->usart_info->usart_base, lpc_usart->usart_handle);
    }
    else if (lpc_usart->irqn >= 14)
    {
        USART_DisableInterrupts(lpc_usart->usart_info->usart_base, kUSART_RxLevelInterruptEnable);
    }
    
    lpc_usart->receiveXfer.transfer.data = OS_NULL;
    lpc_usart->receiveXfer.transfer.dataSize = 0;
    lpc_usart->receiveXfer.count_cur = 0;
    
    return OS_EOK;
}

static int lpc_usart_recv_count(struct os_serial_device *serial)
{
    status_t status = kStatus_Success;
    
    lpc_usart_t *lpc_usart;
    
    OS_ASSERT(serial != OS_NULL);

    lpc_usart = (lpc_usart_t *)serial;

    if (lpc_usart->usart_DmaHandle != OS_NULL)
    {
        status = USART_TransferGetReceiveCountDMA(lpc_usart->usart_info->usart_base, lpc_usart->usart_DmaHandle, (uint32_t *)&lpc_usart->receiveXfer.count_cur);
    }
    else if (lpc_usart->usart_handle != OS_NULL)
    {
        status = USART_TransferGetReceiveCount(lpc_usart->usart_info->usart_base, lpc_usart->usart_handle, (uint32_t *)&lpc_usart->receiveXfer.count_cur);
    }

    return (status == kStatus_Success) ? lpc_usart->receiveXfer.count_cur : 0;
}

static int lpc_usart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int32_t  ret;
    status_t status = kStatus_Success;
    
    lpc_usart_t *lpc_usart;
    
    os_base_t level;
    
    OS_ASSERT(serial != OS_NULL);

    lpc_usart = os_container_of(serial, lpc_usart_t, serial);

    level = os_irq_lock();
    status = USART_WriteBlocking(lpc_usart->usart_info->usart_base, (uint8_t *)buff, size);
    os_irq_unlock(level);
    
    return (status == kStatus_Success) ? size : 0;
}

static struct os_uart_ops lpc_usart_ops = {
    .init    = lpc_usart_configure,

    .start_send   = lpc_usart_start_send,
    .stop_send    = lpc_usart_stop_send,

    .start_recv   = lpc_usart_start_recv,
    .stop_recv    = lpc_usart_stop_recv,
#ifdef OS_SERIAL_IDLE_TIMER
    .recv_count   = lpc_usart_recv_count,
#endif
    .poll_send    = lpc_usart_poll_send,
};

void lpc_usart_parse_configs_from_configtool(lpc_usart_t *lpc_usart)
{
    struct os_serial_device *serial = &lpc_usart->serial;
    
    serial->config.baud_rate = lpc_usart->usart_info->usart_config->baudRate_Bps;
    switch (lpc_usart->usart_info->usart_config->stopBitCount)
    {
    case kUSART_OneStopBit:
        serial->config.stop_bits = STOP_BITS_1;
        break;
    case kUSART_TwoStopBit:
        serial->config.stop_bits = STOP_BITS_2;
        break;
    }
    switch (lpc_usart->usart_info->usart_config->parityMode)
    {
    case kUSART_ParityDisabled:
        serial->config.parity   = PARITY_NONE;
        break;
    case kUSART_ParityOdd:
        serial->config.parity   = PARITY_ODD;
        break;
    case kUSART_ParityEven:
        serial->config.parity   = PARITY_EVEN;
        break;
    }

    switch (lpc_usart->usart_info->usart_config->bitCountPerChar)
    {
    case kUSART_7BitsPerChar:
        serial->config.data_bits = DATA_BITS_7;
        break;
    case kUSART_8BitsPerChar:
        serial->config.data_bits = DATA_BITS_8;
        break;
    }
}

os_err_t lpc_usart_param_cfg(lpc_usart_t *lpc_usart)
{
    os_err_t err = 0;
    switch((os_uint32_t)lpc_usart->usart_info->usart_base)
    {
    case (os_uint32_t)FLEXCOMM0:
        USART0_CFG_INIT(lpc_usart, 0);
        break;
    case (os_uint32_t)FLEXCOMM1:
        USART1_CFG_INIT(lpc_usart, 1);
        break;
    case (os_uint32_t)FLEXCOMM2:
        USART2_CFG_INIT(lpc_usart, 2);
        break;
    case (os_uint32_t)FLEXCOMM3:
        USART3_CFG_INIT(lpc_usart, 3);
        break;
    case (os_uint32_t)FLEXCOMM4:
        USART4_CFG_INIT(lpc_usart, 4);
        break;
    case (os_uint32_t)FLEXCOMM5:
        USART5_CFG_INIT(lpc_usart, 5);
        break;
    case (os_uint32_t)FLEXCOMM6:
        USART6_CFG_INIT(lpc_usart, 6);
        break;
    case (os_uint32_t)FLEXCOMM7:
        USART7_CFG_INIT(lpc_usart, 7);
        break;
    case (os_uint32_t)FLEXCOMM8:
         USART8_CFG_INIT(lpc_usart, 8);
        break;
    default:
        break;
    }
    if (lpc_usart->clk_src == 0)
        return OS_ERROR;
    
    if ((lpc_usart->usart_DmaHandle))
        return OS_EOK;
    else
        return OS_EEMPTY;
}

static int lpc_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;
    
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    lpc_usart_info_t *usart_info = (lpc_usart_info_t *)dev->info;

    lpc_usart_t *lpc_usart = os_calloc(1, sizeof(lpc_usart_t));

    OS_ASSERT(lpc_usart);
    
    lpc_usart->usart_info = usart_info;
    
    struct os_serial_device *serial = &lpc_usart->serial;

    serial->ops    = &lpc_usart_ops;
    serial->config = config;

    result = lpc_usart_param_cfg(lpc_usart);
    if ( result == OS_ERROR)
    {
        os_free(lpc_usart);
        return result;
    }
    else if (result == OS_EEMPTY)
    {
        lpc_usart_ops.start_send = OS_NULL;
    }
    lpc_usart_parse_configs_from_configtool(lpc_usart);

    level = os_irq_lock();
    os_list_add_tail(&lpc_usart_list, &lpc_usart->list);
    os_irq_unlock(level);
    
    result = os_hw_serial_register(serial, dev->name, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO lpc_usart_driver = {
    .name   = "USART_Type",
    .probe  = lpc_usart_probe,
};

OS_DRIVER_DEFINE(lpc_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

void __os_hw_console_output(char *str)
{
    if (console_uart == OS_NULL)
        return;

    while (*str)
    {   
        USART_WriteBlocking(console_uart, (uint8_t *)str, 1);
        str++;
    }
}

static int lpc_usart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    if(!strcmp(dev->name,OS_CONSOLE_DEVICE_NAME))
    {
        lpc_usart_info_t *usart_info = (lpc_usart_info_t *)dev->info;
        console_uart = (USART_Type *)usart_info->usart_base;
    }
    return OS_EOK;
}

OS_DRIVER_INFO lpc_usart_early_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = lpc_usart_early_probe,
};

OS_DRIVER_DEFINE(lpc_usart_early_driver, CORE, OS_INIT_SUBLEVEL_MIDDLE);

