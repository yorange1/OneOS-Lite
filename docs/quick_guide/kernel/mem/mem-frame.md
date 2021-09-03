# 内存管理-框架

## 概述

该篇文章，不讲算法，先说说内存管理模块是如何在系统中初始化的？她的配置选项如何选择？以及如何进行内存管理的？以及内存管理框架是如何的？

## 配置选项

```bash
(Top) → Kernel→ Memory
[*] system memory heap
-*- memory heap
    algorithm  --->
            [ ] firstfit
            [ ] buddy
            [*] small
[ ] memory trace
[*] memory minimal funtion
[ ] memory stats
[*] memory pool
[ ]     Memory-Pool check tag
```

其中`system memory heap`和`memory heap`是内存堆管理的必要使能

## 初始化



