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
 * @file        drv_uart_l1xx.c
 *
 * @brief       This file implements usart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <device.h>
#include <string.h>
#include "hc_gpio.h"
#include "hc_uart.h"
#include "hc_dmac.h"
#include "drv_usart.h"
#include "board.h"

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
#include "hc_lpuart.h"
#endif

#ifdef BSP_USING_LPUART0
static struct os_serial_device serial0;
#endif
#ifdef BSP_USING_LPUART1
static struct os_serial_device serial1;
#endif
#ifdef BSP_USING_UART0
static struct os_serial_device serial2;
#endif
#ifdef BSP_USING_UART1
static struct os_serial_device serial3;
#endif
#ifdef BSP_USING_UART2
static struct os_serial_device serial4;
#endif
#ifdef BSP_USING_UART3
static struct os_serial_device serial5;
#endif

/* uart driver */
struct hc_uart
{
    uint8_t    uart_device;
    IRQn_Type   irqn;
    void*   huart;
    en_dma_trig_sel_t req_num;
    en_sysctrl_peripheral_gate_t peripheral;
    en_gpio_port_t tx_port;
    en_gpio_pin_t  tx_pin;
    en_gpio_af_t   tx_af;
    en_gpio_port_t rx_port;
    en_gpio_pin_t  rx_pin;
    en_gpio_af_t   rx_af;

    struct os_serial_device *serial_dev;
    char   *device_name;
    uint8_t   dma_support;
    en_dma_channel_t dma_channel;
    uint8_t  *buff;
    uint32_t  rx_total;
    uint32_t  rx_cnt;    
    os_size_t tx_count;
    os_size_t tx_size;
    const os_uint8_t *tx_buff;
};

static struct hc_uart uarts[] = {
#ifdef BSP_USING_LPUART0
    {
        0,
        LPUART0_IRQn,
        (void *)M0P_LPUART0,
        DmaLpUart0RxTrig,
        SysctrlPeripheralLpUart0,
        LPUART0_TX_PORT, LPUART0_TX_PIN, LPUART0_TX_AF,
        LPUART0_RX_PORT, LPUART0_RX_PIN, LPUART0_RX_AF,
        &serial0,
        "lpuart0",
#ifdef  LPUART0_USING_DMA
        1
#else
        0
#endif
    },
#endif

#ifdef BSP_USING_LPUART1
    {
        1,
        LPUART1_IRQn,
        (void *)M0P_LPUART1,
        DmaLpUart1RxTrig,
        SysctrlPeripheralLpUart1,
        LPUART1_TX_PORT, LPUART1_TX_PIN, LPUART1_TX_AF,
        LPUART1_RX_PORT, LPUART1_RX_PIN, LPUART1_RX_AF,
        &serial1,
        "lpuart1",
#ifdef  LPUART1_USING_DMA
        1
#else
        0
#endif
    },
#endif

#ifdef BSP_USING_UART0
    {
        2,
        UART0_2_IRQn,
        (void *)M0P_UART0,
        DmaUart0RxTrig,
        SysctrlPeripheralUart0,
        UART0_TX_PORT, UART0_TX_PIN, UART0_TX_AF,
        UART0_RX_PORT, UART0_RX_PIN, UART0_RX_AF,
        &serial2,
        "uart0",
#ifdef  UART0_USING_DMA
        1
#else
        0
#endif
    },
#endif

#ifdef BSP_USING_UART1
    {
        3,
        UART1_3_IRQn,
        (void *)M0P_UART1,
        DmaUart1RxTrig,
        SysctrlPeripheralUart1,
        UART1_TX_PORT, UART1_TX_PIN, UART1_TX_AF,
        UART1_RX_PORT, UART1_RX_PIN, UART1_RX_AF,
        &serial3,
        "uart1",
#ifdef  UART1_USING_DMA
        1
#else
        0
#endif
    },
#endif

#ifdef BSP_USING_UART2
    {
        4,
        UART0_2_IRQn,
        (void *)M0P_UART2,
        DmaUart2RxTrig,
        SysctrlPeripheralUart2,
        UART2_TX_PORT, UART2_TX_PIN, UART2_TX_AF,
        UART2_RX_PORT, UART2_RX_PIN, UART2_RX_AF,
        &serial4,
        "uart2",
#ifdef  UART2_USING_DMA
        1
#else
        0
#endif
    },
#endif

#ifdef BSP_USING_UART3
    {
        5,
        UART1_3_IRQn,
        (void *)M0P_UART3,
        DmaUart3RxTrig,
        SysctrlPeripheralUart3,
        UART3_TX_PORT, UART3_TX_PIN, UART3_TX_AF,
        UART3_RX_PORT, UART3_RX_PIN, UART3_RX_AF,
        &serial5,
        "uart3",
#ifdef  UART3_USING_DMA
        1
#else
        0
#endif
    }
#endif
};

