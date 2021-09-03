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
 * @file        lpmgr.c
 *
 * @brief       this file implements Low power management related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <stdlib.h>
#include <arch_interrupt.h>
#include <os_clock.h>
#include <os_timer.h>
#include <os_memory.h>
#include <os_assert.h>
#include <os_errno.h>
#include <lpmgr.h>
#include <dlog.h>

#ifdef OS_USING_TICKLESS_LPMGR

#define DBG_TAG "lpmgr"

static struct os_lpmgr_dev *gs_lpmgr = NULL;
static struct lpmgr_notify gs_lpmgr_notify;
static os_list_node_t lp_register_dev_list = OS_LIST_INIT(lp_register_dev_list);

#define LPMGR_TICKLESS_THRESH (2)


struct os_lpmgr_dev *os_lpmgr_dev_get(void)
{
    return gs_lpmgr;
}

OS_WEAK os_uint32_t lpmgr_enter_critical(os_uint8_t sleep_mode)
{
    return os_irq_lock();
}

OS_WEAK void lpmgr_exit_critical(os_uint32_t ctx, os_uint8_t sleep_mode)
{
    os_irq_unlock(ctx);
}

static int lpmgr_device_suspend(lpmgr_sleep_mode_e mode)
{
    int ret = OS_EOK;
    os_ubase_t level;
    struct os_lpmgr_dev *lpm;
    os_list_node_t *register_dev;
    os_list_node_t *pos;
    os_list_node_t *n;
    struct os_lpmgr_register_dev *tmp_dev;

    lpm = os_lpmgr_dev_get();
    register_dev = lpm->register_dev;
    level = os_irq_lock();
    os_list_for_each_safe(pos,n,register_dev)
    {        
        tmp_dev = os_list_entry(pos, struct os_lpmgr_register_dev, lp_node);

        if (tmp_dev->lp_device.ops->suspend != OS_NULL)
        {
            ret = tmp_dev->lp_device.ops->suspend(tmp_dev->lp_device.device, mode);
            if (ret != OS_EOK)
                break;
        }
    }
    os_irq_unlock(level);
    return ret;
}

static void lpmgr_device_resume(lpmgr_sleep_mode_e mode)
{
    os_ubase_t level;
    struct os_lpmgr_dev *lpm;
    os_list_node_t *register_dev;    
    os_list_node_t *pos;
    os_list_node_t *n;
    struct os_lpmgr_register_dev *tmp_dev;

    lpm = os_lpmgr_dev_get();
    register_dev = lpm->register_dev;
    level = os_irq_lock();
    os_list_for_each_safe(pos,n,register_dev)
    {        
        tmp_dev = os_list_entry(pos, struct os_lpmgr_register_dev, lp_node);

        if (tmp_dev->lp_device.ops->resume != OS_NULL)
        {
            tmp_dev->lp_device.ops->resume(tmp_dev->lp_device.device, mode);
        }
    }
    os_irq_unlock(level);
}


os_tick_t timer_next_timeout_tick(void)
{
    return os_tickless_get_sleep_ticks();
}

int lpmgr_enter_sleep_call(lpmgr_sleep_mode_e mode)
{
    int ret = OS_EOK;
    /* Notify app will enter sleep mode */
    if (gs_lpmgr_notify.notify)
        gs_lpmgr_notify.notify(SYS_ENTER_SLEEP, mode, gs_lpmgr_notify.data);

    /* Suspend all peripheral device */
    ret = lpmgr_device_suspend(mode);
    if (ret != OS_EOK)
    {
        lpmgr_device_resume(mode);
        if (gs_lpmgr_notify.notify)
            gs_lpmgr_notify.notify(SYS_EXIT_SLEEP, mode, gs_lpmgr_notify.data);

        return OS_ERROR;
    }

    return OS_EOK;
}

int lpmgr_exit_sleep_call(lpmgr_sleep_mode_e mode)
{
    /* resume all device */
    lpmgr_device_resume(mode);

    if (gs_lpmgr_notify.notify)
        gs_lpmgr_notify.notify(SYS_EXIT_SLEEP, mode, gs_lpmgr_notify.data);

    return OS_EOK;
}

OS_WEAK os_err_t other_check(struct os_lpmgr_dev *lpm)
{
    return OS_EOK;
}


