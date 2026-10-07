/* d4/heap (0x3E7850-0x3E8DD0): the memory manager's sub-heap table: lookup, take and release by category, and
 * the per-frame aging of released slots. One source file; the asm units game/unit_003E7850, unit_003E7AD0 and
 * unit_003E7D40 are cut at its three jump tables. */

#include "heap.h"

static inline u32 slotSize(HeapSlot *slot)
{
    return slot->size;
}

static inline void slotAge(HeapSlot *slot)
{
    if (slot->state > 0) {
        slot->state--;
    }
}

static inline void slotRelease(HeapSlot *slot)
{
    slot->state = 2;
}

static inline void *slotTake(HeapSlot *slot, int layout)
{
    if (slot->layout < 0 || slot->layout == layout) {
        if (slot->state != 0) {
            return NULL;
        }
        slot->state = -1;
        return slot->base;
    }
    return NULL;
}

static inline void *slotPeek(HeapSlot *slot, int layout)
{
    if (slot->layout < 0 || slot->layout == layout) {
        return slot->base;
    }
    return NULL;
}

/* game/unit_003E7850 */

u32 func_003E7850(Heap *heap, int cat)
{
    switch (cat) {
    case 0:
        return slotSize(&heap->cat0);
    case 1:
        if (heap->layout == 0) {
            return slotSize(&heap->cat1_l0[0]);
        } else if (heap->layout == 2) {
            return slotSize(&heap->l2.cat1[0]);
        } else {
            return slotSize(&heap->l1.cat1[0]);
        }
    case 2:
        if (heap->layout == 2) {
            return slotSize(&heap->l2.cat2[0]);
        } else {
            return slotSize(&heap->l1.cat2[0]);
        }
    case 3:
        if (heap->layout == 0) {
            return slotSize(&heap->cat3_l0);
        } else if (heap->layout == 2) {
            return slotSize(&heap->l2.cat3);
        } else {
            return slotSize(&heap->l1.cat3);
        }
    case 5:
        if (heap->layout == 0) {
            return slotSize(&heap->cat5_l0);
        } else if (heap->layout == 2) {
            return slotSize(&heap->l2.cat5);
        } else {
            return slotSize(&heap->l1.cat5);
        }
    case 6:
        if (heap->layout == 2) {
            return slotSize(&heap->l2.cat6[0]);
        } else {
            return slotSize(&heap->l1.cat6[0]);
        }
    case 7:
        if (heap->layout == 2) {
            return slotSize(&heap->l2.cat7[0]);
        } else {
            return slotSize(&heap->l1.cat7[0]);
        }
    case 8:
        return slotSize(&heap->cat8);
    case 9:
        if (heap->layout == 2) {
            return slotSize(&heap->l2.cat9);
        } else {
            return slotSize(&heap->l1.cat9);
        }
    case 10:
        return slotSize(&heap->cat10);
    case 11:
        return slotSize(&heap->cat11);
    case 13:
        if (heap->layout == 2) {
            return slotSize(&heap->l2.cat13);
        } else {
            return slotSize(&heap->l1.cat13);
        }
    case 12:
        return slotSize(&heap->cat12);
    case 14:
        return slotSize(&heap->cat14);
    case 15:
        return slotSize(&heap->cat15);
    case 16:
        return slotSize(&heap->cat16);
    case 17:
        return slotSize(&heap->cat17);
    case 20:
        return slotSize(&heap->cat20);
    case 18:
        return slotSize(&heap->cat18);
    case 19:
        return slotSize(&heap->cat19);
    case 21:
        return slotSize(&heap->cat21[0]);
    case 22:
        return slotSize(&heap->cat22);
    }
    return -1;
}

void *func_003E7A50(Heap *heap, int cat)
{
    switch (cat) {
    case 13:
        if (heap->layout == 2) {
            return slotPeek(&heap->l2.cat13, heap->layout);
        } else {
            return slotPeek(&heap->l1.cat13, heap->layout);
        }
    }
    return NULL;
}

/* game/unit_003E7AD0 */

