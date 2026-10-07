#ifndef USTRING_H
#define USTRING_H

#include "types.h"

/* 16-bit (UCS-2) strings: the game's text is UTF-16 in memory and UTF-8 in some data (d4/ustring, 0x2130F0).
 * Names are ours; the game's are unknown. The copy and concatenate functions return a pointer to the terminator
 * they wrote, and the size limits count characters including it. */

#ifdef __cplusplus
extern "C" {
#endif

/* Number formatting: thousands separator (0 for none) and decimal point, ',' and '.' by default. */
void ustrSetSeparators(u16 thousands, u16 point);
u16 *ustrFromFloat(float value, u16 *out, int decimals, int plus);
u16 *ustrFromFloatNoSep(float value, u16 *out, int decimals, int plus);
u16 *ustrFromInt(u16 *out, int value, int minDigits, int plus);
u16 *ustrFromIntNoSep(u16 *out, int value, int minDigits, int plus);
u16 *ustrFromUInt(u16 *out, u32 value, int minDigits);

u16 *ustrncat(u16 *dst, int size, const u16 *src);
u16 *ustrcat(u16 *dst, const u16 *src);
u16 *ustrncpy(u16 *dst, int size, const u16 *src);
u16 *ustrcpy(u16 *dst, const u16 *src);
int ustrlen(const u16 *s);

/* UTF-8 <-> UTF-16, after Unicode, Inc.'s ConvertUTF.c. ustrFromUtf8Checked returns 0 (ok), 1 (input ends inside a
 * character), 2 (output full) or 3 (character outside the BMP, or a surrogate). */
int ustrFromUtf8Checked(const u8 *src, u16 *dst, int size);
void ustrFromUtf8(const u8 *src, u16 *dst, int size);
void ustrToUtf8(const u16 *src, char *dst, int size);

#ifdef __cplusplus
}
#endif

#endif
