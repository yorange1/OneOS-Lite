/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        os_mem_small.c
 *
 * @brief       This file implements the functions to allocate and free dynamic memory.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-09   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <dlog.h>
#include <os_sem.h>
#include <os_errno.h>
#include <os_memory.h>
#include <string.h>

#include "os_kernel_internal.h"

#if defined (OS_USING_MEM_HEAP) && defined (OS_USING_ALG_SMALL)

#define SMALL_TAG               "MSMALL"

#define HEAP_MAGIC 0x1ea0
struct small_block
{
    os_uint16_t magic;        /* Magic and used flag */
    os_uint16_t used;

    os_size_t next, prev;

#ifdef OS_USING_MEM_TRACE
    os_uint8_t task[4];      /* Task name */
#endif
};

struct small_heap
{
    /* The pointer to the heap: for alignment, heap_ptr is now a pointer instead of an array */
    os_uint8_t *heap_ptr;
    
    /* The last entry, always unused! */
    struct small_block *heap_end;

    /* The pointer to the lowest free block */
    struct small_block *lfree;

    os_size_t   available_total;
};

#define MIN_SIZE 12
#define MIN_SIZE_ALIGNED     OS_ALIGN_UP(MIN_SIZE, OS_ALIGN_SIZE)
#define SIZEOF_STRUCT_MEM    OS_ALIGN_UP(sizeof(struct small_block), OS_ALIGN_SIZE)


#ifdef OS_MEM_STATS

#define SMALL_MEM_INFO_INIT(x, size) do {\
    x->mem_total = (size);                  \
    x->mem_used = 0;                        \
    x->mem_maxused = 0;                     \
} while(0)

#define SMALL_MEM_USED_INC(x, size)  do {\
    x->mem_used += (size);                  \
    if (x->mem_used > x->mem_maxused) {     \
        x->mem_maxused = x->mem_used;       \
    }                                       \
}while(0)

#define SMALL_MEM_USED_DEC(x, size)  do {\
    x->mem_used -= (size);                  \
}while(0)

#else

#define SMALL_MEM_INFO_INIT(x, size)

#define SMALL_MEM_USED_INC(x, size)

#define SMALL_MEM_USED_DEC(x, size)

#endif

static void _k_small_mem_free(struct heap_mem *h_mem, void *rmem);

#ifdef OS_USING_MEM_TRACE
OS_INLINE void os_mem_setname(struct small_block *mem, const char *name)
{
    int index;
    for (index = 0; index < sizeof(mem->task); index ++)
    {
        if ('\0' == name[index])
        {
            break;
        }
        mem->task[index] = name[index];
    }

    for (; index < sizeof(mem->task); index ++)
    {
        mem->task[index] = ' ';
    }
}
#endif

