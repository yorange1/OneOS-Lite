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
 * @file        board.c
 *
 * @brief       Initializes the CPU, System clocks, and Peripheral device
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_gpio.h>

#ifdef OS_USING_LED
const led_t led_table[] = 
{
    {GET_PIN(E, 3), PIN_LOW},
};

const int led_table_size = ARRAY_SIZE(led_table);
#endif

#ifdef OS_USING_PUSH_BUTTON
const struct push_button key_table[] = 
{
    {GET_PIN(A, 7),     PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},
};

const int key_table_size = ARRAY_SIZE(key_table);
#endif

#ifdef OS_USING_SN
const int board_no_pin_tab[] = 
{
    GET_PIN(B, 3),
    GET_PIN(A, 15),
    GET_PIN(A, 12),
    GET_PIN(A, 11),
    GET_PIN(A, 3),
    GET_PIN(A, 2),
    GET_PIN(A, 1),
    GET_PIN(A, 0),
};

const int board_no_pin_tab_size = ARRAY_SIZE(board_no_pin_tab);

const int slot_no_pin_tab[] = 
{
    GET_PIN(B, 0),
    GET_PIN(B, 1),
    GET_PIN(B, 5),
    GET_PIN(B, 6),
    GET_PIN(B, 7),    
};

const int slot_no_pin_tab_size = ARRAY_SIZE(slot_no_pin_tab);
#endif

void SystemClkInit_RCH(en_sysctrl_rch_freq_t enRchFreq)
{  
    Sysctrl_SetRCLTrim(SysctrlRclFreq32768);
    Sysctrl_SetRCLStableTime(SysctrlRclStableCycle64);
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
    Sysctrl_SysClkSwitch(SysctrlClkRCL);
    
    Sysctrl_SetRCHTrim(enRchFreq);
    Sysctrl_ClkSourceEnable(SysctrlClkRCH, TRUE);
    Sysctrl_SysClkSwitch(SysctrlClkRCH);
    
    Flash_WaitCycle(FlashWaitCycle0);
}

void SystemClkInit_PLL48M_byRCH(void)
{
    stc_sysctrl_pll_cfg_t stcPLLCfg;    
    
    Sysctrl_SetRCLTrim(SysctrlRclFreq32768);
    Sysctrl_SetRCLStableTime(SysctrlRclStableCycle64);
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
    Sysctrl_SysClkSwitch(SysctrlClkRCL);
    
    Sysctrl_SetRCHTrim(SysctrlRchFreq4MHz);
    Sysctrl_ClkSourceEnable(SysctrlClkRCH, TRUE);
    
    stcPLLCfg.enInFreq    = SysctrlPllInFreq4_6MHz;
    stcPLLCfg.enOutFreq   = SysctrlPllOutFreq36_48MHz;
    stcPLLCfg.enPllClkSrc = SysctrlPllRch;
    stcPLLCfg.enPllMul    = SysctrlPllMul12;
    Sysctrl_SetPLLFreq(&stcPLLCfg); 
    
    Flash_WaitCycle(FlashWaitCycle1);    
    
    Sysctrl_ClkSourceEnable(SysctrlClkPLL, TRUE);
    Sysctrl_SysClkSwitch(SysctrlClkPLL);
}

void System_cfg(void)
{
    SystemClkInit_PLL48M_byRCH();
}
