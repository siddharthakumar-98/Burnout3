#ifndef FS_H
#define FS_H

#include "types.h"

/* File system (game/unit_00212580, 0x212580-...): devices that hold open files, and the RenderWare
 * RwFileFunctions replacements installed by fsInstallRwFileInterface. Names are ours; offsets from the code. */

#ifdef __cplusplus

/* An open file. MW vtables start with two header words, so the first virtual is at +0x08. */
class FsFile {
public:
    virtual s32 open(void *dev, const char *path, s32 flags);  /* vtable +0x08 */
    virtual void close();                                      /* vtable +0x0C */
    virtual u32 read(void *buf, u32 size);                     /* vtable +0x10 */
    virtual u32 write(const void *buf, u32 size);              /* vtable +0x14 */
    virtual s32 seek(s32 offset, s32 whence);                  /* vtable +0x18, whence as SEEK_SET/CUR/END */
    virtual void unk1C(s32 arg);                               /* vtable +0x1C */
    virtual void unk20();                                      /* vtable +0x20 */
    virtual s32 status();                                      /* vtable +0x24 */

    /* 0x00 vtable */
    s32 unk4;  /* 0x04 */
    s64 size;  /* 0x08 */
    s64 pos;   /* 0x10 */
    s32 unk18; /* 0x18, cleared by fsDeviceInit */
    s32 field1C; /* 0x1C */
    s32 field20; /* 0x20 */
    s32 state; /* 0x24, 2 once opened by fsDeviceOpen */
};

/* A device (a place files come from), picked by path in fsFindDevice. */
class FsDevice {
public:
    virtual s32 exists(const char *path);  /* vtable +0x08 */
    virtual void unk0C();                  /* vtable +0x0C */
    virtual FsFile *slot(s32 i);           /* vtable +0x10 */

    /* 0x00 vtable */
    s32 unk4;      /* 0x04 */
    s32 lastError; /* 0x08 */
    u32 numSlots;  /* 0x0C */
};

extern "C" {
#else
typedef struct FsFile FsFile;
typedef struct FsDevice FsDevice;
#endif

/* RenderWare file interface (RwFileFunctions order: exist, open, close, read, write, gets, puts, eof, seek, flush, tell). */
s32 fsRwTell(FsFile *f);                                       /* rwftell */
s32 fsRwExists(const char *path);                                /* rwfexist */
s32 fsRwFlush(FsFile *f);                                       /* rwfflush */
s32 fsRwSeek(FsFile *f, s32 offset, s32 whence);               /* rwfseek */
s32 fsRwEof(FsFile *f);                                       /* rwfeof */
s32 fsRwPuts(const char *buf, FsFile *f);                      /* rwfputs */
char *fsRwGets(char *buf, s32 maxLen, FsFile *f);              /* rwfgets */
u32 fsRwWrite(const void *buf, u32 size, u32 count, FsFile *f); /* rwfwrite */
u32 fsRwRead(void *buf, u32 size, u32 count, FsFile *f);       /* rwfread */
s32 fsRwClose(FsFile *f);                                       /* rwfclose */
FsFile *fsRwOpen(const char *path, const char *mode);          /* rwfopen */

FsFile *fsDeviceOpen(FsDevice *dev, const char *path, s32 flags);
s32 fsDeviceInit(FsDevice *dev, u32 numSlots, const char *prefix);
void fsSetDefaultDevice(FsDevice *dev);
char *fsJoinPath(char *dst, s32 size, const char *a, const char *b, char sep);
const char *fsSkipDevicePrefix(const char *path);
FsDevice *fsFindDevice(const char *path);
s32 fsInstallRwFileInterface(void);

#ifdef __cplusplus
}
#endif

#endif
