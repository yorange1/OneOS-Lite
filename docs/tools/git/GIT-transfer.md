## 迁移git仓库

开发在很多时候，会遇到一个问题。GIT仓库的管理，特别是仓库的迁移。我需要保留已有的历史记录，而不是重新开发，重头再来。

我们可以这样做：

使用`--mirror`模式会把本地的分支都克隆。

```
// 先用--bare克隆裸仓库
git clone git@gitee.com:xxx/testApp1.git --bare
// 进入testApp1.git，这样就可以把所有分支都克隆到新仓库了
git push --mirror git@gitee.com:xxx/testApp2.git
```

我们还可以这样做：

```
git remote add gitlab git@gitee.com:xxx/testApp2.git
git push gitlab
git checkout dev
git push gitlab
...
```

这样的方法是，把`testApp2`作为新增的远程仓库，然后使用push一次推送一个分支。这种方式的好处是：迁移的git仓库，只需迁移指定的分支。

## 迁移git仓库子目录

上面的方法是整个仓库的迁移，但如果只想迁移git仓库的子目录，则可以使用`git subtree`，比如想要把仓库目录`thirdparty\telnetd`拆分出去作为一个新的库，并且还要带上她所有的历史提交记录。

首先，我们同样需要新建一个新的空仓库（**注意**：不要带有任何初始化文件，包括README.md）。

然后，到主仓库执行以下git命令，就可以看到`telnetd`仓库已有代码了。

```
git subtree push --prefix=thirdparty/telnetd git@gitee.com:components/telnetd.git master
```

这样，就完成子目录的迁移啦，就是有个问题，好慢~