static void uart_isr(struct os_serial_device *serial)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if (uart->uart_device <= 1)
    {
        if(LPUart_GetStatus((M0P_LPUART_TypeDef*)(uart->huart), LPUartRC) != 0)
        {
            LPUart_ClrStatus((M0P_LPUART_TypeDef*)(uart->huart), LPUartRC);
            uart->buff[uart->rx_cnt++] = LPUart_ReceiveData((M0P_LPUART_TypeDef*)(uart->huart));

            if (uart->rx_cnt >= uart->rx_total)
            {
                LPUart_DisableIrq((M0P_LPUART_TypeDef*)(uart->huart), LPUartRxIrq);
                os_hw_serial_isr_rxdone(serial, uart->rx_cnt);
            }
        }
        
        if(LPUart_GetStatus((M0P_LPUART_TypeDef*)(uart->huart), LPUartTC) != 0)
        {
            LPUart_ClrStatus((M0P_LPUART_TypeDef*)(uart->huart), LPUartTC);
            uart->tx_count++;
            if(uart->tx_count == uart->tx_size)
            {            
                uart->tx_size = 0;
                uart->tx_count = 0;

                LPUart_DisableIrq((M0P_LPUART_TypeDef*)(uart->huart), LPUartTxIrq);
                os_hw_serial_isr_txdone(serial);
                return;
            }
            LPUart_SendDataIt((M0P_LPUART_TypeDef*)(uart->huart), uart->tx_buff[uart->tx_count]);
        }
    }
#endif

#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1) || defined(BSP_USING_UART2) || defined(BSP_USING_UART3)
    if (uart->uart_device > 1)
    {
        if(Uart_GetStatus((M0P_UART_TypeDef*)(uart->huart), UartRC) != 0)
        {
            Uart_ClrStatus((M0P_UART_TypeDef*)(uart->huart), UartRC);
            uart->buff[uart->rx_cnt++] = Uart_ReceiveData((M0P_UART_TypeDef*)(uart->huart));

            if (uart->rx_cnt >= uart->rx_total)
            {
                Uart_DisableIrq((M0P_UART_TypeDef*)(uart->huart), UartRxIrq);
                os_hw_serial_isr_rxdone(serial, uart->rx_cnt);
            }
        }
        
        if(Uart_GetStatus((M0P_UART_TypeDef*)(uart->huart), UartTC) != 0)
        {
            Uart_ClrStatus((M0P_UART_TypeDef*)(uart->huart), UartTC);
            uart->tx_count++;
            if(uart->tx_count == uart->tx_size)
            {            
                uart->tx_size = 0;
                uart->tx_count = 0;

                Uart_DisableIrq((M0P_UART_TypeDef*)(uart->huart), UartTxIrq);
                os_hw_serial_isr_txdone(serial);
                return;
            }
            Uart_SendDataIt((M0P_UART_TypeDef*)(uart->huart), uart->tx_buff[uart->tx_count]);
        }
    }
#endif

}

#ifdef BSP_USING_LPUART0
void LpUart0_IRQHandler(void)
{
    uart_isr(&serial0);
}
#endif

