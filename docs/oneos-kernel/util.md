# 标准输出

---

## 简介

调试日志做为常规的调试手段，OneOS提供了一种常规的输出到标准输出终端的调试接口。

---
## API介绍


| 接口 | 说明 |
| :--- | :--- |
| os\_kprintf | 打印函数 |

### os\_kprintf

该函数用于打印，使用方法同printf，为减少对栈的使用，此函数不支持浮点数，函数原型如下：

```c
void os_kprintf(const char *fmt, ...);
```

| **参数** | **说明** |
| :--- | :--- |
| fmt | 包含格式字符串的C字符串，其格式字符串与printf中的格式相同 |
| ... | 可变参数 |
| **返回** | **说明** |
| 无 | 无 |


