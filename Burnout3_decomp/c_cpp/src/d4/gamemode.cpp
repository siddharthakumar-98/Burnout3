/* game/unit_0013C940 (0x13C940-0x13CE20): a game mode, derived from the base mode in game/unit_00134570 (D5 work,
 * matched while looking for the load queue that follows it). C++ for the virtual calls on the mode's target (they
 * load the slot into $t9); the mode's virtual methods keep their func_ names: they are written as extern "C"
 * functions taking `self` (same code as methods), and the vtable is emitted as plain data. */

#include "gamemode.h"

extern "C" {

/* base mode (game/unit_00134570) */
void func_00134600(GameMode *self);
void func_00134660(GameMode *self);
s32 func_001346C0(GameMode *self);
void func_00134910(GameMode *self);
void func_00134DF0(void *p);

void func_0014E860(void *p);
s32 func_00131E40(void *p);
void func_00135A90(void *p);
void func_00135AF0(void *p);
void func_0019E760(void *p);
void func_0019E990(void *p);
s32 func_0019E9E0(void *p, s32 i);
void func_0019EA30(void *p);
void func_0019EAC0(void *p, s32 i);
void func_001A0660(void *p, s32 i);
void func_001A0790(void *p);
void func_001A0930(void *p, f32 f);
void func_00130C20(void *p, u64 hash);
void func_00217E70(s32 i);
void func_00219F20(void);
void func_0021A0F0(void);
void func_00227360(void *p, s32 i);
void func_002768F0(void *p);
void func_00276870(void *p);
void func_00289AF0(void *p, s32 i);
void func_0030A9D0(void *p, s32 a, void *b);
void func_0030ED50(void *p);
void func_003E8750(void *p);
s32 func_003E8760(void *p, s32 i);

/* Incomplete arrays keep these out of small data, so they load with lui like the original. */
extern u8 D_004EB1E0[];
extern u8 D_004EB1E1[];
extern u8 D_004EE040[];
extern f32 D_004F509C[];
extern u8 D_00516DF0[];
extern u8 D_00516E10[]; /* D_00516DF0 + 0x20 */
extern u8 D_00516E14[];
extern u8 D_00516E34[]; /* D_00516E14 + 0x20 */
extern u8 D_0051A6A8[];
extern GameMode *D_0051BA88[];
extern u8 D_0051BAA5[];
extern s32 D_0051BAA8[];
extern s32 D_0051BACC[];
extern u8 D_00522660[];
extern u8 D_006481A0[];
extern u8 D_0064D3B0[];
extern u8 D_0064FB80[];
extern u8 D_00665E52[];
extern u8 D_00665EC0[];
extern u8 D_00666090[];
extern u8 D_00666130[];
extern u8 D_0066A3D0[];
extern s32 D_0066A544[];
extern u8 theMemMgr[];
extern u8 D_01E65499[];
extern u8 D_01E77540[];
extern s32 D_01E90430[];
extern u8 D_01EA2970[];
extern u64 *D_01EA2974[];

/* .data: vtable of this mode (MW header, then the slots) */
void *D_004DDD90[12] = {
    0,
    0,
    (void *)func_0013CDD0,
    (void *)func_0013CC70,
    (void *)func_0013CBC0,
    (void *)func_0013CA10,
    (void *)func_0013C9B0,
    (void *)func_0013C940,
    (void *)func_0013C990,
    (void *)func_0013C9A0,
    0,
    0,
};

s32 func_0013C940(GameMode *self)
{
    if (!self->target->unk2C()) {
        return 0;
    }
    D_00665E52[0] = 1;
    return 1;
}

void func_0013C990(GameMode *self)
{
    func_0014E860(D_006481A0);
}

void func_0013C9A0(GameMode *self)
{
}

void func_0013C9B0(GameMode *self)
{
    func_00134600(self);
    D_00516E10[0] = 1;
    func_00135A90(D_00516DF0);
    D_00516E34[0] = 1;
    func_00135A90(D_00516E14);
    func_003E8750(theMemMgr);
}

void func_0013CA10(GameMode *self)
{
    u8 *p = D_0064D3B0;
    u64 hash;

    func_0019EAC0(D_00665EC0, 1);
    func_0019EAC0(D_00665EC0, 2);
    func_0030A9D0(D_00666090, func_0019E9E0(D_00665EC0, 1), p);
    func_0030A9D0(D_00666130, func_0019E9E0(D_00665EC0, 2), D_0064FB80);
    if (func_00131E40(D_004EE040) && D_0051BA88[0]->target->unkAC()) {
        func_002768F0(D_00522660);
    } else {
        func_00276870(D_00522660);
    }
    func_00227360(D_00665EC0, 0);
    if (D_01E90430[0] != 0) {
        if (D_01EA2974[0] != 0) {
            hash = *D_01EA2974[0];
        } else {
            hash = 0x6D6123044330FCCFull;
        }
        if (hash != 0x94413EA3518C496Cull) {
            goto end;
        }
    }
    func_001A0790(D_0066A3D0);
    func_0021A0F0();
    func_00217E70(0);
    func_00134DF0(p);
    func_00219F20();
end:
    func_0019EA30(D_00665EC0);
}

s32 func_0013CBB0(void)
{
    return 0;
}

void func_0013CBC0(GameMode *self)
{
    if (D_004EB1E0[0]) {
        func_00134660(self);
        func_001A0930(D_0066A3D0, D_004F509C[0]);
        if (self->target->unk28()) {
            D_0051BACC[0] = 12;
        }
    }
    if (D_004EB1E1[0]) {
        self->target->unk14();
    }
}

s32 func_0013CC70(GameMode *self)
{
    D_01E65499[0] = 0;
    if (!func_003E8760(theMemMgr, 2)) {
        return 0;
    }
    if (!func_001346C0(self)) {
        return 0;
    }
    func_00135AF0(D_00516DF0);
    func_00135AF0(D_00516E14);
    D_00516E10[0] = 1;
    func_00135A90(D_00516DF0);
    D_00516E34[0] = 1;
    func_00135A90(D_00516E14);
    if (func_00131E40(D_004EE040)) {
        func_0019E990(D_00665EC0);
    } else {
        func_0019E760(D_00665EC0);
    }
    D_0066A544[0] = self->target->unk30();
    func_001A0660(D_0066A3D0, 2);
    func_00289AF0(D_01E77540, 2);
    func_0030ED50(D_01EA2970);
    func_00130C20(D_0051A6A8, 0x94413FC035B0F871ull);
    D_0051BAA5[0] = 1;
    D_0051BAA8[0] = 0;
    return 1;
}

/* 94.44%: the original enters the loop with a jump to the condition; every for/while/do/inline form tried drops the
 * guard (i < 2 is known true on entry). */
void func_0013CDD0(GameModeC940 *self)
{
    s32 i;

    func_00134910(&self->base);
    for (i = 0; i < 2; i++) {
        self->unk1C0[i] = 0;
    }
}

/* CAsyncLoadManager::Abort (0x13CE20) onward is the load queue, another file (c_cpp/src/d4/loadqueue.cpp). */

}
