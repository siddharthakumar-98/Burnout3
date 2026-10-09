/* d4/pool (0x2B6C40-0x2B6FF0): GtLList, a pool of fixed-size links with a free list, and lists that take links from it. */

#include "pool.h"

#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

void GtLListRemoveAllLinks(GtLListTag *list)
{
    GtLLLinkTag *head = list->lpFirstLink;
    GtLLLinkTag *n;

    if (head != NULL) {
        for (n = head; n->lpNext != NULL; n = n->lpNext) {
        }
        n->lpNext = list->lppPool->lpFirstFree;
        if (n->lpNext != NULL) {
            n->lpNext->lpPrev = n;
        }
        list->lppPool->lpFirstFree = head;
        list->lpFirstLink = NULL;
    }
}

void GtLListRemoveLink(GtLListTag *list, GtLLLinkTag *node)
{
    if (node->lpNext != NULL) {
        node->lpNext->lpPrev = node->lpPrev;
    }
    if (node->lpPrev != NULL) {
        node->lpPrev->lpNext = node->lpNext;
        node->lpPrev = NULL;
    } else if (node->lpNext != NULL) {
        list->lpFirstLink = node->lpNext;
    } else {
        list->lpFirstLink = NULL;
    }
    node->lpNext = list->lppPool->lpFirstFree;
    if (node->lpNext != NULL) {
        node->lpNext->lpPrev = node;
    }
    list->lppPool->lpFirstFree = node;
}

void GtLListTakeLinkFromList(GtLListTag *dst, GtLLLinkTag *node, GtLListTag *src)
{
    if (node->lpNext != NULL) {
        node->lpNext->lpPrev = node->lpPrev;
    }
    if (node->lpPrev != NULL) {
        node->lpPrev->lpNext = node->lpNext;
    } else if (node->lpNext != NULL) {
        src->lpFirstLink = node->lpNext;
    } else {
        src->lpFirstLink = NULL;
    }
    if (dst->lpFirstLink != NULL) {
        node->lpNext = dst->lpFirstLink;
        node->lpNext->lpPrev = node;
    } else {
        node->lpNext = NULL;
    }
    node->lpPrev = NULL;
    dst->lpFirstLink = node;
}

GtLLLinkTag *GtLListAddLink(GtLListTag *list)
{
    GtLLLinkTag **free = &list->lppPool->lpFirstFree;
    GtLLLinkTag *n = *free;

    if (n == NULL) {
        return NULL;
    }
    *free = n->lpNext;
    if (n->lpNext != NULL) {
        n->lpNext->lpPrev = NULL;
    }
    if (list->lpFirstLink != NULL) {
        n->lpNext = list->lpFirstLink;
        n->lpNext->lpPrev = n;
    } else {
        n->lpNext = NULL;
    }
    list->lpFirstLink = n;
    return n;
}

void GtLListInitialise(GtLListTag *list, GtLListPoolTag *pool)
{
    list->lppPool = pool;
    list->lpFirstLink = NULL;
}

#define MARK ((GtLLLinkTag *)1)

void GtLListPoolSortFree(GtLListPoolTag *pool)
{
    GtLLLinkTag *n;
    GtLLLinkTag *prev;
    u32 left;

    if (pool->lpLinks != NULL && pool->lpFirstFree != NULL) {
        left = 0;
        n = pool->lpFirstFree;
        do {
            n->lpPrev = MARK;
            left++;
            n = n->lpNext;
        } while (n != NULL);

        n = pool->lpLinks;
        prev = NULL;
        for (;;) {
            if (n->lpPrev == MARK) {
                if (prev != NULL) {
                    prev->lpNext = n;
                    n->lpPrev = prev;
                } else {
                    pool->lpFirstFree = n;
                    n->lpPrev = NULL;
                }
                prev = n;
                if (--left == 0) {
                    n->lpNext = NULL;
                    return;
                }
            }
            n = (GtLLLinkTag *)((u8 *)n + pool->nLinkSize);
        }
    }
}

u32 GtLListPoolCalculateSize(u32 count, u32 size, u32 align)
{
    u32 stride = size + 8;

    if (align != 0) {
        stride = (stride + align - 1) & ~(align - 1);
    }
    return align + stride * count;
}

void GtLListPoolCreateInMemory(GtLListPoolTag *pool, u32 count, u32 size, u32 align, const void *init, void *buffer)
{
    GtLLLinkTag *n;
    GtLLLinkTag *prev;
    u32 stride;
    u32 i;

    if (align != 0) {
        pool->nAlignment = align;
    } else {
        pool->nAlignment = 1;
    }
    if (count == 0) {
        pool->lpLinks = NULL;
        pool->nLinkSize = size;
        pool->nNumLinks = 0;
        pool->lpFirstFree = NULL;
        return;
    }

    pool->lpFirstFree = pool->lpLinks = (GtLLLinkTag *)(ALIGN((u32)buffer + sizeof(GtLLLinkTag), pool->nAlignment) - sizeof(GtLLLinkTag));
    n = pool->lpFirstFree;
    stride = size + sizeof(GtLLLinkTag);
    stride = ALIGN(stride, pool->nAlignment);
    n->lpPrev = NULL;
    for (i = 1; i < count; i++) {
        prev = n;
        n = (GtLLLinkTag *)((u8 *)n + stride);
        prev->lpNext = n;
        n->lpPrev = prev;
    }
    n->lpNext = NULL;

    if (init != NULL) {
        n = pool->lpFirstFree;
        do {
            u32 len;
            const u8 *s;
            u8 *d;
            d = (u8 *)(n + 1);
            s = (const u8 *)init;
            len = size;
            while (len != 0) {
                *d = *s;
                len--;
                s++;
                d++;
            }
            n = n->lpNext;
        } while (n != NULL);
    }
    pool->nLinkSize = stride;
    pool->nNumLinks = count;
}
