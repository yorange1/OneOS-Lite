# Socket套件

## 简介

OneOS socket组件为用户提供了一套兼容BSD的标准接口，用来实现网络连接及数据传输，下层涵盖了以太模块、蜂窝模组和wifi模组等不同制式的通信介质以及不同的通信协议栈。以太模块使用lwip协议栈，蜂窝模组和wifi模组使用at指令进行拨号连接和数据收发。

使用限制：标准socket组件使用时lwip和at只能二选一，在编译配置时决定。

## Socket API

### API简介

OneOS socket组件为用户提供了一套兼容BSD的标准接口，用来实现网络连接及数据传输，下层涵盖了以太模块、蜂窝模组和wifi模组等不同制式的通信介质以及不同的通信协议栈。以太模块使用lwip协议栈，蜂窝模组和wifi模组使用at指令进行拨号连接和数据收发。

使用限制：标准socket组件使用时lwip和at只能二选一，在编译配置时决定。

### API列表

| 接口          | 说明                                   |
| ------------- | -------------------------------------- |
| socket        | 创建套接字                             |
| closesocket   | 关闭套接字                             |
| shutdown      | shutdown套接字                         |
| bind          | 绑定ip和端口到套接字                   |
| listen        | 设置套接字为监听模式                   |
| accept        | 接受socket连接请求                     |
| connect       | 连接到服务器                           |
| sendto        | 可用于发送UDP数据                      |
| send          | 发送数据                               |
| recvfrom      | 可用于接收UDP数据                      |
| recv          | 接收数据                               |
| getsockopt    | 获取套接字属性                         |
| setsockopt    | 设置套接字属性                         |
| ioctlsocket   | 套接字模式控制                         |
| select        | 套接字状态监测                         |
| gethostbyname | 获取域名或主机名的IP地址               |
| getaddrinfo   | 将域名或主机名转为IP地址，服务转为端口 |
| freeaddrinfo  | 释放地址信息，与getaddrinfo配对使用    |
| getpeername   | 获取套接字远程信息                     |
| getsockname   | 获取套接字本地信息                     |

### API说明

#### socket

该函数用于新创建一个套接字（socekt），创建成功，返回套接字描述符，函数原型定义如下：

```
int socket(int domain, int type, int protocol);
```

| 参数     | 说明                         |
| -------- | ---------------------------- |
| domain   | 协议簇                       |
| type     | 套接字类型                   |
| protocol | 协议类型                     |
| 返回     | 说明                         |
| int      | 成功：套接字描述符，失败：-1 |

#### closesocket

该函数用于关闭一个套接字

```
int closesocket(int fd);
```

| 参数 | 说明         |
| ---- | ------------ |
| fd   | 套接字描述符 |
| 返回 | 说明         |
| 0    | 成功         |
| 非0  | 失败         |

#### shutdown

该函数shutdown一个套接字，函数原型定义如下：

```
int shutdown(int fd, int how);
```

| 参数 | 说明         |
| ---- | ------------ |
| fd   | 套接字描述符 |
| how  | 关闭方式     |
| 返回 | 说明         |
| 0    | 成功         |
| 非0  | 失败         |

备注：molink暂不支持该接口

#### bind

调用此函数将套接字（socket）和本地接口、端口绑定，函数原型定义如下：

```
int bind(int fd, const struct sockaddr *name, socklen_t namelen);
```

| 参数    | 说明                                                              |
| ------- | ----------------------------------------------------------------- |
| fd      | 套接字描述符                                                      |
| name    | 指向struct sockaddr 的指针，含绑定的地址信息：名称、端口和IP 地址 |
| namelen | 一般设置为sizeof(struct sockaddr)                                 |
| 返回    | 说明                                                              |
| 0       | 成功                                                              |
| 非0     | 失败                                                              |

备注：molink暂不支持该接口

#### listen

调用此函数将套接字设置为侦听模式，函数原型定义如下：

```
int listen(int fd, int backlog);
```

| 参数    | 说明                         |
| ------- | ---------------------------- |
| fd      | 套接字描述符                 |
| backlog | 连接请求队列可以容纳最大数目 |
| 返回    | 说明                         |
| 0       | 成功                         |
| 非0     | 失败                         |

备注：molink暂不支持该接口

#### connect

该函数用于建立与指定套接字的连接，函数原型定义如下：

