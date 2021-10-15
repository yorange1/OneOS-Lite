## 1.简介
上一篇文章见了，`renode`是什么，以及使用安装`renode`后，使用官方示例跑了一个最简单的系统，并且有了串口输出。

此次，我们使用`renode`来运行一款物联网操作系统`OneOS-Lite`。

## 2.renode官网

在此，我们给出renode的官方网址：[https://renode.io/](https://renode.io/)，以及它的官方文档：[https://renode.readthedocs.io/en/latest/](https://renode.readthedocs.io/en/latest/)

相关资料可以通过该网址进行查阅。如果需要该renode文档和中文翻译，欢迎关注公众号CMCC-OneOS，或者留言和我联系。

## 3.renode基础操作
为了能更好的使用renode，我们在这里简单说明一下renode的基本操作，来满足我们的日常工作需求。

### 3.1 帮助help
熟悉命令，最好的方式之一是巧用`help`命令。我们可以直接在终端中直接输入`help`来查看它所支持的命令有哪些，也可以使用`help xxx`的方式来查看绝体某个命令的详细使用方法。

![help-renode](C:\Users\luosy\Music\10.使用renode仿真开发OneOS-Lite物联网操作系统\help-renode.gif)

### 3.2 创建Machine
在`monitor`中执行`mach create`，将会创建一个新的`machine`，默认情况下，系统会自动分配`machine`的`index`，从0开始计数，分别命名为`machine-0`、`machine-1`等，并且会自动将`monitor`的上下文切换到新建的`machine`。

如果需要指定`machine`的名字，可以执行命令`mach create ”xxx“`，如下图：

![mach-create](C:\Users\luosy\Music\10.使用renode仿真开发OneOS-Lite物联网操作系统\mach-create.gif)



### 3.3 装载Platform
当完成Machine的创建之后，该Machine只有一个外设，即系统总线（Renode中简写为sysbus）。此时该Machine并不具备CPU和内存，因此还无法执行任何代码。

我们可以通过 peripherals命令列出所有的外设，可以看到，此时只有sysbus：

![sysbus](C:\Users\luosy\Music\10.使用renode仿真开发OneOS-Lite物联网操作系统\sysbus.gif)



这时，我们需要加载平台描述文件：`machine LoadPlatformDescription @platforms/boards/stm32f4_discovery.repl`。

此时，再次查看外设。可以发现，包含串口uart等更多的外设信息。而我们虚拟的硬件平台也已经初步完成。

![sysbus-more](C:\Users\luosy\Music\10.使用renode仿真开发OneOS-Lite物联网操作系统\sysbus-more.gif)



### 3.4 加载程序
在完成平台的创建以及配置后，就可以将要运行的软件上载到平台上。`Renode`可以使得用户上载与在真实硬件上同样的软件可执行文件，无需修改或者重新编译。

加载`elf`文件到虚拟的内存中，所用命令格式如下：`sysbus LoadELF @my-project.elf`

这里我们使用的是`OneOS-Lite`下的`STM32F407VG-renode`项目，进入该项目中，编译生成elf文件。

![platform](C:\Users\luosy\Music\10.使用renode仿真开发OneOS-Lite物联网操作系统\platform.gif)



### 3.5 运行程序
程序加载好以后，可以使用命令`start`让程序运行起来，我们可以通过命令`showAnalyzer sysbus.uart1`打开串口，证明程序是跑起来了的！

![uart1](C:\Users\luosy\Music\10.使用renode仿真开发OneOS-Lite物联网操作系统\uart1.gif)



## 4.结语
通过renode，我们让`OneOS-Lite`这个超轻量的物联网操作系统跑在了虚拟的硬件里面，这对我们学习一款操作系统是非常有益的。

renode的使用，可以让我们更加了解硬件，`OneOS-Lite`则揭开了操作系统的神秘面纱。后面，我们还会推出更多的相关文章，欢迎关注。

