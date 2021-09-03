# 链表

---

## 简介

链表是一种常用的数据结构，它使用指针将一系列数据节点链接成数据链。相对于数组，链表具有良好的动态性，建立链表时不需要提前知道数据量，可以随时分配空间，可以高效地在链表中的任意位置插入或者删除数据。

通常链表数据结构包含两部分，一部分是数据域，用于存储数据；另外一部分是指针域，用于建立与其它节点的关系。而本操作系统中提供的链表不包含数据域，使用时不是在链表结构中包含数据，而是在用户的数据结构中包含链表节点。本操作系统中提供了双向链表及单向链表的一些比较通用的操作接口。

---

## 重要定义及数据结构

### 链表结构体

```c
struct os_list_node
{
    struct os_list_node *next;      /* Point to next node */
    struct os_list_node *prev;      /* point to previous node */
};
```

| **双向链表节点** | **说明** |
| :--- | :--- |
| next | 下一个节点的指针 |
| prev | 上一个节点的指针 |

```c
struct os_slist_node
{
    struct os_slist_node *next;     /* Point to next node */
};
```

| **单向链表节点** | **说明** |
| :--- | :--- |
| next | 下一个节点的指针 |

---

## API介绍

### 双向链表API列表

| **接口** | **说明** |
| :--- | :--- |
| os\_list\_init | 链表头初始化 |
| os\_list\_add | 把链表项加到链表头部 |
| os\_list\_add\_tail | 把链表项加到链表尾部 |
| os\_list\_del | 删除链表项 |
| os\_list\_del\_init | 删除链表项并初始化链表项 |
| os\_list\_move | 把链表项移动到链表头，即把链表项移除然后挂到链表头部 |
| os\_list\_move\_tail | 把链表项移动到链表尾，即把链表项移除然后挂到链表尾部 |
| os\_list\_empty | 判断链表是否为空 |
| os\_list\_splice | 合并两个链表 |
| os\_list\_splice\_init | 合并两个链表 |
| os\_list\_len | 获取链表长度 |
| os\_list\_first | 获取第一个链表项 |
| os\_list\_tail | 获取最后一个链表项 |
| OS\_LIST\_INIT | 宏，初始化链表 |
| os\_list\_entry | 宏，通过链表结点指针和结构体类型，获取结构体指针 |
| os\_list\_first\_entry | 宏，通过链表头和结构体类型，获取第一个链表项所在结构体指针 |
| os\_list\_first\_entry\_or\_null | 宏，通过链表头和结构体类型，获取第一个链表项所在结构体指针，若链表为空，返回OS\_NULL |
| os\_list\_tail\_entry | 宏，通过链表头和结构体类型，获取最后一个链表项所在结构体指针 |
| os\_list\_tail\_entry\_or\_null | 宏，通过链表头和结构体类型，获取最后一个链表项所在结构体指针，若链表为空，返回OS\_NULL |
| os\_list\_for\_each | 宏，遍历链表 |
| os\_list\_for\_each\_safe | 宏，安全遍历链表 |
| os\_list\_for\_each\_entry | 宏，遍例链表，同时获取链表项结构体指针 |
| os\_list\_for\_each\_entry\_safe | 宏，安全遍例链表，同时获取链表项结构体指针 |

### 单向链表API列表

| **接口** | **说明** |
| :--- | :--- |
| os\_slist\_init | 单向链表头初始化 |
| os\_slist\_add | 把链表项加到单向链表头部 |
| os\_slist\_add\_tail | 把链表项加到单向链表尾部 |
| os\_slist\_del | 删除链表项 |
| os\_slist\_len | 获取单向链表长度，即链表项数量 |
| os\_slist\_first | 获取第一个链表项 |
| os\_slist\_tail | 获取最后一个链表项 |
| os\_slist\_next | 获取指定链表项的下一个链表项 |
| os\_slist\_empty | 判断链表是否为空 |
| OS\_SLIST\_INIT | 宏，用于初始化链表头 |
| os\_slist\_entry | 通过链表结点指针和结构体类型，获取结构体指针 |
| os\_slist\_first\_entry | 宏，通过链表头和结构体类型，获取第一个链表项所在结构体指针 |
| os\_slist\_first\_entry\_or\_null | 宏，通过链表头和结构体类型，获取第一个链表项所在结构体指针，若链表为空，返回OS\_NULL |
| os\_slist\_tail\_entry | 宏，通过链表头和结构体类型，获取最后一个链表项所在结构体指针 |
| os\_slist\_tail\_entry\_or\_null | 宏，通过链表头和结构体类型，获取最后一个链表项所在结构体指针，若链表为空，返回OS\_NULL |
| os\_slist\_for\_each | 宏，遍历链表 |
| os\_slist\_for\_each\_safe | 宏，安全遍历链表 |

