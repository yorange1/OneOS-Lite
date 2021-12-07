#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_task.h>
#include <arch_interrupt.h>
#include <os_assert.h>
#include <os_spinlock.h>

void os_spin_lock_init(os_spinlock_t *lock)
{
    OS_ASSERT(lock);

#ifdef OS_USING_SMP
	lock->lock = (os_arch_spinlock_t)SPINLOCK_ARCH_UNLOCKED;
#endif    

#ifdef OS_USING_SPINLOCK_CHECK
	lock->owner_cpu = -1;
	lock->owner     = OS_NULL;
#endif

    return;
}

#ifdef OS_USING_SPINLOCK_CHECK
OS_INLINE void _k_spin_lock_dbg(os_spinlock_t *lock)
{
    os_task_t *task;
    
    task = os_task_self();
    if (OS_NULL != task)
    {
        OS_ASSERT_EX(*((volatile void **)(&lock->owner)) != (void *)task,
                     "Task[%s] spinlock recursion\r\n",
                     task->name);
#ifdef OS_USING_SMP
        OS_ASSERT_EX((*((volatile os_int32_t *)(&lock->owner_cpu))) != os_current_cpu_id,
                     "Cpu[%d] spinlock recursion task[%s]",
                     os_current_cpu_id,
                     task->name);
#endif
    }
    
    return;
}

OS_INLINE void _k_spin_unlock_dbg(os_spinlock_t *lock)
{	
    os_task_t *task;

    task = os_task_self();
    if (OS_NULL != task)
    {
        OS_ASSERT_EX((*((volatile void **)(&lock->owner))) == ((void *)task),
                     "Spinlock owner[%s] is not current[%s]\n",
                     (OS_NULL == lock->owner) ? "none" : ((os_task_t *)lock->owner)->name,
                     task->name);

#ifdef OS_USING_SMP
        OS_ASSERT_EX((*((volatile os_int32_t *)(&lock->owner_cpu))) == os_current_cpu_id,
                     "Spinlock owner cpu[%d] is not current cpu[%d], owner[%s] current[%s]",
                     lock->owner_cpu,
                     os_current_cpu_id,
                     (OS_NULL == lock->owner) ? "none" : ((os_task_t *)lock->owner)->name,
                     task->name);
#endif
    }
    
    return;
}


OS_INLINE void _k_spin_lock_set_owner(os_spinlock_t *lock)
{
    os_task_t *task;

    task = os_task_self();
    if (OS_NULL != task)
    {    
        *((volatile void **)(&lock->owner)) = (void *)task;

#ifdef OS_USING_SMP
        *((os_int32_t *)(&lock->owner_cpu)) = os_current_cpu_id;
#else
        *((os_int32_t *)(&lock->owner_cpu)) = 0;
#endif
    }
    
    return;
}

OS_INLINE void _k_spin_lock_cancel_owner(os_spinlock_t *lock)
{
    *(volatile void **)(&lock->owner)          = OS_NULL;
    *(volatile os_int32_t *)(&lock->owner_cpu) = -1; 

    return;
}
#endif /* OS_USING_SPINLOCK_CHECK */

void os_spin_lock(os_spinlock_t *lock)
{
    OS_ASSERT(lock);

    os_schedule_lock();

#ifdef OS_USING_SPINLOCK_CHECK
	_k_spin_lock_dbg(lock);
#endif

#ifdef OS_USING_SMP
	os_arch_spin_lock(&lock->lock);
#endif 

#ifdef OS_USING_SPINLOCK_CHECK
	_k_spin_lock_set_owner(lock);
#endif

	return;
}

void os_spin_unlock(os_spinlock_t *lock)
{
    OS_ASSERT(lock);

#ifdef OS_USING_SPINLOCK_CHECK
	_k_spin_unlock_dbg(lock);
	_k_spin_lock_cancel_owner(lock);
#endif

#ifdef OS_USING_SMP
	os_arch_spin_unlock(&lock->lock);
#endif

    os_schedule_unlock();  

	return;
}

void os_spin_lock_irqsave(os_spinlock_t *lock, os_ubase_t *irq_save)
{
    OS_ASSERT(lock);
    OS_ASSERT(irq_save);

	*irq_save = os_irq_lock();

#ifdef OS_USING_SPINLOCK_CHECK
	_k_spin_lock_dbg(lock);
#endif

#ifdef OS_USING_SMP
	os_arch_spin_lock(&lock->lock);
#endif

#ifdef OS_USING_SPINLOCK_CHECK
	_k_spin_lock_set_owner(lock);
#endif

	return;
}

void os_spin_unlock_irqrestore(os_spinlock_t *lock, os_ubase_t irq_save)
{
    OS_ASSERT(lock);

#ifdef OS_USING_SPINLOCK_CHECK
	_k_spin_unlock_dbg(lock);
    _k_spin_lock_cancel_owner(lock);
#endif

#ifdef OS_USING_SMP
	os_arch_spin_unlock(&lock->lock);
#endif

	os_irq_unlock(irq_save);

	return;
}

