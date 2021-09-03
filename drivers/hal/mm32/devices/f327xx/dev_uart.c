#ifdef BSP_USING_UART
#include "drv_uart.h"

#ifdef BSP_USING_UART1
static mm32_uart_info_t uart1_info = 
{
    .huart                                              = UART1,
    .uart_clk                                           = RCC_APB2Periph_UART1,
    .tx_pin_info.GPIO_Pin                               = GPIO_Pin_9,
    .tx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .tx_pin_info.GPIO_Mode                              = GPIO_Mode_AF_PP,
    .tx_pin_source                                      = GPIO_PinSource9,
    .tx_pin_port                                        = GPIOA,
    .rx_pin_info.GPIO_Pin                               = GPIO_Pin_10,
    .rx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .rx_pin_info.GPIO_Mode                              = GPIO_Mode_IPU,
    .rx_pin_source                                      = GPIO_PinSource10,
    .rx_pin_port                                        = GPIOA,
    .pin_af                                             = GPIO_AF_7,
    .uart_info.UART_BaudRate                            = 115200U,
    .uart_info.UART_WordLength                          = UART_WordLength_8b,
    .uart_info.UART_StopBits                            = UART_StopBits_1,
    .uart_info.UART_Parity                              = UART_Parity_No,
    .uart_info.UART_Mode                                = UART_Mode_Rx | UART_Mode_Tx,
    .uart_info.UART_HardwareFlowControl                 = UART_HardwareFlowControl_None,
    .uart_nvic_info.NVIC_IRQChannel                     = UART1_IRQn,
    .uart_nvic_info.NVIC_IRQChannelPreemptionPriority   = 2,
    .uart_nvic_info.NVIC_IRQChannelSubPriority          = 0,
    .uart_nvic_info.NVIC_IRQChannelCmd                  = ENABLE,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", uart1_info);
#endif

#ifdef BSP_USING_UART2
static mm32_uart_info_t uart2_info = 
{
    .huart                                              = UART2,
    .uart_clk                                           = RCC_APB1Periph_UART2,
    .tx_pin_info.GPIO_Pin                               = GPIO_Pin_2,
    .tx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .tx_pin_info.GPIO_Mode                              = GPIO_Mode_AF_PP,
    .tx_pin_source                                      = GPIO_PinSource2,
    .tx_pin_port                                        = GPIOA,
    .rx_pin_info.GPIO_Pin                               = GPIO_Pin_3,
    .rx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .rx_pin_info.GPIO_Mode                              = GPIO_Mode_IPU,
    .rx_pin_source                                      = GPIO_PinSource3,
    .rx_pin_port                                        = GPIOA,
    .pin_af                                             = GPIO_AF_7,
    .uart_info.UART_BaudRate                            = 115200U,
    .uart_info.UART_WordLength                          = UART_WordLength_8b,
    .uart_info.UART_StopBits                            = UART_StopBits_1,
    .uart_info.UART_Parity                              = UART_Parity_No,
    .uart_info.UART_Mode                                = UART_Mode_Rx | UART_Mode_Tx,
    .uart_info.UART_HardwareFlowControl                 = UART_HardwareFlowControl_None,
    .uart_nvic_info.NVIC_IRQChannel                     = UART2_IRQn,
    .uart_nvic_info.NVIC_IRQChannelPreemptionPriority   = 0,
    .uart_nvic_info.NVIC_IRQChannelSubPriority          = 0,
    .uart_nvic_info.NVIC_IRQChannelCmd                  = ENABLE,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", uart2_info);
#endif

#ifdef BSP_USING_UART6
static mm32_uart_info_t uart6_info = 
{
    .huart                                              = UART6,
    .uart_clk                                           = RCC_APB2ENR_UART6,
    .tx_pin_info.GPIO_Pin                               = GPIO_Pin_0,
    .tx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .tx_pin_info.GPIO_Mode                              = GPIO_Mode_AF_PP,
    .tx_pin_source                                      = GPIO_PinSource0,
    .tx_pin_port                                        = GPIOB,
    .rx_pin_info.GPIO_Pin                               = GPIO_Pin_1,
    .rx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .rx_pin_info.GPIO_Mode                              = GPIO_Mode_IPU,
    .rx_pin_source                                      = GPIO_PinSource1,
    .rx_pin_port                                        = GPIOB,
    .pin_af                                             = GPIO_AF_8,
    .uart_info.UART_BaudRate                            = 115200U,
    .uart_info.UART_WordLength                          = UART_WordLength_8b,
    .uart_info.UART_StopBits                            = UART_StopBits_1,
    .uart_info.UART_Parity                              = UART_Parity_No,
    .uart_info.UART_Mode                                = UART_Mode_Rx | UART_Mode_Tx,
    .uart_info.UART_HardwareFlowControl                 = UART_HardwareFlowControl_None,
    .uart_nvic_info.NVIC_IRQChannel                     = UART6_IRQn,
    .uart_nvic_info.NVIC_IRQChannelPreemptionPriority   = 0,
    .uart_nvic_info.NVIC_IRQChannelSubPriority          = 0,
    .uart_nvic_info.NVIC_IRQChannelCmd                  = ENABLE,
    .dma_info.DMA_PeripheralInc                         = DMA_PeripheralInc_Disable,
    .dma_info.DMA_MemoryInc                             = DMA_MemoryInc_Enable,
    .dma_info.DMA_PeripheralDataSize                    = DMA_PeripheralDataSize_Byte,
    .dma_info.DMA_MemoryDataSize                        = DMA_MemoryDataSize_Byte,
    .dma_info.DMA_Mode                                  = DMA_Mode_Normal,
    .dma_info.DMA_Priority                              = DMA_Priority_Low,
    .dma_info.DMA_M2M                                   = DMA_M2M_Disable,
    .dma_info.DMA_Auto_reload                           = DMA_Auto_Reload_Disable,
    .dma_tx_clk                                         = RCC_AHBENR_DMA2,
    .dma_tx_channel                                     = DMA2_Channel4,
    .dma_tx_nvic_info.NVIC_IRQChannel                   = DMA2_Channel4_IRQn,
    .dma_tx_nvic_info.NVIC_IRQChannelPreemptionPriority = 2,
    .dma_tx_nvic_info.NVIC_IRQChannelSubPriority        = 0,
    .dma_tx_nvic_info.NVIC_IRQChannelCmd                = ENABLE,
    .dma_rx_clk                                         = RCC_AHBENR_DMA1,
    .dma_rx_channel                                     = DMA1_Channel1,
    .dma_rx_nvic_info.NVIC_IRQChannel                   = DMA1_Channel1_IRQn,
    .dma_rx_nvic_info.NVIC_IRQChannelPreemptionPriority = 2,
    .dma_rx_nvic_info.NVIC_IRQChannelSubPriority        = 0,
    .dma_rx_nvic_info.NVIC_IRQChannelCmd                = ENABLE,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart6", uart6_info);
#endif

#ifdef BSP_USING_UART8
static mm32_uart_info_t uart8_info = 
{
    .huart                                              = UART8,
    .uart_clk                                           = RCC_APB1ENR_UART8,
    .tx_pin_info.GPIO_Pin                               = GPIO_Pin_1,
    .tx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .tx_pin_info.GPIO_Mode                              = GPIO_Mode_AF_PP,
    .tx_pin_source                                      = GPIO_PinSource1,
    .tx_pin_port                                        = GPIOE,
    .rx_pin_info.GPIO_Pin                               = GPIO_Pin_0,
    .rx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .rx_pin_info.GPIO_Mode                              = GPIO_Mode_IPU,
    .rx_pin_source                                      = GPIO_PinSource0,
    .rx_pin_port                                        = GPIOE,
    .pin_af                                             = GPIO_AF_8,
    .uart_info.UART_BaudRate                            = 115200U,
    .uart_info.UART_WordLength                          = UART_WordLength_8b,
    .uart_info.UART_StopBits                            = UART_StopBits_1,
    .uart_info.UART_Parity                              = UART_Parity_No,
    .uart_info.UART_Mode                                = UART_Mode_Rx | UART_Mode_Tx,
    .uart_info.UART_HardwareFlowControl                 = UART_HardwareFlowControl_None,
    .uart_nvic_info.NVIC_IRQChannel                     = UART8_IRQn,
    .uart_nvic_info.NVIC_IRQChannelPreemptionPriority   = 0,
    .uart_nvic_info.NVIC_IRQChannelSubPriority          = 0,
    .uart_nvic_info.NVIC_IRQChannelCmd                  = ENABLE,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart8", uart8_info);
#endif

#endif
