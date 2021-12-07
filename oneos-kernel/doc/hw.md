# 硬件操作抽象接口

---
## 简介

硬件操作抽象接口主要提供一些跟硬件架构强相关且操作系统运行必要的通用接口，如全局开关中断，中断上下文及开关状态查询等。
因为该接口相关函数往往直接调用硬件寄存器，跟具体硬件架构强相关，API接口声明对应体系结构的arch_interrupt.h和arch_misc.h中。

---
## API介绍

| **接口** | **说明** |
| :--- | :--- |
| os_irq_lock | 带返回的全局关中断 |
| os_irq_unlock | 恢复上次的中断状态 |
| os_irq_disable | 全局关中断 |
| os_irq_enable | 全局开中断 |
| os_is_irq_active | 查询当前是否中断上下文 |
| os_is_irq_disabled | 查询当前是否关中断 |
| os_irq_num | 查询当前中断号 |
| os_is_fault_active | 查询当前是否异常状态 |
| os_ffs | 查找最低为1位 |
| os_fls | 查找最高为1位 |
| os_get_current_task_sp | 查询当前运行任务sp指针 |

### os_irq_lock

该函数用于关闭全局中断，并返回之前中断状态，函数原型如下：

```c
os_ubase_t os_irq_lock(void);
```

| **参数** | **说明** |
| :--- | :--- |
| 无 | 无 |
| **返回** | **说明** |
| os\_base\_t | 返回本次关中断前的中断状态，一般需主动保存该中断状态并在恢复中断状态时作为参数传入 |

### os_irq_unlock

该函数用于恢复调用 os_irq_lock(\)函数前的中断状态。需要注意的是：如果调用前是关中断状态，那么调用后依然是关中断状态。恢复中断往往是和关闭中断成对使用的。函数原型如下：

```c
void os_irq_unlock(os_ubase_t irq_save);
```

| **参数** | **说明** |
| :--- | :--- |
| irq_save | 需要恢复的中断状态，一般传入上次调用os_irq_lock时保存的中断状态 |
| **返回** | **说明** |
| 无 | 无 |

### os_irq_disable

该函数不保存当前中断状态直接关闭全局中断，函数原型如下：

```c
void os_irq_disable(void);
```

| **参数** | **说明** |
| :--- | :--- |
| 无 | 无 |
| **返回** | **说明** |
| 无 | 无 |

### os_irq_enable

该函数不关心调用关中断函数前的中断状态，直接打开全局中断。函数原型如下：

```c
void os_irq_enable(void);
```

| **参数** | **说明** |
| :--- | :--- |
| 无 | 无 |
| **返回** | **说明** |
| 无 | 无 |

### os_is_irq_active

该函数用于查询当前系统是否处于中断上下文，真为中断上下文，假为非中断上下文。函数原型如下：

```c
os_bool_t os_is_irq_active(void);
```

| **参数** | **说明** |
| :--- | :--- |
| 无 | 无 |
| **返回** | **说明** |
| 1 | 处于中断上下文 |
| 0 | 处于任务上下文 |

### os_is_irq_disabled

该函数用于查询当前系统是否处于关中断状态，真为关中断状态，假为开中断状态。函数原型如下：

```c
os_bool_t os_is_irq_disabled(void);
```

| **参数** | **说明** |
| :--- | :--- |
| 无 | 无 |
| **返回** | **说明** |
| OS_TRUE | 处于关中断状态 |
| OS_FALSE | 处于开中断状态 |

### os_irq_num

该函数用于查询当前中断号。注意如果当前如处于非中断上下文，查询结果应该视为无效值。函数原型如下：

```c
os_uint32_t os_irq_num(void);
```

| **参数** | **说明** |
| :--- | :--- |
| 无 | 无 |
| **返回** | **说明** |
| os_uint32_t | 当前中断号 |

### os_is_fault_active

