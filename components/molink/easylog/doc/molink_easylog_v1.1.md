# MOLINK -- easylog V1.1使用手册

- [MOLINK --easylog V1.1使用手册](#MOLINK--easylog-v1.1使用手册)
  - [一、简介](#一简介)
  - [二、工程配置](#二工程配置)
    - [1. 配置示例](#1-配置示例)
    - [2. 特殊配置说明](#2-特殊配置说明)
  - [三、重要数据结构](#三重要数据结构)
    - [日志信息数据结构](#日志信息数据结构)
    - [模组注网状态数据结构](#模组注网状态数据结构)
  - [四、Easylog API](#四easylog API)
  - [五、使用方式](#五使用方式)
  - [六、使用示例](#六使用示例)
  - [七、注意与说明](#七注意与说明)
  - [八、后续优化建议](#八后续优化建议)

## 一、简介

OneOS的molink为无线网络模组提供了一套统一的管理接口，但用户需要获取模组的注网状态、IMEI、IMSI、ICCID、CELL ID等等状态和信息，仍需要用户应用层逐个调用molink对应的API进行获取，而目前基于对表计等行业的业务场景了解，上述信息都是客户普遍关心的。因此，设计molink的无线模组网络信息日志组件easylog，将用户关心的网络信息封装成统一的数据帧，用户可根据实际产品和应用场景需要对日志数据帧内容进行后续自定义处理。

easylog V1.1提供的功能主要如下：

* 通过配置确定所要收集的日志包含的信息内容，每项信息均可灵活裁剪，根据客户需要选择；
* 日志信息包含系统时间、模组注网状态、IMEI、IMSI、ICCID、CSQ、PCI、EARFCN、CELLID、RSRP、RSRQ、SNR、TX_PWR、ECL、用户自定义数据、CHECKSUM，网络模组相关信息全部可通过配置由用户应用需求灵活裁剪；
* 执行创建一条日志操作，easylog组件会封装一条连带用户数据及配置对应的日志报文，日志信息的后续处理需要用户自定义；
* 提供销毁一条日志接口，用户创建一条日志后会申请相应的内存资源，用户需要根据应用在合适的时机释放相关资源，避免内存泄漏；
* 提供数据流按日志数据格式解析的printf接口；
* 当配置启用校验和功能时，提供读回数据正确性校验接口；

---

## 二、工程配置

### 1. 配置示例

用户根据使用需求及软硬件环境，使能molink easylog组件，并根据实际需要完成所需日志信息配置并保存，生成工程
Cube工具配置路径：(Top) → Components→ Network→ Molink → Easylog

```c
[*] Enable molink network log info features            ------->easylog功能使能
[*]     Enable easylog imei features                   ------->配置启用IMEI信息
[*]     Enable easylog imsi features                   ------->配置启用IMSI信息
[*]     Enable easylog iccid features                  ------->配置启用ICCID信息
[*]     Enable easylog csq features                    ------->配置启用信号质量CSQ信息
[*]     Enable easylog radio info features             ------->配置使能radio信息
[*]         Enable easylog pci info features           ------->配置启用PCI信息
[*]         Enable easylog earfcn info features        ------->配置启用EARFCN信息
[*]         Enable easylog cell id info features       ------->配置启用CELL ID信息
[*]         Enable easylog rsrp info features          ------->配置启用RSRP信息
[*]         Enable easylog rsrq info features          ------->配置启用RSRQ信息
[*]         Enable easylog snr info features           ------->配置启用SNR信息
[*]         Enable easylog tx power info features      ------->配置启用TX POWER信息
[*]         Enable easylog ecl info features           ------->配置启用ECL信息
[*]     Enable easylog user-defined data area          ------->配置使能用户自定义数据空间
(8)         Define easylog user-defined data area len  ------->配置用户自定义数据长度默认配置为8字节
[*]     Enable easylog info checksum features          ------->配置使能校验和功能
[*]     Enable easylog info printf features            ------->配置使能日志信息打印功能，日志信息创建并更新后会解析打印日志信息
```

### 2. 特殊配置说明

|NO.|选项|说明|
|:------------:|:-------------------|:------------------------|
|1|用户自定义数据区|默认不配置，若用户有自定义数据则需要根据实际需要配置打开及配置数据大小|
|2| 校验和           |默认不配置，用户对日志信息有校验需求自行配置|
|3|日志信息打印|默认不配置，属于调试功能，可在开发调测试阶段按需自行配置|

---

## 三、重要数据结构

### 日志信息数据结构

molink easylog组件使能中可配置项对应如下数据结构，可选配置项供用户开启molink easylog组件时自定义配置选项，可配置项说明见配置示例，在此不复述。需要注意的是，前面的上传云服务器标记upload_flag、日志报文长度len、模组注网状态net_status是不可配置的，是每条日志必包含的信息，系统时间sys_time获取依赖RTC功能，需要支持RTC功能。

```c
typedef struct mo_easylog
{
    os_uint8_t  upload_flag;
    os_uint16_t len;
    os_uint8_t  net_status;
#ifdef OS_USING_RTC
    char        sys_time[MO_EASYLOG_SYS_TIME_LEN + 1];  /* "Jan  1 01:07:59 2000"--2000年01月01日01时07分59秒 */
#endif
#ifdef MOLINK_EASYLOG_USING_IMEI
    char        imei[MO_IMEI_LEN + 1];
#endif
#ifdef MOLINK_EASYLOG_USING_IMSI
    char        imsi[MO_IMSI_LEN + 1];
#endif
#ifdef MOLINK_EASYLOG_USING_ICCID
    char        iccid[MO_ICCID_LEN + 1];
#endif
#ifdef MOLINK_EASYLOG_USING_CSQ
    os_uint8_t  csq_rssi;
    os_uint8_t  csq_ber;
#endif
#ifdef MOLINK_EASYLOG_USING_PCI
    os_int32_t  pci;
#endif
#ifdef MOLINK_EASYLOG_USING_EARFCN
    os_int32_t  earfcn;
#endif
#ifdef MOLINK_EASYLOG_USING_CELLID
    char        cell_id[CELL_ID_MAX_LEN + 1];
#endif
#ifdef MOLINK_EASYLOG_USING_RSRP
    os_int32_t  rsrp;
#endif
#ifdef MOLINK_EASYLOG_USING_RSRQ
    os_int32_t  rsrq;
#endif
#ifdef MOLINK_EASYLOG_USING_SNR
    os_int32_t  snr;
#endif
#ifdef MOLINK_EASYLOG_USING_TX_PWR
    os_int32_t  tx_power;
#endif
#ifdef MOLINK_EASYLOG_USING_ECL
    os_int32_t  ecl;
#endif
#ifdef MOLINK_EASYLOG_USING_USER_DATA
    os_uint8_t  user_data[MO_EASYLOG_USER_DATA_LEN];
#endif
#ifdef MOLINK_EASYLOG_USING_CHECKSUM
    os_uint8_t  checksum;
#endif
} mo_easylog_t;
```

### 模组注网状态数据结构

根据一般的注网流程，定义了模组的注网状态码，每个状态码的解析如下

```c
typedef enum mo_easylog_network_status
{
    NETWORK_STATUS_OK = 0,          ------->模组注网OK
    NETWORK_AT_TEST_ERROR,          ------->模组与MCU无法通讯
    NETWORK_CFUN_STATUS_ERROR,      ------->模组CFUN未使能，无法注网
    NETWORK_SIM_CARD_ERROR,         ------->读取SIM卡信息异常，无法注网
    NETWORK_ATTACH_STATUS_ERROR,    ------->模组attach失败，无法注网
    NETWORK_CEREG_STATUS_ERROR,     ------->模组注册网络失败
    NETWORK_CSQ_WARNING,            ------->模组注网成功，但是信号低于MO_EASTLOG_CSQ_WARNING_LV定义值，目前定义是10，用户可自行更改等级
    NETWORK_STATUS_UNDEF = 0xFF,    ------->未定义状态
} mo_easylog_network_status_t;
```

---

## 四、easylog API

easylog对用户开放了四个API接口，详细介绍如下。

**1、创建一条easylog 日志信息接口：**

```c
mo_easylog_t *mo_easylog_create(const os_uint8_t *user_data, os_size_t data_len);
```

| **参数**  | **说明**                                                     |
| :-------- | :----------------------------------------------------------- |
| user_data | 日志的用户自定义数据地址data，该数据将填写到日志的user_data中 |
| data_len  | 日志的用户自定义数据大小，最大长度由MO_EASYLOG_USER_DATA_LEN定义 |
| **返回**  | **说明**                                                     |
| 非OS_NULL | 成功则返回创建日志信息对象指针                               |
| OS_NULL   | 失败则返回OS_NULL                                            |

**2、销毁一条easylog 日志信息接口：**

```c
os_err_t mo_easylog_destory(mo_easylog_t *easylog);
```

| **参数** | **说明**                   |
| :------- | :------------------------- |
| easylog  | 所要销毁的日志信息对象指针 |
| **返回** | **说明**                   |
| OS_EOK   | 成功则返回OS_EOK           |
| 非OS_EOK | 销毁失败                   |

**3、开启校验功能后日志信息数据校验接口：**

当用户配置开启CHECKSUM功能后，创建的日志信息将包含累加校验和checksum，用户在其他地方获取日志信息后，可调用rx checksum接口对日志信息进行校验，如不配置CHECKSUM功能，则该接口无定义。

```c
os_uint8_t mo_easylog_calculate_rx_checksum(os_uint8_t *data, os_uint32_t len);
```

| **参数** | **说明**                                     |
| :------- | :------------------------------------------- |
| data     | 所读取完整日志的起始地址，按os_uint8_t *传入 |
| len      | 读取日志信息长度，需为sizeof(mo_easylog_t)   |
| **返回** | **说明**                                     |
| 0        | 成功，日志信息正确                           |
| 非0      | 失败，日志信息有坏数据                       |

**4、easylog 信息解析打印接口：**

```c
os_err_t mo_easylog_info_printf(mo_easylog_t *easylog_obj);
```

| **参数**    | **说明**                                     |
| :---------- | :------------------------------------------- |
| easylog_obj | 所读取完整日志的起始地址，按os_uint8_t *传入 |
| **返回**    | **说明**                                     |
| OS_EOK      | 成功                                         |
| 非OS_EOK    | 日志信息解析及打印失败                       |



## 五、使用方式

> 按要求调用API编译烧录即可

## 六、使用示例

配置启用easylog组件后，接口使用示例见mo_easylog.c文件的static void mo_easylog_test_info_printf(void)，开启shell功能后可直用对应命令测试。

## 七、注意与说明

* V1.1版本为试验版本，测试使用中如有额外需求或是根据项目应用需作出调整，可与开发联系修改。

## 八、后续优化建议

* 在实际产品中，模组和SIM卡配套绑定，不会变更，因此IMEI、IMSI、ICCID这类固定不变的信息可以分配一个静态数据区单独存储，理论上只需获取写入一次即可，其他可变信息动态查询更新，这样每条日志能减少封装日志信息所需时间，提升效率。
* 目前仅是依据个人理解，封装了日志的格式和信息，用户关心的数据可能会更多，可以根据实际应用情况不断扩充包含进来。
* easylog是高度可扩展、可裁剪的组件，用户可以根据产品实际需要对本组件进行改造，以更符合实际产品的需要。如用户可扩展设备类型dev_type、序列号SN、设备故障码err_code、软件版本号firmware_version、硬件版本号hardware_version、数据加密等功能。
* 对于日志的部分数据或者整个日志进行压缩，以减少单条日志的大小。