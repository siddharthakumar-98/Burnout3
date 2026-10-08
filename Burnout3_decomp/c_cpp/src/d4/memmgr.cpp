/* game/unit_00222300 (0x222300-0x222C90): the memory manager, the heap core's object (Heap) seen through its
 * vtable 0x4DE030. Its virtual methods are written as extern "C" functions taking
 * `self` (same code as methods). */

#include "memmgr.h"

extern "C" {

void free(void *p);

extern void *memMgrMallocBlock;
extern u8 *memMgrArena;   /* the arena's base, set by memMgrInit */
extern u8 D_0067D880[];  /* the arena, 0x67D880-0x1D6D880 */

u32 memMgrSize(Heap *self, int cat)
{
    switch (cat) {
    case 0x17:
        return func_003E8B70(&self->extra[1]);
    case 0x18:
        return func_003E8B70(&self->extra[2]);
    case 0x19:
        return func_003E8B70(&self->extra[0]);
    case 0x1A:
        return func_003E8B70(&self->extra[3]);
    case 4:
        return 0x80000;
    }
    return func_003E7850(self, cat);
}

void *memMgrPeekCat13(Heap *self, int cat)
{
    return func_003E7A50(self, cat);
}

void memMgrRelease(Heap *self, int cat, int index)
{
    switch (cat) {
    case 0x17:
        func_003E8BA0(&self->extra[1], self->layout);
        break;
    case 0x18:
        func_003E8BA0(&self->extra[2], self->layout);
        break;
    case 0x19:
        func_003E8BA0(&self->extra[0], self->layout);
        break;
    case 0x1A:
        func_003E8BA0(&self->extra[3], self->layout);
        break;
    case 4:
        if (self->layout == 0) {
            func_003E8BA0(&self->cat5_l0, self->layout);
        } else if (self->layout == 2) {
            func_003E8BA0(&self->l2.cat5, self->layout);
        } else {
            func_003E8BA0(&self->l1.cat5, self->layout);
        }
        break;
    default:
        func_003E7AD0(self, cat, index);
        break;
    }
}

void *memMgrTake(Heap *self, int cat, int index)
{
    switch (cat) {
    case 0x17:
        return func_003E8BB0(&self->extra[1], self->layout);
    case 0x18:
        return func_003E8BB0(&self->extra[2], self->layout);
    case 0x19:
        return func_003E8BB0(&self->extra[0], self->layout);
    case 0x1A:
        return func_003E8BB0(&self->extra[3], self->layout);
    case 4:
        if (self->layout == 0) {
            return func_003E8BB0(&self->cat5_l0, self->layout);
        } else if (self->layout == 2) {
            return func_003E8BB0(&self->l2.cat5, self->layout);
        } else {
            return func_003E8BB0(&self->l1.cat5, self->layout);
        }
    }
    return func_003E7D40(self, cat, index);
}

void memMgrUpdate(Heap *self)
{
    func_003E87A0(self);
    func_003E8B80(&self->extra[0], self->layout);
    func_003E8B80(&self->extra[3], self->layout);
    func_003E8B80(&self->extra[1], self->layout);
    func_003E8B80(&self->extra[2], self->layout);
}

void memMgrFreeBlock(Heap *self)
{
    if (memMgrMallocBlock != 0) {
        free(memMgrMallocBlock);
    }
    memMgrMallocBlock = 0;
    memMgrArena = 0;
}

void memMgrInit(Heap *self)
{
    u8 *arena;
    u8 *shared;
    int i;

    self->layout = -1;
    self->layoutRelease = 0;
    memMgrArena = D_0067D880;
    arena = memMgrArena;
    shared = arena + 0x3E3800;

    /* every layout */
    func_003E8C00(&self->cat14, arena, -1, 0x127000);
    func_003E8C00(&self->cat10, arena + 0x127000, -1, 0x41800);
    func_003E8C00(&self->cat11, arena + 0x168800, -1, 0x800);
    func_003E8C00(&self->cat12, arena + 0x169000, -1, 0x5000);
    func_003E8C00(&self->cat15, arena + 0x16E000, -1, 0x9000);
    func_003E8C00(&self->cat0, arena + 0x177000, -1, 0x8000);
    func_003E8C00(&self->extra[0], arena + 0x17F000, -1, 0x200000);
    func_003E8C00(&self->extra[3], arena + 0x37F000, -1, 0x5800);
    func_003E8C00(&self->cat8, arena + 0x39D800, -1, 0x4000);
    func_003E8C00(&self->cat22, arena + 0x3C3000, -1, 0x20800);
    for (i = 0; i < 8; i++) {
        func_003E8C00(&self->cat21[i], arena + i * 0x4300 + 0x3A1800, -1, 0x4300);
    }

    /* layout 0 */
    for (i = 0; i < 2; i++) {
        func_003E8C00(&self->cat1_l0[i], shared + i * 0x72000, 0, 0x72000);
    }
    func_003E8C00(&self->cat19, shared + 0xE4000, 0, 0x20000);
    func_003E8C00(&self->cat3_l0, shared + 0x104000, 0, 0x8800);
    func_003E8C00(&self->cat5_l0, shared + 0x10C800, 0, 0xE0000);
    func_003E8C00(&self->cat16, shared + 0x1EC800, 0, 0x900000);
    func_003E8C00(&self->cat17, shared + 0xAECC00, 0, 0x4EB000);
    func_003E8C00(&self->cat20, shared + 0xAEC800, 0, 0x400);
    func_003E8C00(&self->cat18, shared + 0xFD7C00, 0, 0x64000);
    func_003E8C00(&self->extra[1], shared + 0x103BC00, 0, 1);
    func_003E8C00(&self->extra[2], shared + 0x103BC80, 0, 0x221000);

    /* layout 1 */
    for (i = 0; i < 6; i++) {
        func_003E8C00(&self->l1.cat1[i], shared + i * 0x72000, 1, 0x72000);
    }
    func_003E8C00(&self->l1.cat3, shared + 0x5AC000, 1, 0x8800);
    func_003E8C00(&self->l1.cat5, shared + 0x5B4800, 1, 0x780000);
    for (i = 0; i < 10; i++) {
        func_003E8C00(&self->l1.cat6[i], shared + i * 0x28000 + 0xD34800, 1, 0x28000);
    }
    for (i = 0; i < 18; i++) {
        func_003E8C00(&self->l1.cat7[i], shared + i * 0x18000 + 0xEC4800, 1, 0x18000);
    }
    func_003E8C00(&self->l1.cat9, shared + 0x1074800, 1, 0x180000);
    func_003E8C00(&self->l1.cat13, shared + 0x11F4800, 1, 0x18000);
    for (i = 0; i < 12; i++) {
        func_003E8C00(&self->l1.cat2[i], shared + i * 0x40000 + 0x2AC000, 1, 0x40000);
    }

    /* layout 2 */
    for (i = 0; i < 6; i++) {
        func_003E8C00(&self->l2.cat1[i], shared + i * 0x72000, 2, 0x72000);
    }
    func_003E8C00(&self->l2.cat3, shared + 0x5AC000, 2, 0x8800);
    func_003E8C00(&self->l2.cat5, shared + 0x5B4800, 2, 0x580000);
    for (i = 0; i < 19; i++) {
        func_003E8C00(&self->l2.cat6[i], shared + i * 0x28000 + 0xB34800, 2, 0x28000);
    }
    for (i = 0; i < 35; i++) {
        func_003E8C00(&self->l2.cat7[i], shared + i * 0x18000 + 0xE2C800, 2, 0x18000);
    }
    func_003E8C00(&self->l2.cat9, shared + 0x1174800, 2, 0x180000);
    func_003E8C00(&self->l2.cat13, shared + 0x12F4800, 2, 0x18000);
    for (i = 0; i < 12; i++) {
        func_003E8C00(&self->l2.cat2[i], shared + i * 0x40000 + 0x2AC000, 2, 0x40000);
    }
}

}