### os\_list\_init

该函数用于链表头初始化，函数原型如下：

```c
OS_INLINE void os_list_init(os_list_node_t *node);
```

| **参数** | **说明** |
| :--- | :--- |
| node | 链表头节点 |

### os\_list\_add

该函数把链表项添加到链表头部（在头节点之后），函数原型如下：

```c
OS_INLINE void os_list_add(os_list_node_t *head, os_list_node_t *entry);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| entry | 待添加的链表项 |

### os\_list\_add\_tail

该函数把链表项添加到链表尾部（在头节点之前，因为是环形链表，就相当于在链表尾部），函数原型如下：

```c
OS_INLINE void os_list_add_tail(os_list_node_t *head, os_list_node_t *entry);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| entry | 待添加的链表项 |

### os\_list\_del

该函数删除链表项，函数原型如下：

```c
OS_INLINE void os_list_del(os_list_node_t *entry);
```

| **参数** | **说明** |
| :--- | :--- |
| entry | 待删除的链表项 |

### os\_list\_del\_init

该函数删除链表项并初始化链表项，函数原型如下：

```c
OS_INLINE void os_list_del_init(os_list_node_t *entry);
```

| **参数** | **说明** |
| :--- | :--- |
| entry | 待删除的链表项 |

### os\_list\_move

该函数把链表项移动到链表头，即先把链表项移除，然后挂到链表头部，函数原型如下：

```c
OS_INLINE void os_list_move(os_list_node_t *head, os_list_node_t *entry);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| entry | 待移动的链表项 |

### os\_list\_move\_tail

该函数把链表项移动到链表尾，即先把链表项移除，然后挂到链表尾部，函数原型如下：

```c
OS_INLINE void os_list_move_tail(os_list_node_t *head, os_list_node_t *entry);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| entry | 待移动的链表项 |

### os\_list\_empty

该函数判断链表是否为空，函数原型如下：

```c
OS_INLINE os_bool_t os_list_empty(const os_list_node_t *head);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| **返回** | **说明** |
| os\_bool\_t | 是否为空 |

### os\_list\_splice

该函数用于合并两个链表，将list链表的所有链表项添加到head链表的头部（在head头节点之后），函数原型如下：

```c
OS_INLINE void os_list_splice(os_list_node_t *head, os_list_node_t *list);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| list | 另一个链表头，将会把此链表中的所有链表项添加到head链表 |

### os\_list\_splice\_init

该函数用于合并两个链表，将list链表的所有链表项添加到head链表的头部（在head头节点之后），然后初始化list链表的链表头，函数原型如下：

```c
OS_INLINE void os_list_splice_init(os_list_node_t *head, os_list_node_t *list);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| list | 另一个链表头，将会把此链表中的所有链表项添加到head链表 |

### os\_list\_len

该函数用于获取链表长度，函数原型如下：

```c
OS_INLINE os_uint32_t os_list_len(const os_list_node_t *head);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| **返回** | **说明** |
| os\_uint32\_t | 链表的长度 |

### os\_list\_first

该函数用于获取第一个链表项（即头节点的下一个链表项），若链表为空，则返回OS\_NULL，函数原型如下：

