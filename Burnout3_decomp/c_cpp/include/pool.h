#ifndef POOL_H
#define POOL_H

#include "types.h"

/* Fixed-size pool with a free list (d4/pool, 0x2B6C40). Every element sits behind an 8-byte header that links it
 * into either the pool's free list or one owner list's used list. Names are ours. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PoolNode {
    struct PoolNode *next; /* 0x0 */
    struct PoolNode *prev; /* 0x4 */
} PoolNode;                /* the element's data follows */

typedef struct Pool {
    PoolNode *base;  /* 0x00 first element's header (0 for an empty pool) */
    u32 stride;      /* 0x04 header + element, rounded up to align */
    u32 count;       /* 0x08 */
    u32 align;       /* 0x0C */
    PoolNode *free;  /* 0x10 free list */
} Pool;

typedef struct PoolList {
    Pool *pool;      /* 0x0 */
    PoolNode *used;  /* 0x4 elements taken by this list */
} PoolList;

void poolListFreeAll(PoolList *list);                            /* return every used element to the pool */
void poolListFree(PoolList *list, PoolNode *node);            /* return one element to the pool */
void poolListMove(PoolList *dst, PoolNode *node, PoolList *src); /* move an element between lists */
PoolNode *poolListAlloc(PoolList *list);                       /* take an element (0 if none) */
void poolListInit(PoolList *list, Pool *pool);
void poolSortFree(Pool *pool);                                /* sort the free list into address order */
u32 poolBufferSize(u32 count, u32 size, u32 align);             /* buffer size poolInit needs */
void poolInit(Pool *pool, u32 count, u32 size, u32 align, const void *init, void *buffer);

#ifdef __cplusplus
}
#endif

#endif
