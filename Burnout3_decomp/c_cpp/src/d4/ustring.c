/* d4/ustring (0x2130F0-0x213E10): 16-bit strings, number formatting into them, and UTF-8 conversion.
 * The conversion tables and code follow Unicode, Inc.'s reference ConvertUTF.c. */

#include "ustring.h"

unsigned int strlen(const char *s);

/* .rodata, 0x4B9700-0x4B9850 */
static const u8 trailingBytesForUTF8[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
};
static const u32 offsetsFromUTF8[6] = {
    0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080,
};
static const u8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const float powersOf10[10] = { 1.0f, 10.0f, 100.0f, 1e3f, 1e4f, 1e5f, 1e6f, 1e7f, 1e8f, 1e9f };

/* .sdata, 0x4E1934-0x4E1944 */
static u16 thousandsSep[2] = { ',', 0 };
static u16 decimalPoint[2] = { '.', 0 };
static u16 plusSign[2] = { '+', 0 };
static u16 minusSign[2] = { '-', 0 };

void ustrSetSeparators(u16 thousands, u16 point)
{
    thousandsSep[0] = thousands;
    decimalPoint[0] = point;
}

/* Inline digit writers for the float formatters (ustrFromUInt with and without thousands grouping). */
static inline u16 *uintToStrSep(u16 *out, u32 n, int minDigits)
{
    u16 buf[16];
    int group, i, k;

    group = 0;
    for (i = 0, k = 0; i < 12; i++, k++) {
        u32 q = n / 10;
        buf[k] = n - q * 10 + '0';
        if (q == 0) {
            i++;
            k++;
            break;
        }
        if (thousandsSep[0] != 0 && ++group == 3) {
            k++;
            group = 0;
            buf[k] = thousandsSep[0];
        }
        n = q;
    }
    while (i < minDigits) {
        minDigits--;
        *out++ = '0';
    }
    while (k > 0) {
        k--;
        *out++ = buf[k];
    }
    *out = 0;
    return out;
}

static inline u16 *uintToStr(u16 *out, u32 n, int minDigits)
{
    u16 buf[16];
    int i, k;

    for (i = 0, k = 0; i < 12; i++, k++) {
        u32 q = n / 10;
        buf[k] = n - q * 10 + '0';
        if (q == 0) {
            i++;
            k++;
            break;
        }
        n = q;
    }
    while (i < minDigits) {
        minDigits--;
        *out++ = '0';
    }
    while (k > 0) {
        k--;
        *out++ = buf[k];
    }
    *out = 0;
    return out;
}

/* 97.20%: integer-part loop registers off by one (minDigits/group get t2/t1, original t1/t7, temps one higher),
 * the two digit buffers swap stack slots (original: integer at sp+0, fraction at sp+32), and the shared epilogue
 * differs (original ends `beq; addiu sp` / `jr ra; nop`). */
u16 *ustrFromFloat(float value, u16 *out, int decimals, int plus)
{
    u16 point = decimalPoint[0];
    u16 sep = thousandsSep[0];
    u16 saved;
    int whole;
    u32 n;

    if (value < 0.0f) {
        value = -value;
        *out++ = minusSign[0];
    } else if (plus == 1) {
        *out++ = plusSign[0];
    }
    whole = (int)value;
    saved = thousandsSep[0];
    thousandsSep[0] = sep;
    n = whole;
    if (whole < 0) {
        n = -whole;
        *out++ = minusSign[0];
    }
    out = uintToStrSep(out, n, 0);
    thousandsSep[0] = saved;
    if (decimals == 0) {
        return out;
    }
    *out = point;
    return uintToStr(out + 1, (int)((value - (float)whole) * powersOf10[decimals]), decimals);
}

/* 97.19%: same residuals as ustrFromFloat (register numbering in the integer loop, buffer slots, epilogue). */
u16 *ustrFromFloatNoSep(float value, u16 *out, int decimals, int plus)
{
    u16 point = decimalPoint[0];
    u16 saved;
    int whole;
    u32 n;

    if (value < 0.0f) {
        value = -value;
        *out++ = minusSign[0];
    } else if (plus == 1) {
        *out++ = plusSign[0];
    }
    whole = (int)value;
    saved = thousandsSep[0];
    thousandsSep[0] = 0;
    n = whole;
    if (whole < 0) {
        n = -whole;
        *out++ = minusSign[0];
    }
    out = uintToStrSep(out, n, 0);
    thousandsSep[0] = saved;
    if (decimals == 0) {
        return out;
    }
    *out = point;
    return uintToStr(out + 1, (int)((value - (float)whole) * powersOf10[decimals]), decimals);
}

u16 *ustrFromInt(u16 *out, int value, int minDigits, int plus)
{
    u16 buf[16];
    u32 n, q;
    int i, k, group;

    if (value < 0) {
        value = -value;
        *out++ = minusSign[0];
    } else if (plus == 1) {
        *out++ = plusSign[0];
    }
    n = value;
    group = 0;
    for (i = 0, k = 0; i < 12; i++, k++) {
        q = n / 10;
        buf[k] = n - q * 10 + '0';
        if (q == 0) {
            i++;
            k++;
            break;
        }
        if (thousandsSep[0] != 0 && ++group == 3) {
            k++;
            group = 0;
            buf[k] = thousandsSep[0];
        }
        n = q;
    }
    while (i < minDigits) {
        minDigits--;
        *out++ = '0';
    }
    while (k > 0) {
        k--;
        *out++ = buf[k];
    }
    *out = 0;
    return out;
}

