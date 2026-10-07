/* game/unit_00212580: the file system. Devices hold open files; the RenderWare file interface
 * (RwFileFunctions) is replaced by the rwf* functions here (installed by fsInstallRwFileInterface).
 * C++ for the virtual calls (they load the slot into $t9); the functions keep their func_ names (extern "C"). */

#include "fs.h"

extern "C" {

u32 strlen(const char *s);
char *func_00127BF8(const char *s, const char *sub); /* strstr */

/* fopen's mode strings stay in assembly for now: the original spaces them 8 bytes apart (CodeWarrior gives each a
 * 4-aligned 2-byte section), and "r" sits in the slice before this unit's (docs/d4.md). */
extern const char D_004B96D8[]; /* "r" */
extern const char D_004B96E0[]; /* "w" */
extern const char D_004B96E8[]; /* "a" */
extern const char D_004B96F0[]; /* "+" */
extern const char D_004B96F8[]; /* "s" */

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, u32 n);
s32 func_00127520(const char *a, const char *b, u32 n); /* strncmp */
void *func_001D8B88(void);                             /* RwOsGetFileInterface */

/* .sbss */
extern u32 fsNumDevices;         /* number of registered devices */
extern s32 fsRwInstalled;         /* rwf* functions installed */
extern FsDevice *fsDefaultDevice;   /* default device */
extern FsDevice *fsDevices[2]; /* registered devices */

/* .bss: RenderWare's original file functions, saved by fsInstallRwFileInterface (likely one RwFileFunctions in the original;
 * splat names each word, and incomplete arrays keep them out of small data so the relocations match). */
extern void *D_00676440[];
extern void *D_00676444[];
extern void *D_00676448[];
extern void *D_0067644C[];
extern void *D_00676450[];
extern void *D_00676454[];
extern void *D_00676458[];
extern void *D_0067645C[];
extern void *D_00676460[];
extern void *D_00676464[];
extern void *D_00676468[];
extern char D_00676470[2][5]; /* registered device prefixes */

/* rwftell */
s32 fsRwTell(FsFile *f)
{
    return f->pos;
}

/* rwfexist */
s32 fsRwExists(const char *path)
{
    FsDevice *dev = fsFindDevice(path);
    if (dev == 0) {
        return 0;
    }
    return dev->exists(path);
}

/* rwfflush */
s32 fsRwFlush(FsFile *f)
{
    return 0;
}

/* rwfseek */
s32 fsRwSeek(FsFile *f, s32 offset, s32 whence)
{
    switch (whence) {
    case 1:
        f->seek(offset, 1);
        break;
    case 2:
        f->seek(offset, 2);
        break;
    case 0:
        f->seek(offset, 0);
        break;
    default:
        return -1;
    }
    return 0;
}

/* rwfeof */
s32 fsRwEof(FsFile *f)
{
    return f->pos >= f->size;
}

/* rwfputs */
s32 fsRwPuts(const char *buf, FsFile *f)
{
    return f->write(buf, strlen(buf));
}

/* rwfgets */
char *fsRwGets(char *buf, s32 maxLen, FsFile *f)
{
    return 0;
}

/* rwfwrite */
u32 fsRwWrite(const void *buf, u32 size, u32 count, FsFile *f)
{
    return f->write(buf, size * count) / size;
}

/* rwfread */
u32 fsRwRead(void *buf, u32 size, u32 count, FsFile *f)
{
    return f->read(buf, size * count) / size;
}

/* rwfclose */
s32 fsRwClose(FsFile *f)
{
    f->close();
    return 0;
}

/* rwfopen */
FsFile *fsRwOpen(const char *path, const char *mode)
{
    s32 flags = 0;
    FsDevice *dev;

    if (func_00127BF8(mode, D_004B96D8)) {
        flags |= 1;
    }
    if (func_00127BF8(mode, D_004B96E0)) {
        flags |= 6;
    }
    if (func_00127BF8(mode, D_004B96E8)) {
        flags |= 10;
    }
    if (func_00127BF8(mode, D_004B96F0)) {
        flags |= 3;
    }
    if (func_00127BF8(mode, D_004B96F8)) {
        flags |= 0x10;
    }
    dev = fsFindDevice(path);
    if (dev) {
        return fsDeviceOpen(dev, path, flags);
    }
    return 0;
}