static void plug_holes(struct heap_mem *h_mem, struct small_block *mem)
{
    struct small_block *nmem;
    struct small_block *pmem;
    struct small_heap  *h;
    
    h = (struct small_heap *)h_mem->header;
    
    OS_ASSERT((os_uint8_t *)mem >= h->heap_ptr);
    OS_ASSERT((os_uint8_t *)mem < (os_uint8_t *)h->heap_end);
    OS_ASSERT(0 == mem->used);

    /* Plug hole forward */
    nmem = (struct small_block *)&h->heap_ptr[mem->next];
    if (mem != nmem &&
        0 == nmem->used &&
        (os_uint8_t *)nmem != (os_uint8_t *)h->heap_end)
    {
        /* If mem->next is unused and not end of h->heap_ptr,
         * combine mem and mem->next
         */
        if (h->lfree == nmem)
        {
            h->lfree = mem;
        }
        mem->next = nmem->next;
        ((struct small_block *)&h->heap_ptr[nmem->next])->prev = (os_uint8_t *)mem - h->heap_ptr;
    }

    /* Plug hole backward */
    pmem = (struct small_block *)&h->heap_ptr[mem->prev];
    if (pmem != mem && 0 == pmem->used)
    {
        /* If mem->prev is unused, combine mem and mem->prev */
        if (h->lfree == mem)
        {
            h->lfree = pmem;
        }
        pmem->next = mem->next;
        ((struct small_block *)&h->heap_ptr[mem->next])->prev = (os_uint8_t *)pmem - h->heap_ptr;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function initlializes the default heap in the system.
 *
 * @param[in]       begin_addr      The start address of heap.
 * @param[in]       end_addr        The end address of heap.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void _k_small_mem_init(struct heap_mem *h_mem, void *mem, os_size_t bytes)
{
    struct small_block *heap_obj;
    struct small_heap  *h;
    os_uint32_t head_align;
    os_uint32_t begin_align;
    os_uint32_t end_align;
    
    head_align = OS_ALIGN_UP((os_uint32_t)mem, OS_ALIGN_SIZE);
    h_mem->header = (void*)head_align;
    
    begin_align = head_align + sizeof(struct small_heap);
    begin_align = OS_ALIGN_UP(begin_align, OS_ALIGN_SIZE);
    end_align = OS_ALIGN_DOWN((os_uint32_t)mem + bytes, OS_ALIGN_SIZE);

    OS_ASSERT(OS_FALSE == os_is_irq_active());

    /* Alignment addr */
    if ((end_align > (2 * SIZEOF_STRUCT_MEM)) &&
        ((end_align - 2 * SIZEOF_STRUCT_MEM) >= begin_align))
    {
        /* Calculate the aligned memory size */
        h_mem->mem_total = end_align - head_align;
        SMALL_MEM_USED_INC(h_mem, begin_align - head_align);
        SMALL_MEM_USED_INC(h_mem, 2 * SIZEOF_STRUCT_MEM);   
    }
    else
    {
        OS_KERN_LOG(KERN_ERROR, SMALL_TAG, "mem init, error begin address 0x%x, and size 0x%x",
                    (os_uint32_t)mem, (os_uint32_t)bytes);

        return;
    }

    h = (struct small_heap *)h_mem->header;
    /* The pointer to begin address of heap */
    h->heap_ptr = (os_uint8_t *)begin_align;
    h->available_total = end_align - begin_align - 2 * SIZEOF_STRUCT_MEM;

    OS_KERN_LOG(KERN_DEBUG, SMALL_TAG, "mem init, heap begin address 0x%x, size %d",
                (os_uint32_t)h->heap_ptr, h_mem->mem_total);

    /* Initialize the start of the heap */
    heap_obj        = (struct small_block *)(begin_align);
    heap_obj->magic = HEAP_MAGIC;
    heap_obj->next  = h->available_total + SIZEOF_STRUCT_MEM;
    heap_obj->prev  = 0;
    heap_obj->used  = 0;
#ifdef OS_USING_MEM_TRACE
    os_mem_setname(heap_obj, "INIT");
#endif

    /* Initialize the end of the heap */
    h->heap_end        = (struct small_block *)&h->heap_ptr[heap_obj->next];
    h->heap_end->magic = HEAP_MAGIC;
    h->heap_end->used  = 1;
    h->heap_end->next  = h->available_total + SIZEOF_STRUCT_MEM;
    h->heap_end->prev  = h->available_total + SIZEOF_STRUCT_MEM;
#ifdef OS_USING_MEM_TRACE
    os_mem_setname(h->heap_end, "INIT");
#endif

    os_sem_init(&h_mem->sem, "mem_s_sem", 1, 1);

    /* Initialize the lowest-free pointer to the start of the heap */
    h->lfree = (struct small_block *)h->heap_ptr;
}

/**
 ***********************************************************************************************************************
 * @brief           This function allocates 'size' bytes and returns a pointer to the allocated memory. The memory is not
 *                  initialized. If size is 0, then it returns NULL.
 *
 * @param[in]       size            The size of the requested block in bytes.
 *
 * @return          The pointer to allocated memory or OS_NULL if no free memory was found.
 ***********************************************************************************************************************
 */
static void *_k_small_mem_alloc(struct heap_mem *h_mem, os_size_t size)
{
    os_size_t ptr, ptr2;
    struct small_block *mem, *mem2;
    struct small_heap  *h;

    OS_ASSERT(OS_FALSE == os_is_irq_active());

    if (0 == size)
    {
        return OS_NULL;
    }

    /* Wait memory semaphore */
    os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);
    
    h = (struct small_heap *)h_mem->header;

    if (size > h->available_total)
    {
        os_sem_post(&h_mem->sem);
        OS_KERN_LOG(KERN_ERROR, SMALL_TAG, "no memory");
        return OS_NULL;
    }

    /* Alignment size */
    size = OS_ALIGN_UP(size, OS_ALIGN_SIZE);

    if (size > h->available_total)
    {
        os_sem_post(&h_mem->sem);
        OS_KERN_LOG(KERN_ERROR, SMALL_TAG, "no memory");
        
        return OS_NULL;
    }

    /* Every data block must be at least MIN_SIZE_ALIGNED long */
    if (size < MIN_SIZE_ALIGNED)
    {
        size = MIN_SIZE_ALIGNED;
    }

    for (ptr = (os_uint8_t *)h->lfree - h->heap_ptr;
         ptr < h->available_total - size;
         ptr = ((struct small_block *)&h->heap_ptr[ptr])->next)
    {
        mem = (struct small_block *)&h->heap_ptr[ptr];

        if ((!mem->used) && (mem->next - (ptr + SIZEOF_STRUCT_MEM)) >= size)
        {
            /* mem is not used and at least perfect fit is possible:
             * mem->next - (ptr + SIZEOF_STRUCT_MEM) gives us the 'user data size' of mem */

            if (mem->next - (ptr + SIZEOF_STRUCT_MEM) >=
                (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED))
            {
                /* (in addition to the above, we test if another struct small_block (SIZEOF_STRUCT_MEM) containing
                 * at least MIN_SIZE_ALIGNED of data also fits in the 'user data space' of 'mem')
                 * -> split large block, create empty remainder,
                 * remainder must be large enough to contain MIN_SIZE_ALIGNED data: if
                 * mem->next - (ptr + (2*SIZEOF_STRUCT_MEM)) == size,
                 * struct small_block would fit in but no data between mem2 and mem2->next
                 * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
                 *       region that couldn't hold data, but when mem->next gets freed,
                 *       the 2 regions would be combined, resulting in more free memory
                 */
                ptr2 = ptr + SIZEOF_STRUCT_MEM + size;

                /* Create mem2 struct */
                mem2       = (struct small_block *)&h->heap_ptr[ptr2];
                mem2->magic = HEAP_MAGIC;
                mem2->used = 0;
                mem2->next = mem->next;
                mem2->prev = ptr;
#ifdef OS_USING_MEM_TRACE
                os_mem_setname(mem2, "    ");
#endif

                /* And insert it between mem and mem->next */
                mem->next = ptr2;
                mem->used = 1;

                if (mem2->next != h->available_total + SIZEOF_STRUCT_MEM)
                {
                    ((struct small_block *)&h->heap_ptr[mem2->next])->prev = ptr2;
                }
                SMALL_MEM_USED_INC(h_mem, size + SIZEOF_STRUCT_MEM);
            }
            else
            {
                /* (A mem2 struct does no fit into the user data space of mem and mem->next will always
                 * be used at this point: if not we have 2 unused structs in a row, plug_holes should have
                 * take care of this).
                 * -> near fit or excact fit: do not split, no mem2 creation
                 * also can't move mem->next directly behind mem, since mem->next
                 * will always be used at this point!
                 */
                mem->used = 1;

                SMALL_MEM_USED_INC(h_mem, mem->next - ((os_uint8_t *)mem - h->heap_ptr));
            }
            /* Set memory block magic */
            mem->magic = HEAP_MAGIC;
#ifdef OS_USING_MEM_TRACE
            if (os_task_self())
            {
                os_mem_setname(mem, os_task_name(os_task_self()));
            }
            else
            {
                os_mem_setname(mem, "NONE");
            }
#endif

            if (mem == h->lfree)
            {
                /* Find next free block after mem and update lowest free pointer */
                while (h->lfree->used && h->lfree != h->heap_end)
                    h->lfree = (struct small_block *)&h->heap_ptr[h->lfree->next];

                OS_ASSERT(((h->lfree == h->heap_end) || (!h->lfree->used)));
            }

            os_sem_post(&h_mem->sem);
            OS_ASSERT((os_uint32_t)mem + SIZEOF_STRUCT_MEM + size <= (os_uint32_t)h->heap_end);
            OS_ASSERT((os_uint32_t)((os_uint8_t *)mem + SIZEOF_STRUCT_MEM) % OS_ALIGN_SIZE == 0);
            OS_ASSERT((((os_uint32_t)mem) & (OS_ALIGN_SIZE - 1)) == 0);

            OS_KERN_LOG(KERN_DEBUG, SMALL_TAG, "allocate memory at 0x%x, size: %d",
                        (os_uint32_t)((os_uint8_t *)mem + SIZEOF_STRUCT_MEM),
                        (os_uint32_t)(mem->next - ((os_uint8_t *)mem - h->heap_ptr)));

            /* Return the memory data except mem struct */
            return (os_uint8_t *)mem + SIZEOF_STRUCT_MEM;
        }
    }

    os_sem_post(&h_mem->sem);

    return OS_NULL;
}


OS_UNUSED static void *_k_small_mem_aligned_alloc(struct heap_mem *h_mem, os_size_t align, os_size_t bytes)
{
    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           This function changes the size of the memory block pointed to by 'rmem' to 'newsize' bytes. If 'newsize'
 *                  is equal to 0, it works in the same way as os_free(). If 'rmem' is OS_NULL, it works in the same way
 *                  as os_malloc().
 *
 * @param[in]       rmem            The pointer of memory block to change.
 * @param[in]       newsize         The size of new memory block.
 *
 * @return          The pointer to newly allocated memory or NULL.
 ***********************************************************************************************************************
 */
OS_UNUSED static void *_k_small_mem_realloc(struct heap_mem *h_mem, void *rmem, os_size_t newsize)
{
    os_size_t size;
    os_size_t ptr, ptr2;
    struct small_block *mem, *mem2;
    void *nmem;
    struct small_heap  *h;

    OS_ASSERT(OS_FALSE == os_is_irq_active());

    if (OS_NULL == rmem)
    {
        return _k_small_mem_alloc(h_mem, newsize);
    }

    if (0 == newsize)
    {
        _k_small_mem_free(h_mem, rmem);

        return OS_NULL;
    }

    os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);
    h = (struct small_heap *)h_mem->header;
    if (newsize > h->available_total)
    {
        os_sem_post(&h_mem->sem);
        OS_KERN_LOG(KERN_ERROR, SMALL_TAG, "realloc: out of memory");
        return OS_NULL;
    }

    /* Alignment size */
    newsize = OS_ALIGN_UP(newsize, OS_ALIGN_SIZE);
    if (newsize > h->available_total)
    {
        os_sem_post(&h_mem->sem);
        OS_KERN_LOG(KERN_DEBUG, SMALL_TAG, "realloc: out of memory");

        return OS_NULL;
    }

    if ((os_uint8_t *)rmem < (os_uint8_t *)h->heap_ptr ||
        (os_uint8_t *)rmem >= (os_uint8_t *)h->heap_end)
    {
        /* Illegal memory */
        os_sem_post(&h_mem->sem);

        return rmem;
    }

    mem = (struct small_block *)((os_uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    ptr = (os_uint8_t *)mem - h->heap_ptr;
    size = mem->next - ptr - SIZEOF_STRUCT_MEM;
    if (size == newsize)
    {
        /* The size is the same as */
        os_sem_post(&h_mem->sem);

        return rmem;
    }

    if (newsize + SIZEOF_STRUCT_MEM + MIN_SIZE < size)
    {
        /* Split memory block */
        SMALL_MEM_USED_DEC(h_mem, size - newsize);

        ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
        mem2 = (struct small_block *)&h->heap_ptr[ptr2];
        mem2->magic = HEAP_MAGIC;
        mem2->used = 0;
        mem2->next = mem->next;
        mem2->prev = ptr;
#ifdef OS_USING_MEM_TRACE
        os_mem_setname(mem2, "    ");
#endif
        mem->next = ptr2;
        if (mem2->next != h->available_total + SIZEOF_STRUCT_MEM)
        {
            ((struct small_block *)&h->heap_ptr[mem2->next])->prev = ptr2;
        }

        plug_holes(h_mem, mem2);

        os_sem_post(&h_mem->sem);

        return rmem;
    }
    os_sem_post(&h_mem->sem);

    /* Expand memory */
    nmem = _k_small_mem_alloc(h_mem, newsize);
    if (OS_NULL != nmem) /* check memory */
    {
        memcpy(nmem, rmem, size < newsize ? size : newsize);
        _k_small_mem_free(h_mem, rmem);
    }

    return nmem;
}


/**
 ***********************************************************************************************************************
 * @brief           This function frees the memory space pointed to by 'rmem', which must have been returned by a previous
 *                  call to os_malloc(), os_realloc or os_calloc().
 *
 * @param[in]       rmem           The pointer to memory space.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void _k_small_mem_free(struct heap_mem *h_mem, void *rmem)
{
    struct small_block *mem;
    struct small_heap  *h;

    if (OS_NULL == rmem)
    {
        return;
    }

    /* Protect the heap from concurrent access */
    os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);
    h = (struct small_heap *)h_mem->header;

    OS_ASSERT(OS_FALSE == os_is_irq_active());

    OS_ASSERT((((os_uint32_t)rmem) & (OS_ALIGN_SIZE - 1)) == 0);
    OS_ASSERT((os_uint8_t *)rmem >= (os_uint8_t *)h->heap_ptr &&
              (os_uint8_t *)rmem < (os_uint8_t *)h->heap_end);

    if ((os_uint8_t *)rmem < (os_uint8_t *)h->heap_ptr ||
        (os_uint8_t *)rmem >= (os_uint8_t *)h->heap_end)
    {
        os_sem_post(&h_mem->sem);
        OS_KERN_LOG(KERN_DEBUG, SMALL_TAG, "illegal memory");

        return;
    }

    /* Get the corresponding struct small_block ... */
    mem = (struct small_block *)((os_uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    OS_KERN_LOG(KERN_DEBUG, SMALL_TAG, "release memory 0x%x, size: %d",
                (os_uint32_t)rmem,
                (os_uint32_t)(mem->next - ((os_uint8_t *)mem - h->heap_ptr)));

    /* Which has to be in a used state */
    if (!mem->used || HEAP_MAGIC != mem->magic)
    {
        OS_KERN_LOG(KERN_WARNING, SMALL_TAG, "to free a bad data block:");
        OS_KERN_LOG(KERN_WARNING, SMALL_TAG, "mem: 0x%08x, used flag: %d, magic code: 0x%04x", mem, mem->used, mem->magic);
    }
    OS_ASSERT(mem->used);
    OS_ASSERT(HEAP_MAGIC == mem->magic);
    /* And is now unused. */
    mem->used  = 0;
    mem->magic = HEAP_MAGIC;
#ifdef OS_USING_MEM_TRACE
    os_mem_setname(mem, "    ");
#endif

    if (mem < h->lfree)
    {
        /* The newly freed struct is now the lowest */
        h->lfree = mem;
    }

    SMALL_MEM_USED_DEC(h_mem, mem->next - ((os_uint8_t *)mem - h->heap_ptr));

    /* Finally, see if prev or next are free also */
    plug_holes(h_mem, mem);
    os_sem_post(&h_mem->sem);
}

OS_UNUSED static os_size_t _k_small_mem_ptr_to_size(struct heap_mem *h_mem, void *mem)
{
    struct small_block *block;

    block = (struct small_block *)mem - 1;
    return block->next - (os_size_t)mem;
}

OS_UNUSED static void _k_small_mem_deinit(struct heap_mem *h_mem)
{
    os_sem_deinit(&h_mem->sem);
}

OS_UNUSED static os_err_t _k_small_mem_check(struct heap_mem *h_mem)
{
    int position;
    struct small_block *mem;
    struct small_heap  *h;
    
    os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);
    
    h = (struct small_heap *)h_mem->header;
    for (mem = (struct small_block *)h->heap_ptr; mem != h->heap_end; mem = (struct small_block *)&h->heap_ptr[mem->next])
    {
        position = (os_uint32_t)mem - (os_uint32_t)h->heap_ptr;
        if (position < 0)
        {
            goto __exit;
        }
        if (position > h->available_total)
        {
            goto __exit;
        }
        if (HEAP_MAGIC != mem->magic)
        {
            goto __exit;
        }
        if (mem->used != 0 && 1 != mem->used)
        {
            goto __exit;
        }
    }
    
    os_sem_post(&h_mem->sem);

    return OS_EOK;
__exit:
    os_kprintf("Memory block wrong:\r\n");
    os_kprintf("address: 0x%08x\r\n", mem);
    os_kprintf("  magic: 0x%04x\r\n", mem->magic);
    os_kprintf("   used: %d\r\n", mem->used);
    os_kprintf("  size: %d\r\n", mem->next - position - SIZEOF_STRUCT_MEM);
    os_sem_post(&h_mem->sem);

    return OS_EOK;
}

#ifdef OS_USING_MEM_TRACE

static os_err_t _k_small_mem_trace(struct heap_mem *h_mem)
{
    struct small_block *mem;
    struct small_heap  *h;

    os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);
    
    h = (struct small_heap *)h_mem->header;

    os_kprintf("\r\nmemory heap address:\r\n");
    os_kprintf("h->heap_ptr: 0x%08x\r\n", h->heap_ptr);
    os_kprintf("h->lfree   : 0x%08x\r\n", h->lfree);
    os_kprintf("h->heap_end: 0x%08x\r\n", h->heap_end);

    os_kprintf("\r\n--memory item information --\r\n");
    os_kprintf("\r\n  address     size   task\r\n");
    
    for (mem = (struct small_block *)h->heap_ptr; mem != h->heap_end; mem = (struct small_block *)&h->heap_ptr[mem->next])
    {
        int position = (os_uint32_t)mem - (os_uint32_t)h->heap_ptr;
        int size;

        os_kprintf("[0x%08x - ", mem);

        size = mem->next - position - SIZEOF_STRUCT_MEM;
        if (size < 1024)
        {
            os_kprintf("%5d", size);
        }
        else if (size < 1024 * 1024)
        {
            os_kprintf("%4dK", size / 1024);
        }
        else
        {
            os_kprintf("%4dM", size / (1024 * 1024));
        }

        os_kprintf("] %c%c%c%c", mem->task[0], mem->task[1], mem->task[2], mem->task[3]);

        if (HEAP_MAGIC != mem->magic)
        {
            os_kprintf(": ***\r\n");
        }
        else
        {
            os_kprintf("\r\n");
        }
    }

    os_sem_post(&h_mem->sem);
    
    return OS_EOK;
}

#endif /* end of OS_USING_MEM_TRACE */

void k_small_mem_init(struct heap_mem *h_mem, void *start_addr, os_size_t size)
{
    _k_small_mem_init(h_mem, start_addr, size);

    h_mem->k_alloc         = _k_small_mem_alloc;
    h_mem->k_free          = _k_small_mem_free;
#ifndef OS_MEM_ALGORITHM_MINIMIZATION
    h_mem->k_aligned_alloc = _k_small_mem_aligned_alloc;
    h_mem->k_realloc       = _k_small_mem_realloc;
    h_mem->k_ptr_to_size   = _k_small_mem_ptr_to_size;
    h_mem->k_deinit        = _k_small_mem_deinit;
    h_mem->k_mem_check     = _k_small_mem_check;
#else
    h_mem->k_aligned_alloc = OS_NULL;
    h_mem->k_realloc       = OS_NULL;
    h_mem->k_ptr_to_size   = OS_NULL;
    h_mem->k_deinit        = OS_NULL;
    h_mem->k_mem_check     = OS_NULL;
#endif

#ifdef OS_USING_MEM_TRACE
    h_mem->k_mem_trace     = _k_small_mem_trace;
#endif
}

#endif /* end of OS_USING_HEAP */

