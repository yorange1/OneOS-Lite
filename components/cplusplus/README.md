# C++支持

## 简介

`c++`组件是为了支持编译`c++`源文件。该组件把全局对象的构造函数放到了指定的段地址中，以支撑`c++`的正常使用与运行。

因为`OneOS-Lite`是很精简的`RTOS`系统，因此提供的`C++`支撑比较简单。

在`cpp_init.c`源文件中，完成了`C++`系统的初始化工作：

```c
OS_WEAK int cpp_init(void)
{

    typedef void(*pfunc)();
    extern pfunc __ctors_start__[];
    extern pfunc __ctors_end__[];
    pfunc *p;

    for (p = __ctors_start__; p < __ctors_end__; p++)
        (*p)();

    return 0;
}
OS_CMPOENT_INIT(cpp_init,OS_INIT_SUBLEVEL_LOW);
```

其中的`__ctors_start__`和`__ctors_end__`在链接脚本中被指定位置：

```c
PROVIDE(__ctors_start__ = .);
KEEP (*(SORT(.init_array.*)))
KEEP (*(.init_array))
PROVIDE(__ctors_end__ = .);
```

可见，在`cpp_init`函数中，全局对象的构造函数就被一一的链接到了链接文件所分配到的段中。然后，`OS_CMPOENT_INIT`是`OneOS-Lite`组件初始化的宏，它会把`cpp_init`函数放到组件初始化的位置进行初始化工作。

1. `__ctors_start__`：C++ 全局构造函数段的起始地址

2. `__ctors_end__`：C++ 全局构造函数段的结束地址



## 图形化配置

```
(Top) → Components→ Cplusplus
[*] cplusplus
```



## 简单体验

```c++
#include <iostream>  //包含头文件iostream
using namespace std;  //使用命名空间std

extern "C" 
{
int test(void);
}
int test( )
{
    cout<<"This is a C++ program!";
    
    return 0;
}
```

编辑一个`c++`文件，并在`main.c`文件中，调用`test`函数，编译运行，既可以使用串口打印该信息。

## 注意

`MicroLIB`不支持`c++`，需要将`options`中`Use MicroLIB`去掉勾选。

支持`c++`会增大程序的`ram`和`flash`。

最后，如果你需要使用到`c++`代码。那么，请在`SConscript`中，把`cpp`文件添加到编译系统中去。

```bash
src = Glob('*.c') + Glob('*.cpp')
```