os_err_t is_enter_lpmgr(struct os_lpmgr_dev *lpm)
{

    if (lpm == NULL)
    {
        return OS_EEMPTY;
    }

    if (!lpm->init_falg)
    {
        return OS_EFULL;
    }

    if (!(lpm->sleep_mask & (1 << lpm->sleep_mode)))
    {
        return OS_EINVAL;
    }

    if (OS_EOK != other_check(lpm))
    {
        return OS_EBUSY;
    }

    return OS_EOK;
}

void lpmgr_updatetick(struct os_lpmgr_dev *lpm,os_tick_t delt_tick, os_tick_t timeout_tick)
{
    os_tick_t update_tick;

    if (delt_tick < timeout_tick)
    {
        if((timeout_tick - delt_tick) <= lpm->min_tick)
        {
            update_tick = timeout_tick;
        }
        else
        {
            update_tick = delt_tick;
        }
    }
    else
    {
        update_tick = timeout_tick;
    }

    os_tickless_update(update_tick);
}

os_err_t lpmgr_start(struct os_lpmgr_dev *lpm)
{
    os_err_t ret = OS_EOK;
    os_tick_t delt_tick = 0;
    os_tick_t timeout_tick = 0;
    const struct os_lpmgr_ops *lpm_ops = lpm->ops;

    ret = lpmgr_enter_sleep_call(lpm->sleep_mode);
    if (ret != OS_EOK)
    {
        return ret;
    }
    timeout_tick = timer_next_timeout_tick();
    if (timeout_tick < lpm->min_tick)
    {
        lpmgr_exit_sleep_call(lpm->sleep_mode);
        return OS_EBUSY;
    }

    /* timeout_tick = OS_TICK_MAX is sleep all the time */
    if (timeout_tick != OS_TICK_MAX)
    {
        if (timeout_tick > lpm->max_tick)
        {
            timeout_tick = lpm->max_tick;
        }

        lpm_ops->sleeptick_start(lpm, timeout_tick);
    }
    lpm_ops->sleep(lpm->sleep_mode);

    if (timeout_tick != OS_TICK_MAX)
    {
        /* get lptim tick */
        delt_tick = lpm_ops->sleeptick_gettick();
        lpm_ops->sleeptick_stop();

        /* update lptim to systick */
        lpmgr_updatetick(lpm,delt_tick, timeout_tick);

    }

    lpmgr_exit_sleep_call(lpm->sleep_mode);
    LOG_I(DBG_TAG, "[%s]-[%d], timeout_tick[%u], delt_tick[%u], cur_tick[%d]\r\n", __FILE__, __LINE__, timeout_tick, delt_tick, os_tick_get());

    return OS_EOK;
}


void os_low_power_manager(void)
{
    os_err_t ret;
    struct os_lpmgr_dev *lpm;
    os_uint32_t level;

    lpm = os_lpmgr_dev_get();
    if (lpm == NULL)
    {
        return;
    }

    if (lpm->register_dev == NULL)
    {
        return;
    }

    level = lpmgr_enter_critical(lpm->sleep_mode);
    ret = is_enter_lpmgr(lpm);
    if (ret == OS_EOK)
    {
        lpmgr_start(lpm);
    }
    lpmgr_exit_critical(level, lpm->sleep_mode);
}


