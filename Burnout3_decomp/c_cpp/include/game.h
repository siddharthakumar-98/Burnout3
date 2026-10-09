#ifndef GAME_H
#define GAME_H

#include "types.h"

#ifdef __cplusplus
#include "loadqueue.h"
#endif

#ifdef __cplusplus

/* The object the current game mode drives (mode+0x1B4), as game/unit_00131D20 sees it. Only the slots this unit
 * calls are named; MW vtables start with two header words, so the first virtual is at +0x08. */
class GameModeDriver {
public:
    virtual void unk08();
    virtual void unk0C();
    virtual void unk10();
    virtual void unk14();
    virtual void unk18();
    virtual void unk1C();
    virtual void unk20();
    virtual void unk24();
    virtual void unk28();
    virtual void unk2C();
    virtual void unk30();
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
    virtual s32 unk98();  /* vtable +0x98: state (6 checked) */
    virtual s32 unk9C();  /* vtable +0x9C: sub-state (3, 4, 5 checked) */
    virtual s32 unkA0();  /* vtable +0xA0 */
};

/* A game mode (see gamemode.h for the full view). MW puts the vtable pointer after the data members of the class
 * that declares the first virtual, here at +0x1B0 (gamemode.h's vtable D_004DDD90). */
class GameModeRef {
public:
    u8 pad0[0x1AC];
    u8 unk1AC;              /* 0x1AC */
    virtual void unk08();
    virtual s32 unk0C();    /* vtable +0x0C */
    virtual void unk10();
    virtual void unk14();
    virtual void unk18();
    virtual void unk1C();
    GameModeDriver *driver; /* 0x1B4 */
    void *unk1B8;
    u8 reload;             /* 0x1BC */
};

/* Objects this unit only starts, updates and shuts down through their vtables (D_01EA2970, Game+0x28E00,
 * Game+0x299B0). */
class GameSubsystem {
public:
    virtual void unk08();
    virtual void unk0C(); /* vtable +0x0C: shutdown */
    virtual void unk10(); /* vtable +0x10: per-frame update */
};

/* A loaded data file's header as func_00133190 patches it. */
struct GameDataHeader {
    s32 unk0;
    s32 *unk4; /* unk4[1] is clamped */
};