void func_003E7AD0(Heap *heap, int cat, int index)
{
    switch (cat) {
    case 0:
        slotRelease(&heap->cat0);
        break;
    case 1:
        if (heap->layout == 0) {
            slotRelease(&heap->cat1_l0[index]);
        } else if (heap->layout == 2) {
            slotRelease(&heap->l2.cat1[index]);
        } else {
            slotRelease(&heap->l1.cat1[index]);
        }
        break;
    case 2:
        if (heap->layout == 2) {
            slotRelease(&heap->l2.cat2[index]);
        } else {
            slotRelease(&heap->l1.cat2[index]);
        }
        break;
    case 3:
        if (heap->layout == 0) {
            slotRelease(&heap->cat3_l0);
        } else if (heap->layout == 2) {
            slotRelease(&heap->l2.cat3);
        } else {
            slotRelease(&heap->l1.cat3);
        }
        break;
    case 5:
        if (heap->layout == 0) {
            slotRelease(&heap->cat5_l0);
        } else if (heap->layout == 2) {
            slotRelease(&heap->l2.cat5);
        } else {
            slotRelease(&heap->l1.cat5);
        }
        break;
    case 6:
        if (heap->layout == 2) {
            slotRelease(&heap->l2.cat6[index]);
        } else {
            slotRelease(&heap->l1.cat6[index]);
        }
        break;
    case 7:
        if (heap->layout == 2) {
            slotRelease(&heap->l2.cat7[index]);
        } else {
            slotRelease(&heap->l1.cat7[index]);
        }
        break;
    case 8:
        slotRelease(&heap->cat8);
        break;
    case 9:
        if (heap->layout == 2) {
            slotRelease(&heap->l2.cat9);
        } else {
            slotRelease(&heap->l1.cat9);
        }
        break;
    case 10:
        slotRelease(&heap->cat10);
        break;
    case 11:
        slotRelease(&heap->cat11);
        break;
    case 13:
        if (heap->layout == 2) {
            slotRelease(&heap->l2.cat13);
        } else {
            slotRelease(&heap->l1.cat13);
        }
        break;
    case 12:
        slotRelease(&heap->cat12);
        break;
    case 14:
        slotRelease(&heap->cat14);
        break;
    case 15:
        slotRelease(&heap->cat15);
        break;
    case 16:
        slotRelease(&heap->cat16);
        break;
    case 17:
        slotRelease(&heap->cat17);
        break;
    case 20:
        slotRelease(&heap->cat20);
        break;
    case 18:
        slotRelease(&heap->cat18);
        break;
    case 19:
        slotRelease(&heap->cat19);
        break;
    case 21:
        slotRelease(&heap->cat21[index]);
        break;
    case 22:
        slotRelease(&heap->cat22);
        break;
    }
}

/* game/unit_003E7D40 */

void *func_003E7D40(Heap *heap, int cat, int index)
{
    switch (cat) {
    case 0:
        return slotTake(&heap->cat0, heap->layout);
    case 1:
        if (heap->layout == 0) {
            return slotTake(&heap->cat1_l0[index], heap->layout);
        } else if (heap->layout == 2) {
            return slotTake(&heap->l2.cat1[index], heap->layout);
        } else {
            return slotTake(&heap->l1.cat1[index], heap->layout);
        }
    case 2:
        if (heap->layout == 2) {
            return slotTake(&heap->l2.cat2[index], heap->layout);
        } else {
            return slotTake(&heap->l1.cat2[index], heap->layout);
        }
    case 3:
        if (heap->layout == 0) {
            return slotTake(&heap->cat3_l0, heap->layout);
        } else if (heap->layout == 2) {
            return slotTake(&heap->l2.cat3, heap->layout);
        } else {
            return slotTake(&heap->l1.cat3, heap->layout);
        }
    case 5:
        if (heap->layout == 0) {
            return slotTake(&heap->cat5_l0, heap->layout);
        } else if (heap->layout == 2) {
            return slotTake(&heap->l2.cat5, heap->layout);
        } else {
            return slotTake(&heap->l1.cat5, heap->layout);
        }
    case 6:
        if (heap->layout == 2) {
            return slotTake(&heap->l2.cat6[index], heap->layout);
        } else {
            return slotTake(&heap->l1.cat6[index], heap->layout);
        }
    case 7:
        if (heap->layout == 2) {
            return slotTake(&heap->l2.cat7[index], heap->layout);
        } else {
            return slotTake(&heap->l1.cat7[index], heap->layout);
        }
    case 8:
        return slotTake(&heap->cat8, heap->layout);
    case 9:
        if (heap->layout == 2) {
            return slotTake(&heap->l2.cat9, heap->layout);
        } else {
            return slotTake(&heap->l1.cat9, heap->layout);
        }
    case 10:
        return slotTake(&heap->cat10, heap->layout);
    case 11:
        return slotTake(&heap->cat11, heap->layout);
    case 13:
        if (heap->layout == 2) {
            return slotTake(&heap->l2.cat13, heap->layout);
        } else {
            return slotTake(&heap->l1.cat13, heap->layout);
        }
    case 12:
        return slotTake(&heap->cat12, heap->layout);
    case 14:
        return slotTake(&heap->cat14, heap->layout);
    case 15:
        return slotTake(&heap->cat15, heap->layout);
    case 16:
        return slotTake(&heap->cat16, heap->layout);
    case 17:
        return slotTake(&heap->cat17, heap->layout);
    case 18:
        return slotTake(&heap->cat18, heap->layout);
    case 19:
        return slotTake(&heap->cat19, heap->layout);
    case 20:
        return slotTake(&heap->cat20, heap->layout);
    case 21:
        return slotTake(&heap->cat21[index], heap->layout);
    case 22:
        return slotTake(&heap->cat22, heap->layout);
    }
    return NULL;
}

