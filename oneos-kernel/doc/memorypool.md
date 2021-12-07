# 内存池管理

---

## 简介

内存池适用于分配大量大小相同的内存块的场景，它能够快速的分配和释放内存，且能够尽量避免内存碎片化。内存池在创建时需要有一块大内存，在初始化时把这块大内存划分成大小相等的小内存块，并用链表连接起来。用户可根据实际需要创建多个内存池，不同内存池中的内存块大小可以不同。

内存池还支持任务挂起功能，即当内存池中无空闲内存块时，申请的任务会被阻塞，直到该内存池有可用内存块，任务才会被唤醒。这个特性比较适合需要通过内存资源进行同步的场景。

---

## 重要定义及数据接口

### 内存池宏定义

```c
#ifdef OS_USING_MP_CHECK_TAG
#define OS_MEMPOOL_BLK_HEAD_SIZE                4
#else
#define OS_MEMPOOL_BLK_HEAD_SIZE                0
#endif
#define OS_MEMPOOL_SIZE(block_count, block_size) ((OS_ALIGN_UP(block_size + OS_MEMPOOL_BLK_HEAD_SIZE, OS_ALIGN_SIZE)) * block_count)
```

| **内存池宏** | **说明** |
| :--- | :--- |
| OS\_MEMPOOL\_BLK\_HEAD\_SIZE | 内存池每个内存块头信息大小 |
| OS\_MEMPOOL\_SIZE\(block\_count, block\_size\) | 根据内存池块个数和内存池块大小计算内存池说下总内存空间大小 |

### 内存池结构体

```c
struct os_mempool
{
    void                       *start_addr;             /* Memory pool start. */
    void                       *free_list;              /* Avaliable memory blocks list. */
    os_size_t                   size;                   /* Size of memory pool. */
    os_size_t                   blk_size;               /* Size of memory blocks, maybe not the size for users. */
    os_size_t                   blk_total_num;          /* Numbers of memory block. */
    os_size_t                   blk_free_num;           /* Numbers of free memory block. */
    os_list_node_t              task_list_head;         /* Task suspend on this memory pool. */
    os_list_node_t              resource_node;          /* Node in resource list */
    os_uint8_t                  object_alloc_type;      /* Indicates whether object is allocated */
    os_uint8_t                  object_inited;          /* Whether inited. */
    char                        name[OS_NAME_MAX + 1];  /* mempool name. */
};
typedef struct os_mempool os_mp_t;
```

| **内存池对象成员变量** | **说明** |
| :--- | :--- |
| start\_addr | 内存池的起始地址 |
| free\_list | 空闲块链表头，所有的空闲块都挂在该链表上 |
| size | 总内存大小 |
| block\_size | 每个内存块的大小 |
| block\_list | 内存块的链表，指向下一个空闲内存块 |
| block\_total\_num | 总内存块个数 |
| block\_free\_num | 空闲内存块个数 |
| task\_list\_head | 任务阻塞队列头，任务获取内存块失败时将其阻塞在该队列上 |
| resource\_node | 资源管理节点，通过该节点将创建的内存池挂载到gs_os_mp_resource_list_head上 |
| object\_alloc\_type | 标记该内存池是静态内存池还是动态内存池 |
| object\_inited | 初始化状态，标记是否已经初始化 |
| name | 名字，名字长度不能大于OS_NAME_MAX |

```c
struct os_mpinfo 
{
    os_size_t                   blk_size;               /* Size can be used for users in one memory block. */
    os_size_t                   blk_total_num;          /* Numbers of memory block. */
    os_size_t                   blk_free_num;           /* Numbers of free memory block. */
};
```

| **内存池信息成员变量** | **说明** |
| :--- | :--- |
| blk_size | 每个内存块的大小 |
| blk_total_num | 总内存块个数 |
| blk_free_num | 空闲内存块个数 |

---

## API介绍

