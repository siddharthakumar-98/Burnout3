#ifndef OPTIONS_H
#define OPTIONS_H

#include "types.h"

/* Game options (d4/options, 0x21B5E0-0x21B8C0). Field names are ours; offsets from the reset and apply code. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Options {
    u8 unk0[0x2C];  /* 0x00 reset to 0x0F */
    s8 unk2C[4];    /* 0x2C reset to 1; the first two are copied into the object at 0x4EE040 (+0x8025) */
    s8 unk30;       /* 0x30 times 0.0075 into 0x665EF0 */
    s8 unk31;       /* 0x31 times -0.0075 into 0x665EF4 */
    s8 unk32;       /* 0x32 */
    s8 unk33;       /* 0x33 */
    s8 volume34;    /* 0x34 0..100 (reset 90), times 0.01 into func_0028AC80 */
    s8 volume35;    /* 0x35 0..100 (reset 90), times 0.01 into func_003F8890 */
    s8 unk36;       /* 0x36 reset 90 */
    s8 unk37;       /* 0x37 */
    s8 unk38;       /* 0x38 */
    s8 unk39;       /* 0x39 func_003F8870 switch */
    s8 unk3A;       /* 0x3A */
    s8 unk3B;       /* 0x3B */
    s8 unk3C;       /* 0x3C */
    s8 unk3D;       /* 0x3D picks 2 or 5 for 0x5179DC */
    s8 unk3E;       /* 0x3E picks 2 or 5 for 0x51858C */
    s8 unk3F;       /* 0x3F */
} Options;

void func_0021B5E0(Options *opts);
void func_0021B760(void);
void func_0021B770(Options *opts);
void func_0021B820(void);
int func_0021B830(void *db, const char *path, void *fs);

#ifdef __cplusplus
}
#endif

#endif
