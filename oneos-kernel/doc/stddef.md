# 通用宏定义

---

## 简介

为了规避编译器的不同,OneOS内核在os_stddef.h中定义了一些编译器指令相关的宏。为了使用方便，OneOS内核把一些通用的宏也在此文件定义。

---


## 编译器指令相关宏定义

```c
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#define __CLANG_ARM
#endif

/* Compiler related definitions */
#if defined(__CC_ARM) || defined(__CLANG_ARM)   /* For ARM compiler */
    #include <stdarg.h>
    
    #define OS_SECTION(x)               __attribute__((section(x)))
    #define OS_ALIGN(n)                 __attribute__((aligned(n)))
    #define OS_UNUSED                   __attribute__((unused))
    #define OS_USED                     __attribute__((used))
    #define OS_WEAK                     __attribute__((weak))
    #define OS_INLINE                   static __inline
#elif defined (__IAR_SYSTEMS_ICC__)             /* For IAR compiler */
    #include <stdarg.h>
    
    #define OS_SECTION(x)               @ x
    #define OS_PRAGMA(x)                _Pragma(#x)
    #define OS_ALIGN(n)                 OS_PRAGMA(data_alignment=n)
    #define OS_UNUSED
    #define OS_USED                     __root
    #define OS_WEAK                     __weak
    #define OS_INLINE                   static inline
#elif defined (__GNUC__)                        /* For GNU GCC compiler */
    #include <stdarg.h>

    #define OS_SECTION(x)               __attribute__((section(x)))
    #define OS_ALIGN(n)                 __attribute__((aligned(n)))
    #define OS_UNUSED                   __attribute__((unused))
    #define OS_USED                     __attribute__((used))
    
    #define OS_WEAK                     __attribute__((weak))
    #define OS_INLINE                   static __inline
#else
    #error "Not supported the tool chain."
#endif
```

| **定义** | **说明** |
| :--- | :--- |
| OS\_SECTION\(x\) | 段定义，将变量或函数放到指定的段中 |
| OS\_ALIGN\(n\) | 字节对齐，作用是在给某对象分配地址空间时，指定n字节对齐，n一般为2的幂次方 |
| OS\_UNUSED | 表示函数或者变量可能不使用，避免编译器产生告警 |
| OS\_USED | 告诉编译器静态函数即使没有被调用也要链接 |
| OS\_WEAK | 编译器链接时优先链接没有该关键字的函数，如果找不到才链接由该关键字修饰的函数 |
| OS\_INLINE | 内联函数 |

---

## 自动初始化相关宏定义

该功能用于在系统初始化过程中，自动执行被OS\_INIT\_EXPORT修饰的函数，其中的数字越小，优先级越高，越先被执行。定义如下：

```c
typedef os_err_t (*os_init_fn_t)(void);

#define OS_INIT_SUBLEVEL_HIGH          "1"
#define OS_INIT_SUBLEVEL_MIDDLE        "2"
#define OS_INIT_SUBLEVEL_LOW           "3"

#define OS_INIT_EXPORT(fn, level, sublevel)                                                           \
    OS_USED const os_init_fn_t  _os_call_##fn OS_SECTION(".init_call." level sublevel) = fn

/* Core init routines(hardware initialization required for kernel starting) should be called before the kernel starts */
#define OS_CORE_INIT(fn, sublevel)      OS_INIT_EXPORT(fn, "1.", sublevel)

/* postcore/prev/device/component/env/app init routines will be called before main() function */
#define OS_POSTCORE_INIT(fn, sublevel)  OS_INIT_EXPORT(fn, "2.", sublevel)  /* hardware initialization after kernel starting */
#define OS_PREV_INIT(fn, sublevel)      OS_INIT_EXPORT(fn, "3.", sublevel)  /* Pre-initialization(pure software initilization) */
#define OS_DEVICE_INIT(fn, sublevel)    OS_INIT_EXPORT(fn, "4.", sublevel)  /* Device initialization */
#define OS_CMPOENT_INIT(fn, sublevel)   OS_INIT_EXPORT(fn, "5.", sublevel)  /* Components initialization (vfs, lwip, ...) */
#define OS_ENV_INIT(fn, sublevel)       OS_INIT_EXPORT(fn, "6.", sublevel)  /* Environment initialization (mount disk, ...) */
#define OS_APP_INIT(fn, sublevel)       OS_INIT_EXPORT(fn, "7.", sublevel)  /* Appliation initialization */
```

