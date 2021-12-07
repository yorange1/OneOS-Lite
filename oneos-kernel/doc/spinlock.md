# 自旋锁

---

自旋锁（Spinlock）在内核中主要是用来在多核（SMP）环境下代替全局开关中断保护内核临界区资源使用，由于在多核环境下，内核进入临界区操作时一般无法继续沿用单核环境下的全局关中断保护方法，即同时关闭所有核心的中断响应，否则会造成系统响应效率过于低下。使用自旋锁保护临界资源时，获得锁的任务进入临界区执行，未获得锁的任务在试图获取锁的循环中“自旋”等待，直到获得锁为止再进入临界区执行。

相比于互斥锁等阻塞类型锁，自旋锁属于非阻塞类型，因此在使用过程中要注意保护区内执行时间要尽量短，防止导致其他任务在“自旋”等待中耗时过长，另外要防止出现“死锁”的情况出现。

### 自旋锁实现原理

对于单核环境：由全局开关中断或全局开关调度实现；

对于多核环境：取决于硬件架构的内存互斥访问实现方式，因此跟架构相关，一般由原子操作命令“原子比较后交换（os_atomic_cmpxchg）”实现，任务获取锁时先判断该自旋锁是否已被获取，若已被其他任务获取则重复执行原子操作等待锁释放。

---

## 重要定义及数据结构

### 自旋锁对象结构体

内核通过结构体定义自旋锁类型如下。

```c
struct os_spinlock
{
#ifdef OS_USING_SMP
	os_arch_spinlock_t  lock;
#endif

#ifdef OS_USING_SPINLOCK_CHECK
	os_int32_t          owner_cpu;
	void               *owner;
#endif

#if !defined(OS_USING_SMP) && !defined(OS_USING_SPINLOCK_CHECK)
	os_uint8_t          padding;
#endif
};
typedef struct os_spinlock os_spinlock_t;
```

| **自旋锁对象成员变量** | **说明** |
| :--- | :--- |
| lock | 自旋锁计数值，当OneOS内核打开多核支持时用以获得锁、释放锁使用 |
| owner_cpu | 获得锁的cpu id，当OneOS内核打开自旋锁检查选项时存放获得锁的cpu id号，单核下默认为-1 |
| owner | 获得锁的任务控制块指针，当OneOS内核打开自旋锁检查选项时存放获得锁的任务控制块指针 |
| padding | 当内核未支持多核和自旋锁检查时，填充值防止结构体为空，默认为0 |


### 自旋锁类型变量初始化宏定义

内核提供一个初始化宏用于自旋锁类型变量的定义及初始化，建议用户使用该宏同时定义并初始化自旋锁类型变量。

```c
#define OS_DEFINE_SPINLOCK(var) struct os_spinlock var =
{
    SPIN_LOCK_INIT
    SPINLOCK_DEBUG_INIT
    SPINLOCK_PADDING_INIT
}
```

---

## API列表

| **接口** | **说明** |
| :--- | :--- |
| os_spin_lock_init | 自旋锁初始化 |
| os_spin_lock | 自旋锁获取 |
| os_spin_unlock | 自旋锁释放 |
| os_spin_lock_irqsave | 带关中断的自旋锁获取 |
| os_spin_unlock_irqrestore | 带中断恢复的自旋锁释放 |

## os_spin_lock_init

自旋锁初始化接口，如果自定义一个自旋锁变量而未使用“OS_DEFINE_SPINLOCK(var)”宏定义并初始化该变量，则应该调用该函数进行初始化，函数原型如下：

```c
void os_spin_lock_init(os_spinlock_t *lock);
```

| **参数** | **说明** |
| :--- | :--- |
| lock | 要初始化的自旋锁变量指针 |
| **返回** | **说明** |
| 无 | 无 |

## os_spin_lock

自旋锁获取，对于单核相当于全局关调度，对于多核先全局关调度再获取真正的自旋锁，函数原型如下：

```c
void os_spin_lock(os_spinlock_t *lock);
```

| **参数** | **说明** |
| :--- | :--- |
| lock | 要获取的自旋锁变量指针 |
| **返回** | **说明** |
| 无 | 无 |

## os_spin_unlock

自旋锁释放，对于单核相当于全局开调度，对于多核先释放真正的自旋锁再全局开调度，函数原型如下：

```c
void os_spin_unlock(os_spinlock_t *lock)；
```

| **参数** | **说明** |
| :--- | :--- |
| lock | 要释放的自旋锁变量指针 |
| **返回** | **说明** |
| 无 | 无 |

## os_spin_lock_irqsave

带关中断的自旋锁获取，对于单核相当于全局关中断，对于多核先全局关中断再获取真正的自旋锁，函数原型如下：

```c
void os_spin_lock_irqsave(os_spinlock_t *lock, os_ubase_t *irqsave);
```

| **参数** | **说明** |
| :--- | :--- |
| lock | 要获取的自旋锁变量指针 |
| irqsave | 关中断之前的中断状态，一般需要传入带中断恢复的自旋锁释放中使用 |
| **返回** | **说明** |
| 无 | 无 |

## os_spin_unlock_irqrestore

带中断恢复的自旋锁释放，对于单核相当于恢复为传入的中断状态，对于多核先释放真正的自旋锁再恢复为传入的中断状态，函数原型如下：

```c
void os_spin_unlock_irqrestore(os_spinlock_t *lock, os_ubase_t irqsave);
```

| **参数** | **说明** |
| :--- | :--- |
| lock | 要释放的自旋锁变量指针 |
| irqsave | “os_spin_lock_irqsave”中保存的中断状态，注意如果之前的中断状态为关中断，则恢复后仍处于关中断状态 |
| **返回** | **说明** |
| 无 | 无 |

---

## 配置选项

OneOS在使用自旋锁时提供了功能裁剪的配置，具体配置如下图所示:

![](./images/spinlock_check.png)

| **配置项** | **说明** |
| :--- | :--- |
| Enable spinlock check | 使能自旋锁检查，如果不使能该功能，则不检查获得自旋锁的任务控制块指针和cpu id号，默认使能 |

---

## 使用示例

### 单核自旋锁获取、释放使用示例

本例在单核系统下初始化一个自旋锁变量，执行带关中断的自旋锁获取命令后，查询系统关中断状态，再执行带中断恢复的自旋锁释放后，再次查询系统关中断状态。

```c
#include <board.h>
#include <os_task.h>
#include <shell.h>
#include "arch_interrupt.h"
#include "os_spinlock.h"

#define TEST_TAG "TEST"
void test_spinlock(void)
{
    OS_DEFINE_SPINLOCK(test_spinlock);
    os_ubase_t level = 0;

    os_spin_lock_irqsave(&test_spinlock, &level);

    if (os_is_irq_disabled())
    {
        LOG_W(TEST_TAG, "OS enter irq disabled context.");
    }
	  
    os_spin_unlock_irqrestore(&test_spinlock, level);
		
    if (os_is_irq_disabled() == 0)
    {
        LOG_W(TEST_TAG, "OS quit irq disabled context.");
    }
}
SH_CMD_EXPORT(test_spinlock, test_spinlock, "an sample of spinlock");
```

运行结果如下：

```c
sh />test_spinlock
W/TEST: OS enter irq disabled context.
W/TEST: OS quit irq disabled context.
sh />
```
