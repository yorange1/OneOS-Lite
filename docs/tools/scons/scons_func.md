# SCons常用语法

---

## SCons常用函数

OneOs scons脚本涉及到的函数分为两部分：标准函数，自定义函数。这里简单介绍几个常用的函数。其它的可以参照已经写好的脚本进行编写或者上网查询更详细的scons语法。Scons是基于python实现的，因此它的脚本也支持标准python语法。

#### Import\(\)、Export\(\)函数

Import\(vars\)

导入其它脚本定义的变量

‘vars’变量名，变量名是字符串，可以是一个变量或一个变量列表

Export\(vars\)

导出变量，供其它脚本使用

‘vars’变量名，变量名是字符串，可以是一个变量或一个变量列表

#### SConscript\(\)函数

SConscript\(scripts, \[exports, variant\_dir, duplicate\]\)

读取一个或多个sconscript脚本，返回一个node列表，node是指一个编译对象

‘scripts’：指定要加载的sconscript脚本名称与路径

‘exports’：可选参数，导出一个变量，此变量比一般Export\(\)导出的变量有更高优先权

‘variant\_dir’：可选参数，指定一个目录，编译动作都在该目录中进行，不影响源码路径

‘duplicate’：可选参数，指定编译目录中是否拷贝源码

#### PresentDir\(\)函数

PresentDir\(\)

获取当前脚本所在路径

#### Glob\(\)函数

Glob\(pattern\)

返回满足pattern指定条件的Node\(编译对象\)

‘pattern’：指定匹配条件，支持unix shell的通配符替换。支持当前脚本所在路径进行相对路径索引

#### IsDefined\(\)函数

IsDefined\(depend\)

判断依赖的宏或宏列表是否被定义，被定义返回True，如果存在未被定义的宏返回False

‘depend’：指定依赖的宏，或宏列表

#### DeleteSrcFile\(\)函数

DeleteSrcFile\(src, remove\)

将指定的文件从编译列表中移除

‘src’：编译node列表

‘remove’：指定被移除的文件

#### AddCodeGroup\(\)函数

AddCodeGroup\(name, src, depend, \*\*parameters\)

将编译对象添加到一个代码组中进行编译管理，返回编译对象列表

‘name’：指定代码组名称，如果该名称已经存在，则将添加到已经存在的组中。在keil工程中以该名称呈现一个工作组

‘src’：指定要被添加的编译对象列表

‘depend’：关键字参数，指定创建该组依赖的宏，如果条件宏不满足，则直接返回空列表

‘parameters’：可变关键字参数，指定相应的编译行为，支持的关键字如下：

CPPPATH指定头文件路径，对应gcc的-I

CCFLAGS对应gcc的--include选项

CPPDEFINES定义编译宏，对应gcc的-D

LINKFLAGS定义链接选项

LOCAL\_CPPPATH

定义仅对当前组有效的头文件路径选项

---

## SCons的编译环境类

scons中的一个基本类是构造环境Environment，scons对编译源码的管理，以及对编译行为的控制都是通过配置构造环境\(Env\)来管理的。通过调用Environment\(\)可以创建一个构造环境对象（Env）

配置Env对象中的各个变量，我们可以方便的定义需要的编译行为。简单举例：

通过CPPPATH变量可以添加gcc编译的-I选项

通过CPPDEFINES变量可以添加gcc的-D选项

通过CC变量可以指定C语言的交叉编译器

通过RESULT\_SUFFIX变量可以指定生成目标文件的类型

通过env\['ENV'\]\['PATH'\]变量可以指定交叉编译工具链的路径

scons支持的配置变量与操作很多，具体可参考scons官方文档“[https://scons.org/doc/](https://scons.org/doc/)”。

