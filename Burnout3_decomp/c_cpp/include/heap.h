#ifndef HEAP_H
#define HEAP_H

#include "types.h"

/* Heap core (d4/heap, 0x3E7850-0x3E8C20): the memory manager's table of sub-heaps ("slots"), looked up by
 * category, taken, released and aged once per frame. The same object is the memory manager at 0x1D6D880 (vtable
 * 0x4DE030, unit_00222300), which carves the arena into these slots at start-up (func_003E8C00) and handles
 * categories 4 and 0x17-0x1A itself. Categories 0-22 are the game's; what each holds is still open.
 * Three layouts (0, 1, 2) share the arena; layouts 0 and 1 use the l1 group for categories 2, 6, 7, 9 and 13.
 * Names are ours. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HeapSlot {
    s32 state;   /* 0x0 0 free, -1 taken, > 0 released and still aging (frames until free) */
    s32 layout;  /* 0x4 layout the slot belongs to, -1 for every layout */
    u32 size;    /* 0x8 */
    void *base;  /* 0xC */
} HeapSlot;

/* Slots that layouts 1 (l1) and 2 (l2) each have their own copies of; same order, different counts. */
typedef struct HeapLayout1 {
    HeapSlot cat1[6];   /* 0x104 */
    HeapSlot cat3;      /* 0x164 */
    HeapSlot cat5;      /* 0x174 */
    HeapSlot cat6[10];  /* 0x184 */
    HeapSlot cat7[18];  /* 0x224 */
    HeapSlot cat9;      /* 0x344 */
    HeapSlot cat13;     /* 0x354 */
    HeapSlot cat2[12];  /* 0x364 */
} HeapLayout1;

typedef struct HeapLayout2 {
    HeapSlot cat1[6];   /* 0x424 */
    HeapSlot cat3;      /* 0x484 */
    HeapSlot cat5;      /* 0x494 */
    HeapSlot cat6[19];  /* 0x4A4 */
    HeapSlot cat7[35];  /* 0x5D4 */
    HeapSlot cat9;      /* 0x804 */
    HeapSlot cat13;     /* 0x814 */
    HeapSlot cat2[12];  /* 0x824 */
} HeapLayout2;

typedef struct Heap {
    void *vtable;         /* 0x000 */
    HeapSlot cat14;       /* 0x004 */
    HeapSlot cat10;       /* 0x014 */
    HeapSlot cat11;       /* 0x024 */
    HeapSlot cat12;       /* 0x034 */
    HeapSlot cat15;       /* 0x044 */
    HeapSlot cat0;        /* 0x054 */
    HeapSlot cat8;        /* 0x064 */
    HeapSlot cat21[8];    /* 0x074 */
    HeapSlot cat22;       /* 0x0F4 */
    HeapLayout1 l1;       /* 0x104 */
    HeapLayout2 l2;       /* 0x424 */
    HeapSlot cat1_l0[2];  /* 0x8E4 */
    HeapSlot cat19;       /* 0x904 */
    HeapSlot cat3_l0;     /* 0x914 */
    HeapSlot slot924;     /* 0x924 aged, but no category reaches it here */
    HeapSlot cat5_l0;     /* 0x934 */
    HeapSlot cat16;       /* 0x944 */
    HeapSlot cat20;       /* 0x954 */
    HeapSlot cat17;       /* 0x964 */
    HeapSlot cat18;       /* 0x974 */
    s32 layout;           /* 0x984 current layout, -1 for none */
    s32 layoutRelease;    /* 0x988 > 0: frames until the layout is dropped */
    HeapSlot extra[4];    /* 0x98C categories 0x17-0x1A (unit_00222300) */
    /* ... to 0xA00 */
} Heap;

u32 func_003E7850(Heap *heap, int cat);                   /* size of a category's (first) slot, -1 if none */
void *func_003E7A50(Heap *heap, int cat);                 /* base of category 13's slot without taking it */
void func_003E7AD0(Heap *heap, int cat, int index);       /* release a category's slot */
void *func_003E7D40(Heap *heap, int cat, int index);      /* take a category's slot: its base, or 0 */
void func_003E8750(Heap *heap);                           /* drop the layout in two frames */
int func_003E8760(Heap *heap, int layout);                /* set the layout; 0 while one is being dropped */
void func_003E87A0(Heap *heap);                           /* once per frame: age released slots */

u32 func_003E8B70(HeapSlot *slot);
void func_003E8B80(HeapSlot *slot);                       /* age */
void func_003E8BA0(HeapSlot *slot);                       /* release */
void *func_003E8BB0(HeapSlot *slot, int layout);          /* take */
void func_003E8C00(HeapSlot *slot, void *base, int layout, u32 size);

int func_003E8C20(int n);

#ifdef __cplusplus
}
#endif

#endif
