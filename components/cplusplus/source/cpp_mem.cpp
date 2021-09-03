/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <os_memory.h>
#include "cpp_mem.h"

void *operator new[](size_t size)
{
    return os_malloc(size);
}

void *operator new(size_t size)
{
    return os_malloc(size);
}

void operator delete[](void *ptr)
{
    os_free(ptr);
}

void operator delete(void *ptr)
{
    os_free(ptr);
}