```c
OS_INLINE os_list_node_t *os_list_first(os_list_node_t *head);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| **返回** | **说明** |
| 非OS\_NULL | 第一个链表项 |
| OS\_NULL | 链表为空 |

### os\_list\_tail

该函数用于获取最后一个链表项（头节点的前一个链表项，因为是环形链表，即相当于最后一个链表项），若链表为空，则返回OS\_NULL，函数原型如下：

```c
OS_INLINE os_list_node_t *os_list_tail(os_list_node_t *head);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| **返回** | **说明** |
| 非OS\_NULL | 最后一个链表项 |
| OS\_NULL | 链表为空 |

### OS\_LIST\_INIT

该宏用于初始化链表，该宏只能在表达式中作为右值，定义如下：

```c
#define OS_LIST_INIT(name)          {&(name), &(name)}
```

| **参数** | **说明** |
| :--- | :--- |
| name | 链表名，该链表名的变量类型需要为os\_list\_node\_t |
| **返回** | **说明** |
| os\_list\_node\_t | 初始化的结果（给结构体赋值） |

### os\_list\_entry

该宏用于已知结构体type的成员member的地址ptr，求结构体type的起始地址；若链表节点member是type结构体的成员，根据该链表节点地址ptr，则可以获取链表节点所在type结构体的地址

```c
#define os_list_entry(ptr, type, member)                                    \
    os_container_of(ptr, type, member)
```

| **参数** | **说明** |
| :--- | :--- |
| ptr | member成员的地址 |
| type | member所在结构体类型 |
| member | 该成员在结构体中的名字 |
| **返回** | **说明** |
| 地址 | type结构体的地址 |

### os\_list\_first\_entry

该宏用于通过链表头和结构体类型，获取第一个链表项所在结构体指针，注意使用之前需确认链表不为空，定义如下：

```
#define os_list_first_entry(head, type, member)                             \
    os_list_entry((head)->next, type, member)
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头，注意使用之前需确认链表不为空 |
| type | 链表节点所在的结构体 |
| member | 链表在结构体中的名字 |
| **返回** | **说明** |
| 非OS\_NULL | 第一个链表项所在结构体的地址 |

### os\_list\_first\_entry\_or\_null

该宏用于通过链表头和结构体类型，获取第一个链表项所在结构体指针，若链表为空，返回OS\_NULL，定义如下：

```c
#define os_list_first_entry_or_null(head, type, member)                     \
    (!os_list_empty(head) ? os_list_first_entry(head, type, member) : OS_NULL)
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| type | 链表节点所在的结构体 |
| member | 链表在结构体中的名字 |
| **返回** | **说明** |
| 非OS\_NULL | 第一个链表项所在结构体的地址 |
| OS\_NULL | 链表项为空 |

### os\_list\_tail\_entry

该宏用于通过链表头和结构体类型，获取最后一个链表项所在结构体指针，注意使用之前需确认链表不为空，定义如下：

```c
#define os_list_tail_entry(head, type, member)                              \
    os_list_entry(os_list_tail(head), type, member)
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头，注意使用之前需确认链表不为空 |
| type | 链表节点所在的结构体 |
| member | 链表在结构体中的名字 |
| **返回** | **说明** |
| 非OS\_NULL | 最后一个链表项所在结构体的地址 |

### os\_list\_tail\_entry\_or\_null

该宏用于通过链表头和结构体类型，获取最后一个链表项所在结构体指针，若链表为空，返回OS\_NULL，定义如下：

```c
#define os_list_tail_entry_or_null(head, type, member)                      \
    (!os_list_empty(head) ? os_list_tail_entry(head, type, member) : OS_NULL)
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| type | 链表节点所在的结构体 |
| member | 链表在结构体中的名字 |
| **返回** | **说明** |
| 非OS\_NULL | 最后一个链表项所在结构体的地址 |
| OS\_NULL | 链表项为空 |

### os\_list\_for\_each

该宏用于遍历链表，函数原型如下：

```c
#define os_list_for_each(pos, head)                                         \
    for (pos = (head)->next; pos != (head); pos = pos->next)
```

| **参数** | **说明** |
| :--- | :--- |
| pos | 链表项，当前遍历项 |
| head | 链表头 |

