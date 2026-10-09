/* D5 mode lifecycle. Drawing callbacks remain in the original assembly for D10. */
#include "play_flow.h"

struct ClientFlag { u8 enabled; u8 rest[0x4F20 - 1]; };
struct InputRosterView { u8 pad0[0xB080]; ClientFlag clients[8]; };

extern "C" {
void func_00135420(NetworkMode *self)
{
    s32 i;
    func_00134600(&self->base);
    func_002709D0(D_01E32AE0);
    D_01E65499[0] = 0;
    D_01E6549A[0] = 1;
    D_00516E10[0] = 1;
    func_00135A90(D_00516DF0);
    D_00516E34[0] = 1;
    func_00135A90(D_00516E14);
    for (i = 0; i < 8; i++) { ((InputRosterView *)D_01E32AE0)->clients[i].enabled = 0; }
    self->state = 24;
    D_0051BAA8[0] = 1;
    func_003E8750((Heap *)theMemMgr);
}

void func_00135620(NetworkMode *self)
{
    if (D_004EB1E0[0]) {
        func_00134660(&self->base);
        if (self->base.target->unk28()) {
            func_00131F10(&D_004EE040, (GameModeRef *)D_0051A6A8);
        }
        func_001A0930(D_0066A3D0, D_004F509C[0]);
        if (self->base.target != (GameModeTarget *)D_0051B714 && self->base.target != (GameModeTarget *)D_0051B788) {
            if (D_0064EC9A[0]) {
                if (!D_0051BAA6[0]) { D_0051BAA5[0] = 1; }
            } else if (D_0051BAA6[0]) {
                D_0051BAA5[0] = 0;
            }
        }
    }
    if (D_004EB1E1[0]) { self->base.target->unk14(); }
}

s32 func_00135740(NetworkMode *self)
{
    s32 i;
    s32 index;
    bool removed;
    FlowPlayer *player;
    if (!func_003E8760((Heap *)theMemMgr, 1)) { return 0; }
    if (!func_001346C0(&self->base)) { return 0; }
    switch (self->state) {
    case 1:
    case 24:
        D_01EA2F0D[0] = 1;
        D_01EA2F0E[0] = 1;
        func_00130C20((FrontendMode *)D_0051A6A8, 0x94413E09C8514CA1ull);
        D_01E65499[0] = 1;
        removed = false;
        D_01E6549A[0] = 0;
        for (i = 0; i < D_01E32B08[0]; i++) {
            player = func_002366D0(D_01E32AE0, i);
            if (player == 0) {
                D_01E32AE4[0] = 1;
                removed = true;
            } else {
                index = removed ? i - 1 : i;
                if (D_0051BA88[0]->target->unk9C() != 3) {
                    player->traffic = &D_0065D930[index];
                    D_0065F3B8[index].player = player;
                }
            }
        }
        func_0026CAB0(D_01E32AE0);
        self->state = 2;
    case 2:
        self->unk1C0 = 0;
        self->state = 3;
    case 3:
        self->state = 4;
    case 4:
        self->state = 5;
    case 5:
        self->state = 6;
    case 6:
        for (i = 0; i < D_01E32B08[0]; i++) {
            player = func_002366D0(D_01E32AE0, i);
            if (player != 0) { player->enabled = 1; }
        }
        self->state = 7;
        D_01E65458[0] = 0;
    default:
        func_00270AC0(D_01E32AE0);
        func_00135AF0(D_00516DF0);
        D_00516E10[0] = 1;
        func_00135A90(D_00516DF0);
        D_00516E34[0] = 0;
        func_0019E990(D_00665EC0);
        D_0066A544[0] = self->base.target->unk30();
        func_001A0660(D_0066A3D0, 1);
        func_00289AF0(D_01E77540, 1);
        self->state = 23;
        return 1;
    }
}

void func_00135A60(NetworkMode *self)
{
    func_00134910(&self->base);
    self->state = 1;
}
}