```
int connect(int fd, const struct sockaddr *name, socklen_t namelen);
```

| 参数    | 说明                                                              |
| ------- | ----------------------------------------------------------------- |
| fd      | 套接字描述符                                                      |
| name    | 指向struct sockaddr 的指针，含绑定的地址信息：名称、端口和IP 地址 |
| namelen | 一般设置为sizeof(struct sockaddr)                                 |
| 返回    | 说明                                                              |
| 0       | 成功                                                              |
| 非0     | 失败                                                              |

#### accept

该函数等待一个新的连接，函数返回新连接的套接字描述符，函数原型定义如下：

```
int accept(int fd, struct sockaddr *addr, socklen_t *addrlen);
```

| 参数    | 说明                                                              |
| ------- | ----------------------------------------------------------------- |
| fd      | 套接字描述符                                                      |
| addr    | 指向struct sockaddr 的指针，含连接的地址信息：名称、端口和IP 地址 |
| addrlen | 一般设置为sizeof(struct sockaddr)                                 |
| 返回    | 说明                                                              |
| >= 0    | 新的连接套接字描述符                                              |
| < 0     | 失败                                                              |

备注：molink暂不支持该接口

#### sendto

该函数将数据发送到指定地址，用于发送UDP数据，函数原型定义如下：

```
int sendto(int fd, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
```

| 参数  | 说明                                                            |
| ----- | --------------------------------------------------------------- |
| fd    | 套接字描述符                                                    |
| data  | 待发送数据缓存区首地址                                          |
| size  | 待发送数据长度                                                  |
| flags | 发送标记，一般设为0                                             |
| to    | 指向struct sockaddr 的指针，含目的地址信息：名称、端口和IP 地址 |
| tolen | 一般设置为sizeof(struct sockaddr)                               |
| 返回  | 说明                                                            |
| > 0   | 数据发送成功的字节数                                            |
| <= 0  | 失败                                                            |

#### send

套接字fd发送数据，函数原型定义如下：

```
int send(int fd, const void *data, size_t size, int flags);
```

| 参数  | 说明                   |
| ----- | ---------------------- |
| fd    | 套接字描述符           |
| data  | 待发送数据缓存区首地址 |
| size  | 待发送数据长度         |
| flags | 发送标记，一般设为0    |
| 返回  | 说明                   |
| > 0   | 数据发送成功的字节数   |
| <= 0  | 失败                   |

#### recvfrom

指定套接字上接收数据，函数原型定义如下：

```
int recvfrom(int fd, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
```

| 参数    | 说明                                                              |
| ------- | ----------------------------------------------------------------- |
| fd      | 套接字描述符                                                      |
| men     | 接收数据缓存区首地址                                              |
| len     | 接收缓存区长度                                                    |
| flags   | 接收标记                                                          |
| from    | 指向struct sockaddr 的指针，含数据源地址信息：名称、端口和IP 地址 |
| fromlen | 一般设置为sizeof(struct sockaddr)                                 |
| 返回    | 说明                                                              |
| > 0     | 成功接收到数据字节数                                              |
| <= 0    | 失败                                                              |

#### recv

该函数用于指定套接字上接收数据，函数原型定义如下：

```
int recv(int fd, void *mem, size_t len, int flags);
```

| 参数  | 说明                 |
| ----- | -------------------- |
| fd    | 套接字描述符         |
| mem   | 接收数据缓存区首地址 |
| len   | 接收缓存区长度       |
| flags | 接收标记             |
| 返回  | 说明                 |
| > 0   | 成功接收到数据字节数 |
| <= 0  | 失败                 |

#### getsockopt

该函数用于设定对应套接字选项，函数原型定义如下：

```
int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen);
```

| 参数    | 说明             |
| ------- | ---------------- |
| fd      | 套接字描述符     |
| level   | 协议标准         |
| optname | 子选项           |
| optval  | 指针，选项值     |
| optlen  | 指针，选项值长度 |
| 返回    | 说明             |
| 0       | 成功             |
| 非0     | 失败             |

#### setsockopt

该函数，获取套接字选项值配置，函数原型定义如下：

```
int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
```