### os\_list\_for\_each\_safe

该宏用于安全遍历链表，适用于遍历过程中需要删除链表项的情况，定义如下：

```c
#define os_list_for_each_safe(pos, n, head)                                 \
    for (pos = (head)->next, n = pos->next;                                 \
         pos != (head);                                                     \
         pos = n, n = pos->next)
```

| **参数** | **说明** |
| :--- | :--- |
| pos | 当前遍历的链表项 |
| n | 用来临时存储下一个链表项的指针，防止pos被删后，无法继续遍历 |
| head | 链表头 |

### os\_list\_for\_each\_entry

该宏用于遍例链表，同时获取链表项结构体指针，定义如下：

```c
#define os_list_for_each_entry(pos, head, type, member)                     \
    for (pos = os_list_entry((head)->next, type, member);                   \
         &pos->member != (head);                                            \
         pos = os_list_entry(pos->member.next, type, member))
```

| **参数** | **说明** |
| :--- | :--- |
| pos | 当前遍历的链表项所在结构体 |
| head | 链表头 |
| type | 链表项所在结构体的类型 |
| member | 链表在结构体中的名字 |

### os\_list\_for\_each\_entry\_safe

该宏用于安全遍例链表，同时获取链表项结构体指针，适用于在遍历过程中需要删除链表项的情况，定义如下：

```c
#define os_list_for_each_entry_safe(pos, n, head, type, member)             \
    for (pos = os_list_entry((head)->next, type, member),                   \
         n = os_list_entry(pos->member.next, type, member);                 \
         &pos->member != (head);                                            \
         pos = n, n = os_list_entry(n->member.next, type, member))
```

| **参数** | **说明** |
| :--- | :--- |
| pos | 当前遍历的链表项所在结构体 |
| n | 用来临时存储下一个链表项所在结构体的指针，防止pos被删后，无法继续遍历 |
| head | 链表头 |
| type | 链表项所在结构体的类型 |
| member | 链表在结构体中的名字 |

### os\_slist\_init

该函数用于单向链表的初始化，函数原型如下：

```c
OS_INLINE void os_slist_init(os_slist_node_t *node);
```

| **参数** | **说明** |
| :--- | :--- |
| node | 链表头节点 |

### os\_slist\_add

该函数用于把链表项加到单向链表头部，函数原型如下：

```c
OS_INLINE void os_slist_add(os_slist_node_t *head, os_slist_node_t *entry);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| entry | 待添加的链表项 |

### os\_slist\_add\_tail

该函数用于把链表项添加到单向链表尾部，函数原型如下：

```c
OS_INLINE void os_slist_add_tail(os_slist_node_t *head, os_slist_node_t *entry);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| entry | 待添加的链表项 |

### os\_slist\_del

该函数用于从单向链表中删除指定的链表项，函数原型如下：

```c
OS_INLINE void os_slist_del(os_slist_node_t *head, os_slist_node_t *entry);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| entry | 待删除的链表项 |

### os\_slist\_len

该函数用于获取单向链表的长度，函数原型如下：

```c
OS_INLINE os_uint32_t os_slist_len(const os_slist_node_t *head);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| **返回** | **说明** |
| os\_uint32\_t | 链表的长度 |

### os\_slist\_first

该函数用于获取单向链表的第一个链表项，函数原型如下：

```c
OS_INLINE os_slist_node_t *os_slist_first(os_slist_node_t *head);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| **返回** | **说明** |
| 非OS\_NULL | 第一个链表项 |
| OS\_NULL | 链表为空 |

### os\_slist\_tail

该函数用于获取单向链表的最后一个链表项，函数原型如下：

```c
OS_INLINE os_slist_node_t *os_slist_tail(os_slist_node_t *head);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| **返回** | **说明** |
| 非OS\_NULL | 最后一个链表项 |
| OS\_NULL | 链表为空 |

### os\_slist\_next

该函数用于获取单向链表中指定链表项的下一个链表项，函数原型如下：

```c
OS_INLINE os_slist_node_t *os_slist_next(os_slist_node_t *node);
```

