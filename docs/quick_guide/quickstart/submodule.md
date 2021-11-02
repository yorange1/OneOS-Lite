## 前言

简单清晰的代码结构有利于项目代码的管理。`OneOS-Lite`使用`git submodule`的方式来进行代码的管理。

其中，`OneOS-Lite`仓库为主存储库。而内核`kernel`，以及组件`components`和驱动`drivers`都将作为子模块的形式和`OneOS-Lite`主仓库关联管理。

## 获取子模块

使用`git clone`是不会把子模块仓库源码文件检入的，只会把拉取下来对应子模块组件的空文件夹。这时，如果需要使用某个子模块组件，可以使用以下命令：

```
git submodule init
git submodule update xxx(子模块组件名，可通过git submodule查看已有的贡献组件)
```

举个栗子：

```
$ git submodule init
Submodule 'thirdparty/cJSON' (git@10.12.3.198:luoshunyuan/cJSON.git) registered for path 'thirdparty/cJSON'
Submodule 'thirdparty/telnetd' (git@10.12.3.198:luoshunyuan/telnetd.git) registered for path 'thirdparty/telnetd'
```

```
$ git submodule update thirdparty/telnetd
Cloning into 'D:/xiaoneng/oneos-2.0-test/thirdparty/telnetd'...
Authorized users only. All activity may be monitored and reported.
Submodule path 'thirdparty/telnetd': checked out 'e99c7b71349f74804eb215c59fa426bf5a357d8f'
```

如果需要获取所有子模块仓库，可以直接使用`git submodule update`。

## 检索子模块

该命令`git submodule`可以查看主仓库中，已有的子模块或子仓库。

```
$ git submodule
-ba2c4170016ff14db298c5e87134704996c5aff5 thirdparty/cJSON
-e99c7b71349f74804eb215c59fa426bf5a357d8f thirdparty/telnetd
```

拿以上举例：

- `-`代表该子模块仓库源码文件还未检入（空文件夹），没有`-`代表已检入；
- `e99c7b71349f74804eb215c59fa426bf5a357d8f`代表该子模块组件和`OneOS-Lite`所关联的版本id；
- `thirdparty/telnetd`代表该子模块仓库对应的目录和其组件名称。

## 关联子模块

通过项目根目录下的一个`.gitmodules`文件，可以发现已经关联的子仓库，每添加一个子模块仓库就会新增一条记录。内容如下：

```
[submodule "thirdparty/telnetd"]
	path = thirdparty/telnetd
	url = git@10.12.3.198:luoshunyuan/telnetd.git
[submodule "thirdparty/cJSON"]
	path = cJSON
	url = git@10.12.3.198:luoshunyuan/cJSON.git
```

继续关联新的子模块可以使用如下`git`命令，这里以`telnetd`为例：

```bash
git submodule add git@10.12.3.198:luoshunyuan/telnetd.git thirdparty/telnetd
```

## 修改&&提交子模块

如果需要更改子仓库，可先在对应的子仓库目录中，打开`git`命令行，进入对应贡献组件的git仓库管理，并执行代码提交操作：

![](C:/Users/luosy/Music/15.OneOS-Lite的源码管理方式-子模块管理/1.gif)

因为修改了子模块仓库代码并提交了，但是主仓库的指针依旧指向那个老的子仓库的`commit id`，如果不提交这个修改的话，别人拉取主项目并且使用`git submodule update`更新子模块还是会拉取到修改前的代码。

因此，这时候需要把主项目进行提交更新。

![](C:/Users/luosy/Music/15.OneOS-Lite的源码管理方式-子模块管理/2.gif)

## 删除废弃模块

当某些子仓库不需要了，需要废弃，删除的时候，`git`没有直接删除子模块的命令，所以只能逐步删除相关文件：

1. 在版本控制中删除子模块：

   `git rm -r thirdparty/cJSON`

2. 在编辑器中删除`.gitmodules`相关内容：

   ```
   [submodule "thirdparty/cJSON"]
   	path = thirdparty/cJSON
   	url = git@10.12.3.198:luoshunyuan/cJSON.git
   ```

3. 在编辑器中删除`.git/config`相关内容：

   ```
   [submodule "thirdparty/cJSON"]
   	url = git@10.12.3.198:luoshunyuan/cJSON.git
   	active = true
   ```

4. 删除`.git`下的缓存模块：

   ```
   rm -rf .git/modules/thirdparty/cJSON
   ```

5. 提交修改：

   ```
   git commit -am "delete submodule"
   git push
   ```



