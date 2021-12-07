> 在这个在家工作的新时代，固件工程师可能没有他们在办公室里所拥有的所有设备和开发板。有什么解决办法吗？仿真！

虽然它们不是真的，但是这些模拟器可以运行我们的固件，通过 UART 打印数据，从 I2C 传感器读取寄存器，甚至在 SPI 闪存设备上运行文件系统。这足够写一些真正的程序来执行了！

在本文中，我们将详细介绍了如何设置 Renode 模拟器并在其中为 STM32运行程序。使用这种设置，我们将调试程序，通过集成测试运行它，并缩短开发的迭代周期。

## 什么是 Renode

Renode 是一个用于嵌入式平台的开源仿真器。今天，它支持 `x86(Intel Quark)`、 `Cortex-A (NVIDIA Tegra)`、 `Cortex-M`、 `SPARC (Leon)`和基于`risc-v` 的平台。

Renode 可以采用运行在真实硬件设备上的相同程序，并在模拟核心、外围设备甚至传感器和驱动器上运行它。更好的是，它广泛的网络支持和多系统仿真使它成为由多个设备组成的一个测试系统。

使用 Renode，您可以在硬件准备就绪之前开始开发，在不部署硬件的情况下测试程序，并通过减少闪存加载延迟来缩短迭代周期。并且，Renode 是使用 Mono 框架构建的，它允许跨平台运行。

Renode和QEMU之间的差异？- 具有仿真经验的读者指出 QEMU 已经存在了很长时间，并且能够仿真 Cortex-M 目标。根据我们的经验，QEMU 专注于模仿面向更高级别操作系统(如 Linux 计算机)的系统，而不是嵌入式设备。到目前为止，它只支持两个 cortex-M 系列MCU，都是 TI 公司制造的。

## Renode初体验

Renode 项目发布 Windows、 MacOS 和多个 Linux 发行版的安装程序。在撰写本文时，您可以在 Github 上找到 v1.9版本。

本指南是在Windows 上编写的，但并不是特定于操作系统。为了验证你的 Renode 安装，你可以运行一个例子:

1.打开renode终端，并输入命令：`start @scripts/single-node/stm32f4_discovery.resc`

![](.\1.png)

2.这时，我们可以看到到一个新的终端弹出，里面显示的是串口的输出

![](.\2.png)

**由此可见，示例中的系统已经跑起来了，并且显示了串口输出！**