| **参数** | **说明** |
| :--- | :--- |
| node | 链表项 |
| **返回** | **说明** |
| 非OS\_NULL | 下一个链表项 |
| OS\_NULL | 无下一个链表项 |

### os\_slist\_empty

该函数用于判断单向链表是否为空，函数原型如下：

```c
OS_INLINE os_bool_t os_slist_empty(os_slist_node_t *head);
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| **返回** | **说明** |
| os\_bool\_t | 是否为空 |

### OS\_SLIST\_INIT

该宏用于单向链表的初始化，在表达式中作为右值，定义如下：

```c
#define OS_SLIST_INIT(name)         {OS_NULL}
```

| **参数** | **说明** |
| :--- | :--- |
| name | 链表名，该链表名的变量类型需要为os\_slist\_node\_t |

### os\_slist\_entry

该宏用于通过链表结点指针ptr和链表所在结构体类型type，获取链表所在结构体指针

```c
#define os_slist_entry(ptr, type, member)                                   \
    os_container_of(ptr, type, member)
```

| **参数** | **说明** |
| :--- | :--- |
| ptr | 链表节点指针 |
| type | 链表所在结构体类型 |
| member | 链表在结构体中的名字 |
| **返回** | **说明** |
| 地址 | 链表所在结构体的地址 |

### os\_slist\_first\_entry

该宏用于通过链表头和结构体类型，获取第一个链表项所在结构体指针，使用前需确认链表不为空，定义如下：

```c
#define os_slist_first_entry(head, type, member)                            \
    os_slist_entry((head)->next, type, member)
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头，使用前需确认链表不为空 |
| type | 链表项所在结构体类型 |
| member | 链表在结构体中的名字 |
| **返回** | **说明** |
| 非OS\_NULL | 第一个链表项所在结构体的地址 |

### os\_slist\_first\_entry\_or\_null

该宏用于通过链表头和结构体类型，获取第一个链表项所在结构体指针，若链表为空，返回OS\_NULL，定义如下：

```c
#define os_slist_first_entry_or_null(head, type, member)                    \
    (!os_slist_empty(head) ? os_slist_first_entry(head, type, member) : OS_NULL)
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| type | 链表项所在结构体类型 |
| member | 链表在结构体中的名字 |
| **返回** | **说明** |
| 非OS\_NULL | 第一个链表项所在结构体的地址 |
| OS\_NULL | 链表为空 |

### os\_slist\_tail\_entry

该宏用于通过链表头和结构体类型，获取最后一个链表项所在结构体指针，使用前需确认链表不为空，定义如下：

```c
#define os_slist_tail_entry(head, type, member)                             \
    os_slist_entry(os_slist_tail(head), type, member)
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头，使用前需确认链表不为空 |
| type | 链表项所在结构体类型 |
| member | 链表在结构体中的名字 |
| **返回** | **说明** |
| 非OS\_NULL | 最后一个链表项所在结构体的地址 |

### os\_slist\_tail\_entry\_or\_null

该宏用于通过链表头和结构体类型，获取最后一个链表项所在结构体指针，若链表为空，返回OS\_NULL，定义如下：

```c
#define os_slist_tail_entry_or_null(head, type, member)                     \
    (!os_slist_empty(head) ? os_slist_tail_entry(head, type, member) : OS_NULL)
```

| **参数** | **说明** |
| :--- | :--- |
| head | 链表头 |
| type | 链表项所在结构体类型 |
| member | 链表在结构体中的名字 |
| **返回** | **说明** |
| 非OS\_NULL | 最后一个链表项所在结构体的地址 |
| OS\_NULL | 链表为空 |

### os\_slist\_for\_each

该宏用于遍历链表，定义如下：

```c
#define os_slist_for_each(pos, head)                                        \
    for (pos = (head)->next; pos != OS_NULL; pos = pos->next)
```

| **参数** | **说明** |
| :--- | :--- |
| pos | 当前遍历的链表项 |
| head | 链表头 |

### os\_slist\_for\_each\_safe

