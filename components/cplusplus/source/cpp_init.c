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
 * @file        cpp_init.c
 *
 * @brief       This function support the c++ code.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include "os_stddef.h"

#if defined(__GNUC__)

OS_WEAK int cpp_init(void)
{
    /*  initialize the C++ runtime.*/
    typedef void(*pfunc)();
    extern pfunc __ctors_start__[];
    extern pfunc __ctors_end__[];
    pfunc *p;

    for (p = __ctors_start__; p < __ctors_end__; p++)
        (*p)();

    return 0;
}
#endif

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern void $Super$$__cpp_initialize__aeabi_(void);
/* if we do not add cpp_initialize,it will init error */
OS_WEAK void $Sub$$__cpp_initialize__aeabi_(void)
{
    /* do nothing */
}

OS_WEAK int cpp_init(void)
{
    /*  initialize the C++ runtime.*/
    typedef void FUNC();
    extern const unsigned long SHT$$INIT_ARRAY$$Base[];
    extern const unsigned long SHT$$INIT_ARRAY$$Limit[];

    const unsigned long *base = SHT$$INIT_ARRAY$$Base;
    const unsigned long *lim  = SHT$$INIT_ARRAY$$Limit;

    for (; base != lim; base++)
    {
        FUNC *func = (FUNC *)((const char *)base + *base);
        (*func)();
    }
    return 0;
}
#endif

OS_CMPOENT_INIT(cpp_init,OS_INIT_SUBLEVEL_LOW);
