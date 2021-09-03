# menuconfig使用

## 启动menuconfig

`menuconfig`默认会到当前工作路径下去寻找`Kconfig`文件作为入口配置文件并解析，因此必须在顶层`Kconfig`文件所在路径下运行该命令。否则将无法找到`kconfig`文件或造成配置项加载不完全的问题。

启动`OneOS-Cube`命令行，进入顶层`Kconfig`文件所在路径（一般即具体硬件型号对应的`project`目录），直接输入`menconfig `或 `menuconfig.bat`即可启动系统配置命令，如下图所示：

![](/images/start-menuconfig.png)


## 修改配置

正常启动`menuconfig`后，即进入基于菜单选项的可视化配置环境：

![](/images/menu说明.png)

如上图所示，配置菜单有上下两个区域组成：配置区域，操作说明区域。

`menuconfig`的操作风格与linux的`menuconfig`配置操作风格是基本一致的。常用的基本操作有：

通过“上”、“下”键切换同级菜单；

通过“左”、“右”键退出与进入子菜单；

通过“Y”与“N”键来使能或取消具体配置项；

通过“?”键查看配置项说明；

通过“/”进行快速的宏查找；

`menuconfig`非常灵活，这里不一一列举，用户可以参考提示区说明。