static void lpmgr_check_sleep_mode(struct os_lpmgr_dev *lpm)
{
    lpmgr_sleep_mode_e index;
    lpmgr_sleep_mode_e sleepmode = lpm->sleep_mode;

    for (index = SYS_SLEEP_MODE_NONE; index < SYS_SLEEP_MODE_MAX; index++)
    {
        if (lpm->modes[index])
        {
            sleepmode = index;
            break;
        }
    }

    if (lpm->sleep_mode != sleepmode)
    {
        lpm->sleep_mode = sleepmode;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Upper application or device driver requests the system stall in corresponding power mode
 *
 * @param[in]       sleep_mode                the parameter of run mode or sleep mode
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_request(lpmgr_sleep_mode_e sleep_mode)
{
    os_base_t     level;
    struct os_lpmgr_dev *lpm;
    lpm = os_lpmgr_dev_get();

    if (lpm->init_falg != 1)
        return;

    if (sleep_mode > (SYS_SLEEP_MODE_MAX - 1))
        return;

    level = os_irq_lock();
    if (lpm->modes[sleep_mode] < 255)
        lpm->modes[sleep_mode]++;

    lpmgr_check_sleep_mode(lpm);

    os_irq_unlock(level);
}

/**
 ***********************************************************************************************************************
 * @brief           Upper application or device driver releases the system stall in corresponding power mode
 *
 * @param[in]       sleep_mode                the parameter of run mode or sleep mode
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_release(lpmgr_sleep_mode_e sleep_mode)
{
    os_ubase_t    level;
    struct os_lpmgr_dev *lpm;

    lpm = os_lpmgr_dev_get();

    if (lpm->init_falg != 1)
        return;

    if (sleep_mode > (SYS_SLEEP_MODE_MAX - 1))
        return;

    level = os_irq_lock();
    if (lpm->modes[sleep_mode] > 0)
        lpm->modes[sleep_mode]--;

    lpmgr_check_sleep_mode(lpm);

    os_irq_unlock(level);
}



/**
 ***********************************************************************************************************************
 * @brief           Register a device with PM feature
 *
 * @param[in]       device              pointer of os device with lpmgr feature
 * @param[in]       ops                 pointer of lpmgr device operation function set
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
os_err_t os_lpmgr_device_register(struct os_device *device, const struct os_lpmgr_device_ops *ops)
{
    os_ubase_t level;
    struct os_lpmgr_dev *lpm;
    os_list_node_t *register_dev;    
    os_list_node_t *pos;
    os_list_node_t *n;
    struct os_lpmgr_register_dev *tmp_dev;

    lpm = os_lpmgr_dev_get();
    register_dev = &lp_register_dev_list;
    level = os_irq_lock();
    os_list_for_each_safe(pos,n,register_dev)
    {        
        tmp_dev = os_list_entry(pos, struct os_lpmgr_register_dev, lp_node);

        if ((tmp_dev->lp_device.device == device) && (tmp_dev->lp_device.ops == ops))
        {
            os_irq_unlock(level);
            LOG_D(DBG_TAG, "dev[%s], ops[%p] alread register!\n", device->name, ops);
            return OS_EOK;
        }
    }
    os_irq_unlock(level);
    tmp_dev = (struct os_lpmgr_register_dev *)os_malloc(sizeof(struct os_lpmgr_register_dev));

    if(tmp_dev == OS_NULL)
    {
        return OS_ENOMEM;
    }

    tmp_dev->lp_device.device = device;
    tmp_dev->lp_device.ops = ops;

    level = os_irq_lock();
    os_list_add_tail(register_dev,&tmp_dev->lp_node);
    os_irq_unlock(level);
    LOG_D(DBG_TAG, "register dev[%s], ops[%p]!\n", device->name, ops);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Unregister device from low power manager
 *
 * @param[in]       device              pointer of device with lpmgr feature
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_device_unregister(struct os_device *device, const struct os_lpmgr_device_ops *ops)
{
    os_ubase_t level;
    struct os_lpmgr_dev *lpm;
    os_list_node_t *register_dev;    
    os_list_node_t *pos;
    os_list_node_t *n;
    struct os_lpmgr_register_dev *tmp_dev;

    lpm = os_lpmgr_dev_get();
    register_dev = &lp_register_dev_list;
    level = os_irq_lock();
    os_list_for_each_safe(pos,n,register_dev)
    {        
        tmp_dev = os_list_entry(pos, struct os_lpmgr_register_dev, lp_node);

        if ((tmp_dev->lp_device.device == device) && (tmp_dev->lp_device.ops == ops))
        {
            os_list_del(&tmp_dev->lp_node);
            os_free(tmp_dev);
            os_irq_unlock(level);
            return;
        }
    }
    os_irq_unlock(level);
}

/**
 ***********************************************************************************************************************
 * @brief           set notification callback for application
 *
 * @param[in]       notify              pointer of notify
 * @param[in]       data                pointer of data
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_notify_set(void (*notify)(os_lpmgr_sys_e event, lpmgr_sleep_mode_e mode, void *data), void *data)
{
    gs_lpmgr_notify.notify = notify;
    gs_lpmgr_notify.data   = data;

    LOG_D(DBG_TAG, "register lpmgr_notify notify[%p]!\n", notify);
}


os_err_t  lpmgr_device_init(os_device_t *dev)
{
    struct os_lpmgr_dev *lpm;

    lpm = (struct os_lpmgr_dev *)dev;
    OS_ASSERT(lpm != OS_NULL);

    lpm->init_falg = 1;

    return OS_EOK;
}


const static struct os_device_ops lpm_ops = {
    .init    = lpmgr_device_init,
};

/**
 ***********************************************************************************************************************
 * @brief           initialize low power manager
 *
 * @param[in]       ops             pointer of lpmgr operation function set
 * @param[in]       timer_mask      indicates which mode has timer feature
 * @param[in]       user_data       user_data
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_register(struct os_lpmgr_dev  *lpmgr, os_uint8_t sleep_mask, void *user_data)
{
    struct os_device *device;
    struct os_lpmgr_dev *lpm;

    lpm    = lpmgr;
    device = &(lpmgr->parent);

    device->type        = OS_DEVICE_TYPE_PM;

    device->ops = &lpm_ops;
    device->user_data = user_data;

    /* register low power manager device to the system */
    os_device_register(device, OS_LPMGR_DEVICE_NAME);

    lpm->sleep_mode = SYS_SLEEP_MODE_IDLE;
    lpm->sleep_mask = sleep_mask;

    lpm->register_dev = &lp_register_dev_list;

}

os_device_t *os_lpmgr_set_device(const char *name)
{
    os_device_t *new;

    /* Find new console device */
    new = os_device_find(name);
    if (OS_NULL != new)
    {
        /* Set new console device */
        os_device_open(new);

        gs_lpmgr = (struct os_lpmgr_dev *)new;
    }

    return new;
}

os_err_t os_lpmgr_dev_init(void)
{
    os_lpmgr_set_device(OS_LPMGR_DEVICE_NAME);

    return OS_EOK;
}

OS_PREV_INIT(os_lpmgr_dev_init, OS_INIT_SUBLEVEL_LOW);


#ifdef OS_USING_SHELL
#include <shell.h>

static const char *gs_lpmgr_sleep_str[] = SYS_SLEEP_MODE_NAMES;

static void lpmgr_release_mode(int argc, char **argv)
{
    lpmgr_sleep_mode_e mode = SYS_SLEEP_MODE_NONE;
    if (argc >= 2)
    {
        mode = (lpmgr_sleep_mode_e)atoi(argv[1]);
    }

    os_lpmgr_release(mode);
}
SH_CMD_EXPORT(power_release, lpmgr_release_mode, "release power management mode");

static void lpmgr_request_mode(int argc, char **argv)
{
    lpmgr_sleep_mode_e mode = SYS_SLEEP_MODE_NONE;
    if (argc >= 2)
    {
        mode = (lpmgr_sleep_mode_e)atoi(argv[1]);
    }

    os_lpmgr_request(mode);
}
SH_CMD_EXPORT(power_request, lpmgr_request_mode, "request power management mode");


static void lpmgr_dump_status(void)
{
    os_uint32_t   index;
    struct os_lpmgr_dev *lpm;

    lpm = gs_lpmgr;

    os_kprintf("| Power Management Mode | Counter | Timer |\n");
    os_kprintf("+-----------------------+---------+-------+\n");
    for (index = 0; index < SYS_SLEEP_MODE_MAX; index++)
    {
        int has_timer = 0;
        if (lpm->sleep_mask & (1 << index))
            has_timer = 1;

        os_kprintf("| %021s | %7d | %5d |\n", gs_lpmgr_sleep_str[index], lpm->modes[index], has_timer);
    }
    os_kprintf("+-----------------------+---------+-------+\n");

    os_kprintf("lpmgr current sleep mode: %s\n", gs_lpmgr_sleep_str[lpm->sleep_mode]);
}
SH_CMD_EXPORT(power_status, lpmgr_dump_status, "dump power management status");

static void lpmgr_dump_dev(void)
{
    os_ubase_t level;
    struct os_lpmgr_dev *lpm;
    os_list_node_t *register_dev;    
    os_list_node_t *pos;
    os_list_node_t *n;
    struct os_lpmgr_register_dev *tmp_dev;
    os_uint32_t device_num = 0;

    lpm = os_lpmgr_dev_get();
    register_dev = lpm->register_dev;
    level = os_irq_lock();
    os_list_for_each_safe(pos,n,register_dev)
    {
        tmp_dev = os_list_entry(pos, struct os_lpmgr_register_dev, lp_node);
        os_kprintf("| %-5d | %-20s | 0x%p |\n\r", device_num++, tmp_dev->lp_device.device->name, tmp_dev->lp_device.ops);        
    }
    os_irq_unlock(level);
    os_kprintf("total register num: %d\n\r",device_num);

}
SH_CMD_EXPORT(power_dev, lpmgr_dump_dev, "dump power management dev");


#endif


#endif /* OS_USING_TICKLESS_LPMGR */

