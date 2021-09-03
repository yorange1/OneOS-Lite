# Kconfig编写

## OneOS的Kconfig结构

`OneOS`的`Kconfig`文件按照下面的逻辑结构进行组织的：

![](/images/OneOSKconfig结构.png)  

`menuconfig`首先会在当前目录下寻找入口`Kconfig`文件，即顶层`Kconfig`文件，并解析；

顶层`Kconfig`会`source`调用`OneOS`源码的根目录下的`Kconfig`文件；

根目录`Kconfig`文件会`source`调用各个主要代码路径下的`Kconfig`，通过递归`source`子目录下的`Kconfig`文件；

如上流程，`menuconfig`完成对系统配置文件`Kconfig`的加载。用户要新添加`Kconfig`宏定义，可以参考`OneOS`的`Kconfig`结构，在相应的`Kconfig`文件中直接添加宏定义，或者通过`source`加载自己的`Kconfig`文件。


## 顶层Kconfig

顶层`Kconfig`是`menuconfig`加载的第一个`Kconfig`文件，主要功能是：

定义`menconfig`主菜单标题；

定义表示顶层`Kconfig`与`OneOS`源码根目录的相对路径宏`OS\_ROOT`；

加载子`Kconfig`文件，如根目录`Kconfig`等。

举例说明如下图：

![](/images/顶层kconfig.png)

定义顶层`Kconfig`文件与源码根目录的相对路径宏`OS\_ROOT`是为了方便用户调整自己的顶层`Kconfig`位置。

举例说明：`OneOS`源码路径是"E:\OneOS", 而用户自己的工程顶层`Kconfig`位于`"E:\OneOS\project\myproject"`中，这种情况下顶层`Kconfig`所在目录回退两级目录即可进入源码根目录，因此应该定义为`“OS\_ROOT=../..”` 。如果用户自己的顶层`Kconfig`位于`“E:\OneOS\myproject”`中，则相应的需要定义相对路径宏为`"OS\_ROOT=.." `。


## 根目录Kconfig

根目录`Kconfig`的主要功能就是指定需要加载的各个模块目录下的`Kconfig`，如图示：

![](/images/根目录Kconfig.png)


### Kconfig语法

`OneOS`的`Kconfig`语法与开源的`linux Kconfig`语法完全一致，用户可以自行网上查询，或参考linux官方文档[https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html](https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html)