| **接口** | **说明** |
| :--- | :--- |
| os\_mp\_init | 使用静态的方式初始化内存池，内存池对象及内存池空间由使用者提供 |
| os\_mp\_deinit | 反初始化内存池，与os\_mp\_init\(\)匹配使用 |
| os\_mp\_create | 动态创建并初始化内存池，内存池对象的空间及内存池空间通过动态申请获取 |
| os\_mp\_destroy | 销毁内存池，与os\_mp\_create\(\)匹配使用 |
| os\_mp\_alloc | 从内存池中申请一块内存 |
| os\_mp\_free | 释放内存到内存池中，与os\_mp\_alloc\(\)匹配使用 |
| os\_mp\_info | 返回内存池对象信息 |

### os\_mp\_init

该函数使用静态的方式初始化内存池，内存池对象及内存池空间由使用者提供，函数原型如下：

```c
os_err_t os_mp_init(os_mp_t *mp, const char *name, void  *start, os_size_t size, os_size_t blk_size);
```

| **参数** | **说明** |
| :--- | :--- |
| mp | 内存池句柄 |
| name | 内存池名字 |
| start | 内存池起始地址 |
| size | 内存池大小 |
| blk\_size | 每个内存块的大小 |
| **返回** | **说明** |
| OS\_EOK | 初始化成功 |

### os\_mp\_deinit

该函数反初始化内存池，唤醒被阻塞任务，与os\_mp\_init\(\)匹配使用，函数原型如下：

```c
os_err_t os_mp_deinit(os_mp_t *mp);
```

| **参数** | **说明** |
| :--- | :--- |
| mp | 内存池句柄 |
| **返回** | **说明** |
| OS\_EOK | 反初始化成功 |

### os\_mp\_create

该函数动态创建并初始化内存池，内存池对象的空间及内存池空间都通过动态申请内存的方式获取，函数原型如下：

```c
os_mp_t *os_mp_create(const char *name, os_size_t blk_count, os_size_t blk_size)
```

| **参数** | **说明** |
| :--- | :--- |
| name | 内存池名字 |
| blk\_count | 内存块的个数 |
| blk\_size | 每个内存块的大小 |
| **返回** | **说明** |
| 非OS\_NULL | 创建成功，返回内存池句柄 |
| OS\_NULL | 创建失败 |

### os\_mp\_destroy

该函数销毁内存池，唤醒被阻塞的任务，释放内存池空间和内存池对象的空间，与os\_mp\_create\(\)匹配使用，函数原型如下：

```c
os_err_t os_mp_destroy(os_mp_t *mp);
```

| **参数** | **说明** |
| :--- | :--- |
| mp | 内存池句柄 |
| **返回** | **说明** |
| OS\_EOK | 销毁成功 |

### os\_mp\_alloc

该函数从内存池中申请一块内存，若暂时无法获取且设置了等待时间，则当前任务会阻塞，函数原型如下：

```c
void *os_mp_alloc(os_mp_t *mp, os_tick_t timeout);
```

| **参数** | **说明** |
| :--- | :--- |
| mp | 内存池句柄 |
| timeout | 暂无空闲内存块时的等待时间；若为OS\_NO\_WAIT，则不等待直接返回OS\_NULL；若为OS\_WAIT\_FOREVER，则永久等待直到有内存块可用；若为其它，则等待timeout时间或者直到有内存块可用 |
| **返回** | **说明** |
| 非OS\_NULL | 分配成功，返回内存块的地址 |
| OS\_NULL | 分配失败 |

### os\_mp\_free

该函数释放由os\_mp\_alloc\(\)申请的内存块，函数原型如下：

```c
void os_mp_free(os_mp_t *mp, void *mem);
```

| **参数** | **返回** |
| :--- | :--- |
| mp | 内存池句柄 |
| mem | 待释放的内存块地址 |
| **返回** | **说明** |
| 无 | 无 |

### os\_mp\_info

该函数返回内存池对象信息，包括内存池块大小、内存池块总个数、内存池块空闲个数，函数原型如下：