#ifdef BSP_USING_LPUART1
void LpUart1_IRQHandler(void)
{
    uart_isr(&serial1);
}
#endif

#ifdef BSP_USING_UART0
void Uart0_IRQHandler(void)
{
    uart_isr(&serial2);
}
#endif

#ifdef BSP_USING_UART1
void Uart1_IRQHandler(void)
{
    uart_isr(&serial3);
}
#endif

#ifdef BSP_USING_UART2
void Uart2_IRQHandler(void)
{
    uart_isr(&serial4);
}
#endif

#ifdef BSP_USING_UART3
void Uart3_IRQHandler(void)
{
    uart_isr(&serial5);
}
#endif

#if defined(LPUART0_USING_DMA) || defined(LPUART1_USING_DMA) || defined(UART0_USING_DMA)  \
    || defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA)


struct dma_channel_info
{
    uint8_t ch0_device;
    uint8_t ch1_device;
};

struct dma_channel_info dma_ch_info = {0};

void dma_channel_info_get(struct hc_uart *uart)
{
    static uint8_t dma_channel_used = 0;

    uint8_t *chan_device = (uint8_t *)&dma_ch_info;

    if(dma_channel_used >= 2)
    {
        OS_ASSERT(0);
    }

    *(uint8_t *)(chan_device + dma_channel_used) = uart->uart_device;
    
    if(dma_channel_used == 0)
    {
        uart->dma_channel = DmaCh0;
        return;
    }
    else if(dma_channel_used == 1)
    {
         uart->dma_channel = DmaCh1;
        return;
    }
    
    dma_channel_used++;

    return;
}

static void uart_dma_isr(struct os_serial_device *serial)
{
    struct hc_uart *uart;
    int cnt_data = 0;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    if (uart->dma_channel == DmaCh0)
    {
        cnt_data = uart->rx_total - M0P_DMAC->CONFA0_f.TC;
    }
    else
    {
        cnt_data = uart->rx_total - M0P_DMAC->CONFA1_f.TC;
    }
#if defined(LPUART0_USING_DMA) || defined(LPUART1_USING_DMA) || defined(UART0_USING_DMA)  \
                || defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA)
    Dma_ClrStat(uart->dma_channel);
    Dma_EnableChannel(uart->dma_channel);
    Dma_EnableChannelIrq(uart->dma_channel);
#endif
    os_hw_serial_isr_rxdone(serial, cnt_data);

}

void Dmac_IRQHandler(void)
{
    if(DmaTransferComplete == Dma_GetStat(DmaCh0))
    {
        Dma_DisableChannel(DmaCh0);
        Dma_DisableChannelIrq(DmaCh0);

        uart_dma_isr(uarts[dma_ch_info.ch0_device].serial_dev);
        
    }

    if(DmaTransferComplete == Dma_GetStat(DmaCh1))
    {
        Dma_DisableChannel(DmaCh1);
        Dma_DisableChannelIrq(DmaCh1);

        uart_dma_isr(uarts[dma_ch_info.ch1_device].serial_dev);        
    }
}

void uart_dma_cfg(struct os_serial_device *serial)
{
    struct hc_uart *uart;

    stc_dma_cfg_t stcDmaCfg;
    DDL_ZERO_STRUCT(stcDmaCfg);

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    Sysctrl_SetPeripheralGate(SysctrlPeripheralDma, TRUE);

    stcDmaCfg.u32SrcAddress = (os_uint32_t)uart->huart;
    stcDmaCfg.enRequestNum = uart->req_num;
    stcDmaCfg.u32DstAddress = (uint32_t)uart->buff;
    stcDmaCfg.enSrcAddrReloadCtl = DmaMskSrcAddrReloadDisable;
    stcDmaCfg.enSrcBcTcReloadCtl = DmaMskBcTcReloadDisable;
    stcDmaCfg.enDestAddrReloadCtl = DmaMskDstAddrReloadDisable;
    stcDmaCfg.enTransferMode = DmaMskOneTransfer;
    stcDmaCfg.enDstAddrMode = DmaMskDstAddrInc;
    stcDmaCfg.enSrcAddrMode = DmaMskSrcAddrFix;
    stcDmaCfg.u16BlockSize = 1;
    stcDmaCfg.u16TransferCnt = uart->rx_total;
    stcDmaCfg.enMode = DmaMskBlock;
    stcDmaCfg.enTransferWidth = DmaMsk8Bit;
    stcDmaCfg.enPriority = DmaMskPriorityFix;

    Dma_Enable();
    Dma_InitChannel(uart->dma_channel, &stcDmaCfg);

    EnableNvic(DMAC_IRQn, IrqLevel3, TRUE);
}
#endif

