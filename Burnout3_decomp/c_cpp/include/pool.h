#ifndef POOL_H
#define POOL_H

#include "types.h"

/* Criterion's linked-list pool (d4/pool, 0x2B6C40): a pool of fixed-size links with a free list, and lists that take
 * links from it. Every element sits behind an 8-byte link header. Function, type and field names are Criterion's,
 * from Burnout 2's symbols and debug info (docs/burnout2.md); Burnout 3's pool layout is smaller than Burnout 2's
 * (0x14 bytes, not 0x1C), so its fields and parameter types are ours where the two differ. C++ functions (mangled). */

struct GtLLLinkTag {
    GtLLLinkTag *lpNext; /* 0x0 */
    GtLLLinkTag *lpPrev; /* 0x4 */
};                       /* the element's data follows */

struct GtLListPoolTag {
    GtLLLinkTag *lpLinks;     /* 0x00 first link (0 for an empty pool) */
    u32 nLinkSize;            /* 0x04 header + element, rounded up to the alignment */
    u32 nNumLinks;            /* 0x08 */
    u32 nAlignment;           /* 0x0C */
    GtLLLinkTag *lpFirstFree; /* 0x10 free list */
};

struct GtLListTag {
    GtLListPoolTag *lppPool;  /* 0x0 */
    GtLLLinkTag *lpFirstLink; /* 0x4 links taken by this list */
};

void GtLListRemoveAllLinks(GtLListTag *list);                                      /* return every link to the pool */
void GtLListRemoveLink(GtLListTag *list, GtLLLinkTag *link);                       /* return one link to the pool */
void GtLListTakeLinkFromList(GtLListTag *dst, GtLLLinkTag *link, GtLListTag *src); /* move a link between lists */
GtLLLinkTag *GtLListAddLink(GtLListTag *list);                                     /* take a link (0 if none) */
void GtLListInitialise(GtLListTag *list, GtLListPoolTag *pool);
void GtLListPoolSortFree(GtLListPoolTag *pool);                       /* sort the free list into address order */
u32 GtLListPoolCalculateSize(u32 count, u32 size, u32 align);         /* buffer size CreateInMemory needs */
void GtLListPoolCreateInMemory(GtLListPoolTag *pool, u32 count, u32 size, u32 align, const void *init, void *buffer);

#endif
