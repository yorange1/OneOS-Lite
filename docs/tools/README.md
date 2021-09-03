# 工具简介

本文档主要提供辅助`OneOS Lite`开发的各种工具的使用说明。目前，`OneOS` 开发环境主要有两种，分别是：

* 基于命令行的`OneOS-Cube`开发环境；
* MDK开发环境。

这两种开发环境都支持在WINDOWS（WIN7 或 WIN10）操作系统下运行，`OneOS-Cube`支持LINUX操作系统。

## 推荐

针对不同的项目或使用习惯，开发者可行选择熟悉的开发工具。我们热情推荐大家使用`OneOS-Cube`进行系统的配置，编译和构建。如果你希望使用MDK开发环境，也建议使用`OneOS-Cube`工具根据`Menuconfig`生成的配置文件和`Scons`编译脚本生成正确的MDK工程，之后再使用MDK进行编译、烧录以及调试。

![cube](cube/images/OneOS-Cube总体组成.png)

## 配置&编译

总之，配置阶段，`OneOS-Cube`能很好的帮助到您。

而在编译和构建阶段，如果您习惯于命令行和使用gnu工具链，那么您可以在`OneOS-Cube`上执行。如果您习惯于使用MDK，那么`OneOS-Cube`也能帮助您生成正确的MDK工程，之后再打开该工程，使用MDK进行编译、烧录以及调试。

在开发工具篇中，我们会基于`OneOS-Cube`，编写`menuconfig`、`scons`、`gnu`工具链的基本语法和使用说明，我们也会编写MDK工具的使用，以及`STM32CubeMX`等工具的使用说明。

## 建议

非常建议，在开发`OneOS Lite`之前，能对我们使用的工具有大致的了解，磨刀不误砍柴工，祝大家快乐开发。