| 参数    | 说明         |
| ------- | ------------ |
| fd      | 套接字描述符 |
| level   | 协议标准     |
| optname | 子选项       |
| optval  | 指针，选项值 |
| optlen  | 选项值长度   |
| 返回    | 说明         |
| 0       | 成功         |
| 非0     | 失败         |

#### ioctlsocket

套接字模式控制，函数原型定义如下：

```
int ioctlsocket(int fd, long cmd, void *argp);
```

| 参数 | 说明                   |
| ---- | ---------------------- |
| fd   | 套接字描述符           |
| cmd  | 操作命令码             |
| argp | 操作命令所带参数的指针 |
| 返回 | 说明                   |
| 0    | 成功                   |
| 非0  | 失败                   |

备注：molink暂不支持该接口

#### select

该函数，readset/writeset/execptset有效或者超时后函数返回，函数原型定义如下：

```
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
```

| 参数      | 说明                 |
| --------- | -------------------- |
| maxfdp1   | 最大套接字描述符 + 1 |
| readset   | 读套接字描述符集合   |
| writeset  | 写套接字描述符集合   |
| exceptset | 异常套接字描述符结合 |
| timeout   | 超时时间             |
| 返回      | 说明                 |
| > 0       | 成功                 |
| 0         | 超时                 |
| < 0       | 失败                 |

#### gethostbyname

获取域名或主机名的IP地址，函数原型定义如下：

```
struct hostent *gethostbyname(const char *name);
```

| 参数   | 说明           |
| ------ | -------------- |
| name   | 域名或者主机名 |
| 返回值 | 说明           |
| NULL   | 失败           |
| 非NULL | 成功           |

#### getaddrinfo

该函数将域名或主机名转为IP地址，服务转为端口，函数原型定义如下：

```
int  getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
```

| 参数     | 说明                 |
| -------- | -------------------- |
| nodename | 一个主机名或者地址串 |
| servname | 服务器名，端口号     |
| hints    | 期望返回的信息类型   |
| res      | 返回地址信息         |
| 返回     | 说明                 |
| 0        | 成功                 |
| 非0      | 失败                 |

#### freeaddrinfo

与getaddrinfo配对使用，释放获取信息，函数原型定义如下：

```
void freeaddrinfo(struct addrinfo *ai);
```

| 参数 | 说明           |
| ---- | -------------- |
| ai   | 待释放地址信息 |
| 返回 | 说明           |
| 无   | 无             |

#### getpeername

该函数可以取得一个已经连接上的套接字的远程信息（比如IP 地址和端口），函数原型定义如下：

```
int getpeername (int fd, struct sockaddr *name, socklen_t *namelen);
```

| 参数    | 说明                                                              |
| ------- | ----------------------------------------------------------------- |
| fd      | 套接字描述符                                                      |
| name    | 指向struct sockaddr 的指针，含连接的地址信息：名称、端口和IP 地址 |
| namelen | 一般设置为sizeof(struct sockaddr)                                 |
| 返回    | 说明                                                              |
| 0       | 成功                                                              |
| 非0     | 失败                                                              |

备注：molink暂不支持该接口

#### getsockname

该函数返回套接字描述符的本地信息（比如IP 地址和端口），函数原型定义如下：

```
int getsockname (int fd, struct sockaddr *name, socklen_t *namelen);
```

| 参数    | 说明                                                              |
| ------- | ----------------------------------------------------------------- |
| fd      | 套接字描述符                                                      |
| name    | 指向struct sockaddr 的指针，含连接的地址信息：名称、端口和IP 地址 |
| namelen | 一般设置为sizeof(struct sockaddr)                                 |
| 返回    | 说明                                                              |
| 0       | 成功                                                              |
| 非0     | 失败                                                              |

备注：molink暂不支持该接口

## 配置指南

#### menuconfig的配置

##### step 1. 使能BSD socket API

```
(Top) → Components→ Network→ Socket
                             OneOS-Lite Configuration
[*] Enable BSD socket API
        protocol stack implement (Support OneOS modules stack)  --->
```

##### step 2. 根据项目需求配置是否启用设备文件系统

```
(Top) → Components→ FileSystem

[*] Enable virtual file system
(4)     The max number of mounted file system
(2)     The max number of file system type
(16)    The max number of opened files
[*]     Enable DevFS file system
[ ]     Enable CuteFs file system
[ ]     Enable JFFS2
[ ]     Enable Yaffs2 file system
[ ]     Enable FatFs
[ ]     Enable NFS v3 client file system
[ ]     Enable little filesystem

```