该宏用于安全的遍历链表，适用于遍历过程中需要删除链表项的情况，定义如下：

```c
#define os_slist_for_each_safe(pos, n, head)                                \
    for (pos = (head)->next, n = (pos != OS_NULL) ? pos->next : OS_NULL;    \
         pos != OS_NULL;                                                    \
         pos = n, n = (pos != OS_NULL) ? pos->next : OS_NULL)
```

| **参数** | **说明** |
| :--- | :--- |
| pos | 当前遍历的链表项 |
| n | 用来临时存储下一个链表项的指针，防止pos被删除后，无法继续遍历 |
| head | 链表头 |

---

## 应用示例

### 双向链表应用示例

本例演示了双向链表多个常用接口的使用方法

```c
#include <oneos_config.h>
#include <dlog.h>
#include <os_errno.h>
#include <shell.h>
#include <string.h>
#include <os_memory.h>
#include <os_list.h>

#define TEST_TAG        "TEST"

#define STUDENT_NUM     10
#define TEST_NAME_MAX   16

char *name[STUDENT_NUM] = {"xiaoming", "xiaohua", "xiaoqiang", "xiaoli", "xiaofang", "zhangsan", "lisi", "wangwu", "zhaoliu", "qianqi"};
os_uint32_t score[STUDENT_NUM] = {70, 83, 68, 80, 88, 86, 78, 92, 55, 82};

struct student_score
{
    os_list_node_t list_node;
    char name[TEST_NAME_MAX];
    os_uint32_t id;
    os_uint32_t score;
};
typedef struct student_score student_score_t;

void list_sample(void)
{
    os_uint32_t i = 0;
    os_list_node_t list_head = OS_LIST_INIT(list_head);
    student_score_t *data;
    student_score_t *data_temp;
    os_list_node_t *node;
    os_list_node_t *node_temp;

    LOG_W(TEST_TAG, "list_sample insert data");
    for (i = 0; i < STUDENT_NUM; i++)
    {
        data = os_malloc(sizeof(student_score_t));
        data->id = i;
        memset(data->name, 0, TEST_NAME_MAX);
        strncpy(data->name, name[i], TEST_NAME_MAX);
        data->score = score[i];
        if (i < STUDENT_NUM/2)
        {
            LOG_W(TEST_TAG, "insert tail -- id:%d score:%d name:%s", data->id, data->score, data->name);
            os_list_add_tail(&list_head, &data->list_node);
        }
        else
        {
            LOG_W(TEST_TAG, "insert front-- id:%d score:%d name:%s", data->id, data->score, data->name);
            os_list_add(&list_head, &data->list_node);
        }
    }

    LOG_W(TEST_TAG, "list_sample show result");
    os_list_for_each_entry(data, &list_head, student_score_t, list_node)
    {
        LOG_W(TEST_TAG, "id:%d score:%d name:%s", data->id, data->score, data->name);
    }

    LOG_W(TEST_TAG, "list_sample list_len is:%d", os_list_len(&list_head));
    LOG_W(TEST_TAG, "list_sample delete the score less than 60");
    os_list_for_each_entry_safe(data, data_temp, &list_head, student_score_t, list_node)
    {
        if (data->score < 60)
        {
            LOG_W(TEST_TAG, "delete -- id:%d score:%d name:%s", data->id, data->score, data->name);
            os_list_del(&data->list_node);
            os_free(data);
        }
    }

    LOG_W(TEST_TAG, "list_sample list_len is:%d", os_list_len(&list_head));
    LOG_W(TEST_TAG, "list_sample show result, and then delete");
    os_list_for_each_safe(node, node_temp, &list_head)
    {
        data = os_list_entry(node, student_score_t, list_node);
        LOG_W(TEST_TAG, "delete -- id:%d score:%d name:%s", data->id, data->score, data->name);
        os_list_del(&data->list_node);
        os_free(data);
    }

    LOG_W(TEST_TAG, "list_sample list_len is:%d", os_list_len(&list_head));
}

SH_CMD_EXPORT(test_list, list_sample, "test list");
```

运行结果如下：