该函数用于查询当前系统是否处于异常状态，真为处于异常状态，假为处于任务或中断状态。注意：异常状态是指硬件厂商中断向量表中除去外部中断状态以外的状态。函数原型如下：

```c
os_bool_t os_is_fault_active(void);
```

| **参数** | **说明** |
| :--- | :--- |
| 无 | 无 |
| **返回** | **说明** |
| OS_TRUE | 处于异常状态 |
| OS_FALSE | 处于非异常状态 |

### os_ffs

该函数用于在32位整形数中，从低位到高位获取第一个值为1的bit，函数原型如下：

```c
os_int32_t os_ffs(os_uint32_t value)；
```

| **参数** | **说明** |
| :--- | :--- |
| value | 任意一个32位整形数 |
| **返回** | **说明** |
| os\_int32\_t | 从低位到高位中第一个值为1的bit。返回为0，表示未找到；1~32表示最低位为1的bit所在的位置 |

### os_fls

该函数用于在32位整形数中，从高位到低位获取第一个值为1的bit，函数原型如下：

```c
os_int32_t os_fls(os_uint32_t value)；
```

| **参数** | **说明** |
| :--- | :--- |
| value | 任意一个32位整形数 |
| **返回** | **说明** |
| os\_int32\_t | 从高位到低位中第一个值为1的bit。返回为0，表示未找到；1~32表示最高位为1的bit所在的位置 |

### os_get_current_task_sp

该函数用于返回当前sp指针值，函数原型如下：

```c
void *os_get_current_task_sp(void)；
```

| **参数** | **说明** |
| :--- | :--- |
| 无 | 无 |
| **返回** | **说明** |
| void * | 当前运行任务sp指针 |

---

## 应用示例

### 全局中断开关应用示例

本例中创建了2个任务，都会去访问临界资源（全局变量count），访问时用关全局中断来保护

```c
#include <board.h>
#include <os_task.h>
#include <shell.h>
#include "arch_task.h"

#define TEST_TAG        "TEST"
#define TASK_STACK_SIZE 1024
#define TASK1_PRIORITY  15
#define TASK2_PRIORITY  16

static os_uint32_t count = 0;

void task1_entry(void *para)
{
    os_uint32_t readcount;
    os_uint32_t level;

    while (1)
    {
        level = os_irq_lock();
        count++;
        readcount = count;
        os_irq_unlock(level);

        LOG_E(TEST_TAG, "task1_entry count:%d", readcount);
        os_task_msleep(100);
    }
}

void task2_entry(void *para)
{
    os_uint32_t readcount;
    os_uint32_t level;

    while (1)
    {
        level = os_irq_lock();
        count += 2;
        readcount = count;
        os_irq_unlock(level);

        LOG_E(TEST_TAG, "task2_entry count:%d", readcount);
        os_task_msleep(100);
    }
}

void interrupt_sample(void)
{
    os_task_t *task1 = OS_NULL;
    os_task_t *task2 = OS_NULL;

    task1 = os_task_create("task1", 
                           task1_entry, 
                           OS_NULL, 
                           TASK_STACK_SIZE, 
                           TASK1_PRIORITY);
    if (task1)
    {
        os_task_startup(task1);
    }

    task2 = os_task_create("task2", 
                           task2_entry, 
                           OS_NULL, 
                           TASK_STACK_SIZE, 
                           TASK2_PRIORITY);
    if (task2)
    {
        os_task_startup(task2);
    }
}

SH_CMD_EXPORT(interrupt_sample, interrupt_sample, "an sample of enable/disable interrupt");
```

运行结果如下：

```c
sh />interrupt_sample
E/TEST: task1_entry count:1
E/TEST: task2_entry count:3
E/TEST: task1_entry count:4
E/TEST: task2_entry count:6
E/TEST: task1_entry count:7
E/TEST: task2_entry count:9
E/TEST: task1_entry count:10
E/TEST: task2_entry count:12
E/TEST: task1_entry count:13
E/TEST: task2_entry count:15
E/TEST: task1_entry count:16
......
```