`Enable DevFS file system`, FileSystem配置项影响select函数的实现，如果启用`Enable DevFS file system`，select实现基于vfs的poll机制；如果不启用`Enable DevFS file system`,select由molink或者lwip单独实现。

##### step 3. molink配置

以ESP8266配置举例，Enable *** BSD Socket Operates必须启用

```
(Top) → Components → Network → Molink → Enable IoT modules support → Modules → WiFi Modules Support → ESP8266 → ESP8266 Config
                                OneOS Configuration
[*] Enable ESP8266 Module Object Auto Create
(uart2) ESP8266 Interface Device Name
(115200) ESP8266 Interface Device Rate
(512)   The maximum length of AT command data accepted
[*]     Enable ESP8266 Module Auto Connect
(hw_hsj) ESP8266 Connect AP SSID
(Aa123456) ESP8266 Connect AP Password
[*] Enable ESP8266 Module General Operates
-*- Enable ESP8266 Module WiFi Operates
[*] Enable ESP8266 Module Network Service Operates
-*- Enable ESP8266 Module Network TCP/IP Operates
[*] Enable ESP8266 Module BSD Socket Operates
```

##### step 4. lwip配置

```
[*] Enable lwIP stack
        lwIP version (lwIP v2.1.2)  --->
[ ]     IPV6 protocol
[*]     IGMP protocol
-*-     ICMP protocol
[ ]     SNMP protocol
[*]     Enble DNS for name resolution
[*]     Enable alloc ip address through DHCP
(1)         SOF broadcast
(1)         SOF broadcast recv
[ ]     Enable DHCP server
        Static IPv4 Address  --->
-*-     UDP protocol
[*]     TCP protocol
-*-     RAW protocol
[ ]     PPP protocol
[ ]     NAT protocol
(8)     the number of struct netconns
(16)    the number of PBUF
(4)     the number of raw connection
(4)     the number of UDP socket
(4)     the number of TCP socket
(40)    the number of TCP segment
(8196)  the size of send buffer
(8196)  the size of TCP send window
(10)    the priority level value of lwIP task
(8)     the number of mail in the lwIP task mailbox
(1024)  the stack size of lwIP task
[ ]     Not use Rx task
[ ]     Not use Tx task                                       
```

## 应用示例

包含标准的socket头文件即可，如