static void hc_uart_gpio_cfg(struct hc_uart *uart)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    stcGpioCfg.enDir =  GpioDirOut;
    Gpio_Init(uart->tx_port, uart->tx_pin, &stcGpioCfg);       /* TX */
    stcGpioCfg.enDir =  GpioDirIn;
    Gpio_Init(uart->rx_port, uart->rx_pin, &stcGpioCfg);       /* RX */
    Gpio_SetAfMode(uart->tx_port, uart->tx_pin, uart->tx_af);
    Gpio_SetAfMode(uart->rx_port, uart->rx_pin, uart->rx_af);
}

static os_err_t hc_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if (uart->uart_device <= 1)
    {
        stc_lpuart_cfg_t  stcCfg;
        DDL_ZERO_STRUCT(stcCfg);

        Sysctrl_SetPeripheralGate(uart->peripheral, TRUE);

        switch (cfg->stop_bits)
        {
        case STOP_BITS_1:
            stcCfg.enStopBit = LPUart1bit;
            break;
        case STOP_BITS_2:
            stcCfg.enStopBit = LPUart2bit;
            break;
        default:
            return OS_EINVAL;
        }

        switch (cfg->parity)
        {
        case PARITY_NONE:
            stcCfg.enRunMode = LPUartMskMode1;
            break;
        case PARITY_ODD:
            stcCfg.enMmdorCk = LPUartOdd;
            stcCfg.enRunMode = LPUartMskMode3;
            break;
        case PARITY_EVEN:
            stcCfg.enMmdorCk = LPUartEven;
            stcCfg.enRunMode = LPUartMskMode3;
            break;
        default:
            return OS_EINVAL;
        }

        stcCfg.stcBaud.enSclkSel = LPUartMskPclk;//LPUartMskRcl;  /* Transfer clock source RCL */
        stcCfg.stcBaud.u32Sclk = Sysctrl_GetPClkFreq();//38400;  /* RCL clock frequency 38400Hz */
        stcCfg.stcBaud.enSclkDiv = LPUartMsk4Or8Div;
        stcCfg.stcBaud.u32Baud = cfg->baud_rate;

        LPUart_Init((M0P_LPUART_TypeDef*)(uart->huart), &stcCfg);

        LPUart_ClrStatus((M0P_LPUART_TypeDef*)(uart->huart), LPUartRC);
        LPUart_ClrStatus((M0P_LPUART_TypeDef*)(uart->huart), LPUartTC);

        if (uart->dma_support == 1)
        {
            LPUart_EnableFunc((M0P_LPUART_TypeDef*)(uart->huart), LPUartDmaRxFunc);
        }
        else
        {
            LPUart_EnableIrq((M0P_LPUART_TypeDef*)(uart->huart), LPUartRxIrq);
        }
        EnableNvic(uart->irqn, IrqLevel3, TRUE);
    }
#endif

