# SCons简介

scons是一个用Python开发的开源的自动化构建工具，它跟GUN make是同一类工具，可以认为是GNU make的替代工具，它是一个更简便，更可靠，更高效的构建软件。

本章节只对scons的常用功能与特性进行简要介绍，更详尽的信息也可以参考scons官方文档“[https://scons.org/doc/](https://scons.org/doc/)”。

---

### SCons编译构造概述

Scons是构造工具，通过读取SConstruct/Sconstruct/sconstruct和SConscript脚本来组织管理源码，并调用指定的编译链接器来对源文件进行编译。

scons目前支持的编译器包括 ARM GCC、MDK、IAR、VisualStudio、Visual DSP。对流行的ARM Cortex M0、M3、M4 等架构，一般都同时支持ARM GCC、MDK、IAR 等编译器。用户可以通过 project 目录下的osconfig.py 里的 CROSS\_TOOL 选项查看相关的编译器配置。

使用SCons工具时，会使用到SConstruct/Sconstruct/sconstruct和SConscript 2种类型的文件。这2种类型的文件是Scons工具必备的，是编译的配置脚本文件，相当于make中的makefile文件。

Scons将在当前目录下按SConstruct、Sconstruct、sconstruct次序来搜索配置文件，从读取的第1个文件中读取相关配置。

在配置文件SConstruct中，可以使用SConscript\(\)函数来读取附属的配置文件。按惯例，这些附属配置文件被命名为“SConscript”，当然也可以使用任意其它名字（在OneOS操作系统，使用名字SConscript）。

### SCons的扩展性

scons的配置文件都是基于python脚本语法的。因此可以在SConstruct文件中使用python语言很方便的调用自定义的python脚本与模块。

同时，scons自身也是python实现的，在自己的python脚本文件中通过添加语句：

from SCons.Script import \*

可以很方便的直接使用scons内置类与操作。正因为scons与python脚本可以方便的互相调用，我们可以对scons方便的进行自定义与扩展。