/* The game object main passes around (D_004EE040). */
struct Game {
    u8 pad0[0x7000];
    u8 unk7000[0x7080 - 0x7000];     /* 0x7000 */
    u8 unk7080[0x8080 - 0x7080];     /* 0x7080 */
    void *unk8080;                   /* 0x8080: a CGTFileSystem (the tuning database's) */
    u8 pad8084[0x80C0 - 0x8084];
    CAsyncLoadManager loader;        /* 0x80C0 */
    u8 unk8854[0x8860 - 0x8854];     /* 0x8854 */
    u8 unk8860[0x28DB0 - 0x8860];    /* 0x8860: a vdb database */
    u8 unk28DB0[0x28DD4 - 0x28DB0];  /* 0x28DB0 */
    u8 unk28DD4[0x28E00 - 0x28DD4];  /* 0x28DD4 */
    u8 unk28E00[0x299A6 - 0x28E00];  /* 0x28E00: a GameSubsystem */
    u8 unk299A6;
    u8 pad299A7[0x299B0 - 0x299A7];
    u8 unk299B0[0x2A556 - 0x299B0];  /* 0x299B0: a GameSubsystem */
    u8 unk2A556;
    u8 pad2A557[0x2A560 - 0x2A557];
    u8 unk2A560[0x2A640 - 0x2A560];  /* 0x2A560 */
    s32 unk2A640;                    /* 0x2A640: seeded 0xFD462907 */
    s32 unk2A644;                    /* 0x2A644: seeded 0x02B9D6F8 */
    void *unk2A648;                  /* 0x2A648: start-up file (category 0xE block) */
    bool unk2A64C;                   /* 0x2A64C: its load is done */
    u8 pad2A64D[0x2A650 - 0x2A64D];
    GameDataHeader *unk2A650;        /* 0x2A650: = unk2DA7C once loaded */
    bool unk2A654;                   /* 0x2A654: load done flags */
    bool unk2A655;                   /* 0x2A655 */
    bool unk2A656;                   /* 0x2A656 */
    bool unk2A657;                   /* 0x2A657 */
    bool unk2A658;                   /* 0x2A658 */
    u8 pad2A659[0x2A660 - 0x2A659];
    u8 unk2A660[4];                  /* 0x2A660: a 0x1000-byte file buffer */
    s32 unk2A664;                    /* 0x2A664 */
    u8 pad2A668[0x2B660 - 0x2A668];
    u8 unk2B660[4];                  /* 0x2B660: a 0x1000-byte file buffer */
    s32 unk2B664;                    /* 0x2B664 */
    u8 pad2B668[0x2C660 - 0x2B668];
    bool unk2C660;                   /* 0x2C660 */
    bool unk2C661;                   /* 0x2C661 */
    u8 unk2C662;                     /* 0x2C662 */
    u8 pad2C663[0x2C668 - 0x2C663];
    u8 unk2C668[0x2C9A0 - 0x2C668];  /* 0x2C668: an embedded mode */
    u8 unk2C9A0[0x2CCC0 - 0x2C9A0];  /* 0x2C9A0: an embedded mode */
    u8 unk2CCC0[0x2CE80 - 0x2CCC0];  /* 0x2CCC0: an embedded mode */
    u8 unk2CE80[0x2D048 - 0x2CE80];  /* 0x2CE80: an embedded mode */
    u8 unk2D048[0x2D218 - 0x2D048];  /* 0x2D048: an embedded mode */
    u8 unk2D218[0x2D264 - 0x2D218];  /* 0x2D218: embedded drivers from here to 0x2D948 */
    u8 unk2D264[0x2D2B0 - 0x2D264];
    u8 unk2D2B0[0x2D300 - 0x2D2B0];
    u8 unk2D300[0x2D374 - 0x2D300];
    u8 unk2D374[0x2D3CC - 0x2D374];
    u8 unk2D3CC[0x2D424 - 0x2D3CC];
    u8 unk2D424[0x2D47C - 0x2D424];
    u8 unk2D47C[0x2D4D4 - 0x2D47C];
    u8 unk2D4D4[0x2D52C - 0x2D4D4];
    u8 unk2D52C[0x2D578 - 0x2D52C];
    u8 unk2D578[0x2D5EC - 0x2D578];
    u8 unk2D5EC[0x2D650 - 0x2D5EC];
    u8 unk2D650[0x2D6D4 - 0x2D650];
    u8 unk2D6D4[0x2D748 - 0x2D6D4];
    u8 unk2D748[0x2D7BC - 0x2D748];
    u8 unk2D7BC[0x2D8E0 - 0x2D7BC];
    u8 unk2D8E0[0x2D8E8 - 0x2D8E0];
    u8 unk2D8E8[0x2D948 - 0x2D8E8];
    u8 unk2D948[0x2DA00 - 0x2D948];
    u8 unk2DA00;                     /* 0x2DA00 */
    u8 pad2DA01[0x2DA04 - 0x2DA01];
    s32 unk2DA04;                    /* 0x2DA04 */
    s32 unk2DA08;                    /* 0x2DA08 */
    u8 pad2DA0C[0x2DA10 - 0x2DA0C];
    u8 unk2DA10[0x2DA38 - 0x2DA10];  /* 0x2DA10: frame timer (D_0051BA50) */
    s32 unk2DA38;
    u8 pad2DA3C[0x2DA48 - 0x2DA3C];
    GameModeRef *current;            /* 0x2DA48: current mode */
    GameModeRef *next;               /* 0x2DA4C: requested mode */
    u8 unk2DA50;                     /* 0x2DA50 */
    s32 unk2DA54;                    /* 0x2DA54 */
    s32 unk2DA58;                    /* 0x2DA58 */
    s32 unk2DA5C;                    /* 0x2DA5C */
    s32 unk2DA60;                    /* 0x2DA60 */
    u8 unk2DA64;                     /* 0x2DA64 */
    u8 unk2DA65;                     /* 0x2DA65 */
    u8 unk2DA66;                     /* 0x2DA66 */
    u8 pad2DA67;
    s32 unk2DA68;                    /* 0x2DA68 */
    s32 unk2DA6C;                    /* 0x2DA6C */
    s32 unk2DA70;                    /* 0x2DA70 */
    void *unk2DA74;                  /* 0x2DA74: category 0xA block */
    void *unk2DA78;                  /* 0x2DA78: category 0xB block */
    void *unk2DA7C;                  /* 0x2DA7C: category 0xC block */
    u8 unk2DA80;                     /* 0x2DA80 */
    u8 unk2DA81;                     /* 0x2DA81 */
    u8 unk2DA82;                     /* 0x2DA82 */
    u8 pad2DA83;
    s32 unk2DA84;                    /* 0x2DA84 */
    f32 unk2DA88;                    /* 0x2DA88: unk2DA84 scaled by D_0052267C */
    s32 unk2DA8C;                    /* 0x2DA8C: next state? */
    s32 unk2DA90;                    /* 0x2DA90: state */
    u8 unk2DA94;                     /* 0x2DA94 */
    u8 unk2DA95;                     /* 0x2DA95 */
    u8 unk2DA96;                     /* 0x2DA96 */
};
extern "C" {
#else
typedef struct Game Game;
#endif

s32 func_00131D20(Game *game);
s32 func_00131D90(Game *game);
s32 func_00131DF0(void);
s32 func_00131E00(Game *game);
s32 func_00131E40(Game *game);
void func_00131F10(Game *game, GameModeRef *next);
#ifdef __cplusplus
bool func_00131F60(Game *game);
#else
u8 func_00131F60(Game *game);
#endif
void func_00131F90(Game *game);
void func_00132090(Game *game);
s32 func_001322B0(Game *game);
void func_00132420(Game *game);
void func_00132560(Game *game);
void func_00132600(Game *game);
void func_00133180(void);
s32 func_00133190(Game *game);
void func_00133BB0(Game *game);

#ifdef __cplusplus
}
#endif

#endif
