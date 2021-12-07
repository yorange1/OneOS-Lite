# 断言

---

## 简介

assert()断言，用于在调试过程中捕捉程序的错误。一般格式为assert(expression)

断言，用于在调试过程中捕捉程序的错误。

“断言”在语文中的意思是“断定”、“十分肯定地说”，在编程中是指对某种假设条件进行检测，如果条件成立就不进行任何操作，如果条件不成立就捕捉到这种错误，并打印出错误信息，终止程序执行。

assert() 会对表达式expression进行检测：
如果expression的结果为 0（条件不成立），那么断言失败，表明程序出错，assert() 会向标准输出设备打印一条错误信息，并调用 abort() 函数终止程序的执行。
如果expression的结果为非 0（条件成立），那么断言成功，表明程序正确，assert() 不进行任何操作。

要打印的错误信息的格式和内容没有统一的规定，这和标准库的具体实现有关（也可以说和编译器有关），但是错误信息至少应该包含以下几个方面的信息：
 - 断言失败的表达式，也即expression；
 - 源文件名称；
 - 断言失败的代码的行号。

OneOS提供了以下两种断言方式:
 - 标准格式断言:OS_ASSERT(condition)
 - 带额外提示信息的断言OS_ASSERT_EX(condition, fmt, ...)
 
---

## OS_ASSERT

标准格式断言，定义如下：

```c
#define OS_ASSERT(condition)                                                                                \
do                                                                                                          \
{                                                                                                           \
    if (!(condition))                                                                                       \
    {                                                                                                       \
        os_kprintf("Assert failed. Condition(%s). [%s][%d]\r\n", #condition, __FUNCTION__, __LINE__);       \
        while(1)                                                                                            \
        {                                                                                                   \
            ;                                                                                               \
        }                                                                                                   \
    }                                                                                                       \
} while (0)
```

当断言中的条件为假时，会打印出错的函数名、行号以及断言的内容，然后进入死循环。

## OS_ASSERT_EX

带提示信息的断言主要用来判断与设计预期是否相符，并且可以添加自己的提示信息，定义如下：

```c
#define OS_ASSERT_EX(condition, fmt, ...)                                                                   \
    do                                                                                                      \
    {                                                                                                       \
        if (!(condition))                                                                                   \
        {                                                                                                   \
            os_kprintf("Assert failed. " fmt " [%s][%d]\r\n", ##__VA_ARGS__, __FUNCTION__, __LINE__);       \
            while (1)                                                                                       \
            {                                                                                               \
                ;                                                                                           \
            }                                                                                               \
        }                                                                                                   \
    } while (0)
```

当断言中的条件为假时，会打印出错的错误提示信息、函数名、行号以及断言的内容，然后进入死循环。

## 配置选项

OneOS提供了一些配置选项用于裁剪系统中的断言，具体配置如下所示:

```
(Top) → Kernel
                                              OneOS Configuration
    The max size of kernel object name (15)  --->
    The max level value of priority of task (32)  --->
(100) Tick frequency(Hz)
(10) Task time slice(unit: tick)
[ ] Using stack overflow checking
[ ] Using task hook
[ ] Enable global assert
[ ] Enable kernel lock check
[ ] Enable function safety mechanism
[ ] Enable kernel debug
(2048) The stack size of main task
(1024) The stack size of idle task
(512) The stack size of recycle task
[*] Enable software timer with a timer task
(512)   The stack size of timer task
[ ]     Software timers in each hash bucket are sorted
[*]     Enable workqueue
[*]         Enable system workqueue
(2048)          System workqueue task stack size
(8)             System workqueue task priority level
    Inter-task communication and synchronization  --->
    Memory management  --->
```

| **配置项** | **说明** |
| :--- | :--- |
| Enable global assert| 使能系统assert |


## 应用示例

### OS_ASSERT应用示例

本例演示了OS_ASSERT的常规使用方法

```c
#include <oneos_config.h>
#include <shell.h>
#include <os_assert.h>

void assert_sample(void)
{
    int value;
    
    value = 100;
    
    OS_ASSERT(value == 100);
    OS_ASSERT(value != 100);
}

SH_CMD_EXPORT(assert, assert_sample, "test OS_ASSERT")

```

运行结果如下：

```c
sh />assert
Assert failed. Condition(value != 100). [assert_sample][37]
```

### OS_ASSERT_EX应用示例

本例演示了OS_ASSERT_EX的常规使用方法

```c
#include <oneos_config.h>
#include <shell.h>
#include <os_assert.h>

void assertex_sample(void)
{
    int value;
    
    value = 100;
    
    OS_ASSERT_EX(value == 100, "value must be eqaul to 100");
    OS_ASSERT_EX(value != 100, "value must not be eqaul to 100");
}

SH_CMD_EXPORT(assertex, assertex_sample, "test OS_ASSERT_EX")

```

运行结果如下：

```c
sh />assertex
Assert failed. value must not be eqaul to 100 [assertex_sample][40]
```