#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1) || defined(BSP_USING_UART2) || defined(BSP_USING_UART3)
    if (uart->uart_device > 1)
    {
        stc_uart_cfg_t  stcCfg;
        DDL_ZERO_STRUCT(stcCfg);

        Sysctrl_SetPeripheralGate(uart->peripheral, TRUE);

        switch (cfg->stop_bits)
        {
        case STOP_BITS_1:
            stcCfg.enStopBit = UartMsk1bit;
            break;
        case STOP_BITS_2:
            stcCfg.enStopBit = UartMsk2bit;
            break;
        default:
            return OS_EINVAL;
        }

        switch (cfg->parity)
        {
        case PARITY_NONE:
            stcCfg.enRunMode = UartMskMode1;
            break;
        case PARITY_ODD:
            stcCfg.enMmdorCk = UartMskOdd;
            stcCfg.enRunMode = UartMskMode3;
            break;
        case PARITY_EVEN:
            stcCfg.enMmdorCk = UartMskEven;
            stcCfg.enRunMode = UartMskMode3;
            break;
        default:
            return OS_EINVAL;
        }

        stcCfg.stcBaud.u32Pclk = Sysctrl_GetPClkFreq();
        stcCfg.stcBaud.enClkDiv = UartMsk8Or16Div;
        stcCfg.stcBaud.u32Baud = cfg->baud_rate;

        Uart_Init((M0P_UART_TypeDef*)(uart->huart), &stcCfg);

        Uart_ClrStatus((M0P_UART_TypeDef*)(uart->huart), UartRC);

        if (uart->dma_support == 1)
        {
            Uart_EnableFunc((M0P_UART_TypeDef*)(uart->huart), UartDmaRxFunc);
        }
        else
        {
            Uart_EnableIrq((M0P_UART_TypeDef*)(uart->huart), UartRxIrq);
        }
        
        EnableNvic(uart->irqn, IrqLevel3, TRUE);
    }
#endif

    return OS_EOK;
}


static int hc_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    uart->buff = buff;
    uart->rx_cnt = 0;
    uart->rx_total = size;

    if (uart->dma_support == 1)
    {
#if defined(LPUART0_USING_DMA) || defined(LPUART1_USING_DMA) || defined(UART0_USING_DMA)  \
                    || defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA)
        uart_dma_cfg(uart->serial_dev);
        Dma_ClrStat(uart->dma_channel);
        Dma_EnableChannel(uart->dma_channel);
        Dma_EnableChannelIrq(uart->dma_channel);
#endif
    }
    else
    {    
#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
        if (uart->uart_device <= 1)
        {
            LPUart_ClrStatus((M0P_LPUART_TypeDef*)(uart->huart), LPUartRC);
            LPUart_EnableIrq((M0P_LPUART_TypeDef*)(uart->huart), LPUartRxIrq);
        }
#endif
#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1) || defined(BSP_USING_UART2) || defined(BSP_USING_UART3)
        if (uart->uart_device > 1)
        {
            Uart_ClrStatus((M0P_UART_TypeDef*)(uart->huart), UartRC);
            Uart_EnableIrq((M0P_UART_TypeDef*)(uart->huart), UartRxIrq);
        }
#endif
    }

    return 0;
}

static int hc_uart_stop_recv(struct os_serial_device *serial)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    uart->buff = OS_NULL;
    uart->rx_cnt = 0;
    uart->rx_total = 0;

    if (uart->dma_support == 1)
    {
#if defined(LPUART0_USING_DMA) || defined(LPUART1_USING_DMA) || defined(UART0_USING_DMA)  \
            || defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA)
        Dma_ClrStat(uart->dma_channel);
        Dma_DisableChannel(uart->dma_channel);
#endif
    }
    else
    {
#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
        if (uart->uart_device <= 1)
        {
            LPUart_DisableIrq((M0P_LPUART_TypeDef*)(uart->huart), LPUartRxIrq);
        }
#endif
#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1) || defined(BSP_USING_UART2) || defined(BSP_USING_UART3)
        if (uart->uart_device > 1)
        {
            Uart_DisableIrq((M0P_UART_TypeDef*)(uart->huart), UartRxIrq);
        }
#endif
    }

    return 0;
}

