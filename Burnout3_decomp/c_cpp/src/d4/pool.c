/* d4/pool (0x2B6C40-0x2B6FF0): fixed-size pool with a free list, and owner lists that take elements from it. */

#include "pool.h"

#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

void poolListFreeAll(PoolList *list)
{
    PoolNode *head = list->used;
    PoolNode *n;

    if (head != NULL) {
        for (n = head; n->next != NULL; n = n->next) {
        }
        n->next = list->pool->free;
        if (n->next != NULL) {
            n->next->prev = n;
        }
        list->pool->free = head;
        list->used = NULL;
    }
}

void poolListFree(PoolList *list, PoolNode *node)
{
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
        node->prev = NULL;
    } else if (node->next != NULL) {
        list->used = node->next;
    } else {
        list->used = NULL;
    }
    node->next = list->pool->free;
    if (node->next != NULL) {
        node->next->prev = node;
    }
    list->pool->free = node;
}

void poolListMove(PoolList *dst, PoolNode *node, PoolList *src)
{
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else if (node->next != NULL) {
        src->used = node->next;
    } else {
        src->used = NULL;
    }
    if (dst->used != NULL) {
        node->next = dst->used;
        node->next->prev = node;
    } else {
        node->next = NULL;
    }
    node->prev = NULL;
    dst->used = node;
}

PoolNode *poolListAlloc(PoolList *list)
{
    PoolNode **free = &list->pool->free;
    PoolNode *n = *free;

    if (n == NULL) {
        return NULL;
    }
    *free = n->next;
    if (n->next != NULL) {
        n->next->prev = NULL;
    }
    if (list->used != NULL) {
        n->next = list->used;
        n->next->prev = n;
    } else {
        n->next = NULL;
    }
    list->used = n;
    return n;
}

void poolListInit(PoolList *list, Pool *pool)
{
    list->pool = pool;
    list->used = NULL;
}

#define MARK ((PoolNode *)1)

void poolSortFree(Pool *pool)
{
    PoolNode *n;
    PoolNode *prev;
    u32 left;

    if (pool->base != NULL && pool->free != NULL) {
        left = 0;
        n = pool->free;
        do {
            n->prev = MARK;
            left++;
            n = n->next;
        } while (n != NULL);

        n = pool->base;
        prev = NULL;
        for (;;) {
            if (n->prev == MARK) {
                if (prev != NULL) {
                    prev->next = n;
                    n->prev = prev;
                } else {
                    pool->free = n;
                    n->prev = NULL;
                }
                prev = n;
                if (--left == 0) {
                    n->next = NULL;
                    return;
                }
            }
            n = (PoolNode *)((u8 *)n + pool->stride);
        }
    }
}

u32 poolBufferSize(u32 count, u32 size, u32 align)
{
    u32 stride = size + 8;

    if (align != 0) {
        stride = (stride + align - 1) & ~(align - 1);
    }
    return align + stride * count;
}

void poolInit(Pool *pool, u32 count, u32 size, u32 align, const void *init, void *buffer)
{
    PoolNode *n;
    PoolNode *prev;
    u32 stride;
    u32 i;

    if (align != 0) {
        pool->align = align;
    } else {
        pool->align = 1;
    }
    if (count == 0) {
        pool->base = NULL;
        pool->stride = size;
        pool->count = 0;
        pool->free = NULL;
        return;
    }

    pool->free = pool->base = (PoolNode *)(ALIGN((u32)buffer + sizeof(PoolNode), pool->align) - sizeof(PoolNode));
    n = pool->free;
    stride = size + sizeof(PoolNode);
    stride = ALIGN(stride, pool->align);
    n->prev = NULL;
    for (i = 1; i < count; i++) {
        prev = n;
        n = (PoolNode *)((u8 *)n + stride);
        prev->next = n;
        n->prev = prev;
    }
    n->next = NULL;

    if (init != NULL) {
        n = pool->free;
        do {
            u32 len;
            const u8 *s;
            u8 *d;
            d = (u8 *)(n + 1);
            s = init;
            len = size;
            while (len != 0) {
                *d = *s;
                len--;
                s++;
                d++;
            }
            n = n->next;
        } while (n != NULL);
    }
    pool->stride = stride;
    pool->count = count;
}
