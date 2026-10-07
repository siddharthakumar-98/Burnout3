/* d4/ustrfmt (0x212E30-0x2130F0): %1..%9 argument substitution into 16-bit strings. */

#include "types.h"

int ustrFmtScan(u16 **dst, u16 **src);

static inline void copyArg(u16 *d, const u16 *s)
{
    *d = *s;
    while (*s != 0) {
        s++;
        d++;
        *d = *s;
    }
}

u16 *ustrFormat(u16 *dst, u16 *fmt, u16 *arg1, u16 *arg2, u16 *arg3, u16 *arg4)
{
    int n;

    for (;;) {
        switch (ustrFmtScan(&dst, &fmt)) {
        case 0:
            return dst;
        case 1:
            if (arg1 == 0) {
                *dst = 0;
                return dst;
            }
            copyArg(dst, arg1);
            break;
        case 2:
            if (arg2 == 0) {
                *dst = 0;
                return dst;
            }
            copyArg(dst, arg2);
            break;
        case 3:
            if (arg3 == 0) {
                *dst = 0;
                return dst;
            }
            copyArg(dst, arg3);
            break;
        case 4:
            if (arg4 == 0) {
                *dst = 0;
                return dst;
            }
            copyArg(dst, arg4);
            break;
        default:
            *dst = 0;
            return dst;
        }
        for (n = 0; dst[n] != 0; n++) {
        }
        dst += n;
    }
}

/* Copies src to *dst up to the next "%<1-9>" (returns the digit) or the terminator (writes 0, returns 0); "%%"
 * copies one '%'. Both pointers are left after what was consumed. */
int ustrFmtScan(u16 **dst, u16 **src)
{
    u16 *d = *dst;
    u16 *s = *src;
    u16 c;

    while (*s != 0) {
        if (*s == '%') {
            s++;
            c = *s;
            if (c == '%') {
                *d = c;
                s++;
                d++;
            }
            if (c >= '1' && c <= '9') {
                *dst = d;
                *src = s + 1;
                return c - '0';
            }
        } else {
            *d = *s;
            s++;
            d++;
        }
    }
    *d = 0;
    *dst = d;
    *src = s;
    return 0;
}
