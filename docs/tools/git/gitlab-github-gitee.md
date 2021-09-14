# 简介

我们开发的时候，需要把代码同步到多个远程仓库中去。此次项目开发，我们首先基于`gitlib`开发，开发到一定阶段，我们准备开源，于是上线码云`gitee`，这时新建了一个仓库，以前的提交记录留在了内网的`gitlab`，并且以后的开发均在码云`gitee`进行。而后，因为内网项目需求，需要同步管理仓库`gitlab`和`gitee`，这个时候就需要同步代码了。

方法有很多，我们介绍一种。

## 1.关联远程库

我们以`gitee`为主体，从`gitee`中`clone`出一份本地仓库。然后，关联`gitlab`的远程仓库：

```
git remote add gitlab git@10.12.3.198:luoshunyuan/OneOS-Lite.git
```

关联完成后，使用`git remote -v`查看：

```
gitlab  git@10.12.3.198:luoshunyuan/OneOS-Lite.git (fetch)
gitlab  git@10.12.3.198:luoshunyuan/OneOS-Lite.git (push)
origin  git@gitee.com:xuxeu/OneOS-Lite.git (fetch)
origin  git@gitee.com:xuxeu/OneOS-Lite.git (push)
```

## 2.拉取`gitee`和`gitlab`代码

因为，我们的提交都以`gitee`为主，因此，首先拉取`gitee`的代码。

`git pull`

我们在第一次同步的时候，需要拉取`gitlab`代码，并创建本地分支：

```
git clone git@10.12.3.198:luoshunyuan/OneOS-Lite.git
git checkout -b dev origin/dev
```

## 3.提交到`gitlab`

在关联好的`gitee`仓库中，执行命令来合并`gitee`代码到`gitlab`仓库中：

`git push gitlab dev`

这是，如果报错无法推送，则可以强制推送：

`git push gitlab dev -f`

## 4.合并历史记录

执行了第三步操作后，`gitlab`远程仓库中将只会存在强制推送的`gitee`的历史记录了。

但，还好我们之前在第二步，有拉过`gitlab`代码，并创建了本地分支。因此我们执行拉取操作，这时它会提醒我们：

```
fatal: refusing to merge unrelated histories
```

这是可以执行以下命令，来忽略版本不同造成的影响：

```
git pull --allow-unrelated-histories
```

然后，进行合并`merge`，推送代码。这时，我们可以在远程仓库上看到，所有的历史记录都已经回来了。

## 5.之后的同步

之后的同步就比较简单了。每次都从`gitee`远程仓库拉取代码到本地，然后再次推送到`gitlab`即可。