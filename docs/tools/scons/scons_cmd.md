# SCons基本命令

#### **scons**

编译指令，根据scons脚本（SConstruct, SConscript）的配置，组织编译代码。scons命令必须在工程路径下执行，因为scons默认会在当前路径下寻找SConstruct文件作为第一个解析的脚本。

scons支持增量编译，对已经编译过的文件，且编译后没有进行过修改，那scons不会重新编译这些文件，从而达到提高编译效率的目的。

#### **scons --verbose**

直接使用scons编译时，默认是不会输出编译选项等信息的。使用scons --verbose进行编译时，将会打印编译器选项等辅助信息。

#### **scons -c**

清除上一次编译的临时文件。执行该命令后，下一次编译将能保证所有源文件都会被重新编译。

#### **scons  --help**

帮助命令，该命令会例举出scons支持的所有操作选项，包括原生scons自带的选项与OneOS扩展的操作选项。scons支持的操作选项很丰富，推荐读者都去了解一下。

#### **scons --ide=xxx**

生成第三方IDE工程命令，其中“XXX”对应想要生成的工程类型。具体支持的工程类型可以通过scons --help查看。举例生成keil5工程的命令如下：

`scons --ide=mdk5`

需要注意的是，要生产指定的IDE工程文件，一般需要提供一个空的工程文件作为模版，用户应该在相应的模版文件中配置好硬件相关的编译选项。具体可以参考示例开发板的工程目录中的文件。