```c
sh>test_list
W/TEST: list_sample insert data
W/TEST: insert tail -- id:0 score:70 name:xiaoming
W/TEST: insert tail -- id:1 score:83 name:xiaohua
W/TEST: insert tail -- id:2 score:68 name:xiaoqiang
W/TEST: insert tail -- id:3 score:80 name:xiaoli
W/TEST: insert tail -- id:4 score:88 name:xiaofang
W/TEST: insert front-- id:5 score:86 name:zhangsan
W/TEST: insert front-- id:6 score:78 name:lisi
W/TEST: insert front-- id:7 score:92 name:wangwu
W/TEST: insert front-- id:8 score:55 name:zhaoliu
W/TEST: insert front-- id:9 score:82 name:qianqi
W/TEST: list_sample show result
W/TEST: id:9 score:82 name:qianqi
W/TEST: id:8 score:55 name:zhaoliu
W/TEST: id:7 score:92 name:wangwu
W/TEST: id:6 score:78 name:lisi
W/TEST: id:5 score:86 name:zhangsan
W/TEST: id:0 score:70 name:xiaoming
W/TEST: id:1 score:83 name:xiaohua
W/TEST: id:2 score:68 name:xiaoqiang
W/TEST: id:3 score:80 name:xiaoli
W/TEST: id:4 score:88 name:xiaofang
W/TEST: list_sample list_len is:10
W/TEST: list_sample delete the score less than 60
W/TEST: delete -- id:8 score:55 name:zhaoliu
W/TEST: list_sample list_len is:9
W/TEST: list_sample show result, and then delete
W/TEST: delete -- id:9 score:82 name:qianqi
W/TEST: delete -- id:7 score:92 name:wangwu
W/TEST: delete -- id:6 score:78 name:lisi
W/TEST: delete -- id:5 score:86 name:zhangsan
W/TEST: delete -- id:0 score:70 name:xiaoming
W/TEST: delete -- id:1 score:83 name:xiaohua
W/TEST: delete -- id:2 score:68 name:xiaoqiang
W/TEST: delete -- id:3 score:80 name:xiaoli
W/TEST: delete -- id:4 score:88 name:xiaofang
W/TEST: list_sample list_len is:0
```

### 单向链表应用示例

本例演示了单向链表的多个接口的使用方法，实现和上面例子中双向链表一样的功能，但是单向链表的某些操作时间复杂度比双向链表高

