#include <os_types.h>
#include <os_task.h>
#include <os_util.h>
#include <os_assert.h>
#include <arch_task.h>
#include <arch_exception.h>


#ifdef OS_USING_OVERFLOW_CHECK
/**
 ***********************************************************************************************************************
 * @brief           This function is used to check the stack of task is overflow or not.
 *
 * @param[in]       task            The descriptor of task control block
 * 
 * @return          Whether the stack of this task overflows.
 * @retval          OS_TRUE         The stack of this task is overflow.
 * @retval          OS_FALSE        The stack of this task is not overflow.
 ***********************************************************************************************************************
 */
os_bool_t task_stack_check(os_task_t *task)
{
    os_size_t sp;
    OS_ASSERT(OS_NULL != task);

    sp = (os_size_t)task->stack_top;

    if ((*((os_uint8_t *)task->stack_begin) != '$')
        || ((os_uint32_t)sp < (os_uint32_t)task->stack_begin)
        || ((os_uint32_t)sp >= (os_uint32_t)task->stack_end))
    {
        return OS_TRUE;
    }

    return OS_FALSE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function is used to check the task stack overflows during task switching.
 *
 * @param[in]       from_task           The from task during task switching
 * @param[in]       to_task             The to task during task switching
 * 
 * @return          None.
 ***********************************************************************************************************************
 */
void task_switch_stack_check(os_task_t *from_task, os_task_t *to_task)
{
    OS_ASSERT(OS_NULL != from_task);
    OS_ASSERT(OS_NULL != to_task);

    if (OS_TRUE == task_stack_check(from_task))
    {
        os_kprintf("[ERROR]swtich form task:%s stack overflow,the sp is 0x%x.\r\n",
                    os_task_name(from_task),
                    from_task->stack_top);

        (void)os_irq_lock();
        OS_ASSERT(0);
    }

    if (OS_TRUE == task_stack_check(to_task))
    {
        os_kprintf("[ERROR]swtich to task:%s stack overflow,the sp is 0x%x.\r\n",
                    os_task_name(to_task),
                    to_task->stack_top);

        (void)os_irq_lock();
        OS_ASSERT(0);
    }
}

#endif /* OS_USING_OVERFLOW_CHECK */

