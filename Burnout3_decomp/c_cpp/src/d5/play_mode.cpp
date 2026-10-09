/* D5 mode lifecycle. Drawing callbacks remain in the original assembly for D10. */
#include "play_flow.h"

extern "C" {
void func_00134930(GameMode *self)
{
    func_0014E860(D_006481A0);
}

s32 func_00134940(GameMode *self)
{
    if (!self->target->unk2C()) { return 0; }
    D_00665E52[0] = 1;
    return 1;
}

void func_00134990(GameMode *self) { func_001345F0(self); }

void func_001349A0(GameMode *self)
{
    func_00134600(self);
    D_00665E52[0] = 0;
    D_00516E10[0] = 1;
    func_00135A90(D_00516DF0);
    D_00516E34[0] = 1;
    func_00135A90(D_00516E14);
    D_0051BAA8[0] = 1;
    D_0051BAA5[0] = 0;
    func_003E8750((Heap *)theMemMgr);
}
}