```c
void os_mp_info(os_mp_t *mp, os_mpinfo_t *info);
```

| **参数** | **说明** |
| :--- | :--- |
| heap | 内存池对象句柄 |
| info | 返回系统池对象信息 |
| **返回** | **说明** |
| 无 | 无 |

---

## 配置选项
   OneOS在使用内存池时提供了一些功能选项和裁剪的配置，具体配置如下图所示:

```
(Top) → Kernel→ Memory management
                                              OneOS Configuration
[*] Enable System-Memory-Heap
-*- Enable Memory-Heap
[*] Enable FIRSTFIT memory algorithm
[ ] Enable BUDDY memory algorithm
[ ] Enable memory trace
[*] Enable Memory-Pool
[ ]     Enable Memory-Pool check tag
 ```

| **配置项** | **说明** |
| :--- | :--- |
| Enable Memory-Pool| 使能内存池功能 |
| Enable Memory-Pool check tag| 使能内存池校验标记功能，用于校验内存块存储数据是否正确。 |

---

## 应用示例

### 静态内存池应用示例

本例初始化了一个静态内存池，并在一个任务中去申请内存块，另外一个任务中去释放内存块

```c
#include <oneos_config.h>
#include <dlog.h>
#include <os_errno.h>
#include <os_task.h>
#include <shell.h>
#include <os_memory.h>

#define TEST_TAG        "TEST"
#define TASK_STACK_SIZE 1024
#define TASK1_PRIORITY  15
#define TASK2_PRIORITY  16
#define TASK_TIMESLICE  10

#define TEST_ALLOC_CNT  10
#define BLOCK_SIZE      100
#define BLOCK_CNT       5

#define POOL_SIZE       500

static os_uint8_t mem_pool[OS_MEMPOOL_SIZE(BLOCK_CNT,BLOCK_SIZE)];
static os_mp_t mp_static;

void *block_ptr[TEST_ALLOC_CNT];

void task1_entry(void *para)
{
    os_uint32_t i = 0;

    for (i = 0 ; i < TEST_ALLOC_CNT; i++)
    {        
        LOG_W(TEST_TAG, "task1 try to alloc");
        block_ptr[i] = os_mp_alloc(&mp_static, OS_WAIT_FOREVER);
        if (block_ptr[i])
        {
            LOG_W(TEST_TAG, "task1 alloc:%p", block_ptr[i]);
        }
    }
}

void task2_entry(void *para)
{
    os_uint32_t i = 0;

    for (i = 0 ; i < TEST_ALLOC_CNT; i++)
    {
        LOG_W(TEST_TAG, "task2 free :%p", block_ptr[i]);
        os_mp_free(&mp_static, block_ptr[i]);
        block_ptr[i] = OS_NULL;
        os_task_tsleep(10);
    }
}

void mempool_static_sample(void)
{
    os_task_t *task1 = OS_NULL;
    os_task_t *task2 = OS_NULL;

    os_mp_init(&mp_static, "mempool_static", &mem_pool, sizeof(mem_pool), BLOCK_SIZE);

    task1 = os_task_create("task1", 
                           task1_entry, 
                           OS_NULL, 
                           TASK_STACK_SIZE, 
                           TASK1_PRIORITY);
    if (task1)
    {        
        LOG_W(TEST_TAG, "mempool_static_sample startup task1");
        os_task_startup(task1);
    }
    else
    {
        LOG_W(TEST_TAG, "create mempool_static_sample task1 failed");
        return;
    }

    os_task_tsleep(10);
    task2 = os_task_create("task2", 
                           task2_entry, 
                           OS_NULL, 
                           TASK_STACK_SIZE, 
                           TASK2_PRIORITY);
    if (task2)
    {        
        LOG_W(TEST_TAG, "mempool_static_sample startup task2");
        os_task_startup(task2);
    }
    else
    {
        LOG_W(TEST_TAG, "create mempool_static_sample task2 failed");
        return;
    }

    os_task_tsleep(200);

    os_mp_deinit(&mp_static);
}

SH_CMD_EXPORT(static_mempool, mempool_static_sample, "test static mempool");
```

