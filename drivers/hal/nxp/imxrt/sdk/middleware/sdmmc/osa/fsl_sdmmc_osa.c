/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sdmmc_osa.h"

#include <string.h>
#include <os_event.h>
#include <os_types.h>
#include <dlog.h>
#include <os_clock.h>
#include <os_task.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.sdmmc_osa"
#include <drv_log.h>


/*******************************************************************************
 * Definitons
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * brief Initialize OSA.
 */
void SDMMC_OSAInit(void)
{
    /* Intentional empty */
}

/*!
 * brief OSA Create event.
 * param event handle.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventCreate(void *eventHandle)
{
    assert(eventHandle != NULL);
    
    return os_event_init(eventHandle, "sdmmc_event");
}

/*!
 * brief Wait event.
 *
 * param eventHandle The event type
 * param eventType Timeout time in milliseconds.
 * param timeoutMilliseconds timeout value in ms.
 * param event event flags.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventWait(void *eventHandle, uint32_t eventType, uint32_t timeoutMilliseconds, uint32_t *event)
{
    assert(eventHandle != NULL);
    
    os_event_t *event_per = eventHandle;
    if (os_event_recv(event_per, eventType, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, 
        os_tick_from_ms(timeoutMilliseconds), (os_uint32_t *)event) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "wait event timeout");
        return OS_ERROR;
    } 

    return OS_EOK;
}

/*!
 * brief set event.
 * param event event handle.
 * param eventType The event type
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventSet(void *eventHandle, uint32_t eventType)
{
    assert(eventHandle != NULL);
    os_event_t *event = eventHandle;
    return os_event_send(event, eventType);
}

/*!
 * brief Get event flag.
 * param eventHandle event handle.
 * param eventType The event type
 * param flag pointer to store event value.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventGet(void *eventHandle, uint32_t eventType, uint32_t *flag)
{
    assert(eventHandle != NULL);
    assert(flag != NULL);
    
    os_event_t *event = eventHandle;
    
    *flag = event->set & eventType;

    return OS_EOK;
}

/*!
 * brief clear event flag.
 * param eventHandle event handle.
 * param eventType The event type
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventClear(void *eventHandle, uint32_t eventType)
{
    assert(eventHandle != NULL);

    os_event_t *event = eventHandle;
    
    event->set &= ~eventType;

    return OS_EOK;
}

/*!
 * brief Delete event.
 * param event The event handle.
 */
status_t SDMMC_OSAEventDestroy(void *eventHandle)
{
    os_event_deinit(eventHandle);

    return OS_EOK;
}

/*!
 * brief sdmmc delay.
 * param milliseconds time to delay
 */
void SDMMC_OSADelay(uint32_t milliseconds)
{
    os_task_msleep(milliseconds);
}
