/* game/unit_00131D20 (0x131D20-0x134260): the game object's file (D_004EE040). main (func_0012EB30) calls its
 * init (func_00133BB0), load-wait (func_00133190), main loop (func_00132600) and shutdown (func_00132560,
 * func_00132420). Names are unknown; the functions keep their func_ names (extern "C"). C++ only for the virtual
 * calls on the current mode's driver. Data (.rodata at 0x4B5B90, .bss at 0x4F5080) stays in assembly for now. */

#include "types.h"
#include "game.h"
#include "memmgr.h"
#include "vdb.h"

extern "C" {

/* Static drivers (members of the big global object) the current mode may be driving. */
extern u8 D_0051B40C[];
extern u8 D_0051B4BC[];
extern u8 D_0051B714[];
extern u8 D_004EB1E0[];
extern u8 D_004EB1E1[];
extern u8 D_00522660[];
extern f32 D_0052267C[];
extern u8 D_01EA2970[];
extern u8 D_01E75640[];
extern u8 D_01E32AE0[];
extern u8 D_00665E50[];
extern u8 D_00665E51[];
extern u8 D_00665EC0[];
extern u8 D_0051BAD4[];
extern u8 D_0051BA50[];
extern u8 D_0055D7FB[];
extern s8 D_004E2910;
extern u8 D_004F5080[];
extern u8 D_0051BB00[];
extern s32 D_01E86560[];
extern s32 D_01E86564[];
extern s32 D_01E86568[];
extern s32 D_01E8656C[];
extern s32 D_01E86570[];
extern s32 D_01E86574[];
extern s32 D_01E86578[];
extern s32 D_01E8657C[];
extern s32 D_01E86580[];
extern s32 D_01E86584[];
extern s32 D_01E86588[];
extern u8 D_01E90240[];
extern u8 D_01EA4070[];
extern u8 D_01EA7810[];
extern u8 D_01ECD520[];
extern u8 D_01ECDE90[];
extern u8 theMemMgr[];
extern CAsyncLoadManager D_004F6100; /* the global game object's loader (D_004EE040 + 0x80C0) */
extern s32 D_0051BA44;               /* the global game object's unk2DA04: the front-end text table */
extern s32 D_004E2918;               /* language */
extern s32 D_01E7751C[];
extern u8 D_01E3C63C[];
extern u8 D_01E754E0[];
extern const char *const D_004B5BA8[]; /* "Data/GlobalUs.bin" */
extern const char *const D_004B5C20[];
extern const char D_004B5C70[];
extern const char D_004B5C80[];
extern const char D_004B5CB0[];
extern const char D_004B5CD8[];
extern const char D_004B5CF0[];
extern const char D_004B5D10[];
extern const char D_004B5D28[];
extern const char D_004B5D40[];
extern const char D_004B5D60[];

void ustrSetSeparators(u16 thousands, u16 point);
u16 *ustrncpy(u16 *dst, int size, const u16 *src);
void *memset(void *dst, int c, u32 n);
void func_0014E670(void);
void func_0015D0A0(void);
void func_00130B00(void *self);
void *func_00131AA0(void *self, s32 i);
void func_00135AF0(void *self);
s32 func_001364B0(void *self);
void func_001D3C90(void *self);
s32 func_001D41E0(Game *game);
void func_0021AAF0(void *self);
void func_0021B4E0(void *self);
void func_0021B5E0(void *self);
int func_0021B830(void *db, const char *path, void *fs, void *arg);
s32 func_00222C90(void *self);
s32 func_00228070(void *self);
void func_0022B8E0(void *self, s32 arg);
void func_002514C0(void *arg);
u16 *func_00271A00(s32 *self, s32 id);
void func_00271A20(s32 *self, void *buf, const char *name, s32 n);
void func_00271A80(void *self);
s32 func_0028B380(void *self);
void func_0028B580(void *self);
void func_0030A8E0(void *self);
void func_0030DE10(void *self);
void func_00344310(void);
void func_00367E70(void *data, void *base);
s32 func_00371940(void *self);
void func_003753B0(s32 arg, s32 arg2);
s32 func_003D11D0(void *self);
void func_003E9580(void);
void func_003EA580(void);
void func_003EB0A0(void);
void func_00439110(void *self);

void func_0013C870(void *self);
void func_0013C880(void *self);
void func_0014E600(void);
void func_0015CB10(void);
void func_00130980(void *self);
void func_00199DC0(void *self);
void func_0019A1D0(void *self);
void func_0019EB10(void *self);
void func_001D3C10(void *self);
void func_001D3C50(void *self);
void func_001F4290(s32 arg);
void func_00213EB0(void *self, s32 arg);
void func_0021B760(void *self);
void func_0021B820(void *self);
void func_00270C90(void *self);
void func_00270D00(void *self);
void func_00271A10(s32 *self);
void func_0028AE40(void *self, s32 arg);
void func_0028AE80(void *self);
void func_0028AEC0(void *self);
void func_0028AF10(void *self);
void func_0028B160(void *self);
void func_002E7610(void *self, f32 dt);
void func_00344000(void);
void func_003752E0(void);
void func_003752F0(void);
void func_003767E0(void *self, f32 a, f32 b);
void func_00130B80(void *self);
void func_00135B40(void *self, s32 arg);
void func_00136E00(void *self);
void func_0013C890(void *self);
void func_0019B380(void *self);
void func_001D3CE0(void *self);
void func_001D3D30(s32 fps);
void func_001D3FF0(Game *game);
void func_001D4020(Game *game);
void func_001D45B0(Game *game);
void func_0021B770(void *self);
void func_00228160(void *self);
void func_00237200(void);
void func_002372F0(void);
void func_00271730(void *self);
void func_00271A50(s32 *self);
void func_002B6C30(void);
void func_002E7BA0(void *self, s32 arg);
void func_002F03A0(s32 *self);
void func_0030DF10(void *self);
void *func_0030ED60(void *self);
void func_0030ED80(void *self, Game *game, void *arg, void *arg2);
void func_00371B90(void *self);
void func_003753D0(void);
void func_003D14F0(void *self);
void func_003FDF90(void *self);
void func_00439280(void *self);

extern u8 D_01E65489[];
extern u8 D_01E91C3D[];
extern s32 D_004E2A14[];
extern u8 *D_004E25DC;
void func_00130970(void *self);
void func_001309C0(void *self);
void func_00130A00(void *self);
void func_00131B90(Game *game);
void func_00135AA0(void *self);
void func_0019A2E0(void *self);
void func_0019A550(void *self);
u8 func_0019A950(void *self);
void func_0019EA30(void *self);
void func_0019EB60(void *self);
void func_001D3A60(void *self);
void func_001D3D20(void);
void func_001F43B0(void);
void func_00214050(void *self);
void func_002270B0(void *self);
void func_00227360(void *self, s32 arg);
void func_00270E10(void *self);
void func_00270ED0(void *self);
void func_0028AEB0(void *self);
void func_0030D6E0(void *self);
void func_0030D7C0(void *self);
void func_0030EB10(void *self);
void func_00375300(s32 arg);
s32 func_00376570(void *self, s32 minimum, s32 maximum);
void func_003767B0(void *self, s32 rate, s32 blocked);
void func_003767C0(void *self);
void func_00438F30(void *self);

s32 func_00131D20(Game *game)
{
    GameModeDriver *driver;

    if (game->current == 0) {
        return 0;
    }
    driver = game->current->driver;
    if (driver == (GameModeDriver *)D_0051B40C || driver == (GameModeDriver *)D_0051B4BC ||
        driver == (GameModeDriver *)D_0051B714) {
        return 1;
    }
    return 0;
}

s32 func_00131D90(Game *game)
{
    if (game->current != 0) {
        GameModeDriver *driver = game->current->driver;

        if (driver != 0) {
            return driver->unkA0();
        }
        return 0;
    }
    return 0;
}

/* Returns 0; also a shared slot of many 65-slot vtables. */
s32 func_00131DF0(void)
{
    return 0;
}

s32 func_00131E00(Game *game)
{
    if (game->current != 0 && game->current->driver == (GameModeDriver *)game->unk2D2B0) {
        return 1;
    }
    return 0;
}

s32 func_00131E40(Game *game)
{
    GameModeDriver *driver;

    if (game->current == 0) {
        return 0;
    }
    driver = game->current->driver;
    if (driver == 0) {
        return 0;
    }
    if (driver->unk98() == 6 || driver->unk9C() == 3 || driver->unk9C() == 4 || driver->unk9C() == 5) {
        return 1;
    }
    return 0;
}

void func_00131F10(Game *game, GameModeRef *next)
{
    switch (game->unk2DA90) {
    case 5:
        game->next = next;
        game->unk2DA8C = 6;
        break;
    }
}

/* Returns bool: an s32 return type adds an andi 0xff after the && result. */
bool func_00131F60(Game *game)
{
    return !(game->unk2DA94 && D_004EB1E0[0]);
}

void func_00131F90(Game *game)
{
    if (D_004EB1E0[0]) {
        func_0013C880(game->unk8854);
        func_0021B820(game->unk8860);
        ((GameSubsystem *)D_01EA2970)->unk10();
    }
    func_00199DC0(D_00522660);
    func_0028B160(D_01E75640);
    game->unk2DA88 = (f32)game->unk2DA84 * D_0052267C[0];
    if (game->unk2DA54 == -1 && D_004EB1E1[0]) {
        func_002E7610(game->unk28E00, game->unk2DA88);
        func_002E7610(game->unk299B0, game->unk2DA88);
    }
}

/* Inline helpers: the bool temporaries they leave are what func_00132090 compiles to. */
static inline bool func_00132090_busy(void)
{
    if (D_00665E51[0] || D_00665E50[0]) {
        return true;
    }
    return false;
}

static inline bool func_00132090_blocked(void)
{
    return !(D_0051BAD4[0] && D_004EB1E0[0]);
}

void func_00132090(Game *game)
{
    if (game->unk2DA54 < 0 && !func_00132090_busy()) {
        if (!func_00132090_blocked()) {
            func_001D3C50(D_00522660);
        } else {
            func_001D3C10(D_00522660);
        }
    }
    if (D_004EB1E0[0]) {
        if (game->unk2DA66 != game->unk2DA65) {
            game->unk2DA66 = game->unk2DA65;
            if (game->unk2DA66) {
                func_001F4290(2);
            } else {
                func_001F4290(1);
            }
        }
        if (game->unk2DA58 != -1) {
            game->unk2DA54 = game->unk2DA58;
            func_001D3C10(D_00522660);
            func_0028AE80(D_01E75640);
            func_00213EB0(game->unk7080, 1);
            game->unk2DA58 = -1;
            game->unk2DA5C = -1;
        }
        if (game->unk2DA54 != -1 && game->unk2DA5C != -1) {
            if (game->unk2DA54 == game->unk2DA5C) {
                game->unk2DA54 = -1;
                func_0028AE40(D_01E75640, -1);
                func_00213EB0(game->unk7080, 0);
                func_001D3C50(D_00522660);
            }
            game->unk2DA58 = -1;
            game->unk2DA5C = -1;
        }
    }
}

s32 func_001322B0(Game *game)
{
    bool normal;
    GameModeDriver *driver;

    func_0028B160(D_01E75640);
    if (!game->current->unk0C()) {
        return 0;
    }
    normal = true;
    if (game->current == (GameModeRef *)game->unk2C668) {
        normal = false;
    } else {
        driver = game->current->driver;
        if (driver != 0 &&
            (driver == (GameModeDriver *)game->unk2D6D4 || driver == (GameModeDriver *)game->unk2D748)) {
            normal = false;
        }
    }
    if (normal) {
        if (D_004E2910) {
            func_003767E0(D_0051BA50, 20.0f, 0.0f);
        } else {
            func_003767E0(D_0051BA50, 16.666666f, 0.0f);
        }
    } else {
        if (D_004E2910) {
            func_003767E0(D_0051BA50, 40.0f, 0.0f);
        } else {
            func_003767E0(D_0051BA50, 33.333332f, 0.0f);
        }
    }
    D_0055D7FB[0] = game->current->unk1AC;
    return 1;
}

/* Shutdown, part 2 (after func_00132560). */
void func_00132420(Game *game)
{
    game->loader.Shutdown();
    func_0019EB10(D_00665EC0);
    func_0028AEC0(D_01E75640);
    func_00130980(D_004EB1E0);
    func_0019A1D0(D_00522660);
    func_0013C870(game->unk8854);
    ((GameSubsystem *)D_01EA2970)->unk0C();
    func_00270C90(D_01E32AE0);
    game->unk2DA50 = 0;
    game->current = 0;
    game->next = 0;
    ((GameSubsystem *)game->unk28E00)->unk0C();
    ((GameSubsystem *)game->unk299B0)->unk0C();
    func_00271A10(&game->unk2DA04);
    func_00271A10(&game->unk2DA08);
    func_0021B760(game->unk7000);
    memMgrFreeBlock((Heap *)theMemMgr);
    func_003752E0();
    game->unk2DA90 = 3;
    game->unk2DA8C = 3;
}

/* Shutdown, part 1. */
void func_00132560(Game *game)
{
    func_0015CB10();
    func_0014E600();
    func_00344000();
    func_00270D00(D_01E32AE0);
    func_0028AF10(D_01E75640);
    memMgrRelease((Heap *)theMemMgr, 10, 0);
    game->unk2DA74 = 0;
    game->unk2DA78 = 0;
    func_003752F0();
    game->unk2DA90 = 2;
    game->unk2DA8C = 2;
}

/* The top-level frame/state machine. Mode-specific gameplay remains in each mode's driver. */
void func_00132600(Game *game)
{
    s32 ready;
    s32 rate;
    s32 i;
    GameModeDriver *driver;

    game->unk2A640 = (game->unk2A640 << 16) + (game->unk2A640 >> 16);
    game->unk2A640 += game->unk2A644;
    game->unk2A644 += game->unk2A640;
    func_00130A00(D_004EB1E0);
    if (game->unk2DA95) {
        game->unk2DA94 = 0;
    } else if (game->unk2DA96) {
        game->unk2DA94 = 1;
    }
    game->unk2DA95 = 0;
    game->unk2DA96 = 0;
    if (!func_00132090_blocked() && game->unk2DA90 != game->unk2DA8C) {
        game->unk2DA90 = game->unk2DA8C;
    }
    game->loader.Update();
    memMgrUpdate((Heap *)theMemMgr);
    func_00375300(func_00222C90(D_00665EC0));
    func_001D4020(game);
    *(f32 *)(D_004E25DC + 0x18) += *(f32 *)(game->unk7000 + 0x5C);
    switch (game->unk2DA90) {
    case 1:
        func_001D3D20();
        D_004E2A14[0]++;
        func_001D3A60(game->unk7000 + 0x40);
        func_001D3A60(D_00522660);
        D_01E91C3D[0] = 1;
        game->current = game->next;
        game->next = 0;
        ((GameSubsystem *)D_01EA2970)->unk10();
        func_0030D7C0(D_01E90240);
        game->unk2DA8C = 7;
        break;
    case 6:
        func_001D3D20();
        D_004E2A14[0]++;
        func_001D3A60(game->unk7000 + 0x40);
        func_001D3A60(D_00522660);
        D_01E91C3D[0] = 1;
        game->unk2DA54 = -1;
        game->unk2DA58 = -1;
        game->unk2DA5C = -1;
        if (!D_01E65489[0]) {
            func_0022B8E0(game->unk7080, -1);
            func_00214050(game->unk7080);
        }
        func_0019A2E0(D_00522660);
        game->unk2DA64 = 0;
        game->current->unk18();
        game->current = game->next;
        game->next = 0;
        game->unk2DA66 = 0;
        game->unk2DA65 = 0;
        ((GameSubsystem *)D_01EA2970)->unk10();
        func_0030D7C0(D_01E90240);
        game->unk2DA8C = 7;
        break;
    case 7:
        func_001D3D20();
        D_004E2A14[0]++;
        func_001D3A60(game->unk7000 + 0x40);
        func_001D3A60(D_00522660);
        ready = func_001322B0(game);
        if (!ready) {
            D_01E91C3D[0] = 1;
        }
        ((GameSubsystem *)D_01EA2970)->unk10();
        func_0030D7C0(D_01E90240);
        if (ready) {
            game->unk2DA8C = 4;
            game->unk2DA90 = 4;
        }
        break;
    case 8:
        func_001D3D20();
        D_004E2A14[0]++;
        func_001D3A60(game->unk7000 + 0x40);
        func_001D3A60(D_00522660);
        D_01E91C3D[0] = 1;
        game->unk2DA54 = -1;
        game->unk2DA58 = -1;
        game->unk2DA5C = -1;
        func_0022B8E0(game->unk7080, -1);
        func_0028AEB0(D_01E75640);
        func_0019A2E0(D_00522660);
        game->unk2DA64 = 0;
        game->current->reload = 1;
        game->unk2DA8C = 9;
        game->unk2DA90 = 9;
        break;
    case 9:
        func_001D3D20();
        D_004E2A14[0]++;
        func_001D3A60(game->unk7000 + 0x40);
        func_001D3A60(D_00522660);
        ready = func_001322B0(game);
        if (!ready) {
            D_01E91C3D[0] = 1;
        }
        ((GameSubsystem *)D_01EA2970)->unk10();
        func_0030D7C0(D_01E90240);
        if (ready) {
            game->unk2DA8C = 4;
            game->unk2DA90 = 4;
        }
        break;
    case 10:
        game->unk2DA54 = -1;
        game->unk2DA58 = -1;
        game->unk2DA5C = -1;
        func_0022B8E0(game->unk7080, -1);
        func_0028AEB0(D_01E75640);
        func_0019A2E0(D_00522660);
        game->unk2DA64 = 0;
        game->current->reload = 1;
        game->unk2DA8C = 11;
        game->unk2DA90 = 11;
    case 11:
        func_001D3D20();
        D_004E2A14[0]++;
        func_001D3A60(game->unk7000 + 0x40);
        func_001D3A60(D_00522660);
        ready = func_001322B0(game);
        if (!ready) {
            D_01E91C3D[0] = 1;
        }
        ((GameSubsystem *)D_01EA2970)->unk10();
        func_0030D7C0(D_01E90240);
        if (ready) {
            game->unk2DA8C = 12;
            game->unk2DA90 = 12;
        }
        break;
    case 12:
        game->current->unk1C();
        game->unk2DA8C = 4;
        game->unk2DA90 = 4;
    case 4:
        game->unk2DA64 = func_0019A950(D_00522660);
        if (!game->unk2DA64) {
            func_001D3D20();
            D_004E2A14[0]++;
            func_001D3A60(game->unk7000 + 0x40);
            func_001D3A60(D_00522660);
            D_01E91C3D[0] = 1;
            ((GameSubsystem *)D_01EA2970)->unk10();
            func_0030D7C0(D_01E90240);
            if (!D_01E65489[0]) {
                func_0022B8E0(game->unk7080, -1);
                func_00214050(game->unk7080);
            }
            func_0028B160(D_01E75640);
            func_00270ED0(D_01E32AE0);
            driver = game->current->driver;
            if (driver != 0) {
                driver->unk78();
            }
            func_00270E10(D_01E32AE0);
            func_00131B90(game);
            break;
        }
        if (game->current != 0) {
            driver = game->current->driver;
            if (driver != 0) {
                driver->unk74();
            }
        }
        func_003767C0(game->unk2DA10);
        game->unk2DA8C = 5;
        game->unk2DA90 = 5;
        func_0030EB10(D_01EA2970);
    case 5:
        func_00132090(game);
        rate = game->unk2DA6C;
        if (rate == 3) {
            rate = game->unk2DA68;
        }
        if (game->unk2DA54 == -1 && !func_00132090_blocked()) {
            func_003767B0(game->unk2DA10, rate, 0);
        } else {
            func_003767B0(game->unk2DA10, rate, 1);
        }
        for (i = 0; i < game->unk2DA84; i++) {
            if (game->unk2DA58 != -1) {
                game->unk2DA38 = i;
                break;
            }
            func_001D3D20();
            D_004E2A14[0]++;
            func_001D3A60(game->unk7000 + 0x40);
            func_001D3A60(D_00522660);
            if (!D_01E65489[0]) {
                func_00214050(game->unk7080);
            }
            func_00135AA0(game->unk28DB0);
            func_00135AA0(game->unk28DD4);
            func_001309C0(D_004EB1E0);
            if (game->unk2DA54 == -1 && D_004EB1E0[0]) {
                func_00270ED0(D_01E32AE0);
                if (!func_00132090_blocked()) {
                    func_0019A550(D_00522660);
                }
                game->current->unk10();
                func_00270E10(D_01E32AE0);
                func_00131B90(game);
            }
            if (D_004EB1E0[0]) {
                ((GameSubsystem *)D_01EA2970)->unk10();
            }
            func_0030D7C0(D_01E90240);
            if (game->unk2DA8C != 5) {
                break;
            }
        }
        func_00131F90(game);
        func_001F43B0();
        if (game->unk2DA66) {
            game->unk2DA84 = func_00376570(game->unk2DA10, 2, 8);
        } else {
            game->unk2DA84 = func_00376570(game->unk2DA10, 1, 4);
        }
        func_0019EB60(D_00665EC0);
        game->current->unk14();
        break;
    }
    func_00227360(D_00665EC0, 0);
    func_0030D6E0(D_01E90240);
    func_00438F30(D_01ECDE90);
    func_00130970(D_004EB1E0);
    func_0019EA30(D_00665EC0);
    func_002270B0(D_00665EC0);
    game->unk299A6 = 0;
    game->unk2A556 = 0;
}

void func_00133180(void)
{
}

/* Start-up load, one stage per call: main calls it every frame until it returns 1. */
s32 func_00133190(Game *game)
{
    u16 thousands;
    u16 point;
    void *file;
    const char *globalName;
    const char *headName;
    s32 i;

    func_001D4020(game);
    game->loader.Update();
    switch (game->unk2DA60) {
    case 1:
    case 24:
        if (!func_001D41E0(game)) {
            return 0;
        }
        func_0030A8E0(game->unk2A560);
        game->unk2C662 = 1;
        game->unk2DA60 = 2;
    case 2:
        game->unk2A648 = memMgrTake((Heap *)theMemMgr, 0xE, 0);
        if (game->unk2A648 == 0) {
            return 0;
        }
        D_004F6100.QueueLoadRequest(D_004B5C70, &game->unk2A64C, game->unk2A648, memMgrSize((Heap *)theMemMgr, 0xE));
        game->unk2DA60 = 3;
    case 3:
        if (!game->unk2A64C) {
            return 0;
        }
        file = game->unk2A648;
        func_0021AAF0(file);
        for (i = 0; i < ((s32 *)file)[2]; i++) {
            func_002514C0(func_00131AA0(file, i));
        }
        game->unk2DA74 = memMgrTake((Heap *)theMemMgr, 0xA, 0);
        game->unk2DA78 = memMgrTake((Heap *)theMemMgr, 0xB, 0);
        globalName = D_004B5BA8[0];
        headName = D_004B5C20[0];
        if (game->unk2DA74 == 0 || game->unk2DA78 == 0) {
            return 0;
        }
        D_004E2918 = 1;
        switch (D_004E2918) {
        case 4:
            thousands = ' ';
            point = '.';
            break;
        case 5:
        case 6:
        case 10:
            thousands = '.';
            point = ',';
            break;
        default:
            thousands = ',';
            point = '.';
            break;
        }
        ustrSetSeparators(thousands, point);
        game->loader.QueueLoadRequest(globalName, &game->unk2A654, game->unk2DA74,
                                      memMgrSize((Heap *)theMemMgr, 0xA));
        game->loader.QueueLoadRequest(headName, &game->unk2A655, game->unk2DA78,
                                      memMgrSize((Heap *)theMemMgr, 0xB));
        game->unk2DA60 = 4;
    case 4:
        if (!game->unk2A654 || !game->unk2A655) {
            return 0;
        }
        func_00271A20(&game->unk2DA04, game->unk2DA74, D_004B5C80, 0xFA5);
        func_00271A20(&game->unk2DA08, game->unk2DA78, D_004B5CB0, 0xA);
        game->loader.QueueLoadRequest(D_004B5CD8, &game->unk2C660, game->unk2A660, 0x1000);
        game->unk2DA60 = 5;
    case 5:
        if (!game->unk2C660) {
            return 0;
        }
        game->loader.QueueLoadRequest(D_004B5D40, &game->unk2C661, game->unk2B660, 0x1000);
        game->unk2DA60 = 6;
    case 6:
        if (!game->unk2C661) {
            return 0;
        }
        game->unk2DA7C = memMgrTake((Heap *)theMemMgr, 0xC, 0);
        if (game->unk2DA7C == 0) {
            return 0;
        }
        game->loader.QueueLoadRequest(D_004B5D60, &game->unk2A656, game->unk2DA7C, memMgrSize((Heap *)theMemMgr, 0xC));
        game->unk2DA60 = 7;
    case 7:
        if (!game->unk2A656) {
            return 0;
        }
        game->unk2A650 = (GameDataHeader *)game->unk2DA7C;
        func_00367E70(game->unk2A650, game->unk2A650);
        if (game->unk2A650->unk4[1] >= 7) {
            game->unk2A650->unk4[1]--;
        }
        func_003EA580();
        func_003E9580();
        func_003EB0A0();
        game->unk2DA60 = 8;
    case 8:
        if (!func_00228070(D_00665EC0)) {
            return 0;
        }
        if (!func_001364B0(D_0051BB00)) {
            return 0;
        }
        if (!func_003D11D0(D_01EA7810)) {
            return 0;
        }
        game->unk2DA60 = 9;
    case 9:
        if (D_01E7751C[0] <= 0) {
            func_0028B580(D_01E75640);
        }
        if (!func_0028B380(D_01E75640)) {
            return 0;
        }
        game->unk2DA60 = 10;
    case 10:
        game->unk2DA60 = 11;
    case 11:
        game->unk2A657 = 0;
        memset(game, 0, 0x3000);
        game->loader.QueueLoadRequest(D_004B5CF0, &game->unk2A657, game, 0x3000);
        game->unk2DA60 = 12;
    case 12:
        if (!game->unk2A657) {
            return 0;
        }
        game->unk2DA60 = 13;
    case 13:
        if (!func_00371940(D_01EA4070)) {
            return 0;
        }
        game->unk2DA60 = 14;
    case 14:
        game->unk2A658 = 0;
        memset(game->pad0 + 0x3000, 0, 0x4000);
        game->loader.QueueLoadRequest(D_004B5D10, &game->unk2A658, game->pad0 + 0x3000, 0x4000);
        game->unk2DA60 = 15;
    case 15:
        if (!game->unk2A658) {
            return 0;
        }
        game->unk2DA60 = 16;
    default:
        if (D_004E2910) {
            func_003753B0(0, func_00222C90(D_00665EC0));
        } else {
            func_003753B0(1, func_00222C90(D_00665EC0));
        }
        func_001D3C90(game->unk7000 + 0x40);
        func_001D3C50(game->unk7000 + 0x40);
        func_00130B00(D_004EB1E0);
        func_0030DE10(D_01E90240);
        func_00439110(D_01ECDE90);
        func_00135AF0(game->unk28DB0);
        ustrncpy((u16 *)(game->unk28DB0 + 4), 9, func_00271A00(&D_0051BA44, 0x1B9));
        func_00135AF0(game->unk28DD4);
        ustrncpy((u16 *)(game->unk28DD4 + 4), 9, func_00271A00(&D_0051BA44, 0x1BA));
        game->unk2DA58 = -1;
        game->unk2DA5C = -1;
        game->unk2DA54 = -1;
        func_0022B8E0(game->unk7080, -1);
        game->unk2DA64 = 0;
        game->unk2DA65 = 0;
        game->unk2DA66 = 0;
        game->unk2DA68 = 1;
        game->unk2DA6C = 3;
        func_0021B830(game->unk8860, D_004B5D28, game->unk8080, D_01E3C63C);
        func_00271A80(D_01E754E0);
        func_0015D0A0();
        func_0014E670();
        func_00344310();
        game->unk2DA84 = 1;
        func_0021B5E0(game->unk7000);
        func_0021B4E0(game->unk7000);
        game->next = (GameModeRef *)game->unk2C668;
        game->unk2DA60 = 23;
        game->unk2DA90 = 1;
        game->unk2DA8C = 1;
        return 1;
    }
}

/* Init: main's first call. */
void func_00133BB0(Game *game)
{
    game->unk2DA81 = 0;
    game->unk2DA80 = 0;
    game->unk2DA94 = 1;
    game->unk2DA95 = 0;
    game->unk2DA96 = 0;
    game->unk2DA00 = 1;
    game->unk2A64C = 0;
    game->unk2A648 = 0;
    game->unk2A654 = 0;
    game->unk2A655 = 0;
    game->unk2A664 = 0;
    game->unk2C660 = 0;
    game->unk2B664 = 0;
    game->unk2C661 = 0;
    game->unk2DA82 = 0;
    game->unk2A644 = 0x2B9D6F8;
    game->unk2A640 = 0xFD462907;
    func_001D45B0(game);
    func_001D3FF0(game);
    func_001D4020(game);
    func_002372F0();
    func_00237200();
    memMgrInit((Heap *)theMemMgr);
    func_0021B770(game->unk7000);
    func_003753D0();
    if (D_004E2910) {
        func_001D3D30(50);
        func_003767E0(game->unk2DA10, 20.0f, 0.0f);
    } else {
        func_001D3D30(60);
        func_003767E0(game->unk2DA10, 16.666666f, 0.0f);
    }
    func_001D3CE0(game->unk7000 + 0x40);
    func_002B6C30();
    func_002F03A0(D_01E86560); /* then clear its words; this store order matches the original */
    D_01E86560[0] = 0;
    D_01E86570[0] = 0;
    D_01E86564[0] = 0;
    D_01E86584[0] = 0;
    D_01E86568[0] = 0;
    D_01E8656C[0] = 0;
    D_01E86574[0] = 0;
    D_01E86578[0] = 0;
    D_01E8657C[0] = 0;
    D_01E86580[0] = 0;
    D_01E86588[0] = 0;
    func_00371B90(D_01EA4070);
    func_00271A50(&game->unk2DA04);
    func_00271A50(&game->unk2DA08);
    game->loader.Init();
    func_00228160(D_00665EC0);
    func_00130B80(D_004EB1E0);
    func_00271730(D_01E32AE0);
    func_0019B380(D_00522660);
    func_0013C890(game->unk8854);
    func_0030DF10(D_01E90240);
    func_00439280(D_01ECDE90);
    func_0030ED80(D_01EA2970, game, func_0030ED60(D_01EA2970), D_004F5080);
    func_00136E00(D_0051BB00);
    func_003D14F0(D_01EA7810);
    func_003FDF90(D_01ECD520);
    vdbSetDatabase(game->unk8860, 0);
    game->unk2DA50 = 0;
    game->current = 0;
    game->next = 0;
    game->unk2A648 = 0;
    ((GameModeRef *)game->unk2C668)->unk08();
    ((GameModeRef *)game->unk2C9A0)->unk08();
    ((GameModeRef *)game->unk2CCC0)->unk08();
    ((GameModeRef *)game->unk2CE80)->unk08();
    ((GameModeRef *)game->unk2D048)->unk08();
    ((GameSubsystem *)game->unk2D218)->unk08();
    ((GameSubsystem *)game->unk2D264)->unk08();
    ((GameSubsystem *)game->unk2D2B0)->unk08();
    ((GameSubsystem *)game->unk2D300)->unk08();
    ((GameSubsystem *)game->unk2D374)->unk08();
    ((GameSubsystem *)game->unk2D3CC)->unk08();
    ((GameSubsystem *)game->unk2D47C)->unk08();
    ((GameSubsystem *)game->unk2D424)->unk08();
    ((GameSubsystem *)game->unk2D4D4)->unk08();
    ((GameSubsystem *)game->unk2D52C)->unk08();
    ((GameSubsystem *)game->unk2D578)->unk08();
    ((GameSubsystem *)game->unk2D5EC)->unk08();
    ((GameSubsystem *)game->unk2D650)->unk08();
    ((GameSubsystem *)game->unk2D6D4)->unk08();
    ((GameSubsystem *)game->unk2D748)->unk08();
    ((GameSubsystem *)game->unk2D7BC)->unk08();
    ((GameSubsystem *)game->unk2D8E0)->unk08();
    ((GameSubsystem *)game->unk2D8E8)->unk08();
    ((GameSubsystem *)game->unk2D948)->unk08();
    func_00135B40(game->unk28DB0, 0);
    func_00135B40(game->unk28DD4, 1);
    func_002E7BA0(game->unk28E00, 0);
    func_002E7BA0(game->unk299B0, 1);
    game->unk2DA58 = -1;
    game->unk2DA5C = -1;
    game->unk2DA54 = -1;
    game->unk2DA64 = 0;
    game->unk2C662 = 0;
    game->unk2DA60 = 1;
    game->unk2DA70 = 1;
    game->unk2DA74 = 0;
    game->unk2DA78 = 0;
    game->unk2DA90 = 0;
    game->unk2DA8C = 0;
}

}
