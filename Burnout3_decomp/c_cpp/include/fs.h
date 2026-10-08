#ifndef FS_H
#define FS_H

#include "types.h"

/* Criterion's file system (GameShared\GameClasses\FileSystem\GTFileSystem.cpp in Burnout 2; here d4/fs,
 * 0x212580-0x212E30): file systems ("devices", picked by a "xxx:" prefix) hold open files, and static F* callbacks
 * replace RenderWare's file interface (RwFileFunctions). Class, method and field names are Criterion's, from Burnout 2's
 * debug info (docs/burnout2.md). Virtual signatures keep the types our callers matched with; Burnout 2's differ in places
 * (SetPosition takes and returns 64-bit values there). MW vtables start with two header words: first virtual at +0x08. */

class CGTFileSystem;

/* An open file (Burnout 2: 0x28 bytes). */
class CGTFile {
public:
    virtual s32 Open(CGTFileSystem *fs, const char *path, s32 flags); /* vtable +0x08, returns an error code */
    virtual void Close();                                             /* vtable +0x0C */
    virtual u32 Read(void *buf, u32 size);                            /* vtable +0x10 */
    virtual u32 Write(const void *buf, u32 size);                     /* vtable +0x14 */
    virtual s32 SetPosition(s32 offset, s32 origin);                  /* vtable +0x18, origin as SEEK_SET/CUR/END */
    virtual void Sync(s32 block);                                     /* vtable +0x1C */
    virtual void Abort();                                             /* vtable +0x20 */
    virtual s32 GetStatus();                                          /* vtable +0x24, 2 while busy */

    /* 0x00 vtable */
    CGTFileSystem *mpFileSys; /* 0x04 */
    s64 mnFileLength;         /* 0x08 */
    s64 mnFilePosition;       /* 0x10 */
    s32 mnStatus;             /* 0x18, cleared by Init */
    s32 mnError;              /* 0x1C */
    s32 mbAsynchronousFile;   /* 0x20 */
    s32 mnOpenState;          /* 0x24, set to 2 by CGTFileSystem::Open (not in Burnout 2's layout) */
};

/* A file system: a device that hands out file objects. */
class CGTFileSystem {
public:
    virtual s32 FileExists(const char *path);         /* vtable +0x08 */
    virtual s32 GetStatus();                          /* vtable +0x0C */
    virtual CGTFile *GetFileObject(u32 index);        /* vtable +0x10 */

    /* 0x00 vtable */
    s32 mnStatus;    /* 0x04 */
    s32 mnLastError; /* 0x08 */
    u32 mnNumFiles;  /* 0x0C */

    /* RenderWare file interface callbacks (RwFileFunctions order: exist, open, close, read, write, gets, puts, eof,
     * seek, flush, tell); FTell is not in Burnout 2. */
    static s32 FTell(void *fptr);
    static s32 FExist(const char *name);
    static s32 FFlush(void *fptr);
    static s32 FSeek(void *fptr, s32 offset, s32 origin); /* Burnout 2: long offset */
    static s32 FEof(void *fptr);
    static s32 FPuts(const char *buf, void *fptr);
    static char *FGets(char *buf, s32 maxLen, void *fptr);
    static u32 FWrite(const void *addr, u32 size, u32 count, void *fptr);
    static u32 FRead(void *addr, u32 size, u32 count, void *fptr);
    static s32 FClose(void *fptr);
    static void *FOpen(const char *name, const char *access);

    CGTFile *Open(const char *filename, u32 openFlags);            /* on the first free file object */
    s32 Init(u32 maxOpenFiles, const char *rwName);                 /* rwName: "xxx" prefix to register under */
    void SetAsDefaultFilesystem();
    static char *BuildFileName(char *buffer, s32 bufferLength, const char *driveName, const char *filePath, char separator);
    static const char *GetFileNameFromDeviceName(const char *name); /* skips a "xxx:" prefix */
    static CGTFileSystem *GetFileSystemFromFileName(const char *name);
    static s32 InstallToRenderWare();

    /* .sbss */
    static u32 mnNumFilesystems;
    static s32 mbInstalledToRW;
    static CGTFileSystem *mpDefaultFilesystem;
    static CGTFileSystem *mpaFilesystems[2];
};

#endif