| **定义** | **说明** |
| :--- | :--- |
| OS\_CORE\_INIT\(fn\) | 硬件自动初始化，运行在操作系统内核启动之前 |
| OS\_POSTCORE\_INIT\(fn\) | 硬件自动初始化，运行在操作系统内核启动之后 |
| OS\_PREV\_INIT\(fn\) | 前置自动初始化，主要用于纯软件且没有太多依赖的初始化 |
| OS\_DEVICE\_INIT\(fn\) | 设备驱动级自动初始化 |
| OS\_CMPOENT\_INIT\(fn\) | 组件级自动初始化 |
| OS\_ENV\_INIT\(fn\) | 环境级自动初始化 |
| OS\_APP\_INIT\(fn\) | 应用程序级自动初始化 |

---

## 其它宏定义

```c
#ifdef __cplusplus
#define OS_NULL                         0
#else
#define OS_NULL                         ((void *)0)
#endif

/* Boolean value definitions */
#define OS_FALSE                        0
#define OS_TRUE                         1


/* Return the most contiguous size aligned at specified width. OS_ALIGN_UP(13, 4) would return 16. */
#define OS_ALIGN_UP(size, align)        (((size) + (align) - 1) & ~((align) - 1))

/* Return the down number of aligned at specified width. OS_ALIGN_DOWN(13, 4) would return 12. */
#define OS_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

/* Calculate array size */
#define OS_ARRAY_SIZE(x)                (sizeof(x) / sizeof(x[0]))

#define OS_WAIT_FOREVER                 OS_TICK_MAX
#define OS_NO_WAIT                      0U

#define OS_UNREFERENCE(x)               ((void)(x))

/**
 ***********************************************************************************************************************
 * @def         os_container_of
 *
 * @brief       Cast a member of a structure out to the containing structure.
 *
 * @param       ptr             The pointer to the member.
 * @param       type            The type of the container struct this is embedded in.
 * @param       member          The name of the member within the struct. 
 ***********************************************************************************************************************
 */
#define os_container_of(ptr, type, member)      \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 ***********************************************************************************************************************
 * @def         os_offsetof
 *
 * @brief       This macro will return a byte offset of a member to the beginning of the struct.
 *
 * @param       type            The type of the struct
 * @param       member          The name of the member within the struct.
 ***********************************************************************************************************************
 */
#define os_offsetof(type, member)       ((os_size_t) &((type *)0)->member)
```

| **定义** | **说明** |
| :--- | :--- |
| OS\_ALIGN\_UP\(size, align\) | 数字大小向上对齐，如OS\_ALIGN\_UP\(5,4\)，返回8 |
| OS\_ALIGN\_DOWN\(size, align\) | 数字大小向下对齐，如OS\_ALIGN\_DOWN\(5,4\)，返回4 |
| OS\_ARRAY\_SIZE | 求数组尺寸 |
| OS\_WAIT\_FOREVER | 任务阻塞,永久等待，直到获取资源 |
| OS\_NO\_WAIT | 任务不阻塞,有资源返回成功,无资源返回失败 |
| OS\_UNREFERENCE(x) | 使用未使用的局部变量,避免编译时提示告警信息 |
| os\_container\_of\(ptr, type, member\) | 已知结构体type的成员member的地址ptr，求结构体type的起始地址 |
| os\_offsetof\(type, member\) | 求成员member在结构体type内的偏移 |



