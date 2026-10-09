#ifndef PLAY_FLOW_H
#define PLAY_FLOW_H
#ifdef __cplusplus
#include "gamemode.h"
#include "game.h"
#include "frontend_flow.h"
#include "memmgr.h"

struct NetworkMode {
    GameMode base;
    s32 unk1C0;
    s32 unk1C4;
    s32 state;
};
struct FlowPlayer {
    u8 pad0[0x5C0];
    u8 enabled;
    u8 pad5C1[0x6DC - 0x5C1];
    void *traffic;
};
struct FlowAssignment {
    FlowPlayer *player;
    u8 pad4[0x6A8 - 4];
};
struct FlowTraffic { u8 data[0x1AA0]; };

extern "C" {
extern u8 D_004EB1E0[];
extern u8 D_004EB1E1[];
extern f32 D_004F509C[];
extern Game D_004EE040;
extern u8 D_00516DF0[];
extern u8 D_00516E10[];
extern u8 D_00516E14[];
extern u8 D_00516E34[];
extern u8 D_0051A6A8[];
extern u8 D_0051B714[];
extern u8 D_0051B788[];
extern GameMode *D_0051BA88[];
extern u8 D_0051BAA5[];
extern u8 D_0051BAA6[];
extern s32 D_0051BAA8[];
extern s32 D_0051BACC[];
extern u8 D_006481A0[];
extern u8 D_0064EC9A[];
extern FlowTraffic D_0065D930[];
extern FlowAssignment D_0065F3B8[];
extern u8 D_00665E52[];
extern u8 D_00665EC0[];
extern u8 D_0066A3D0[];
extern s32 D_0066A544[];
extern u8 theMemMgr[];
extern u8 D_01E32AE0[];
extern u8 D_01E32AE4[];
extern s32 D_01E32B08[];
extern u8 D_01E3DB60[];
extern u8 D_01E65458[];
extern u8 D_01E65499[];
extern u8 D_01E6549A[];
extern u8 D_01E77540[];
extern u8 D_01EA2970[];
extern u8 D_01EA2F0D[];
extern u8 D_01EA2F0E[];

void func_0014E860(void *self);
void func_001345F0(GameMode *self);
void func_00135A90(void *self);
void func_00135AF0(void *self);
void func_0019E990(void *self);
void func_001A0660(void *self, s32 players);
void func_001A0930(void *self, f32 time);
FlowPlayer *func_002366D0(void *self, s32 player);
void func_0026CAB0(void *self);
void func_002709D0(void *self);
void func_00270AC0(void *self);
void func_00289AF0(void *self, s32 players);
void func_0030ED50(void *self);

}
#endif
#endif