void func_003E8750(Heap *heap)
{
    heap->layoutRelease = 2;
}

int func_003E8760(Heap *heap, int layout)
{
    if (heap->layoutRelease > 0) {
        return 0;
    }
    if (heap->layout == layout) {
        return 1;
    }
    heap->layout = layout;
    return 1;
}

void func_003E87A0(Heap *heap)
{
    int i;

    if (heap->layoutRelease > 0) {
        heap->layoutRelease--;
        if (heap->layoutRelease == 0) {
            heap->layout = -1;
        }
    }
    slotAge(&heap->cat0);
    slotAge(&heap->cat8);
    for (i = 0; i < 8; i++) {
        slotAge(&heap->cat21[i]);
    }
    slotAge(&heap->cat22);
    slotAge(&heap->cat10);
    slotAge(&heap->cat11);
    slotAge(&heap->cat12);
    slotAge(&heap->l1.cat9);
    slotAge(&heap->l1.cat5);
    slotAge(&heap->l1.cat3);
    slotAge(&heap->l1.cat13);
    for (i = 0; i < 10; i++) {
        slotAge(&heap->l1.cat6[i]);
    }
    for (i = 0; i < 18; i++) {
        slotAge(&heap->l1.cat7[i]);
    }
    slotAge(&heap->l2.cat9);
    slotAge(&heap->l2.cat5);
    slotAge(&heap->l2.cat3);
    slotAge(&heap->l2.cat13);
    for (i = 0; i < 19; i++) {
        slotAge(&heap->l2.cat6[i]);
    }
    for (i = 0; i < 35; i++) {
        slotAge(&heap->l2.cat7[i]);
    }
    slotAge(&heap->cat14);
    slotAge(&heap->cat15);
    for (i = 0; i < 12; i++) {
        slotAge(&heap->l1.cat2[i]);
        slotAge(&heap->l2.cat2[i]);
    }
    for (i = 0; i < 6; i++) {
        slotAge(&heap->l1.cat1[i]);
        slotAge(&heap->l2.cat1[i]);
    }
    slotAge(&heap->cat1_l0[0]);
    slotAge(&heap->cat1_l0[1]);
    slotAge(&heap->slot924);
    slotAge(&heap->cat3_l0);
    slotAge(&heap->cat5_l0);
    slotAge(&heap->cat16);
    slotAge(&heap->cat17);
    slotAge(&heap->cat18);
    slotAge(&heap->cat19);
    slotAge(&heap->cat20);
}

/* Out-of-line slot operations, for the memory manager (unit_00222300). */

u32 func_003E8B70(HeapSlot *slot)
{
    return slotSize(slot);
}

void func_003E8B80(HeapSlot *slot)
{
    slotAge(slot);
}

void func_003E8BA0(HeapSlot *slot)
{
    slotRelease(slot);
}

void *func_003E8BB0(HeapSlot *slot, int layout)
{
    if (slot->layout >= 0 && slot->layout != layout) {
        return NULL;
    }
    if (slot->state != 0) {
        return NULL;
    }
    slot->state = -1;
    return slot->base;
}

void func_003E8C00(HeapSlot *slot, void *base, int layout, u32 size)
{
    slot->state = 0;
    slot->base = base;
    slot->layout = layout;
    slot->size = size;
}

/* Probably not heap code (no Heap, called from unit_003C35C0 only): maps n (0-99) to one of 0xA8E-0xA93. */
int func_003E8C20(int n)
{
    if (n == 99) {
        return 0xA93;
    }
    if (n == 89 || n == 79 || n == 59 || n == 39 || n == 19) {
        return 0xA92;
    }
    if (n == 93 || n == 73 || n == 53 || n == 33 || n == 13) {
        return 0xA91;
    }
    if (n == 85 || n == 65 || n == 45 || n == 25 || n == 5) {
        return 0xA90;
    }
    if (n == 94 || n == 86 || n == 80 || n == 74 || n == 66 || n == 60 || n == 54 || n == 46 || n == 40 || n == 34 ||
        n == 26 || n == 20 || n == 14 || n == 6 || n == 0) {
        return 0xA8E;
    }
    return 0xA8F;
}
