# C是如何调用C++接口的？

> ✨有的时候，当我想在c代码中调用c++接口，然后就编译报错了！！！

## 引出问题

C++支持函数重载的, 函数名相同但是参数不同的重载函数在编译后链接的名字并不相同而可以被识别, 这种情况下, 我们引入一个中间层的方法同样可以实现C中调用C++的函数接口。

其实这与C中调用C++非重载基本函数成员的实现没有什么区别, 只是为各个重载函数均实现一套接口而已。

## 通常的做法

```c++
//test.cpp
#include <iostream>
#include "hello.h"
int getNum() {
    std::cout << "get number" << std::endl;
    return 123456;
}
```

这时，我们需要对该`c++`函数，进行声明，告诉编译器，需要按c的命名规则来：

```c++
//test.h
#ifdef __cplusplus
extern "C" {
#endif

int getNum();

#ifdef __cplusplus
}
#endif
#endif
```

这时，我们就可以在`C`源码中调用`getNum`接口了。

## 重载函数

如果你想在 `C` 里调用重载函数，则必须提供不同名字的包装，这样才能被 `C` 代码调用。首先是我们的`C++`接口, 如下所示：

```cpp
//  add.cpp
//#include <iostream>
int add(const int a, const int b)
{
    return (a + b);
}

double add(const double a, const double b)
{
    //std::cout <<a <<", " <<b <<std::endl;
    return (a + b);
}
```

我们为此实现一个中间层`libadd.cpp`, 通过`C++`编译器用`extern "C"`将其编译成C编译器可识别的接口:

```cpp
// libadd.cpp
int add(const int a, const int b);
double add(const double a, const double b);

#ifdef __cplusplus
extern "C"
{
#endif

int call_cpp_add_int(const int a, const int b)
{
    return add(a, b);
}

double call_cpp_add_double(const double a, const double b)
{
    return add(a, b);
}

#ifdef __cplusplus
}
#endif
```

最后是我们的C源程序, 调用我们的中间层:

```cpp
//  main.c
#include <stdio.h>
#include <stdlib.h>


int call_cpp_add_int(const int a, const int b);
double call_cpp_add_double(const double a, const double b);

int main( )
{
    printf("2 + 4 = %d\n", call_cpp_add_int(2, 4));
    printf("2.1 + 4.5 = %lf\n", call_cpp_add_double(2.1, 4.5));

    return 0;
}
```

唔，完美！

当然也可以把函数声明括起来~

```cpp
// libadd.cpp
int add(const int a, const int b);
double add(const double a, const double b);

#ifdef __cplusplus
extern "C"
{
#endif

int call_cpp_add_int(const int a, const int b);
double call_cpp_add_double(const double a, const double b);

#ifdef __cplusplus
}
#endif
int call_cpp_add_int(const int a, const int b)
{
    return add(a, b);
}

double call_cpp_add_double(const double a, const double b)
{
    return add(a, b);
}
```

