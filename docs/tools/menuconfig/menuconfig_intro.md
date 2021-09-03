# menuconfig工作机制


`menuconfig`命令的工作机制如下图所示：

![](/images/OneOS-menuconfig流程.png)

1. `menuconfig`启动后默认到当前目录下寻找`Kconfig`文件，并解析。`Kconfig`中可以通过`source`指定加载子`Kconfig`文件，这样根据`Kconfig`文件的内部调用结构，`menuconfig`依次解析所有被引用的`Kconfig`文件，生成内部的配置选项数据库。

1. 解析`.config`文件，根据上一次的配置结果初始化各个配置选项的初值。

2. 给用户展示配置界面，并根据用户的选择更新内部数据库。

3. 用户退出交互界面并保存配置，`menuconfig`根据内部数据库内容，生成新的.`config`文件。

4. 将`.config`文件翻译成c语言能够识别的`oneos\_config.h`文件，位于当前工作目录下。

通过上面的步骤，`menuconfig`根据用户的配置，最终生成了`oneos\_config.h`文件，用户便可以直接引用`oneos\_config.h`文件来使用系统宏。

