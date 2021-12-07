# 内核日志

---

## 简介

日志\(log\)是对发生某个事件的记录，并进行显示或者保存到文件。

内核日志主要有以下特性:

- 日志输出到串口终端。
- 日志支持 4 个等级，优先级由高到低 KERN_ERROR 、KERN_WARNING 、KERN_INFO 、KERN_DEBUG，并支持分别显示红色，黄色，绿色和原始色。
- 日志支持按等级进行全局过滤，低于配置等级的将不输出日志。
- 日志支持标签方式过滤，标签设置了过滤等级后，此标签低于配置的等级将不输出日志。使用了标签过滤器，全局过滤将不再对此标签起作用，直到删除标签过滤器。
- 配置过滤器等级支持 shell 操作，简单易用。

---

## 重要定义及数据结构

### 结构体

```c
struct os_klog_tag_lvl_filter
{
    os_list_node_t list;                                    /* The filter list */
    char           tag[KLOG_FILTER_TAG_MAX_LEN + 1];        /* Tag name */
    os_uint16_t    level;                                   /* The filter level of the tag */
};
```

| struct os_klog_tag_lvl_filter 参数 | **说明**                                       |
| :--------------------------------- | :--------------------------------------------- |
| list                               | 标签过滤器链表，用于维护与其他标签过滤器的关系 |
| tag                                | 标签名称                                       |
| level                              | 标签过滤器的等级                               |

```c
struct os_klog_ctrl_info
{
    os_list_node_t tag_lvl_list;                            /* The filter list */
    os_uint16_t    global_level;                            /* The global filter level */
    char           log_buff[OS_LOG_BUFF_SIZE];              /* Buffer of formatted log information */
};
```

| struct os_klog_ctrl_info 参数 | **说明**                 |
| :---------------------------- | :----------------------- |
| tag_lvl_list                  | 所有标签过滤器           |
| global_level                  | 全局过滤器的等级         |
| log_buff                      | 格式化后日志信息的缓冲区 |

---

## API 列表

| 接口           | **说明**                                                                    |
| :------------- | :-------------------------------------------------------------------------- |
| klog_glvl_ctrl | 控制内核日志全局过滤器等级的 shell 函数实现，包括设置，获取等操作           |
| klog_tlvl_ctrl | 控制内核日志模块标签过滤器等级的 shell 函数实现，包括设置、获取、删除等操作 |

## klog_glvl_ctrl

该函数用于控制内核日志全局过滤器等级的 shell 函数实现，包括设置，获取等操作，函数原型如下:

```c
os_err_t  klog_glvl_ctrl (os_int32_t argc, char **argv);
```

