#ifndef GAMEMODE_H
#define GAMEMODE_H

#include "types.h"

#ifdef __cplusplus

/* The object a game mode drives (this+0x1B4). Only the slots this unit calls are named; MW vtables start with two
 * header words, so the first virtual is at +0x08. */
class GameModeTarget {
public:
    virtual void unk08();  /* vtable +0x08 */
    virtual void unk0C();  /* vtable +0x0C */
    virtual void unk10();  /* vtable +0x10 */
    virtual void unk14();  /* vtable +0x14 */
    virtual void unk18();  /* vtable +0x18 */
    virtual void unk1C();  /* vtable +0x1C */
    virtual void unk20();  /* vtable +0x20 */
    virtual void unk24();  /* vtable +0x24 */
    virtual s32 unk28();   /* vtable +0x28 */
    virtual s32 unk2C();   /* vtable +0x2C */
    virtual s32 unk30();   /* vtable +0x30 */
    virtual void unk34();
    virtual void unk38();
    virtual void unk3C();
    virtual void unk40();
    virtual void unk44();
    virtual void unk48();
    virtual void unk4C();
    virtual void unk50();
    virtual void unk54();
    virtual void unk58();
    virtual void unk5C();
    virtual void unk60();
    virtual void unk64();
    virtual void unk68();
    virtual void unk6C();
    virtual void unk70();
    virtual void unk74();
    virtual void unk78();
    virtual void unk7C();
    virtual void unk80();
    virtual void unk84();
    virtual void unk88();
    virtual void unk8C();
    virtual void unk90();
    virtual void unk94();
    virtual void unk98();
    virtual s32 unk9C();
    virtual void unkA0();
    virtual void unkA4();
    virtual void unkA8();
    virtual s32 unkAC();   /* vtable +0xAC */
};

/* A game mode (state of the top-level state machine). Shared lifecycle helpers are at
 * 0x134600-0x134930; the modes are members of one big global object
 * whose vtables sinit installs (this unit's vtable D_004DDD90). */
struct GameMode {
    u64 sceneHash;         /* 0x00 */
    u64 playerHash[2];     /* 0x08 */
    u64 opponentHash[5];   /* 0x18 */
    u64 trafficHash[5];    /* 0x40 */
    u64 unk68;            /* 0x68 */
    s32 playerType[2];    /* 0x70 */
    s32 opponentType[5];  /* 0x78 */
    s32 trafficType[5];   /* 0x8C */
    s32 playerCount;      /* 0xA0 */
    s32 opponentCount;    /* 0xA4 */
    s32 trafficCount;     /* 0xA8 */
    u8 padAC[0x1B4 - 0xAC];
    GameModeTarget *target; /* 0x1B4 */
    GameModeTarget *aux;    /* 0x1B8 */
    u8 unk1BC;              /* 0x1BC */
};

/* The derived mode behind D_004DDD90. */
struct GameModeC940 {
    GameMode base;
    s32 unk1C0[2]; /* 0x1C0, cleared by func_0013CDD0 */
};

extern "C" {
#else
typedef struct GameMode GameMode;
typedef struct GameModeC940 GameModeC940;
#endif

/* vtable D_004DDD90, in slot order (+0x08 on) */
void func_0013CDD0(GameModeC940 *self);
s32 func_0013CC70(GameMode *self);
void func_0013CBC0(GameMode *self);
void func_0013CA10(GameMode *self);
void func_0013C9B0(GameMode *self);
s32 func_0013C940(GameMode *self);
void func_0013C990(GameMode *self);
void func_0013C9A0(GameMode *self);

s32 func_0013CBB0(void); /* returns 0; shared slot of other vtables */
void func_00134600(GameMode *self);
void func_00134660(GameMode *self);
s32 func_001346C0(GameMode *self);
void func_00134910(GameMode *self);

#ifdef __cplusplus
}
#endif

#endif