#ifdef OS_SERIAL_IDLE_TIMER
static int hc_uart_recv_count(struct os_serial_device *serial)
{
    int count;
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    if (uart->dma_support == 1)
    {
#if defined(LPUART0_USING_DMA) || defined(LPUART1_USING_DMA) || defined(UART0_USING_DMA)  \
    || defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA)

        if (uart->dma_channel == DmaCh0)
        {
            count = uart->rx_total - M0P_DMAC->CONFA0_f.TC - 1;
        }
        else
        {
            count = uart->rx_total - M0P_DMAC->CONFA1_f.TC - 1;
        }
#endif
    }
    else
    {
        count = uart->rx_cnt;
    }

    return count;
}
#endif

static int hc_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct hc_uart *uart;

    uart = (struct hc_uart *)serial->parent.user_data;

    uart->tx_size = size;
    uart->tx_count = 0;
    uart->tx_buff = buff;

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if (uart->uart_device <= 1)
    {
        LPUart_ClrStatus((M0P_LPUART_TypeDef*)(uart->huart), LPUartTC);
        LPUart_EnableIrq((M0P_LPUART_TypeDef*)(uart->huart), LPUartTxIrq);
        LPUart_SendDataIt((M0P_LPUART_TypeDef*)(uart->huart), uart->tx_buff[0]);
    }
#endif
#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1) || defined(BSP_USING_UART2) || defined(BSP_USING_UART3)
    if (uart->uart_device > 1)
    {
        Uart_ClrStatus((M0P_UART_TypeDef*)(uart->huart), UartTC);
        Uart_EnableIrq((M0P_UART_TypeDef*)(uart->huart), UartTxIrq);
        Uart_SendDataIt((M0P_UART_TypeDef*)(uart->huart), uart->tx_buff[0]);
    }
#endif

    return size;
}

static int hc_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();
#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
        if (uart->uart_device <= 1)
        {
            LPUart_SendData((M0P_LPUART_TypeDef*)(uart->huart), buff[i]);
        }
#endif
#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1) || defined(BSP_USING_UART2) || defined(BSP_USING_UART3)
        if (uart->uart_device > 1)
        {
            Uart_SendDataPoll((M0P_UART_TypeDef*)(uart->huart), buff[i]);
        }
#endif
        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops hc_uart_ops =
{
    .init    = hc_configure,

    .start_send   = hc_uart_start_send,
    .stop_send    = OS_NULL, 

    .start_recv   = hc_uart_start_recv,
    .stop_recv    = hc_uart_stop_recv,

#ifdef OS_SERIAL_IDLE_TIMER
    .recv_count   = hc_uart_recv_count,
#endif

    .poll_send    = hc_uart_poll_send,
};

void __os_hw_console_output(char *str)
{
}

/**
 * @brief Initialize the UART
 *
 * This function initialize the UART
 *
 * @return None.
 */
int os_hw_usart_init(void)
{
    int i;
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    for (i = 0; i < sizeof(uarts) / sizeof(uarts[0]); i++)
    {
#if defined(LPUART0_USING_DMA) || defined(LPUART1_USING_DMA) || defined(UART0_USING_DMA)  \
        || defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA)
        dma_channel_info_get(&(uarts[i]));
#endif
        hc_uart_gpio_cfg(&(uarts[i]));

        if (uarts[i].uart_device <= 1)
        {
            config.baud_rate = BAUD_RATE_57600;      /* lpuart support baud rate 57600 */
        }
        else
        {
            if (SystemCoreClock >= 16000000)
            {
                config.baud_rate = BAUD_RATE_38400;//BAUD_RATE_115200;
            }
            else
            {
                config.baud_rate = BAUD_RATE_9600;
            }
        }

        uarts[i].serial_dev->ops    = &hc_uart_ops;
        uarts[i].serial_dev->config = config;

        /* register uart device */
        os_hw_serial_register(uarts[i].serial_dev, uarts[i].device_name, (void *)&uarts[i]);
    }

    return 0;
}
