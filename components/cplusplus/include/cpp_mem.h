/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef CPP_MEM_H
#define CPP_MEM_H

#include <inttypes.h>
#include <stdlib.h>

void *operator new(size_t size);
void *operator new[](size_t size);

void operator delete(void * ptr);
void operator delete[](void *ptr);

#ifdef __cplusplus
extern "C" {
#endif

int  cpp_init(void);

#ifdef __cplusplus
}
#endif
#endif
