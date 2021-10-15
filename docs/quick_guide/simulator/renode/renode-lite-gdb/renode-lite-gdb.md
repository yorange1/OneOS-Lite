## 前言
我们已经使用renode仿真开发OneOS-Lite物联网操作系统，并使用模拟出的串口，输出了`Hello OneOS-Lite`。而想要更深入地了解系统的执行流程，或者解决程序的bug，拥有调试的手段，必不可少！

本篇文章将从调试的角度出发，基于Renode，使用gdb调试OneOS-Lite物联网操作系统。通过调试OneOS-Lite，也可以更加直白地了解她。

## renode调试支持
Renode支持使用 GDB 调试在仿真机器上运行的应用程序。

它使用 GDB 远程协议，并支持使用最常见的 GDB 函数，如断点，观察点，单步，内存访问等。

在真实硬件上调试最重要的区别是：当仿真的CPU停止时，虚拟时间不会进展。这使得调试过程对于仿真机器来说是透明的。

## 编译程序
首先，当然是编译程序，这里我们使用的是OneOS-Lite下的STM32F407VG-renode项目，进入该项目中，编译生成elf文件。
建议克隆源码仓库[https://gitee.com/cmcc-oneos/OneOS-Lite](https://gitee.com/cmcc-oneos/OneOS-Lite)，体验尝试。

![](.\1.gif)

## 创建仿真
在文章《使用renode仿真开发OneOS-Lite物联网操作系统》，我们已经实战操作过如何进行仿真的全过程。为了简化操作，我们已经把步骤写成了脚本文件`stm32f4_discovery.resc`，放在`STM32F407VG-renode`项目中。

使用以下命令：

```
include @D:\gitee\OneOS-Lite\projects\STM32F407VG-renode\stm32f4_discovery.resc
```

![](.\2.gif)

## 连接gdb
在renode监视器中运行以下指令，我们可以在端口 3333 上启动 GDB 服务器:

```
(machine-0) machine StartGdbServer 3333
```

![](.\3.gif)

我们可以自行选择适当的工具链启动 GDB，并连接到远程目标(这里我们选择的是CUBE工具自带的GDB调试程序):

```
$ arm-none-eabi-gdb /path/to/application.elf
(gdb) target remote :3333
```
在elf程序的目录下，执行以上命令（这样可以免除路径的烦恼）：

![](.\4.gif)

## 开始调试
GDB 连接到renode后，仿真就可以开始了。仅仅告诉 GDB 继续，并不足以启动程序，这是我们还要告诉renode监视器，才能得以调试继续。

```
(gdb) monitor start
(gdb) continue
```
在此之后，我们就可以像正常使用gdb调试程序的样子，单步、断点、查看栈层次等等。
![](.\5.gif)

