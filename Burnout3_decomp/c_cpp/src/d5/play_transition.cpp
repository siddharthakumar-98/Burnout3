/* D5 mode lifecycle. Drawing callbacks remain in the original assembly for D10. */
#include "play_flow.h"

extern "C" {
void func_00135180(GameMode *self)
{
    if (D_004EB1E0[0]) {
        func_00134660(self);
        if (self->target->unk28()) { D_0051BACC[0] = 12; }
        if (D_0064EC9A[0]) {
            if (!D_0051BAA6[0]) {
                D_0051BAA8[0] = 0;
                D_0051BAA5[0] = 1;
            }
        } else if (D_0051BAA6[0]) {
            D_0051BAA8[0] = 1;
            D_0051BAA5[0] = 0;
        }
    }
    func_001A0930(D_0066A3D0, D_004F509C[0]);
    if (D_004EB1E1[0]) { self->target->unk14(); }
}

s32 func_00135280(GameMode *self)
{
    D_01E65499[0] = 0;
    if (!func_003E8760((Heap *)theMemMgr, 1)) { return 0; }
    if (!func_001346C0(self)) { return 0; }
    func_00135AF0(D_00516DF0);
    D_00516E10[0] = 1;
    func_00135A90(D_00516DF0);
    D_00516E34[0] = 0;
    func_0019E990(D_00665EC0);
    D_0066A544[0] = self->target->unk30();
    func_001A0660(D_0066A3D0, 1);
    func_00289AF0(D_01E77540, 1);
    func_0030ED50(D_01EA2970);
    func_00130C20((FrontendMode *)D_0051A6A8, 0x94413FC035B0F871ull);
    D_0051BAA8[0] = 0;
    return 1;
}

void func_001353A0(GameMode *self) { func_00134910(self); }

s32 func_001353B0(NetworkMode *self) { return self->unk1C4; }

s32 func_001353C0(NetworkMode *self)
{
    if (!self->base.target->unk2C()) { return 0; }
    D_00665E52[0] = 1;
    return 1;
}

void func_00135410(NetworkMode *self) { self->state = 25; }
}