```c
#include <oneos_config.h>
#include <dlog.h>
#include <os_errno.h>
#include <shell.h>
#include <string.h>
#include <os_memory.h>
#include <os_list.h>

#define TEST_TAG        "TEST"

#define STUDENT_NUM     10
#define TEST_NAME_MAX   16

char *name[STUDENT_NUM] = {"xiaoming", "xiaohua", "xiaoqiang", "xiaoli", "xiaofang", "zhangsan", "lisi", "wangwu", "zhaoliu", "qianqi"};
os_uint32_t score[STUDENT_NUM] = {70, 83, 68, 80, 88, 86, 78, 92, 55, 82};

struct student_score
{
    os_slist_node_t list_node;
    char name[TEST_NAME_MAX];
    os_uint32_t id;
    os_uint32_t score;
};
typedef struct student_score student_score_t;

void single_list_sample(void)
{
    os_uint32_t i = 0;
    os_slist_node_t list_head = OS_SLIST_INIT(list_head);
    student_score_t *data;
    os_slist_node_t *node_temp;
    os_slist_node_t *node;

    LOG_W(TEST_TAG, "single_list_sample insert data");
    for (i = 0; i < STUDENT_NUM; i++)
    {
        data = os_malloc(sizeof(student_score_t));
        data->id = i;
        memset(data->name, 0, TEST_NAME_MAX);
        strncpy(data->name, name[i], TEST_NAME_MAX);
        data->score = score[i];
        if (i < STUDENT_NUM/2)
        {
            LOG_W(TEST_TAG, "insert tail -- id:%d score:%d name:%s", data->id, data->score, data->name);
            os_slist_add_tail(&list_head, &data->list_node);
        }
        else
        {
            LOG_W(TEST_TAG, "insert front-- id:%d score:%d name:%s", data->id, data->score, data->name);
            os_slist_add(&list_head, &data->list_node);
        }
    }

    LOG_W(TEST_TAG, "single_list_sample show result");
    os_slist_for_each(node, &list_head)
    {
        data = os_slist_entry(node, student_score_t, list_node);
        LOG_W(TEST_TAG, "id:%d score:%d name:%s", data->id, data->score, data->name);
    }

    LOG_W(TEST_TAG, "single_list_sample list_len is:%d", os_slist_len(&list_head));
    LOG_W(TEST_TAG, "single_list_sample delete the score less than 60");
    os_slist_for_each_safe(node, node_temp, &list_head)
    {
        data = os_slist_entry(node, student_score_t, list_node);
        if (data->score < 60)
        {
            LOG_W(TEST_TAG, "delete -- id:%d score:%d name:%s", data->id, data->score, data->name);
            os_slist_del(&list_head, &data->list_node);
            os_free(data);
        }
    }

    LOG_W(TEST_TAG, "single_list_sample list_len is:%d", os_slist_len(&list_head));
    LOG_W(TEST_TAG, "single_list_sample show result, and then delete");
    os_slist_for_each_safe(node, node_temp, &list_head)
    {
        data = os_slist_entry(node, student_score_t, list_node);
        LOG_W(TEST_TAG, "delete -- id:%d score:%d name:%s", data->id, data->score, data->name);
        os_slist_del(&list_head, &data->list_node);
        os_free(data);
    }

    LOG_W(TEST_TAG, "single_list_sample list_len is:%d", os_slist_len(&list_head));
}

SH_CMD_EXPORT(test_single_list, single_list_sample, "test single list");
```

运行结果如下：

```c
sh>test_single_list
W/TEST: single_list_sample insert data
W/TEST: insert tail -- id:0 score:70 name:xiaoming
W/TEST: insert tail -- id:1 score:83 name:xiaohua
W/TEST: insert tail -- id:2 score:68 name:xiaoqiang
W/TEST: insert tail -- id:3 score:80 name:xiaoli
W/TEST: insert tail -- id:4 score:88 name:xiaofang
W/TEST: insert front-- id:5 score:86 name:zhangsan
W/TEST: insert front-- id:6 score:78 name:lisi
W/TEST: insert front-- id:7 score:92 name:wangwu
W/TEST: insert front-- id:8 score:55 name:zhaoliu
W/TEST: insert front-- id:9 score:82 name:qianqi
W/TEST: single_list_sample show result
W/TEST: id:9 score:82 name:qianqi
W/TEST: id:8 score:55 name:zhaoliu
W/TEST: id:7 score:92 name:wangwu
W/TEST: id:6 score:78 name:lisi
W/TEST: id:5 score:86 name:zhangsan
W/TEST: id:0 score:70 name:xiaoming
W/TEST: id:1 score:83 name:xiaohua
W/TEST: id:2 score:68 name:xiaoqiang
W/TEST: id:3 score:80 name:xiaoli
W/TEST: id:4 score:88 name:xiaofang
W/TEST: single_list_sample list_len is:10
W/TEST: single_list_sample delete the score less than 60
W/TEST: delete -- id:8 score:55 name:zhaoliu
W/TEST: single_list_sample list_len is:9
W/TEST: single_list_sample show result, and then delete
W/TEST: delete -- id:9 score:82 name:qianqi
W/TEST: delete -- id:7 score:92 name:wangwu
W/TEST: delete -- id:6 score:78 name:lisi
W/TEST: delete -- id:5 score:86 name:zhangsan
W/TEST: delete -- id:0 score:70 name:xiaoming
W/TEST: delete -- id:1 score:83 name:xiaohua
W/TEST: delete -- id:2 score:68 name:xiaoqiang
W/TEST: delete -- id:3 score:80 name:xiaoli
W/TEST: delete -- id:4 score:88 name:xiaofang
W/TEST: single_list_sample list_len is:0
```



