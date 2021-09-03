#ifdef BSP_USING_ONCHIP_RTC
#include "drv_rtc.h"

struct mm32_rtc_info rtc_info = {.hrtc = RTC, .rtc_clk = RCC_APB1ENR_PWR | RCC_APB1ENR_BKP, .rcc_init_func = RCC_APB1PeriphClockCmd};
OS_HAL_DEVICE_DEFINE("RTC_Type", "rtc", rtc_info);
#endif
                                    
                                    
                                    
                                    