```
#include <atest.h>
#include <shell.h>
#include <os_kernel.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#ifdef OS_USING_POSIX
#include <sys/select.h>
#endif
#include <os_mutex.h>
#include <os_dbg_ext.h>

os_task_t *gs_socket_select_task = OS_NULL;
int gs_select_fd = -1;
os_mutex_t *gs_socket_mutex = OS_NULL;

static void socket_dump_hex(const os_uint8_t *ptr, os_size_t buflen)
{
    unsigned char *buf;
    int i;
    int j;

    buf = (unsigned char *)ptr;

    for (i = 0; i < buflen; i += 16)
    {
        os_kprintf("%08X: ", i);

        for (j = 0; j < 16; j++)
        {
            if (i + j < buflen)
            {
                os_kprintf("%02X ", buf[i + j]);
            }
            else
            {
                os_kprintf("   ");
            }
        }
            
        os_kprintf("\n");
    }
}

static void socket_do_select_task(void *parm)
{
    os_uint8_t recv_buf[128];
    fd_set readfds;    
    fd_set exfds;
    int maxfd;
    int err;
    int fd;
    
    while (1)
    {
        os_mutex_lock(gs_socket_mutex, OS_IPC_WAITING_FOREVER);
        if (gs_select_fd < 0)
        {
            os_mutex_unlock(gs_socket_mutex);
            os_task_msleep(100);
            
            continue;
        }
        
        fd = gs_select_fd;
        maxfd = fd + 1; 
        os_mutex_unlock(gs_socket_mutex);
        
        do
        {    
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            FD_ZERO(&exfds);
            FD_SET(fd, &exfds);

            err = select(maxfd, &readfds, OS_NULL, OS_NULL, OS_NULL);
            if (err > 0)
            {
                if (FD_ISSET(fd, &readfds))
                {
                    err = recv(fd, recv_buf, sizeof(recv_buf), 0);
                    
                    os_kprintf("recv len[%d]\n", err);
                    if (err > 0)
                    {
                        socket_dump_hex(recv_buf, err);
                    }
                    else
                    {                        
                        os_mutex_lock(gs_socket_mutex, OS_IPC_WAITING_FOREVER);
                        if(gs_select_fd >= 0)
                        {
                            closesocket(gs_select_fd);
                            gs_select_fd = -1;
                        }
                        os_mutex_unlock(gs_socket_mutex);
                        break;
                    }
                }
            }
            
            os_mutex_lock(gs_socket_mutex, OS_IPC_WAITING_FOREVER);
            if (gs_select_fd != fd)
            {
                os_mutex_unlock(gs_socket_mutex);
                break;
            }
            os_mutex_unlock(gs_socket_mutex);
        } while (err >= 0);
        os_task_msleep(50);
    }
}

static int socket_cmd_select_init(void)
{
    if (OS_NULL == gs_socket_mutex)
    {
        gs_socket_mutex = os_mutex_create("slt_mtx", OS_IPC_FLAG_FIFO, OS_FALSE);
    }
    
    if (OS_NULL == gs_socket_mutex)
    {
        os_kprintf("mutex failed\n");
        return OS_ERROR;
    }

    if (OS_NULL == gs_socket_select_task)
    {
        gs_socket_select_task = os_task_create("slt_task", socket_do_select_task, 
                                OS_NULL, 1024, OS_TASK_PRIORITY_MAX - 5, 5);
        if (OS_NULL != gs_socket_select_task)
        {
            (void)os_task_startup(gs_socket_select_task);
        }
        else
        {
            os_kprintf("select task create failed\n");
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

static int socket_cmd_socket_select(int argc, char **argv)
{
    struct sockaddr_in server_addr;
    char send_buf[] = "hello world";
    int type;
    int flag;
    int port;
    int err;

    if (argc != 5)
    {
        os_kprintf("Input errror, please input: socket_select flag ip port type\n");
        return OS_ERROR;
    }
    
    err = socket_cmd_select_init();
    if (OS_EOK != err)
    {
        os_kprintf("Do socket select init failed\n");
        return OS_ERROR;
    }

    flag = atoi(argv[1]);
    port = atoi(argv[3]);

    if (!strcmp(argv[4], "udp"))
    {
        type = SOCK_DGRAM;
    }
    else if (!strcmp(argv[4], "tcp"))
    {
        type = SOCK_STREAM;
    }
    else
    {
        type = SOCK_RAW;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(argv[2]); 
    
    if (!flag)
    {        
        os_kprintf("stop socket select fd = %d\n", gs_select_fd);
        os_mutex_lock(gs_socket_mutex, OS_IPC_WAITING_FOREVER);
        if (gs_select_fd >= 0)
        {
            closesocket(gs_select_fd);
            gs_select_fd = -1;
        }        
        os_mutex_unlock(gs_socket_mutex);        
    }
    else
    { 
        os_mutex_lock(gs_socket_mutex, OS_IPC_WAITING_FOREVER);
        if (gs_select_fd >= 0)
        {
            os_mutex_unlock(gs_socket_mutex);
            os_kprintf("socket select fd = %d is already exist\n", gs_select_fd);
            return OS_EOK;
        }
        
        gs_select_fd = socket(AF_INET, type, 0);
        if (gs_select_fd < 0)
        {            
            os_mutex_unlock(gs_socket_mutex);
            os_kprintf("why socket failed = %d\n", gs_select_fd);
            return OS_ERROR;
        }

        err = connect(gs_select_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
        if (err < 0)
        {
            os_kprintf("connect socket =%d failed\n", gs_select_fd);
            closesocket(gs_select_fd);
            gs_select_fd = -1;         
            os_mutex_unlock(gs_socket_mutex);
            
            return OS_ERROR;
        }

        if (SOCK_DGRAM == type)
        {
            err = sendto(gs_select_fd, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));           
            os_kprintf("udp[fd = %d] send = %d\n", gs_select_fd, err);
        }        
        os_mutex_unlock(gs_socket_mutex);
    }

    return OS_EOK;
}

SH_CMD_EXPORT(socket_select, socket_cmd_socket_select, "socket_select 1/0 ipaddr port tcp/udp");

```