/* Opens path on the first free slot of dev. */
FsFile *fsDeviceOpen(FsDevice *dev, const char *path, s32 flags)
{
    FsFile *file = 0;
    u32 i;

    for (i = 0; i < dev->numSlots; i++) {
        FsFile *f = dev->slot(i);
        if (f->status() == 0) {
            file = f;
            break;
        }
    }
    if (file) {
        dev->lastError = file->open(dev, path, flags);
        if (dev->lastError) {
            file = 0;
        }
    } else {
        dev->lastError = 3;
    }
    if (file) {
        file->state = 2;
    }
    return file;
}

/* Sets up dev; a prefix ("xxx" of "xxx:path") registers it for fsFindDevice. */
s32 fsDeviceInit(FsDevice *dev, u32 numSlots, const char *prefix)
{
    u32 i;

    dev->numSlots = numSlots;
    dev->lastError = 0;
    dev->unk4 = 1;
    if (prefix) {
        if (fsNumDevices >= 2) {
            dev->lastError = 1;
            return 0;
        }
        fsDevices[fsNumDevices] = dev;
        strncpy(D_00676470[fsNumDevices], prefix, 4);
        fsNumDevices++;
    }
    for (i = 0; i < dev->numSlots; i++) {
        dev->slot(i)->unk18 = 0;
    }
    return 1;
}

/* Sets the device used for paths without a prefix. */
void fsSetDefaultDevice(FsDevice *dev)
{
    fsDefaultDevice = dev;
}

/* Joins a and b into dst (size bytes), turning b's '/' and '\\' into sep. */
char *fsJoinPath(char *dst, s32 size, const char *a, const char *b, char sep)
{
    s32 lenA = strlen(a);
    s32 lenB = strlen(b);
    char *p;
    s32 i;
    char c;

    if (lenA + lenB > size - 1) {
        return 0;
    }
    strcpy(dst, a);
    p = dst + lenA;
    for (i = 0; i <= lenB; i++) {
        c = b[i];
        if (c == '/' || c == '\\') {
            p[i] = sep;
        } else {
            p[i] = c;
        }
    }
    return dst;
}

/* Skips a device prefix ("xxx:"). */
const char *fsSkipDevicePrefix(const char *path)
{
    u32 i;

    for (i = 0; i < 4; i++) {
        if (path[i] == ':') {
            path += i + 1;
            break;
        }
    }
    return path;
}

/* The device for path: the one registered under its prefix, else the default. */
FsDevice *fsFindDevice(const char *path)
{
    FsDevice *dev = fsDefaultDevice;
    u32 i;
    char prefix[5];

    for (i = 0; i < 4; i++) {
        if (path[i] == ':') {
            strncpy(prefix, path, i + 1);
            prefix[i + 1] = 0;
            for (i = 0; i < fsNumDevices; i++) {
                if (func_00127520(prefix, D_00676470[i], 4) == 0) {
                    dev = fsDevices[i];
                    break;
                }
            }
            break;
        }
    }
    return dev;
}

/* Installs the rwf* functions in RenderWare's file interface, saving the old ones; 1 if it did now. */
s32 fsInstallRwFileInterface(void)
{
    void **funcs;

    if (fsRwInstalled) {
        return 0;
    }
    funcs = (void **)func_001D8B88();
    D_00676440[0] = funcs[0];
    D_00676444[0] = funcs[1];
    D_00676448[0] = funcs[2];
    D_0067644C[0] = funcs[3];
    D_00676450[0] = funcs[4];
    D_00676454[0] = funcs[5];
    D_00676458[0] = funcs[6];
    D_0067645C[0] = funcs[7];
    D_00676460[0] = funcs[8];
    D_00676464[0] = funcs[9];
    D_00676468[0] = funcs[10];
    funcs[1] = (void *)fsRwOpen;
    funcs[2] = (void *)fsRwClose;
    funcs[3] = (void *)fsRwRead;
    funcs[4] = (void *)fsRwWrite;
    funcs[5] = (void *)fsRwGets;
    funcs[6] = (void *)fsRwPuts;
    funcs[7] = (void *)fsRwEof;
    funcs[8] = (void *)fsRwSeek;
    funcs[9] = (void *)fsRwFlush;
    funcs[0] = (void *)fsRwExists;
    funcs[10] = (void *)fsRwTell;
    fsRwInstalled = 1;
    return 1;
}

}
