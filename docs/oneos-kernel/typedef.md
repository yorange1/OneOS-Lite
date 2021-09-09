# 基础数据类型

---

## 简介

OneOS内核使用一些常规的宏和数据类型，他们在os_types.h中定义。

---

## 宏定义

```c
/* Maximum value of base type */
#define OS_UINT8_MAX                    0xffU                   /* Maxium value of UINT8 */
#define OS_UINT16_MAX                   0xffffU                 /* Maxium value of UINT16 */
#define OS_UINT32_MAX                   0xffffffffU             /* Maxium value of UINT32 */
#define OS_UINT64_MAX                   0xffffffffffffffffULL   /* Maxium value of UINT64 */

#ifdef OS_ARCH_CPU_64BIT
#define OS_TICK_MAX                     OS_UINT64_MAX           /* Maxium value of tick */
#else
#define OS_TICK_MAX                     OS_UINT32_MAX           /* Maxium value of tick */
#endif

/* Alignment size for CPU architecture data access */
#define OS_ALIGN_SIZE                   sizeof(os_base_t)
```

---

## 数据类型重定义

内核使用自己重定义的基本数据类型，具体如下：

```c
typedef signed char                     os_int8_t;              /* 8bit integer type */
typedef signed short                    os_int16_t;             /* 16bit integer type */
typedef signed int                      os_int32_t;             /* 32bit integer type */
typedef signed long long                os_int64_t;             /* 64bit integer type */
typedef unsigned char                   os_uint8_t;             /* 8bit unsigned integer type */
typedef unsigned short                  os_uint16_t;            /* 16bit unsigned integer type */
typedef unsigned int                    os_uint32_t;            /* 32bit unsigned integer type */
typedef unsigned long long              os_uint64_t;            /* 64bit unsigned integer type */
typedef signed int                      os_bool_t;              /* Boolean type */

/* Represent Nbit CPU */
typedef signed long                     os_base_t;              /* Nbit CPU related date type */
typedef unsigned long                   os_ubase_t;             /* Nbit unsigned CPU related data type */

/* Else data type definitions */
typedef os_int32_t                      os_err_t;               /* Type for error number */
typedef os_ubase_t                      os_tick_t;              /* Type for tick count */ 
typedef os_ubase_t                      os_size_t;              /* Type for size */
typedef os_base_t                       os_off_t;               /* Type for offset */
```

数据类型补充说明如下：

| **类型** | **说明** |
| :--- | :--- |
| os\_base\_t | 机器长度大小有符号整形数据类型 |
| os\_ubase\_t | 机器长度大小无符号整形数据类型 |
| os\_err\_t | 错误码数据类型 |
| os\_tick\_t | 系统时钟节拍数据类型 |
| os\_size\_t | 机器长度大小无符号整形数据类型 |
| os\_off\_t | 偏移数据类型 |