| **参数**  | **说明**                                                                                                                                                                                                                      |
| :-------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| argc      | 参数个数                                                                                                                                                                                                                      |
| argv      | 参数值，可以为：-s 设置模式 -g 获取模式 -d 删除模式 -l level 要设置的全局过滤器等级 0：KERN_ERROR 1：KERN_WARNING 2：KERN_INFO 3：KERN_DEBUG shell 格式：dlog_tlvl_ctrl [-s &#124; -g &#124; -d] [-n tag name] [-l tag level] |
| **返回**  | **说明**                                                                                                                                                                                                                      |
| OS_EOK    | 操作成功                                                                                                                                                                                                                      |
| 非 OS_EOK | 操作失败                                                                                                                                                                                                                      |

## klog_tlvl_ctrl

该函数用于控制内核日志模块标签过滤器等级的 shell 函数实现，包括设置、获取、删除等操作，函数原型如下：

```c
os_err_t  klog_tlvl_ctrl (os_int32_t argc, char **argv);
```

| **参数**  | **说明**                                                                                                                                                                                                                                                                  |
| :-------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| argc      | 参数个数                                                                                                                                                                                                                                                                  |
| argv      | 参数值，可以为：-s 设置模式 -g 获取模式 -d 删除模式 -n tag name 标签名称 -l level 要设置的标签过滤器等级 要设置的标签过滤器等级 0：KERN_ERROR 1：KERN_WARNING 2：KERN_INFO 3：KERN_DEBUG shell 格式：klog_glvl_ctrl [-s &#124; -g &#124; -d] [-n tag name] [-l tag level] |
| **返回**  | **说明**                                                                                                                                                                                                                                                                  |
| OS_EOK    | 操作成功                                                                                                                                                                                                                                                                  |
| 非 OS_EOK | 操作失败                                                                                                                                                                                                                                                                  |

## 宏列表

| 宏名称      | **说明**                     |
| :---------- | :--------------------------- |
| OS_KERN_LOG | 调用日志输出函数实现日志功能 |

## OS_KERN_LOG

该宏为内核日志的实际调用接口，实际定义如下：

```c
#define OS_KERN_LOG(level, tag, fmt, ...)
```

| **参数** | **说明**                                                            |
| :------- | :------------------------------------------------------------------ |
| level    | 过滤器等级 0：KERN_ERROR 1：KERN_WARNING 2：KERN_INFO 3：KERN_DEBUG |
| tag      | 标签名称                                                            |
| fmt      | 格式化前的字符串                                                    |
| ...      | 可变参数                                                            |
| **返回** | 说明                                                                |
| 无       | 无                                                                  |

---

## 使用示例

### 调整全局过滤器等级使用示例

本示例函数打印了所有等级的日志，通过调整全局过滤器等级来屏蔽日志信息。

```c
#include <oneos_config.h>
#include <os_util.h>
#include "os_kernel_internal.h"
#include  <shell.h>

#define TEST_TAG            "TEST_TAG"

void sh_klog_global_level_test(os_int32_t argc, char **argv)
{
    os_kprintf("The Current Global level is %d\r\n", *g_klog_global_lvl);
    OS_KERN_LOG(KERN_ERROR,     TEST_TAG, "GLOBAL LEVEL KERN_ERROR");
    OS_KERN_LOG(KERN_WARNING,   TEST_TAG, "GLOBAL LEVEL KERN_WARNING");
    OS_KERN_LOG(KERN_INFO,      TEST_TAG, "GLOBAL LEVEL KERN_INFO");
    OS_KERN_LOG(KERN_DEBUG,     TEST_TAG, "GLOBAL LEVEL KERN_DEBUG");
}
SH_CMD_EXPORT(klog_global_level_test, sh_klog_global_level_test, "test klog global level");
```

运行结果如下：

```c
sh>klog_glvl_ctrl
Command format:
klog_glvl_ctrl [-s | -g] [-l global level]
parameter Usage:
         -s     Set global level option.
         -g     Get global level option.
         -l     Specify a global level that want to be set.
                level: 0-error, 1-warning, 2-info, 3-debug
sh>
sh>klog_glvl_ctrl -g
Kernel log level is: 1
sh>
sh>klog_glvl_ctrl -s -l 1
Set kernel log global level(1) success
sh>
sh>klog_global_level_test
The Current Global level is 1
[3033] E/TEST_TAG: GLOBAL LEVEL KERN_ERROR [sh_klog_global_level_test][34]
[3034] W/TEST_TAG: GLOBAL LEVEL KERN_WARNING [sh_klog_global_level_test][35]
sh>
sh>klog_glvl_ctrl -s -l 0
Set kernel log global level(0) success
sh>
sh>klog_global_level_test
The Current Global level is 0
[4414] E/TEST_TAG: GLOBAL LEVEL KERN_ERROR [sh_klog_global_level_test][34]
sh>
sh>klog_glvl_ctrl -s -l 2
Set kernel log global level(2) success
sh>
sh>klog_global_level_test
The Current Global level is 2
[6019] E/TEST_TAG: GLOBAL LEVEL KERN_ERROR [sh_klog_global_level_test][34]
[6020] W/TEST_TAG: GLOBAL LEVEL KERN_WARNING [sh_klog_global_level_test][35]
[6021] I/TEST_TAG: GLOBAL LEVEL KERN_INFO [sh_klog_global_level_test][36]
```

### 调整标签过滤器等级使用示例

本示例函数使用了两个标签，分别打印了所有等级的日志，通过设置一个标签的等级来屏蔽日志信息。

```c
#include <oneos_config.h>
#include <os_util.h>
#include "os_kernel_internal.h"
#include  <shell.h>

#define TAG_A               "TAG_A"
#define TAG_B               "TAG_B"

void sh_klog_tag_level_test(os_int32_t argc, char **argv)
{
    os_kprintf("The Current Global level is %d\r\n", *g_klog_global_lvl);
    OS_KERN_LOG(KERN_ERROR,     TAG_A, "TAG_A KERN_ERROR");
    OS_KERN_LOG(KERN_WARNING,   TAG_A, "TAG_A KERN_WARNING");
    OS_KERN_LOG(KERN_INFO,      TAG_A, "TAG_A KERN_INFO");
    OS_KERN_LOG(KERN_DEBUG,     TAG_A, "TAG_A KERN_DEBUG");

    OS_KERN_LOG(KERN_ERROR,     TAG_B, "TAG_B KERN_ERROR");
    OS_KERN_LOG(KERN_WARNING,   TAG_B, "TAG_B KERN_WARNING");
    OS_KERN_LOG(KERN_INFO,      TAG_B, "TAG_B KERN_INFO");
    OS_KERN_LOG(KERN_DEBUG,     TAG_B, "TAG_B KERN_DEBUG");
}
SH_CMD_EXPORT(klog_tag_level_test, sh_klog_tag_level_test, "test klog tag level");
```

运行结果如下：

```c
sh>klog_tlvl_ctrl
Command format:
klog_tlvl_ctrl [-s | -g | -d] [-n tag name] [-l tag level]
parameter Usage:
         -s     Set tag level option.
         -g     Get tag level option.
         -d     Delete tag level option.
         -n     Specify the tag name that want set level.
         -l     Specify a tag level that want to be set.
                level: 0-error, 1-warning, 2-info, 3-debug
sh>
sh>klog_tag_level_test
The Current Global level is 1
[1576] E/TAG_A: TAG_A KERN_ERROR [sh_klog_tag_level_test][47]
[1577] W/TAG_A: TAG_A KERN_WARNING [sh_klog_tag_level_test][48]
[1577] E/TAG_B: TAG_B KERN_ERROR [sh_klog_tag_level_test][52]
[1578] W/TAG_B: TAG_B KERN_WARNING [sh_klog_tag_level_test][53]
sh>
sh>klog_tlvl_ctrl -s -n "TAG_A" -l 0
Set kernel log tag level success, tag(TAG_A), level(0)
sh>
sh>klog_tlvl_ctrl -g -n "TAG_A"
Tag(TAG_A) level is: 0
sh>
sh>klog_tag_level_test
The Current Global level is 1
[4031] E/TAG_A: TAG_A KERN_ERROR [sh_klog_tag_level_test][47]
[4032] E/TAG_B: TAG_B KERN_ERROR [sh_klog_tag_level_test][52]
[4032] W/TAG_B: TAG_B KERN_WARNING [sh_klog_tag_level_test][53]
sh>
sh>klog_tlvl_ctrl -d -n "TAG_A"
Del tag(TAG_A) level success
sh>
sh>klog_tag_level_test
The Current Global level is 1
[5815] E/TAG_A: TAG_A KERN_ERROR [sh_klog_tag_level_test][47]
[5816] W/TAG_A: TAG_A KERN_WARNING [sh_klog_tag_level_test][48]
[5816] E/TAG_B: TAG_B KERN_ERROR [sh_klog_tag_level_test][52]
[5817] W/TAG_B: TAG_B KERN_WARNING [sh_klog_tag_level_test][53]
```
