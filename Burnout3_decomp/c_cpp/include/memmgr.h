#ifndef MEMMGR_H
#define MEMMGR_H

#include "types.h"
#include "heap.h"

/* Memory manager (game/unit_00222300, 0x222300-0x222C90): the heap core's object seen through its vtable
 * (0x4DE030), one global instance at 0x1D6D880. Its methods take a category: 4 is a fixed 0x80000 block (the
 * current layout's category-5 slot), 0x17-0x1A are the four extra slots at +0x98C, the rest go to the heap core.
 * Names are ours. */

#ifdef __cplusplus
extern "C" {
#endif

u32 memMgrSize(Heap *self, int cat);              /* vtable +0x20: size of a category's block */
void *memMgrPeekCat13(Heap *self, int cat);            /* vtable +0x1C: base of category 13's slot, not taken */
void memMgrRelease(Heap *self, int cat, int index);  /* vtable +0x18: release */
void *memMgrTake(Heap *self, int cat, int index); /* vtable +0x14: take */
void memMgrUpdate(Heap *self);                      /* vtable +0x10: once per frame, age released slots */
void memMgrFreeBlock(Heap *self);                      /* vtable +0x0C: free the malloc'd block */
void memMgrInit(Heap *self);                      /* vtable +0x08: init, carve the arena into slots */

#ifdef __cplusplus
}
#endif

#endif
