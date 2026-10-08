/* d4/options (0x21B5E0-0x21B8C0): applying and resetting the game options, and loading the tuning database file.
 * C++ for the file's virtual calls (they load the slot into $t9); the functions keep their func_ names (extern "C"). */

#include "options.h"
#include "vdb.h"

/* A file opened through the file system (game/unit_00212580). The vtable's first two words are MW's header. */
class FsFile {
public:
    virtual void unk08();                     /* vtable +0x08 */
    virtual void close();                     /* vtable +0x0C */
    virtual s32 read(void *buf, s32 size);    /* vtable +0x10 */

    /* 0x00 vtable */
    s32 unk4; /* 0x04 */
    s32 size; /* 0x08 */
};

extern "C" {

void *memset(void *dst, int c, u32 n);
FsFile *fsDeviceOpen(void *fs, const char *path, int mode);
void func_0028AC80(void *obj, float volume);
void func_003F8890(void *obj, float volume);
void func_003F8870(void *obj, int on);
void func_00131B30(void *obj);

/* Reached with lui/lo, not $gp: likely members of larger objects; incomplete arrays keep them out of small data. */
extern s32 D_005179DC[];
extern s32 D_0051858C[];
extern float D_00665EF0[];
extern float D_00665EF4[];
extern u8 D_01E75640[];
extern u8 D_01E7A888[];
extern u8 D_004EE040[];

/* .sbss, 0x4E2910-0x4E291C */
u8 D_004E2910;
u8 D_004E2914;
s32 D_004E2918;

/* 92.25%: the flag copy loop builds the 0/1 as `movz` from a hoisted 1 then `xori 1; sltiu 1` (== 1); `!= 0`
 * gives `sltu`. `bool == true` gives the xori/sltiu tail, but s8->bool always becomes `sltu` (or a branch for
 * `on = 1; if (!x) on = 0;`, 89.45%); every ternary/if/inline form folds to `sltu`. */
void func_0021B5E0(Options *opts)
{
    float y;
    float x;
    s8 i;

    if (opts->unk3D) {
        D_005179DC[0] = 2;
    } else {
        D_005179DC[0] = 5;
    }
    if (opts->unk3E) {
        D_0051858C[0] = 2;
    } else {
        D_0051858C[0] = 5;
    }
    x = 0.0075f * opts->unk30;
    y = -0.0075f * opts->unk31;
    D_00665EF0[0] = x;
    D_00665EF4[0] = y;
    func_0028AC80(D_01E75640, 0.01f * opts->volume34);
    func_003F8890(D_01E7A888, 0.01f * opts->volume35);
    if (opts->unk39) {
        func_003F8870(D_01E7A888, 1);
    } else {
        func_003F8870(D_01E7A888, 0);
    }
    for (i = 0; i < 2; i++) {
        D_004EE040[0x8025 + i] = opts->unk2C[i] != 0;
    }
}

void func_0021B760(void)
{
}

void func_0021B770(Options *opts)
{
    int i;

    D_004E2910 = 0;
    D_004E2914 = 0;
    opts->unk30 = 0;
    opts->unk31 = 0;
    opts->unk33 = 0;
    opts->volume34 = 90;
    opts->volume35 = 90;
    opts->unk36 = 90;
    opts->unk37 = 0;
    opts->unk38 = 0;
    opts->unk39 = 1;
    opts->unk3A = 1;
    opts->unk3B = 1;
    opts->unk3C = 0;
    opts->unk3D = 1;
    opts->unk3E = 1;
    opts->unk3F = 0;
    D_004E2918 = 1;
    func_00131B30(D_004EE040);
    memset(opts, 0xF, 0x2C);
    for (i = 0; i < 4; i++) {
        opts->unk2C[i] = 1;
    }
}

void func_0021B820(void)
{
}

/* Reads the tuning database file into db + 0x10 (a VdbDatabase) and applies it. */
int func_0021B830(void *db, const char *path, void *fs)
{
    FsFile *f;

    if (path != 0) {
        f = fsDeviceOpen(fs, path, 1);
        if (f != 0) {
            f->read((u8 *)db + 0xE540, (f->size + 0x7FF) & ~0x7FF);
            vdbDbLoad((VdbDatabase *)((u8 *)db + 0x10));
            f->close();
        }
    }
    return 1;
}

}
