/* d4/fs (0x212580-0x212E30): Criterion's CGTFileSystem (GTFileSystem.cpp in Burnout 2, docs/burnout2.md). File systems
 * hold open files; the static F* callbacks replace RenderWare's file interface (installed by InstallToRenderWare). */

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
}

/* .bss: RenderWare's original file functions, saved by InstallToRenderWare (Burnout 2: mStdRwFileInterface) (likely one RwFileFunctions in the original;
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
extern char D_00676470[2][5]; /* registered prefixes (Burnout 2: msaFSNames) */

/* rwftell */
s32 CGTFileSystem::FTell(void *fptr)
{
    return ((CGTFile *)fptr)->mnFilePosition;
}

/* rwfexist */
s32 CGTFileSystem::FExist(const char *path)
{
    CGTFileSystem *dev = GetFileSystemFromFileName(path);
    if (dev == 0) {
        return 0;
    }
    return dev->FileExists(path);
}

/* rwfflush */
s32 CGTFileSystem::FFlush(void *fptr)
{
    return 0;
}

/* rwfseek */
s32 CGTFileSystem::FSeek(void *fptr, s32 offset, s32 origin)
{
    CGTFile *f = (CGTFile *)fptr;

    switch (origin) {
    case 1:
        f->SetPosition(offset, 1);
        break;
    case 2:
        f->SetPosition(offset, 2);
        break;
    case 0:
        f->SetPosition(offset, 0);
        break;
    default:
        return -1;
    }
    return 0;
}

/* rwfeof */
s32 CGTFileSystem::FEof(void *fptr)
{
    return ((CGTFile *)fptr)->mnFilePosition >= ((CGTFile *)fptr)->mnFileLength;
}

/* rwfputs */
s32 CGTFileSystem::FPuts(const char *buf, void *fptr)
{
    return ((CGTFile *)fptr)->Write(buf, strlen(buf));
}

/* rwfgets */
char *CGTFileSystem::FGets(char *buf, s32 maxLen, void *fptr)
{
    return 0;
}

/* rwfwrite */
u32 CGTFileSystem::FWrite(const void *buf, u32 size, u32 count, void *fptr)
{
    return ((CGTFile *)fptr)->Write(buf, size * count) / size;
}

/* rwfread */
u32 CGTFileSystem::FRead(void *buf, u32 size, u32 count, void *fptr)
{
    return ((CGTFile *)fptr)->Read(buf, size * count) / size;
}

/* rwfclose */
s32 CGTFileSystem::FClose(void *fptr)
{
    ((CGTFile *)fptr)->Close();
    return 0;
}

/* rwfopen */
void *CGTFileSystem::FOpen(const char *path, const char *mode)
{
    s32 flags = 0;
    CGTFileSystem *dev;

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
    dev = GetFileSystemFromFileName(path);
    if (dev) {
        return dev->Open(path, flags);
    }
    return 0;
}

/* Opens path on the first free file object. */
CGTFile *CGTFileSystem::Open(const char *path, u32 flags)
{
    CGTFileSystem *dev = this;
    CGTFile *file = 0;
    u32 i;

    for (i = 0; i < dev->mnNumFiles; i++) {
        CGTFile *f = dev->GetFileObject(i);
        if (f->GetStatus() == 0) {
            file = f;
            break;
        }
    }
    if (file) {
        dev->mnLastError = file->Open(dev, path, flags);
        if (dev->mnLastError) {
            file = 0;
        }
    } else {
        dev->mnLastError = 3;
    }
    if (file) {
        file->mnOpenState = 2;
    }
    return file;
}

/* Sets up the file system; a prefix ("xxx" of "xxx:path") registers it for GetFileSystemFromFileName. */
s32 CGTFileSystem::Init(u32 numSlots, const char *prefix)
{
    u32 i;

    mnNumFiles = numSlots;
    mnLastError = 0;
    mnStatus = 1;
    if (prefix) {
        if (mnNumFilesystems >= 2) {
            mnLastError = 1;
            return 0;
        }
        mpaFilesystems[mnNumFilesystems] = this;
        strncpy(D_00676470[mnNumFilesystems], prefix, 4);
        mnNumFilesystems++;
    }
    for (i = 0; i < mnNumFiles; i++) {
        GetFileObject(i)->mnStatus = 0;
    }
    return 1;
}

/* Sets the file system used for paths without a prefix. */
void CGTFileSystem::SetAsDefaultFilesystem()
{
    mpDefaultFilesystem = this;
}

/* Joins a and b into dst (size bytes), turning b's '/' and '\\' into sep. */
char *CGTFileSystem::BuildFileName(char *dst, s32 size, const char *a, const char *b, char sep)
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
const char *CGTFileSystem::GetFileNameFromDeviceName(const char *path)
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

/* The file system for path: the one registered under its prefix, else the default. */
CGTFileSystem *CGTFileSystem::GetFileSystemFromFileName(const char *path)
{
    CGTFileSystem *dev = mpDefaultFilesystem;
    u32 i;
    char prefix[5];

    for (i = 0; i < 4; i++) {
        if (path[i] == ':') {
            strncpy(prefix, path, i + 1);
            prefix[i + 1] = 0;
            for (i = 0; i < mnNumFilesystems; i++) {
                if (func_00127520(prefix, D_00676470[i], 4) == 0) {
                    dev = mpaFilesystems[i];
                    break;
                }
            }
            break;
        }
    }
    return dev;
}

/* Installs the F* callbacks in RenderWare's file interface, saving the old ones; 1 if it did now. */
s32 CGTFileSystem::InstallToRenderWare()
{
    void **funcs;

    if (mbInstalledToRW) {
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
    funcs[1] = (void *)FOpen;
    funcs[2] = (void *)FClose;
    funcs[3] = (void *)FRead;
    funcs[4] = (void *)FWrite;
    funcs[5] = (void *)FGets;
    funcs[6] = (void *)FPuts;
    funcs[7] = (void *)FEof;
    funcs[8] = (void *)FSeek;
    funcs[9] = (void *)FFlush;
    funcs[0] = (void *)FExist;
    funcs[10] = (void *)FTell;
    mbInstalledToRW = 1;
    return 1;
}
