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
 * @file        main.c
 *
 * @brief       User application entry
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_util.h>
#include <stdint.h>
#include "oneos_config.h"
#include "board.h"

#include "mo_api.h"
#include "m5311.h"

#include <cmiot_user.h>

/*test ota demo in main task*/
#define USE_OTA_DEMO      1

/* M5311 */
#define M5311_DEV_NAME      "uart0"
#define M5311_DEV_RATE      9600



static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;
mo_object_t *module;

static void user_task(void *parameter)
{
#ifdef OS_USING_LED
    int i = 0;

    for (i = 0; i < led_table_size; i++)
    {
        os_pin_mode(led_table[i].pin, PIN_MODE_OUTPUT);
    }

    while (1)
    {
        for (i = 0; i < led_table_size; i++)
        {
            os_pin_write(led_table[i].pin, led_table[i].active_level);
            os_task_msleep(200);

            os_pin_write(led_table[i].pin, !led_table[i].active_level);
            os_task_msleep(200);
        }
    }
#endif
}

/* M5311 module init */
os_err_t m5311_module_init(void)
{
    os_device_t *device = os_device_find(M5311_DEV_NAME);

    if (OS_NULL == device)
    {
        os_kprintf("Create failed,not find M5311 device %s!\n", M5311_DEV_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = M5311_DEV_RATE;
    uart_config.rx_bufsz  = M5311_RECV_BUFF_LEN;
    
    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = M5311_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = M5311_RECV_BUFF_LEN
                                       };

    module = mo_create(M5311_NAME, MODULE_TYPE_M5311, &parser_config);

    if (OS_NULL == module)
    {
        os_kprintf("Create failed,not create %s module!\n", M5311_NAME);
        return OS_ERROR;
    }

    os_kprintf("Create %s module success!\n", M5311_NAME);
    return OS_EOK;
}
/* M5311 module deinit */
os_err_t m5311_module_deinit(void)
{
    mo_destroy(module, MODULE_TYPE_M5311);
    return OS_EOK;
}

#define NB_POWER_RST_PIN     63
#define NB_PWR_PIN_IDX       86
#define NB_ON_OFF_PIN_IDX    87

void mod_nb_poweron(void)
{
    
    os_pin_mode(NB_POWER_RST_PIN, PIN_MODE_OUTPUT);
    os_pin_mode(NB_PWR_PIN_IDX, PIN_MODE_OUTPUT);
    os_pin_mode(NB_ON_OFF_PIN_IDX, PIN_MODE_OUTPUT);
    
    /*power on*/
    os_pin_write(NB_PWR_PIN_IDX, PIN_LOW); 
    
    /*hard reset*/
    os_pin_write(NB_PWR_PIN_IDX, PIN_LOW); 
    os_task_msleep(10);
    os_pin_write(NB_PWR_PIN_IDX, PIN_HIGH); 
    os_task_msleep(500);
    os_pin_write(NB_PWR_PIN_IDX, PIN_LOW);

    /*pwr key*/
    os_pin_write(NB_ON_OFF_PIN_IDX, PIN_HIGH);
    os_task_msleep(10);
    os_pin_write(NB_ON_OFF_PIN_IDX, PIN_LOW);
    os_task_msleep(1100);
    os_pin_write(NB_ON_OFF_PIN_IDX, PIN_HIGH);

}

int main(void)
{
#if USE_OTA_DEMO
    cmiot_int8 rst = E_CMIOT_FAILURE;
    os_err_t ret = OS_EOK;

    mod_nb_poweron();

    m5311_module_init();

    os_task_msleep(10000);
    
    os_kprintf("----------------hello world %s ------------\r\n", CMIOT_FIRMWARE_VERSION);
    os_kprintf("---------------------r9000p---------------------------\r\n");
    
//    os_kprintf("[%d]report upgrade.\n",os_tick_get());
//    for (int i = 0; i < 1; i++)
//    {
//        rst = cmiot_report_upgrade();
//        if (0 == rst)   /* report success */
//        {
//            os_kprintf("[%d]report success\n",os_tick_get());
//            break;
//        }
//        else if (-5 == rst)   /* no new upgrade */
//        {
//            os_kprintf("[%d]no new upgrade\n",os_tick_get());
//            break;
//        }
//        else
//        {
//            ret = OS_ERROR;
//        }
//    }
//    os_kprintf("[%d]start upgrade.\n",os_tick_get());

    for (int i = 0; i < 1; i++)
    {
        rst = cmiot_upgrade();
        if (0 == rst)   /* new version and upgrade success */
        {
            os_kprintf("[%d]upgrade success\n",os_tick_get());
            os_kprintf("reset the mcu..\n");
            NVIC_SystemReset();
            break;
            
        }
        else if (-4 == rst)   /* no new version */
        {
            os_kprintf("[%d]no new version\n",os_tick_get());
            ret = OS_EOK;
            break;
        }
        else
        {
            os_kprintf("[%d]upgrade err!\n",os_tick_get());
            ret = OS_ERROR;
        }
        os_task_msleep(1000);
    }    

    //m5311_module_deinit();
#endif
    user_task(OS_NULL);
    return 0;
}