运行结果如下：

```c
sh>static_mempool
W/TEST: mempool_static_sample startup task1
W/TEST: task1 try to alloc
W/TEST: task1 alloc:20000674
W/TEST: task1 try to alloc
W/TEST: task1 alloc:200006d8
W/TEST: task1 try to alloc
W/TEST: task1 alloc:2000073c
W/TEST: task1 try to alloc
W/TEST: task1 alloc:200007a0
W/TEST: task1 try to alloc
W/TEST: task1 alloc:20000804
W/TEST: task1 try to alloc
W/TEST: mempool_static_sample startup task2
W/TEST: task2 free :20000674
W/TEST: task1 alloc:20000674
W/TEST: task1 try to alloc
W/TEST: task2 free :200006d8
W/TEST: task1 alloc:200006d8
W/TEST: task1 try to alloc
W/TEST: task2 free :2000073c
W/TEST: task1 alloc:2000073c
W/TEST: task1 try to alloc
W/TEST: task2 free :200007a0
W/TEST: task1 alloc:200007a0
W/TEST: task1 try to alloc
W/TEST: task2 free :20000804
W/TEST: task1 alloc:20000804
W/TEST: task2 free :20000674
W/TEST: task2 free :200006d8
W/TEST: task2 free :2000073c
W/TEST: task2 free :200007a0
W/TEST: task2 free :20000804
```

### 动态内存池应用示例

本例动态创建了一个内存池，然后申请和释放内存块

```c
#include <oneos_config.h>
#include <dlog.h>
#include <os_errno.h>
#include <shell.h>
#include <string.h>
#include <os_memory.h>

#define TEST_TAG        "TEST"

#define BLOCK_CNT       5
#define BLOCK_SIZE      100

static os_mp_t *mp_dynamic;

void mempool_dynamic_sample(void)
{
    os_uint32_t i = 0;    
    void *block_ptr[BLOCK_CNT + 1];

    memset(&block_ptr[0], 0, sizeof(block_ptr));
    mp_dynamic = os_mp_create("mempool_dynamic", BLOCK_CNT, BLOCK_SIZE);
    if (mp_dynamic)
    {
        LOG_W(TEST_TAG, "mempool_dynamic_sample create mempool, bolock cnt:%d", BLOCK_CNT);
    }

    for (i = 0; i < BLOCK_CNT + 1; i++)
    {
        block_ptr[i] = os_mp_alloc(mp_dynamic, OS_NO_WAIT);
        if (block_ptr[i])
        {
            LOG_W(TEST_TAG, "alloc:%p", block_ptr[i]);
        }
        else
        {
            LOG_W(TEST_TAG, "alloc ERR");
        }
    }

    for (i = 0; i < BLOCK_CNT + 1; i++)
    {
        if (block_ptr[i])
        {            
            LOG_W(TEST_TAG, "free :%p", block_ptr[i]);
            os_mp_free(mp_dynamic, block_ptr[i]);
        }
    }

    LOG_W(TEST_TAG, "mempool_dynamic_sample destroy mempool");
    os_mp_destroy(mp_dynamic);

}

SH_CMD_EXPORT(dynamic_mempool, mempool_dynamic_sample, "test dynamic mempool");
```

运行结果如下：

```c
sh>dynamic_mempool
W/TEST: mempool_dynamic_sample create mempool, bolock cnt:5
W/TEST: alloc:20003584
W/TEST: alloc:200035e8
W/TEST: alloc:2000364c
W/TEST: alloc:200036b0
W/TEST: alloc:20003714
W/TEST: alloc ERR
W/TEST: free :20003584
W/TEST: free :200035e8
W/TEST: free :2000364c
W/TEST: free :200036b0
W/TEST: free :20003714
W/TEST: mempool_dynamic_sample destroy mempool
```