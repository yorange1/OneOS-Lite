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
    .tx_pin_port                                        = GPIOA,
    .rx_pin_info.GPIO_Pin                               = GPIO_Pin_10,
    .rx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .rx_pin_info.GPIO_Mode                              = GPIO_Mode_IN_FLOATING,
    .rx_pin_port                                        = GPIOA,
    .uart_info.UART_BaudRate                            = 115200U,
    .uart_info.UART_WordLength                          = UART_WordLength_8b,
    .uart_info.UART_StopBits                            = UART_StopBits_1,
    .uart_info.UART_Parity                              = UART_Parity_No,
    .uart_info.UART_Mode                                = UART_Mode_Rx | UART_Mode_Tx,
    .uart_info.UART_HardwareFlowControl                 = UART_HardwareFlowControl_None,
    .uart_nvic_info.NVIC_IRQChannel                     = UART1_IRQn,
    .uart_nvic_info.NVIC_IRQChannelPriority             = 0,
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
    .tx_pin_port                                        = GPIOA,
    .rx_pin_info.GPIO_Pin                               = GPIO_Pin_3,
    .rx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .rx_pin_info.GPIO_Mode                              = GPIO_Mode_IN_FLOATING,
    .rx_pin_port                                        = GPIOA,
    .uart_info.UART_BaudRate                            = 115200U,
    .uart_info.UART_WordLength                          = UART_WordLength_8b,
    .uart_info.UART_StopBits                            = UART_StopBits_1,
    .uart_info.UART_Parity                              = UART_Parity_No,
    .uart_info.UART_Mode                                = UART_Mode_Rx | UART_Mode_Tx,
    .uart_info.UART_HardwareFlowControl                 = UART_HardwareFlowControl_None,
    .uart_nvic_info.NVIC_IRQChannel                     = UART2_IRQn,
    .uart_nvic_info.NVIC_IRQChannelPriority             = 0,
    .uart_nvic_info.NVIC_IRQChannelCmd                  = ENABLE,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", uart2_info);
#endif

#endif
