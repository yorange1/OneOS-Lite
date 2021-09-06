# 内存管理-序言

## 概述

内存是计算机系统中除了处理器以外最重要的资源，用于存储当前正在执行的程序和数据。内存管理模块则是用于管理`RTOS`系统的内存资源，它是`RTOS`操作系统的核心模块之一。主要包括内存的初始化、分配以及释放。

内存管理的方式有很多，从分配内存是否连续的角度来看，可以分为两大类：**连续内存管理**、**非连续内存管理**。在嵌入式系统中，**连续内存管理**是最常用的内存管理方式。它为进程分配的内存空间是连续的，这种分配方式容易形成内存碎片，进而降低内存使用率。

在**快速上手**环节，我们希望通过对内存管理进行一次系统的剖析，让开发者们了解，`OneOS-Lite`所使用的内存管理方式都有哪些，从而能够更好地使用它。

## 算法

```c
static struct alg_init_func alg_init_table[] = {
#ifdef OS_USING_ALG_FIRSTFIT
    {OS_MEM_ALG_FIRSTFIT, k_firstfit_mem_init},
#endif
#ifdef OS_USING_ALG_BUDDY
    {OS_MEM_ALG_BUDDY,    k_buddy_mem_init},
#endif
#ifdef OS_USING_ALG_SMALL
    {OS_MEM_ALG_SMALL,    k_small_mem_init},
#endif
    {OS_MEM_ALG_DEFAULT,  OS_NULL}
};
```

通过该结构体，我们了解到，`OneOS-Lite`支持的内存管理算法主要有三个：

- First Fit（首次适应算法）
- buddy（伙伴算法）
- small（小内存算法）

## 知识框图

我们将按照源码的书写方式：

- 讲解内存管理的框架结构(`os_memory.c`)
- 讲解内存管理的算法(`os_mem_small.c`、`os_mem_firstfit.c`、`os_mem_buddy.c`)
- 讲解内存池的使用(`os_mem_pool.c`)