u16 *ustrFromIntNoSep(u16 *out, int value, int minDigits, int plus)
{
    u16 buf[16];
    u32 n;
    int i, k;

    if (value < 0) {
        value = -value;
        *out++ = minusSign[0];
    } else if (plus == 1) {
        *out++ = plusSign[0];
    }
    n = value;
    for (i = 0, k = 0; i < 12; i++, k++) {
        u32 q = n / 10;
        buf[k] = n - q * 10 + '0';
        if (q == 0) {
            i++;
            k++;
            break;
        }
        n = q;
    }
    while (i < minDigits) {
        minDigits--;
        *out++ = '0';
    }
    while (k > 0) {
        k--;
        *out++ = buf[k];
    }
    *out = 0;
    return out;
}

u16 *ustrFromUInt(u16 *out, u32 n, int minDigits)
{
    u16 buf[16];
    int i, k;

    for (i = 0, k = 0; i < 12; i++, k++) {
        u32 q = n / 10;
        buf[k] = n - q * 10 + '0';
        if (q == 0) {
            i++;
            k++;
            break;
        }
        n = q;
    }
    while (i < minDigits) {
        minDigits--;
        *out++ = '0';
    }
    while (k > 0) {
        k--;
        *out++ = buf[k];
    }
    *out = 0;
    return out;
}

u16 *ustrncat(u16 *dst, int size, const u16 *src)
{
    u16 *last = dst + (size - 1);

    while (*dst != 0) {
        dst++;
    }
    *dst = *src;
    while (dst < last) {
        if (*src == 0) {
            goto done;
        }
        src++;
        dst++;
        *dst = *src;
    }
    *last = 0;
    return last;
done:
    return dst;
}

u16 *ustrcat(u16 *dst, const u16 *src)
{
    while (*dst != 0) {
        dst++;
    }
    *dst = *src;
    while (*src != 0) {
        src++;
        dst++;
        *dst = *src;
    }
    return dst;
}

u16 *ustrncpy(u16 *dst, int size, const u16 *src)
{
    u16 *last = dst + (size - 1);

    *dst = *src;
    while (dst < last) {
        if (*src == 0) {
            goto done;
        }
        src++;
        dst++;
        *dst = *src;
    }
    *last = 0;
    return last;
done:
    return dst;
}

u16 *ustrcpy(u16 *dst, const u16 *src)
{
    *dst = *src;
    while (*src != 0) {
        src++;
        dst++;
        *dst = *src;
    }
    return dst;
}

int ustrlen(const u16 *s)
{
    int n = 0;

    while (s[n] != 0) {
        n++;
    }
    return n;
}

int ustrFromUtf8Checked(const u8 *src, u16 *dst, int size)
{
    int result = 0;
    const u8 *end = src + strlen((const char *)src);

    while (*src != 0) {
        u32 ch = 0;
        u8 extra = trailingBytesForUTF8[*src];

        if (src + extra >= end) {
            result = 1;
            break;
        }
        switch (extra) {
        case 3:
            ch += *src++;
            ch <<= 6;
        case 2:
            ch += *src++;
            ch <<= 6;
        case 1:
            ch += *src++;
            ch <<= 6;
        case 0:
            ch += *src++;
        }
        ch -= offsetsFromUTF8[extra];
        if (size <= 1) {
            result = 2;
            break;
        }
        if (ch <= 0xFFFF) {
            if (ch >= 0xD800 && ch <= 0xDFFF) {
                result = 3;
                break;
            }
            *dst++ = ch;
            size--;
        } else {
            result = 3;
            break;
        }
    }
    *dst = 0;
    return result;
}

void ustrFromUtf8(const u8 *src, u16 *dst, int size)
{
    while (*src != 0 && size > 1) {
        u32 ch = 0;
        u8 extra = trailingBytesForUTF8[*src];

        switch (extra) {
        case 3:
            ch += *src++;
            ch <<= 6;
        case 2:
            ch += *src++;
            ch <<= 6;
        case 1:
            ch += *src++;
            ch <<= 6;
        case 0:
            ch += *src++;
        }
        size--;
        *dst++ = ch - offsetsFromUTF8[extra];
    }
    *dst = 0;
}

void ustrToUtf8(const u16 *src, char *dst, int size)
{
    while (*src != 0 && size > 1) {
        u32 ch = *src++;
        u8 bytes = 0;

        if (ch < 0x80) {
            bytes = 1;
        } else if (ch < 0x800) {
            bytes = 2;
        } else if (ch < 0x10000) {
            bytes = 3;
        } else if (ch < 0x200000) {
            bytes = 4;
        }
        if (bytes >= size) {
            return;
        }
        dst += bytes;
        switch (bytes) {
        case 4:
            *--dst = (ch | 0x80) & 0xBF;
            ch >>= 6;
        case 3:
            *--dst = (ch | 0x80) & 0xBF;
            ch >>= 6;
        case 2:
            *--dst = (ch | 0x80) & 0xBF;
            ch >>= 6;
        case 1:
            *--dst = ch | firstByteMark[bytes];
        }
        dst += bytes;
        size -= bytes;
    }
    *dst = 0;
}
